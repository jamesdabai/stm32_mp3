#ifndef __AUDIOPLAY_H__
#define __AUDIOPLAY_H__

//#include "common.h"
#include "ff.h"
#include "stm32f4xx.h"


#define AUDIO_FLAG_INFO_READY (1<<1)//音乐信息结构体更新完毕
#define AUDIO_FLAG_PAUSED (1<<2)//播放被暂停
#define AUDIO_FLAG_ABORT (1<<3)//结束播放线程

#define DAC_Buffer_Size 2304
//2304*32bit 等于是双声道的2304*16bit

typedef enum
{
	AudioPlay_OK = 0,
	AudioPlay_OpenFileError,//无法打开文件
	AudioPlay_ReadFileError,//无法读取文件
	AudioPlay_UnsupportedFormat,//不是音频文件文件
	AudioPlay_UnsupportedParameter,//文件参数不正确
	AudioPlay_DamagedFile,//文件损坏
	
	AudioPlay_MP3_UnsupportedID3v2Version,
	AudioPlay_MP3_NoID3v1Tag,
	AudioPlay_MP3_NoID3v2Tag,
	
	AudioPlay_Next,
	AudioPlay_Prev,
	
	AudioPlay_Exit,
	AudioPlay_PlayEnd,
}AudioPlayRes;

typedef enum
{
	AudioFileType_ERROR,
	AudioFileType_MP3,
	AudioFileType_WAV,
}AudioFileType;

typedef struct
{
	AudioFileType FileType;
	volatile AudioPlayRes PlayRes;
	
	uint16_t TotalSec;
	uint16_t CurrentSec;
	
	uint32_t Bitrate;
	uint32_t Samplerate;
	uint32_t BufferSize;
	uint8_t Channels;
	volatile uint8_t Flags;
}AudioPlay_Info;

extern AudioPlay_Info AudioPlayInfo;
extern FIL AudioFile;
extern uint32_t DualSine12bit[DAC_Buffer_Size];

void AudioPlay_ClearBuf(void);
void AudioPlay_ClearSem(void);
void AudioPlay_PendSem(void);

void AudioPlay_DataProc(uint16_t* buff,uint16_t num);
AudioPlayRes MusicPlayingProcess(void);
void* AudioPlay_GetCurrentBuff(void);
uint8_t AudioPlay_Config(uint8_t Bits,uint32_t SampleRate,uint16_t BufSize);

void AudioPlay_Init(void);
void Play_Start(void);
void Play_Stop(void);
void Audio_StopPlay(void);

void AudioPlayFile(char* path);

#endif
