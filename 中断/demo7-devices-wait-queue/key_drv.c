#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <mach/platform.h>	//PB_PIO_IRQ

struct gpio_irq_t
{
	unsigned int  gpio;	
	unsigned int  irq_n;
	irq_handler_t fun;
	unsigned long irq_f;
	const char 	 *name; 
	void         *dev;

};

struct key_irq_t
{
#define KEY_AMOUNT	4

	struct gpio_irq_t gi[KEY_AMOUNT];
	wait_queue_head_t wq;
	int cond;				// 条件变量，非0表示有中断发生
	unsigned char val;		// 每个位代表一个按键的状态
};

static irqreturn_t mykey_irq_handler(int irq, void *dev_id);

static struct key_irq_t keys_irq={
	.gi[0]={(PAD_GPIO_A+28),PB_PIO_IRQ(PAD_GPIO_A+28),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpioa_28_irq",&keys_irq},
	.gi[1]={(PAD_GPIO_B+30),PB_PIO_IRQ(PAD_GPIO_B+30),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_30_irq",&keys_irq},	
	.gi[2]={(PAD_GPIO_B+31),PB_PIO_IRQ(PAD_GPIO_B+31),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_31_irq",&keys_irq},	
	.gi[3]={(PAD_GPIO_B+9 ),PB_PIO_IRQ(PAD_GPIO_B+9 ),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_9_irq" ,&keys_irq},
	
	.cond=0,
	
};

static irqreturn_t mykey_irq_handler(int irq, void *dev_id)
{
	int i;

	struct key_irq_t *ki  = (struct key_irq_t *)dev_id;
	
	struct gpio_irq_t *gi = ki->gi;
	
	for(i=0; i<KEY_AMOUNT; i++)
	{
		if(irq == gi->irq_n)
			ki->val|=gpio_get_value(gi->gpio)?0:1<<i;
		
		gi++;		
	}
		
	//唤醒等待队列头
	wake_up(&ki->wq);
	ki->cond=1;

	return IRQ_HANDLED;

}

int mykey_open (struct inode * inode, struct file * file)
{
	printk(KERN_INFO"mykey_open\n");

	return 0;
}

int mykey_release (struct inode *inode, struct file *file)
{

	printk(KERN_INFO"mykey_release\n");

	return 0;


}


ssize_t mykey_read (struct file *file, char __user *buf, size_t len, loff_t *loff)
{
	int rt;
	unsigned char key_val;
	
	if(buf == NULL)
		return -EFAULT;
	
	if(len > sizeof key_val)
		len = sizeof key_val;

	//阻塞等待队列头，同时条件为真才能继续往下执行
	wait_event_interruptible(keys_irq.wq,keys_irq.cond);
	keys_irq.cond=0;
	
	rt = copy_to_user(buf,&keys_irq.val,sizeof keys_irq.val);
	keys_irq.val = 0;
	
	len = len - rt;
	
	return len;
}

static struct file_operations mykey_fops={
	.owner = THIS_MODULE,
	.open = mykey_open,
	.release = mykey_release,
	.read = mykey_read,
};

static struct miscdevice mykey_misc={
	.minor = MISC_DYNAMIC_MINOR,//动态分配次设备号
	.name  = "mykey",//设备文件名,/dev/mykey
	.fops  = &mykey_fops,//文件操作集
};


static int __init mykey_init(void)
{
	int rt;
	int i;
	
	struct gpio_irq_t *gi = keys_irq.gi;
	
	/*1.申请中断 */
	for(i=0; i<KEY_AMOUNT; i++)
	{
		
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
	
	/*2.混杂设备的注册 */
	rt = misc_register(&mykey_misc);
	if(rt < 0)
	{
		printk(KERN_ERR"misc_register fail\n");

		goto err_misc_register;			
		
	}	
	
	//初始化等待队列头
	init_waitqueue_head(&keys_irq.wq);
	
	printk(KERN_INFO"mykey_init\n");
	
	return 0;
	
err_misc_register:
	gi = keys_irq.gi;
	while(i--)
	{
		free_irq(gi->irq_n,gi->dev);
		gi++;
	}
		
	
err_request_irq:
	return rt;
}


static void __exit mykey_exit(void)
{
	int i=KEY_AMOUNT;
	
	struct gpio_irq_t *gi = keys_irq.gi;
	
	/* 注销混杂设备 */
	misc_deregister(&mykey_misc);	
	
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