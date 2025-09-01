#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h> // 放到最后

// 临时声明，防止隐式声明报错
int misc_unregister(struct miscdevice *misc);

static struct i2c_client* this_client; 


static int gec6818_eeprom_open(struct inode * inode, struct file *file){
	
	printk(KERN_INFO"gec6818_eeprom_open ok\n");
	
	return 0;
}


static int gec6818_eeprom_release(struct inode * inode, struct file *file){
	
	printk(KERN_INFO"gec6818_eeprom_release ok\n");
	
	return 0;
	
}


static ssize_t gec6818_eeprom_write(struct file * file, const char __user * buf, size_t len, loff_t * off){
	
	int rt;
	char wbuf[2]={0};
	
	unsigned char addr,data;
	
	if(len>sizeof(wbuf)){
		return -EINVAL;
	}
	
	
	rt=copy_from_user(wbuf,buf,len);
	if(rt!=0){
		return -EFAULT;
	}
	
	//写入的地址
	addr=wbuf[0];
	
	//写入的数据
	data=wbuf[1];
	
	rt=i2c_smbus_write_byte_data(this_client,addr,data);
	
	if(rt<0){
		return rt;
	}
	
	return 2;
}


static ssize_t gec6818_eeprom_read(struct file * file, char __user * buf, size_t len, loff_t * off){
	
	int rt;
	
	unsigned char addr,data;
		
	
	rt=copy_from_user(&addr,buf,1);
	if(rt!=0){
		return -EFAULT;
	}
	
	
	rt=i2c_smbus_read_byte_data(this_client,addr);
	
	if(rt<0){
		return rt;
	}
	
	data=rt&0xFF;
	
	rt=copy_to_user(buf,&data,1);
	if(rt != 0){
		return -EFAULT;
	}
	return 1;
	
}


static const struct file_operations gec6818_eeprom_fops={
	.owner=THIS_MODULE,
	.write=gec6818_eeprom_write,
	.read=gec6818_eeprom_read,
	.open=gec6818_eeprom_open,
	.release=gec6818_eeprom_release,
};

static struct miscdevice gec6818_eeprom_miscdev={
	.minor=MISC_DYNAMIC_MINOR,
	.name="gec6818_eeprom",
	.fops=&gec6818_eeprom_fops,
};

static int gec6818_eeprom_probe(struct i2c_client* client,const struct i2c_device_id* id){
	int rt;
	
	if(!i2c_check_functionality(client->adapter,I2C_FUNC_I2C)){
		printk(KERN_INFO"i2c_check_functionality_fail");
		
		return -ENODEV;
	}
	
	rt=misc_register(&gec6818_eeprom_miscdev);
	if(rt<0){
		printk(KERN_INFO"misc_register_fail");
		
		return rt;
	}
	
	this_client=client;
	
	printk(KERN_INFO"gec6818_eeprom_probe ok\n");
	
	return 0;
}

static int gec6818_eeprom_remove(struct i2c_client* client){
	
	misc_unregister(&gec6818_eeprom_miscdev);
	
	return 0;
}

static const struct i2c_device_id gec6818_eeprom_id[]={
	{"gec6818_eeprom", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c,gec6818_eeprom_id);


static struct i2c_driver gec6818_eeprom_driver={
	.driver={
		.name="gec6818_eeprom",
		.owner=THIS_MODULE,
	},
	.probe=gec6818_eeprom_probe,
	.remove=gec6818_eeprom_remove,
	.id_table=gec6818_eeprom_id,
};


//入口函数
static int __init gec6818_eeprom_drv_init(void){
	
	int rt;
	
	rt=i2c_add_driver(&gec6818_eeprom_driver);
	
	return rt;
	
}


//出口函数
static void __exit gec6818_eeprom_drv_exit(void){
	
	i2c_del_driver(&gec6818_eeprom_driver);

}


module_init(gec6818_eeprom_drv_init);
module_exit(gec6818_eeprom_drv_exit);


//模块描述
MODULE_AUTHOR("LI XIAOFAN");			//作者信息
MODULE_DESCRIPTION("gec6818 eeprom driver");	//模块功能说明
MODULE_LICENSE("GPL");							//许可证：驱动遵循GPL协议