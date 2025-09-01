#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <mach/platform.h>	//PB_PIO_IRQ

struct gpio_irq_t{
	unsigned int 	gpio;
	unsigned int 	irq_n;
	irq_handler_t 	fun;
	unsigned long 	irq_f;
	const char* 	name;
	void* 			dev;	// 传给中断函数的参数（通常是父结构体指针）
};

struct key_irq_t{
#define KEY_AMOUNT 4

	struct gpio_irq_t gi[KEY_AMOUNT];
	wait_queue_head_t wq;
	int cond;				// 条件变量，非0表示有中断发生
	unsigned char val;		// 每个位代表一个按键的状态
};

struct key_irq_t keys_irq={
	.gi[0]={(PAD_GPIO_A+28),PB_PIO_IRQ(PAD_GPIO_A+28),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpioa_28_irq",&keys_irq},
	.gi[1]={(PAD_GPIO_B+30),PB_PIO_IRQ(PAD_GPIO_B+30),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_30_irq",&keys_irq},	
	.gi[2]={(PAD_GPIO_B+31),PB_PIO_IRQ(PAD_GPIO_B+31),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_31_irq",&keys_irq},	
	.gi[3]={(PAD_GPIO_B+9 ),PB_PIO_IRQ(PAD_GPIO_B+9 ),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_9_irq" ,&keys_irq},
	
	.cond=0,
};
/*
为什么 .dev 要写 &keys_irq
 答案：这是为了中断回调时能访问整个按键系统的状态！
.dev 是用来传给 request_irq() 的最后一个参数的，会在中断触发时原样传给中断处理函数中的 dev_id 参数。

.dev = &keys_irq 是为了让中断服务函数能在 dev_id 中拿到整个按键管理结构体，从而访问所有按键、等待队列、条件变量。
*/

static irqreturn_t mykey_irq_handler(int irq,void *dev_id){
	int i;

	struct key_irq_t* ki=(struct key_irq_t*)dev_id;
	struct gpio_irq_t *gi=ki->gi;

	for(i=0;i<KEY_AMOUNT;i++){
		if(irq==gi->irq_n){
			ki->val|=gpio_get_valuer(gi->gpio)?0:1<<i;
		}
		gi++;
	}
	wake_up(&ki->wq);
	ki->cond=1;

	return IRQ_HANDLED;
}

int mykey_open (struct inode * inode, struct file * file){
	printk(KERN_INFO"mykey_open\n");

	return 0;
}

int mykey_release (struct inode *inode, struct file *file){
	printk(KERN_INFO"mykey_release\n");

	return 0;
}

ssize_t mykey_read(struct file *file, char __user *buf, size_t len, loff_t *loff){
	int rt;

	if(buf==NULL){
		return -EFAULT;
	}

	if(len>sizeof(key_val)){
		len=sizeof(key_val);
	}

	wait_event_interruptible(keys_irq.wq,keys_irq.cond);
	keys_irq.cond=0;

	rt=copy_to_user(buf,&keys_irq.val,sizeof(keys_irq.val));
	keys_irq.val=0;

	len=len-rt;
	return len;
}

static struct file_operation mykey_fops={
	.owern=		THIS_MODULE,
	.open=		mykey_open,
	.release=	mykey_release,
	.read=		mykey_read,
};

static struct miscdevice mykey_misc={
	.minor=	MISC_DYNAMIC_MINOR,
	.name=	"MY_KEY",
	.fops=	&mykey_fops,
};

static int __init mykey_init(void){

	int rt;
	int i;

	struct gpio_irq_t* gi=keys_irq.gi;

	for(i=0;i<KEY_AMOUNT;i++){
		rt=request_irq(	gi->irq_n,
						gi->fun,
						gi->irq_f,
						gi->name,
						gi->dev);
		if(rt<0){
			printk(KERN_ERR"request_irq %s fail\n",gi->name);

			goto err_request_irq;
		}
		gi++;
	}

	rt=misc_register(&mykey_misc);
	if(rt<0){
		printk(KERN_ERR"misc_register fail\n");

		goto err_misc_register;	
	}

	init_waitqueue_head(&keys_irq.wq);

	printk(KERN_INFO"mykey_init\n");
	
	return 0;

err_misc_register:
	gi=keys_irq.gi;
	while(i--){
		free_irq(gi->irq_n,gi->dev);
		gi++;
	}

err_request_irq:
	return rt;
}

static void __exit mykey_exit(void){
	struct gpio_irq_t* gi=keys_irq.gi;

	int i=KEY_AMOUNT;

	misc_deregister(&mykey_misc);

	while(i--){
		free_irq(gi->irq_n,gi->dev);
		gi++;
	}
	printk(KERN_INFO"mykey_exit\n");
}


module_init(mykey_init);
module_exit(mykey_exit);

MODULE_AUTHOR("LI XIAOFAN");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");