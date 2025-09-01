#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/delay.h>

//未按下时，GPIO 引脚被上拉（高电平）；
//按下时，GPIO 引脚被接地（低电平）；



static dev_t key_num=0;
static struct cdev key_dev;

static struct class *key_class;
static struct device *key_device;

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
static struct gpio key_gpios[2]={
	//引脚编号，引脚输出模式-初始电平状态为高电平，自定义引脚的名字
	{PAD_GPIO_A+28,GPIOF_IN,"key_2"},
	{PAD_GPIO_B+9,GPIOF_IN,"key_6"},
};


ssize_t key_read(struct file *file, char __user *buf, size_t len, loff_t *pos){
	char key_val=0;
	
	int rt = 0;
	
	if(len>1){
		len=1;
	}
	
	if(buf==NULL){
		return -EINVAL;
	}
	
	/*if(len>sizeof(kbuf)){
		return -EINVAL;
	}*/
	
		
	//读取K2按键的引脚电平
	key_val|=gpio_get_value(PAD_GPIO_A+28)?0:(1<<0);
	
	//读取K6按键的引脚电平
	key_val|=gpio_get_value(PAD_GPIO_B+9)?0:(1<<1);
	
	//将内核空间的数据传递用户空间
	rt = copy_to_user(buf,&key_val,len);
	if(rt)
	{
		//返回错误码，可以用过应用层的perror进行得到
		return -EFAULT;
	}
	
	//获得实际拷贝的字节数

	len = len - rt;
	
	return len;
		
	//long copy_from_user(void *to, const void __user *from, unsigned long n);
}

int key_open(struct inode *inode, struct file *file){
	printk(KERN_INFO"key_open\n");
	return 0;
}

int key_release(struct inode *inode, struct file *file){
	printk(KERN_INFO"key_release\n");
	return 0;
}

static const struct file_operations key_fops={
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_release,
	.read = key_read,
};

int __init key_init(void){
	int rt=0;
	
	rt=alloc_chrdev_region(&key_num,0,1,"mykey");
	if(rt<0){
		printk(KERN_ERR"err_alloc_chrdev_region\n");
		goto err_alloc_chrdev_region;
	}
	
	printk("major=%d\n",MAJOR(key_num));
	printk("minor=%d\n",MINOR(key_num));
	
	cdev_init(&key_dev,&key_fops);
	rt=cdev_add(&key_dev,key_num,1);
	if(rt<0){
		printk(KERN_ERR"err_cdev_add\n");
		goto err_cdev_add;
	}
	
	key_class = class_create(THIS_MODULE,"key_class");  //创建一个设备类 key_class，会在 /sys/class/key_class/ 生成对应目录。
	if(IS_ERR(key_class)){
		printk(KERN_ERR"err_class_create\n");
		goto err_class_create;
	}
	
	key_device = device_create(key_class,NULL,key_num,NULL,"mykey");  // /dev/mykey
	if(IS_ERR(key_device)){
		printk(KERN_ERR"err_device_create\n");
		goto err_device_create;
	}
	
	gpio_free_array(key_gpios, ARRAY_SIZE(key_gpios));
	
	rt=gpio_request_array(key_gpios, ARRAY_SIZE(key_gpios));
	if(rt<0){
		printk(KERN_ERR"err_gpio_request_array\n");
		goto err_gpio_request_array;
	}
	printk(KERN_INFO"lxf is success\n");
	
	return 0;
	
err_gpio_request_array:
	device_destroy(key_class, key_num); 
	
err_device_create:
	class_destroy(key_class); 
	
err_class_create:
	cdev_del(&key_dev);
	
err_cdev_add:
	unregister_chrdev_region(key_num,1);
		
err_alloc_chrdev_region:
	return rt; 
	
}

void __exit key_exit(void){
	gpio_free_array(key_gpios, ARRAY_SIZE(key_gpios));
	device_destroy(key_class, key_num); 
	class_destroy(key_class); 
	cdev_del(&key_dev);
	unregister_chrdev_region(key_num,1);
	return ; 
}

EXPORT_SYMBOL(key_init);
EXPORT_SYMBOL(key_exit);

MODULE_AUTHOR("LIXIAOFAN");
MODULE_DESCRIPTION("this is a led driver");
MODULE_LICENSE("GPL");