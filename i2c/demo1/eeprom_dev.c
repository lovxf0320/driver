#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include "linux/err.h"
#include "linux/regmap.h"
#include "linux/slab.h"

static struct i2c_board_info eeprom_info = {
	I2C_BOARD_INFO("gec6818_eeprom", 0x50)
};

static struct i2c_client *eeprom_client;

static int __init gec6818_eeprom_dev_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(0); // 0号适配器，实际根据平台调整
	if (!i2c_adap)
		return -ENODEV;

	eeprom_client = i2c_new_device(i2c_adap, &eeprom_info);
	i2c_put_adapter(i2c_adap);

	if (!eeprom_client)
		return -ENODEV;

	return 0;
}

static void __exit gec6818_eeprom_dev_exit(void)
{
	if (eeprom_client)
		i2c_unregister_device(eeprom_client);
}

module_init(gec6818_eeprom_dev_init);
module_exit(gec6818_eeprom_dev_exit);

MODULE_AUTHOR("LI XIAOFAN");			//作者信息
MODULE_DESCRIPTION("gec6818 eeprom driver");		//模块功能说明
MODULE_LICENSE("GPL");							//许可证：驱动遵循GPL协议