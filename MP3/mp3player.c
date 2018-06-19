#include "mp3player.h" 

#define DIR_MAX_ITEM 50  //128
#define MUSIC_DEFAULT_DIR "music"
    
//FATFS fatfs;
char FileList[DIR_MAX_ITEM][50];
const char* MUSIC_EXTNAME_FILTER[] = {"MP3","WAV",""};
    
extern uint8_t buf_fin;
u8 change_flag = 0;//每当有一个buff被推送完成时就开始读数据并转化

static const  ID3V2 id3v2[] =
{
    "TEXT", "歌词作者",
    "TENC", "编码",
    "WXXX", "URL链接(URL)",
    "TCOP", "版权(Copyright)",
    "TOPE", "原艺术家",
    "TCOM", " 作曲家",
    "TDAT", "日期",
    "TPE3", "指挥者",
    "TPE2", "乐队",
    "TPE1", "艺术家相当于ID3v1的Artist",
    "TPE4", "翻译（记录员、修改员）",
    "TYER", "年代相当于ID3v1的Year",
    "USLT", "歌词",
    "TALB", "专辑相当于ID3v1的Album",
    "TIT1", "内容组描述",
    "TIT2", "标题相当于ID3v1的Title",
    "TIT3", "副标题",
    "TCON", "流派（风格）",
    "TBPM", "每分钟节拍数",
    "COMM", "注释相当于ID3v1的Comment",
    "TDLY", "播放列表返录",
    "TRCK", "音轨（曲号）",
    "TFLT", "编码",
    "TENC", "文件类型",
    "TIME", "时间　",
    "TKEY", "最初关键字",
    "TLAN", "语言",
    "TLEN", "长度",
    "TMED", "媒体类型",
    "TOAL", "原唱片集",
    "TOFN", "原文件名",
    "TOLY", "原歌词作者",
    "TORY", "文件所有者（许可证者）",
    "TPOS", "作品集部分",
    "TPUB", "发行人",
    "TRDA", "录制日期",
    "TRSN", "Intenet电台名称",
    "TRSO", "Intenet电台所有者",
    "TSIZ", "大小",
    "TSRC", "ISRC（国际的标准记录代码）",
    "TSSE", "编码使用的软件（硬件设置）",
    "UFID", "唯一的文件标识符",
    "TENC", "编码",
    "AENC", "音频加密技术",
    "TXXX", "保存文本数据的",
    "APIC", "保存小图片的标签"
};

u8 CheckExtName(const char* path,const char** extname)
{
    uint8_t i = 0;
    char* ch = strrchr(path,'.');
    
    if(ch == NULL)
        return 0;
    
    if(*extname[0] == '\0')
        return 1;//未指定扩展名 均返回真
    
    while(*extname[i] != '\0')
    {
        if(!strncasecmp(ch+1,extname[i],strlen(extname[i])))
            return 1;
        i ++;
    }
    
    return 0;
}

//struct ID3V2_HEAD ID3V2_head;
unsigned char ReadDir(char* dir_str)
{
	uint16_t cnt = 0;
	DIR dir;
	FILINFO fileinfo;
	FRESULT res;
	
	#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = (u8 *)mymalloc(fileinfo.lfsize);
    #endif 
	
	f_opendir(&dir,dir_str);
	
	res = f_readdir(&dir,&fileinfo);

	while((fileinfo.fname[0] && res == FR_OK))
	{
		if(fileinfo.fattrib & AM_DIR)
			goto NEXT;
		
		if(!CheckExtName(fileinfo.fname,MUSIC_EXTNAME_FILTER))
			goto NEXT;
		//uart_printf("%s,len=%d\n",fileinfo.lfname,fileinfo.lfsize);
		Str_Copy(&FileList[cnt][0],fileinfo.lfname);
		
		cnt ++;
		if (cnt >= DIR_MAX_ITEM)
			break;
		NEXT:
		res = f_readdir(&dir,&fileinfo);
	}
	myfree(fileinfo.lfname);
	f_closedir(&dir);
	
	return cnt;
}



/**
  * @brief  mp3_player 进行mp3文件解码、播放
  * @param  filename:要播放的文件路径
  * @retval key
  */
