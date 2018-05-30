#ifndef __AUDIO_MP3_H__ 
#define __AUDIO_MP3_H__

#include "ff.h"
#include "audio_common.h"

#define MP3_INPUT_BUFFER_SIZE 4096//MP3解码时,文件buf大小

typedef __packed struct//ID3V1 标签
{
	unsigned char id[3];			//ID,TAG三个字母
	unsigned char title[30];	//歌曲名字
	unsigned char artist[30];	//艺术家名字
	unsigned char album[30];	//专辑名名字
	unsigned char year[4];		//年代
	unsigned char comment[30];	//备注
	unsigned char genre;			//流派 
}ID3V1_Tag;

typedef __packed struct//ID3V2 标签头
{
    unsigned char id[3];		//ID
    unsigned char mversion;	//主版本号
    unsigned char sversion;	//子版本号
    unsigned char flags;		//标签头标志
    unsigned char size[4];		//标签信息大小(不包含标签头10字节) 标签大小=size+10.
}ID3V2_TagHead;

//ID3V2.3 版本帧头
typedef __packed struct 
{
	unsigned char id[4];			//帧ID
	unsigned char size[4];		//帧大小
	unsigned char flags[2];		//帧标志
}ID3V23_TagFrameHead;

typedef __packed struct//MP3 Xing帧信息(没有全部列出来,仅列出有用的部分)
{
	unsigned char id[4];			//帧ID,为Xing/Info
	unsigned char flags[4];		//存放标志
	unsigned char frames[4];	//总帧数
	unsigned char fsize[4];		//文件总大小(不包含ID3)
}MP3_FrameXing;
 
typedef __packed struct//MP3 VBRI帧信息(没有全部列出来,仅列出有用的部分)
{
    unsigned char id[4];		//帧ID,为Xing/Info
	unsigned char version[2];	//版本号
	unsigned char delay[2];		//延迟
	unsigned char quality[2];	//音频质量,0~100,越大质量越好
	unsigned char fsize[4];		//文件总大小
	unsigned char frames[4];	//文件总帧数 
}MP3_FrameVBRI;

typedef __packed struct//MP3控制结构体
{
	unsigned char Channels;				//声道数
	unsigned int TotalSec;				//整首歌时长,单位:秒
	unsigned int CurrentSec;			//当前播放时长
	unsigned int Bitrate;	   		//比特率
	unsigned int Samplerate;			//采样率
	unsigned short SampleSize;			//PCM输出数据量大小(以16位为单位),单声道MP3,则等于实际输出*2(方便DAC输出)
	unsigned int DataStartOffset;		//数据帧开始的位置(在文件里面的偏移)
	unsigned int DataSize;				//MP3除去各种信息以后的数据量
}MP3_Info;

void MonoChannelFill(uint16_t* buf,uint16_t size);
AudioPlayRes MP3_Play(char* path);

#endif
