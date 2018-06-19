#include <string.h>
#include "audio_mp3.h"
#include "mp3dec.h"
//#include "common.h"

#define BigEndianTab2LittleEndianInt(tab) (((unsigned int)(tab)[0]<<24) + ((unsigned int)(tab)[1]<<16) +((unsigned int)(tab)[2]<<8) + ((tab)[3]))

uint8_t MP3_INPUT_BUFFER[MP3_INPUT_BUFFER_SIZE];//输入buffer

/*
将单声道填充为双声道
从中间一个元素开始 将其复制到数组最后面两个
然后被复制的元素向前移动1 复制目标向前移动2
可以防止前面未被复制的数据的损坏
    (i==0)                   (i = size-1)
┌←─┬→─┐        ┌──→→→→──┬───┐
↓	   ↑    ↓        ↑                ↓      ↓
*[0]  ┘  *[1]      *[2]       [3]     [4]     [5]
            ↓        ↑        ↑
            ┕→→──┴────┘
                    (i--)
*/
void MonoChannelFill(uint16_t* buf,uint16_t size)
{
	uint16_t i;
	
	i = size-1;
	
	do
	{
		buf[2*i] = buf[i];//目标元素的第一个数
		buf[2*i + 1] = buf[i];//目标元素的第二个数
		i --;
	}
	while(i);
}

//解析ID3V1
//option:是否写入信息 如果为0只是查找存不存在
AudioPlayRes MP3_GetID3v1(FIL* file,MP3_Info *pctrl,unsigned char option)
{
	ID3V1_Tag *id3v1tag;
	uint32_t br;
	
	f_lseek(file,f_size(file)-128);//偏移到文件结束128字节处
	f_read(file,MP3_INPUT_BUFFER,128-(4+30+1),&br);//读取文件 忽略年代 备注和流派
	
	id3v1tag = (ID3V1_Tag*)MP3_INPUT_BUFFER;
	
	if(strncmp("TAG",(char*)id3v1tag->id,3)==0)//是MP3 ID3V1 TAG
	{
		return AudioPlay_OK;
	}
	else
	{
		return AudioPlay_MP3_NoID3v1Tag;
	}
}

AudioPlayRes MP3_GetID3v2(FIL* file,MP3_Info *pMP3info)
{
	ID3V2_TagHead *taghead;
	uint32_t br;
	uint32_t id3v2size;//帧大小
	
	f_lseek(file,0);//定位到文件开头
	f_read(file,MP3_INPUT_BUFFER,sizeof(ID3V2_TagHead),&br);//读取ID3V2标签头
	taghead = (ID3V2_TagHead*)MP3_INPUT_BUFFER;//解析标签头
	
	if(strncmp("ID3",(char*)taghead->id,3) == 0)//前3个字符为"ID3"
	{
		id3v2size=((u32)taghead->size[0]<<21)|((u32)taghead->size[1]<<14)|((u16)taghead->size[2]<<7)|taghead->size[3];//得到tag大小 4个字节是大端模式 只取低7位
		
		pMP3info->DataStartOffset = id3v2size;//得到mp3数据开始的偏移量
	}
	else
	{
		pMP3info->DataStartOffset = 0;//不存在ID3,mp3数据是从0开始
		return AudioPlay_MP3_NoID3v2Tag;
	}
	
	return AudioPlay_OK;
}