u8 mp3_play_song(u8 *pname)
{
    #define read_buf_size 4096
    #define pcm_buf_each_size 4608	
//	/* MP3解码时用到的变量 */	
	int err, i, current_sample_rate = 0;	

    int				read_offset = 0;				/* 读偏移指针			*/
    int				bytes_left = 0;					/* 剩余字节数			*/	
	unsigned long	Frames = 0;						/* mP3帧计数			*/
	unsigned char	*read_ptr;						/* 缓冲区指针			*/
	HMP3Decoder		Mp3Decoder;						/* mp3解码器指针	  	*/	

	uint8_t bufflag=0;										//用于切换buffer的标志
	static MP3FrameInfo		Mp3FrameInfo;		//mP3帧信息      

	FIL* fmp3;
	u8 res,rval=0;
	u16 br;
	u8 key;
	u8 *data_read_buf;// read文件缓冲区
	short *data_pcm_buf;
	short *data_pcm_buf2;// PCM流缓冲，使用两个缓冲区 
	char  *head_size;

	int frame_len;
	
	fmp3=(FIL*)mymalloc(sizeof(FIL));//申请内存
	data_read_buf=(u8*)mymalloc(read_buf_size);		//开辟内存区域
	data_pcm_buf=(short*)mymalloc(pcm_buf_each_size);
	data_pcm_buf2=(short*)mymalloc(pcm_buf_each_size);
	
	if(data_read_buf==NULL||data_pcm_buf==NULL||data_pcm_buf2==NULL||fmp3==NULL)
	{
	    return 0XFF;//内存申请失败.
	}
	    	
	res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	
	printf("打开%s文件==%d \r\n",pname,res);
	if (res!=FR_OK)
	{
		printf("open file %s error \r\n",pname);	
		myfree(fmp3);			
		myfree(data_read_buf);	
		myfree(data_pcm_buf);	
		myfree(data_pcm_buf2);
		return 1;										//文件无法打开，终止解码。进入下一次循环，读取下一个文件
	}

	res = f_read(fmp3,data_read_buf,read_buf_size,(UINT*)&br);//读出4096个字节
	printf("第一次读出 %d 字节 \r\n",br);
	
	//printf_format(data_read_buf,160);//打印数据
	if (res!=FR_OK)
	{
		printf("read file %s error \r\n",pname);	
		myfree(fmp3);			
		myfree(data_read_buf);	
		myfree(data_pcm_buf);	
		myfree(data_pcm_buf2);
		return 1;			//文件无法打开，终止解码。进入下一次循环，读取下一个文件
	}
	
	//打开成功
	//初始化MP3解码器
	Mp3Decoder = MP3InitDecoder();//一个数据组织的结构体，动态申请内存xqy
	if (Mp3Decoder==0)
	{
		printf("ERR_MP3_Allocate Buffers\r\n");
		myfree(fmp3);			
		myfree(data_read_buf);	
		myfree(data_pcm_buf);	
		myfree(data_pcm_buf2);
		return 1;										
	}	
	printf("memory_used_mp3_play:%d%%\r\n",my_mem_perused(0));
	read_ptr = data_read_buf;									//指向mp3输入流
	head_size = data_read_buf;//帧头用来算长度的指针
	//bytes_left = br;								//实际读到的输入流大小大小

	frame_len = (head_size[0+6]&0x7f)*0x200000+(head_size[1+6]&0x7f)*0x4000+(head_size[2+6]&0x7f)*0x80+(head_size[3+6]&0x7f);
    uart_printf("帧头长度为%d\n",frame_len+10);
    
	MP3_ID3V2_disp(read_ptr);//打印ID3V2标签帧的信息
    uart_printf("通过标签帧算得到的长度为%d\n",frame_len+10);

    f_lseek(fmp3, frame_len+10);//xqy定位到真正的数据处
    uart_printf("当时的文件指针为%d\n",f_tell(fmp3));
    
	res = f_read(fmp3,data_read_buf,read_buf_size,(UINT*)&br);//读出4096个字节//再次读出数据供解析
	read_ptr = data_read_buf;
	bytes_left = br;	
	
	while(rval==0)
	{
	   // uart_printf("%d\n",change_flag);
	    //if(change_flag>=2)
	       // continue;//当数据还没有推送完时，暂时不用往下面解析。
		read_offset = MP3FindSyncWord(read_ptr, bytes_left);	//寻找帧同步，返回第一个同步字的位置//xqyFF FE是每个数据帧的开头
		//printf("寻找同步帧\r\n");
		//printf("read_offset===%d\r\n",read_offset);
		if(read_offset < 0)																		//没有找到同步字
		{
		    //printf("没有找到FF FE的同步帧\r\n");
			rval=1;
			break;																							//跳出循环2，回到循环1	
		}
		read_ptr += read_offset;					//偏移至同步字的位置
		bytes_left -= read_offset;				//同步字之后的数据大小	
		if(bytes_left < 1024)							//补充数据
		{
		    //补充数据，确保buff里面的音频待解析数据大于1024字节xqy
			/* 注意这个地方因为采用的是DMA读取，所以一定要4字节对齐  */ //文件系统用DMA读取，所以最后字节对齐
			i=(uint32_t)(bytes_left)&3;					
			//判断多余的字节
			if(i) i=4-i;														//需要补充的字节
			memcpy(data_read_buf+i, read_ptr, bytes_left);	//从对齐位置开始复制
			read_ptr = data_read_buf+i;										//指向数据对齐位置
			//四字节对齐是为了文件系统读的时候，读出的数据4字节对齐
			res = f_read(fmp3, data_read_buf+bytes_left+i, read_buf_size-(bytes_left+i), (UINT*)&br);//补充数据
			//printf("补充数据\r\n");
			bytes_left += br;										//有效数据流大小
		}	

		//uart_printf("文件指针=%d\n",f_tell(fmp3));
		if(bufflag)
		{
		    PA1_out_high();
			err = MP3Decode(Mp3Decoder, &read_ptr, &bytes_left, data_pcm_buf, 0);					//开始解码 参数：mp3解码结构体、输入流指针、输入流大小、输出流指针、数据格式
            PA1_out_low();
			//printf("第一个缓冲区解码\r\n");
			//printf_format(data_pcm_buf,4608);//打印数据
			//printf("剩余字节数=%d\r\n",bytes_left);
			change_flag++;
			
		}
		else
		{
		    PA1_out_high();
			err = MP3Decode(Mp3Decoder, &read_ptr, &bytes_left, data_pcm_buf2, 0);	
			PA1_out_low();
			//printf("第二个缓冲区解码\r\n");
			//printf_format(data_pcm_buf2,4608);//打印数据
			//printf("剩余字节数=%d\r\n",bytes_left);
			change_flag++;
		}
		if(Frames==0)
		{
		    //printf("第一帧，正在初始化DMA\r\n");
		    set_buff(data_pcm_buf,data_pcm_buf2);//将储存PCM数据的指针地址赋值给DAC可以操作的全局变量
		    Dac1_Init();
		    Dac2_Init();//初始化DAC
		    TIM3_Int_Init(1,7619-1);//1906/2-1);//7619-1);//初始化定时器44.1Khz//降到22.05khz
		    
			//DMA_Double_Buf_Init((uint32_t)data_pcm_buf,(uint32_t)data_pcm_buf2,pcm_buf_each_size/2);//在第一侦位置，初始化DMA
		}
		Frames++;
		if (err != ERR_MP3_NONE)									//错误处理
		{
		    printf("错误处理\r\n");
		    printf("return==%d\r\n",err);
			switch (err)
			{
				case ERR_MP3_INDATA_UNDERFLOW:
					printf("ERR_MP3_INDATA_UNDERFLOW\r\n");
					read_ptr = data_read_buf;
					res = f_read(fmp3, data_read_buf, read_buf_size, (UINT*)&br);
					bytes_left = br;
					break;
		
				case ERR_MP3_MAINDATA_UNDERFLOW:
					rval=1;	
					/* do nothing - next call to decode will provide more mainData */
					/*什么都不做 - 接下来调用解码会提供更多的mainData */
					printf("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
					break;
		
				default:
					rval=1;	
					printf("UNKNOWN ERROR:%d\r\n", err);
		
					// 跳过此帧
					if (bytes_left > 0)
					{
						bytes_left --;
						read_ptr ++;
					}	
					break;
			}
		}
		else
		{
				MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);		//获取解码信息		
				//printf("MP3解析成功\r\n", err);
				//printf("获取解码信息\r\n", err);

		    /* 根据解码信息设置采样率 */
				if (Mp3FrameInfo.samprate != current_sample_rate)	//采样率 
				{
					current_sample_rate = Mp3FrameInfo.samprate;

					printf(" \r\n Bitrate       %dKbps", Mp3FrameInfo.bitrate/1000);
				    printf(" \r\n Samprate      %dHz", current_sample_rate);
					printf(" \r\n BitsPerSample %db", Mp3FrameInfo.bitsPerSample);
					printf(" \r\n nChans        %d", Mp3FrameInfo.nChans);
					printf(" \r\n Layer         %d", Mp3FrameInfo.layer);
					printf(" \r\n Version       %d", Mp3FrameInfo.version);
					printf(" \r\n OutputSamps   %d\r\n", Mp3FrameInfo.outputSamps);
					if((Mp3FrameInfo.bitsPerSample==16)&&(Mp3FrameInfo.nChans==2)&&(current_sample_rate>44000)&&(current_sample_rate<48100)&&(Mp3FrameInfo.outputSamps==2304))
					{
					    //DMA_set_size(Mp3FrameInfo.outputSamps);
					    //outputSamps==2304的由来，每帧数据固定是采样1152次（规定好的），双声道的话每次采样相当于采样两次
					    //左声道和右声道采样，所以1152次采样相当于2304次采样，每个声道的采样位数又是16位。低位在前，高位在后。
					    printf("DMA 设置尺寸\r\n", err);
					}
					else
					{
					    rval=1;
					    printf("参数不对，跳出循环\r\n", err);
					}
				}		
				
				#if 0 /*xqy 2018-1-27*/
				while(buf_fin==0||play_state==stop)
				{
					key=KEY_Scan(0);
					switch(key)
					{
						case KEY1_PRES:
							rval=1;		//下一曲
							break;
						case KEY0_PRES:
							if(play_state==play)
							{
								play_state=stop;
								printf("stop\r\n");
								stop_buf=Disable_DMA();
							}
							else
							{
								play_state=play;
								printf("play\r\n");
								if(stop_buf==bufflag)
								{
									DMA_Double_Buf_Init((uint32_t)data_pcm_buf,(uint32_t)data_pcm_buf2,pcm_buf_each_size/2);
								}
								else
								{
									DMA_Double_Buf_Init((uint32_t)data_pcm_buf2,(uint32_t)data_pcm_buf,pcm_buf_each_size/2);
								}
							}
							break;						
						case WKUP_PRES:
							rval=2;		//上一曲
							break;
					}
				}	
				#endif
			//buf_fin=0;
			bufflag = 1 -bufflag;																			//切换buffer
			//printf("切换buffer\r\n", err);
		}
	}
	printf("MP3播放结束\r\n", err);
	f_close(fmp3);
	//Disable_DMA();
	TIM3_Deinit();
	MP3FreeDecoder(Mp3Decoder);	
	myfree(fmp3);			
	myfree(data_read_buf);	
	myfree(data_pcm_buf);	
	myfree(data_pcm_buf2);	
	return rval;
}







