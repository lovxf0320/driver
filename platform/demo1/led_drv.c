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

static struct resource *gec6818_dev_res;    //存储从平台设备获取的资源信息
static int gpio_num;                        // GPIO引脚号
static const char *gpio_name=NULL;           // GPIO引脚名称


static int gec6818_led_open(struct inode *inode, struct file *file){
    gpio_direction_output(gpio_num,1);

    printk("gec6818_led_open\n");       // 内核日志输出

    return 0;
}

static int gec6818_led_close(struct inode *inode, struct file *file){
    printk("gec6818_led_close\n");

    return 0;
}


static ssize_t gec6818_led_write(struct file *file, const char __user *buf, size_t len, loff_t *off){
    int rt;
    char kbuf[2]={0};   //内核缓冲区

    //1.长度检查
    if(len>sizeof(kbuf)){
        return -EINVAL;  // 无效参数错误
    }

    //2.从用户空间复制数据
    rt=copy_from_user(kbuf,buf,len);

    //3.数据有效性检查
    if(kbuf[0] < 7 || kbuf[0] > 10){
        return -EINVAL;
    }

    //4.控制LED亮灭
    if(kbuf[1])
        gpio_set_value(gpio_num, 0);  // 点亮LED
    else
        gpio_set_value(gpio_num, 1);  // 熄灭LED
        
    //5.计算实际写入字节数
    len = len - rt;
    
    printk("gec6818_led_write\n");
    
    return len;  //返回成功写入的字节数
}


static const struct file_operations gec6818_led_fops={
    .open   = gec6818_led_open,
    .release= gec6818_led_close,
    .write  = gec6818_led_write,
    .owner  = THIS_MODULE,
};


static struct miscdevice gec6818_led_misc={
    .minor  =   MISC_DYNAMIC_MINOR,     //动态分配次设备号
    .name   =   "myled",                 //设备的名字，会在/dev目录当中找到
    .fops   =   &gec6818_led_fops,
};


static int __devinit gec6818_led_probe(struct platform_device *pdev){
    int rt;

    //混杂设备的注册
    rt=misc_register(&gec6818_led_misc);
    if(rt<0){
        printk("misc_register fail\n");
		
		return rt;
    }

    //获取平台设备传递过来的资源
    gec6818_dev_res=platform_get_resource(pdev,IORESOURCE_IO,0);
    if(gec6818_dev_res==NULL){
        printk("platfrom_get_resouce fail\n");
		
		rt = -ENOMEM;
		
		goto platfrom_get_resouce_fail;
    }

    //获取gpio引脚
    gpio_num=gec6818_dev_res->start;

    gpio_free(gpio_num);

    //获取gpio引脚名字
    gpio_name=gec6818_dev_res->name;

    //申请gpio
    rt=gpio_request(gpio_num,gpio_name);
    if(rt<0){
        printk("gpio_request fail\n");
		
		goto gpio_request_fail;		
    }

gpio_request_fail:	
	
platfrom_get_resouce_fail:
	misc_deregister(&gec6818_led_misc);
	
	return rt;	

}

static int __devexit gec6818_led_remove(struct platform_device *pdev){
    gpio_free(gpio_num);

    misc_deregister(&gec6818_led_misc);

    return 0;
}


static struct platform_driver gec6818_led_driver={
    .probe  =   gec6818_led_probe,                  //平台驱动的初始化函数
    .remove =   __devexit_p(gec6818_led_remove),    //平台驱动的卸载函数
    .driver =   {
        .owner  =   THIS_MODULE,
        .name   =   "gec6818_led",                  //平台驱动的名字，必须跟平台设备的名字一样
    },
};


//入口函数
static int __init gec6818_drv_init(void){
    int rt = platform_driver_register(&gec6818_led_driver);

    printk("gec6818_drv_init\n");

    return rt;
}


//出口函数
static void __exit gec6818_drv_exit(void){
    platform_driver_unregister(&gec6818_led_driver);

    printk("gec6818_drv_exit\n");
}


//驱动程序的入口
module_init(gec6818_drv_init);

//驱动程序的出口
module_exit(gec6818_drv_exit);

MODULE_AUTHOR("lixiaofan");		            //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证
