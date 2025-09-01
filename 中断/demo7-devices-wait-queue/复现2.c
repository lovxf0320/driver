#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <mach/platform.h>	//PB_PIO_IRQ

struct gpio_irq_t{
	unsigned int    gpio;
    unsigned int    irq_n;
	irq_handler_t   fun;
    unsigned long   irq_f;    //中断触发方式
    const char*     name;
    void*           dev;
};

#define KEY_AMOUNT 4

struct key_irq_t{
    struct gpio_irq_t gi[KEY_AMOUNT];
    wait_queue_head_t wq;
    int cond;               // 条件变量，1 表示有中断
    unsigned char val; 
}

irqreturn_t mykey_irq_handler(int irq,void* dev_id);

struct key_irq_t keys_irq{
    .gi[0]={(PAD_GPIO_A+28),PB_PIO_IRQ(PAD_GPIO_A+28),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpioa_28_irq",&keys_irq},
    .gi[1]={(PAD_GPIO_B+30),PB_PIO_IRQ(PAD_GPIO_B+30),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_30_irq",&keys_irq},
    .gi[2]={(PAD_GPIO_B+31),PB_PIO_IRQ(PAD_GPIO_B+31),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_31_irq",&keys_irq},
    .gi[3]={(PAD_GPIO_B+9),PB_PIO_IRQ(PAD_GPIO_B+9),mykey_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"gpiob_9_irq",&keys_irq},

    .cond=0;
}

irqreturn_t mykey_irq_handler(int irq,void* dev_id){
    int i;

    struct key_irq_t* ki=(struct key_irq_t*)dev_id;
    struct gpio_irq_t* gi=ki->gi;

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

int mykey_open(struct inode * inode, struct file * file){
    printk(KERN_INFO"mykey_open\n");

	return 0;
}

int mykey_release(struct inode * inode, struct file * file){
    printk(KERN_INFO"mykey_release\n");

	return 0;
}

//在 read() 函数中，*loff 表示读操作开始的位置，返回后内核会用新的 *loff 更新文件偏移。
ssize_t mykey_read(struct file* file,char __user* buf,size_t len,loff_t* loff){
    int rt;

    if(buf==NULL){
        return -EFAULT;
    }

    /*| 名称       | 类型     | 返回内容                     | 用于什么          |
    | -------- | ------ | ------------------------ | ------------- |
    | `sizeof` | 编译时运算符 | **类型或变量占用的字节数（包括 `\0`）** | 获取内存大小、结构体大小等 |
    | `strlen` | 运行时函数  | **字符串的长度（不包括 `\0`）**     | 获取字符串实际长度     |
    */
    if(len>sizeof(buf)){
        len=sizeof(buf);
    }

    wait_event_interruptible(keys_irq.wq,keys_irq.cond);
    keys_irq.cond=0;

    //&keys_irq.val重点关注
    rt=copy_to_user(buf,&keys_irq.val,sizeof(buf));
    keys_irq.val=0;

    len=len-rt;
    return len;
}

static struct file_operation mykey_fops={
    .owern=     THIS_MODULE,
    .open=      mykey_open,
    .release=   mykey_release,
    .read=      mykey_read,
};

static struct miscdevice mykey_misc={
    .minor= MISC_DYNAMIC_MINOR,
    .name=  "MY_KEY",
    .fops=  &mykey_fops,
};

static int __init mykey_init(void){
    int rt;
    int i;

    struct gpio_irq_t* gi=keys_irq.gi;

    for(i=0;i<KEY_AMOUNT;i++){
        rt=request_irq(
            gi->gpio,
            gi->irq_n,
            gi->fun,
            gi->irq_f,
            gi->name,
            gi->dev,
        );
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
    }

err_request_irq:
    return 0;
}

static void __exit mykey_exit(void){
    struct gpio_irq_t* gi=keys_irq.gi;

    int i=KEY_AMOUNT;

    misc_deregister(&mykey_misc);

    while(i--){
        free_irq(gi->irq_n,gi->dev);
    }

    printk(KERN_INFO"mykey_exit\n");
}

module_init(mykey_init);
module_exit(mykey_exit);

MODULE_AUTHOR("LI XIAOFAN");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");