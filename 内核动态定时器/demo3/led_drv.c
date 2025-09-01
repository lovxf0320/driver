#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/platform.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

/*
struct gpio {
	unsigned	gpio;      //引脚编号
	unsigned long	flags; //引脚模式
	const char	*label;    //自动逸引脚名称
};

struct timer_list {
    struct hlist_node        entry;         // 哈希链表节点
    unsigned long            expires;       // 定时器超时的 jiffies 时间（相对当前时间）
    unsigned long            data;          // 传递给回调函数的数据
    void (*function)(unsigned long);        // 定时器回调函数
    unsigned int            flags;         // 定时器标志
    unsigned int            start_pid;     // 定时器启动的进程 PID
};

*/
unsigned long led_flash_time=HZ*0.5;
EXPORT_SYMBOL_GPL(led_flash_time);

static struct timer_list mytimer;

static struct gpio led_gpios[]={
	{PAD_GPIO_E+13,GPIOF_OUT_INIT_HIGH,"D7 LED"},
};

void mytiner_func(unsigned long data){
	//这里的b被设置为static是很巧妙的，他的生命周期会延续到程序的整个运行期间。
	static int b=0;
	
	mod_timer(&mytimer,jiffies+msecs_to_jiffies(led_flash_time));
	
	b=!b;
	
	gpio_set_value(PAD_GPIO_E+13,b);	
	
}

static int __init myled_init(void){
	int rt;
	
	gpio_free_array(led_gpios,ARRAY_SIZE(led_gpios));
	rt=gpio_request_array(led_gpios,ARRAY_SIZE(led_gpios));
	if(rt<0){
		printk(KERN_ERR"gpio_request_arrayfaile");
		
		goto err_gpio_request_array;
	}
	
	mytimer.function=mytiner_func;	//超时处理函数
	mytimer.expires=jiffies+HZ;		//超时时间为1秒
	mytimer.data=10;				//传递参数
	
	//初始化动态定时器
	init_timer(&mytimer);
	
	//将动态定时器加入内核，并启动动态定时器
	add_timer(&mytimer);
	
err_gpio_request_array:
	return rt;
}

static void __exit myled_exit(void){
	
	del_timer(&mytimer);
	
	gpio_free_array(led_gpios,ARRAY_SIZE(led_gpios));
	
	printk("myled_exit\n");
	
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("LIXIAOFAN");
MODULE_DESCRIPTION("this is a led driver");
MODULE_LICENSE("GPL");