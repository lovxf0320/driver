#include <stdio.h>
#include <sys/stat.h>       //文件权限说明（open权限）
#include <sys/types.h>      //系统数据类型（文件描述符）
#include <unistd.h>         //系统调用（write、read）
#include <fcntl.h>          //文件控制（open函数）
#include <string.h>    
#include <errno.h>          //错误（perror）
#include <stdlib.h>         //通用操作
#include <sys/mman.h>       //新增mmap相关的头文件
int main(){

    int lcdfd = open("/dev/fb0",O_RDWR);
    if(lcdfd < 0){
        perror("open");
        return -1;
    }

    FILE* file = fopen("../pic.bmp","r");
    if(file == NULL){
        perror("fopen");
        return -1;
    }

    fseek(file,54,SEEK_SET);

    //这段代码通过mmap将lcd映射到虚拟地址空间，使程序可以像控制内存一样控制lcd
    unsigned int *p = mmap(NULL, 800 * 480 * 4, PROT_READ|PROT_WRITE, MAP_SHARED, lcdfd, 0);
    //mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset)
    //void* start：映射起始地址​
        //NULL是指系统自动分配内存
    //size_t length：映射区域长度
        //指的是字节数
    //int prot：内存保护标志​
        //PROT_READ：允许读取显存（如屏幕回读）。
        //PROT_WRITE：允许修改显存（写入像素数据）。
        //若设备不支持写入（如只读显存），此操作会失败并返回 EACCES 错误
    //int flags：映射类型与行为
        //对显存的修改会​​同步到硬件​​，其他进程映射同一显存时也能看到变化。
        //若需私有映射（如双缓冲机制），应使用 MAP_PRIVATE，但需手动管理数据同步。
    //int fd：文件描述符​
    //off_t offset：映射偏移量​
        //需要按页对其
    //返回值与使用
    //成功：指向显存映射区的指针，通过 p[i] 可直接修改第 i 个像素的ARGB值。
    //失败：返回 MAP_FAILED（即 (void*)-1），需检查 errno 定位错误。
    if(p==MAP_FAILED){
        perror("mmap");
        return -1;
    }
    
    unsigned char rgb[3]={0};
    //在内存中是bgr，但是在lcd中是rgb
    //bgr分别是rgb[2]、rgb[1]、rgb[0]
    unsigned int data=0;
    for(int i=0;i<800*480;i++){
        fread(rgb,1,3,file);
        data=(0XFF<<24)|(rgb[0]<<16)|(rgb[1]<<8)|rgb[2];
        p[i]=data;
    }

    close(lcdfd);
    fclose(file);

}