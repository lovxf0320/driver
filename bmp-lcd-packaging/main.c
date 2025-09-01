#include <stdio.h>
#include <stdlib.h>                 //标注库（malloc、calloc、exit等）
#include <sys/types.h>              //文件描述符
#include <sys/stat.h>               //文件状态（open的状态）
#include <fcntl.h>                  //文件控制 open等函数
#include <unistd.h>                 //系统调用 write、read等
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include "bmp.h"
#include "lcddevice.h"

int main(){
    struct LcdDevice* lcd= lcd_creat("/dev/fb0");
    clear(0Xffffffff);
    struct Bmp* bmp = creat_bmp("./picture.bmp");
    reverse(bmp);
    scale(bmp ,400, 240);
    lcd->draw_bmp(0,0,bmp);
    destory_Bmp(bmp);
    destory_lcd();
    
}
