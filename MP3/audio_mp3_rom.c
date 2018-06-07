#include <string.h>
#include "audio_mp3_rom.h"
#include "mp3dec.h"
//#include "common.h"

#define BigEndianTab2LittleEndianInt(tab) (((unsigned int)(tab)[0]<<24) + ((unsigned int)(tab)[1]<<16) +((unsigned int)(tab)[2]<<8) + ((tab)[3]))

//获取MP3基本信息
//pname:MP3文件路径
//pctrl:MP3控制信息结构体 
//返回值:0,成功
//    其他,失败
AudioPlayRes MP3_GetInfo_ROM(uint32_t addr,MP3_Info* pMP3info)
{
	MP3FrameInfo frame_info;
	HMP3Decoder decoder;
	MP3_FrameXing* fxing;
	MP3_FrameVBRI* fvbri;
	AudioPlayRes res = AudioPlay_OK;
	
	int offset = 0;//帧同步数据相对数组头的偏移
	unsigned int br;
	short samples_per_frame;//一帧的采样个数
	unsigned int p;//用于识别VBR数据块的指针
	unsigned int filesize;//文件大小
	unsigned int totframes;//总帧数
	uint8_t* readaddr = (uint8_t *)addr;
	
	decoder = MP3InitDecoder();//MP3解码申请内存
	offset = MP3FindSyncWord(readaddr,br);//查找帧同步信息
	if(offset >= 0 && MP3GetNextFrameInfo(decoder,&frame_info,&readaddr[offset])==0)//找到帧同步信息且下一帧信息获取正常	
	{
		p = offset+4+32;//跳过帧头(4bytes)和通道信息(32bytes) 偏移到数据区
		fvbri = (MP3_FrameVBRI*)(readaddr+p);//VBR文件头位于第一个有效帧的数据区
		
		if(!strncmp("VBRI",(char*)fvbri->id,4))//存在VBRI帧
		{
			if(frame_info.version == MPEG1)
			{
				samples_per_frame = 1152;//MPEG1,layer3每帧采样数等于1152
			}
			else
			{
				samples_per_frame = 576;//MPEG2/MPEG2.5,layer3每帧采样数等于576
			}
			
			totframes = BigEndianTab2LittleEndianInt(fvbri->frames);//得到总帧数
			pMP3info->TotalSec = totframes * samples_per_frame / frame_info.samprate;//得到文件总长度 总帧数乘以每帧的采样数除以采样率
		}
		else//不是VBRI帧,尝试是不是Xing帧或者Info帧
		{
			if(frame_info.version==MPEG1)//MPEG1 
			{
				p = (frame_info.nChans==2) ? 32:17;//根据声道数确定偏移量
				samples_per_frame = 1152;//MPEG1,layer3每帧采样数等于1152
			}
			else
			{
				p = (frame_info.nChans==2) ? 17:9;
				samples_per_frame = 576;//MPEG2/MPEG2.5,layer3每帧采样数等于576
			}
			
			p += (offset+4);
			fxing = (MP3_FrameXing*)(readaddr + p);
			
			if(!strncmp("Xing",(char*)fxing->id,4) || !strncmp("Info",(char*)fxing->id,4))//是Xing帧或者Info帧
			{
				if(fxing->flags[3] & 0x01)//存在总帧数字段
				{
					totframes = BigEndianTab2LittleEndianInt(fxing->frames);//得到总帧数
					pMP3info->TotalSec = totframes * samples_per_frame / frame_info.samprate;//得到文件总长度 总帧数乘以每帧的采样数除以采样率
				}
				else//不存在总frames字段
				{
					pMP3info->TotalSec = filesize/(frame_info.bitrate/8);//使用文件大小进行计算
				}
			}
			else//CBR格式,直接计算总播放时间
			{
				pMP3info->TotalSec=filesize/(frame_info.bitrate/8);
			}
		}
		
		pMP3info->Bitrate = frame_info.bitrate;//得到当前帧的码率
		pMP3info->Samplerate = frame_info.samprate;//得到采样率
		pMP3info->Channels = frame_info.nChans;//声道数
		
		if(frame_info.nChans==2)
		{
			pMP3info->SampleSize = frame_info.outputSamps;//输出PCM数据量大小
		}
		else
		{
			pMP3info->SampleSize = frame_info.outputSamps*2;//输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
		}
	}
	else
	{
		res = AudioPlay_UnsupportedFormat;
	}
	
	MP3FreeDecoder(decoder);//释放内存	
	return res;
}

AudioPlayRes MP3_Play_ROM(uint32_t addr)
{
	AudioPlayRes res = AudioPlay_OK;
	HMP3Decoder mp3decoder = NULL;
	uint8_t* readptr = (uint8_t *)addr;//MP3解码读指针
	uint16_t* pOutputBuffer;//指向输出缓冲区的指针
	int32_t offset,bytesleft;//buffer还剩余的有效数据
	MP3_Info MP3Info;
	
	memset(&MP3Info,0,sizeof(MP3_Info));
	
	res = MP3_GetInfo_ROM(addr,&MP3Info);//获取文件信息
		
	if(!res)
	{
		AudioPlayInfo.BufferSize = MP3Info.SampleSize*2;
		
		//if(AudioPlay_I2SConfig(16,MP3Info.Samplerate,MP3Info.SampleSize))//设置I2S
		{
			res = AudioPlay_UnsupportedParameter;
		}
	}
	
	if(res == AudioPlay_OK)
	{
		mp3decoder = MP3InitDecoder();//MP3解码器初始化
		
		Play_Start();
		while(res == AudioPlay_OK)//没有出现数据异常(即可否找到帧同步字符)
		{
			offset = MP3FindSyncWord(readptr,MP3_INPUT_BUFFER_SIZE);//在readptr位置,开始查找同步字符
			
			if(offset < 0)//没有找到同步字符 播放结束也依赖此处跳出循环
			{
				res = AudioPlay_PlayEnd;
				break;//跳出帧解码循环
			}
			
			else//找到同步字符了
			{
				readptr += offset;//MP3读指针偏移到同步字符处
				bytesleft = MP3_INPUT_BUFFER_SIZE;
				
				AudioPlay_PendSem();
				pOutputBuffer = AudioPlay_GetCurrentBuff();//并获得当前空闲缓冲区的地址
				
				if(MP3Decode(mp3decoder,&readptr,&bytesleft,(short*)pOutputBuffer,0))//解码一帧MP3数据 直接解码到I2S的缓冲区里 节约内存
				{
					res = AudioPlay_DamagedFile;//如果解码错误 跳出帧解码循环
				}
				AudioPlay_DataProc(pOutputBuffer,MP3Info.SampleSize);
				
				if(MP3Info.Channels == 1)//如果是单声道
				{
					MonoChannelFill(pOutputBuffer,MP3Info.SampleSize/2);//将数据填充为立体声
				}
			}
		}
	}
	
	Play_Stop();
	
	return res;
}
