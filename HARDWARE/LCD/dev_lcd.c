#include "dev_uart.h"
#include "string.h"
#include "drv_lcd.h"
#include "math.h"
#include "dev_lcd.h"
//#include "drv_lcd_font_data.h"
#include "ff.h"

const char ascii_16_8[]=
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*" ", */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x67, 0xfe, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"!",0*/
0x00, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x02, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x02, 0x00, 0x00, 0x00, /*""",1*/
0x00, 0x02, 0x20, 0x3f, 0xe0, 0x02, 0x3c, 0x3e, 0xa0, 0x03, 0x7c, 0x02, 0x20, 0x00, 0x00, 0x00, /*"#",2*/
0x00, 0x00, 0x78, 0x1c, 0x84, 0x20, 0xfe, 0x7f, 0x04, 0x21, 0x38, 0x1e, 0x00, 0x00, 0x00, 0x00, /*"$",3*/
0x70, 0x60, 0x88, 0x18, 0x70, 0x06, 0x80, 0x01, 0x60, 0x0e, 0x18, 0x11, 0x0c, 0x0e, 0x00, 0x00, /*"%",4*/
0x00, 0x1f, 0xf8, 0x20, 0x84, 0x23, 0x84, 0x34, 0x78, 0x0e, 0x00, 0x33, 0x00, 0x20, 0x00, 0x00, /*"&",5*/
0x10, 0x00, 0x16, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"'",6*/
0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x18, 0x18, 0x04, 0x20, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, /*"(",7*/
0x00, 0x00, 0x02, 0x20, 0x04, 0x10, 0x18, 0x0c, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*")",8*/
0x00, 0x00, 0x40, 0x02, 0x80, 0x01, 0xf0, 0x0f, 0x80, 0x01, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, /*"*",9*/
0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0xf0, 0x07, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, /*"+",10*/
0x00, 0x40, 0x00, 0x58, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*",",11*/
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, /*"-",12*/
0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*".",13*/
0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x80, 0x03, 0x60, 0x00, 0x1c, 0x00, 0x02, 0x00, 0x00, 0x00, /*"/",14*/
0x00, 0x00, 0xf0, 0x0f, 0x08, 0x12, 0x04, 0x21, 0x84, 0x20, 0x48, 0x10, 0xf0, 0x0f, 0x00, 0x00, /*"0",15*/    
0x00, 0x00, 0x00, 0x20, 0x10, 0x20, 0x18, 0x20, 0xfc, 0x3f, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, /*"1",16*/     
0x00, 0x00, 0x18, 0x38, 0x04, 0x24, 0x04, 0x22, 0x04, 0x21, 0x84, 0x20, 0x78, 0x30, 0x00, 0x00, /*"2",17*/
0x00, 0x00, 0x18, 0x18, 0x04, 0x20, 0x84, 0x20, 0x84, 0x20, 0x44, 0x21, 0x38, 0x1e, 0x00, 0x00, /*"3",18*/
0x00, 0x07, 0xc0, 0x04, 0x30, 0x04, 0x08, 0x24, 0xfc, 0x3f, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, /*"4",19*/
0x00, 0x00, 0xfc, 0x18, 0x84, 0x20, 0x44, 0x20, 0x44, 0x20, 0x44, 0x20, 0x8c, 0x1f, 0x00, 0x00, /*"5",20*/ 
0x00, 0x00, 0xf0, 0x1f, 0x08, 0x21, 0x84, 0x20, 0x84, 0x20, 0x84, 0x20, 0x08, 0x1f, 0x00, 0x00, /*"6",21*/
0x00, 0x00, 0x1c, 0x00, 0x04, 0x00, 0x04, 0x3f, 0xc4, 0x00, 0x34, 0x00, 0x0c, 0x00, 0x00, 0x00, /*"7",22*/
0x00, 0x00, 0x78, 0x1e, 0x84, 0x21, 0x84, 0x20, 0x84, 0x20, 0x84, 0x21, 0x78, 0x1e, 0x00, 0x00, /*"8",23*/
0x00, 0x00, 0xf8, 0x10, 0x04, 0x21, 0x04, 0x21, 0x04, 0x21, 0x84, 0x10, 0xf8, 0x0f, 0x00, 0x00, /*"9",24*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x18, 0xc0, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*":",25*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xc0, 0x30, 0xc0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*";",26*/
0x00, 0x00, 0x80, 0x00, 0x40, 0x01, 0x20, 0x02, 0x10, 0x04, 0x08, 0x08, 0x04, 0x10, 0x00, 0x00, /*"<",27*/
0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x00, 0x00, /*"=",28*/
0x00, 0x00, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x00, 0x00, 0x00, /*">",29*/
0x00, 0x00, 0x38, 0x00, 0x04, 0x00, 0x04, 0x36, 0x04, 0x37, 0x84, 0x00, 0x78, 0x00, 0x00, 0x00, /*"?",30*/
0xe0, 0x0f, 0x18, 0x10, 0xc4, 0x27, 0x24, 0x24, 0xe4, 0x23, 0x08, 0x24, 0xf0, 0x13, 0x00, 0x00, /*"@",31*/
0x00, 0x20, 0x00, 0x3e, 0xf0, 0x23, 0x0c, 0x02, 0xf0, 0x23, 0x00, 0x3e, 0x00, 0x20, 0x00, 0x00, /*"A",32*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x84, 0x20, 0x84, 0x20, 0x44, 0x21, 0x38, 0x1e, 0x00, 0x00, /*"B",33*/
0xe0, 0x07, 0x18, 0x18, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x10, 0x18, 0x0c, 0x00, 0x00, /*"C",34*/
0x04, 0x20, 0xfc, 0x3f, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x08, 0x10, 0xf0, 0x0f, 0x00, 0x00, /*"D",35*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x84, 0x20, 0xe4, 0x23, 0x04, 0x20, 0x08, 0x10, 0x00, 0x00, /*"E",36*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x84, 0x00, 0xe4, 0x03, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, /*"F",37*/
0xf0, 0x0f, 0x08, 0x10, 0x04, 0x20, 0x04, 0x20, 0x04, 0x22, 0x0c, 0x1e, 0x10, 0x02, 0x00, 0x00, /*"G",38*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x80, 0x00, 0x84, 0x20, 0xfc, 0x3f, 0x04, 0x20, 0x00, 0x00, /*"H",39*/
0x00, 0x00, 0x04, 0x20, 0x04, 0x20, 0xfc, 0x3f, 0x04, 0x20, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, /*"I",40*/
0x00, 0x10, 0x00, 0x20, 0x04, 0x20, 0x04, 0x20, 0xfc, 0x1f, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, /*"J",41*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0xc0, 0x01, 0x20, 0x26, 0x1c, 0x38, 0x04, 0x20, 0x00, 0x00, /*"K",42*/
0x04, 0x20, 0xfc, 0x3f, 0x04, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x30, 0x00, 0x00, /*"L",43*/
0x0c, 0x20, 0xfc, 0x3f, 0x7c, 0x00, 0x80, 0x3f, 0x7c, 0x00, 0xfc, 0x3f, 0x04, 0x20, 0x00, 0x00, /*"M",44*/
0x04, 0x20, 0xfc, 0x3f, 0x3c, 0x20, 0xc0, 0x01, 0x04, 0x0e, 0xfc, 0x3f, 0x04, 0x00, 0x00, 0x00, /*"N",45*/
0xf0, 0x0f, 0x08, 0x10, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x08, 0x10, 0xf0, 0x0f, 0x00, 0x00, /*"O",46*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x84, 0x00, 0x84, 0x00, 0x84, 0x00, 0x78, 0x00, 0x00, 0x00, /*"P",47*/
0xf0, 0x0f, 0x08, 0x10, 0x04, 0x24, 0x04, 0x24, 0x04, 0x38, 0x08, 0x70, 0xf0, 0x4f, 0x00, 0x00, /*"Q",48*/
0x04, 0x20, 0xfc, 0x3f, 0x84, 0x20, 0x84, 0x01, 0x84, 0x06, 0x78, 0x38, 0x00, 0x20, 0x00, 0x00, /*"R",49*/
0x00, 0x00, 0x78, 0x18, 0x84, 0x20, 0x84, 0x20, 0x04, 0x21, 0x04, 0x21, 0x18, 0x1e, 0x00, 0x00, /*"S",50*/ 
0x0c, 0x00, 0x04, 0x00, 0x04, 0x20, 0xfc, 0x3f, 0x04, 0x20, 0x04, 0x00, 0x0c, 0x00, 0x00, 0x00, /*"T",51*/
0x04, 0x00, 0xfc, 0x1f, 0x04, 0x20, 0x00, 0x20, 0x04, 0x20, 0xfc, 0x1f, 0x04, 0x00, 0x00, 0x00, /*"U",52*/
0x04, 0x00, 0x7c, 0x00, 0x84, 0x07, 0x00, 0x38, 0x84, 0x07, 0x7c, 0x00, 0x04, 0x00, 0x00, 0x00, /*"V",53*/
0x04, 0x00, 0xfc, 0x03, 0x00, 0x3f, 0xfc, 0x01, 0x00, 0x3f, 0xfc, 0x03, 0x04, 0x00, 0x00, 0x00, /*"W",54*/
0x04, 0x20, 0x1c, 0x38, 0x64, 0x26, 0x80, 0x01, 0x64, 0x26, 0x1c, 0x38, 0x04, 0x20, 0x00, 0x00, /*"X",55*/
0x04, 0x00, 0x3c, 0x00, 0xc4, 0x20, 0x00, 0x3f, 0xc4, 0x20, 0x3c, 0x00, 0x04, 0x00, 0x00, 0x00, /*"Y",56*/
0x00, 0x00, 0x08, 0x30, 0x04, 0x2c, 0x04, 0x23, 0xc4, 0x20, 0x34, 0x20, 0x0c, 0x30, 0x00, 0x00, /*"Z",57*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x3f, 0x02, 0x20, 0x02, 0x20, 0x02, 0x20, 0x00, 0x00, /*"[",58*/
0x00, 0x00, 0x0c, 0x00, 0x30, 0x00, 0xc0, 0x01, 0x00, 0x06, 0x00, 0x38, 0x00, 0x40, 0x00, 0x00, /*"\",59*/
0x00, 0x00, 0x02, 0x20, 0x02, 0x20, 0x02, 0x20, 0xfe, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"]",60*/
0x00, 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, /*"^",61*/
0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x00, /*"_",62*/
0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"`",63*/
0x00, 0x00, 0x40, 0x0c, 0x20, 0x12, 0x20, 0x11, 0x20, 0x11, 0xc0, 0x1f, 0x00, 0x10, 0x00, 0x00, /*"a",64*/
0x08, 0x00, 0xf8, 0x1f, 0x00, 0x11, 0x80, 0x10, 0x80, 0x10, 0x80, 0x10, 0x00, 0x0f, 0x00, 0x00, /*"b",65*/
0x00, 0x00, 0x00, 0x07, 0x80, 0x08, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x80, 0x08, 0x00, 0x00, /*"c",66*/
0x00, 0x00, 0x80, 0x0f, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x48, 0x08, 0xf8, 0x1f, 0x00, 0x10, /*"d",67*/
0x00, 0x00, 0x80, 0x0f, 0x40, 0x11, 0x40, 0x11, 0x40, 0x11, 0x40, 0x11, 0x80, 0x09, 0x00, 0x00, /*"e",68*/
0x00, 0x00, 0x40, 0x10, 0x40, 0x10, 0xf0, 0x1f, 0x48, 0x10, 0x48, 0x10, 0x18, 0x00, 0x00, 0x00, /*"f",69*/
0x00, 0x00, 0x80, 0x35, 0x40, 0x4a, 0x40, 0x4a, 0x40, 0x4a, 0xc0, 0x49, 0x40, 0x30, 0x00, 0x00, /*"g",70*/
0x08, 0x10, 0xf8, 0x1f, 0x80, 0x10, 0x40, 0x00, 0x40, 0x00, 0x40, 0x10, 0x80, 0x1f, 0x00, 0x10, /*"h",71*/
0x00, 0x00, 0x40, 0x10, 0x58, 0x10, 0xd8, 0x1f, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /*"i",72*/
0x00, 0x00, 0x00, 0x60, 0x00, 0x40, 0x40, 0x40, 0x58, 0x40, 0xd8, 0x3f, 0x00, 0x00, 0x00, 0x00, /*"j",73*/
0x08, 0x10, 0xf8, 0x1f, 0x00, 0x12, 0x00, 0x01, 0xc0, 0x16, 0x40, 0x18, 0x40, 0x10, 0x00, 0x00, /*"k",74*/
0x00, 0x00, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /*"l",75*/
0x40, 0x10, 0xc0, 0x1f, 0x40, 0x00, 0xc0, 0x1f, 0x40, 0x00, 0x80, 0x1f, 0x00, 0x10, 0x00, 0x00, /*"m",76*/
0x40, 0x10, 0xc0, 0x1f, 0x80, 0x10, 0x40, 0x00, 0x40, 0x00, 0x40, 0x10, 0x80, 0x1f, 0x00, 0x10, /*"n",77*/
0x00, 0x00, 0x00, 0x07, 0x80, 0x08, 0x40, 0x10, 0x40, 0x10, 0x80, 0x08, 0x00, 0x07, 0x00, 0x00, /*"o",78*/
0x40, 0x40, 0xc0, 0x7f, 0x80, 0x50, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x80, 0x0f, 0x00, 0x00, /*"p",79*/
0x00, 0x00, 0x80, 0x0f, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x40, 0x48, 0xc0, 0x7f, 0x00, 0x40, /*"q",80*/
0x40, 0x10, 0x40, 0x10, 0xc0, 0x1f, 0x80, 0x10, 0x40, 0x10, 0x40, 0x00, 0xc0, 0x00, 0x00, 0x00, /*"r",81*/
0x00, 0x00, 0x80, 0x19, 0x40, 0x12, 0x40, 0x12, 0x40, 0x12, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, /*"s",82*/
0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0xf0, 0x0f, 0x40, 0x10, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00, /*"t",83*/
0x40, 0x00, 0xc0, 0x0f, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x40, 0x08, 0xc0, 0x1f, 0x00, 0x10, /*"u",84*/
0x40, 0x00, 0xc0, 0x00, 0x40, 0x07, 0x00, 0x18, 0x00, 0x04, 0x40, 0x03, 0xc0, 0x00, 0x40, 0x00, /*"v",85*/
0x40, 0x00, 0xc0, 0x03, 0x00, 0x1c, 0xc0, 0x03, 0x00, 0x1c, 0xc0, 0x03, 0x40, 0x00, 0x00, 0x00, /*"w",86*/
0x00, 0x00, 0x40, 0x10, 0xc0, 0x18, 0x00, 0x17, 0x40, 0x07, 0xc0, 0x18, 0x40, 0x10, 0x00, 0x00, /*"x",87*/
0x40, 0x40, 0xc0, 0x40, 0x40, 0x47, 0x00, 0x38, 0x00, 0x0c, 0x40, 0x03, 0xc0, 0x00, 0x40, 0x00, /*"y",88*/
0x00, 0x00, 0xc0, 0x10, 0x40, 0x18, 0x40, 0x16, 0x40, 0x11, 0xc0, 0x10, 0x40, 0x18, 0x00, 0x00, /*"z",89*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x7e, 0x3f, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00, /*"{",90*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"|",91*/
0x00, 0x00, 0x00, 0x00, 0x02, 0x20, 0x7e, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*"}",92*/
0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x04, 0x00, 0x03, 0x00, /*"~",93*/
0x00, 0x00, 0xFE, 0x7F, 0x06, 0x60, 0x0A, 0x50, 0x12, 0x48, 0x22, 0x44, 0x42, 0x42, 0x82, 0x41, /*汉字无字提示前半框*/
0x82, 0x41, 0x42, 0x42, 0x22, 0x44, 0x12, 0x48, 0x0A, 0x50, 0x06, 0x60, 0xFE, 0x7F, 0x00, 0x00, /*汉字无字提示后半框*/
};



