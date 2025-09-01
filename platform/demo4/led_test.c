#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char **argv){
    int fd=open("/dev/myled",O_RDWR);
    if(fd<0){
        perror("open");
        return fd;
    }

    char led_d7_on[2]={7,0};
    char led_d7_off[2]={7,1};

    while(1){
        write(fd,led_d7_on,2);
        slepp(1);

        write(fd,led_d7_off,2);
        slepp(1);
    }

    close(fd);

    return 0;
}