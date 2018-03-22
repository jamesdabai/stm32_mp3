
#ifndef FONT_H
#define FONT_H


#include "global.h"


typedef struct
{
	u16 char_code;
	u8  dot_data[32];
}
FONT_STR;


typedef struct
{
    int filelen;
    int hz_cnt;
    int reserverd;
}
HZ_FLAGS;


#define FONT_ADDRESS_START  0//INTERNAL_ZIKU_ADDR
#define FONT_ADDRESS_END	0//INTERNAL_ZIKU_ADDR+INTERNAL_ZIKU_SIZE-1
#define FONT_ADDRESS_SIZE   0//INTERNAL_ZIKU_SIZE

    



extern const char ascii_16_8[];
extern const char qr_ascii_16_8[];
#define HZNUM 21

//extern const FONT_STR font_16_16[];
extern BOOL drv_lcd_find_dot_data_16x16(u32 char_code, u8 *dot_buffer, u8 dot_buffer_size);
extern BOOL drv_lcd_find_dot_data_8x16(u8 asc_code, u8 *dot_buffer, u8 dot_buffer_size);
extern BOOL drv_lcd_find_dot_data_12x12(u32 char_code, u8 *dot_buffer, u8 dot_buffer_size);
extern BOOL drv_lcd_find_dot_data_qr(u8 *char_code, u8 *dot_buffer, u8 dot_buffer_size);
extern void ascii_clumn2cross(u8* pin,u8* pout,u8 len);
#endif
