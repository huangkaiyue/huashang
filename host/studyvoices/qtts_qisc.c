#include "comshead.h"
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "base/queWorkCond.h"
#include "qtts_qisc.h"
#include "host/voices/callvoices.h"
#include "config.h"

typedef struct _MSPLogin{
	char appid[9];
	char dvc[9];
}_MspLogin;
_MspLogin login_config;

typedef struct{
	unsigned char downState:4,playState:4;
	int (*WritePcm)(char *data,int size);
	WorkQueue * qttsList;
}QttsStream_t;
static QttsStream_t *Qstream=NULL;

void __ExitQueueQttsPlay(void){
	Qstream->playState=PLAY_QTTS_WAIT;
	Qstream->downState= DOWN_QTTS_QUIT;
 }
static char savebuf[1];
static unsigned char cacheFlag=0;	
void initputPcmdata(void){
	cacheFlag=0;
	memset(savebuf,0,1);
}

void setPlayAudioSize(int downSize){
}
//将下载到的pcm单声道数据添加到队列当中
void putPcmStreamToQueue(const void *data,int size){
	int ret =0;
	void *newdata=NULL;
	if(size%2==0&&cacheFlag==0){		//偶数，没有预留
		newdata= (char *)calloc(1,size);
		memcpy(newdata,(void *)data,size);
		ret=size;
		cacheFlag=0;
	}else if(size%2==0&&cacheFlag==1){	//偶数，有预留
		newdata= (char *)calloc(1,size);
		memcpy(newdata,savebuf,1);
		memcpy(savebuf,(void *)(data+size-1),1);
		memcpy(newdata+1,(void *)data,size-1);
		ret=size;
		cacheFlag=1;
	}else if(size%2==1&&cacheFlag==1){	//奇数，有预留
		newdata= (char *)calloc(1,size+1);
		memcpy(newdata,savebuf,1);
		memcpy(newdata+1,(void *)data,size);
		ret=size+1;
		cacheFlag=0;
	}else if(size%2==1&&cacheFlag==0){	//奇数，没有预留
		newdata= (char *)calloc(1,size-1);
		memcpy(savebuf,(void *)(data+size-1),1);
		memcpy(newdata,(void *)data,size-1);
		ret=size-1;
		cacheFlag=1;
	}
	putMsgQueue(Qstream->qttsList,newdata,ret);	//添加到播放队列
	
}
static void *GetQueue_Voices_Forplay(void *arg){
	char *data;
	int len=0;
	while(Qstream->playState==PLAY_QTTS_ING){
	 	if(getWorkMsgNum(Qstream->qttsList)==0){
			if(Qstream->downState==DOWN_QTTS_QUIT){
				printf("%s start exit \n",__func__);
				break;
			}
			usleep(1000);
			continue;
		}
		printf("%s : write pcm\n",__func__);
		getMsgQueue(Qstream->qttsList,&data,&len);
		free(data);
		if(Qstream->WritePcm(data,len)){
			break;
		}
	}
	if(Qstream->playState==PLAY_QTTS_WAIT){	//被外部事件设置异常退出,需要清除消息队列里面音频数据
		while(getWorkMsgNum(Qstream->qttsList)){
			getMsgQueue(Qstream->qttsList,&data,&len);
			free(data);
			printf("%s: wait exit ..............\n",__func__);
			usleep(100);
		}
	}
	
	Qstream->playState=PLAY_QTTS_QUIT;
	printf("%s exit ok\n",__func__);
	return NULL;
}