//获取MP3基本信息
//pname:MP3文件路径
//pctrl:MP3控制信息结构体 
//返回值:0,成功
//    其他,失败
AudioPlayRes MP3_GetInfo(FIL* fmp3,MP3_Info* pMP3info)
{
	MP3FrameInfo frame_info;
	HMP3Decoder decoder;
	MP3_FrameXing* fxing;
	MP3_FrameVBRI* fvbri;
	AudioPlayRes res = AudioPlay_OK;
	
	unsigned char mp3_id3_present = 0;//是否存在ID3V2
	int offset = 0;//帧同步数据相对数组头的偏移
	unsigned int br;
	short samples_per_frame;//一帧的采样个数
	unsigned int p;//用于识别VBR数据块的指针
	unsigned int filesize;//文件大小
	unsigned int totframes;//总帧数
	
	filesize = f_size(fmp3);//得到MP3文件大小
	
	if(!MP3_GetID3v2(fmp3,pMP3info))//解析ID3V2数据
	{
		mp3_id3_present = 1;//存在ID3V2
		filesize -= pMP3info->DataStartOffset;//如果存在ID3V2 减去它的大小
	}
	
	if(!MP3_GetID3v1(fmp3,pMP3info,!mp3_id3_present))//解析ID3V1数据 不允许覆盖ID3V2
	{
		filesize -= 128;//如果存在ID3V1 减去它的大小
	}
	
	pMP3info->DataSize = filesize;
	memset(MP3_INPUT_BUFFER,0,MP3_INPUT_BUFFER_SIZE);
	f_lseek(fmp3,pMP3info->DataStartOffset);//偏移到数据开始的地方
	f_read(fmp3,MP3_INPUT_BUFFER,MP3_INPUT_BUFFER_SIZE,&br);//读满缓冲区
	
	decoder = MP3InitDecoder();//MP3解码申请内存
	offset = MP3FindSyncWord(MP3_INPUT_BUFFER,br);//查找帧同步信息
	//uart_printf("offset==%d\n",offset);
	if(offset >= 0 && MP3GetNextFrameInfo(decoder,&frame_info,&MP3_INPUT_BUFFER[offset])==0)//找到帧同步信息且下一帧信息获取正常	
	{
		p = offset+4+32;//跳过帧头(4bytes)和通道信息(32bytes) 偏移到数据区
		fvbri = (MP3_FrameVBRI*)(MP3_INPUT_BUFFER+p);//VBR文件头位于第一个有效帧的数据区
		
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
			fxing = (MP3_FrameXing*)(MP3_INPUT_BUFFER + p);
			
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

AudioPlayRes MP3_Play(char* path)
{
	MP3_Info __MP3Info;
	AudioPlayRes res = AudioPlay_OK;
	HMP3Decoder mp3decoder = NULL;
	MP3FrameInfo mp3frameinfo;
	uint8_t* readptr;//MP3解码读指针
	uint16_t* pOutputBuffer;//指向输出缓冲区的指针
	int32_t offset,bytesleft;//buffer还剩余的有效数据
	UINT br;
	FIL* MP3_File;
	MP3_Info* MP3Info;
	u8 exit_falg = 0;
	
	MP3Info = &__MP3Info;
	MP3_File = &AudioFile;
	memset(MP3Info,0,sizeof(MP3_Info));
	memset(MP3_File,0,sizeof(FIL));
	
	if(f_open(MP3_File,path,FA_READ))//打开文件
	{
		res =  AudioPlay_OpenFileError;//打开文件错误
		uart_printf("打开失败\n");
	}
	else//打开成功
	{
	    uart_printf("打开成功\n");
		res = MP3_GetInfo(MP3_File,MP3Info);//获取文件信息
		
		if(!res)
		{			
			AudioPlayInfo.FileType = AudioFileType_MP3;
			AudioPlayInfo.Channels = MP3Info->Channels;
			
			AudioPlayInfo.TotalSec = MP3Info->TotalSec;
			AudioPlayInfo.Samplerate = MP3Info->Samplerate;
			AudioPlayInfo.Bitrate = MP3Info->Bitrate/1000;
			AudioPlayInfo.BufferSize = MP3Info->SampleSize*2;
			
			AudioPlayInfo.Flags |= AUDIO_FLAG_INFO_READY;
			//AudioPlay_Init();//xqy添加，ADC选择定时器为ADC转换触发源
			if(AudioPlay_Config(16,MP3Info->Samplerate,MP3Info->SampleSize))//设置I2S
			{
			    uart_printf("AudioPlay_UnsupportedParameter\n");
				res = AudioPlay_UnsupportedParameter;
			}
		}
	}
	
	if(res == AudioPlay_OK)
	{
		mp3decoder = MP3InitDecoder();//MP3解码器初始化
		
		f_lseek(MP3_File,MP3Info->DataStartOffset);//跳过文件头中tag信息
		if(f_read(MP3_File,MP3_INPUT_BUFFER,MP3_INPUT_BUFFER_SIZE,&br))//填充满缓冲区
		{
		    uart_printf("read error\n");
			res = AudioPlay_ReadFileError;
		}
		
		bytesleft = br;//读取的数据都是有效数据
		readptr = MP3_INPUT_BUFFER;//MP3读指针指向buffer首地址
		
		Play_Start();
		while(res == AudioPlay_OK)//没有出现数据异常(即可否找到帧同步字符)
		{
			offset = MP3FindSyncWord(readptr,bytesleft);//在readptr位置,开始查找同步字符
			
			if(offset < 0)//没有找到同步字符 播放结束也依赖此处跳出循环
			{
				
				uart_printf("没有找到同步帧\n");
				break;//跳出帧解码循环
			}
			
			else//找到同步字符了
			{
				readptr += offset;//MP3读指针偏移到同步字符处
				bytesleft -= offset;//buffer里面的有效数据个数 帧同步之前的数据视为无效数据减去
				
				MP3Info->CurrentSec = MP3Info->TotalSec * (MP3_File->fptr - MP3Info->DataStartOffset) / MP3Info->DataSize;//计算播放时间
				
				AudioPlayInfo.CurrentSec = MP3Info->CurrentSec;
				
				/*用户代码区*/
				res = MusicPlayingProcess();
				if(res)
				{
				    uart_printf("跳出循环\n");
				    exit_falg=1;
				    break;//提高反应速度
				}
					
				/*用户代码区*/
				
				AudioPlay_PendSem();//等待信号
				pOutputBuffer = AudioPlay_GetCurrentBuff();//并获得当前空闲缓冲区的地址
				
				if(MP3Decode(mp3decoder,&readptr,&bytesleft,(short*)pOutputBuffer,0))//解码一帧MP3数据 直接解码到I2S的缓冲区里 节约内存
				{
				    uart_printf("解码错误\n");
					res = AudioPlay_DamagedFile;//如果解码错误 跳出帧解码循环
				}
				AudioPlay_DataProc(pOutputBuffer,MP3Info->SampleSize);
				
				MP3GetLastFrameInfo(mp3decoder,&mp3frameinfo);//得到刚刚解码的MP3帧信息
				if(MP3Info->Bitrate != mp3frameinfo.bitrate)//更新码率
				{
					MP3Info->Bitrate = mp3frameinfo.bitrate;
					AudioPlayInfo.Bitrate = mp3frameinfo.bitrate;//更新比特率
				}
				
				if(mp3frameinfo.nChans == 1)//如果是单声道
				{
					MonoChannelFill(pOutputBuffer,mp3frameinfo.outputSamps/2);//将数据填充为立体声
				}
				
				if(bytesleft < MAINBUF_SIZE*2)//当数组内容小于2倍MAINBUF_SIZE的时候,必须补充新的数据进来
				{
					memmove(MP3_INPUT_BUFFER,readptr,bytesleft);//将输入数组后面剩余的部分移到前面
					
					if(f_read(MP3_File, MP3_INPUT_BUFFER+bytesleft, MP3_INPUT_BUFFER_SIZE-bytesleft, &br))//补充余下的数据
					{
						res = AudioPlay_ReadFileError;
					}
					
					if(br < MP3_INPUT_BUFFER_SIZE-bytesleft)//如果读取的数据填不满输入数据缓冲区
					{
						memset(MP3_INPUT_BUFFER+bytesleft+br, 0, MP3_INPUT_BUFFER_SIZE-bytesleft-br);//补0
					}
					
					bytesleft = MP3_INPUT_BUFFER_SIZE;//复位读取指针和剩余字节
					readptr = MP3_INPUT_BUFFER;
				}
			}
		}
	}
	if(!exit_falg)
	{
	    res = music_end_process();//一首歌曲播放完成了;
	}
	uart_printf("内存 used= %d\n",my_mem_perused(0));
	MP3FreeDecoder(mp3decoder);	//之前没加，导致内存泄漏了，无法切换歌曲xqy
	Play_Stop();
	f_close(MP3_File);
	
	return res;
}