extern u8 lcd_lbuf[640];

#define LCD_DEBUG 1

#if defined(LCD_DEBUG)&&LCD_DEBUG
	#define lcd_printf uart_printf
#else
	#define lcd_printf(arg, ...)
#endif



extern u16 qr_backcolor;//背景色

u8 		g_qr_lcd_fd=0;
struct 	_strLcdConf LcdConf;

strFont CurrFont=
{
    0,//宋体
    FONT_TYPE_ASC8X16,
    FONT_TYPE_HZ1616,
};
		
FIL hz_fd;//XQY汉字打开的文件全局变量


#if 0 /*xqy 2018-1-29*/
/***************************************************************************************************
函数：unsigned long gt(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
功能：计算汉字点阵在字库中的地址
参数：c1,c2,c3,c4：4字节汉字内码通过参数c1,c2,c3,c4传入，双字节内码通过参数c1,c2传入，c3=0,c4=0
返回：汉字点阵的字节地址(byte address)。如果用户是按word mode 读取点阵数据，则其地址(word
address)为字节地址除以2，即：word address = byte address / 2 .
例如：BaseAdd: 说明汉字点阵数据在字库中的起始地址，
“啊”字的内码为0xb0a1,则byte address = gt(0xb0,0xa1,0x00,0x00) *32

“ ”字的内码为0x8139ee39,则byte address = gt(0xee,0x39,0x81,0x39) *32
总数:27538+1038
****************************************************************************************************/
unsigned long gt (unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    unsigned long h=0;

    if(c2 == 0x7f)return (h);

    if(c1 >= 0xA1 && c1 <= 0xA9 && c2 >= 0xa1) //Section 1        (yx:本区字符846 A1A1—A9FE)
    {
        h= (c1 - 0xA1) * 94 + (c2 - 0xA1);
    }

    else if(c1 >= 0xa8 && c1 <= 0xa9 && c2 < 0xa1) //Section 5    (yx:本区字符192 A840—A9A0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0xa8)*96 + (c2 - 0x40) + 846;
    }

    if(c1 >= 0xb0 && c1 <= 0xf7 && c2 >= 0xa1) //Section 2         (yx:本区字符6768 B0A1—F7FE)
        h = (c1 - 0xB0) * 94 + (c2 - 0xA1) + 1038;
    else if(c1 < 0xa1 && c1 >= 0x81) //Section 3                  (yx:本区字符6080 8140—A0FE)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1 - 0x81)*190 + (c2 - 0x40) + 7806;//1038 + 6768;
    }
    else if(c1 >= 0xaa && c2 < 0xa1) //Section 4                  (yx:本区字符8160 AA40—FEA0)
    {
        if(c2 > 0x7f)
            c2--;
        h = (c1-0xaa)*96 + (c2-0x40) + 13886;//1038 +12848;
    }

    if(c2 >= 0x30 && c2 <= 0x39) //Extended Section (With 4 BYTES InCode)    (yx:本区字符6530 0x81308130--0x8439FE39)
    {
        if(c4 < 0x30 || c4 > 0x39)
            return(h = 0);

        h = (c3 - 0x81) * 12600 + (c4 - 0x39) * 1260 + (c1 - 0xee) * 10 + (c2 - 0x39) + 22046;
        if(h >= 22046 && h <= 28576) //22046+6530
            return(h);
        else
        {
            h=0;
            return(h);
        }
    }
    return(h);
}

