#include "string.h"
#include "dev_gpio.h"
#include "gpio.h"


group_global group_status[19];


//给gpx分组
const u8 get_gpio_grop[MAXIO] = {
	0,0,0,0,0,0,		//gpio0-gpio5
	1,1,1,1,			//gpio6-gpio9
	2,2,2,2,			//gpio10-gpio13
	3,3,3,				//gpio14-gpio16
	4,4,4,4,4,			//gpio17-gpio21
	5,5,5,				//gpio22-gpio24
	6,6,6,				//gpio25-gpio27
	7,7,				//gpio28-gpio29
	8,8,				//gpio30-gpio31
	9,9,				//gpio32-gpio33
	10,10,10,10,10,10,10,10,10,10,		//gpio34-gpio43
	11,11,				//gpio44-gpio45
	12,12,				//gpio46-gpio47
	13,13,				//gpio48-gpio49
	14,					//gpio50
	15,					//gpio51
	16,					//gpio52
	17,17,  			//gpio53-gpio54
	18,                 ////gpio55
	19,19,19,19,19,19	//gpio56-gpio61
};

const u8 get_grop_ionum[MAXGROP+1] = {
	0,				//gpio0-gpio5
	6,				//gpio6-gpio9
	10,				//gpio10-gpio13
	14,				//gpio14-gpio16
	17,				//gpio17-gpio21
	22,				//gpio22-gpio24
	25,				//gpio25-gpio27
	28,				//gpio28-gpio29
	30,				//gpio30-gpio31
	32,				//gpio32-gpio33
	34,				//gpio34-gpio43
	44,				//gpio44-gpio45
	46,				//gpio46-gpio47
	48,				//gpio48-gpio49
	50,				//gpio50
	51,				//gpio51
	52,				//gpio52
	53,				//gpio53-gpio54
	55,
	56,				//gpio56-gpio61
	62				//end,不存在此IO，仅为了方便计算最后一组的IO个数
};

u32 GpioInterruptFun[62];	//中断函数入口表

int_en GpioInterruptEn;

/******************************************************************
*** 函 数 名:   void gpio_status_init(void)
*** 功能描述：  上电初始化GPIO状态
*** 参    数:   	
*** 返    回：   成功返回0
***	                否则-1
******************************************************************/	
void gpio_status_init(void)
{
	memset(group_status, 0, sizeof(group_status));	//初始化所有组和IO状态为未使用状态
	
	memset(GpioInterruptFun, 0, sizeof(GpioInterruptFun));	//初始化所有入口指针为空
	
	GpioInterruptEn.GpioIntEn64 = 0;	//所有GPIO口中断禁止
	
	gpioClearAllExtInt();	//清除所有中断状态
}

/******************************************************************
*** 函 数 名:   s32 dev_iomutex_getsta(void)
*** 功能描述：  获取某组的GPIO状态
*** 参    数:   	
*** 返    回：   返回0表示未占用
***	             返回1表示已占用
******************************************************************/
s32 dev_iomutex_getsta(u8 group)
{
	u8 cnt, i;
	u8 status=0;
	//0.数量判断
	if(group>=MAXGROP)
	{
		return -1;		
	}
	
	//1.此组有几个IO口
	cnt = get_grop_ionum[group+1]-get_grop_ionum[group];
	
	//2.检索此组IO有没有被占用的
	for(i=0; i<cnt; i++)
	{
		status += group_status[group].gpx[i];
	}
	//3.返回状态
	if(status || group_status[group].status)
	{
		return USED;
	}
	else
	{
		return UNUSED;
	}
}

/******************************************************************
*** 函 数 名:   s32 dev_iomutex_setsta(u8 group, u16 status)
*** 功能描述：  设置某组的GPIO中各引脚的状态，每1bit对应一个IO，
***              gpio0-gpio5对应bit0-bit5，如整组都占用则可设置为0xFFFF
*** 参    数:   	
*** 返    回：   返回0表示未占用
***	             返回1表示已占用
******************************************************************/
s32 dev_iomutex_setsta(u8 group, u16 status)
{
	//0.数量判断
	if(group>=MAXGROP)
	{
		return -1;		
	}
	
	//1.设置此组中的对应的引脚状态，已用的脚对应bit设置为1，未用的脚对应bit为0；
	group_status[group].status = status;
	
	//2.返回状态
	return SUCCESS;
}

