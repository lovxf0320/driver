#include <stdio.h>
#include <sys/stat.h>      //文件权限说明（open权限）
#include <sys/types.h>     //系统数据类型（文件描述符）
#include <unistd.h>        //系统调用（write、read）
#include <fcntl.h>         //文件控制（open函数）
#include <string.h>    
#include <errno.h>         //错误（perror）
#include <stdlib.h>        //通用操作

int main(){
    //打开/dev/fb0，/dev/fb0是linux帧缓冲设备文件，直接操作这个文件，可以控制设备显存
    int lcdfd = open("/dev/fb0",O_RDWR);
    if(lcdfd < 0){
        perror("open");
        return -1;
    }

    //打开图片
    FILE* file=fopen("../pic.bmp","r");
    if(file==NULL){
        perror("fopen ");
        return -1;
    }
    //移动到像素位置
    fseek(file,54,SEEK_SET);

    unsigned char rgb[3]={0};
    //BMP存储顺序为BGR，而LCD可能需RGB，需转换顺序
    //RGB颜色分量（rgb[0]、rgb[1]、rgb[2]，每个分量占1字节）
    unsigned int data=0;
    for(int i=0;i<800*480;i++){
        fread(rgb,1,3,file);
        data=(0xFF<<24)|(rgb[0]<<16)|(rgb[1]<<8)|(rgb[0]);
        write(lcdfd,&data,4);
    }

    close(lcdfd);
    fclose(file);

}