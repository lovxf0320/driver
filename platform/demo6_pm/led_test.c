#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>

#define CMD_LED_ON       _IOW('L',0,unsigned int)
#define CMD_LED_OFF      _IOW('L',1,unsigned int)

int main(int argc,char **argv){
    int i;

    int fd=open("/dev/myled",O_RDWR);
    if(fd<0){
        perror("open");
        return fd;
    }

    while(1){
        for(i=7;i<10;i++){
            ioctl(fd,CMD_LED_ON,i);
            sleep(1);

            ioctl(fd,CMD_LED_OFF,i);
            sleep(1);
        }
    }
}