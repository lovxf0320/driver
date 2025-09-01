#include <linux/init.h>           // __init/__exit 宏定义
#include <linux/module.h>         // module_init/module_exit，模块宏
#include <mach/devices.h>         // 提供 PAD_GPIO_* 定义（平台相关）
#include <linux/interrupt.h>      // 中断注册、释放、处理函数
#include <linux/gpio.h>           // GPIO 与 IRQ 相关函数（如 gpio_to_irq）
#include <mach/platform.h>        // 提供 PB_PIO_IRQ 宏定义

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

unsigned int data1=100;
unsigned int data2=200;
unsigned int data3=300;
unsigned int data4=400;

struct gpio_irq_t{
	unsigned int  irq_n;      // 中断号
	irq_handler_t fun;        // 中断处理函数（函数指针）
	unsigned long irq_f;      // 中断触发方式（如下降沿）
	const char 	  *name;      // 中断名，出现在 /proc/interrupts
	void          *dev;       // 可选的私有参数（传给处理函数）
};
static irqreturn_t mykey_irq_handler(int irq, void *dev_id);

static struct gpio_irq_t keys_irq[]={
	{PB_PIO_IRQ(PAD_GPIO_A+28),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_28_irq",&data1},
	{PB_PIO_IRQ(PAD_GPIO_B+30),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_30_irq",&data2},
	{PB_PIO_IRQ(PAD_GPIO_B+31),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_31_irq",&data3},
	{PB_PIO_IRQ(PAD_GPIO_B+9),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_9_irq",&data4},	
};

static irqreturn_t mykey_irq_handler(int irq, void *dev_id){  //irq 是当前中断号
	int i;
	for(i=0;i<ARRAY_SIZE(keys_irq);i++){
		if(keys_irq[i].irq_n==irq){
			printk(KERN_INFO "%s trigger\n %d", keys_irq[i].name,keys_irq[i].dev);
		}
	}
	printk(KERN_INFO"mykey_irq_handler");
	return IRQ_HANDLED;
}
/*irq_handler_t是一个typedef：typedef irqreturn_t (*irq_handler_t)(int, void*)
irqreturn_t：返回类型，表示中断是否处理成功。
int irq：触发该中断的中断号（会自动传入）。
void *dev_id：你在注册中断时传的参数，这里为 NULL。
printk()：内核打印函数，KERN_INFO 代表“普通信息级别”。
IRQ_HANDLED：表示当前中断已被成功处理。
*/

static int __init mykey_init(void){
	int rt=0;
	int i;
	for(i=0;i<ARRAY_SIZE(keys_irq);i++){
		rt=request_irq(
			keys_irq[i].irq_n,
			keys_irq[i].fun,
			keys_irq[i].irq_f,
			keys_irq[i].name,
			keys_irq[i].dev		
		);
		if(rt<0){
			printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
			goto err_request_irq;
		}
	}
/*	
| 宏常量                  | 意义                 | 场景                    |
| ----------------------- | -------------------- | ----------------------- |
| `IRQF_TRIGGER_RISING`   | 上升沿（低 → 高）    | 松开按键，信号恢复      |
| `IRQF_TRIGGER_FALLING`  | 下降沿（高 → 低）    | 按下按键，信号拉低      |
| `IRQF_TRIGGER_HIGH`     | 高电平持续时触发     | 很少用，部分 SoC 有限制 |
| `IRQF_TRIGGER_LOW`      | 低电平持续时触发     | 某些传感器输出          |
| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | 双边沿触发，按下和抬起都响应 |*/
	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_request_irq:
	while(i--){
		free_irq(keys_irq[i].irq_n,keys_irq[i].dev);
	}
	return rt;
}


static void __exit mykey_exit(void){
	int i=ARRAY_SIZE(keys_irq);
	while(i--){
		free_irq(keys_irq[i].irq_n,keys_irq[i].dev);
	}
	printk(KERN_INFO"mykey_exit\n");
}


module_init(mykey_init);//驱动的入口，使用insmod命令加载该驱动
module_exit(mykey_exit);//驱动的出口，使用rmmod命令卸载该驱动

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");