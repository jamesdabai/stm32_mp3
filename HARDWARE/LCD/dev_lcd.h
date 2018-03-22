
#ifndef DEV_LCD_H
#define DEV_LCD_H

#include "stm32f4xx.h"

//#include "global.h"

struct _strLcdConf
{
    //器件参数
    u32 m_driver_id;//LCD 控制器型号
    u32 m_lcd_id;//lcd 型号
    u16 m_width;          //LCD 宽度
    u16 m_height;         //LCD 高度
    
    //配置参数
    u32 m_pencolor;//前景色，画笔颜色
    u32 m_backcolor;//背景色
    u32 m_linewidth;//线宽, 1到10倍

    //控制
    u8  m_dir;            //横屏还是竖屏控制：0，竖屏；1，横屏。  
    u8  m_wramcmd;        //开始写gram指令
    u8  m_setxcmd;        //设置x坐标指令
    u8  m_setycmd;        //设置y坐标指令  
    u8  m_gd;             //LCD ON/OFF 状态 0 OFF, 1 开   

    u16 m_lastx;    //上一次写的X Y 坐标
    u16 m_lasty;
};

typedef struct _strFont
{
u32  m_font;            //字体，关联ASCII字符
u32  m_ascsize;         //ASCII字符字号
u32  m_nativesize;       //国语字号
} strFont;


//2 左上角为屏幕原点（0,0），横向是x轴，纵向是y轴
typedef struct _strRect
{
u32  m_x0;  //矩形起点X坐标
u32  m_y0;  //矩形起点Y坐标
u32  m_x1;  //矩形终点X坐标
u32  m_y1;  //矩形终点Y坐标
} strRect;

typedef struct _strPicture
{
    u32  m_width;   //图形像素点宽度
    u32  m_height;  //图形像素点高度
    u32  * m_pic;   //图形像素数据,m_pic：格式：aRGB, 排列规则：横向排列，从左到右、从上到下
} strPicture;

//返回点阵使用的结构
typedef struct _strFontPra
{
    u32 hv;//横库还是纵库
    u32 width;
    u32 high;
    u32 size;
    u8  *dot;
}strFontPra;


typedef struct tagBITMAPFILEHEADER  //文件头  14B  
{ 
    u16  bfType;   
    u32  bfSize;   
    u16  bfReserved1;   
    u16  bfReserved2;   
    u32  bfOffBits;   
} BITMAPFILEHEADER; 

typedef struct tagBITMAPINFOHEADER  //头文件信息
{ 
    u32 biSize;   
    s32 biWidth;     
    s32 biHeight;     
    u16 biPlanes;   
    u16 biBitCount;  
    u32 biCompression;   
    u32 biSizeImage;   
    s32 biXPelsPerMeter;   
    s32 biYPelsPerMeter;   
    u32 biClrUsed;  
    u32 biClrImportant;  
} BITMAPINFOHEADER;

struct _strFontPara
{
    u8 type;//字库类型
    u8 rare;//关联生僻字库
    u8 name[16];
    u32 offset;
    u32 width;
    u32 high;
    u32 size;//一个字符占空间 BYTE
    u8 hv;//说明是横库还是纵库
    //u32 ;//是否需要指明到底是GB2312还是GB18030?
    u8 font;  //关联的字库
};

typedef struct _strLine
{
    u32  m_x0;  //线起点X坐标
    u32  m_y0;  //线起点Y坐标
    u32  m_x1;  //线终点X坐标
    u32  m_y1;  //线终点Y坐标
} strLine ;

//------线宽补偿方法--------------
struct _strLineWith
{
    s32 m_x0;
    s32 m_x1;
    s32 m_y0;
    s32 m_y1;
};
/*************************************************************************/
#define QR_LCD_H	128
#define QR_LCD_W	160

/*******************************************************************************/

#define LCD_MAX_ROW          4
#define LCD_MAX_COL          128

