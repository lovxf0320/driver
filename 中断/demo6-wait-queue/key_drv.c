#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>

//它允许内核中的任务在条件不满足时睡眠（阻塞），而不是一直忙等，占用 CPU
//常用于 read() 阻塞，直到硬件有事件发生（比如 按键被按下、传感器有新数据）

//设置一个等待的条件，条件满足，进程就继续向下执行；
//条件不满足，进程就阻塞在等待队列上。
//当条件满足后，中断会唤醒等待队列中的进程，进程再继续向下执行。

static wait_queue_head_t key_wq;	// 等待队列头

static int key_flag=0;				// 定义一个等待条件，条件变量，1=有中断发生

static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	printk(KERN_INFO"mykey_irq_handler\n");
	
	if(irq==gpio_to_irq(PAD_GPIO_A+28)){
		wake_up(&key_wq);
		//唤醒等待队列的进程
		//wait_queue_head_t *q ，定义并初始化好的等待队列
		key_flag=1;		// 设置条件为“已触发”
	}
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

ssize_t mykey_read(struct file *file,char __user *buf,size_t len,loff_t* loff){
	int rt;
	unsigned char key_val;
	
	if(buf==NULL){
		return -EFAULT;
	}
	
	if(len>sizeof(key_val)){
		len=sizeof(key_val);
	}
	
	wait_event_interruptible(key_wq,key_flag);	// 阻塞等待中断
	key_flag=0;									// 重置
	
	key_val |= gpio_get_value(PAD_GPIO_A+28)?0:1;
	
	rt=copy_to_user(buf,&key_val,sizeof(key_val));
	
	len=len-rt;
	return len;	
}

static struct file_operations mykey_fops={
	.owner = THIS_MODULE,
	.open  = mykey_open,
	.release = mykey_release,
	.read = mykey_read,	
};

//混杂设备是一种简化字符设备注册的方式，自动分配主设备号。
static struct miscdevice mykey_misc={
	.minor=MISC_DYNAMIC_MINOR,
	.name="mykey",
	.fops=&mykey_fops,
};

static int __init mykey_init(void){
	int rt=0;
	rt=request_irq(
			gpio_to_irq(PAD_GPIO_A+28),	// 把 GPIO 编号转为中断号
			mykey_irq_handler,			// 中断服务函数
			IRQF_TRIGGER_FALLING,		// 下降沿触发（触发类型）
			"gpio_28_irq",				// /proc/interrupts 中显示的名字	
			NULL						//参数传递给中断服务函数(要求一个指针，是一个地址)
	);
	if(rt<0){
		printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
		goto err_request_irq;
	}

	rt=misc_register(&mykey_misc);
	if(rt<0){
		printk(KERN_ERR"misc_register fail\n");
		goto err_misc_register;
	}
	
	init_waitqueue_head(&key_wq);
	
	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_misc_register:
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);

err_request_irq:
	return rt;
}

static void __exit mykey_exit(void){
	
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	
	misc_deregister(&mykey_misc);
	
	printk(KERN_INFO"mykey_exit\n");
}

module_init(mykey_init);
module_exit(mykey_exit);

MODULE_AUTHOR("LI XIAONFAN");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");


/*
目的：
让驱动中的某些操作（如 read）在没有事件发生时不要瞎忙，
等事件（如按键中断）发生后再继续执行。*/