#include  <includes.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "stm32f4xx.h"

#include "dev_uart.h"

#include "os_app_hooks.h"
#include "sdio_sdcard.h"
#include "dma.h"
#include "malloc.h" 
#include "exfuns.h"    
#include "dev_lcd.h"
#include "drv_lcd.h"






#define MAIN_DEBUG
//#undef MAIN_DEBUG

#ifdef MAIN_DEBUG
    #define main_printf uart_printf
#else
    #define main_printf //
#endif

static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  void  AppTaskStart          (void     *p_arg);


static  OS_TCB   SystemTaskStartTCB;//跑系统函数的任务，比如键盘
static  CPU_STK  SystemTaskStartStk[SYS_CFG_TASK_START_STK_SIZE];
static  void  SystemTaskStart (void *p_arg);//声明任务

static  OS_TCB   MusicTaskStartTCB;
static  CPU_STK  MusicTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static  void  MusicTaskStart          (void     *p_arg);





static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);



//跑马灯实验 -库函数版本
//STM32F4工程模板-库函数版本
//淘宝店铺：http://mcudev.taobao.com
void show_sdcard_info(void)
{
	switch(SDCardInfo.CardType)
	{
		case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf("Card Type:SDSC V1.1\r\n");break;
		case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf("Card Type:SDSC V2.0\r\n");break;
		case SDIO_HIGH_CAPACITY_SD_CARD:printf("Card Type:SDHC V2.0\r\n");break;
		case SDIO_MULTIMEDIA_CARD:printf("Card Type:MMC Card\r\n");break;
	}	
  	printf("Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);	//制造商ID
 	printf("Card RCA:%d\r\n",SDCardInfo.RCA);								//卡相对地址
	printf("Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));	//显示容量
 	printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);			//显示块大小
}


int main(void)
{
    OS_ERR  err;
    u32 total,free;
    s32 ret;

    BSP_Init();////时钟配置，APB配置1:42,2:84Mhz，PLL配置、切换到PLL168Mhz,/* Initialize BSP functions                             */
    CPU_Init();                /* Initialize the uC/CPU services                       */
    uart_init(115200);//初始化打印串口
    BSP_IntDisAll();          //由汇编代码关掉中断         /* Disable all interrupts.                              */
    LED_Init();		        //初始化LED端口
	dev_key_init();         //xqy
	ddi_key_open();
	my_mem_init(SRAMIN);		//初始化内部内存池  
    while(SD_Init())//检测不到SD卡
	{
		main_printf("SD Card Error!\n");
		delayms(500);					
		main_printf("Please Check! \n");
		delayms(500);	
		LED0=!LED0;//DS0闪烁
	}
	show_sdcard_info();	//打印SD卡相关信息
	exfuns_init();							//为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1);                  //挂载sd卡到fatfs系统中

  	while(exf_getfree("0",&total,&free))	//得到SD卡的总容量和剩余容量
	{
		main_printf("SD Card Fatfs Error!");
		delayms(500);
		LED0=!LED0;//DS0闪烁
	}	
    main_printf("SD Total Size:%d MB\n",(total>>10));
    main_printf("SD  Free Size: %d    MB\n",(free>>10));
    
    #if 0 /*xqy 2018-1-29*/
    ret = f_open(&file, "121/hello.txt", FA_READ | FA_WRITE );
    if(ret==0)
    {
        main_printf("打开成功！");
        printf("该文件长度为%d:\r\n",f_size(&file));
       // ret = f_read(&file, buf, 512, &ret);
        if(ret == 0)
        {
            //printf("SECTOR 0 DATA:\r\n");
			for(sd_size=0;sd_size<512;sd_size++)
			{
			    //printf("%02x ",buf[sd_size]);//打印0扇区数据    	
			    //delay_ms(4);
			    OSTimeDlyHMSM(0u, 0u, 0u, 4u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
			}
			//printf("\r\nDATA ENDED\r\n");
			//delay_ms(4);
        }
    }
    f_close(&file);
    #endif
	dev_QR_lcd_open();//打开lcd屏
	main_printf("打开lcd屏\n");
	ddi_qr_lcd_show_text_ext(1,1,"已经打开lcd屏",CDISP);
	
    OSInit(&err);    /* Init uC/OS-III.xqy会将hook函数指针清零                                      */
    main_printf("初始化hook函数\n");
    //App_OS_SetAllHooks();
    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
    main_printf("开始任务\n");
    (void)&err;

    return (0u);
}



static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;
    u8 sz,i;
    u32 key;
    u32 ret;
    
    FIL file;          //文件1
    u8  buf[512];
    u32 sd_size;

    (void)p_arg;
    delay_init(168);		  //初始化延时函数
    main_printf("LEDapp任务\n");
    //cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */
    //cnts         = cpu_clk_freq                                 /* Determine nbr SysTick increments                     */
   //              / (CPU_INT32U)OSCfg_TickRate_Hz;               //1ms
   // OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

    //Mem_Init();                                                 /* Initialize memory managment module                   */
    //Math_Init();                                                /* Initialize mathematical module*/

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    printf("\r\n正在创建系统任务\r\n");
    OSTaskCreate((OS_TCB       *)&SystemTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"System Task Start",
                 (OS_TASK_PTR   )SystemTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&SystemTaskStartStk[0u],
                 (CPU_STK_SIZE  )SystemTaskStartStk[SYS_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )SYS_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
    printf("\r\n创建系统任务结束\r\n");
    
    printf("\r\n正在创建音乐播放任务\r\n");
    OSTaskCreate((OS_TCB       *)&MusicTaskStartTCB,              /* Create the start task                                */
                (CPU_CHAR     *)"mp3 Task Start",
                (OS_TASK_PTR   )MusicTaskStart,
                (void         *)0u,
                (OS_PRIO       )MP3_CFG_TASK_START_PRIO,
                (CPU_STK      *)&MusicTaskStartStk[0u],
                (CPU_STK_SIZE  )MusicTaskStartStk[SYS_CFG_TASK_START_STK_SIZE / 10u],
                (CPU_STK_SIZE  )SYS_CFG_TASK_START_STK_SIZE,
                (OS_MSG_QTY    )0u,
                (OS_TICK       )0u,
                (void         *)0u,
                (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR       *)&err);
    printf("\r\n创建音乐播放任务结束\r\n");
    
    while(1)
	{
    	GPIO_ResetBits(GPIOA,GPIO_Pin_6);  //LED0对应引脚GPIOA.6拉低，亮  等同LED0=0;
    	GPIO_SetBits(GPIOA,GPIO_Pin_7);   //LED1对应引脚GPIOA.7拉高，灭 等同LED1=1;
    	OSTimeDlyHMSM(0u, 0u, 0u, 1000u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
    	GPIO_SetBits(GPIOA,GPIO_Pin_6);	   //LED0对应引脚GPIOA.6拉高，灭  等同LED0=1;
    	GPIO_ResetBits(GPIOA,GPIO_Pin_7); //LED1对应引脚GPIOA.7拉低，亮 等同LED1=0;
    	OSTimeDlyHMSM(0u, 0u, 0u, 1000u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        ret = ddi_key_read(&key);
        //main_printf("ret=%d\n",ret);
        if(ret>0)
        {   
            disp_keyvalue(key);
        }
	}

}

static  void  SystemTaskStart (void *p_arg)
{
    OS_ERR      err;
    (void)p_arg;
    
    main_printf("进入系统键盘扫描任务\n");
    while(1)
    {
        drv_key_scan();
        dev_key_task();
        OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
    }
}
static  void  MusicTaskStart(void *p_arg)
{
    OS_ERR      err;
    (void)p_arg;
    
    main_printf("进入音乐播放任务\n");
    
    mp3_play_song("music/无所谓 (Live).mp3");
    
    main_printf("正在删除音乐任务控制块！！\n");
    OSTaskDel(&MusicTaskStartTCB, &err);
    main_printf("删除音乐任务控制块结束err==%d\n",err);

}

 



