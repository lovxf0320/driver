#include <linux/init.h>
#include <linux/module.h>

extern int __init led_init(void);
extern int __init key_init(void);
extern int __init beep_init(void);
extern int __init sr04_init(void);

extern void __exit led_exit(void);
extern void __exit key_exit(void);
extern void __exit beep_exit(void);
extern void __exit sr04_exit(void);

static int __init rader_init(void){
	int ret=0;
	ret=led_init();
	if(ret<0){
		return ret;
	}
	ret=key_init();
	if(ret<0){
		return ret;
	}
	ret=beep_init();
	if(ret<0){
		return ret;
	}
	ret=sr04_init();
	if(ret<0){
		return ret;
	}
	printk(KERN_INFO"rader module loaded\n");
	return ret;
}

static void __exit rader_exit(void){
	led_exit();
	key_exit();
	beep_exit();
	sr04_exit();
	printk(KERN_INFO"rader module unloaded\n");
}

module_init(rader_init);
module_exit(rader_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("Combined radar module");



