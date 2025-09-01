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
/*结论：结构体数组通常需要 for 循环来：
逐个判断条件（比如：哪个按键按下？哪个设备状态为 0？）
逐个执行操作（比如：批量开灯、关灯、释放资源等）
“结构体数组 + 操作字段” → 基本都要写 for 来遍历。*/

#define LED_MAGIC				'L'
#define LED_BUF_MAGIC			'B'
#define LED_KEY_MAGIC			'K'
//_IOWR(type, number, data_type)
/*
| 字段        | 说明                                                    |
| ----------- | ------------------------------------------------------- |
| `type`      | 一个字符，标识设备类型（这里是 `'L'` 表示 led）         |
| `number`    | 命令编号（每个命令唯一）                                |
| `data_type` | 传输的数据结构类型（会自动计算大小）                    |
| `_IOWR`     | 表示这是 **双向传输**，用户空间与内核空间都要访问数据   |
*/
#define CMD_LED_ON             _IOW(LED_MAGIC,0,unsigned long)
#define CMD_LED_OFF            _IOW(LED_MAGIC,1,unsigned long)
/*
| 字段  | 含义                        |
| ----- | --------------------------- |
| `'L'` | 类型标识，表示 LED 驱动类   |
| `3`   | 命令编号，第 4 个命令       |
| `_IO` | 表示无参数传输              |

#define CMD_LED_ALL_ON         _IO(LED_MAGIC,2)
#define CMD_LED_ALL_OFF        _IO(LED_MAGIC,3)
这是一个 多字符字符常量，C语言中 'A' 是合法的字符（char），
但 'ABC' 是非法的；
'LED_MAGIC' 被当成“超长字符常量”，其类型是 int，
不是一个合法的 char 字面量。
*/
#define CMD_KEY_R			   _IOR(LED_KEY_MAGIC,4,unsigned long)
#define CMD_BUF_W			   _IOW(LED_BUF_MAGIC,5,unsigned long)
#define CMD_BUF_R			   _IOW(LED_BUF_MAGIC,6,unsigned long)

static struct gpio leds_gpios[] = {
	{ PAD_GPIO_E+13, GPIOF_OUT_INIT_HIGH, "D7 LED" }, /* default to OFF */
	{ PAD_GPIO_C+17, GPIOF_OUT_INIT_HIGH, "D8 LED" }, /* default to OFF */
	{ PAD_GPIO_C+8,  GPIOF_OUT_INIT_HIGH, "D9 LED" }, /* default to OFF */	
	{ PAD_GPIO_C+7,  GPIOF_OUT_INIT_HIGH, "D10 LED" }, /* default to OFF */	
};

static struct gpio key_gpios[]={
	//引脚编号，引脚输出模式-初始电平状态为高电平，自定义引脚的名字
	{PAD_GPIO_A+28,GPIOF_IN,"key_2"},
	{PAD_GPIO_B+30,GPIOF_IN,"key_3"},
	{PAD_GPIO_B+31,GPIOF_IN,"key_4"},
	{PAD_GPIO_B+9,GPIOF_IN,"key_6"},
};

int myled_open (struct inode * inode, struct file * file){
	printk(KERN_INFO"myled_open\n");
	return 0;
}

int myled_release (struct inode *inode, struct file *file){
	printk(KERN_INFO"myled_release\n");
	return 0;
}

