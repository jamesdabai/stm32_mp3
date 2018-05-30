#include "dac.h"
//////////////////////////////////////////////////////////////////////////////////	 

//DAC 驱动代码	   
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com
////////////////////////////////////////////////////////////////////////////////// 	
 

//DAC通道1输出初始化
void Dac1_Init(void)
{  
    GPIO_InitTypeDef  GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitType;
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//使能DAC时钟
	   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

	DAC_InitType.DAC_Trigger=DAC_Trigger_None;	//不使用触发功能 TEN1=0
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_None;//不使用波形发生
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;//屏蔽、幅值设置
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Disable ;	//DAC1输出缓存关闭 BOFF1=1
    DAC_Init(DAC_Channel_1,&DAC_InitType);	 //初始化DAC通道1

	DAC_Cmd(DAC_Channel_1, ENABLE);  //使能DAC通道1
  
    DAC_SetChannel1Data(DAC_Align_12b_R, 0);  //12位右对齐数据格式设置DAC值
}
//设置通道1输出电压
//vol:0~3300,代表0~3.3V
void Dac1_Set_Vol(u16 vol)//直接是16位的采样值
{
	u8 temp[2];
	u16 data;
	temp[0] = vol;
	temp[1] = vol>>8;
	data = ((temp[1]-0x80)<<4)|(temp[0]>>4);//网上抄的16位PCM转换成12位的PCM值得代码，我不知道原理
	//temp = temp>>4;//右移16位，相当于除以65536，将16位数据转化为DAC12位精度的
	
	//temp/=1000;
	//temp=temp*4096/3.3;
	DAC_SetChannel1Data(DAC_Align_12b_R,data);//12位右对齐数据格式设置DAC值
}


//DAC通道1输出初始化
void Dac2_Init(void)
{  
    GPIO_InitTypeDef  GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitType;
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//使能DAC时钟
	   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

	DAC_InitType.DAC_Trigger=DAC_Trigger_None;	//不使用触发功能 TEN1=0
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_None;//不使用波形发生
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;//屏蔽、幅值设置
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Disable ;	//DAC1输出缓存关闭 BOFF1=1
    DAC_Init(DAC_Channel_2,&DAC_InitType);	 //初始化DAC通道1

	DAC_Cmd(DAC_Channel_2, ENABLE);  //使能DAC通道2
  
    DAC_SetChannel2Data(DAC_Align_12b_R, 0);  //12位右对齐数据格式设置DAC值
}
//设置通道2输出电压
//vol:0~3300,代表0~3.3V
void Dac2_Set_Vol(u16 vol)
{
	u8 temp[2];
	u16 data;
	temp[0] = vol;
	temp[1] = vol>>8;
	//data = (s16)vol;
	//data = data>>4;
	//data = data + 2048;
	data = ((temp[1]-0x80)<<4)|(temp[0]>>4);//网上抄的16位PCM转换成12位的PCM值得代码，我不知道原理
	
	DAC_SetChannel2Data(DAC_Align_12b_R,data);//12位右对齐数据格式设置DAC值
}



















