//得到path路径下,目标文件的总个数 wav
////path:路径		    
////返回值:总有效文件数
//u16 wav_get_tnum(u8 *path)
//{	  
//	u8 res;
//	u16 rval=0;
// 	DIR tdir;	 		//临时目录
//	FILINFO tfileinfo;	//临时文件信息		
//	u8 *fn; 			 			   			     
//    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
//  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
//	tfileinfo.lfname=mymalloc(tfileinfo.lfsize);	//为长文件缓存区分配内存
//	if(res==FR_OK&&tfileinfo.lfname!=NULL)
//	{
//		while(1)//查询总的有效文件数
//		{
//	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
//	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
//     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
//			res=f_typetell(fn);	
//			if(res==0X4A)//取高四位,看看是不是音乐文件	
//			{
//				rval++;//有效文件数增加1
//			}	    
//		}  
//	} 
//	myfree(tfileinfo.lfname);
//	return rval;
//}



//播放音乐  wav
//void mp3_play(void)
//{
//	u8 res;
// 	DIR mp3dir;	 		//目录
//	FILINFO mp3fileinfo;//文件信息
//	u8 *fn;   			//长文件名
//	u8 *pname;			//带路径的文件名
//	u16 totmp3num; 		//音乐文件总数
//	u16 curindex;		//图片当前索引
//	u8 key;				//键值		  
// 	u16 temp;
//	
//	u16 *mp3indextbl;	//音乐索引表 
//	
//	u16 *wavindextbl;	//wav索引表 
//	u16 wav_num;
//	u16 totwavnum;//wav tol
//	
// 	while(f_opendir(&mp3dir,"0:/音乐"))//打开图片文件夹
// 	{	    
//		printf("MUSIC_fl_err");			  
//	} 									  
//	totmp3num=mp3_get_tnum("0:/音乐"); //得到总有效文件数
//  	while(totmp3num==NULL)//音乐文件总数为0		
// 	{	    
//		printf("0 music");						  
//	}	
//	totwavnum=wav_get_tnum("0:/音乐");
//	
//  mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
//	mp3fileinfo.lfname=mymalloc(mp3fileinfo.lfsize);	//为长文件缓存区分配内存
// 	pname=mymalloc(mp3fileinfo.lfsize);				//为带路径的文件名分配内存
// 	mp3indextbl=mymalloc(2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
//	wavindextbl=mymalloc(2*totwavnum);