/****************************************************************************
**Description:      读取点阵数据,
                     如字库不存在汉字，则打开生僻字库
**Input parameters: Font 字体类型
                    Str  字符内码

**Output parameters:
                    Dot 返回点阵
**
**Returned value:   -1 失败
                    -2 无字库
                    >0 Str 偏移  1 说明是ASC，2 一般是汉字，4 也是汉字
**
**Created by:            hxj(2014-4-22) ---谭波2014 05 21因为打印需要修改
**--------------------------------------------------------------------------
**Modified by:      2014.06.21 将1212转为横库
**Modified by:
****************************************************************************/
s32 font_get_dot(strFont Font, const u8* Str, strFontPra *FontPra)
{
	BOOL ret=FALSE;
	u16 temp;
	
	//u8 str_tmp[3]={0};
	
	if(Font.m_ascsize==FONT_TYPE_ASC8X16 && Font.m_nativesize==FONT_TYPE_HZ1616)
	{
		if(FontPra->dot==NULL)
			return -1;
		if(*Str>0x80)
		{
			FontPra->hv=FONT_VERTICAL;
			FontPra->high=16;
			FontPra->width=16;
			FontPra->size=32;

			temp=(u16)Str[0]<<8|Str[1];
			ret=drv_lcd_find_dot_data_16x16(temp,FontPra->dot , 32);//
			
			if(ret==FALSE)
			{
                memcpy(FontPra->dot, &ascii_16_8[1520], 32); //显示无字符号
			}
			return 2;
		}
		else
		{
			FontPra->hv=FONT_VERTICAL;
			FontPra->high=16;
			FontPra->width=8;
			FontPra->size=16;
			
			//ret=drv_lcd_find_dot_data_16x16((u32)*Str, FontPra->dot, 32);
            temp=(Str[0]-0x20)*16;
            memcpy(FontPra->dot, &ascii_16_8[temp], 16); 
			
			return 1;
		}
	}

	return -1;
}
#endif