/******************************************************************
*** 函 数 名:   s32 dev_gpio_open(u8 gpx, u8 pull, u8 dir)
*** 功能描述：  打开引脚
*** 参    数:   	输入: 	u8 gpx, 引脚
*** 						u8 pull, 上下拉
***						u8 dir, 方向
***						u32 *func,是否设置中断
***     	         成功返回0
***	                否则-1
******************************************************************/
s32 dev_gpio_open(u8 gpx, u8 pull, u8 dir)
{
	u8 group,grp_io;

	//0.数量判断
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//1.组判断；引脚判断
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//第group的第grp_io个

	//if(/*USED == */group_status[group].status & (0x1<<grp_io))	//检查此IO是否被组占用，此处考虑到SPI的片选需要属于SPI组，但是可以单独做IO操作
	if(/*USED == */group_status[group].status)	//THM3100是只要设置为非IO功能，则此组的IO都不能被使用
	{
		return -1;
	}

	if(/*USED == */group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	//2.设置GPIO/FUNCTION
	gpioSetGropProp(group, GROP_GPIO);

	//3.设置方向
	if(GPIO_IN == dir)
	{
		gpioSetDirIn(gpx);
	}
	else
	{
		gpioSetDirOut(gpx);
		
	}
	
	//4.设置上下拉
	if(pull == GPIO_PD)
	{
		/*set pull-down*/
		gpioSetPullDownBit(gpx);
		/*reset pull-up*/
		gpioResetPullUpBit(gpx);
	}
	else if(pull == GPIO_PU)
	{
		/*set pull-up*/
		gpioSetPullUpBit(gpx);
		/*reset pull-down*/
		gpioResetPullDownBit(gpx);
	}
	else//高阻
	{
		/*reset pull-up*/
		gpioResetPullUpBit(gpx);
		/*reset pull-down*/
		gpioResetPullDownBit(gpx);
	}	

	//4.引脚互斥
	group_status[group].gpx[grp_io] = USED;
	
#if 0	//做GPIO使用时，不需要设置组状态，否则导致同组IO中的另一个IO无法设置，做复用功能时才需要设置状态。
	group_status[group].status = USED;
#endif
	
	return SUCCESS;
}

/******************************************************************
*** 函 数 名:   s32 dev_gpio_close(u8 gpx)
*** 功能描述：  关闭引脚
*** 参    数:   	输入: 	u8 gpx, 引脚
***     	         成功返回0
***	                否则-1
******************************************************************/
s32 dev_gpio_close(u8 gpx)
{
	u8 group,grp_io;
	
	//1.数量判断
	if(gpx>=MAXIO)
	{
		return -1;		
	}

	//2.判断此端口是否打开，打开才需要关闭
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//第group的第grp_io个
	if(UNUSED == group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	gpioSetDirIn(gpx);
	//gpioResetBit(gpx);
	/*set pull-up*/
	gpioSetPullUpBit(gpx);
	/*reset pull-down*/
	gpioResetPullDownBit(gpx);
	
	//3.关闭成功的情况下，将占有的IO释放为unused。
	group_status[group].gpx[grp_io] = UNUSED;
	group_status[group].status = UNUSED;
	
	return SUCCESS;
}

/******************************************************************
*** 函 数 名:   s32 dev_gpio_read(u8 gpx)
*** 功能描述：  读引脚值
*** 参    数:   	输入: 	u8 gpx, 引脚
***     	         成功返回0
***	                否则-1
******************************************************************/
s32 dev_gpio_read(u8 gpx)
{
	u8 ret;
	
	//1.数量判断
	if(gpx>=MAXIO)
	{
		return -1;		
	}

	//2.判断此端口是否打开，打开才允许操作
	
	//3.读取数据
	ret = gpioReadInputDataBit(gpx);
	
	return ret;
}

/******************************************************************
*** 函 数 名:   s32 dev_gpio_write(u8 gpx, u8 level)
*** 功能描述：  给引脚赋值
*** 参    数:   	输入: 	u8 gpx, 引脚
***						u8 level,电平
***     	         成功返回0
***	                否则-1
******************************************************************/
s32 dev_gpio_write(u8 gpx, u8 level)
{
	//1.数量判断
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//2.判断此端口是否打开，打开才允许操作
	
	//3.设置端口状态
	if(GPIO_LOW == level)
	{
		gpioResetBit(gpx);
	}
	else
	{
		gpioSetBit(gpx);
	}
	
	return SUCCESS;
}

static void GpioSetCallbackFunction(u8 gpx, u32 *func)
{
	s64 st_bit=0x1;
	st_bit <<= gpx;
	GpioInterruptFun[gpx] = (u32)func;

	if(func==NULL)
	{
		GpioInterruptEn.GpioIntEn64 &= ~st_bit;
	}
	else
	{
		GpioInterruptEn.GpioIntEn64  |= st_bit;
	}
}

/******************************************************************
*** 函 数 名:   s32 dev_gpio_set_irq(u8 gpx, u8 type, u32 *func)
*** 功能描述：  给引脚赋值
*** 参    数:   	输入: 	u8 gpx, 引脚
***							u8 type, 触发类型：0-低电平触发，1-高电平触发，2-下降沿触发，3-上升沿触发
***                         u32 *func, 中断的回调函数
***     	         成功返回0
***	                否则-1
******************************************************************/
s32 dev_gpio_set_irq(u8 gpx, u8 type, u32 *func)
{
	u8 group,grp_io;
	//1.数量判断
	if(gpx>=MAXIO)
	{
		return -1;		
	}
	
	//2.判断此端口是否打开，打开且为input才允许设置为中断
	group = get_gpio_grop[gpx];
	grp_io = gpx - get_grop_ionum[group];//第group的第grp_io个
	if(UNUSED == group_status[group].gpx[grp_io])
	{
		return -1;
	}
	
	//3.判断端口已设置为输入
	if(GPIO_OUT==gpioGetDir(gpx))
	{
		return -1;
	}
	
	//输入可产生中断
	if((func == NULL)||(type > 3))
	{
		return -1;
	}
	else
	{
		//设置中断
		gpioSetExtInt(gpx, type);
		//设置中断函数
		GpioSetCallbackFunction(gpx, func);

		return SUCCESS;
	}
	
}

s32 dev_gpio_free_irq(u8 gpx)
{
	//释放中断
	gpioClrExtInt(gpx);
	//清中断函数
	GpioSetCallbackFunction(gpx, NULL);
	
	return SUCCESS;
}