// 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
// 	{	    
//			printf("mem_err");	
//	}  
//	
//	
//	res=f_opendir(&mp3dir,"0:/音乐"); //打开目录
//	if(res==FR_OK)
//	{
//		curindex=0;//当前索引为0
//		while(1)//全部查询一遍
//		{
//			temp=mp3dir.index;								//记录当前index
//	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
//	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
//     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
//			res=f_typetell(fn);	
//			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
//			{
//				mp3indextbl[curindex]=temp;//记录索引
//				curindex++;
//				mp3_index_show(curindex,totmp3num);
//				puts((const char*)fn);
//				printf("\r\n");
//				if(res==0X4A)
//				{
//					wavindextbl[wav_num]=temp;
//					printf("wav");
//					wav_num++;
//					mp3_index_show(wav_num,totwavnum);					
//					puts((const char*)fn);					
//					printf("\r\n");
//				}										
//			}					
//		} 
//	}   
//	printf("memory_used:%d%%\r\n",mem_perused());
//	
//	wav_num=0;
//	res=f_opendir(&mp3dir,(const TCHAR*)"0:/音乐"); 	//打开目录
//	while(res==FR_OK)//打开成功
//	{	
//		dir_sdi(&mp3dir,wavindextbl[wav_num]);			//改变当前目录索引	   
//		res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
//		if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
//		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);	
//		mp3_index_show(wav_num+1,totwavnum);		 
//		printf("now playing->");
//		puts((const char*)fn);					//显示歌曲名字 
//		printf("\r\n");				 		
//		
//		strcpy((char*)pname,"0:/音乐/");				//复制路径(目录)
//		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
//		
//		if(wav_decode_init(pname,&wavctrl)==0)//得到文件的信息,且为wav
//		{
//			if((wavctrl.bps==16)&&(wavctrl.nchannels==2)&&(wavctrl.samplerate>44000)&&(wavctrl.samplerate<48100))
//			{
//				key=wav_play_song(pname); 				 		//播放这个MP3  
//			}
//			else
//			{
//				key=1;
//				printf("wav_file_err 1\r\n");				
//			}
//		}
//		else
//		{
//			key=1;
//			printf("wav_file_err 2\r\n");
//		}			
//		if(key==2)		//上一曲
//		{
//			if(wav_num)wav_num--;
//			else wav_num=totwavnum-1;
// 		}else if(key==1)//下一曲
//		{
//			wav_num++;		   	
//			if(wav_num>=totwavnum)wav_num=0;//到末尾的时候,自动从头开始
// 		}else break;	//产生了错误 	 
//	} 						
//	
//	myfree(mp3fileinfo.lfname);	//释放内存			    
//	myfree(pname);				//释放内存			    
//	myfree(mp3indextbl);			//释放内存	 
//}






