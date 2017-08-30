#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "base/pool.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "gpio_7620.h"
#include "host/studyvoices/qtts_qisc.h"
#include "config.h"
#include "uart/uart.h"
#include "huashangMusic.h"
#include "mplay.h"

static RecoderVoices_t *RV=NULL;
struct wave_pcm_hdr pcmwavhdr = {//默认音频wav头部数据
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


void lockRecoderPthread_TimeoutCheck(void){
	RV->lockTimeOutcheck=TIME_OUT_CHECK_LOCK;
}
void unlockRecoderPthread_TimeoutCheck(void){
	RV->lockTimeOutcheck=TIME_OUT_CHECK_UNLOCK;
}	
int getlockRecoderPthread_TimeoutCheck(void){
	return (int)RV->lockTimeOutcheck;
}

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
unsigned int GetCurrentEventNums(void){
	return RV->CurrentuploadEventNums;
}
unsigned int updateCurrentEventNums(void){
	struct timeval starttime;
    gettimeofday(&starttime,0); 
	RV->CurrentuploadEventNums=(unsigned int)starttime.tv_sec/2+starttime.tv_usec;
	return RV->CurrentuploadEventNums;
}

/*****************************************************
*获取状态
*****************************************************/
int GetRecordeVoices_PthreadState(void){
	return RV->recorde_live;
}

/*****************************************************
*设置状态
*****************************************************/
static void SetRecordeVoices_PthreadState(unsigned char state){
	printf("%s: recorde_live %d  %d \n",__func__,RV->recorde_live,state);
	RV->recorde_live=state;
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
void StopTuling_RecordeVoices(void){
	if(GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES){
		SetRecordeVoices_PthreadState(END_SPEEK_VOICES);
		showFacePicture(WAIT_CTRL_NUM4);
	}
}
/*****************************************************
*进入播放wav原始数据状态
*****************************************************/
void start_event_play_wav(void){
	SetRecordeVoices_PthreadState(PLAY_WAV);
}
void start_speek_wait(void){
	SetRecordeVoices_PthreadState(SPEEK_WAIT);
}
void start_play_tuling(void){
	SetRecordeVoices_PthreadState(PLAY_DING_VOICES);
}
/*****************************************************
*进入播放Mp3music状态
*****************************************************/
void start_event_play_Mp3music(void){
	SetRecordeVoices_PthreadState(PLAY_MP3_MUSIC);
}
//混音播放
void start_event_play_soundMix(void){
	SetRecordeVoices_PthreadState(SOUND_MIX_PLAY);
}

/*****************************************************
*暂停录音状态
*****************************************************/
void pause_record_audio(void){
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
}
void lock_pause_record_audio(void){
	if(RV->recorde_live==PLAY_WAV){
		SetRecordeVoices_PthreadState(RECODE_PAUSE);
		showFacePicture(WAIT_CTRL_NUM4);
	}else{
		printf("%s cannot interrupt state \n",__func__);
	}
}

void closeSystem(void){
	RV->WaitSleep=SYSTEM_INIT;
	System_StateLog("close system");
	SleepRecoder_Phthread();
	RV->closeTime =0;
#if defined(HUASHANG_JIAOYU)	
	led_lr_oc(openled);
	Close_tlak_Light();
	Create_PlayImportVoices(CMD_4547_SLEEP);
#else
	SetMucClose_Time(1);	//设置一分钟后关机
#endif
}
void SleepRecoder_Phthread(void){
	SetRecordeVoices_PthreadState(HUASHANG_SLEEP);
}
int SleepSystem(void){
	if(++RV->WaitSleep>=SYSTEM_SLEEP){
		printf("-----------------------\n close system --------------------\n");
		closeSystem();
		return -1;
	}
	return 0;
}
//检查并唤醒当前系统
int  checkAndWakeupSystem(void){
	if(RV->recorde_live==HUASHANG_SLEEP||RV->recorde_live==HUASHANG_SLEEP_OK){
		RV->WaitSleep =SYSTEM_INIT;
		pause_record_audio();
		pool_add_task(PlayWakeUpVoices,NULL);
		usleep(1000);
		return -1;
	}
	return 0;
}

/*****************************************************
*录制短消息状态
******************************************************/
void start_event_talk_message(void){
	SetRecordeVoices_PthreadState(START_TAIK_MESSAGE);
}

static void *uploadPcmPthread(void *arg){
	HandlerText_t *handText = (HandlerText_t *)arg;
	SpeekEvent_process_log("request tuling","start",0);
	ReqTulingServer(handText,"pcm","0",RECODE_RATE*2);
	RV->uploadState = END_UPLOAD;
	return NULL;
}
#ifdef TEST_FACTORY
static void FactoryTestWritePLay(char *data,int len){
	int i=0,wLen=0; 
	for(i=0;i<=len;i+=2){
		memcpy(play_buf+wLen,data+i,2);
		wLen+=2;
		memcpy(play_buf+wLen,data+i,2);
		wLen+=2;
		if(wLen==I2S_PAGE_SIZE){
			wLen=0;
			write_pcm(play_buf);	
		}
	}
}
#endif
/****************************************
@函数功能:	开始上传语音到服务器
@参数:	无
*****************************************/
static void Start_uploadVoicesData(void){
#ifdef TEST_FACTORY
	if(getFactoryTest()){
		FactoryTestWritePLay(RV->buf_voices+WAV_HEAD,RV->len_voices);
		return ;	
	}
#endif
	if(RV->uploadState==START_UPLOAD){
		printf("%s add upload voices failed \n",__func__);
		SpeekEvent_process_log("add upload voices failed","",0);
		return ;
	}
	RV->uploadState = START_UPLOAD;
	start_play_tuling();	//设置当前播放状态为 : 播放上传请求
	Create_PlayImportVoices(TULING_WAIT_VOICES);
	usleep(200);
	HandlerText_t *up = (HandlerText_t *)calloc(1,sizeof(HandlerText_t));
	if(up){
		up->dataSize =RV->len_voices*2;
		up->data= (char *)calloc(1,up->dataSize+2);
		if(up->data){
			up->EventNums = GetCurrentEventNums();
			pcmVoice8kTo16k(RV->buf_voices+WAV_HEAD,up->data,RV->len_voices);
			pthread_create_attr(uploadPcmPthread,(void * )up);
		}
	}
}
/****************************************
@函数功能:	处理采集数据函数
@参数:	data 采集到的数据  size 数据大小
*****************************************/
static void Save_VoicesPackt(const char *data,int size){
	int i;
	if(data != NULL){
		if((RV->len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){//大于5秒的音频丢掉
			printf("%s > STD_RECODE_SIZE RV->len_voices=%d\n",__func__,RV->len_voices);
			SpeekEvent_process_log((const char *)"STD_RECODE_SIZE","is >5s",RV->len_voices);
			goto exit1;
		}
#if defined(HUASHANG_JIAOYU)
		//if(--RV->freeVoicesNum>0){
		//	return ;
		//}
#endif
#if 0		
		//只有有声道有数据，左声道没有，需要拷贝2、3两个数据
		for(i=2; i<size; i+=4){
			//双声道数据转成单声道数据
			memcpy(RV->buf_voices+WAV_HEAD+RV->len_voices,data+i,2);
			RV->len_voices += 2;
		}
#else
		for(i=0; i<size; i+=4){
			memcpy(RV->buf_voices+WAV_HEAD+RV->len_voices,data+i,2);
			RV->len_voices += 2;
		}
#endif	//end 0
	}else{
		if(RV->len_voices > VOICES_MIN)	//大于0.5s 音频，则上传到服务器当中开始识别  13200
		{
			SpeekEvent_process_log((const char *)"start","upload",GetRecordeVoices_PthreadState());
			printf("%s Start_uploadVoicesData RV->len_voices=%d\n",__func__,RV->len_voices);
			Start_uploadVoicesData();		//开始上传语音
			goto exit0;
		}
		else if(RV->len_voices < VOICES_ERR){			//
			printf("%s < VOICES_ERR RV->len_voices=%d\n",__func__,RV->len_voices);
			SpeekEvent_process_log((const char *)"< VOICES_ERR","error voices",GetRecordeVoices_PthreadState());
			goto exit1;	//误触发
		}
		else{	//VOICES_ERR --->VOICES_MIN 区间的音频，认定为无效音频
			printf("%s < VOICES_MIN RV->len_voices=%d\n",__func__,RV->len_voices);
			SpeekEvent_process_log((const char *)"< VOICES_MIN","error voices",GetRecordeVoices_PthreadState());
			goto exit1;
		}
	}
	return ;
exit1:
	pause_record_audio();
exit0:
	memset(RV->buf_voices,0,RV->len_voices);
	RV->len_voices = 0;
#if defined(HUASHANG_JIAOYU)
	RV->freeVoicesNum =FREE_VOICE_NUMS;
#endif	
	return ;
}
//设置多久之后关机，将当前系统时间发送给MCU
int SetMucClose_Time(unsigned char closeTime){
	time_t timep;
	struct tm *p;
	time(&timep);
	char syscloseTime[64]={0};
	printf("\n .............SetMucClose_Time : %d.............\n",closeTime);
	SocSendMenu(3,0);
	usleep(100*1000);
	p=localtime(&timep); /*取得当地时间*/
	if((p->tm_hour)+8>=24){
		p->tm_hour=(p->tm_hour)+8-24;
	}else{
		p->tm_hour=(p->tm_hour)+8;
	}
	sprintf(syscloseTime,"%d:%d",(p->tm_hour),(p->tm_min)+closeTime);
	printf("\n .............SetMucClose_Time : %s.............\n",syscloseTime);
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
	lockRecoderPthread_TimeoutCheck();
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
	time_t t;
	int endtime,starttime,closeSystemTime;
	starttime=time(&t);
	endtime=time(&t);
	pause_record_audio();
	while(GetRecordeVoices_PthreadState()!=RECODE_STOP){
		endtime=time(&t);
		//printf("%s: time=%d  =GetRecordeVoices_PthreadState =%d\n",__func__,endtime-starttime,GetRecordeVoices_PthreadState());
		if((endtime-starttime)>ERRORTIME){	//开机时候，没有获取网络时间，导致时间差过大
			starttime=time(&t);
		}else{
			if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
				if((endtime-starttime)==LONG_TIME_NOT_USER_MUTE_VOICES){	//10s 之后，不用关闭音频
					Mute_voices(MUTE);
				}
				if((endtime-starttime)>SYSTEMOUTSIGN){		//第一次长时间不触发事件，则关闭
					//printf("entry not long time play voices =%d\n",getlockRecoderPthread_TimeoutCheck());
					SetRecordeVoices_PthreadState(TIME_SIGN);
				}
			}else{
				starttime=time(&t);
			}		
		}
		switch(GetRecordeVoices_PthreadState()){
			case START_SPEEK_VOICES:	//会话录音
				pBuf = I2sGetvoicesData();
				Save_VoicesPackt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:		//会话录音结束
				Save_VoicesPackt(NULL, 0);	//发送空音频作为一段音频的结束标志
				break;
			case START_TAIK_MESSAGE:	//对讲录音
				pBuf = I2sGetvoicesData();
				SaveRecorderVoices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);			//不会有快进的感觉
				break;

			case TIME_SIGN:				//提示休息很久了
				if(getlockRecoderPthread_TimeoutCheck()==TIME_OUT_CHECK_LOCK){
					starttime=time(&t);
					SetRecordeVoices_PthreadState(RECODE_PAUSE);
					break;
				}
				System_StateLog("time out for play music");
				if(!SleepSystem())	{
					Create_PlaySystemEventVoices(CMD_40_NOT_USER_WARN);
				}
				sleep(1);
				break;
			case PLAY_DING_VOICES:
				usleep(50000);
				break;
			case HUASHANG_SLEEP:		//华上睡眠状态	
				if(++RV->closeTime==60){
					showFacePicture(CLEAR_SYSTEM_PICTURE);
					SetRecordeVoices_PthreadState(HUASHANG_SLEEP_OK);
					RV->closeTime=0;
					starttime=time(&t);
					closeSystemTime=time(&t);
					printf("\n .............HUASHANG_SLEEP .............\n");
				}
				I2sGetvoicesData();		//默认状态清除音频
				usleep(50000);
				break;
			case HUASHANG_SLEEP_OK:
				//printf("\n ............. HUASHANG endtime =%d  closeSystemTime=%d.............\n",endtime,closeSystemTime);
				I2sGetvoicesData();		//默认状态清除音频
				usleep(50000);
				endtime=time(&t);
				if(endtime-closeSystemTime>CLOSE_SYSTEM_TIME){
					//printf("\n ............. start HUASHANG_SLEEP_OK endtime=%d  closeSystemTime=%d.............\n",endtime,closeSystemTime);
					SetRecordeVoices_PthreadState(HUASHANG_CLOSE_SYSTEM);
					System_StateLog(".......close system........");
					SetMucClose_Time(1);
				}
				break;
			case PLAY_MP3_MUSIC:
				if(GetPlayMusicState()==MAD_PAUSE){
					endtime=time(&t);
					//printf("\n ............play mp3 endtime=%d  closeSystemTime=%d.............\n",endtime,closeSystemTime);
					if(endtime-closeSystemTime>CLOSE_SYSTEM_TIME){
						//SetRecordeVoices_PthreadState(HUASHANG_CLOSE_SYSTEM);
						System_StateLog(".......play music not user close........");
						//printf("\n ............play mp3 endtime=%d  closeSystemTime=%d.............\n",endtime,closeSystemTime);
						SetMucClose_Time(1);
						disable_gpio();
					}
				}else if(GetPlayMusicState()==MAD_PLAY){
					closeSystemTime=time(&t);
				}
			default:
				I2sGetvoicesData();		//默认状态清除音频
				usleep(50000);
				break;
		}
	}
	SetRecordeVoices_PthreadState(RECODE_EXIT_FINNISH);
	DEBUG_VOICES("handle record voices exit\n");
	return NULL;
}
/*****************************************************
开启录音工作线程
*****************************************************/
void InitRecord_VoicesPthread(void){	
	RV =(RecoderVoices_t *)calloc(1,sizeof(RecoderVoices_t));
	if(RV==NULL){
		perror("calloc RecoderVoices_t memory failed");
        exit(-1);
	}
#if 1	
	PlayStartPcm(AMR_11_START_SYSTEM_OK,0);
	sleep(3);
#endif
	
	char playFile[24]={0};
	srand((unsigned)time(NULL));
	int i=(rand()%10)+1;
	if(i%2){
		PlayStartPcm(AMR_9_START_PLAY,0);
	}else{
		PlayStartPcm(AMR_10_START_PLAY,0);
	}
	//PlayStartPcm(AMR_11_START_SYSTEM_OK,0);
#if 0
	sleep(3);	//now 9.amr is 7.amr text  , 10.amr change is 8.amr text for play
#endif
	RV->freeVoicesNum =FREE_VOICE_NUMS;
	if(pthread_create_attr(PthreadRecordVoices,NULL)){
  	  	perror("create handle record voices failed!");
        	exit(-1);
	}
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
