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
#include "audio_common.h"






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
//图片显示 实验-库函数版本 
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	//tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	return rval;
}


int main(void)
{
    OS_ERR  err;
    u32 total,free;
    s32 ret;

    BSP_Init();////时钟配置，APB配置1:42,2:84Mhz，PLL配置、切换到PLL168Mhz,/* Initialize BSP functions                             */
    CPU_Init();                /* Initialize the uC/CPU services                       */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
    uart_init(115200);//初始化打印串口
	
	uart_printf("enter core\n");
    LED_Init();		        //初始化LED端口
    KEY_Init();//初始化开发板键盘，支持组合按键
	//dev_key_init();         //xqy
	//ddi_key_open();
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
    if(font_init() == 0)
    {
        uart_printf("open font succcse\n");
    }
    else
    {
        uart_printf("open font fail\n");
    }
    LCD_Init(1);//初始化5.0寸大屏
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
    
    OS_ERR      err;
    u32 key;
    u32 ret;
	(void)p_arg;//除编译警告
	delay_init(168);		  //初始化延时函数//系统systick都在这里初始化	
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
        ret = key_read(&key);
        if(key>0)
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
        //drv_key_scan();//按键的IO口和屏的PE有冲突了，所以暂时不用按键
       // dev_key_task();
        KEY_Scan(0); 
        OSTimeDlyHMSM(0u, 0u, 0u, 50u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
    }
}
extern u8 FileList[50][50];
extern AudioPlay_Info AudioPlayInfo;
extern u8* const father;

static  void  MusicTaskStart(void *p_arg)
{
    
    OS_ERR      err;
    u16 num,cnt = 0;
    s32 ret;
    char dir_x[50];
    u8 i;
    (void)p_arg;
    num = ReadDir("music");
    main_printf("进入音乐播放任务\n");
    for(i=0;i<num;i++)
    {
        //main_printf("%d.%s\n",i,(u8 *)FileList[i]);
        lcd_show(i,4,(u8 *)FileList[i],24,1);
        delayms(20);
    }
    music_play_init(num);//初始化歌曲播放控制结构体
    AudioPlay_Init();
	while(1)
	{
	    uart_printf("内存 used= %d\n",my_mem_perused(0));
		sprintf(dir_x,"%s/%s","music",FileList[cnt]);
		AudioPlayFile(dir_x);
		ret = AudioPlayInfo.PlayRes;//一首歌曲播放完成了
		if(ret == AudioPlay_Exit)
		{
		    break;
		}
		else
		{
		    cnt = music_ctl.music_num;
		}
		#if 0 /*xqy 2018-6-18*/
		if(music_ctl.music_num == num-1)
		{
		    Lcd_ColorBox(0,0,480,854,0X07FF);
		    lcd_show_text(0,50,father,24,1);
		}
		if(AudioPlayInfo.PlayRes == AudioPlay_Next)
		{
			cnt ++;
			if(cnt >= num)
				cnt = 0;
		}
		else if(AudioPlay_Prev == AudioPlayInfo.PlayRes)
		{
			if(cnt != 0)
				cnt --;
			else
				cnt = num-1;
		}
		#endif
	}
    OSTaskDel(&MusicTaskStartTCB, &err);
    main_printf("删除音乐任务控制块结束err==%d\n",err);

}

 



