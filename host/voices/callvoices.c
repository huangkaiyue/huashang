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
#include "uart/uart.h"

#if defined(HUASHANG_JIAOYU)
#include "huashangMusic.h"
#endif
static RecoderVoices_t *RV=NULL;

//Ĭ����Ƶͷ������
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

#if defined(MY_HTTP_REQ)	//��¼�Ƶ�8k����ת����16k����
static void pcmVoice8kTo16k(const char *inputdata,char *outputdata,int inputLen){
	int pos=0,npos=0;
	for(pos=0;pos<inputLen;pos+=2){
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
		memcpy(outputdata+npos,inputdata+pos,2);
		npos+=2;
	}
}
#else					//��8k����ת����16k��������д�뵽�ļ�����
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
*��ȡ״̬
*****************************************************/
int GetRecordeVoices_PthreadState(void){
	return RV->recorde_live;
}

/*****************************************************
*����״̬
*****************************************************/
static void SetRecordeVoices_PthreadState(unsigned char state){
	printf("%s: recorde_live %d  %d \n",__func__,RV->recorde_live,state);
	RV->recorde_live=state;
}

/*****************************************************
*��������ʶ��״̬
*****************************************************/
void StartTuling_RecordeVoices(void){	
	SetRecordeVoices_PthreadState(START_SPEEK_VOICES);
}
/*****************************************************
*��������ʶ��״̬
*****************************************************/
void StopTuling_RecordeVoices(void){
	if(GetRecordeVoices_PthreadState() ==START_SPEEK_VOICES){
		SetRecordeVoices_PthreadState(END_SPEEK_VOICES);
		showFacePicture(WAIT_CTRL_NUM4);
	}
}
/*****************************************************
*���벥��wavԭʼ����״̬
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
*���벥��Mp3music״̬
*****************************************************/
void start_event_play_Mp3music(void){
	SetRecordeVoices_PthreadState(PLAY_MP3_MUSIC);
}
//��������
void start_event_play_soundMix(void){
	SetRecordeVoices_PthreadState(SOUND_MIX_PLAY);
}

/*****************************************************
*��ͣ¼��״̬
*****************************************************/
void pause_record_audio(void){
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
}
void lock_pause_record_audio(void){
	if(RV->recorde_live==PLAY_WAV){
		SetRecordeVoices_PthreadState(RECODE_PAUSE);
	}else{
		printf("%s cannot interrupt state \n",__func__);
	}
}


void closeSystem(unsigned char eventInterrupt){
	RV->WaitSleep=0;
	systemTimeLog("close system");
	SleepRecoder_Phthread();
	RV->closeTime =0;
#if defined(HUASHANG_JIAOYU)	
	led_lr_oc(openled);
	Close_tlak_Light();
	if(eventInterrupt){
		Create_PlayImportVoices(HUASHANG_SLEEP_VOICES);
	}else{
		usleep(100000);
		showFacePicture(WAIT_CTRL_NUM1);
	}
#else
	SetMucClose_Time(1);	//����һ���Ӻ�ػ�
#endif
}
void SleepRecoder_Phthread(void){
	SetRecordeVoices_PthreadState(HUASHANG_SLEEP);
}
int SleepSystem(void){
	if(++RV->WaitSleep>=4){
		printf("-----------------------\n close system --------------------\n");
		closeSystem(1);
		return -1;
	}
	return 0;
}
void WaitSleepSystem(void){
	RV->WaitSleep =0;
}

/*****************************************************
*¼�ƶ���Ϣ״̬
******************************************************/
void start_event_talk_message(void){
	SetRecordeVoices_PthreadState(START_TAIK_MESSAGE);
}

