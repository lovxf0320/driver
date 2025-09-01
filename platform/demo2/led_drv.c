/*
 * GEC6818开发板LED驱动程序
 * 功能：通过GPIO控制LED灯的开关状态
 * 使用平台设备驱动框架和混杂设备接口
 */

// ========== 头文件包含 ==========
#include <linux/init.h>		    // 提供__init和__exit宏定义
#include <linux/kernel.h>	    // 提供printk函数
#include <linux/module.h>	    // 提供module_init和module_exit宏
#include <linux/cdev.h>         // 字符设备相关
#include <linux/fs.h>           // 文件系统接口
#include <linux/uaccess.h>      // 用户空间数据访问函数
#include <linux/device.h>       // 设备模型相关
#include <linux/ioport.h>       // I/O端口和内存资源管理
#include <linux/io.h>           // I/O内存映射
#include <linux/gpio.h>         // GPIO通用接口
#include <mach/gpio.h>          // 平台特定的GPIO接口
#include <asm/mach-types.h>     // 机器类型定义
#include <asm/gpio.h>           // 架构特定的GPIO接口
#include <cfg_type.h>           // 配置类型定义
#include <linux/miscdevice.h>   // 混杂设备框架
#include <linux/platform_device.h> // 平台设备框架

// ========== 全局变量定义 ==========
static struct resource *res;   // 指向平台设备资源的指针
static int gpio_num[4];                      // 存储GPIO引脚号
static const char *gpio_name[4];	        // 存储GPIO引脚的名称

/*
 * LED设备打开函数
 * 当用户空间程序打开/dev/myled设备文件时被调用
 * @inode: 设备文件的inode结构体指针
 * @file: 设备文件的file结构体指针
 * @return: 成功返回0，失败返回负错误码
 */
static int gec6818_led_open(struct inode *inode, struct file *file)
{
	int i=0;

	for(i=0;i<4;i++){
		// 设置GPIO为输出模式，初始输出高电平（LED关闭状态）
		gpio_direction_output(gpio_num[i], 1);
	}

	printk("gec6818_led_open: 设备已打开\n");
	
	return 0;
}

/*
 * LED设备关闭函数
 * 当用户空间程序关闭设备文件时被调用
 * @inode: 设备文件的inode结构体指针
 * @file: 设备文件的file结构体指针
 * @return: 成功返回0
 */
static int gec6818_led_close(struct inode *inode, struct file *file)
{
	printk("gec6818_led_close: 设备已关闭\n");
	return 0;
}

/*
 * LED设备写入函数
 * 用户空间程序向设备文件写入数据时被调用，用于控制LED状态
 * 数据格式：2字节，第一字节为LED编号(7-10)，第二字节为状态(0关闭/1开启)
 * @file: 设备文件的file结构体指针
 * @buf: 用户空间数据缓冲区指针
 * @len: 要写入的数据长度
 * @off: 文件偏移量指针
 * @return: 成功返回实际写入的字节数，失败返回负错误码
 */
static ssize_t gec6818_led_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	int rt;                 // 存储copy_from_user的返回值
	char kbuf[2] = {0};     // 内核空间缓冲区，用于存储从用户空间复制的数据
	
	int i;

	// 验证数据长度合法性，防止缓冲区溢出
	if(len > sizeof(kbuf))
		return -EINVAL;     // 返回无效参数错误
	
	// 从用户空间复制数据到内核空间
	// copy_from_user返回未成功复制的字节数，0表示全部复制成功
	rt = copy_from_user(kbuf, buf, len);
	
	if(rt!=0){
		return -EINVAL;
	}

	// 检查LED编号是否在有效范围内(7-10)
	if(kbuf[0] < 7 || kbuf[0] > 10)
		return -EINVAL;     // LED编号无效，返回错误
	

	//确定是哪个LED灯
	for(i=7; i<11; i++)
	{
		if(kbuf[0]==i)
			break;
	}

	i=i-7;

	// 根据第二个字节的值控制LED状态
	// 注意：这里的逻辑是反向的（0点亮LED，1熄灭LED）
	// 这可能是因为硬件电路设计为低电平有效
	if(kbuf[1])
		gpio_set_value(gpio_num[i], 0);    // 用户传入非0值，设置GPIO为低电平（LED亮）
	else
		gpio_set_value(gpio_num[i], 1);    // 用户传入0值，设置GPIO为高电平（LED灭）
	
	// 计算实际成功写入的字节数
	len = len - rt;
	
	printk("gec6818_led_write: LED状态已更新\n");
	
	return len;    // 返回实际写入的字节数
}


// ========== 文件操作结构体 ==========
/*
 * 定义设备文件的操作函数集合
 * 将具体的操作函数与标准的文件操作接口关联起来
 */
static const struct file_operations gec6818_led_fops = {
	.open = gec6818_led_open,       // 打开操作
	.release = gec6818_led_close,   // 关闭操作（注意这里用release而不是close）
	.write = gec6818_led_write,     // 写入操作
	.owner = THIS_MODULE,           // 模块所有者，用于引用计数
};


