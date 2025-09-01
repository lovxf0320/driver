//该demo4主要测试platform_device中的platform_data传递给平台驱动
#include <linux/init.h>			    //__init  __exit
#include <linux/kernel.h>		    //printk
#include <linux/module.h>		    //module_init module_exit
#include <linux/cdev.h>			    //设备号相关
#include <linux/fs.h>			    //file_operations
#include <linux/uaccess.h>		    //copy_from_user,copy_to_user
#include <linux/device.h>		    //class_create,devices_create
#include <linux/io.h>
#include <linux/gpio.h>			    //gpio_request gpio_set_value
#include <cfg_type.h>			    //gpio引脚编号
#include <linux/platform_device.h>

static struct resource gec6818_led_resource[]={
    [0]={
        .name   ="led_gpio",
        .start  =PAD_GPIO_E+13,
        .end    =PAD_GPIO_E+13,
        .flags   =IORESOURCE_IO,
    },
};

static void gec6818_led_dev_release(struct device *dev){
    printk("gec6818_led_dev_release\n");
}

static char led_lable[16]="led_d7";

static struct platform_device gec6818_led_device={
    .name           ="gec6818_led",
    .num_resources  =ARRAY_SIZE(gec6818_led_resource),
    .resource       =gec6818_led_resource,
    .dev            ={
        .release        =gec6818_led_dev_release,
        .platform_data  =&led_lable,
    }
};

static int __init gec6818_dev_init(void){
    printk("gec6818_dev_init\n");
    return platform_device_register(&gec6818_led_device);
}

static void __exit gec6818_dev_exit(void){
    printk("gec6818_dev_exit\n");
    platform_device_unregister(&gec6818_led_device);
}


module_init(gec6818_dev_init);
module_exit(gec6818_dev_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证