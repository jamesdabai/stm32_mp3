#include "spi.h"
//////////////////////////////////////////////////////////////////////////////////	 

//SPI 驱动代码	   
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com										  
////////////////////////////////////////////////////////////////////////////////// 	 


//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI2的初始化
void SPI2_Init(void)//APB1、APB2，后者较为高级一点时钟比较快
{	 
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    //SPI_TypeDef* SPIx = SPI2;
    
  	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI1时钟//时钟没打开所以寄存器没起作用，fuck！！！
                                                        //RCC_APB2PeriphClockCmd--之前
   
    //GPIOFB3,4,5初始化设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PB13~15复用功能输出	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); //PB3复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2); //PB4复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2); //PB5复用为 SPI2
 
	//这里只针对SPI口初始化
	//uart_printf("SR1==%08x\n",SPIx->SR);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);//复位SPI1
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//停止复位SPI1
    //uart_printf("SR2==%08x\n",SPIx->SR);
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;  //设置SPI单向或者双向的数据模式:SPI设置为单线单向发送格式xqy
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//串行同步时钟的第1个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	//SPI2_WriteByte(0xff);//启动传输		 
}   
//SPI1速度设置函数
//SPI速度=fAPB2/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256  
//fAPB2时钟一般为84Mhz：
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
	SPI2->CR1&=0XFFC7;//位3-5清零，用来设置波特率
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); //使能SPI2
} 
//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
void SPI2_WriteByte(u8 TxData)
{		 			 
  
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空 标志为1为空。 
	
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个byte  数据
  //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完一个byte  
	//return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据		    
}


/****************************************************************************
**Description:			向spi2中写入一串数据
**Input parameters:	
**Output parameters:	
**					
**Returned value:		
**					
**Created by:			xqy(2018-1-29)
**--------------------------------------------------------------------------
**Modified by:		
**Modified by:		
****************************************************************************/
void dev_spi_write(u8 *data,u16 len)
{
    u16 i;
    for(i=0;i<len;i++)
    {
        SPI2_WriteByte(*data);
        data++;//xqy指针移到下一个字节
    }
}







