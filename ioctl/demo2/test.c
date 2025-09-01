#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LED_MAGIC        'L'
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
*/
#define CMD_LED_ALL_ON         _IO(LED_MAGIC,2)
#define CMD_LED_ALL_OFF        _IO(LED_MAGIC,3)

#define CMD_BUF_W _IOW(LED_MAGIC,4,unsigned long)
#define CMD_BUF_R _IOW(LED_MAGIC,5,unsigned long)
int main(int argc,char **argv)
{
	int i;
	int fd_led;
	unsigned long args=100;
	unsigned long kargs=100;
	
	fd_led=open("/dev/myled",O_RDWR);
	if(fd_led<0){
		perror("open /dev/myled/");
		return  -1;
	}

	ioctl(fd_led,CMD_BUF_W,&args);
	ioctl(fd_led,CMD_BUF_R,&kargs);
	printf("kargs=%d\n",kargs);
	
	while(1){
		for(i=7;i<=10;i++){
			ioctl(fd_led,CMD_LED_ON,i);
			sleep(1);
			ioctl(fd_led,CMD_LED_OFF,i);
			sleep(1);
		}
		for(i=7;i<=10;i++){
			ioctl(fd_led,CMD_LED_ALL_ON);
			sleep(1);
			ioctl(fd_led,CMD_LED_ALL_OFF);
			sleep(1);
		}
	}
	
	close(fd_led);

	return 0;
	
}
