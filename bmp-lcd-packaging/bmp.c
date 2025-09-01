#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

typedef int int32_t;
typedef short int16_t;

#pragma pack(1)
//#pragma 向编译器传递特殊的命令（如对齐，警告抑制等）
//pack：结构体、联合体、类成员的内存对齐方式
//（1）：要求1字节对其，相当于无对其要求（只是要求无填充紧凑排列）
//紧凑排序就是不填充，减少内存宽带占用

struct bitmap_header {
    short type;        // 文件标识（"BM"=0x4D42）[1,2]
    int fsize;         // 文件总字节数
    short reserved1;   // 保留字段（必须为0）
    short reserved2;   // 保留字段（必须为0）
    int offbits;       // 像素数据偏移量（通常54字节）
    int size;          // 信息头大小（固定40）
    int width;         // 图像宽度（像素）
    int height;        // 图像高度（像素）
    short planes;      // 位平面数（固定1）
    short bit_count;   // 每像素位数（24=真彩色）（色深）
    int compression;   // 压缩方式（0=无压缩）
    int size_img;      // 像素数据大小
    int X_pel;         // 水平分辨率（像素/米）
    int Y_pel;         // 垂直分辨率（像素/米）
    int clrused;       // 调色板颜色数（0=使用全部）
    int clrImportant;  // 重要颜色数（0=所有都重要）
};

struct Bmp* creat_bmp(const char* bmpfile){
//const...是用来接收文件路径，需要设置只读

//1.首先需要打开图片文件，获取图片文件的数据
//2.再创建一个文件头类型的结构图变量，用来读取图片的头，获得宽，高，位深数据
//3.创建一个bmp结构体变量，从文件头结构体类型的变量获取宽，高，位深数据，放入bmp结构体变量中
//4.在处理文件对齐，主要是看看是不是需要补全字符。
//5.逐行读取图片文件的数据，放入bmp->data中。
//结果：变量bmp获得了完整的宽高位深数据，为了以后的操作做了铺垫



    //打开.bmp文件
    FILE* file = fopen(bmpfile,"r");  //只读模式
    //open格式为 int fd = open("wenjian",O_WRDE);
    if(!file){
        perror("fopen");
        return NULL;
    } 

    //读取图片头，主要是为了获取图片的宽高，位深数据
    struct bitmap_header header;
    fread(&header,sizeof(header),1,file);

    //创建BMP对象，是为了获得bmp数据
    struct Bmp* bmp = (struct Bmp*)malloc(sizeof(struct Bmp));
    if(!bmp){
        perror("bmp malloc");
    }
    bmp->h = header.height;
    bmp->w = header.width;
    bmp->pixelbyte = header.bit_count/8;
    bmp->data=malloc(bmp->h*bmp->w*bmp->pixelbyte);
    //存储原始RGB数据

    //处理BMP文件行对齐
    //为了后续的将图片文件的数据放进内存中的bmp做准备
    int linesize = bmp->w*bmp->pixelbyte;     //实际每行的字节数
    int filelinesize = linesize;              //先假设无需填充（当 linesize 已是 4 的倍数时）,为后续填充计算提供基准值
    if(linesize%4 != 0){
        filelinesize += 4 - (linesize % 4);
    }

    //逐行读取数据
    for(int i = 0;1 < bmp->h;i++){
        unsigned char fileline[filelinesize];
        fread(fileline,1,filelinesize,file);
        memcpy(bmp->data + i * linesize, fileline, linesize);
    }
    //Q：为何memcpy(bmp->data + i * linesize, fileline, linesize)；放到内存中是放linesize大小的数据
    //A：先分析图片存储和内存存储的差异
    //1.图片存储按照4字节对齐方式存储，在末尾处填充0X00
    //2.内存存储需要存储色块数据的有效区域，也就是填充之前的数据
    //Q：既然最后放入内存的时候是用的linesize，那filelinesize的意义何在？
    //A；文件存储规范​​与​​内存数据优化​​的分离
    //1.文件存储规范​：每行像素数据的字节数必须是4的倍数​​（即4字节对齐），用 fread 读取 filelinesize（含填充字节），​​满足文件格式解析要求​​
    //               CPU访问内存时，对齐数据可提升读取效率（如32位处理器一次读取4字节）
    //               避免跨内存页访问导致的性能损失
    //               Windows系统底层显存操作依赖此规范
    //2.​内存数据优化：填充字节是文件格式的​​冗余数据​​，对图像内容无意义。
    //               内存中的图像缓冲区只需连续存储​​有效像素​​（linesize），节省空间且提升处理效率。
    //               后续操作（如旋转/缩放）直接基于有效数据计算，无需处理填充干扰。

    fclose(file);
    return bmp;
}

void destory_Bmp(struct Bmp* bmp){
    if(bmp != NULL){
        if(bmp->data != NULL){
            free(bmp->data);
        }
        free(bmp);
    }
}

void reverse(struct Bmp* bmp){
    if(bmp == NULL){
        return ;
    }

    unsigned char* linebuffer = (unsigned char*)malloc(bmp->h*bmp->pixelbyte);
    unsigned int linesize = bmp->h*bmp->pixelbyte;  //一行占用的字节数

    unsigned char* start = bmp->data;
    unsigned char* end = bmp->data + (bmp->h-1)*linesize;
    for(int i = 0;i<bmp->h/2;i++){
        memcpy(linebuffer,start,linesize);
        memcpy(start,end,linesize);
        memcpy(end,linebuffer,linesize);
        start += linesize;
        end -= linesize;
    }
}

void scale(struct Bmp* bmp,int w,int h){
    if(bmp == NULL){
        return ;
    }

    unsigned char* destData = (unsigned char*)malloc(sizeof(w*h*bmp->pixelbyte));

    int pty = bmp->pixelbyte;
    //避免循环中重复访问结构体成员，提升性能

    int dw = bmp->w*1.0/w;
    int dh = bmp->h*1.0/h;
    //运算中*1.0是为了使bmp->w强制转化为浮点数，在后面的/w运算中提高精度

    for(int j;j<h;j++){
        for(int i=0;i<w;i++){
            memcpy(destData+i*pty+j*w*pty,                              \
                bmp->data+((int)(i*dw))*pty+((int)(j*dh))*bmp->w*pty,   \
                pty);
        }
    }

    free(bmp->data);
    bmp->w = w;
    bmp->h = h;
    bmp->data = destData;

}