#include <linux/init.h>			//__init  __exit
#include <linux/kernel.h>		//printk
#include <linux/module.h>		//module_init module_exit
#include <linux/cdev.h>			//设备号相关
#include <linux/fs.h>			//file_operations
#include <linux/uaccess.h>		//copy_from_user,copy_to_user
#include <linux/device.h>		//class_create,devices_create
#include <linux/io.h>
#include <linux/gpio.h>			//gpio_request gpio_set_value
#include <cfg_type.h>			//gpio引脚编号
#include <linux/miscdevice.h>	//混杂设备
#include <linux/platform_device.h>

struct gpio_ext{
    int num;        //有效引脚的数量
#define MAX_GPIO_NUMBER 32
    struct gpio pin[MAX_GPIO_NUMBER];
};

struct gpio_ext *leds_gpio;

int gec6818_led_open(struct inode* inode,struct file* file){
    printk("gec6818_led_open\n");

    return 0;
}


int gec6818_led_close(struct inode* inode,struct file* file){
    printk("gec6818_led_close\n");

    return 0;
}


ssize_t gec6818_led_write(struct file *file, const char __user *buf, size_t len, loff_t *off){
    char kbuf[2]={0};
    int rt=0;

    if(buf==NULL){
        return -EINVAL;
    }

    if(len>sizeof(kbuf)){
        len=sizeof(kbuf);
    }

    rt=copy_from_user(kbuf,buf,2);
    if (rt) {
        printk(KERN_ERR "copy_from_user failed: %d\n", rt);
        return -EFAULT;
    }   
    
    switch(kbuf[0]){
        case 7:
        {
            if(kbuf[1])
                gpio_set_value(leds_gpio->pin[0].gpio,0);
            else
                gpio_set_value(leds_gpio->pin[0].gpio,1);
        }break;

        case 8:
        {
            if(kbuf[1])
                gpio_set_value(leds_gpio->pin[1].gpio,0);
            else
                gpio_set_value(leds_gpio->pin[1].gpio,1);
        }break;

        case 9:
        {
            if(kbuf[1])
                gpio_set_value(leds_gpio->pin[2].gpio,0);
            else
                gpio_set_value(leds_gpio->pin[2].gpio,1);
        }break;

        case 10:
        {
            if(kbuf[1])
                gpio_set_value(leds_gpio->pin[3].gpio,0);
            else
                gpio_set_value(leds_gpio->pin[3].gpio,1);
        }break;


        default:break;
    }

    len-=rt;

    return len;
}


static const struct file_operations gec6818_dev_fops={
    .owner  =THIS_MODULE,
    .write  =gec6818_led_write,
    .open   =gec6818_led_open,
    .release=gec6818_led_close,
};

static struct miscdevice gec6818_led_miscdev={
    .minor  =MISC_DYNAMIC_MINOR,
    .name   ="myled",
    .fops   =&gec6818_dev_fops,
};


static int __devinit gec6818_led_probe(struct platform_device *pdev){
    int rt=0;

    rt=misc_register(&gec6818_led_miscdev);
    if(rt<0){
        printk("misc_register_fail\n");

        goto fail_misc_register;
    }

    leds_gpio=(struct gpio_ext *)pdev->dev.platform_data;

    gpio_free_array(leds_gpio->pin,leds_gpio->num);

    rt=gpio_request_array(leds_gpio->pin,leds_gpio->num);
    if(rt<0){
        printk("gpio_request_array fail\n");
		
		goto fail_gpio_request_array;
    }

    //内核打印函数
    printk("gec6818_led_probe\n");

    return 0;

fail_gpio_request_array:
    misc_deregister(&gec6818_led_miscdev);

fail_misc_register:
    return rt;
}

static int __devexit gec6818_led_remove(struct platform_device *pdev){
    gpio_free_array(leds_gpio->pin,leds_gpio->num);
    
    misc_deregister(&gec6818_led_miscdev);

    printk("gec6818_led_remove\n");

    return 0;
}


static struct platform_driver gec6818_led_driver={
    .probe  =gec6818_led_probe,
    .remove =__devexit_p(gec6818_led_remove),
    .driver ={
        .owner  =THIS_MODULE,
        .name   ="gec6818_led",
    },
};

static int __init gec6818_led_init(void){
    int ret = platform_driver_register(&gec6818_led_driver);

    if(ret<0){
        printk(KERN_ERR"platform_driver_register_false");
    }else{
        printk(KERN_ERR"platform_driver_register_false");
    }

    return ret;
}

static void __exit gec6818_led_exit(void){
    platform_driver_unregister(&gec6818_led_driver);

}

module_init(gec6818_led_init);
module_exit(gec6818_led_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证