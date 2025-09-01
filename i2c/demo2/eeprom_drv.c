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
#include <linux/i2c.h>


static struct i2c_client *this_client;

static int  gec6818_eeprom_open (struct inode * inode, struct file *file)
{


	printk("gec6818_eeprom_open \n");
	
	return 0;
}

static int  gec6818_eeprom_release (struct inode * inode, struct file *file)
{
	printk("gec6818_eeprom_release \n");
	
	return 0;
}

static ssize_t gec6818_eeprom_write (struct file * file, const char __user * buf, size_t len, loff_t * off)
{
	int rt;
	
	char wbuf[16]={0};
	
	char addr = 0;

	struct i2c_msg msgs[2];
	
	if(len > sizeof wbuf)
		len = sizeof wbuf;
	
	rt = copy_from_user(wbuf,buf,len);
	
	if(rt != 0)
		return -EFAULT;
	
	
	msgs[0].addr	= this_client->addr;//从机地址
	msgs[0].flags	= 0;				//写操作
	msgs[0].len		= 1;				//写长度
	msgs[0].buf		= &addr;			//写入到eeprom的起始地址，当前设置为0

	msgs[1].addr	= this_client->addr;//从机地址
	msgs[1].flags	= 0|I2C_M_NOSTART;	//再次执行写，不带起始信号
	msgs[1].len		= len;				//写长度
	msgs[1].buf		= &wbuf[0];			//写内容

	//发送2组消息
	rt = i2c_transfer(this_client->adapter, msgs, 2);	

	if(rt < 0)
		return rt;
		
	return len;
}

static ssize_t gec6818_eeprom_read (struct file *file, char __user *buf, size_t len, loff_t * offs)
{
	int rt=0;
	
	char rbuf[16];
	
	char addr = 0;
	
	struct i2c_msg msgs[2];
	
	if(len > sizeof rbuf)
		len = sizeof rbuf;
	
	rt = copy_from_user(rbuf,buf,len);
	
	if(rt != 0)
		return -EFAULT;
	
	
	msgs[0].addr	= this_client->addr;	//从机地址
	msgs[0].flags	= 0;					//写操作
	msgs[0].len		= 1;					//写长度
	msgs[0].buf		= &addr;				//读取eeprom的起始地址，当前设置为0

	msgs[1].addr	= this_client->addr;	//从机地址
	msgs[1].flags	= I2C_M_RD;				//读操作
	msgs[1].len		= len;					//读长度
	msgs[1].buf		= &rbuf[0];				//保存要读取的数据
	

	//发送2组消息
	rt = i2c_transfer(this_client->adapter, msgs, 2);	

	if(rt < 0)
		return rt;
	
	rt = copy_to_user(buf,rbuf,len);
	
	if(rt != 0)
		return -EFAULT;	
	
	len = len -rt;
	
	return len;
}

static const struct file_operations gec6818_eeprom_fops = {
 	.owner 		= THIS_MODULE,
	.write 		= gec6818_eeprom_write,
	.open 		= gec6818_eeprom_open,
	.release 	= gec6818_eeprom_release,
	.read 		= gec6818_eeprom_read,
};


static struct miscdevice gec6818_eeprom_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,	//MISC_DYNAMIC_MINOR,动态分配次设备号
	.name		= "gec6818_eeprom",		//设备名称,/dev/gec6818_eeprom	
	.fops		= &gec6818_eeprom_fops,	//文件操作集
};

static int gec6818_eeprom_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rt;
	//检查是否支持I2C功能
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk("i2c_check_functionality fail\n");
		
		return -ENODEV;
	}	
	
	//混杂设备的注册
	rt = misc_register(&gec6818_eeprom_miscdev);
	
	if (rt) 
	{
		printk("misc_register fail\n");
		return rt;
	}
	
	//保存client指针，里面有各种设备信息
	this_client = client;
	
	return 0;
}

static int __devexit gec6818_eeprom_remove(struct i2c_client *client)
{
	misc_deregister(&gec6818_eeprom_miscdev);

	return 0;
}


//Linux I2C设备驱动中，是通过i2c device id名字进行i2c device和i2c driver匹配的。
static const struct i2c_device_id gec6818_eeprom_id[] = {
	{ "gec6818_eeprom", 0 },	//参数1必须跟进行i2c device和i2c driver匹配的
								//参数2可以作为私有数据传递给gec6818_eeprom_probe的参数2
	{ }							//标识结束
};

//添加到i2c总线设备列表，告诉内核当前设备支持热插拔功能
MODULE_DEVICE_TABLE(i2c, gec6818_eeprom_id);


static struct i2c_driver gec6818_eeprom_driver = {
	.driver = 
	{
		.name = "gec6818_eeprom",						//用于匹配设备
		.owner = THIS_MODULE,
	},
	.probe 		= gec6818_eeprom_probe,					//驱动初始化
	.remove 	= __devexit_p(gec6818_eeprom_remove),	//驱动删除
	.id_table 	= gec6818_eeprom_id,					//添加支持的设备表					
														
};


//入口函数
static int __init gec6818_eeprom_init(void)
{
	//添加I2C设备
	return i2c_add_driver(&gec6818_eeprom_driver);
}


//出口函数
static void __exit gec6818_eeprom_exit(void)
{
	//删除I2C设备
	i2c_del_driver(&gec6818_eeprom_driver);
}

module_init(gec6818_eeprom_init);
module_exit(gec6818_eeprom_exit)


//模块描述
MODULE_AUTHOR("stephenwen88@163.com");			//作者信息
MODULE_DESCRIPTION("gec6818 eeprom driver");	//模块功能说明
MODULE_LICENSE("GPL");							//许可证：驱动遵循GPL协议