/****************************************************************************
**Description:      解析输入的字符串，计算指定的字体的宽度与高度     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-7-1)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 font_get_str_wh(strFont Font, const u8* Str, u32* w, u32* h)
{
    u32 str_index = 0;

    if(Str[0] < 0x80)//ASCII码xqy
    {
        *w = 8;
        *h = 16;
        str_index++;
    }
    else
    {
        if(Str[str_index + 1] >= 0x30 && Str [str_index + 1] <= 0x39)
        {
            str_index += 4;
            uart_printf("4字节汉字\r\n");
        }
        else
        {
            str_index += 2;
        }
        
        *w = 16;
        *h = 16;
    }    

    return str_index;
}

/**
 *@brief:      dev_QR_open_lcd
 *@details:       打开二维码LCD显示屏设备
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */

s32 dev_QR_lcd_open(void)
{
    u32 ret;
    if (g_qr_lcd_fd == 0)
    {
        ret = f_open(&hz_fd,"font/hz1616.bin",FA_READ);//只读的方式打开汉字字库文件系统
        if(ret != FR_OK)
        {
            lcd_printf("打开font/hz1616.bin文件失败\n");
            return -1;
        }
        else
        {
            lcd_printf("打开font/hz1616.bin文件成功\n");
        }
        /*init lcd*/
        st7735s_initial();
		dev_lcd_bl_on();//打开lcd屏的背光
        g_qr_lcd_fd = 1;
    }
    
    return 0;
}

