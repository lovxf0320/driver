#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <mach/platform.h>	//PB_PIO_IRQ
/*
struct work_struct {
     atomic_long_t data;
     struct list_head entry;
     work_func_t func;       // 要执行的函数
     #ifdef CONFIG_LOCKDEP
     struct lockdep_map lockdep_map;
     #endif
};*/

struct gpio_irq_t{
	unsigned int  gpio;	
	unsigned int  irq_n;
	irq_handler_t fun;
	unsigned long irq_f;
	const char 	 *name; 
	void         *dev;
	unsigned int code;
	
};

struct key_irq_t{
#define KEY_AMOUNT	4

	struct gpio_irq_t gi[KEY_AMOUNT];
	//wait_queue_head_t wq;
	//int cond;					// 条件变量，非0表示有中断发生
	//unsigned char val;			// 每个位代表一个按键的状态
	unsigned long irq_r;		//响应的中断
	struct work_struct mywork;	//工作队列
	struct input_dev* input;	//输入设备
};

static irqreturn_t mykey_irq_handler(int irq, void *dev_id);

static struct key_irq_t keys_irq={
	.gi[0]={(PAD_GPIO_A+28),
		PB_PIO_IRQ(PAD_GPIO_A+28),
		mykey_irq_handler,
		IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
		"gpioa_28_irq",
		&keys_irq,
		KEY_ENTER},
	.gi[1]={(PAD_GPIO_B+30),
		PB_PIO_IRQ(PAD_GPIO_B+30),
		mykey_irq_handler,
		IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
		"gpiob_30_irq",
		&keys_irq,
		KEY_RIGHT},	
	.gi[2]={(PAD_GPIO_B+31),
		PB_PIO_IRQ(PAD_GPIO_B+31),
		mykey_irq_handler,
		IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
		"gpiob_31_irq",
		&keys_irq,
		KEY_Z},	
	.gi[3]={(PAD_GPIO_B+9 ),
		PB_PIO_IRQ(PAD_GPIO_B+9 ),
		mykey_irq_handler,
		IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
		"gpiob_9_irq",
		&keys_irq,
		KEY_X},
	
	//.cond=0,
};

void mywork_handler(struct work_struct* work){
	int i;
	/*从嵌套结构体的成员指针 work，反推出外层结构体 struct key_irq_t 的地址。
	| 宏参数         | 说明                  |
	| ----------- | ------------------- |
	| `work`      | 是结构体成员 `mywork` 的地址 |
	| `key_irq_t` | 是外层结构体类型            |
	| `mywork`    | 是成员变量名              |*/
	struct key_irq_t *ki=container_of(work,struct key_irq_t,mywork);

	struct gpio_irq_t *gi=ki->gi;

	mdelay(20);
	
	for(i=0; i<KEY_AMOUNT; i++){

		if(ki->irq_r == gi->irq_n){
			//ki->val|=gpio_get_value(gi->gpio)?0:1<<i;
			
			input_report_key(ki->input,gi->code,!gpio_get_value(gi->gpio));
			//code来自于unsigned int code;
			//他经过初始化，变成了KEY_ENTER、KEY_RIGHT..
			//不再需要位图了。
			
			/* 汇报结束 */
			input_sync(ki->input);
		}

		gi++;
	}

	printk(KERN_INFO"mytasklet_handler exit\n");
}


static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	struct key_irq_t *ki  = (struct key_irq_t *)dev_id;
	
	ki->irq_r=irq;

	printk(KERN_INFO"mykey_irq_handler\n");	

	//登记工作队列
	schedule_work(&ki->mywork);
	
	return IRQ_HANDLED;

}

static int __init mykey_init(void){
	int rt;
	int i;
	
	struct gpio_irq_t *gi = keys_irq.gi;
	struct input_dev  *input;
	
	/*1.申请中断 */
	for(i=0; i<KEY_AMOUNT; i++){
		
		rt = request_irq(gi->irq_n, //中断号
						 gi->fun,  	//中断服务函数
						 gi->irq_f,	//触发方式
						 gi->name,	//申请中断成功后，在/proc/interrupts登记的名字
						 gi->dev);	//参数传递给中断服务函数	
					
		if(rt < 0)
		{
			printk(KERN_ERR"request_irq %s fail\n",gi->name);
			
			goto err_request_irq;
		}

		gi++;
		
	}
	
	/*2.为输入设备分配内存空间 */
	input=input_allocate_device();
	if(!input){
		printk(KERN_ERR"Not enough memory\n");
		rt=	-ENOMEM;
		goto err_input_allocate_device;
	}
	
	keys_irq.input = input;
	
	/* 设置输入设备的事件类型、编码（支持哪些按键） */
	set_bit(EV_KEY,input->evbit);
	
	/* 增加支持的按键 */
	set_bit(KEY_ENTER,	input->keybit);
	set_bit(KEY_SPACE,	input->keybit);	
	set_bit(KEY_UP,		input->keybit);
	set_bit(KEY_DOWN,	input->keybit);	
	set_bit(KEY_LEFT,	input->keybit);	
	set_bit(KEY_RIGHT,	input->keybit);	
	set_bit(KEY_Z,		input->keybit);
	set_bit(KEY_X,		input->keybit);	
	set_bit(KEY_A,		input->keybit);
	set_bit(KEY_S,		input->keybit);		
	set_bit(KEY_Q,		input->keybit);	
	set_bit(KEY_M,		input->keybit);	
	input->name="mykey_input";
	input->id.bustype=0x0000;
	input->id.vendor =0x1688;	
	input->id.product=0x6666;		
	input->id.version=0x1001;	
	
	rt=input_register_devie(input);
	if(rt){
		printk(KERN_ERR "Failed to register device\n");
		goto err_input_register_device;
	}
	
	//初始化等待队列头
	//init_waitqueue_head(&keys_irq.wq);
	
	INIT_WORK(&keys_irq.work,mywork_handler);
	
	printk(KERN_INFO"mykey_init\n");
	
	return 0;
	
err_input_register_device:
	input_free_device(input);
	
err_input_allocate_device:
	gi = keys_irq.gi;
	while(i--)
	{
		free_irq(gi->irq_n,gi->dev);
		gi++;
	}
		
err_request_irq:
	return rt;
}


static void __exit mykey_exit(void){
	int i=KEY_AMOUNT;
	
	struct gpio_irq_t *gi = keys_irq.gi;
	
	/* 注销工作队列 */
	cancel_work_sync(&keys_irq.work);	
	
	/* 注销输入设备 */
	input_unregister_device(keys_irq.input);

	/* 释放设备的内存 */
	input_free_device(keys_irq.input);
	
	//释放中断
	while(i--)
	{
		free_irq(gi->irq_n,gi->dev);
		gi++;
	}
	
	printk(KERN_INFO"mykey_exit\n");	
}


module_init(mykey_init);//驱动的入口，使用insmod命令加载该驱动
module_exit(mykey_exit);//驱动的出口，使用rmmod命令卸载该驱动

MODULE_AUTHOR("LI XIAOFAN");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");
