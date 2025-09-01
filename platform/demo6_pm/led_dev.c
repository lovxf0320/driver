#include <linux/module.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/platform.h>
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


static void myled_dev_release(struct device *dev){
    printk("myled_dev_release\n");
}


static struct platform_device myled_device={
    .name           ="myled_plat",
    .id             =-1,
    .dev            ={
        .release        =myled_dev_release,
        .platform_data  =&leds_gpio,
    },
};

static int __init myled_dev_init(void){
    printk("gec6818_dev_init\n");
    platform_device_register(&myled_device);

    return 0;
}

static void __exit myled_dev_exit(void){
    printk("gec6818_dev_exit\n");
    platform_device_unregister(&myled_device);
}


module_init(myled_dev_init);
module_exit(myled_dev_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("LED Platform Device Driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证