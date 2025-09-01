#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

/*typedef irqreturn_t (*irq_handler_t)(int irq, void *dev_id);
这段定义在内核头文件中：include/linux/interrupt.h
| 参数       | 类型       | 说明                                             |
| -------- | -------- | ---------------------------------------------- |
| `irq`    | `int`    | 表示触发中断的中断号（由内核传入）                              |
| `dev_id` | `void *` | 通常是传给 `request_irq()` 的最后一个参数，用于传递设备数据或私有数据指针。 |

该类型是内核定义的枚举类型，用于表示中断处理结果，定义如下：
typedef enum {
    IRQ_NONE,     // 没有处理这个中断
    IRQ_HANDLED,  // 成功处理了这个中断
    IRQ_WAKE_THREAD  // 唤醒线程化中断（不常用）
} irqreturn_t;
*/

static unsigned int data = 100;

static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	unsigned int d = *(unsigned int*)dev_id;
	
	printk(KERN_INFO"mykey_irq_handler,%d",d);
	return IRQ_HANDLED;
}


static int __init mykey_init(void)
{
	int rt=0;
	rt=request_irq(
			gpio_to_irq(PAD_GPIO_A+28),	// 把 GPIO 编号转为中断号
			mykey_irq_handler,			// 中断服务函数
			IRQF_TRIGGER_FALLING,		// 下降沿触发（触发类型）
			"gpio_28_irq",				// /proc/interrupts 中显示的名字	
			&data						//参数传递给中断服务函数(要求一个指针，是一个地址)
	);
/*	| 宏常量                  | 意义                     | 场景                    |
	| ----------------------- | ------------------------ | ----------------------- |
	| `IRQF_TRIGGER_RISING`   | 上升沿（低 → 高）        | 松开按键，信号恢复      |
	| `IRQF_TRIGGER_FALLING`  | 下降沿（高 → 低）        | 按下按键，信号拉低      |
	| `IRQF_TRIGGER_HIGH`     | 高电平持续时触发         | 很少用，部分 SoC 有限制 |
	| `IRQF_TRIGGER_LOW`      | 低电平持续时触发         | 某些传感器输出          |
	| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | 双边沿触发，按下和抬起都响应 |*/

	if(rt<0){
		printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
		goto err_request_irq;
	}

	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_request_irq:
	return rt;
}


static void __exit mykey_exit(void){
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	printk(KERN_INFO"mykey_exit\n");
}
a

module_init(mykey_init);//驱动的入口，使用insmod命令加载该驱动
module_exit(mykey_exit);//驱动的出口，使用rmmod命令卸载该驱动

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");