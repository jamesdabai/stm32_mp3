//#include "common.h"
#include "audio_wav.h"
#include <string.h>

AudioPlayRes WAV_GetInfo(FIL* file,Wav_Info* info)
{
	uint8_t buf[64];
	uint32_t br;
	uint32_t fptr = 0;
	
	ChunkFMT *fmt;
	ChunkHDR *header;
	
	f_lseek(file,0);
	f_read(file,buf,sizeof(buf),&br);//读取32字节
	
	fptr = 12;//最开始是RIFF块 12bytes
	
	if(((ChunkRIFF*)buf)->ChunkID == 0x46464952 && ((ChunkRIFF*)buf)->Format == 0x45564157)//如果块ID为RIFF 格式是WAVE
	{
		fmt = (ChunkFMT*)(buf + 12);//读取FMT块
		if(fmt->ChunkID == 0x20746D66)//如果是FMT块
		{
			info->AudioFormat=fmt->AudioFormat;//音频格式
			info->nChannels=fmt->NumOfChannels;//通道数
			info->Samplerate=fmt->SampleRate;//采样率
			info->Bitrate=fmt->ByteRate*8;//得到位速
			info->BlockAlign=fmt->BlockAlign;//块对齐
			info->Bps=fmt->BitsPerSample;//位数 16/24/32位
			uart_printf("音频格式==%04x\n",info->AudioFormat);
			uart_printf("通道数==%d\n",info->nChannels);
			uart_printf("采样率==%d\n",info->Samplerate);
			uart_printf("得到位速==%d\n",info->Bitrate);
			uart_printf("块对齐==%d\n",info->BlockAlign);
			uart_printf("位数==%d\n",info->Bps);
		}
		else
		{
			return AudioPlay_UnsupportedFormat;//文件格式不是WAVE
		}
		
		fptr += fmt->ChunkSize + 8;//还有文件头
		
		while(1)
		{
			f_lseek(file,fptr);
			f_read(file,buf,sizeof(buf),&br);//读取32字节
			header = (ChunkHDR*) buf;
			
			if(header->ChunkID == 0x61746164)
			{
				info->DataStartOffset = fptr + 8;//跳过头
				break;//找到数据了
			}
			
			fptr += header->ChunkSize + 8;//继续寻找下一个 别忘了文件头的大小
			
			if(fptr > 4096)//找了4k还没有找到
				return AudioPlay_UnsupportedFormat;//文件格式不是WAVE
		}
	}
	else
	{
		return AudioPlay_UnsupportedFormat;//不是RIFF文件
	}
	uart_printf("数据偏移==%d\n",info->DataStartOffset);
	return AudioPlay_OK;
}

AudioPlayRes WAV_Play(char* path)
{
	AudioPlayRes res = AudioPlay_OK;
	Wav_Info __info;
	Wav_Info* WavInfo = NULL;
	FIL* Wav_File = NULL;
	UINT br = 0xFFFF;
	u8 exit_flag=0;
	
	Wav_File = &AudioFile;
	WavInfo = &__info;
	
	if(f_open(Wav_File,path,FA_READ))//打开文件
	{
		res =  AudioPlay_OpenFileError;//打开文件错误
		uart_printf("打开文件错误\n");
	}
	else//打开成功
	{
		res = WAV_GetInfo(Wav_File,WavInfo);//获取文件信息
		
		if(res == AudioPlay_OK)
		{			
			AudioPlayInfo.FileType = AudioFileType_WAV;
			AudioPlayInfo.Channels = WavInfo->nChannels;
			
			AudioPlayInfo.Samplerate = WavInfo->Samplerate;
			AudioPlayInfo.Bitrate = WavInfo->Bitrate/1000;
			AudioPlayInfo.TotalSec = (f_size(Wav_File) - WavInfo->DataStartOffset) / (WavInfo->Bitrate / WavInfo->Bps * WavInfo->nChannels);
			AudioPlayInfo.BufferSize = DAC_Buffer_Size * 2;

			AudioPlayInfo.Flags |= AUDIO_FLAG_INFO_READY;
			//AudioPlay_Init();//xqy添加，ADC选择定时器为ADC转换触发源
			if(AudioPlay_Config(WavInfo->Bps,WavInfo->Samplerate,DAC_Buffer_Size))//配置DMA等数据
			{
				res = AudioPlay_UnsupportedParameter;//数据格式不支持
			}
		}
	}
	
	if(res == AudioPlay_OK)
	{
		f_lseek(Wav_File,WavInfo->DataStartOffset);//定位到PWM数据的开始地方
		//Play_Start();
		Audio_first_play(Wav_File);
	}
	
	while(res == AudioPlay_OK)
	{
		if(br < DAC_Buffer_Size * 2)//读完文件了
		{
			//res = music_end_process();//一首歌曲播放完成了
			uart_printf("播放结束\n");
			break;
		}
		
		AudioPlayInfo.CurrentSec = (Wav_File->fptr - WavInfo->DataStartOffset) / (WavInfo->Bitrate / WavInfo->Bps * WavInfo->nChannels);//计算播放时间
		
		/*用户代码区 开始*/
		res = MusicPlayingProcess();
		if(res)
		{
		    exit_flag = 1;
		    break;
		}
			
		/*用户代码区 结束*/
		
		//uart_printf("time = %d\n",AudioPlayInfo.CurrentSec);
		AudioPlay_PendSem();
		//uart_printf("read file\n");
		f_read(Wav_File,(unsigned char*)AudioPlay_GetCurrentBuff(),DAC_Buffer_Size*4/2,&br);//填充缓冲区
		AudioPlay_DataProc(AudioPlay_GetCurrentBuff(),DAC_Buffer_Size);

	}
	if(!exit_flag)
	{
	    res = music_end_process();//一首歌曲播放完成了;
	}
	uart_printf("内存 used= %d\n",my_mem_perused(0));
	Play_Stop();
	f_close(Wav_File);//关闭文件
	return res;
}
