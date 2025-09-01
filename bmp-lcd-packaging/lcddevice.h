#ifndef _LCDDEVICE_H
#define _LCDDEVICE_H
//这是一个显示设备LCD的驱动代码，这也是我接触到的第一个驱动代码
#include "bmp.h"

struct LcdDevice{
    //设备属性
    int w;                  //显示屏宽
    int h;                  //显示屏高
    int piexlbyte;           //位深
    int fd;                  //帧缓冲设备文件描述符
    unsigned int* mpty;      //内存映射指针（直接操作内存）

    void (*draw_bmp)(int x,int y,struct Bmp* bmp);    //在屏幕的x,y处开始绘制图片
    void (*clear)(unsigned int color);                //清屏，color使bmp的RGB颜色
    void (*draw_line)(int x1,int y1,int x2,int y2);   //画直线
    void (*draw_circle)(int x,int y,int r);           //画园
    void (*draw_rect)(int x, int y, int w, int h);    //画矩形
};
// 在嵌入式开发中，这种将函数指针封装在结构体中的设计模式是一种典型的​面向对象编程思想
// 在C语言中的实现​​，其核心目的是提升代码的 ​​可维护性、可扩展性和硬件无关性
// 1. ​​实现硬件抽象与接口统一​​
// ​​问题背景​​：不同型号的LCD（如TFT、OLED、COG）的底层驱动差异较大（如初始化时序、数据写入方式）。
// 若将函数直接定义在结构体外，上层代码需针对每种LCD编写重复逻辑。
// ​​解决方案​​：
// 通过函数指针（如 draw_bmp、clear）将操作抽象为统一接口。
// 不同LCD的驱动只需实现这些函数，并赋值给结构体成员。
//
// 2. ​​支持多设备实例化​​
// ​​场景举例​​：系统需同时控制两个OLED屏（如收银机的主屏和客显屏），两者驱动IC相同但分辨率不同（128x64 vs 128x32）。
// ​​解决方案​​：
// 为每个LCD创建独立的 LcdDevice 实例，其函数指针指向各自的驱动函数。
//
// 3. ​​增强可移植性与模块化​​
// ​​问题​​：更换LCD型号时，若函数分散在全局，需重写所有调用点；若驱动代码与硬件绑定，移植到新平台需大量修改。
// ​​解决方案​​：
// 将驱动实现（如 str7565_draw_bmp()）与接口声明分离。
// 通过 lcd_creat() 工厂函数初始化结构体并绑定具体驱动。


struct LcdDevice* lcd_creat(const char* devname);
void destory_lcd();

#endif