u8  dev_QR_lcd_is_open(void)
{	
	return (g_qr_lcd_fd==1);
}

/*******************************************************************
函数名称:   s32 dev_QR_lcd_close(void)
函数功能:   关闭Lcd设备文件
相关调用:
入口参数: 无
返 回 值: 无
备    注:
创建信息: 空函数
修改信息:
********************************************************************/
s32 dev_QR_lcd_close(void)
{

	lcd_printf("%s:to be done!!!!\r\n", __func__);
	if(g_qr_lcd_fd == 1)
	{
	    f_close(&hz_fd);//关闭字库文件
	    lcd_printf("关闭了font/hz1616.bin文件\n");
	    dev_lcd_bl_off();
        g_qr_lcd_fd = 0;
	}
    return 0;
}

/****************************************************************************
**Description:       根据当前设置的字体，计算字符串点阵长度与宽度    
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:            hxj(2014-7-1)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 dev_lcd_get_str_dot_len(u8* lpText, u32* w, u32* h)
{
    u32 str_index, str_len;
    s32 res;
    u32 w_tmp, h_tmp;
    u32 dot_len = 0;
    u32 dot_h=0;
    
    str_len = strlen((const char *)lpText);

    for(str_index = 0; str_index < str_len; )
    {
        if(lpText[str_index]<0x20)
        {
            str_index++;
            uart_printf("非可见字符\r\n");
            continue;
        }
        
        res = font_get_str_wh(CurrFont, &lpText[str_index], &w_tmp, &h_tmp);//获取字符的长度宽度，和ascii嘛共用
        if(res < 0)
        {
            uart_printf("获取点阵失败\r\n");
            return -1;
        }
        else
        {
            str_index += res;//调整字符串偏移
        }
        //uart_printf("w %d\r\n", w_tmp);
        dot_len += w_tmp;
        
        if(h_tmp > dot_h)
            dot_h = h_tmp;
    }

    //uart_printf("字符串占%d个点宽\r\n",  dot_len);

    *w = dot_len;
    *h = dot_h;
    return 0;
}

s32 ddi_lcd_qr_fill_rectangle(u8 x1,u8 y1,u8 x2,u8 y2, u16 color)
{
    //DEBUG_SHOW_FUN_NAME;
    dev_lcd_qr_fill_rectangle(x1,y1,x2,y2,color);
    return 0;
}

/*******************************************************************
函数名称: s32 dev_lcd_show_bmp_ram(u32 x, u32 y, u32 xlen, u32 ylen, s8 *BmpFileName)
函数功能: 显示位图至临时缓存,仅支持单色图片的显示
相关调用:
入口参数:   x:起始点x坐标
            y:起始点y坐标
            xlen:显示区域长
            ylen；显示区域高
            BmpFileName:位图名称
返 回 值:   0:成功
            -6:找不到图片文件目录
            -1:不支持的图片格式
备    注:
创建信息:   hecaiwen 2014-09-17
修改信息:
********************************************************************/