/*
| 参数名 | 类型            | 作用说明                                                                |
| ------ | --------------- | ----------------------------------------------------------------------- |
| `file` | `struct file *` | 内核中的“打开文件”对象，记录设备状态（例如是否为 O\_NONBLOCK 模式）     |
| `cmd`  | `unsigned int`  | 命令码（由 `_IO`/`_IOW` 等宏构造），告诉驱动你要干什么，比如 LED\_ON    |
| `arg`  | `unsigned long` | 附带参数，可以是用户传过来的值、结构体地址等(用户空间地址或值；)                            |

long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
ioctl 函数的参数只是一个 通用壳子（形式），真正的含义（传什么数据、是值还是指针）取决于你怎么定义和使用命令宏。*/
long myled_unlocked_ioctl (struct file *file, unsigned int cmd, unsigned long args){
	int i;
	int rt=0;
	/*ioctl() 的第 3 个参数是 unsigned long args，即一个用户空间地址或值；
	如果命令是 _IOW/_IOR/_IOWR，这个 args 
	实际上是 一个指针（地址），指向用户传递的数据。*/
	void __user *argp=(void __user *)args;
	/*__user 是 Linux 内核的一个标记（宏），
	告诉编译器/静态分析器：这个指针是“用户空间的指针”；
	这样可以防止你误用它去直接访问内核空间，否则会产生安全隐患；
	如果你用了 copy_from_user() / copy_to_user() 函数，
	就必须保证传入的指针是用户空间的。*/
	unsigned int key_val=0;
	/*这个 value 是实际要传输的值，你把用户空间传来的数据 读入到它里面（write），
	或把它的值写出到用户空间（read）*/
	if(_IOC_TYPE(cmd)!=LED_MAGIC&&_IOC_TYPE(cmd)!=LED_BUF_MAGIC&&_IOC_TYPE(cmd)!=LED_KEY_MAGIC){
		return -ENOIOCTLCMD;
	}
	
	if(cmd==CMD_KEY_R){
		for(i=0,key_val=0;i<ARRAY_SIZE(key_gpios);i++){
			if(gpio_get_value(key_gpios[i].gpio)==0){
				key_val|=(1<<i);
			}
		}
		rt=copy_to_user(argp,&key_val,sizeof(key_val));
	}

	if (cmd == CMD_LED_ON) {
        if (args < ARRAY_SIZE(leds_gpios))
            gpio_set_value(leds_gpios[args].gpio, 0);
    } else if (cmd == CMD_LED_OFF) {
        if (args < ARRAY_SIZE(leds_gpios))
            gpio_set_value(leds_gpios[args].gpio, 1);
    }


	return rt;
}

static struct file_operations myled_fops={
	.owner = THIS_MODULE,
	.open = myled_open,
	.release = myled_release,
	.unlocked_ioctl = myled_unlocked_ioctl,
};

static struct miscdevice myled_misc={
	.minor=MISC_DYNAMIC_MINOR,
	.name="myled",
	.fops=&myled_fops,
};

static int __init myled_init(void)
{
	int rt;
	/*使用混杂设备就可以放弃定义struct cdev、申请设备号（register_chrdev_region\alloc_chrdev_region）
	、初始化cdev（cdev_init）、将cdev加入到内核（cdev_add）、
	自动创建设备号（class_create、device_create）*/
	
	/*1.混杂设备的注册 */
	rt = misc_register(&myled_misc);
	if(rt < 0)
	{
		printk(KERN_ERR"misc_register fail\n");

		goto err_misc_register;			
		
	}	
	
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
	rt = gpio_request_array(key_gpios,ARRAY_SIZE(key_gpios));
	if(rt < 0)
	{
		printk(KERN_ERR"key_gpios_request_array fail\n");

		goto err_key_gpios_request_array;			
		
	}
	printk(KERN_INFO"mykey_init\n");
	
	/*2.批量申请gpio引脚 */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	//因为内核已经申请了这些引脚，所以释放之后，才能调用gpio_request_array函数
	rt = gpio_request_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	if(rt < 0)
	{
		printk(KERN_ERR"leds_gpios_request_array fail\n");

		goto err_leds_gpios_request_array;			
		
	}

	printk(KERN_INFO"myled_init\n");
	return 0;

err_leds_gpios_request_array:
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
err_key_gpios_request_array:
	misc_deregister(&myled_misc);
err_misc_register:
	return rt;
}

static void __exit myled_exit(void)
{
	/* 释放gpio引脚 */
	gpio_free_array(leds_gpios,ARRAY_SIZE(leds_gpios));
	gpio_free_array(key_gpios,ARRAY_SIZE(key_gpios));
	/* 注销混杂设备 */
	misc_deregister(&myled_misc);
	printk(KERN_INFO"myled_exit\n");
}

module_init(myled_init);//驱动的入口，使用insmod命令加载该驱动
module_exit(myled_exit);//驱动的出口，使用rmmod命令卸载该驱动

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is led driver");
MODULE_LICENSE("GPL");
/*
| 方式         | 描述                        | 是否需要 `copy_from_user` / `copy_to_user`   |
| ------------ | --------------------------- | -------------------------------------------- |
| **值传递**   | `args` 是一个直接的数值     | ? 不需要拷贝，直接使用 `args` 就行          |        |
| **指针传递** | `args` 是用户空间的地址指针 | ? 需要使用 `copy_from_user` / `copy_to_user`|
*/