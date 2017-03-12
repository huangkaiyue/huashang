#include "comshead.h"
#include "host/voices/wm8960i2s.h"
#include "host/voices/WavAmrCon.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "base/pool.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "gpio_7620.h"
#include "../studyvoices/qtts_qisc.h"
#include "config.h"

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

#define KB	1024
static char buf_voices[STD_RECODE_SIZE];
static char pcm_voices16k[STD_RECODE_SIZE_16K];
static char amr_data[12*KB];
static int len_voices = 0;
//#define PCM_TEST
#ifdef PCM_TEST
void test_save8kpcm(char *Testbuf,int len){
	char file[128]={0};
	time_t ti;
	time(&ti);
	snprintf(file,128,"./%d.8k",(int)ti);
	FILE *fp=fopen(file,"w+");
	if(fp==NULL)
		return;
	fwrite(Testbuf,1,len,fp);
	fclose(fp);
}
void test_save16kpcm(char *Testbuf,int len){
	char file[128]={0};
	time_t ti;
	time(&ti);
	snprintf(file,128,"./%d.16k",(int)ti);
	FILE *fp=fopen(file,"w+");
	if(fp==NULL)
		return;
	int pos=0;
	for(pos=0;pos<len;pos+=2){
		fwrite(Testbuf+pos,1,2,fp);
		fwrite(Testbuf+pos,1,2,fp);
	}
	fclose(fp);
}
#endif
#ifdef MY_HTTP_REQ
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
#else
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
#endif
/*****************************************************
*获取状态
*****************************************************/
int GetRecordeLive(void){
	return sysMes.recorde_live;
}
/*****************************************************
*设置状态
*****************************************************/
static int SetRecordeLive(int i){
	printf("=================SetRecordeLive( %d )========== %d =============\n",sysMes.recorde_live,i);
	sysMes.recorde_live=i;
}

/*****************************************************
*开启语音识别状态
*****************************************************/
void start_event_std(void){	
	SetRecordeLive(START_SPEEK_VOICES);
}
/*****************************************************
*结束语音识别状态
*****************************************************/
void end_event_std(void){
	printf("end_event_std \n");
	if(GetRecordeLive() ==START_SPEEK_VOICES)
		SetRecordeLive(END_SPEEK_VOICES);
}
/*****************************************************
*进入播放wav原始数据状态
*****************************************************/
void start_event_play_wav(int i){
	//printf("start_event_play_wav( %d )========== %d =============\n",sysMes.recorde_live,i);
	SetRecordeLive(PLAY_WAV);
}
void start_speek_wait(void){
	SetRecordeLive(SPEEK_WAIT);
}
void start_play_tuling(void){
	SetRecordeLive(PLAY_TULING);
}
/*****************************************************
*进入播放URL状态
*****************************************************/
void start_event_play_url(void){
	SetRecordeLive(PLAY_URL);
}
/*****************************************************
*暂停录音状态
*****************************************************/
void pause_record_audio(int i){	
	//printf("pause_record_audio( %d )========== %d =============\n",sysMes.recorde_live,i);
	SetRecordeLive(RECODE_PAUSE);
}

