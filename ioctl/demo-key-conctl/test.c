#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LED_MAGIC				'L'
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
/*
| 字段  | 含义                        |
| ----- | --------------------------- |
| `'L'` | 类型标识，表示 LED 驱动类   |
| `3`   | 命令编号，第 4 个命令       |
| `_IO` | 表示无参数传输              |
*/
#define CMD_LED_ON      _IOW(LED_MAGIC, 0, unsigned long)
#define CMD_LED_OFF     _IOW(LED_MAGIC, 1, unsigned long)
#define CMD_KEY_R			   _IOR(LED_KEY_MAGIC,4,unsigned long)
int main(int argc,char **argv)
{
	int i;
	int fd_led;
	unsigned int key_val = 0;
	
	fd_led=open("/dev/myled",O_RDWR);
	if(fd_led<0){
		perror("open /dev/myled/");
		return  -1;
	}
	
	while(1){
		if (ioctl(fd_led, CMD_KEY_R, &key_val) < 0) {
            perror("ioctl CMD_KEY_R");
            break;
        }
		for(i=0;i<4;i++){
			if(key_val&(1<<i)){
				printf("Key %d pressed, turning ON led %d\n", i+1, i);
                ioctl(fd_led, CMD_LED_ON, i);  
			}else {
                ioctl(fd_led, CMD_LED_OFF, i);
            }
		}
		usleep(200000);
	}
	
	
	close(fd_led);

	return 0;
	
}
