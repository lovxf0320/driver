#ifndef _BMP_H
#define _BMP_H


struct Bmp{
    int w,h;                   //宽高（像素）
    int pixelbyte;             //位深（色数）每个像素占三字节
    unsigned char *data;       //用来存放RBG的数组
};

struct Bmp* creat_bmp(const char* bmpfile);
void destory_Bmp(struct Bmp* bmp);
void reverse(struct Bmp* bmp);
void scale(struct Bmp* bmp,int w,int h);

#endif