#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>

static int __init myled_init(void){
	
	printk(KERN_INFO"HZ[1]=%d,jifffies=%lu\n",HZ,jiffies);
	
	ssleep(2);
	
	printk(KERN_INFO"HZ[2]=%d jiffies=%lu\n",HZ,jiffies);
	
	
	printk(KERN_INFO"jifffies/HZ=%lu\n",(jiffies-INITIAL_JIFFIES)/HZ);
	
	ssleep(2);
	
	printk(KERN_INFO"jifffies/HZ=%lu\n",(jiffies-INITIAL_JIFFIES)/HZ);
	
	
	printk(KERN_INFO"jiffies_to_msecs=%dms\n",jiffies_to_msecs(jiffies-INITIAL_JIFFIES));
	
	printk(KERN_INFO"msecs_to_jiffies=%lu\n",msecs_to_jiffies(1000));
	
	printk("myled_init\n");
	
	return 0;
}

static void __exit myled_exit(void){
	
	printk("myled_exit\n");
	
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("LIXIAOFAN");
MODULE_DESCRIPTION("this is a led driver");
MODULE_LICENSE("GPL");