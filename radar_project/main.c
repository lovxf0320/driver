#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

int main(void){
	int fd_sr04=0;
	int fd_beep=0;
	int fd_key=0;
	int fd_led=0;
	char led_buf[2];
	
	uint32_t distance = 0;
	char val_key=0;
	char last_key=0;
	int val_beep=1;
	int rt=0;
	
	fd_sr04=open("/dev/mysr04",O_RDWR);
	if(fd_sr04<0){
		perror("fail open mysr04\n");
		return -1;
	}
	
	fd_beep=open("/dev/mybeep",O_WRONLY);
	if(fd_beep<0){
		perror("fail open mybeep\n");
		return -1;
	}
	
	fd_key=open("/dev/mykey",O_RDWR);
	if(fd_key<0){
		perror("fail open fd_key\n");
		return -1;
	}
	
	fd_led=open("/dev/myled",O_RDWR);
	if(fd_led<0){
		perror("fail open myled\n");
		return -1;
	}
	
	while(1){
		rt=read(fd_key,&val_key,sizeof(val_key));
		if(rt<0){
			perror("fail read mykey\n");
			close(fd_key);
			return -1;
		}
		
		if((val_key&0x01)&&!(last_key&0x01)){
			printf("K2 被按下，开始工作...\n");
			break;
		}
		last_key=val_key;
		usleep(100000);
	}
	
	while(1){
		rt=read(fd_key,&val_key,sizeof(val_key));
		if(rt<0){
			perror("fail read mykey\n");
			break;
		}
		if((val_key&0x02)&&!(last_key&0x02)){
			printf("K6 被按下，停止工作...\n");
			break;
		}
		last_key=val_key;
		
		rt=read(fd_sr04,&distance,sizeof(distance));
		if(rt<0){
			perror("fail read mysr04\n");
			close(fd_sr04);
			return -1;
		}
		printf("distance=%d\n",distance);
		
		//ssize_t write(int fd, const void *buf, size_t count);		
		if(distance<=1){
			val_beep=1;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(100000);
			val_beep=0;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(100000);		
		}else if(distance<=2){
			val_beep=1;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(300000);
			val_beep=0;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(300000);	
		}else if(distance<=3){
			val_beep=1;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(500000);
			val_beep=0;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(500000);	
		}else if(distance<=4){
			val_beep=1;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(900000);
			val_beep=0;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
			usleep(900000);	
		}else{
			val_beep=0;
			rt=write(fd_beep,&val_beep,sizeof(val_beep));
			if(rt<0){
				perror("fail write mybeep\n");
				close(fd_beep);
				return -1;
			}
		}
		
		led_buf[0]=1;
		if(distance<=1){
			led_buf[1]=4;
		}else if(distance<=2){
			led_buf[1]=3;
		}else if(distance<=3){
			led_buf[1]=2;
		}else if(distance<=4){
			led_buf[1]=1;
		}else{
			led_buf[1]=0;
		}
		
		write(fd_led,led_buf,sizeof(led_buf));
		
		usleep(100000);
	}
	
	close(fd_sr04);
	close(fd_beep);
	close(fd_key);
	close(fd_led);
	return 0;
}