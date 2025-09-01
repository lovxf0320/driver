#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/platform.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>

#define CMD_LED_ON       _IOW('L',0,unsigned int)
#define CMD_LED_OFF      _IOW('L',1,unsigned int)


struct gpio_ext{
    int num;        //有效引脚的数量
#define MAX_GPIO_NUMBER 32
    struct gpio pin[MAX_GPIO_NUMBER];
};

struct gpio_ext *led_gpio;


int myled_open(struct inode *inode,struct file *file){
    printk(KERN_INFO"myled_open\n");

    return 0;
}

int myled_close(struct inode *inode,struct file *file){
    printk(KERN_INFO"myled_close\n");

    return 0;
}

long myled_ioctl(struct file *filp, unsigned int cmd, unsigned long args){
    unsigned int i=args-7;

    switch(cmd){
        case CMD_LED_ON:
            gpio_set_value(led_gpio->pin[i].gpio,0);
        break;

        case CMD_LED_OFF:
            gpio_set_value(led_gpio->pin[i].gpio,1);
        break;

        default:
            return -ENOIOCTLCMD;
    }

    return 0;
}

static const struct file_operations myled_fops={
    .owner          =THIS_MODULE,
    .unlocked_ioctl =myled_ioctl,
    .open           =myled_open,
    .release        =myled_close,
};

static struct miscdevice myled_misc={
    .minor  =MISC_DYNAMIC_MINOR,
    .name   ="myled",
    .fops   =&myled_fops,
};

static int __devinit myled_probe(struct platform_device *pdev){
    int rt;

    rt=misc_register(&myled_misc);
    if(rt<0){
        printk(KERN_ERR"misc_register_fail\n");

        goto fail_misc_register;
    }

    led_gpio=(struct gpio_ext*)pdev->dev.platform_data;

    //gpio_free_array(led_gpio->pin,led_gpio->num);

    rt=gpio_request_array(led_gpio->pin,led_gpio->num);
    if(rt<0){
        printk(KERN_ERR"gpio_request_array fail\n");
		
		goto fail_gpio_request_array;
    }

    printk(KERN_INFO"myled_probe\n");

fail_gpio_request_array:
    misc_deregister(&myled_misc);

fail_misc_register:
    return 0;
}

static int __devexit myled_remove(struct platform_device *pdev){
    gpio_free_array(led_gpio->pin,led_gpio->num);

    misc_deregister(&myled_misc);

    printk(KERN_INFO"myled_remove\n");

    return 0;
}

void myled_shutdown(struct platform_device *pdev){
    int i;

    //熄灭所有LED灯
    for(i=0;i<led_gpio->num;i++){
        gpio_set_value(led_gpio->pin[i].gpio,1);
    }

	printk(KERN_INFO"myled_shutdown\n");

}

static int myled_suspend(struct platform_device *pdev,pm_message_t state){
    int i;

    //熄灭所有LED灯
    for(i=0;i<led_gpio->num;i++){
        gpio_set_value(led_gpio->pin[i].gpio,1);
    }

	printk(KERN_INFO"myled_suspend\n");

    return 0;
}

static int myled_resume(struct platform_device *pdev){
    int i;

    for(i=0;i<led_gpio->num;i++){
        gpio_set_value(led_gpio->pin[i].gpio,1);
    }

	printk(KERN_INFO"myled_resume\n");

    return 0;
}

static struct platform_driver myled_platdrv={
    .probe      =myled_probe,
    .remove     =myled_remove,
    .shutdown   =myled_shutdown,
    .suspend    =myled_suspend,
    .resume     =myled_resume,
    .driver     ={
        .owner  =THIS_MODULE,
        .name   ="myled_plat",
    },
};

static int __init myled_init(void){
    int rt=platform_driver_register(&myled_platdrv);

    if(rt<0){
        printk(KERN_ERR"platform_driver_register fail\n");
    }else{
        printk(KERN_INFO"platform_driver_register suceess\n");
    }

    return rt;
}

static void __exit myled_exit(void){
    platform_driver_unregister(&myled_platdrv);

    printk(KERN_INFO"platform_driver_unregister\n");
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证