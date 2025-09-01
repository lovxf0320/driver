#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <cfg_type.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>


static int gpio_num[4];
static const char *gpio_name[4];

static int gec6818_key_open(struct inode * inode, struct file *file){
   int i=0;

	for(i=0;i<4;i++){
		gpio_direction_input(gpio_num[i]);
	}

	printk("gec6818_led_open: 设备已打开\n");
	
	return 0;
}

static int gec6818_key_release(struct inode * inode, struct file *file){
    printk("gec6818_key_release\n");
	
	return 0;
}


static ssize_t gec6818_key_write(struct file *file,const char __user *buf, size_t len, loff_t * offs){
    return 0;
}

static ssize_t gec6818_key_read(struct file *file, char __user *buf, size_t len, loff_t * offs){
    int rt;
    int i;
    char key_val=0;

    if(len>sizeof(key_val)){
        return -EINVAL;
    }

    for(i=0;i<4;i++){
        key_val|=gpio_get_value(gpio_num[i])?0:(1<<i);
    }

    //从内核空间拷贝到用户空间
    rt=copy_to_user(buf,&key_val,len);

    return rt;
}


static const struct file_operations gec6818_key_fops={
    .owner  =THIS_MODULE,
    .write  =gec6818_key_write,
    .read   =gec6818_key_read,
    .open   =gec6818_key_open,
    .release=gec6818_key_release,
};

static struct miscdevice gec6818_key_miscdev={
    .minor  =MISC_DYNAMIC_MINOR,
    .name   ="mykey",
    .fops   =&gec6818_key_fops,
};

static int __devinit gec6818_key_probe(struct platform_device *pdev){
    int rt;
    int i;

    struct resource *res;

    //混杂设备的注册
    rt=misc_register(&gec6818_key_miscdev);
    if(rt<0){
        printk("misc_register_fail\n");
        return rt;
    }

    for(i=0;i<4;i++){
        //获取平台设备传递过来的资源
        res=platform_get_resource(pdev,IORESOURCE_IO,i);
        if(res==NULL){
            printk("platfrom_get_resouce fail\n");

            rt=-ENOMEM;

            goto platfrom_get_resouce_fail;
        }

        //获取gpio引脚
        gpio_num[i]=res->start;

        //获取gpio引脚名字
        gpio_name[i]=res->name;
    }

    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);

        //申请gpio
        rt=gpio_request(gpio_num[i],gpio_name[i]);

        if(rt<0){
            printk("gpio_request fail\n");
			
			goto gpio_request_fail;	
        }
    }

    return 0;

gpio_request_fail:
    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);
    }

platfrom_get_resouce_fail:
    misc_deregister(&gec6818_key_miscdev);

    return rt;
}


//出口函数
static int __devexit gec6818_key_remove(struct platform_device *pdev){
    int i;

    for(i=0;i<4;i++){
        gpio_free(gpio_num[i]);
    }

    return 0;
}


//定义一个平台驱动的结构体
static struct platform_driver gec6818_key_driver={
    .probe  =gec6818_key_probe,
    .remove =__devexit_p(gec6818_key_remove),
    .driver ={
        .owner  =THIS_MODULE,
        .name   ="gec6818_key",
    },
};


static int __init gec6818_drv_init(void){
    int rt=platform_driver_register(&gec6818_key_driver);

    printk("gec6818_drv_init\n");

    return rt;
}

static void __exit gec6818_drv_exit(void){
    platform_driver_unregister(&gec6818_key_driver);

    printk("gec6818_drv_exit\n");
}


module_init(gec6818_drv_init)
module_exit(gec6818_drv_exit)

//模块描述
MODULE_AUTHOR("stephenwen88@163.com");			//作者信息
MODULE_DESCRIPTION("gec6818 key driver");		//模块功能说明
MODULE_LICENSE("GPL");							//许可证：驱动遵循GPL协议