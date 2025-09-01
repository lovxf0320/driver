#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>

static struct timer_list mytimer;

void mytimer_fun(unsigned long data){
	
	printk(KERN_INFO"mytimer_func,data=%lu\n",data);
	
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(500));
	//这一行代码是用来修改一个定时器的时间
	
}

static int __init myled_init(void){
	
	mytimer.function=mytimer_fun;
	mytimer.expires=jiffies+HZ;		//指定超时时间
	mytimer.data=10;
	
	//初始化动态定时器
	init_timer(&mytimer);
	
	//将动态定时器加入到内核，并启动动态定时器
	add_timer($mytimer);
	
	return 0;
}

static void __exit myled_exit(void){
	
	del_timer(&mytimer);
	
	printk("myled_exit\n");
	
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("LIXIAOFAN");
MODULE_DESCRIPTION("this is a led driver");
MODULE_LICENSE("GPL");