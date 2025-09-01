#include <linux/init.h>         // 用于模块初始化和退出宏
#include <linux/module.h>       // 提供模块相关宏，如 module_init() 和 module_exit()
#include <mach/devices.h>       // 与硬件平台相关的设备定义
#include <linux/interrupt.h>    // 中断处理相关函数
#include <linux/gpio.h>         // GPIO 操作函数
#include <linux/input.h>        // 输入设备管理，如按键、触摸屏等

static struct input_dev* button_dev;
/*
input_dev：这是一个结构体，代表了一个输入设备。
每个输入设备（如按键、触摸屏、鼠标等）都有一个 input_dev 结构体，
它描述了设备的属性，比如支持的事件类型、按键类型等。
*/
static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	
	/* 向内核汇报按键状态 */
	input_report_key(button_dev,KEY_ENTER,!gpio_get_value(PAD_GPIO_A));
	/* 
	button_dev：指向输入设备的指针。
	KEY_ENTER：表示我们报告的是“回车”键的事件，你可以根据硬件来选择对应的键值。
	!gpio_get_value(PAD_GPIO_A+28)：读取 GPIO 引脚的电平状态，低电平（0）表示按下，
	反转后报告为按下（1）或松开（0）。 
	*/
	
	/* 汇报结束 */
	input_sync(button_dev);
	
	return IRQ_HANDLED;
}

static int __init mykey_init(void){
	int rt=0;
	rt=request_irq(
			gpio_to_irq(PAD_GPIO_A+28),	// 把 GPIO 编号转为中断号
			mykey_irq_handler,			// 中断服务函数
			IRQF_TRIGGER_FALLING,		// 下降沿触发（触发类型）
			"gpio_28_irq",				// /proc/interrupts 中显示的名字	
			NULL						// 不传递设备结构体
	);
	if(rt<0){
		printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
		goto err_request_irq;
	}
/*	| 宏常量                  | 意义                     | 场景                    |
	| ----------------------- | ------------------------ | ----------------------- |
	| `IRQF_TRIGGER_RISING`   | 上升沿（低 → 高）        | 松开按键，信号恢复      |
	| `IRQF_TRIGGER_FALLING`  | 下降沿（高 → 低）        | 按下按键，信号拉低      |
	| `IRQF_TRIGGER_HIGH`     | 高电平持续时触发         | 很少用，部分 SoC 有限制 |
	| `IRQF_TRIGGER_LOW`      | 低电平持续时触发         | 某些传感器输出          |
	| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | 双边沿触发，按下和抬起都响应 |*/


	/* 为输入设备分配内存空间
	使用 input_allocate_device() 函数分配一个 input_dev 结构体，
	这个结构体会描述我们的输入设备。	*/
	button_dev=input_allocate_device();
	if(!button_dev){
		printk(KERN_ERR "Not enough memory\n");
		rt = -ENOMEM;
		goto err_input_allocate_device;
	}


	/* 设置输入设备的事件类型、编码（支持哪些按键） */
	set_bit(EV_KEY,button_dev->evbit);

	/* 增加支持的按键 */
	set_bit(KEY_ENTER,button_dev->keybit);
	set_bit(KEY_UP,button_dev->keybit);
	set_bit(KEY_DOWN,button_dev->keybit);	
	set_bit(KEY_LEFT,button_dev->keybit);	
	set_bit(KEY_RIGHT,button_dev->keybit);	
	set_bit(KEY_Z,button_dev->keybit);
	set_bit(KEY_X,button_dev->keybit);	
	
	
	button_dev->name="mykey_input";
	button_dev->id.bustype=0x0000;
	button_dev->id.vendor =0x1688;	
	button_dev->id.product=0x6666;		
	button_dev->id.version=0x1001;	

	/* 注册输入设备 */
    rt = input_register_device(button_dev);
    if (rt) {
        printk(KERN_ERR "Failed to register device\n");
        goto err_input_register_device;
    }

	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_input_register_device:
	input_free_device(button_dev);
	
err_input_allocate_device:
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	
err_request_irq:
	return rt;
}

static void __exit mykey_exit(void){
	/* 注销输入设备 */
    input_unregister_device(button_dev);

    /* 释放输入设备分配的内存 */
    input_free_device(button_dev);

    /* 释放中断 */
    free_irq(gpio_to_irq(PAD_GPIO_A+28), NULL);

	printk(KERN_INFO"mykey_exit\n");
}


module_init(mykey_init);//驱动的入口，使用insmod命令加载该驱动
module_exit(mykey_exit);//驱动的出口，使用rmmod命令卸载该驱动

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");