#include <linux/init.h>		//__init __exit
#include <linux/kernel.h>	//printk
#include <linux/module.h>	//module_init module_exit
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

static int gpio_num[4];
static const char* gpio_name[4];

static int gec6818_led_open(struct inode *inode, struct file *file){
    int i=0;

    for(i=0;i<4;i++){
        gpio_direction_output(gpio_num[i],1);
    }

    printk("gec6818_led_open\n");

    return 0;
}

static int gec6818_led_close(struct inode *inode, struct file *file){
    printk("gec6818_led_close\n");
    return 0;
}

static ssize_t gec6818_led_wirte(struct file *file, const char __user *buf, size_t len, loff_t *off){
    int rt;
    int i;
    char kbuf[2]={0};

    if(len>sizeof(kbuf)){
        return -EINVAL;
    }

    rt = copy_from_user(kbuf,buf,len);
	if(rt !=0 )
		return -EFAULT;


    if(kbuf[0]<7||kbuf[0]>10){
        return -EINVAL;
    }

    for(i=7;i<11;i++){
        if(kbuf[0]==i)
            break;
    }

    i=i-7;

    if(kbuf[1]){
        gpio_set_value(gpio_num[i],0);
    }else{
        gpio_set_value(gpio_num[i],1);
    }

    len=len-rt;

    return len;
}


static const struct file_operations gec6818_led_fops={
    .open   =gec6818_led_open,
    .release=gec6818_led_close,
    .write  =gec6818_led_wirte,
    .owner  =THIS_MODULE,
};


static struct miscdevice gec6818_led_misc={
    .minor  =MISC_DYNAMIC_MINOR,
    .name   ="myled",
    .fops   =&gec6818_led_fops,
};


static int __devinit gec6818_led_probe(struct platform_device *pdev){
    int rt;
    int i;

    struct resource *res;

    rt=misc_register(&gec6818_led_misc);
    if(rt<0){
        printk("misc_register fail\n");

        return rt;
    }

    for(i=0;i<4;i++){
        //获取平台设备传递过来的资源
        res=platform_get_resource(pdev,IORESOURCE_IO,i);
        if(res==NULL){
            printk("platfrom_get_resouce fail\n");
            rt=-ENOMEM;
            goto platform_get_resource_fail;
        }

        //获取GPIO的引脚
        gpio_num[i]=res->start;

        //获取GPIO的引脚名字
        gpio_name[i]=res->name;
    }


    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);


        //申请GPIO
        rt=gpio_request(gpio_num[i],gpio_name[i]);
        if(rt<0){
            printk("gpio_request_fail\n");

            goto gpio_request_fail;
        }
    }

    return 0;

gpio_request_fail:
    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);
    }

platform_get_resource_fail:
    misc_deregister(&gec6818_led_misc);
    return rt;

}


static int __devexit gec6818_led_remove(struct platform_device *pdev){
    int i=0;

    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);
    }

    misc_deregister(&gec6818_led_misc);

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


static int __init gec6818_drv_init(void){
    int rt=platform_driver_register(&gec6818_led_driver);

    printk("gec6818_drv_init\n");

    return rt;
}

static void __exit gec6818_drv_exit(void){
    platform_driver_unregister(&gec6818_led_driver);

    printk("gec6818_drv_exit\n");
}


module_init(gec6818_drv_init);
module_exit(gec6818_drv_exit);

MODULE_AUTHOR("LI XIAOFAN");		//作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证