//
//u16 f_kbps=0;//歌曲文件位率	
////显示播放时间,比特率 信息 
////lenth:歌曲总长度
//void mp3_msg_show(u32 lenth)
//{	
//	static u16 playtime=0;//播放时间标记	     
// 	u16 time=0;// 时间变量
//	u16 temp=0;	  
//	if(f_kbps==0xffff)//未更新过
//	{
//		playtime=0;
//		f_kbps=VS_Get_HeadInfo();	   //获得比特率
//	}	 	 
//	time=VS_Get_DecodeTime(); //得到解码时间
//	if(playtime==0)playtime=time;
//	else if((time!=playtime)&&(time!=0))//1s时间到,更新显示数据
//	{
//		playtime=time;//更新时间 	 				    
//		temp=VS_Get_HeadInfo(); //获得比特率	   				 
//		if(temp!=f_kbps)
//		{
//			f_kbps=temp;//更新KBPS	  				     
//		}			 
//		//显示播放时间			 
//		LCD_ShowxNum(30,210,time/60,2,16,0X80);		//分钟
//		LCD_ShowChar(30+16,210,':',16,0);
//		LCD_ShowxNum(30+24,210,time%60,2,16,0X80);	//秒钟		
// 		LCD_ShowChar(30+40,210,'/',16,0); 	    	 
//		//显示总时间
//		if(f_kbps)time=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
//		else time=0;//非法位率	  
// 		LCD_ShowxNum(30+48,210,time/60,2,16,0X80);	//分钟
//		LCD_ShowChar(30+64,210,':',16,0);
//		LCD_ShowxNum(30+72,210,time%60,2,16,0X80);	//秒钟	  		    
//		//显示位率			   
//   		LCD_ShowxNum(30+110,210,f_kbps,3,16,0X80); 	//显示位率	 
//		LCD_ShowString(30+134,210,200,16,16,"Kbps");	  	  
//		LED0=!LED0;		//DS0翻转
//	}   		 
//}			  		 
////得到path路径下,目标文件的总个数
////path:路径		    
////返回值:总有效文件数
//u16 mp3_get_tnum(u8 *path)
//{	  
//	u8 res;
//	u16 rval=0;
// 	DIR tdir;	 		//临时目录
//	FILINFO tfileinfo;	//临时文件信息		
//	u8 *fn; 			 			   			     
//    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
//  	tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
//	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);	//为长文件缓存区分配内存
//	if(res==FR_OK&&tfileinfo.lfname!=NULL)
//	{
//		while(1)//查询总的有效文件数
//		{
//	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
//	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
//     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
//			res=f_typetell(fn);	
//			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
//			{
//				rval++;//有效文件数增加1
//			}	    
//		}  
//	} 
//	myfree(SRAMIN,tfileinfo.lfname);
//	return rval;
//}
////播放音乐
//void mp3_play(void)
//{
//	u8 res;
// 	DIR mp3dir;	 		//目录
//	FILINFO mp3fileinfo;//文件信息
//	u8 *fn;   			//长文件名
//	u8 *pname;			//带路径的文件名
//	u16 totmp3num; 		//音乐文件总数
//	u16 curindex;		//图片当前索引
//	u8 key;				//键值		  
// 	u16 temp;
//	u16 *mp3indextbl;	//音乐索引表 
// 	while(f_opendir(&mp3dir,"0:/MUSIC"))//打开图片文件夹
// 	{	    
//		Show_Str(30,190,240,16,"MUSIC文件夹错误!",16,0);
//		delay_ms(200);				  
//		LCD_Fill(30,190,240,206,WHITE);//清除显示	     
//		delay_ms(200);				  
//	} 									  
//	totmp3num=mp3_get_tnum("0:/MUSIC"); //得到总有效文件数
//  	while(totmp3num==NULL)//音乐文件总数为0		
// 	{	    
//		Show_Str(30,190,240,16,"没有音乐文件!",16,0);
//		delay_ms(200);				  
//		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
//		delay_ms(200);				  
//	}										   
//  	mp3fileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
//	mp3fileinfo.lfname=mymalloc(SRAMIN,mp3fileinfo.lfsize);	//为长文件缓存区分配内存
// 	pname=mymalloc(SRAMIN,mp3fileinfo.lfsize);				//为带路径的文件名分配内存
// 	mp3indextbl=mymalloc(SRAMIN,2*totmp3num);				//申请2*totmp3num个字节的内存,用于存放音乐文件索引
// 	while(mp3fileinfo.lfname==NULL||pname==NULL||mp3indextbl==NULL)//内存分配出错
// 	{	    
//		Show_Str(30,190,240,16,"内存分配失败!",16,0);
//		delay_ms(200);				  
//		LCD_Fill(30,190,240,146,WHITE);//清除显示	     
//		delay_ms(200);				  
//	}  	
//	VS_HD_Reset();
//	VS_Soft_Reset();
//	vsset.mvol=220;						//默认设置音量为220.
//	mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
//	//记录索引
//    res=f_opendir(&mp3dir,"0:/MUSIC"); //打开目录
//	if(res==FR_OK)
//	{
//		curindex=0;//当前索引为0
//		while(1)//全部查询一遍
//		{
//			temp=mp3dir.index;								//记录当前index
//	        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
//	        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
//     		fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
//			res=f_typetell(fn);	
//			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
//			{
//				mp3indextbl[curindex]=temp;//记录索引
//				curindex++;
//			}	    
//		} 
//	}   
//   	curindex=0;											//从0开始显示
//   	res=f_opendir(&mp3dir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
//	while(res==FR_OK)//打开成功
//	{	
//		dir_sdi(&mp3dir,mp3indextbl[curindex]);			//改变当前目录索引	   
//        res=f_readdir(&mp3dir,&mp3fileinfo);       		//读取目录下的一个文件
//        if(res!=FR_OK||mp3fileinfo.fname[0]==0)break;	//错误了/到末尾了,退出
//     	fn=(u8*)(*mp3fileinfo.lfname?mp3fileinfo.lfname:mp3fileinfo.fname);			 
//		strcpy((char*)pname,"0:/MUSIC/");				//复制路径(目录)
//		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
// 		LCD_Fill(30,190,240,190+16,WHITE);				//清除之前的显示
//		Show_Str(30,190,240-30,16,fn,16,0);				//显示歌曲名字 
//		mp3_index_show(curindex+1,totmp3num);
//		key=mp3_play_song(pname); 				 		//播放这个MP3    
//		if(key==2)		//上一曲
//		{
//			if(curindex)curindex--;
//			else curindex=totmp3num-1;
// 		}else if(key<=1)//下一曲
//		{
//			curindex++;		   	
//			if(curindex>=totmp3num)curindex=0;//到末尾的时候,自动从头开始
// 		}else break;	//产生了错误 	 
//	} 											  
//	myfree(SRAMIN,mp3fileinfo.lfname);	//释放内存			    
//	myfree(SRAMIN,pname);				//释放内存			    
//	myfree(SRAMIN,mp3indextbl);			//释放内存	 
//}

