#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lcddevice.h"
#include <sys/mman.h>
#include <linux/fb.h>  //此头文件定义了 framebuffer 的​​数据结构​​和 ​​IOCTL 命令​​，连接内核与用户空间的核心
//应用程序————>framebuffer(字符缓冲设备)(位于应用缓冲区)————>LCD(位于物理层)
//framebuffer(字符缓冲设备) 是内核驱动申请的一片​​连续物理内存​​，作为显存的映射区。
//应用程序通过读写此区域，即可控制屏幕显示内容，无需直接操作硬件寄存器。
#include <sys/ioctl.h>
//提供 ​​ioctl() 系统调用的声明​​，用于向字符设备（如帧缓冲设备）发送​​非标准控制命令​​

static struct LcdDevice* instanceLcd = NULL;
//单例模式实现（外部函数无法修改）
//其他文件比如main.c需要通过lcd_creat()等函数间接的访问，无法直接访问instanceLcd

void clear(unsigned int color){
    for(int j = 0; j<instanceLcd->h; j++){
        for(int i = 0; i<instanceLcd->w; i++){
            instanceLcd->mpty[j*instanceLcd->w + i] = color;
        }
    }
}

//通过​​内存映射​​直接操作显存
void draw_bmp(int x,int y,struct Bmp* bmp){
    int dw = (x + bmp->w > instanceLcd->w)?instanceLcd->w : bmp->w;
    int dh = (x + bmp->h > instanceLcd->h)?instanceLcd->h : bmp->h;

    unsigned int* p = instanceLcd->mpty + instanceLcd->w*y + x;
    unsigned char* q = bmp->data;

    for(int j = 0;j < dh;j++){
        for(int i = 0;i <dw;i++){
            memcmp(p + i,q + i*bmp->pixelbyte,bmp->pixelbyte);
        }

        p += instanceLcd->w;
        q += bmp->w*bmp->pixelbyte;
        //为何不写成p += instanceLcd->w*instanceLcd->piexlbyte;
        //这是因为lcd存储与bmp存在本质差异
        //lcd是按照像素寻址的（像素：unsigned int），一个像素大小是4字节
        //而bmp是按照字节流寻址的（unsigned char），一个色块（RBG）是三个字节
        //他们的单位一定不是对等的，就好比三斤苹果VS五个苹果
        //“对不上”是合理的
        //分辨率是屏幕的大小，位深负责颜色的的大小
    }
}

struct LcdDevice* lcd_creat(const char* devname){
    //1.查看一下是不是只有一个instanceLcd
    //2.为instanceLcd分配一个存储空间
    //3.instanceLcd->fd打开需要使用的设备
    //4.通过ioctl获得struct fb_var_screeninfo的具体数据，放到info中
    //5.最后再用instanceLcd获取info的具体数据



    if(instanceLcd != NULL){
        return instanceLcd;
    }
    //单例思维，全局变量建立了instanceLcd,如果他不为空，就说明系统中已经有一个Lcd了
    //在单例模式中，系统中只能有一个lcd，但是可以有lcd和其他的设备共存

    instanceLcd = (struct LcdDevice*)malloc(sizeof(struct LcdDevice));
    if(instanceLcd==NULL){
        return NULL;
    }

    instanceLcd->fd = open("devname",O_RDWR);
    if(instanceLcd->fd < 0){
        perror("open");
        instanceLcd = NULL;
        return NULL;
    }
    //     struct fb_var_screeninfo {
    //     __u32 xres;          // 可见区域宽度（像素）
    //     __u32 yres;          // 可见区域高度（像素）
    //     __u32 bits_per_pixel; // 每像素位数（如 16/24/32）
    // };
    struct fb_var_screeninfo info;
    int ret = ioctl(instanceLcd->fd ,FBIOGET_VSCREENINFO, &info);
    if(ret < 0){
        perror("ioctl");
        return NULL;
    }
    //ioctl系统调用获取LCD屏幕的可变参数（分辨率、颜色深度等），
    //并将这些参数存储到结构体变量info中，最终赋值给instanceLcd对象的成员变量
    instanceLcd->w = info.xres;
    instanceLcd->h = info.yres;
    instanceLcd->piexlbyte = info.bits_per_pixel;

    instanceLcd->mpty = (unsigned int *)mmap(
    NULL,                          // 1. 映射起始地址（由内核自动分配）
    instanceLcd->w * instanceLcd->h * instanceLcd->piexlbyte,  // 2. 映射区域大小
    PROT_READ | PROT_WRITE,        // 3. 内存保护权限（可读可写）
    MAP_SHARED,                    // 4. 映射类型（共享映射）
    instanceLcd->fd,               // 5. FrameBuffer 设备文件描述符
    0                              // 6. 映射偏移量（从设备头部开始）
    );

    if(instanceLcd->mpty == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }

    instanceLcd->clear = clear;
    instanceLcd->draw_bmp = draw_bmp;
    return instanceLcd;
}

void destory_lcd(){
    if(instanceLcd == NULL){
        return ;
    }
    munmap(instanceLcd->mpty, instanceLcd->w*instanceLcd->h*instanceLcd->piexlbyte);
    //释放映射
    close(instanceLcd->fd);
    free(instanceLcd);
    return ;
}