static s32 dev_lcd_show_bmp_ram(u32 x, u32 y, u32 xlen, u32 ylen, const u8 *BmpFileName)
{
	u16 bmpfileheaddata[56/2] ;
	unsigned short buf[40];
    //BMP16_HEADER   bmp_header;
    unsigned long     bfOffBits;           
    long      biWidth;     
    long      biHeight;     
    unsigned short int   biBitCount;  
	unsigned long LineBytes;
	unsigned long ImgSize;
	unsigned long i, j, k;
	unsigned long NumColors;
	u32 r,g,b,c;
    u8 rgb565_flag = 0;
   	u8 data[3*160] ;
    u16 picdata ;
    u16 *pixel ;
    int  len;

    u8 file_name[20];//保存图片的路径名

	FIL BMP_FD;
	u32 ret;
	
    sprintf(file_name , "picture/%s",BmpFileName);//拼接成完整路径名
	ret = f_open(&BMP_FD,file_name,FA_READ);
	if(ret != 0)
	{
	    lcd_printf("打开%s文件失败\n",file_name);
	    return -1;
	}
	lcd_printf("打开%s文件成功\n",file_name);
    if( x >= LCD_QR_WIDTH || y >= LCD_QR_HEIGHT ) {
        return -1;
    }

    if( x + xlen > LCD_QR_WIDTH ) {
        xlen = LCD_QR_WIDTH - x ;
    }

    if( y + ylen > LCD_QR_HEIGHT ) {
        ylen = LCD_QR_HEIGHT - y;
    }
    f_read(&BMP_FD,bmpfileheaddata,56,&len);//读文件头数据
    
    //dev_spiflash_read(FS_XGD_LOGO_ADDR,bmpfileheaddata,56);
    bfOffBits = (bmpfileheaddata[6]<<16) | bmpfileheaddata[5] ;		
    biWidth = (bmpfileheaddata[10]<<16) | bmpfileheaddata[9] ;//bmp_header.bi.biWidth;
    biHeight = (bmpfileheaddata[12]<<16) | bmpfileheaddata[11] ;//bmp_header.bi.biHeight;		
    biBitCount = bmpfileheaddata[14];//bmp_header.bi.biBitCount;
	LineBytes = (unsigned long) ((biWidth * biBitCount + 31) >> 5) << 2;
	ImgSize   = (unsigned long) LineBytes * biHeight;
    switch(biBitCount)
    {
		case 16:
    	    NumColors=0;
			if( 0xF800 == bmpfileheaddata[27]) { //F800h是蓝色分量的掩码,RGB555的是7C00h
                rgb565_flag = 1;
            }            
    	    break;
    	case 24:
    	    NumColors=0;
    	    break;
        default:
            return 2; 
	}
    
	if (xlen > biWidth)
		xlen = biWidth;
	if (ylen > biHeight)
		ylen = biHeight;
	st7735s_set_disp_rect(x, y, xlen-1, ylen-1);
	switch (biBitCount)
	{
	case 16:    
		
		for (j = 0; j < ylen; j++)
		{
		    f_lseek(&BMP_FD,(bfOffBits+(ylen-j-1)*LineBytes) );//定位偏移
		    f_read(&BMP_FD,data,LineBytes,&len);//读数据
			//dev_spiflash_read(FS_XGD_LOGO_ADDR+bfOffBits+(ylen-j-1)*LineBytes, data, LineBytes);
			//dev_spiflash_read(BmpFileName+bfOffBits+j*LineBytes, data, LineBytes);
			pixel = (u16*)data ;
			if(rgb565_flag)
			{
				for(i=0; i<xlen ; i++)
				{
					lcd_lbuf[i*2] = data[i*2+1] ;
					lcd_lbuf[i*2+1]= data[i*2];
				}
			}
			else
			{
				for(i=0; i<xlen ; i++)
				{
					c = *pixel++;
					b = c & 0x001f;
	        		c = (c & ~0x001f)<<1 | b;
					//lcd_lbuf[(xlen-1)*2-(i*2)] = (u8)(c>>8) ;
					lcd_lbuf[i*2] = (u8)(c>>8) ;
					lcd_lbuf[i*2+1]= (u8)c;
				}
			}
			lcd_qr_send_data(lcd_lbuf,  xlen * 2 );
		}
		break;
	case 24: {
		u8 *sample;
		k = 0;        
		for (j = 0; j < ylen; j++)
		{
		    f_lseek(&BMP_FD,(bfOffBits+(ylen-j-1)*LineBytes));//定位偏移
		    f_read(&BMP_FD,data,LineBytes,&len);//读数据
			//dev_spiflash_read(FS_XGD_LOGO_ADDR+bfOffBits+(ylen-j-1)*LineBytes, data, LineBytes);
			//dev_spiflash_read(BmpFileName+bfOffBits+j*LineBytes, data, LineBytes);
			sample = data;//&sd256_bitmap[k];
			for (i = 0; i < xlen; i++)
			{
				b = *sample++ >> 3;
				g = (*sample++ & 0xfc) << 3;
				r = (*sample++ & 0xf8) << 8;
				c = r | g | b;  
				lcd_lbuf[i*2] = (u8)(c>>8);
				lcd_lbuf[i*2+1] = (u8)c ;
			}
			lcd_qr_send_data(lcd_lbuf,  xlen * 2 );
		}
		break;
	}
	default:
			break;
	}
	f_close(&BMP_FD);
	return 0;
}