static void *uploadPcmPthread(void *arg){
	HandlerText_t *handText = (HandlerText_t *)arg;
#if defined(MY_HTTP_REQ)
	ReqTulingServer(handText,"pcm","0",RECODE_RATE*2);
#else
	PcmVoice8kTo16k_File(RV->buf_voices,"pcm16k.cache",RV->len_voices);
	ReqTulingServer((const char *)"pcm16k.cache",RV->len_voices*2,"pcm","0",RECODE_RATE*2);
#endif
	RV->uploadState = END_UPLOAD;
	return NULL;
}
/****************************************
@��������:	��ʼ�ϴ�������������
@����:	��
*****************************************/
static void Start_uploadVoicesData(void){
	if(RV->uploadState==START_UPLOAD){
		printf("%s add upload voices failed \n",__func__);
		return ;
	}
	RV->uploadState = START_UPLOAD;
	start_play_tuling();	//���õ�ǰ����״̬Ϊ : �����ϴ�����
#if defined(HUASHANG_JIAOYU)

#else
	Create_PlayImportVoices(TULING_WAIT_VOICES);
	usleep(200);
#endif	
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
@��������:	����ɼ����ݺ���
@����:	data �ɼ���������  size ���ݴ�С
*****************************************/
static void Save_VoicesPackt(const char *data,int size){
	int i;
	if(data != NULL){
		if((RV->len_voices+size) > (STD_RECODE_SIZE-WAV_HEAD)){//����5�����Ƶ����
			printf("%s > STD_RECODE_SIZE RV->len_voices=%d\n",__func__,RV->len_voices);
			test_Save_VoicesPackt_function_log((const char *)"STD_RECODE_SIZE is >5s",RV->len_voices);
			goto exit1;
		}
#if defined(HUASHANG_JIAOYU)
		//if(--RV->freeVoicesNum>0){
		//	return ;
		//}
#endif
#if 0		
		//ֻ�������������ݣ�������û�У���Ҫ����2��3��������
		for(i=2; i<size; i+=4){
			//˫��������ת�ɵ���������
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
		if(RV->len_voices > VOICES_MIN)	//����0.5s ��Ƶ�����ϴ������������п�ʼʶ��  13200
		{
			test_Save_VoicesPackt_function_log((const char *)"start upload",GetRecordeVoices_PthreadState());
			printf("%s Start_uploadVoicesData RV->len_voices=%d\n",__func__,RV->len_voices);
			Start_uploadVoicesData();		//��ʼ�ϴ�����
			goto exit0;
		}
		else if(RV->len_voices < VOICES_ERR){			//
			printf("%s < VOICES_ERR RV->len_voices=%d\n",__func__,RV->len_voices);
			test_Save_VoicesPackt_function_log((const char *)"< VOICES_ERR",GetRecordeVoices_PthreadState());
			goto exit1;	//�󴥷�
		}
		else{	//VOICES_ERR --->VOICES_MIN �������Ƶ���϶�Ϊ��Ч��Ƶ
			//Create_PlaySystemEventVoices(AI_KEY_TALK_ERROR);
			printf("%s < VOICES_MIN RV->len_voices=%d\n",__func__,RV->len_voices);
			test_Save_VoicesPackt_function_log((const char *)"< VOICES_MIN",GetRecordeVoices_PthreadState());
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
//�ػ�������ǰϵͳʱ�䷢�͸�MCU
int SetMucClose_Time(unsigned char closeTime){
	time_t timep;
	struct tm *p;
	time(&timep);
	char syscloseTime[64]={0};
	SocSendMenu(3,0);
	usleep(100*1000);
	p=localtime(&timep); /*ȡ�õ���ʱ��*/
	if((p->tm_hour)+8>=24){
		p->tm_hour=(p->tm_hour)+8-24;
	}else{
		p->tm_hour=(p->tm_hour)+8;
	}
	sprintf(syscloseTime,"%d:%d",(p->tm_hour),(p->tm_min)+closeTime);
	printf("SetMucClose_Time : %s\n",syscloseTime);
	SocSendMenu(2,syscloseTime);
	return 0;
}
/*******************************************************
��������: ¼���߳� ���Ե�ǰ������⣬ɸѡ��Ч��Ƶ
����:   	��
����ֵ: 
********************************************************/
static void *PthreadRecordVoices(void *arg){
	char *pBuf;
	SetRecordeVoices_PthreadState(RECODE_PAUSE);
	time_t t;
	int endtime,starttime;
	starttime=time(&t);
	endtime=time(&t);
	while(GetRecordeVoices_PthreadState()!=RECODE_STOP){
		endtime=time(&t);
		//printf("%s: time=%d  =GetRecordeVoices_PthreadState =%d\n",__func__,endtime-starttime,GetRecordeVoices_PthreadState());
		if((endtime-starttime)>ERRORTIME){	//����ʱ��û�л�ȡ����ʱ�䣬����ʱ������
			starttime=time(&t);
		}else{
			if(GetRecordeVoices_PthreadState()==RECODE_PAUSE){
				if((endtime-starttime)==LONG_TIME_NOT_USER_MUTE_VOICES){	//10s ֮�󣬲��ùر���Ƶ
					Mute_voices(MUTE);
				}
				if((endtime-starttime)>SYSTEMOUTSIGN){		//��һ�γ�ʱ�䲻�����¼�����ر�
					SetRecordeVoices_PthreadState(TIME_SIGN);
				}
			}else{
				starttime=time(&t);
			}		
		}
		switch(GetRecordeVoices_PthreadState()){
			case START_SPEEK_VOICES:	//�Ự¼��
				pBuf = I2sGetvoicesData();
				Save_VoicesPackt((const char *)pBuf,I2S_PAGE_SIZE);
				break;
			case END_SPEEK_VOICES:		//�Ự¼������
				Save_VoicesPackt(NULL, 0);	//���Ϳ���Ƶ��Ϊһ����Ƶ�Ľ�����־
				break;
			case START_TAIK_MESSAGE:	//�Խ�¼��
				pBuf = I2sGetvoicesData();
				SaveRecorderVoices((const char *)pBuf,I2S_PAGE_SIZE);
				usleep(5000);			//�����п���ĸо�
				break;

			case TIME_SIGN:				//��ʾ��Ϣ�ܾ���
				systemTimeLog("time out for play music");
				if(!SleepSystem())	{
					Create_PlaySystemEventVoices(MIN_10_NOT_USER_WARN);
				}
				sleep(1);
				break;
			case PLAY_DING_VOICES:
				usleep(50000);
				break;
			case HUASHANG_SLEEP:		//����˯��״̬
				if(++RV->closeTime==60){
					showFacePicture(CLEAR_SYSTEM_PICTURE);
					SetRecordeVoices_PthreadState(HUASHANG_SLEEP_OK);
					RV->closeTime=0;
				}
				I2sGetvoicesData();		//Ĭ��״̬�����Ƶ
				usleep(50000);
				break;
				
			default:
				I2sGetvoicesData();		//Ĭ��״̬�����Ƶ
				usleep(50000);
				break;
		}
	}
	SetRecordeVoices_PthreadState(RECODE_EXIT_FINNISH);
	DEBUG_VOICES("handle record voices exit\n");
	test_Save_VoicesPackt_function_log("recorde pthread exit",0);
	return NULL;
}
static void *start_playHuashang(void *arg){
	int eventsNum = updateCurrentEventNums();
	PlayStartPcm("qtts/start_10.amr",eventsNum);
	return NULL;
}
/*****************************************************
����¼�������߳�
*****************************************************/
void InitRecord_VoicesPthread(void){	
	RV =(RecoderVoices_t *)calloc(1,sizeof(RecoderVoices_t));
	if(RV==NULL){
		perror("calloc RecoderVoices_t memory failed");
        exit(-1);
	}
#if defined(HUASHANG_JIAOYU)
	char playFile[24]={0};
	srand((unsigned)time(NULL));
	int i=(rand()%5)+1;
	if(i>=5)
		i=5;
	//i=5;
	snprintf(playFile,24,"qtts/start_%d.amr",i);
	PlayStartPcm(playFile,0);
	//pool_add_task(start_playHuashang,NULL);
	RV->freeVoicesNum =FREE_VOICE_NUMS;
#else
	//PlayImportVoices(START_SYS_VOICES,0);
	PlayStartPcm(START_SYS_VOICES,0);//����������
#endif

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
�˳�¼��(�˳�ϵͳ)
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

	//���̡߳��������硢¼���������̡߳��¼������̡߳������̡߳������߳�
}
