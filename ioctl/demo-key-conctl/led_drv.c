#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/platform.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
/*���ۣ��ṹ������ͨ����Ҫ for ѭ������
����ж����������磺�ĸ��������£��ĸ��豸״̬Ϊ 0����
���ִ�в��������磺�������ơ��صơ��ͷ���Դ�ȣ�
���ṹ������ + �����ֶΡ� �� ������Ҫд for ��������*/

#define LED_MAGIC				'L'
#define LED_BUF_MAGIC			'B'
#define LED_KEY_MAGIC			'K'
//_IOWR(type, number, data_type)
/*
| �ֶ�        | ˵��                                                    |
| ----------- | ------------------------------------------------------- |
| `type`      | һ���ַ�����ʶ�豸���ͣ������� `'L'` ��ʾ led��         |
| `number`    | �����ţ�ÿ������Ψһ��                                |
| `data_type` | ��������ݽṹ���ͣ����Զ������С��                    |
| `_IOWR`     | ��ʾ���� **˫����**���û��ռ����ں˿ռ䶼Ҫ��������   |
*/
#define CMD_LED_ON             _IOW(LED_MAGIC,0,unsigned long)
#define CMD_LED_OFF            _IOW(LED_MAGIC,1,unsigned long)
/*
| �ֶ�  | ����                        |
| ----- | --------------------------- |
| `'L'` | ���ͱ�ʶ����ʾ LED ������   |
| `3`   | �����ţ��� 4 ������       |
| `_IO` | ��ʾ�޲�������              |

#define CMD_LED_ALL_ON         _IO(LED_MAGIC,2)
#define CMD_LED_ALL_OFF        _IO(LED_MAGIC,3)
����һ�� ���ַ��ַ�������C������ 'A' �ǺϷ����ַ���char����
�� 'ABC' �ǷǷ��ģ�
'LED_MAGIC' �����ɡ������ַ����������������� int��
����һ���Ϸ��� char ��������
*/
#define CMD_KEY_R			   _IOR(LED_KEY_MAGIC,4,unsigned long)
#define CMD_BUF_W			   _IOW(LED_BUF_MAGIC,5,unsigned long)
#define CMD_BUF_R			   _IOW(LED_BUF_MAGIC,6,unsigned long)

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "D7 LED" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "D8 LED" }, /* default to OFF */
	{ PAD_GPIO_C+8,  GPIOF_OUT_INIT_HIGH, "D9 LED" }, /* default to OFF */	
	{ PAD_GPIO_C+7,  GPIOF_OUT_INIT_HIGH, "D10 LED" }, /* default to OFF */	
};

static struct gpio key_gpios[]={
	//���ű�ţ��������ģʽ-��ʼ��ƽ״̬Ϊ�ߵ�ƽ���Զ������ŵ�����
	{PAD_GPIO_A+28,GPIOF_IN,"key_2"},
	{PAD_GPIO_B+30,GPIOF_IN,"key_3"},
	{PAD_GPIO_B+31,GPIOF_IN,"key_4"},
	{PAD_GPIO_B+9,GPIOF_IN,"key_6"},
};

int myled_open (struct inode * inode, struct file * file){
	printk(KERN_INFO"myled_open\n");
	return 0;
}

int myled_release (struct inode *inode, struct file *file){
	printk(KERN_INFO"myled_release\n");
	return 0;
}

