#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static char d7_on[2]={7,1};
static char d7_off[2]={7,0};

static char d8_on[2]={8,1};
static char d8_off[2]={8,0};

static char d9_on[2]={9,1};
static char d9_off[2]={9,0};

static char d10_on[2]={10,1};
static char d10_off[2]={10,0};

static char *leds_on_tb1[4]={
	d7_on,
	d8_on,
	d9_on,
	d10_on,
};

static char *leds_off_tb1[4]={
	d7_off,
	d8_off,
	d9_off,
	d10_off,
};

int main(int argc, char **argv){
	int fd=0;
	int len;
	int i=0;

	//打开myled设备
	fd = open("/dev/myled", O_RDWR); // 修正路径
	if(fd < 0){
		perror("open /dev/myled:");
		return fd;
	}

	while(1){
		for(i = 0; i < 4; i++){
			len=write(fd,leds_on_tb1[i],2);

			sleep(1);

			len=read(fd,leds_off_tb1[i],2);

			sleep(1);
		}
	}
	close(fd);

	return 0;
}