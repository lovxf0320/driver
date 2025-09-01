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
// struct gpio {
//     unsigned gpio;          /* GPIO 引脚编号 */
//     unsigned long flags;    /* GPIO 标志，例如输入/输出、初始状态等 */
//     const char *label;      /* GPIO 的标签/名称 */
// };

struct gpio_ext{
    int num;        //有效引脚的数量
#define MAX_GPIO_NUMBER 32
    struct gpio pin[MAX_GPIO_NUMBER];
};

static struct gpio_ext leds_gpio={
    .num=4,
    .pin[0]={PAD_GPIO_E+13,GPIOF_OUT_INIT_HIGH,"LED D7"},
    .pin[1]={PAD_GPIO_C+17,GPIOF_OUT_INIT_HIGH,"LED D8"},
    .pin[2]={PAD_GPIO_C+8 ,GPIOF_OUT_INIT_HIGH,"LED D9"},
    .pin[3]={PAD_GPIO_C+7 ,GPIOF_OUT_INIT_HIGH,"LED D10"},
};


static void gec6818_led_dev_release(struct device *dev){
    printk("gec6818_led_dev_release\n");
}


static struct platform_device gec6818_led_device={
    .name           ="gec6818_led",
    .id             =-1,
    .dev            ={
        .release        =gec6818_led_dev_release,
        .platform_data  =&leds_gpio,
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