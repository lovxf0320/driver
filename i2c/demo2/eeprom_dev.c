#include "linux/kernel.h"
#include "linux/module.h"
#include "linux/platform_device.h"
#include "linux/i2c.h"
#include "linux/err.h"
#include "linux/regmap.h"
#include "linux/slab.h"

static struct i2c_board_info gec6818_eeprom_info = { 
	//如果地址是0x50，会跟内核自带的24c04地址冲突，所以内核必须卸载24c04驱动才行；
	I2C_BOARD_INFO("gec6818_eeprom", 0x50),	
};

static struct i2c_client *gec6818_eeprom_client;

static int gec6818_eeprom_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	
	// 获取设备号为 2 的adpter ，也就是adapter->nr == 2
	i2c_adap = i2c_get_adapter(2);
	
	
	//直接使用 i2c_new_device 创建 client 自动注册到i2c_bus_type 中去
	//client->name == "gec6818_eeprom" 
	//client->addr = 0x50
	gec6818_eeprom_client = i2c_new_device(i2c_adap, &gec6818_eeprom_info);
	
	// 释放掉 adapter
	i2c_put_adapter(i2c_adap);
	
	return 0;
}

static void gec6818_eeprom_dev_exit(void)
{
	i2c_unregister_device(gec6818_eeprom_client);
}

module_init(gec6818_eeprom_dev_init);
module_exit(gec6818_eeprom_dev_exit);
//模块描述
MODULE_AUTHOR("stephenwen88@163.com");			//作者信息
MODULE_DESCRIPTION("gec6818 eeprom driver");		//模块功能说明
MODULE_LICENSE("GPL");							//许可证：驱动遵循GPL协议