/*
| ������ | ����            | ����˵��                                                                |
| ------ | --------------- | ----------------------------------------------------------------------- |
| `file` | `struct file *` | �ں��еġ����ļ������󣬼�¼�豸״̬�������Ƿ�Ϊ O\_NONBLOCK ģʽ��     |
| `cmd`  | `unsigned int`  | �����루�� `_IO`/`_IOW` �Ⱥ깹�죩������������Ҫ��ʲô������ LED\_ON    |
| `arg`  | `unsigned long` | �����������������û���������ֵ���ṹ���ַ��(�û��ռ��ַ��ֵ��)                            |

long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
ioctl �����Ĳ���ֻ��һ�� ͨ�ÿ��ӣ���ʽ���������ĺ��壨��ʲô���ݡ���ֵ����ָ�룩ȡ��������ô�����ʹ������ꡣ*/
long myled_unlocked_ioctl (struct file *file, unsigned int cmd, unsigned long args){
	int i;
	int rt=0;
	/*ioctl() �ĵ� 3 �������� unsigned long args����һ���û��ռ��ַ��ֵ��
	��������� _IOW/_IOR/_IOWR����� args 
	ʵ������ һ��ָ�루��ַ����ָ���û����ݵ����ݡ�*/
	void __user *argp=(void __user *)args;
	/*__user �� Linux �ں˵�һ����ǣ��꣩��
	���߱�����/��̬�����������ָ���ǡ��û��ռ��ָ�롱��
	�������Է�ֹ��������ȥֱ�ӷ����ں˿ռ䣬����������ȫ������
	��������� copy_from_user() / copy_to_user() ������
	�ͱ��뱣֤�����ָ�����û��ռ�ġ�*/
	unsigned int key_val=0;
	/*��� value ��ʵ��Ҫ�����ֵ������û��ռ䴫�������� ���뵽�����棨write����
	�������ֵд�����û��ռ䣨read��*/
	if(_IOC_TYPE(cmd)!=LED_MAGIC&&_IOC_TYPE(cmd)!=LED_BUF_MAGIC&&_IOC_TYPE(cmd)!=LED_KEY_MAGIC){
		return -ENOIOCTLCMD;
	}
	
	if(cmd==CMD_KEY_R){
		for(i=0,key_val=0;i<ARRAY_SIZE(key_gpios);i++){
			if(gpio_get_value(key_gpios[i].gpio)==0){
				key_val|=(1<<i);
			}
		}
		rt=copy_to_user(argp,&key_val,sizeof(key_val));
	}

	if (cmd == CMD_LED_ON) {
        if (args < ARRAY_SIZE(leds_gpios))
            gpio_set_value(leds_gpios[args].gpio, 0);
    } else if (cmd == CMD_LED_OFF) {
        if (args < ARRAY_SIZE(leds_gpios))
            gpio_set_value(leds_gpios[args].gpio, 1);
    }


	return rt;
}

static struct file_operations myled_fops={
	.owner = THIS_MODULE,
	.open = myled_open,
	.release = myled_release,
	.unlocked_ioctl = myled_unlocked_ioctl,
};

static struct miscdevice myled_misc={
	.minor=MISC_DYNAMIC_MINOR,
	.name="myled",
	.fops=&myled_fops,
};

static int __init myled_init(void)
{
	int rt;
	/*ʹ�û����豸�Ϳ��Է�������struct cdev�������豸�ţ�register_chrdev_region\alloc_chrdev_region��
	����ʼ��cdev��cdev_init������cdev���뵽�ںˣ�cdev_add����
	�Զ������豸�ţ�class_create��device_create��*/
	
	/*1.�����豸��ע�� */
	rt = misc_register(&myled_misc);
	if(rt < 0)
	{
		printk(KERN_ERR"misc_register fail\n");

		goto err_misc_register;			
		
	}	
	
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
	rt = gpio_request_array(key_gpios,ARRAY_SIZE(key_gpios));
	if(rt < 0)
	{
		printk(KERN_ERR"key_gpios_request_array fail\n");

		goto err_key_gpios_request_array;			
		
	}
	printk(KERN_INFO"mykey_init\n");
	
	/*2.��������gpio���� */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	//��Ϊ�ں��Ѿ���������Щ���ţ������ͷ�֮�󣬲��ܵ���gpio_request_array����
	rt = gpio_request_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	if(rt < 0)
	{
		printk(KERN_ERR"leds_gpios_request_array fail\n");

		goto err_leds_gpios_request_array;			
		
	}

	printk(KERN_INFO"myled_init\n");
	return 0;

err_leds_gpios_request_array:
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
err_key_gpios_request_array:
	misc_deregister(&myled_misc);
err_misc_register:
	return rt;
}

static void __exit myled_exit(void)
{
	/* �ͷ�gpio���� */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
	/* ע�������豸 */
	misc_deregister(&myled_misc);
	printk(KERN_INFO"myled_exit\n");
}

module_init(myled_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(myled_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is led driver");
MODULE_LICENSE("GPL");
/*
| ��ʽ         | ����                        | �Ƿ���Ҫ `copy_from_user` / `copy_to_user`   |
| ------------ | --------------------------- | -------------------------------------------- |
| **ֵ����**   | `args` ��һ��ֱ�ӵ���ֵ     | ? ����Ҫ������ֱ��ʹ�� `args` ����          |        |
| **ָ�봫��** | `args` ���û��ռ�ĵ�ַָ�� | ? ��Ҫʹ�� `copy_from_user` / `copy_to_user`|
*/