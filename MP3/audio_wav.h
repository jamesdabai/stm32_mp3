#ifndef __AUDIO_WAV_H__
#define __AUDIO_WAV_H__

#include "ff.h"
#include "audio_common.h"

typedef __packed struct//RIFF块
{
	unsigned int ChunkID;//chunk id;这里固定为"RIFF",即0x46464952
	unsigned int ChunkSize ;//集合大小;文件总大小-8
	unsigned int Format;//格式;WAVE,即0x45564157
}ChunkRIFF;

typedef __packed struct//FMT块
{
	unsigned int ChunkID;//chunk id;这里固定为"fmt ",即0x20746D66
	unsigned int ChunkSize ;//子集合大小(不包括ID和Size)
	unsigned short AudioFormat;//音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
	unsigned short NumOfChannels;//通道数量 1表示单声道 2表示双声道
	unsigned int SampleRate;//采样率
	unsigned int ByteRate;//字节速率
	unsigned short BlockAlign;//块对齐(字节)
	unsigned short BitsPerSample;//单个采样数据大小;4位ADPCM,设置为4
}ChunkFMT;

typedef __packed struct//DATA块
{
	unsigned int ChunkID;//chunk id;这里固定为"data",即0x61746164
	unsigned int ChunkSize;//子集合大小(不包括ID和Size)
}ChunkHDR;

typedef __packed struct
{ 
	unsigned short AudioFormat;//音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
	unsigned short nChannels;//通道数量;1,表示单声道;2,表示双声道
	unsigned short BlockAlign;//块对齐(字节)

	unsigned int Bitrate;//比特率(位速)
	unsigned int Samplerate;//采样率
	unsigned short Bps;//位数,比如16bit,24bit,32bit

	unsigned int DataStartOffset;//数据帧开始的位置(在文件里面的偏移)
}Wav_Info;

AudioPlayRes WAV_Play(char* path);

#endif