////播放一曲指定的歌曲				     	   									    	 
////返回值:0,正常播放完成
////		 1,下一曲
////       2,上一曲
////       0XFF,出现错误了
//u8 mp3_play_song(u8 *pname)
//{	 
// 	FIL* fmp3;
//    u16 br;
//	u8 res,rval;	  
//	u8 *databuf;	   		   
//	u16 i=0; 
//	u8 key;  	    
//			   
//	rval=0;	    
//	fmp3=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
//	databuf=(u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
//	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
//	if(rval==0)
//	{	  
//	  	VS_Restart_Play();  					//重启播放 
//		VS_Set_All();        					//设置音量等信息 			 
//		VS_Reset_DecodeTime();					//复位解码时间 	  
//		res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
//		if(res==0x4c)//如果是flac,加载patch
//		{	
//			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
//		}  				 		   		 						  
//		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
// 		if(res==0)//打开成功.
//		{ 
//			VS_SPI_SpeedHigh();	//高速						   
//			while(rval==0)
//			{
//				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
//				i=0;
//				do//主播放循环
//			    {  	
//					if(VS_Send_MusicData(databuf+i)==0)//给VS10XX发送音频数据
//					{
//						i+=32;
//					}else   
//					{
//						key=KEY_Scan(0);
//						switch(key)
//						{
//							case KEY0_PRES:
//								rval=1;		//下一曲
//								break;
//							case KEY2_PRES:
//								rval=2;		//上一曲
//								break;
//							case WKUP_PRES:	//音量增加
//								if(vsset.mvol<250)
//								{
//									vsset.mvol+=5;
//						 			VS_Set_Vol(vsset.mvol);	
//								}else vsset.mvol=250;
//								mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
//								break;
//							case KEY1_PRES:	//音量减
//								if(vsset.mvol>100)
//								{
//									vsset.mvol-=5;
//						 			VS_Set_Vol(vsset.mvol);	
//								}else vsset.mvol=100;
//								mp3_vol_show((vsset.mvol-100)/5);	//音量限制在:100~250,显示的时候,按照公式(vol-100)/5,显示,也就是0~30   
//								break;
//						}
//						mp3_msg_show(fmp3->fsize);//显示信息	    
//					}	    	    
//				}while(i<4096);//循环发送4096个字节 
//				if(br!=4096||res!=0)
//				{
//					rval=0;
//					break;//读完了.		  
//				} 							 
//			}
//			f_close(fmp3);
//		}else rval=0XFF;//出现错误	   	  
//	}						     	 
//	myfree(SRAMIN,databuf);	  	 		  	    
//	myfree(SRAMIN,fmp3);
//	return rval;	  	 		  	    
//}
int  exist_not(u8 *ptr)//判断该标志是否存在于数组表里面
{
    u8 i;
    uart_printf("长度为%d\n",sizeof(id3v2)/sizeof(ID3V2));
    for(i=0;i<sizeof(id3v2)/sizeof(ID3V2);i++)
    {
        if(memcmp_x(ptr,(u8 *)id3v2[i].FrameID,4)==0)
            return i;
    }
    return -1;
}
void memcpy_x(u8 *d,u8 *s,int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        *(d+i)=*(s+i);
    }
}