// ========== 混杂设备结构体 ==========
/*
 * 混杂设备是Linux内核提供的一种简化的字符设备注册方法
 * 适用于不需要复杂设备号管理的简单字符设备
 */
static struct miscdevice gec6818_led_misc = {
	.minor = MISC_DYNAMIC_MINOR,    // 动态分配次设备号
	.name = "myled",                // 设备名称，将在/dev/目录下创建对应的设备文件
	.fops = &gec6818_led_fops,     	// 指向文件操作结构体
};


/*
 * 平台设备探测函数
 * 当内核检测到匹配的平台设备时自动调用此函数进行初始化
 * @pdev: 指向平台设备结构体的指针
 * @return: 成功返回0，失败返回负错误码
 */
static int __devinit gec6818_led_probe(struct platform_device *pdev)
{
	int rt;    // 存储各种函数的返回值
	int i;
	
	// 注册混杂设备到内核
	rt = misc_register(&gec6818_led_misc);
	if(rt < 0) {
		printk("misc_register fail: 混杂设备注册失败\n");
		return rt;
	}
	
	for(i=0;i<4;i++){
		// 从平台设备中获取GPIO资源信息
		// IORESOURCE_IO表示I/O资源类型，0表示获取第一个资源
		res = platform_get_resource(pdev, IORESOURCE_IO, i);
		if(res == NULL) {
			printk("platform_get_resource fail: 获取平台设备资源失败\n");
			rt = -ENOMEM;    // 内存不足错误
			goto platform_get_resource_fail;
		}
		
		// 从资源结构体中提取GPIO引脚号
		gpio_num[i] = res->start;
		
		// 获取GPIO引脚的名称
		gpio_name[i] = res->name;
	}

	for(i=0;i<4;i++){
		gpio_free(gpio_num[i]);

		// 申请GPIO引脚的使用权
		rt = gpio_request(gpio_num[i], gpio_name[i]);
		if(rt < 0) {
			printk("gpio_request fail: GPIO引脚申请失败\n");

			goto gpio_request_fail;
		}

		printk("gec6818_led_probe: 驱动探测成功，GPIO%d已申请\n", gpio_num[i]);
	}
	
	return 0;
	
// 错误处理标签：使用goto语句实现统一的错误清理
gpio_request_fail:
	for(i=0;i<4;i++){
		gpio_free(gpio_num[i]);
	}

platform_get_resource_fail:
	// 如果初始化过程中出现错误，需要清理已经分配的资源
	misc_deregister(&gec6818_led_misc);
	
	return rt;
}

/*
 * 平台设备移除函数
 * 当平台设备被移除或驱动被卸载时调用，负责清理资源
 * @pdev: 指向平台设备结构体的指针
 * @return: 成功返回0
 */
static int __devexit gec6818_led_remove(struct platform_device *pdev)
{
	int i=0;
	for(i=0;i<4;i++){
		// 释放GPIO引脚
		gpio_free(gpio_num[i]);
	}

	// 注销混杂设备
	misc_deregister(&gec6818_led_misc);
	
	printk("gec6818_led_remove: 驱动已移除，资源已释放\n");
	return 0;
}

// ========== 平台驱动结构体 ==========
/*
 * 平台驱动是Linux设备驱动模型中的一种，适用于
 * 片上系统(SoC)中集成的外设，如GPIO、UART、SPI等
 */
static struct platform_driver gec6818_led_driver = {
	.probe = gec6818_led_probe,                    // 设备探测函数
	.remove = __devexit_p(gec6818_led_remove),    // 设备移除函数
	.driver = {
		.owner = THIS_MODULE,                      // 模块所有者
		.name = "gec6818_led",                     // 驱动名称，必须与平台设备名称匹配
	},
};

/*
 * 驱动模块初始化函数
 * 当使用insmod加载模块时被调用
 * @return: 成功返回0，失败返回负错误码
 */
static int __init gec6818_drv_init(void)
{
	// 注册平台驱动到内核
	int rt = platform_driver_register(&gec6818_led_driver);
	
	printk("gec6818_drv_init: LED驱动模块初始化%s\n", 
	       rt == 0 ? "成功" : "失败");
	
	return rt;
}

/*
 * 驱动模块退出函数
 * 当使用rmmod卸载模块时被调用
 */
static void __exit gec6818_drv_exit(void)
{
	// 注销平台驱动
	platform_driver_unregister(&gec6818_led_driver);
	
	printk("gec6818_drv_exit: LED驱动模块已卸载\n");
}

// ========== 模块信息和注册宏 ==========
module_init(gec6818_drv_init);                      // 指定模块初始化函数
module_exit(gec6818_drv_exit);                      // 指定模块退出函数

MODULE_AUTHOR("stephenwen88@163.com");              // 模块作者信息
MODULE_DESCRIPTION("GEC6818开发板LED GPIO驱动程序"); // 模块功能描述
MODULE_LICENSE("GPL");                              // 模块许可证类型