/*****************************************************
*录制退出状态
******************************************************/
void exit_handle_event(void){
	SetRecordeLive(RECODE_STOP);
}
/*****************************************************
*录制短消息状态
******************************************************/
void start_event_talk_message(void){
	SetRecordeLive(START_TAIK_MESSAGE);
}
#ifdef TIMEOUT_CHECK
/*****************************************************
*保存上一次切换状态和恢复状态
*****************************************************/
void keep_recorde_live(int change){
	if(change==1)
		sysMes.oldrecorde_live=sysMes.recorde_live;
	else
		sysMes.recorde_live=sysMes.oldrecorde_live;
}
#endif	//end TIMEOUT_CHECK
static void TaiBenToTulingNOVoices(void){
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case 1:
			playsysvoices(NO_VOICES);
			break;
		case 2:
			playsysvoices(NO_VOICES_1);
			break;
		case 3:
			playsysvoices(NO_VOICES_2);
			break;
		case 4:
			playsysvoices(NO_VOICES_3);
			break;
		case 5:
			playsysvoices(NO_VOICES_4);
			break;
		case 6:
			playsysvoices(NO_VOICES_5);
			break;
		case 7:
			playsysvoices(NO_VOICES_6);
			break;
		case 8:
			playsysvoices(NO_VOICES_7);
			break;
		case 9:
			playsysvoices(NO_VOICES_8);
			break;
		case 10:
			playsysvoices(NO_VOICES_9);
			break;
	}
}
/****************************************
@函数功能:	处理采集数据函数
@参数:	data 采集到的数据  size 数据大小
*****************************************/
static void voices_packt(const char *data,int size){
	int amr_size=0;
	int i;
	if(data != NULL){
		//大于5秒的音频丢掉
		if((len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){
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
	}
	else if(len_voices > VOICES_MIN)	//音频上传
	{
#ifdef DATOU_JIANG
		led_lr_oc(openled);
#endif
#if 1
		start_event_play_wav(1);		//播放wav
		pool_add_task(play_sys_tices_voices,TULING_WINT);
		//usleep(1000*1000);
		usleep(100);
#else
		Create_PlaySystemEventVoices(2);
#endif
		DEBUG_VOICES("len_voices = %d  \n",len_voices);

#ifdef AMR_DATA		
		pcmwavhdr.size_8 = (len_voices+36);
		pcmwavhdr.data_size = len_voices;
		memcpy(buf_voices,&pcmwavhdr,WAV_HEAD);				//写音频头
		if(WavToAmr8k((const char *)buf_voices,amr_data,&amr_size)){	//转换成amr格式
			DEBUG_VOICES_ERROR("enc failed \n");
		}
		ReqTulingServer((const char *)amr_data,amr_size,"amr","3",RECODE_RATE);
#else
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
	pcmVoice8kTo16k(buf_voices+WAV_HEAD,pcm_voices16k+WAV_HEAD,len_voices);
	pcmwavhdr.size_8 = (len_voices*2+36);
	pcmwavhdr.data_size = len_voices*2;
	pcmwavhdr.samples_per_sec=16000;
	memcpy(pcm_voices16k,&pcmwavhdr,WAV_HEAD); 			//写音频头
	if(WavAmr16k((const char *)pcm_voices16k,amr_data,&amr_size)){	//转换成amr格式
		DEBUG_VOICES_ERROR("enc failed \n");
	}
	ReqTulingServer((const char *)amr_data,amr_size,"amr","3",RECODE_RATE*2);
#endif
#endif
		goto exit0;
	}
	else if(len_voices < VOICES_ERR){
		goto exit1;	//误触发
	}
	else{
		if(GetRecordeLive() !=PLAY_WAV){
#if 1
			TaiBenToTulingNOVoices();
#else
			playsysvoices(NO_VOICES);
#endif
		}
		goto exit1;
	}
	return ;
exit1:
	pause_record_audio(1);
exit0:
	memset(buf_voices,0,len_voices);
	len_voices = 0;
	return ;
}
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
static void *pthread_record_voices(void *arg){
	char *pBuf;
	SetRecordeLive(RECODE_PAUSE);
	time_t t;
	int endtime,starttime;
	starttime=time(&t);
	endtime=time(&t);
	while(GetRecordeLive()!=RECODE_STOP){
#ifdef CLOCESYSTEM
		endtime=time(&t);
		if((endtime-starttime)>ERRORTIME){
			starttime=time(&t);
			sysMes.Playlocaltime=time(&t);
		}else{
			if(GetRecordeLive()==RECODE_PAUSE){
				if((endtime-starttime)==10){
					printf("=============MUTE====%d===========\n",endtime-starttime);
					Mute_voices(MUTE);
				}
				if((endtime-starttime)==SYSTEMOUTSIGN){		//长时间不触发事件，则关闭
					SetRecordeLive(TIME_SIGN);
				}
				if((endtime-starttime)>SYSTEMOUTTIME){		//长时间不触发事件，则关闭
					SetRecordeLive(TIME_OUT);
					TimeLog("TIME_OUT\n");
				}
			}else{
				starttime=time(&t);
			}
			if((endtime-sysMes.Playlocaltime)>PLAYOUTTIME&&GetRecordeLive()==PLAY_URL){
				SetRecordeLive(PLAY_OUT);
				TimeLog("PLAY_OUT\n");
			}
		}
#endif
		switch(GetRecordeLive()){
			case START_SPEEK_VOICES:	//会话录音
				pBuf = I2sGetvoicesData();
				voices_packt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:		//会话录音结束
				voices_packt(NULL, 0);	//发送空音频作为一段音频的结束标志
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
				SetRecordeLive(PLAY_URL);
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
	SetRecordeLive(RECODE_EXIT_FINNISH);
	DEBUG_VOICES("handle record voices exit\n");
	return NULL;
}
#ifdef TEST_MIC
/*******************************************************
函数功能: 测试录音并直接播放出来
参数:
返回值:
********************************************************/
static void *test_recorde_play(void){
	char *pBuf;
	while(GetRecordeLive()!=RECODE_STOP)
	{
		DEBUG_VOICES("test_recorde_play :recorde now...\n");
		pBuf = I2sGetvoicesData();			//占用的时间有点长
		memcpy(play_buf,pBuf,I2S_PAGE_SIZE);	
		write_pcm(play_buf);
		//usleep(3000*1000);
	}
}
#endif	//end TEST_MIC
/*****************************************************
开启录音工作线程
*****************************************************/
void InitRecord_VoicesPthread(void)
{	
#ifdef TEST_MIC
	if(pthread_create_attr(test_recorde_play,NULL)){
		perror("create test recorde play failed!");
		exit(-1);
	}
#else
	if(pthread_create_attr(pthread_record_voices,NULL)){
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
	SetRecordeLive(RECODE_STOP);
	while(GetRecordeLive()!=RECODE_EXIT_FINNISH){
		DEBUG_VOICES("wait record audio exit recorde_live(%d)\n",sysMes.recorde_live);
		usleep(100000);
		if(GetRecordeLive()!=RECODE_EXIT_FINNISH)
			SetRecordeLive(RECODE_STOP);
		if(timeout++>50)
			break;
	}
	DEBUG_VOICES("exit record pthread success recorde_live(%d)\n",GetRecordeLive());
}
