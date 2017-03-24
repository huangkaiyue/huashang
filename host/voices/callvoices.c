#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "base/pool.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "gpio_7620.h"
#include "../studyvoices/qtts_qisc.h"
#include "config.h"

static char buf_voices[STD_RECODE_SIZE];
static char pcm_voices16k[STD_RECODE_SIZE_16K];
static int len_voices = 0;
static unsigned char recorde_live;
static unsigned char oldrecorde_live;

//默认音频头部数据
struct wave_pcm_hdr pcmwavhdr = {
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	RECODE_RATE,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

#ifdef AMR8k_DATA
static char amr_data[12*KB];
#endif

//将录制的8k语音转换成16k语音
static void pcmVoice8kTo16k(const char *inputdata,char *outputdata,int inputLen){
	int pos=0,npos=0;
	for(pos=0;pos<inputLen;pos+=2){
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
	}
}
static int PcmVoice8kTo16k_File(const char *inputdata,const char *outfilename,int inputLen){
	FILE *fp=NULL;
	fp = fopen(outfilename,"w+");
	if(fp==NULL){
		return -1;
	}
	int pos=0,npos=0;
	for(pos=0;pos<inputLen;pos+=2){
		fwrite(inputdata+pos,1,2,fp);
		fwrite(inputdata+pos,1,2,fp);
	}
	fclose(fp);
	return 0;
}	
/*****************************************************
*获取状态
*****************************************************/
int GetRecordeVoices_PthreadState(void){
	return recorde_live;
}
/*****************************************************
*设置状态
*****************************************************/
static void SetRecordeVoices_PthreadState(unsigned char state){
	printf("%s: recorde_live %d  %d \n",__func__,recorde_live,state);
	recorde_live=state;
}

/*****************************************************
*开启语音识别状态
*****************************************************/
void StartTuling_RecordeVoices(void){	
	SetRecordeVoices_PthreadState(START_SPEEK_VOICES);
}
/*****************************************************
*结束语音识别状态
*****************************************************/
void end_event_std(void){
	if(GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES)
		SetRecordeVoices_PthreadState(END_SPEEK_VOICES);
}
/*****************************************************
*进入播放wav原始数据状态
*****************************************************/
void start_event_play_wav(int i){
	SetRecordeVoices_PthreadState(PLAY_WAV);
}
void start_speek_wait(void){
	SetRecordeVoices_PthreadState(SPEEK_WAIT);
}
void start_play_tuling(void){
	SetRecordeVoices_PthreadState(PLAY_TULING);
}
/*****************************************************
*进入播放URL状态
*****************************************************/
void start_event_play_url(void){
	SetRecordeVoices_PthreadState(PLAY_URL);
}
/*****************************************************
*暂停录音状态
*****************************************************/
void pause_record_audio(int i){	
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
}

/*****************************************************
*录制短消息状态
******************************************************/
void start_event_talk_message(void){
	SetRecordeVoices_PthreadState(START_TAIK_MESSAGE);
}

static void TaiBenToTulingNOVoices(void){
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case 1:
			PlaySystemAmrVoices(NO_VOICES);
			break;
		case 2:
			PlaySystemAmrVoices(NO_VOICES_1);
			break;
		case 3:
			PlaySystemAmrVoices(NO_VOICES_2);
			break;
		case 4:
			PlaySystemAmrVoices(NO_VOICES_3);
			break;
		case 5:
			PlaySystemAmrVoices(NO_VOICES_4);
			break;
		case 6:
			PlaySystemAmrVoices(NO_VOICES_5);
			break;
		case 7:
			PlaySystemAmrVoices(NO_VOICES_6);
			break;
		case 8:
			PlaySystemAmrVoices(NO_VOICES_7);
			break;
		case 9:
			PlaySystemAmrVoices(NO_VOICES_8);
			break;
		case 10:
			PlaySystemAmrVoices(NO_VOICES_9);
			break;
	}
}
/****************************************
@函数功能:	开始上传语音到服务器
@参数:	无
*****************************************/
static void Start_uploadVoicesData(void){
	start_event_play_wav(1);		//播放过渡音，等待上传语音识别结果
	pool_add_task(play_sys_tices_voices,TULING_WINT);
	usleep(100);	
	DEBUG_VOICES("len_voices = %d  \n",len_voices);
#ifdef AMR8k_DATA		
	pcmwavhdr.size_8 = (len_voices+36);
	pcmwavhdr.data_size = len_voices;
	memcpy(buf_voices,&pcmwavhdr,WAV_HEAD); 			//写音频头
	if(WavToAmr8k((const char *)buf_voices,amr_data,&amr_size)){	//转换成amr格式
		DEBUG_VOICES_ERROR("enc failed \n");
		return ;
	}
	ReqTulingServer((const char *)amr_data,amr_size,"amr","3",RECODE_RATE);
#endif
	
#ifdef	PCM_TEST
	test_save8kpcm(buf_voices+WAV_HEAD,len_voices);
	test_save16kpcm(buf_voices+WAV_HEAD,len_voices);
#endif
	
#ifdef PCM_DATA
#ifdef MY_HTTP_REQ
	pcmVoice8kTo16k(buf_voices+WAV_HEAD,pcm_voices16k,len_voices);
	ReqTulingServer((const char *)pcm_voices16k,len_voices*2,"pcm","0",RECODE_RATE*2);
#else
	PcmVoice8kTo16k_File(buf_voices,"pcm16k.cache",len_voices);
	ReqTulingServer((const char *)"pcm16k.cache",len_voices*2,"pcm","0",RECODE_RATE*2);
#endif
#endif
	//16kpcm--->16kwav--->16k amr 注意先将这个
#ifdef AMR16K_DATA
	int amr_size=0;
	pcmVoice8kTo16k(buf_voices+WAV_HEAD,pcm_voices16k+WAV_HEAD,len_voices);
	pcmwavhdr.size_8 = (len_voices*2+36);
	pcmwavhdr.data_size = len_voices*2;
	pcmwavhdr.samples_per_sec=16000;
	memcpy(pcm_voices16k,&pcmwavhdr,WAV_HEAD);			//写音频头

#ifdef MY_HTTP_REQ
	if(WavAmr16k((const char *)pcm_voices16k,amr_data,&amr_size)){	//转换成amr格式
		return ;
		DEBUG_VOICES_ERROR("enc failed \n");
	}
	ReqTulingServer((const char *)amr_data,amr_size,"amr","3",RECODE_RATE*2);
#else
	if(WavAmr16kFile((const void *)pcm_voices16k,(void *)"./amr16k.cache",&amr_size)){
		return ;
	}
	ReqTulingServer((const char *)"./amr16k.cache",amr_size,"amr","3",RECODE_RATE*2);
#endif

#endif

}
/****************************************
@函数功能:	处理采集数据函数
@参数:	data 采集到的数据  size 数据大小
*****************************************/
static void Save_VoicesPackt(const char *data,int size){
	int i;
	test_Save_VoicesPackt_function_log((const char *)"start",len_voices);
	if(data != NULL){
		if((len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){//大于5秒的音频丢掉
			test_Save_VoicesPackt_function_log((const char *)"STD_RECODE_SIZE exit1",len_voices);
			goto exit1;
		}
#if 0		
		//只有有声道有数据，左声道没有，需要拷贝2、3两个数据
		for(i=2; i<size; i+=4){
			//双声道数据转成单声道数据
			memcpy(buf_voices+WAV_HEAD+len_voices,data+i,2);
			len_voices += 2;
		}
#else
		for(i=0; i<size; i+=4){
			memcpy(buf_voices+WAV_HEAD+len_voices,data+i,2);
			len_voices += 2;
		}
#endif	//end 0
		test_Save_VoicesPackt_function_log((const char *)"<STD_RECODE_SIZE ",len_voices);
	}else{
		if(len_voices > VOICES_MIN)	//大于0.5s 音频，则上传到服务器当中开始识别  13200
		{
#ifdef DATOU_JIANG
			led_lr_oc(openled);
#endif
			Start_uploadVoicesData();		//开始上传语音
			test_Save_VoicesPackt_function_log((const char *)">VOICES_MIN ",len_voices);
			goto exit0;
		}
		else if(len_voices < VOICES_ERR){			//
			test_Save_VoicesPackt_function_log((const char *)"<VOICES_ERR ",len_voices);
			goto exit1;	//误触发
		}
		else{	//VOICES_ERR --->VOICES_MIN 区间的音频，认定为无效音频
			if(GetRecordeVoices_PthreadState() !=PLAY_WAV){
				TaiBenToTulingNOVoices();
			}
			test_Save_VoicesPackt_function_log((const char *)"TaiBenToTulingNOVoices ",len_voices);
			goto exit1;
		}
		test_Save_VoicesPackt_function_log((const char *)"error ",len_voices);
	}
	return ;
exit1:
	pause_record_audio(1);
exit0:
	memset(buf_voices,0,len_voices);
	len_voices = 0;
	test_Save_VoicesPackt_function_log((const char *)"exit Save_VoicesPackt ok",GetRecordeVoices_PthreadState());
	return ;
}
//关机，将当前系统时间发送给MCU
int SetSystemTime(unsigned char outtime){
	time_t timep;
	struct tm *p;
	time(&timep);
	char syscloseTime[64];
	SocSendMenu(3,0);
	usleep(100*1000);
	p=localtime(&timep); /*取得当地时间*/
	if((p->tm_hour)+8>=24){
		p->tm_hour=(p->tm_hour)+8-24;
	}else{
		p->tm_hour=(p->tm_hour)+8;
	}
	sprintf(syscloseTime,"%d:%d",(p->tm_hour),(p->tm_min)+outtime);
	printf("SetSystemTime : %s\n",syscloseTime);
	SocSendMenu(2,syscloseTime);
	return 0;
}
/*******************************************************
函数功能: 录音线程 ，对当前环境检测，筛选有效音频
参数:   	无
返回值: 
********************************************************/
static void *PthreadRecordVoices(void *arg){
	char *pBuf;
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
	time_t t;
	int endtime,starttime;
	starttime=time(&t);
	endtime=time(&t);
	while(GetRecordeVoices_PthreadState()!=RECODE_STOP){
#ifdef CLOCESYSTEM
		endtime=time(&t);
		if((endtime-starttime)>ERRORTIME){
			starttime=time(&t);
			sysMes.Playlocaltime=time(&t);
		}else{
			if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
				if((endtime-starttime)==10){
					printf("=============MUTE====%d===========\n",endtime-starttime);
					Mute_voices(MUTE);
				}
				if((endtime-starttime)==SYSTEMOUTSIGN){		//长时间不触发事件，则关闭
					SetRecordeVoices_PthreadState(TIME_SIGN);
				}
				if((endtime-starttime)>SYSTEMOUTTIME){		//长时间不触发事件，则关闭
					SetRecordeVoices_PthreadState(TIME_OUT);
					TimeLog("TIME_OUT\n");
				}
			}else{
				starttime=time(&t);
			}
			if((endtime-sysMes.Playlocaltime)>PLAYOUTTIME&&GetRecordeVoices_PthreadState()==PLAY_URL){
				SetRecordeVoices_PthreadState(PLAY_OUT);
				TimeLog("PLAY_OUT\n");
			}
		}
#endif
		switch(GetRecordeVoices_PthreadState()){
			case START_SPEEK_VOICES:	//会话录音
				pBuf = I2sGetvoicesData();
				Save_VoicesPackt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:		//会话录音结束
				Save_VoicesPackt(NULL, 0);	//发送空音频作为一段音频的结束标志
				break;
#ifdef SPEEK_VOICES
			case START_TAIK_MESSAGE:	//对讲录音
				pBuf = I2sGetvoicesData();
				SaveRecorderVoices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);		//不会有快进的感觉
				break;
#endif
#ifdef CLOCESYSTEM
			case TIME_SIGN:
				//PlayTuLingTaibenQtts("小朋友快来跟我玩，跟我说话聊天吧。",QTTS_GBK);
				play_sys_tices_voices(SPEEK_WARNING);
				sleep(1);
				break;
				
			case PLAY_OUT:
				SetSystemTime(1);
				SetRecordeVoices_PthreadState(PLAY_URL);
				sysMes.Playlocaltime=time(&t);
				break;
				
			case TIME_OUT:				//超时退出
				SetSystemTime(1);
				pause_record_audio(2);
				starttime=time(&t);
				break;
#endif
			default:
				I2sGetvoicesData();		//默认状态清除音频
				usleep(50000);
				break;
		}
	}
	SetRecordeVoices_PthreadState(RECODE_EXIT_FINNISH);
	DEBUG_VOICES("handle record voices exit\n");
	test_Save_VoicesPackt_function_log("recorde pthread exit",0);
	return NULL;
}

/*****************************************************
开启录音工作线程
*****************************************************/
void InitRecord_VoicesPthread(void){	
#ifdef TEST_MIC
	if(pthread_create_attr(TestRecordePlay,NULL)){
		perror("create test recorde play failed!");
		exit(-1);
	}
#else
	if(pthread_create_attr(PthreadRecordVoices,NULL)){
    	perror("create handle record voices failed!");
        exit(-1);
	}
#endif	//end TEST_MIC
	usleep(300);
}

/*****************************************************
退出录音(退出系统)
*****************************************************/
void ExitRecord_Voicespthread(void){
	unsigned char timeout=0;
	SetRecordeVoices_PthreadState(RECODE_STOP);
	while(GetRecordeVoices_PthreadState()!=RECODE_EXIT_FINNISH){
		usleep(100000);
		if(GetRecordeVoices_PthreadState()!=RECODE_EXIT_FINNISH)
			SetRecordeVoices_PthreadState(RECODE_STOP);
		if(timeout++>50)
			break;
	}
	DEBUG_VOICES("exit record pthread success (%d)\n",GetRecordeVoices_PthreadState());
}
