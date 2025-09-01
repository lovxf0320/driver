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
#include <linux/platform_device.h>
#include <cfg_type.h>


static struct resource gec6818_key_resource[]={
    [0]={
		.name  = "gpioe_28",	//GPIO名字
		.start = PAD_GPIO_A+28,//GPIO起始编号
		.end   = PAD_GPIO_A+28,//GPIO结束编号		
		.flags = IORESOURCE_IO,	//资源的类型
	},

	[1]={
		.name  = "gpioc_30",	//GPIO名字
		.start = PAD_GPIO_B+30,	//GPIO起始编号
		.end   = PAD_GPIO_B+30,	//GPIO结束编号
		.flags = IORESOURCE_IO  //资源的类型	
	},
	
	[2]={
		.name  = "gpioc_31",		//GPIO名字		
		.start = PAD_GPIO_B+31,	//GPIO起始编号
		.end   = PAD_GPIO_B+31,	//GPIO结束编号
		.flags = IORESOURCE_IO  //资源的类型
	},
	
	[3]={
		.name  = "gpioc_9",		//GPIO名字		
		.start = PAD_GPIO_B+9,	//GPIO起始编号
		.end   = PAD_GPIO_B+9,	//GPIO结束编号
		.flags = IORESOURCE_IO  //资源的类型
	},
};


static void gec6818_key_dev_release(struct device *dev){
    printk("key dev exit\n");
}

static struct platform_device gec6818_key_device={
    .name           = "gec6818_key",
    .num_resources  = ARRAY_SIZE(gec6818_key_resource),
    .id             = -1,
    .resource		=  gec6818_key_resource,				//资源的来源
    .dev={
        .release    = gec6818_key_dev_release,
    },
};

static int __init gec6818_dev_init(void){
    int rt=platform_device_register(&gec6818_key_device);

    printk("gec6818_key_init \n");

    return rt;
}

static void __exit gec6818_dev_exit(void){
    platform_device_unregister(&gec6818_key_device);

    printk("gec6818_key_exit \n");
}


module_init(gec6818_dev_init);
module_exit(gec6818_dev_exit);

MODULE_AUTHOR("LI XIAOFAN");		        //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证