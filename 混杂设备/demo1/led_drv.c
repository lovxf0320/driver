#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/platform.h>
#include <linux/miscdevice.h>

/*#include <linux/miscdevice.h>

struct miscdevice {
    int minor;                    		// ���豸�ţ�MISC_DYNAMIC_MINOR Ϊ�Զ����䣩
    const char *name;             		// �� /dev �´����豸�ڵ���
    const struct file_operations *fops; // �ļ������ṹ��
    struct list_head list;
    struct device *parent;
    struct device *this_device;
    const struct attribute_group **groups;
};
*/
static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "D7 LED" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "D8 LED" }, /* default to OFF */
	{ PAD_GPIO_C+8,  GPIOF_OUT_INIT_HIGH, "D9 LED" }, /* default to OFF */	
	{ PAD_GPIO_C+7,  GPIOF_OUT_INIT_HIGH, "D10 LED" }, /* default to OFF */	
};

int myled_open (struct inode * inode, struct file * file)
{
	printk(KERN_INFO"myled_open\n");
	return 0;
}

int myled_release (struct inode *inode, struct file *file)
{
	printk(KERN_INFO"myled_release\n");
	return 0;
}

ssize_t myled_write (struct file *file, const char __user *buf, size_t len, loff_t *loff)
{

	int rt;
	
	char kbuf[2]={0};
	int n=0;
	
	if(buf == NULL)
		return -EFAULT;

	if(len > sizeof kbuf)
		len = sizeof kbuf;

	rt = copy_from_user(kbuf,buf,len);

	rt = len - rt; //�õ��Ѿ������ɹ����ֽ���
	
	//kbuf[0]:7~10������ָ�������ĸ�led�ƣ�D7 ~ D10
	//kbuf[1]:1-���� 0-Ϩ��
	
	n = kbuf[0] - 7;//0~3

	gpio_set_value(leds_gpios[n].gpio,!kbuf[1]);  

	printk(KERN_INFO"[myled_write]:kbuf[0]=%d kbuf[1]=%d\n",kbuf[0],kbuf[1]);

	return rt;
	
}

static struct file_operations myled_fops={
	.owner = THIS_MODULE,
	.open = myled_open,
	.release = myled_release,
	.write = &myled_write,
	//myled_write ��һ���������������Ͼ��Ǻ���ָ�룬�Զ����˻�Ϊָ�����ͣ�
	//&myled_write �Ǻ�����ַ�������ڸ�ֵʱЧ��һ����
	//����ָ��ĸ�ֵʱ��&������ �� ������ �ǵȼ۵ġ�
};

static struct miscdevice myled_misc={
	.minor=MISC_DYNAMIC_MINOR,
	.name="myled",
	.fops=&myled_fops,
	//�����.fops��һ���ṹ��ָ�룬myled_fops��һ���ṹ���������
	//���ı����ϲ���һ��ָ��
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

MODULE_AUTHOR("Teacher.Wen");
MODULE_DESCRIPTION("This is led driver");
MODULE_LICENSE("GPL");