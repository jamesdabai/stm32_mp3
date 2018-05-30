#include <stdio.h>
//#include "audio_common.h"
#include "audio_mp3.h"
#include "audio_wav.h"
#include "ff.h"
#include <string.h>

FIL AudioFile;
AudioPlay_Info AudioPlayInfo;
uint32_t DualSine12bit[DAC_Buffer_Size];//相当于两个BUFF，前面一个，后面一个，一个buff2304*2个字节
__IO uint8_t DataRequestFlag = 0;

void AudioPlay_DataProc(uint16_t* buff,uint16_t num)
{
	uint16_t i;
	//16位的PWM数据转化为12位的数据，然后将以X轴上移动12位的一半的数值，达到全部电压都是正值+2048
	//xqy
	for(i = 0;i < num;i ++)
	{
		buff[i] = (((int16_t*)buff)[i] / 16 + 2048);
	}
}
void Audio_first_play(FIL* Wav_File)//xqy
{
    u32 br;
    f_read(Wav_File,DualSine12bit,DAC_Buffer_Size*4/2,&br);//填充缓冲区
	AudioPlay_DataProc((void*)DualSine12bit,DAC_Buffer_Size);
	Play_Start();
	f_read(Wav_File,(void*)((uint32_t)DualSine12bit + AudioPlayInfo.BufferSize),DAC_Buffer_Size*4/2,&br);//填充缓冲区
	AudioPlay_DataProc((void*)((uint32_t)DualSine12bit + AudioPlayInfo.BufferSize),DAC_Buffer_Size);
}
void* AudioPlay_GetCurrentBuff(void)
{
	if(DataRequestFlag == 1)//DMA传输完中断则将数据读到第二个buff中，因为马上要传输第一个buff了
	{
		return (void*)((uint32_t)DualSine12bit + AudioPlayInfo.BufferSize);
	}
	else if(DataRequestFlag == 2)//传输一半中断，则将数据读放置到最开始
	{
		return DualSine12bit;
	}
	else
	{
		return NULL;
	}
}

void AudioPlay_PendSem(void)
{
	DataRequestFlag = 0;//传输完成后清除传输完成标志位
	while(DataRequestFlag == 0)//等待传输完成
	{
		__wfi();
	}
}

void AudioPlay_ClearBuf(void)
{
	uint32_t i;
	for(i = 0;i < DAC_Buffer_Size;i ++)
	{
		DualSine12bit[i] = 0x08000800;
	}
}

void AudioPlay_ClearSem(void)
{
	DataRequestFlag = 0;
}
AudioPlayRes MusicPlayingProcess(void)
{
	#if 0 /*xqy 2018-5-17*/
	if(!KEY2)
	{
		Play_Stop();
		delay_ms(20);
		while(!KEY2);
		return AudioPlay_Prev;
	}
	else if(!KEY1)
	{
		Play_Stop();
		delay_ms(20);
		while(!KEY1);
		return AudioPlay_Next;
	}
	#endif
	
	return AudioPlay_OK;
}

uint8_t AudioPlay_Config(uint8_t Bits,uint32_t SampleRate,uint16_t BufSize)
{
	DMA_InitTypeDef            DMA_InitStructure;
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 168000000/SampleRate;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
	TIM_Cmd(TIM2, ENABLE);//设置定时器作为DAC的触发
	
	DMA_DeInit(DMA2_Stream4);//清空之前该stream4上的所有中断标志
	//两个DMA，每个8个数据流，每个数据流有8个通道
    while (DMA_GetCmdStatus(DMA2_Stream4) != DISABLE){}//等待DMA可配置 
    
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(DAC->DHR12RD);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&DualSine12bit;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//内存到外设
	DMA_InitStructure.DMA_BufferSize = BufSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址自动增加屏蔽
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存自动增加打开
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//按字传输2字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//应该是周期模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
  	
	DMA_Init(DMA2_Stream4, &DMA_InitStructure);
	DMA_ITConfig(DMA2_Stream4,DMA_IT_TC|DMA_IT_HT,ENABLE);//DMA_IT_TC|DMA_IT_HT,ENABLE);//设置DMA中断，传输完成中断掩码/传输一半中断掩码
	DMA_Cmd(DMA2_Stream4, ENABLE);
	uart_printf("配置结束\n");
	return 0;
}

AudioFileType Audio_CheckFileExtname(char* path)
{
	char* temp;
	
	temp = strrchr(path,'.');
	
	temp ++;
	
	if(!strcasecmp(temp,"MP3"))
		return AudioFileType_MP3;
	else if(!strcasecmp(temp,"WAV"))
		return AudioFileType_WAV;
	
	return AudioFileType_ERROR;
}

void AudioPlayFile(char* path)
{
	memset(&AudioPlayInfo,0,sizeof(AudioPlay_Info));
	AudioPlay_ClearSem();
	
	switch(Audio_CheckFileExtname(path))//判断后缀名来确定播放的音频文件类型
	{
		case AudioFileType_MP3:
		    uart_printf("MP3播放\n");
			AudioPlayInfo.PlayRes = MP3_Play(path);
			break;
		case AudioFileType_WAV:
		    uart_printf("WAV播放\n");
			AudioPlayInfo.PlayRes = WAV_Play(path);
			break;
		default:
			break;
	}
}

void AudioPlay_Init(void)
{
	NVIC_InitTypeDef				NVIC_InitStructure;
	DAC_InitTypeDef                 DAC_InitStructure;
	GPIO_InitTypeDef				GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//打开外设时钟DMA
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC| RCC_APB1Periph_TIM2, ENABLE);//定时器和DAC时钟
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	
	DAC_StructInit(&DAC_InitStructure);
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;//触发源//定时器触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;//由关闭变成打开
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);//由关闭变为打开
	DAC_SoftwareTriggerCmd(DAC_Channel_2,ENABLE);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn;//设置DMA中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}

void Play_Start(void)
{
	DAC_DMACmd(DAC_Channel_1, ENABLE);
	uart_printf("打开DAC使能\n");
}

void Play_Stop(void)
{
	DAC_DMACmd(DAC_Channel_1, DISABLE);
	DAC->DHR12RD = 0x08000800;
	AudioPlay_ClearBuf();
}

void DMA2_Stream4_IRQHandler(void)
{
    //uart_printf("中断一次\n");
    DataRequestFlag = 1;
	if(DMA_GetITStatus(DMA2_Stream4,DMA_FLAG_TCIF4))//获取DMA2的第四个数据流中的第4通道的传输完成中断标志
    {
        DataRequestFlag = 1;
       // uart_printf("1\n");
    }
	else if(DMA_GetITStatus(DMA2_Stream4,DMA_FLAG_HTIF4))//获取DMA2的第四个数据流中的第4通道的传输一半完成中断标志
	{
        DataRequestFlag = 2;
       // uart_printf("2\n");
    }
	//DMA_ClearITPendingBit(DMA2_Stream4,DMA_IT_TCIF4 | DMA_IT_HTIF4);
	DMA_ClearFlag(DMA2_Stream4,DMA_FLAG_TCIF4 | DMA_FLAG_HTIF4);
}