s32  memcmp_x(u8 *d,u8 *s,int len)
{
    //u16 d_strlen;
    //u16 s_strlen;
    u8 i;
    #if 0 /*xqy 2018-3-3*/
    d_strlen = strlen(d);
    s_strlen = strlen(s);
    if(d_strlen!=s_strlen)
    {
        return -1;
    }

    if(d_strlen > len)
    {
        return -1;
    }
    #endif
    
    for(i=0;i<len;i++)
    {
        if(*(d+i) != *(s+i))
            return -1;
    }
    return 0;
}


int MP3_ID3V2_disp(char *read_ptr)
{
    u8 i,j;
    u8 *ptr;
    u8 frame[5];
    int size=0;//整个标签的字节大小
    u8  frame_num = 0;//标签帧的数量
    s8  exist_flag;
    int size_x;
    u8  size_xqy[4];
    u8  frame_data[50];
    ptr = read_ptr+10;//偏移跳过头帧10字节

    memcpy_x((u8 *)frame,(u8 *)ptr,4);//复制标签帧标志
    frame[4] = '\0';//字符串结束符标志免得打印的时候乱码
    for(i=0;i<4;i++)
    {
        size_xqy[i] = *(ptr+4+i);
    }
    uart_printf("标签名为%s\n",frame);
    while(1)
    {
        exist_flag = exist_not((u8 *)frame);
        if(exist_flag==(-1))//发现没有标签帧了就跳出去，马上到main_data了
            break;

        frame_num++;//增加一个标签帧
        
        ptr = ptr+10;//偏移跳过标签帧头帧10字节
        size_x=(size_xqy[0])*0x100000+(size_xqy[1])*0x10000+(size_xqy[2])*0x100+(size_xqy[3]);//帧长度（除去头部10字节）
        //麻蛋，还好搜到一篇良心文章，不然要被坑惨了，算帧头的长度和算标签帧头的长度不一样。xqy
        if(size_x>50)
        {
            uart_printf("标签帧数据大于50，不打印出来\n");
        }
        else
        {
            memset((u8 *)frame_data,0x00,50);//格式化数组
            memcpy_x((u8 *)frame_data,(u8 *)ptr,size_x);//复制标签帧数据
            uart_printf("%s:%s(%s)\n",(u8 *)frame,(u8 *)(frame_data+1),(u8 *)id3v2[exist_flag].str);
            //ID3V2标签的真实数据的第一字节是文本标志，为1时，代表接下来的数据为文本数据，否则是其他的（暂时的理解）
            
        }
        
        
        ptr = ptr+size_x;//偏移跳过标签帧数据x字节
        
        size += 10;//头信息10字节
        size += size_x;//记录标签帧数据总长
        //重复取信息
        if(ptr - read_ptr>4096)
        {
            uart_printf("第一帧数据超过4096，跳出\n");
            break;
        }
        memcpy_x((u8 *)frame,(u8 *)ptr,4);//复制标签帧标志
        for(i=0;i<4;i++)
        {
            size_xqy[i] = *(ptr+4+i);
        }
    }
    uart_printf("标签帧数据总长度为%d\n",size);
    return size;
}
int MP3_ID3V1_disp(char *read_ptr)
{
    
}

u8 wav_play_song(u8 *pname)
{
    
}









