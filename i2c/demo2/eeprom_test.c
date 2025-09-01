#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd=-1;
	
	int len;
	
	int i=0;
	
	char wbuf[16]={0};
	
	char rbuf[16]={0};
	
	//打开gec6818_eeprom设备
	fd = open("/dev/gec6818_eeprom",O_RDWR);
	
	if(fd < 0)
	{
		perror("open /dev/gec6818_eeprom:");
		return fd;
	}

	memset(wbuf,0x88,sizeof wbuf);
	
	printf("write data all is 0x88\n");
	
	//向0地址连续写入16字节的数据
	write(fd,wbuf,sizeof wbuf);
	
	//写入和读取操作之间必须有一定的延时
	sleep(1);
	
	//从0地址连续读取16字节的数据
	read(fd,rbuf,sizeof rbuf);
	
	printf("read data is:");
	for(i=0; i<16; i++)
	{
		printf("%02X ",rbuf[i]);
	}
	printf("\r\n");

	
	close(fd);
	
	return 0;
}