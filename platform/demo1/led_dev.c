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

// `struct resource` - 硬件资源描述
static struct resource gec6818_led_resource[] = 
{
    [0] = {  // 第一个资源（数组下标0）
        .name   = "gpioe_13",           // 资源名称，便于识别和调试
        .flags  = IORESOURCE_IO,        // 资源类型：IO资源（GPIO属于IO资源）
        .start  = PAD_GPIO_E + 13,      // 资源起始地址：GPIOE组的第13号引脚
        .end    = PAD_GPIO_E + 13,      // 资源结束地址：同起始地址（单个引脚）
    },

    /*
     * 如果有多个资源，可以继续添加：
     * [1] = { ... },  // 第二个资源
     * [2] = { ... },  // 第三个资源
     */

};


/* ================== 设备释放函数 ================== */
/*
 * 设备释放回调函数
 * 当设备被注销时，内核会调用这个函数进行清理工作
 * 这是设备模型要求的，每个device都必须有release函数
 */
static void gec6818_led_dev_release(struct device *dev){

    printk("led dev exit\n");
    // 这里可以添加具体的资源清理代码
    // 例如：释放内存、关闭时钟等
}


/* ================== Platform Device定义 ================== */
/* `struct platform_device` - 平台设备
 * Platform设备结构体 - 这是整个模块的核心
 * 它向内核描述了一个名为"gec6818_led"的平台设备
 */
static struct platform_device gec6818_led_device={
    /*
     * 设备名称 - 这是Platform机制的匹配关键
     * Platform Bus会根据这个名称寻找对应的Platform Driver
     * 必须与驱动程序中的driver.name完全一致
     */
    .name           =   "gec6818_led",                      //设备的名字，该名字必须跟平台驱动一样
    
    /*
     * 资源数量 - 告诉内核这个设备有多少个硬件资源
     * ARRAY_SIZE宏会自动计算数组元素个数
     */
    .num_resources  =   ARRAY_SIZE(gec6818_led_resource),   //资源的数目，当前的资源数目为1
    
    /*
     * 资源指针 - 指向上面定义的资源数组
     * Platform Driver可以通过platform_get_resource()获取这些资源
     */
    .resource       =   gec6818_led_resource,               //资源的来源
    
    /*
     * 设备ID - 用于区分同名设备的不同实例
     * -1表示不使用ID区分，适用于系统中只有一个该类型设备的情况
     * 如果系统中有多个同类设备，可以设置为0, 1, 2...来区分
     */
    .id				=   -1,								    //资源识别的编号，若不使用，默认初始化为-1
    
    /*
     * 内嵌的device结构 - 这是Linux设备模型的要求
     * 每个platform_device都包含一个标准的device结构
     */
    .dev			= 
	{
		.release    = gec6818_led_dev_release,
        /*
         * 还可以添加其他device属性：
         * .init_name = "led_device",     // 设备初始名称
         * .platform_data = &some_data,  // 平台相关数据
         * .dma_mask = &some_mask,       // DMA掩码
         */
	},
};


static int __init gec6818_dev_init(void){
	/*
     * 向内核注册Platform设备
     * 注册成功后，Platform Bus会开始寻找匹配的Driver
     * 返回值：成功返回0，失败返回负数错误码
     */
    int rt = platform_device_register(&gec6818_led_device);
	
    if(rt==0){
        printk("Platform device registered successfully\n");
    }else{
        printk("Platform device registration failed: %d\n", rt);
    }

	return rt;
}

static void __exit gec6818_dev_exit(void){
    /*
     * 从内核中注销Platform设备
     * 这会触发对应Driver的remove函数被调用
     */
	platform_device_unregister(&gec6818_led_device);
	
	printk("gec6818_dev_exit\n");
}


module_init(gec6818_dev_init);

//驱动程序的出口
module_exit(gec6818_dev_exit);

MODULE_AUTHOR("lixiaofan");		            //作者信息
MODULE_DESCRIPTION("gec6818 led driver");	//模块描述
MODULE_LICENSE("GPL");						//许可证