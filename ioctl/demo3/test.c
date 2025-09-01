#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LED_BUF_MAGIC        'B'
#define LED_STRUCT_MAGIC     'S'
//_IOWR(type, number, data_type)
/*
| 字段        | 说明                                                    |
| ----------- | ------------------------------------------------------- |
| `type`      | 一个字符，标识设备类型（这里是 `'L'` 表示 led）         |
| `number`    | 命令编号（每个命令唯一）                                |
| `data_type` | 传输的数据结构类型（会自动计算大小）                    |
| `_IOWR`     | 表示这是 **双向传输**，用户空间与内核空间都要访问数据   |
#define CMD_LED_ON             _IOW(LED_MAGIC,0,unsigned long)
#define CMD_LED_OFF            _IOW(LED_MAGIC,1,unsigned long)
| 字段  | 含义                        |
| ----- | --------------------------- |
| `'L'` | 类型标识，表示 LED 驱动类   |
| `3`   | 命令编号，第 4 个命令       |
| `_IO` | 表示无参数传输              |
#define CMD_LED_ALL_ON         _IO(LED_MAGIC,2)
#define CMD_LED_ALL_OFF        _IO(LED_MAGIC,3)
*/
#define CMD_BUF_W       _IOW(LED_BUF_MAGIC,0,int[4])
#define CMD_BUF_R       _IOW(LED_BUF_MAGIC,1,int[4])

#define CMD_STRUCT_W    _IOW(LED_STRUCT_MAGIC,2,struct led_cfg)
#define CMD_STRUCT_R    _IOW(LED_STRUCT_MAGIC,3,struct led_cfg)

struct led_cfg {
    int index;
    int state;
};

void test_array_ioctl(int fd){
	int write_data[4]={1,0,1,0};
	int read_data[4]={0};
	
	if(ioctl(fd,CMD_BUF_W,write_data)<0){
		perror("CMD_BUF_W error");
	}else{
		printf("CMD_BUF_W success\n");
	}
	
	if (ioctl(fd, CMD_BUF_R, read_data) < 0) {
        perror("CMD_BUF_R error");
    } else {
        printf("CMD_BUF_R result: %d %d %d %d\n",
               read_data[0], read_data[1], read_data[2], read_data[3]);
    }
}

void test_struct_ioctl(int fd){
	struct led_cfg cfg;
	
	cfg.index = 0;
	cfg.state = 1;
	
	if (ioctl(fd, CMD_STRUCT_W, &cfg) < 0) {
        perror("CMD_STRUCT_W error");
    } else {
        printf("CMD_STRUCT_W success\n");
    }

    cfg.index = 0; // 读取 D7 的状态
    if (ioctl(fd, CMD_STRUCT_R, &cfg) < 0) {
        perror("CMD_STRUCT_R error");
    } else {
        printf("CMD_STRUCT_R result: LED[%d] = %d\n", cfg.index, cfg.state);
    }
}

int main(int argc,char **argv){	
	int fd_led;
	fd_led=open("/dev/myled",O_RDWR);
	if(fd_led<0){
		perror("open /dev/myled/");
		return  -1;
	}
		
	test_array_ioctl(fd_led);
	test_struct_ioctl(fd_led);
	
	close(fd_led);

	return 0;
	
}
