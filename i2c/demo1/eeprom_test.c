#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd=0;
	
	int len;
	
	int i=0;
	
	char buf[8];
	
	//打开gec6818_eeprom设备
	fd=open("/dev/gec6818_eeprom",O_RDWR);
	if(fd<0){
		perror("open /dev/gec6818_eeprom:");
		
		return fd;
		
	}
	
	while(1){
		
		for(i=0;i<255;i++){
			
			//地址
			buf[0]=i;
			
			//数据
			buf[1]=i;
			
			len=write(fd,buf,2);
			
			if(len > 0 )
				printf("write ok\n");
			else
				printf("write fail\n");
			
			usleep(500*1000);
			
			len=read(fd,buf,1);
			
			if(len>0)
				printf("read data=%02X\n",buf[0]);
			else
				printf("read fail\n");
			
			usleep(500*1000);
			
		}
		
	}
	
	return 0;
}