/****************************************************************************
**Description:      打开显示设备
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:      测试OK
**Modified by:
****************************************************************************/
s32 ddi_qr_lcd_open (void)
{
    //DEBUG_SHOW_FUN_NAME;
    return dev_QR_lcd_open();
}


/****************************************************************************
**Description:        关闭显示设备
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:     测试OK
**Modified by:
****************************************************************************/
s32 ddi_qr_lcd_close (void)
{
   //DEBUG_SHOW_FUN_NAME;
   return dev_QR_lcd_close();
}

/****************************************************************************
**Description:     在指定位置显示指定大小的图片
**Input parameters:  six0,siy0,图片左上角坐标
                     siXLen,siYlen,图片宽与高
                     lpstrPic->m_pic 图片数据指针，单色点阵液晶字模，
                                纵向取模，字节倒序
**Output parameters:
**
**Returned value: -1 失败   0 成功
**
**Created by:            LJ 2017.9.30
**--------------------------------------------------------------------------
**Modified by: hecaiwen(2014-8-19)
**Modified by:
****************************************************************************/
static s32 dev_qr_lcd_show_bmp(const strRect *lpstrRect, const strPicture * lpstrPic)
{
//    u8 data; 
//    u8 x,y;
    u8 i,j;
    u8 x_max, y_max;
		u8 * p;
		int len;
    //u16 c;
    u8 siXLen, siYlen;
    u32 siX0 = lpstrRect->m_x0;
    u32 siY0 = lpstrRect->m_y0;
    u8 *pasBmpData = (u8 *)lpstrPic->m_pic;

	if((lpstrRect->m_x1 - lpstrRect->m_x0) > lpstrPic->m_width)
	{
		//BLcdDebug("区域比图片宽\r\n");
		x_max = lpstrPic->m_width;
	}
	else
	{
		//BLcdDebug("区域比图片窄\r\n");
		x_max = lpstrRect->m_x1 - lpstrRect->m_x0;
	}
		
	if((lpstrRect->m_y1 - lpstrRect->m_y0) > lpstrPic->m_height)
	{
		//BLcdDebug("区域比图片高\r\n");
		y_max = lpstrPic->m_height;
	}
	else
	{
		//BLcdDebug("区域比图片矮\r\n");
		y_max = lpstrRect->m_y1 - lpstrRect->m_y0;
	}

	if( (y_max + lpstrRect->m_y0) >  QR_LCD_H )
	{	
		y_max = QR_LCD_H - lpstrRect->m_y0;
	}

	if((x_max + lpstrRect->m_x0) > QR_LCD_W)
	{
		x_max = QR_LCD_W - lpstrRect->m_x0;
	}

	siXLen = lpstrPic->m_width;
	siYlen = lpstrPic->m_height; 

	//x = 0;
	//y = 0;


	st7735s_set_disp_rect( siX0, siY0, siX0 + x_max - 1, siY0 + y_max -1);
	//uart_printf("[%d, %d], [%d, %d]\r\n", siX0, siY0, siX0 + x_max-1, siY0 + y_max-1); 
	//for (i = y_max ; i > 0; i-- )
	for (i = 0 ; i < siYlen; i++ )
	{
		for(j=0; j<siXLen; j++)
		{
			lcd_lbuf[j*2] = pasBmpData[i*siXLen*2+j*2+1] ;
			lcd_lbuf[j*2+1] = pasBmpData[i*siXLen*2+j*2] ;
		}
		lcd_qr_send_data(lcd_lbuf,  siXLen * 2 );
	}
	//uart_printf("%s:%d\r\n", __func__, __LINE__);

	return 0;//成功

}

/****************************************************************************
**Description:         在指定矩形区域显示图形
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            lj 2017.9.29
**--------------------------------------------------------------------------
**Modified by:   仅支持RGB565直接显示到屏幕
**Modified by:
****************************************************************************/
s32 ddi_qr_lcd_show_picture(const strRect *lpstrRect, const strPicture * lpstrPic)
{    
    //DEBUG_SHOW_FUN_NAME;
    if(!dev_QR_lcd_is_open())
        return DDI_EIO;
	
	 dev_qr_lcd_show_bmp( lpstrRect, lpstrPic );
    return 0;
}
u8 dev_Find_Hz_pos(char *Hz,u8 *dot)
{
    //使用全字库
	drv_lcd_find_dot_data_qr((u8*)Hz,dot,32);
	return 1;
}
u8 dev_Find_ascii_pos(u8 indata,u8 *dot_buff)
{
    u32 temp;
//    u8 a,i,j;
    u8 data[16];

    temp=(indata-0x20)*16;
    memcpy(data,&ascii_16_8[temp],16);
    
	ascii_clumn2cross(data,dot_buff,16);
    
	return 1;
}

