#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/delay.h>

static dev_t beep_num=0;
static struct cdev beep_dev;

static struct class *beep_class;
static struct device *beep_device;

/**
 * struct gpio - a structure describing a GPIO with configuration
 * @gpio:	the GPIO number
 * @flags:	GPIO configuration as specified by GPIOF_*
 * @label:	a literal description string of this GPIO
 *
struct gpio {
	unsigned	gpio;      //引脚编号
	unsigned long	flags; //引脚模式
	const char	*label;    //自动逸引脚名称
};
*/
static struct gpio beep_gpios[1]={
	{PAD_GPIO_C+14,GPIOF_OUT_INIT_LOW,"BEEP_C7"},
};

ssize_t beep_write(struct file *file, const char __user *buf, size_t len, loff_t *pos){
	unsigned kbuf=0;
	int rt = 0;
	
	if(buf==NULL){
		return -EINVAL;
	}
	
	if(len>sizeof(kbuf)){
		return -EINVAL;
	}
	
	//unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	//void *to是“内核空间的地址”，应该传&kbuf
	//蜂鸣器高电平开始响(1),低电平停止。
	rt=copy_from_user(&kbuf,buf,len);
	if(rt==0){
		gpio_set_value(PAD_GPIO_C+14,kbuf);
	}
	
	len=len-rt;
	
	return len;
}

int beep_open(struct inode *inode, struct file *file){
	printk(KERN_INFO"beep_open\n");
	return 0;
}

int beep_release(struct inode *inode, struct file *file){
	printk(KERN_INFO"beep_release\n");
	return 0;
}

static const struct file_operations beep_fops={
	.owner = THIS_MODULE,
	.open = beep_open,
	.release = beep_release,
	.write = beep_write,
};

int __init beep_init(void){
	int rt=0;
	
	rt=alloc_chrdev_region(&beep_num,0,1,"mybeep");
	if(rt<0){
		printk(KERN_ERR"err_alloc_chrdev_region\n");
		goto err_alloc_chrdev_region;
	}
	
	printk("major=%d\n",MAJOR(beep_num));
	printk("minor=%d\n",MINOR(beep_num));
	
	cdev_init(&beep_dev,&beep_fops);
	rt=cdev_add(&beep_dev,beep_num,1);
	if(rt<0){
		printk(KERN_ERR"err_cdev_add\n");
		goto err_cdev_add;
	}
	
	beep_class = class_create(THIS_MODULE,"beep_class");  //创建一个设备类 beep_class，会在 /sys/class/beep_class/ 生成对应目录。
	if(IS_ERR(beep_class)){
		printk(KERN_ERR"err_class_create\n");
		goto err_class_create;
	}
	
	beep_device = device_create(beep_class,NULL,beep_num,NULL,"mybeep");  // /dev/mybeep
	if(IS_ERR(beep_device)){
		printk(KERN_ERR"err_device_create\n");
		goto err_device_create;
	}
	
	gpio_free_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	
	rt=gpio_request_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	if(rt<0){
		printk(KERN_ERR"err_gpio_request_array\n");
		goto err_gpio_request_array;
	}
	printk(KERN_INFO"lxf is success\n");
	
	return 0;
	
err_gpio_request_array:
	device_destroy(beep_class, beep_num); 
	
err_device_create:
	class_destroy(beep_class); 
	
err_class_create:
	cdev_del(&beep_dev);
	
err_cdev_add:
	unregister_chrdev_region(beep_num,1);
		
err_alloc_chrdev_region:
	return rt; 
	
}

void __exit beep_exit(void){
	gpio_free_array(beep_gpios, ARRAY_SIZE(beep_gpios));
	device_destroy(beep_class, beep_num); 
	class_destroy(beep_class); 
	cdev_del(&beep_dev);
	unregister_chrdev_region(beep_num,1);
	return ; 
}

EXPORT_SYMBOL(beep_init);
EXPORT_SYMBOL(beep_exit);

MODULE_AUTHOR("LIXIAOFAN");
MODULE_DESCRIPTION("this is a beep driver");
MODULE_LICENSE("GPL");