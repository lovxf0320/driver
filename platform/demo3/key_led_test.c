#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


static char d7_on [2]={7,1};
static char d7_off[2]={7,0};

static char d8_on [2]={8,1};
static char d8_off[2]={8,0};

static char d9_on [2]={9,1};
static char d9_off[2]={9,0};

static char d10_on [2]={10,1};
static char d10_off[2]={10,0};


static char *leds_on_tbl[4]={
    d7_on,
    d8_on,
    d9_on,
    d10_on
};

static char *leds_off_tbl[4]={
    d7_off,
    d8_off,
    d9_off,
    d10_off
};


int main(int argc,char **argv){
    int led_fd=-1;
    
    int key_fd=-1;
    int key_val=0;
    int len;

    int i=0;

    //打开myled设备
    led_fd=open("dev/myled",O_RDWR);
    if(led_fd<0){
        perror("open /dev/myled:");
		
		return led_fd;
    }

    //打开mykey设备
    key_fd=open("dev/mykey",O_RDWR);
    if(key_fd<0){
        perror("open /dev/mykey:");
		
		return key_fd;
    }

    while(1){
        for(i=0;i<4;i++){
            len=read(key_fd,&key_val,1);
            if(len!=0){
                printf("key read filed\r\n");
				continue;
            }

            if(key_val&(1<<i)){
                len=write(led_fd,leds_on_tbl,2);
                if(len!=2){
                    printf("led set on filed\r\n");
					break;
                }
            }else{
                len=write(led_fd,leds_off_tbl,2);
                if(len!=2){
                    printf("led set off filed\r\n");
                    break;
                }
            }
            
        }

        usleep(20*1000);
    }

    close(led_fd);
    close(key_fd);
    return 0;
}
