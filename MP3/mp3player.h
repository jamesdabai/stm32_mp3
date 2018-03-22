#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"

#include "delay.h"
#include "led.h"
//#include "key.h"	 
#include "malloc.h"
#include "string.h" 
#include "ff.h"   
#include "usart.h"	
//#include "iis.h"
#include "mp3dec.h"


u8 mp3_play_song(u8 *pname);
typedef struct  
{
    char FrameID[4];
    char str[30];
}ID3V2;
typedef struct  
{
    char Frame[4];
    char strlen[4];
    char hello[2];
}ID3V2_HEAD;


int MP3_ID3V2_disp(char *read_ptr);

s32  memcmp_x(u8 *d,u8 *s,int len);
void memcpy_x(u8 *d,u8 *s,int len);

extern u8 buff_change;


#endif