int StartPthreadPlay(void){
	if(Qstream->playState==PLAY_QTTS_WAIT){
		Qstream->downState=DOWN_QTTS_QUIT;
		return -1;
	}
	Qstream->playState=PLAY_QTTS_ING;
	Qstream->downState=DOWN_QTTS_ING;
	pool_add_task(GetQueue_Voices_Forplay,NULL);		//启动播放线程
	usleep(100);
	return 0;
}
//正常情况下等待播放线程退出
void WaitPthreadExit(void){
	Qstream->downState= DOWN_QTTS_QUIT;
	while(Qstream->downState==DOWN_QTTS_QUIT){		//等待播放线程退出
		if(Qstream->playState==PLAY_QTTS_QUIT)
			break;
		//printf("..................... WaitPthreadExit .....................\n ");
		usleep(100*1000);
	}
	Qstream->playState=PLAY_QTTS_QUIT;
	PlayQtts_log("qtts quit ok\n");
	printf("..................... WaitPthreadExit ok.....................\n ");
}
/***************************************************************************
@函数功能:	文本转换语音
@参数:	src_text 文本文件 
@		params	文本转换语音参数
@返回值:	0 成功
@			其它	错误码
***************************************************************************/
static int text_to_speech(const char* src_text  ,const char* params){
	char* sess_id = NULL;
	int ret = -1;
	unsigned int text_len = 0;
	unsigned int audio_len = 0;
	int synth_status = 1;
	if(StartPthreadPlay()){
		return -1;
	}
	DEBUG_QTTS("\ntext_to_speech :begin to synth...\n");
	text_len = (unsigned int)strlen(src_text);
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSSessionBegin: qtts begin session failed Error code %d\n",ret);
		return ret;
	}
	ret = QTTSTextPut(sess_id, src_text, text_len, NULL);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSTextPut: qtts put text failed Error code %d\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
		return ret;
	}
	while(Qstream->downState==DOWN_QTTS_ING){
		const void *data = QTTSAudioGet(sess_id, &audio_len, &synth_status, &ret);
		if (NULL != data){
			putPcmStreamToQueue(data,audio_len);
		}
		usleep(100*1000);
		if (synth_status==2|| ret!= 0){		//退出
			break;
		}
		printf("%s: down file %d\n",__func__,Qstream->downState);
	}
	WaitPthreadExit();
	ret = QTTSSessionEnd(sess_id, NULL);
	return 0;
}
/****************************************
@函数功能:	文本转换语音参数选择
@参数:	text 文本文件 
@		VINN_GBK	文本转换语音参数
******************************************/
int Qtts_voices_text(char *text,unsigned char type){
	if(type==QTTS_GBK){
		return text_to_speech(text,(const char *)VIMM_GBK);//女童音
	}
	else if(type==QTTS_UTF8){
		return text_to_speech(text,(const char *)VIMM_UTF8);//女童音
	}
}

int init_iat_MSPLogin(int WritePcm(char *data,int size)){
	//APPID请勿随意改动
	//const char* login_configs = "appid = 55253ad2,dvc=hpm10000, work_dir =   .  ";
	int ret = 0;
	char login_configs[100];
#if 0
	strcpy(login_config.appid,"55253ad2");
#else
	strcpy(login_config.appid,"57909831");
#endif
	strcpy(login_config.dvc,"hpm10000");
	snprintf(login_configs,100,"appid=%s,dvc=%s, work_dir =   . ",login_config.appid,login_config.dvc);
	
	//用户登录
	ret = MSPLogin(NULL, NULL, login_configs);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("iat MSPLogin failed , Error code %d\n",ret);
		return -1;
	}
	Qstream = (QttsStream_t *)calloc(1,sizeof(QttsStream_t));
	if(Qstream==NULL){
		DEBUG_QTTS("iat QttsStream_t failed \n");
		return -1;
	}
	
	Qstream->qttsList = initQueue();
	if(Qstream->qttsList ==NULL){
		DEBUG_QTTS("iat qttsList failed \n");
		goto exit;
	}
	Qstream->WritePcm = WritePcm;
	DEBUG_QTTS("iat MSPLogin successfully \n");
	return 0;
exit:
	free(Qstream);
	return -1;
}

void iat_MSPLogout(void){
	MSPLogout();
	char *msg;
	int msgSize;
	if(Qstream){
		while(getWorkMsgNum(Qstream->qttsList)){
			getMsgQueue(Qstream->qttsList,&msg,&msgSize);
			free(msg);
		}
		destoryQueue(Qstream->qttsList);
		free(Qstream);
	}
}
#if 0
struct wave_pcm_hdr pcmwavhdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	8000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};
int main(int argc,char **argv)
{
	if(argc<2)
	{
		exit(0);
	}
	Qtts_voices_text("1234566789",QTTS_GBK);
	return 0;
	FILE *fp =fopen(argv[1],"r");
	if(fp==NULL)
		return -1;
	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char *data = (char *)malloc(len+1);
	if(data==NULL){
		perror("calloc error !!!");
		return;
	}
	fread(data,len,1,fp);
	fclose(fp);
	Qtts_voices_text(data,QTTS_GBK);
	free(data);
	return 0;	
}
#endif
