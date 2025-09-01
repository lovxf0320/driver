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

#define LED_BUF_MAGIC        'B'
#define LED_STRUCT_MAGIC     'S'
//_IOWR(type, number, data_type)
/*
| �ֶ�        | ˵��                                                    |
| ----------- | ------------------------------------------------------- |
| type      | һ���ַ�����ʶ�豸���ͣ������� 'L' ��ʾ led��         |
| number    | �����ţ�ÿ������Ψһ��                                |
| data_type | ��������ݽṹ���ͣ����Զ������С��                    |
| _IOWR     | ��ʾ���� **˫����**���û��ռ����ں˿ռ䶼Ҫ��������   |
#define CMD_LED_ON             _IOW(LED_MAGIC,0,unsigned long)
#define CMD_LED_OFF            _IOW(LED_MAGIC,1,unsigned long)
| �ֶ�  | ����                        |
| ----- | --------------------------- |
| 'L' | ���ͱ�ʶ����ʾ LED ������   |
| 3   | �����ţ��� 4 ������       |
| _IO | ��ʾ�޲�������              |
#define CMD_LED_ALL_ON         _IO(LED_MAGIC,2)
#define CMD_LED_ALL_OFF        _IO(LED_MAGIC,3)
*/
#define CMD_BUF_W       _IOW(LED_BUF_MAGIC,0,int[4])
#define CMD_BUF_R       _IOW(LED_BUF_MAGIC,1,int[4])

#define CMD_STRUCT_W    _IOW(LED_STRUCT_MAGIC,2,struct led_cfg)
#define CMD_STRUCT_R    _IOW(LED_STRUCT_MAGIC,3,struct led_cfg)

struct led_cfg {
    int index;   // �ڼ���LED������ 0 ��Ӧ D7��
    int state;   // ״̬��1 ��ʾ����0 ��ʾ��
};

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "D7 LED" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "D8 LED" }, /* default to OFF */
	{ PAD_GPIO_C+8,  GPIOF_OUT_INIT_HIGH, "D9 LED" }, /* default to OFF */	
	{ PAD_GPIO_C+7,  GPIOF_OUT_INIT_HIGH, "D10 LED" }, /* default to OFF */	
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
| file | struct file * | �ں��еġ����ļ������󣬼�¼�豸״̬�������Ƿ�Ϊ O\_NONBLOCK ģʽ��     |
| cmd  | unsigned int  | �����루�� _IO/_IOW �Ⱥ깹�죩������������Ҫ��ʲô������ LED\_ON    |
| arg  | unsigned long | �����������������û���������ֵ���ṹ���ַ��(�û��ռ��ַ��ֵ��)                            |
*/
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//ioctl �����Ĳ���ֻ��һ�� ͨ�ÿ��ӣ���ʽ���������ĺ��壨��ʲô���ݡ���ֵ����ָ�룩ȡ��������ô �����ʹ������ꡣ
long myled_unlocked_ioctl (struct file *file, unsigned int cmd, unsigned long args){
	int i;
	int rt=0;
	int n=args-7;
	int arr_read[4]={1,1,1,1};
	int arr_write[4]={0};
	struct led_cfg* cfg;
	
	
	/*ioctl() �ĵ� 3 �������� unsigned long args����һ���û��ռ��ַ��ֵ��
	��������� _IOW/_IOR/_IOWR����� args 
	ʵ������ һ��ָ�루��ַ����ָ���û����ݵ����ݡ�*/
	void __user *argp=(void __user *)args;
	/*__user �� Linux �ں˵�һ����ǣ��꣩��
	���߱�����/��̬�����������ָ���ǡ��û��ռ��ָ�롱��
	�������Է�ֹ��������ȥֱ�ӷ����ں˿ռ䣬����������ȫ������
	��������� copy_from_user() / copy_to_user() ������
	�ͱ��뱣֤�����ָ�����û��ռ�ġ�*/
	
	unsigned int value;
	/*��� value ��ʵ��Ҫ�����ֵ������û��ռ䴫�������� ���뵽�����棨write����
	�������ֵд�����û��ռ䣨read��*/
	
	if(_IOC_TYPE(cmd)!=LED_MAGIC){
		return -ENOIOCTLCMD;
	}
	
	switch(cmd){
		case CMD_BUF_W:{
			copy_from_user(arr_write,argp,sizeof(arr_write))
			for(i=0;i<ARRAY_SIZE(leds_gpios);i++){
				gpio_set_value(leds_gpios[i].gpio,argp[i]);
			}
		}break;
		
		case CMD_BUF_R:{
			copy_to_user(argp,arr_raed,sizeof(arr_write));
		}break;
		
		case CMD_STRUCT_W:{
			copy_from_user(&cfg, argp, sizeof(cfg));
			if (cfg.index >= 0 && cfg.index < ARRAY_SIZE(leds_gpios))
                gpio_set_value(leds_gpios[cfg.index].gpio, cfg.state);
		}break;
		
		case CMD_STRUCT_R:{
			cfg.index = 1;
            cfg.state = gpio_get_value(leds_gpios[cfg.index].gpio);
            if (copy_to_user(argp, &cfg, sizeof(cfg)))
                return -EFAULT;
		}break;
		
		default:
			return -ENOIOCTLCMD;
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
	//ʹ�û����豸�Ϳ��Է�������struct cdev�������豸�ţ�register_chrdev_region\alloc_chrdev_region��
	//����ʼ��cdev��cdev_init������cdev���뵽�ںˣ�cdev_add����
	//�Զ������豸�ţ�class_create��device_create��
	
	/*1.�����豸��ע�� */
	rt = misc_register(&myled_misc);
	if(rt < 0)
	{
		printk(KERN_ERR"misc_register fail\n");

		goto err_misc_register;			
		
	}	
	
	/*2.��������gpio���� */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	//��Ϊ�ں��Ѿ���������Щ���ţ������ͷ�֮�󣬲��ܵ���gpio_request_array����
	rt = gpio_request_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	if(rt < 0)
	{
		printk(KERN_ERR"gpio_request_array fail\n");

		goto err_gpio_request_array;			
		
	}

	printk(KERN_INFO"myled_init\n");
	return 0;

err_gpio_request_array:
	misc_deregister(&myled_misc);	
err_misc_register:
	return rt;
}

static void __exit myled_exit(void)
{
	/* ע�������豸 */
	misc_deregister(&myled_misc);
	
	/* �ͷ�gpio���� */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	
	
	printk(KERN_INFO"myled_exit\n");	
	
}

module_init(myled_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(myled_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is led driver");
MODULE_LICENSE("GPL");
/*
| ��ʽ         | ����                        | �Ƿ���Ҫ copy_from_user / copy_to_user   |
| ------------ | --------------------------- | -------------------------------------------- |
| **ֵ����**   | args ��һ��ֱ�ӵ���ֵ     | ? ����Ҫ������ֱ��ʹ�� args ����          |        |
| **ָ�봫��** | args ���û��ռ�ĵ�ַָ�� | ? ��Ҫʹ�� copy_from_user / copy_to_user|
*/
