


#ifndef _DEV_GPIO_H_
#define _DEV_GPIO_H_

#define MAXIO 62				//分组:数组
#define MAXGROP 20				//分组:数组

#define UNUSED 0
#define USED 1



//Grop used status
#define USED_ALL 0xFFFF		//表示整组IO被占用

//GPIO direction config
#define GPIO_IN 0	//输入
#define GPIO_OUT 1	//输出

//GPIO pull config
#define GPIO_HZ	0	//高阻态
#define GPIO_PD 1	//pulldown
#define GPIO_PU 2	//pullup

//GPIO level config
#define GPIO_LOW 0	//低电平
#define GPIO_HIGH 1	//高电平


//define pin
#define GPIO0 	0
#define GPIO1 	1
#define GPIO2 	2
#define GPIO3 	3
#define GPIO4 	4
#define GPIO5 	5
#define GPIO6 	6
#define GPIO7 	7
#define GPIO8 	8
#define GPIO9 	9

#define GPIO10 10
#define GPIO11 11
#define GPIO12 12
#define GPIO13 13
#define GPIO14 14
#define GPIO15 15
#define GPIO16 16
#define GPIO17 17
#define GPIO18 18
#define GPIO19 19

#define GPIO20 20
#define GPIO21 21
#define GPIO22 22
#define GPIO23 23
#define GPIO24 24
#define GPIO25 25
#define GPIO26 26
#define GPIO27 27
#define GPIO28 28
#define GPIO29 29

#define GPIO30 30
#define GPIO31 31
#define GPIO32 32
#define GPIO33 33
#define GPIO34 34
#define GPIO35 35
#define GPIO36 36
#define GPIO37 37
#define GPIO38 38
#define GPIO39 39

#define GPIO40 40
#define GPIO41 41
#define GPIO42 42
#define GPIO43 43
#define GPIO44 44
#define GPIO45 45
#define GPIO46 46
#define GPIO47 47
#define GPIO48 48
#define GPIO49 49

#define GPIO50 50
#define GPIO51 51
#define GPIO52 52
#define GPIO53 53
#define GPIO54 54
#define GPIO55 55
#define GPIO56 56
#define GPIO57 57
#define GPIO58 58
#define GPIO59 59

#define GPIO60 60
#define GPIO61 61

//define group(gpio func2)
#define GROUP_SPI0 			0		//SPI0
#define GROUP_SPI1 			1		//SPI1
#define GROUP_SPI2  		2		//SPI2
#define GROUP_7816S 		3		//7816slave(func1) gpio(func2)
#define GROUP_7816M0 		4		//7816M0
#define GROUP_7816M1 		5		//7816M1
#define GROUP_7816M2 		6		//7816M2
#define GROUP_UART0 		7		//UART0
#define GROUP_UART1 		8		//UART1
#define GROUP_I2C 			9		//I2C
			
#define GROUP_KEY 			10		//KEY
#define GROUP_PWM0 			11		//PWM0
#define GROUP_PWM1 			12		//PWM1
#define GROUP_CMPRESTULT	13		//CMPRESULT0/CMPRESULT1
#define GROUP_EXTCLK 		14		//EXTCLK
#define GROUP_RST 			15		//RST
#define GROUP_CLKOUT 		16		//CLKOUT
#define GROUP_UART2 		17		//UART2
#define GROUP_SWD 			18		//SWD

typedef struct _group_global
{
	u16 status;
	u8 gpx[10];
}group_global;

typedef union
{
	u32 GpioIntEn[2];		//32位
    u64 GpioIntEn64;		//64位
}int_en;

extern int_en GpioInterruptEn;	//IO中断使能
extern u32 GpioInterruptFun[62];	//中断函数入口指针表
extern group_global group_status[19];

extern void gpio_status_init(void);	//系统启动时调用

extern s32 dev_gpio_open(u8 gpx, u8 pull, u8 dir);
extern s32 dev_gpio_close(u8 gpx);
extern s32 dev_gpio_read(u8 gpx);
extern s32 dev_gpio_write(u8 gpx, u8 level);
extern s32 dev_gpio_set_irq(u8 gpx, u8 type, u32 *func);
extern s32 dev_gpio_free_irq(u8 gpx);
extern s32 dev_gpio_clear_allirq(void);

extern s32 dev_iomutex_getsta(u8 group);
extern s32 dev_iomutex_setsta(u8 group, u16 status);



#endif