#define MAXCOL          128 //最大列数
#define MAXROW          4   //最大行数
#define MAXCHAR         21  //每行最大字符数
#define LCDWORDWIDTH    12  //显示字列数
#define LCDCHARWIDTH    6   //显示字符列数

#define OFFSET          0   //偏移列数
#define LCD_COL_OFFSET  OFFSET
#define LCD_MAX_DOT_ROW 32

/*******************************************************************************/

#define ZIKU_BUFF_MAX   64	//一个汉字点阵需要的最大缓冲, 1212汉字，最大是24字节

#define FONT_TYPE_HZ1212    0
#define FONT_TYPE_HZ1616    1
#define FONT_TYPE_HZ2020    2
#define FONT_TYPE_HZ2424    3
#define FONT_TYPE_HZ3232    4  
#define FONT_TYPE_HZ4848    5
#define FONT_TYPE_NOHZ      6//不存在的字库，用于测试黑块

#define FONT_TYPE_ASC6X12    0
#define FONT_TYPE_ASC8X16    1
#define FONT_TYPE_ASC1224    2
#define FONT_TYPE_ASC1632    3
#define FONT_TYPE_ASC2448    4
#define FONT_TYPE_ASC6X8     5
#define FONT_TYPE_ASC1624    6
#define FONT_TYPE_NOASC      7//不存在的字库，用于测试黑块

#define FONT_HORIZONTAL 1//横库
#define FONT_VERTICAL   2//纵库
#define FONT_ORIGINAL   3//原码

#define LINE1   0x00
#define LINE2   0x01
#define LINE3   0x02
#define LINE4   0x03
#define LINE5   0x04

#define FDISP    0x00   //正显(默认正显)//yhz 0905
#define NOFDISP  0x01   //反显
#define INCOL    0x02   //插入一列
#define LDISP    0x04   //左对齐
#define CDISP    0x08   //居中
#define RDISP    0x10   //右对齐

/*用指定颜色填充矩形区域
      1 典型RGB值：红0x00FF0000  绿0x00FF0000  蓝0x00FF0000
                    黑 0x00000000  白0x00FFFFFF
*/
#define DDI_OK 0 //成功
#define DDI_ERR -1 //错误
#define DDI_ETIMEOUT -2 //超时
#define DDI_EBUSY -3 //设备繁忙
#define DDI_ENODEV -4	//设备不存在
#define DDI_EACCES -5	//无权限
#define DDI_EINVAL -6  //参数无效
#define DDI_EIO -7 //设备未打开或设备操作出错
#define DDI_EDATA -8 //数据错误
#define DDI_EPROTOCOL -9 //协议错误
#define DDI_ETRANSPORT -10 //传输错误
    
#define NULL 0
    
#define TRUE 1


#define FALSE 0

void dev_lcd_bl_off(void);
void dev_lcd_bl_on(void);
u8 drv_lcd_find_dot_data_qr(u8 *char_code, u8 *dot_buffer, u8 dot_buffer_size);
void ascii_clumn2cross(u8* pin,u8* pout,u8 len);



extern s32 ddi_qr_lcd_show_picture_file(const strRect *lpstrRect,const u8 * lpBmpName);
//显示二维码
extern s32 ddi_lcd_show_qrcode(char *buf,u8 position,char *tips);
extern s32 ddi_lcd_qr_fill_rectangle(u8 x1,u8 y1,u8 x2,u8 y2, u16 color);
extern s32 ddi_lcd_qr_ShowQR(char *str, int x, int y, int w, int h);




extern s32 ddi_qr_lcd_open(void);
extern s32 ddi_qr_lcd_close(void);
extern s32 ddi_qr_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic);

extern void ddi_qr_backlight_set_level(u8 level) ;
extern u8  ddi_qr_backlight_get_level(void);
extern s32 ddi_qr_lcd_show_text_ext(u32 row, u32 col, void *str, u32 atr);


#endif
