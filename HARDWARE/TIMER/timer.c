#include "timer.h"
#include "led.h"
#include "dac.h"
#include "stm32f4xx_gpio.h"


//////////////////////////////////////////////////////////////////////////////////	 
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com							  
////////////////////////////////////////////////////////////////////////////////// 	 


//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!

short *pcm_buf1;
short *pcm_buf2;// PCM流缓冲，使用两个缓冲区 
short *pcm_buff_temp;
u8  buff_flag=0;
u16 count=0;
extern u8  change_flag;//声明外部变量
u8 PA0_flag = 0;

u8 buff_change = 0;

void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
    TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; //将RCC时钟分频
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断优先级分组//后面自己添加的

    PA0_init();//用于测量时间，示波器
}

void TIM3_Deinit()
{
    TIM_Cmd(TIM3,DISABLE); //关闭定时器3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,DISABLE);  ///关闭TIM3时钟
    return;
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)//通过示波器可以得出周期在22us左右，对应44.1KHZ。
{
	#if 0 /*xqy 2018-3-4*/
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		LED1=!LED1;//DS1翻转
	}
	#endif
	u16 data;

	
	#if 0 /*xqy 2018-3-13*/
	if(PA0_flag)
	{
	    PA0_flag = 0;
	    PA0_out_high();
	}
	else
	{
	    PA0_flag = 1;
	    PA0_out_low();
	}
	#endif
	
	if(buff_flag)
	{
	    data = *(pcm_buf1+count);//低字节在前--高字节在后
	    //data = data<<8;
	    //data += *(pcm_buf1+count);//左声道
	    Dac1_Set_Vol(data);//推出PCM数据

	    data = *(pcm_buf1+count+1);
	    //data = data<<8;
	    //data += *(pcm_buf1+count+2);//右声道
	    Dac2_Set_Vol(data);//推出PCM数据
	    
	    PA0_out_high();
	}
	else
	{
	    data = *(pcm_buf2+count);
	    //data = data<<8;
	    //data += *(pcm_buf2+count);//左声道
	    Dac1_Set_Vol(data);//推出PCM数据

	    data = *(pcm_buf2+count+1);
	    //data = data<<8;
	    //data += *(pcm_buf2+count);//右声道
	    Dac2_Set_Vol(data);//推出PCM数据

	    PA0_out_low();
	}
	count +=2;//向后移动四个字节
	if(count>=2304)//一个PCMbuff完成，切换buff//每帧数据固定1152次采样，每次采样又有左右声道，所以2034个16位数据
	{
	    if(buff_flag)
	    {
	        buff_flag = 0;
	    }
	    else
	    {
            buff_flag = 1;
	    }
	    count = 0;
	    //change_flag--;
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}

void set_buff(short *buff1,short *buff2)
{
    pcm_buf1 = buff1;
    pcm_buf2 = buff2;
    uart_printf("设置好了DAC的buff指针\n");
}

void PA0_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOE时钟

    //GPIOE7~E10初始化设置行输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;//
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
}
void PA0_out_high(void)
{
    GPIO_SetBits(GPIOA,GPIO_Pin_0);
}
void PA0_out_low(void)
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);
}

void PA1_out_high(void)
{
    GPIO_SetBits(GPIOA,GPIO_Pin_1);
}
void PA1_out_low(void)
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

