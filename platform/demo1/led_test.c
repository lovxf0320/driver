#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

char d7_on[2]={7,1};
char d7_off[2]={7,0};


int main(int argc,char **argv){
    int fd=-1;

    int rt=0;

    int i;

    fd=open("/dev/myled",O_RDWR);

    if(fd<0){
        perror("open /dev/myled:");
		
		return fd;
    }

    while(1){
        write(fd,d7_on,2);
        usleep(500*1000);

        write(fd,d7_off,2);
        usleep(500*1000);
    }

    close(fd);

    return 0;
}