s32 ddi_qr_lcd_show_text(u32 nX, u32 nY, u32 nX1, u32 nY1, u8* lpText)
{
	u8  j, len=0 ;
//	u16 temp;
	u8 hzaddr[32] ;
//	char hz[2] ;
//	u8 num ;
	u8 x1 = (nX&0xFF);
	u8 y1 = (nY&0xFF);
//	u8 x2 = (nX1&0xFF);
//	u8 y2 = (nY1&0xFF);

	len = strlen((const char*)lpText) ;
    for(j=0; j<len; j++)
    {
        if(lpText[j]>0x80)
        {   
            dev_Find_Hz_pos((char*)&lpText[j],hzaddr);//16x16的汉字
            lcd_qr_ascii_setPos(x1, y1,x1+15,y1+15);
            lcd_qr_string_display(hzaddr,32);
            x1 += 16;
            j++ ;
        }
        else
        {
            lcd_qr_ascii_setPos(x1, y1,x1+7,y1+15);
            x1 += 8;
            dev_Find_ascii_pos(lpText[j],hzaddr);
            lcd_qr_string_display(hzaddr,16);
        }
    }
	return DDI_EIO;
}


u8 drv_lcd_find_dot_data_qr(u8 *char_code, u8 *dot_buffer, u8 dot_buffer_size)
{
    u32 ziku_offset;
    u8 tmp1,tmp2;
    int real_len;//读出数据的真实长度
    
	if(dot_buffer_size != 32)
		return FALSE;
	
    /*if(char_code >= 0xA1A1)
    {
        tmp1 = (char_code>>8)&0xff;
        tmp2 = (char_code)&0xff;
        
        ziku_offset= ((tmp1 - 0xA1)*94 + (tmp2 - 0xA1));
        ziku_offset = ziku_offset*32;//计算出待显示汉字在字库对应位置
    }*/
    if(char_code[0] >= 0xA1)
    {
        tmp1 = (char_code[0])&0xff;
        tmp2 = (char_code[1])&0xff;
        
        ziku_offset= ((tmp1 - 0xA1)*94 + (tmp2 - 0xA1));
        ziku_offset = ziku_offset*32;//计算出待显示汉字在字库对应位置
    }
    else
    {
        lcd_printf("font,err\r\n");
        return FALSE;
    }
    if(dev_QR_lcd_is_open() == 1)
    {
        f_lseek(&hz_fd, ziku_offset);//定位偏移
        f_read(&hz_fd,dot_buffer,32,&real_len);
    }
    else
    {
        uart_printf("字库文件还没有打开\n");
        return FALSE;
    }
    //dev_spiflash_read(FS_HANZI_ZIKU_ADDR+ziku_offset,dot_buffer,32);
    return TRUE;
}

//将205的ascii的纵库转为横库
void ascii_clumn2cross(u8* pin,u8* pout,u8 len)
{
    u8 i,j;//,m,tmp;
    u8 data[32];
	
		for(j = 0;j<8;j++)
    {
        data[7-j] = 0;
        for(i = 0;i<8;i++)
        {
            data[7-j] <<= 1;
            data[7-j] |= (pin[i*2]>>(7-j))&0x01; 
        }     
    }
    for(j = 0;j<8;j++)
    {
        data[15-j] = 0;
        for(i = 0;i<8;i++)
        {
            data[15-j] <<= 1;
            data[15-j] |= (pin[i*2+1]>>(7-j))&0x01; 
            
        }     
    }
    memcpy(pout,data,16);
}



/****************************************************************************
**Description:			只是将点坐标转换成了行列坐标
**Input parameters:	
**Output parameters:	
**					
**Returned value:		
**					
**Created by:			xqy(2018-1-28)
**--------------------------------------------------------------------------
**Modified by:		
**Modified by:		
****************************************************************************/
s32 ddi_qr_lcd_show_text_ext(u32 row, u32 col, void *str, u32 atr)
{
//	u8 str_len ;
	u32 w,h,w_tmp,h_tmp;
	u32 t_atr;
	u32 row_tmp;
	u32 col_tmp;
	int res ;
	//DEBUG_SHOW_FUN_NAME;
	dev_lcd_get_str_dot_len(str, &w, &h);//取得该字符串的宽度和长度xqy

	if((w==0) || (h==0))
			return -1;

	if(w > LCD_QR_WIDTH-1)
	{
		uart_printf("\r\nshow text length exceed\r\n");
	}
	
		t_atr = atr & 0x1C;//(LDISP | CDISP | RDISP)
		switch(t_atr)
		{
		case CDISP://居中
			//uart_printf("居中");
			col_tmp = col + (LCD_QR_WIDTH - w)/2;
			break;
	
		case RDISP://右对齐
			//uart_printf("右对齐");
			col_tmp = col + LCD_QR_WIDTH - w;//暂时不考虑字符串超出长度的情况。
			break;
	
		default://左对齐
			//uart_printf("左对齐");
			col_tmp = col;
			break;
		}
	
		if(col_tmp > LCD_QR_WIDTH) 
			col_tmp = 0;
	
		res = font_get_str_wh(CurrFont, "a", &w_tmp, &h_tmp);
		if(res < 0)
		{
			lcd_printf("\r\nget dot para fail");
			return -1;
		}
		else
		{
			//lcd_printf("\r\nw=%d,h=%d", w_tmp, h_tmp);
		}
	
		row_tmp = h_tmp*row;
		//uart_printf("col_tmp = %d row_tmp = %d \r\n", col_tmp, row_tmp);
		ddi_qr_lcd_show_text(col_tmp, row_tmp, w, h, str);
        return 0;
}

void dev_lcd_bl_off(void)
{
    BL_LCD_QR_level(OFF);
}

void dev_lcd_bl_on(void)
{
    BL_LCD_QR_level(ON);
}





