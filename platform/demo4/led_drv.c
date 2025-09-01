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

static int gpio_nums;               //引脚的编号
static struct resource *gpio_res;

int gec6818_led_open(struct inode* inode,struct file* file){
    printk("gec6818_led_open\n");

    gpio_direction_output(gpio_nums,1);

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
    
    switch(kbuf[0]){
        case 7:
        {
            if(kbuf[1])
                gpio_set_value(gpio_nums,0);
            else
                gpio_set_value(gpio_nums,1);
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
    char *gpio_name;

    rt=misc_register(&gec6818_led_miscdev);
    if(rt<0){
        printk("misc_register_fail\n");
        return rt;
    }

    gpio_res=platform_get_resource(pdev,IORESOURCE_IO,0);
    if(gpio_res==NULL){
        printk("platform_get_resource_fail\n");
        return -ENOMEM;
        goto platform_get_resource_fail;
    }

    //获取gpio引脚
    gpio_nums=gpio_res->start;

    //获取引脚的名字
    gpio_name=pdev->dev.platform_data;

    //由于内核已经占用了GPIOE13的资源，为了能够当前实验能够正常执行，先释放内核占用引脚的资源
    gpio_free(gpio_nums);


    rt=gpio_request(gpio_nums,gpio_name);
    if(rt<0){
        printk("gpio_request fail\n");
		
		goto fail_gpio_request;
    }

    //内核打印函数
    printk("gec6818_led_probe\n");

    return 0;

fail_gpio_request:

platform_get_resource_fail:
    misc_deregister(&gec6818_led_miscdev);
    return rt;
}

static int __devexit gec6818_led_remove(struct platform_device *pdev){
    gpio_free(gpio_nums);
    
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

    return platform_driver_register(&gec6818_led_driver);
}

static void __exit gec6818_led_exit(void){

    return platform_driver_unregister(&gec6818_led_driver);
}

module_init(gec6818_led_init);
module_exit(gec6818_led_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证