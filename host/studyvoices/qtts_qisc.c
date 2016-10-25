#include "comshead.h"
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "base/queWorkCond.h"
#include "qtts_qisc.h"

#define DBG_QTTS
#ifdef 	DBG_QTTS 
#define DEBUG_QTTS(fmt, args...) printf("QTTS: " fmt, ## args)
#else   
#define DEBUG_QTTS(fmt, args...) { }
#endif	//end DBG_AP_STA

typedef struct _MSPLogin
{
	char appid[9];
	char dvc[9];
}_MspLogin;
_MspLogin login_config;

#define KB 1024
#define QTTS_PLAY_SIZE	12*KB
#define LEN_BUF 1*KB
#define LEN_TAR 2*KB

#define DOWN_QTTS_QUIT	0
#define DOWN_QTTS_ING	1
#define PLAY_QTTS_QUIT	0
#define PLAY_QTTS_ING	1
#define PLAY_QTTS_START	2
#define PLAY_QTTS_EXIT	3

//#define VINN_GBK	"vcn=vinn,aue = speex-wb,auf=audio/L16;rate=8000,spd=7,vol = 8,rdn = 3,tte = gbk"
#define VINN_GBK	"voice_name=vinn,text_encoding=gbk,sample_rate=8000,speed=50,volume=50,pitch=50,rdn =2"
#define VINN_UTF8	"voice_name=vinn,text_encoding=utf8,sample_rate=8000,speed=50,volume=50,pitch=50,rdn =2"
#ifdef VOICS_CH
#define VIMM_GBK	"voice_name=vixx,text_encoding=gbk,sample_rate=8000,speed=50,volume=50,pitch=40,rdn =2"
#endif

typedef struct{
		unsigned char downState:4,playState:4;
		unsigned short cacheSize;
		void (*WritePcm)(char *data,int size);
		WorkQueue * qttsList;
}QttsStream_t;
static QttsStream_t *Qstream=NULL;

#if 0
/*******************************************************
函数功能: 单声道转双声道
参数:	dest_files 输出文件 src_files输入文件
返回值: 0 转换成功 -1 转换失败
********************************************************/
int voices_single_to_stereo(char *dest_files,char *src_files)
{
	FILE  *fd_q,*fd_w;
	size_t read_size;
	int size = 0,pos= 0;
	char buf[LEN_BUF],tar[LEN_TAR];
	fd_q = fopen(src_files,"r");
	fseek(fd_q, WAV_HEAD, SEEK_SET);
	if (NULL == fd_q)
	{
		printf("open file src_voices error\n");
		return -1;
	}
	fd_w = fopen(dest_files,"w+");
	if (NULL == fd_w)
	{
		printf("open file dest_voices error\n");
		return -1;
	}
	while(1)
	{
		read_size=fread(buf, 1,LEN_BUF, fd_q);
		if(read_size==0)
		{
			break;
		}
		while(read_size != size)
		{
			memcpy(tar+pos,buf+size,2);
			pos += 2;
			memcpy(tar+pos,buf+size,2);
			size += 2;
			pos += 2;
		}
		//printf("voices size =%d,len =%d\n",size,len);
      	fwrite(tar, 2*read_size, 1,fd_w);
		pos= 0;
		size = 0;
	}
	fclose(fd_q);
	fclose(fd_w);
	return 0;
}

static void single_to_stereo(char *src,int srclen,char *tar,int *tarlen)
{
	int i=0;
	int pos=0;
	for(i=0; i<srclen; i+=2)
	{
		memcpy(tar+pos,src+i,2);
		pos += 2;
		memcpy(tar+pos,src+i,2);
		pos += 2;
	}
	*tarlen = pos;
}
#endif
/****************************************************
@函数功能:	播放qtts转换数据
@参数:	data 数据
@		len	数据大小
*****************************************************/
static void cleanState(void)
{
	Qstream->playState=PLAY_QTTS_EXIT;
	//Qstream->cacheSize=0;
}
void exitqttsPlay(void)
{
	char *msg;
	int msgSize;
	Qstream->playState=PLAY_QTTS_QUIT;
	clean_qtts_cache();
	while(getWorkMsgNum(Qstream->qttsList)){
		getMsgQueue(Qstream->qttsList,&msg,&msgSize);
		free(msg);
		DEBUG_QTTS("exitqttsPlay: while ...\n");
		usleep(100);
	}
	DEBUG_QTTS("exitqttsPlay: end (%d) ...\n",getWorkMsgNum(Qstream->qttsList));
 }
 static void *play_qtts_data(void *arg)
 {
	 char *data;
	 int len;
	 while(Qstream->playState){
	 	if(getWorkMsgNum(Qstream->qttsList)==0){
			if(Qstream->downState==DOWN_QTTS_QUIT){
				DEBUG_QTTS("play_qtts_data: exit ...\n");
				break;
			}
			usleep(100);
			continue;
		}
#if 0
		if(Qstream->playState==PLAY_QTTS_QUIT){
			usleep(100);
			continue;
		}
#endif
		DEBUG_QTTS("play_qtts_data: while ...\n");
		getMsgQueue(Qstream->qttsList,&data,&len);
		Qstream->WritePcm(data,len);
		free(data);
	}
	cleanState();
	DEBUG_QTTS("play_qtts_data: end (%d) ...\n",getWorkMsgNum(Qstream->qttsList));
	return NULL;
 }
/***************************************************************************
@函数功能:	文本转换语音
@参数:	src_text 文本文件 
@		params	文本转换语音参数
@返回值:	0 成功
@			其它	错误码
***************************************************************************/
static int text_to_speech(const char* src_text  ,const char* params)
{
	char* sess_id = NULL;
	int ret = 0;
	unsigned int text_len = 0;
	unsigned int audio_len = 0;
	int synth_status = 1;
	DEBUG_QTTS("\ntext_to_speech :begin to synth...\n");
	text_len = (unsigned int)strlen(src_text);
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSSessionBegin: qtts begin session failed Error code %d.\n",ret);
		return ret;
	}
	ret = QTTSTextPut(sess_id, src_text, text_len, NULL);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSTextPut: qtts put text failed Error code %d.\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
		return ret;
	}
	stait_qtts_cache();
	//Qstream->cacheSize=0;
	Qstream->downState=DOWN_QTTS_ING;
	//Qstream->playState=PLAY_QTTS_START;
	Qstream->playState=PLAY_QTTS_ING;
	pool_add_task(play_qtts_data,(void *)Qstream);		//启动播放线程
	while(Qstream->playState){
		const void *data = QTTSAudioGet(sess_id, &audio_len, &synth_status, &ret);
		if (NULL != data){
			char *getdata = (char *)malloc(audio_len+1);
			if(getdata ==NULL){
				continue;
			}
			memcpy(getdata,data,audio_len);
			DEBUG_QTTS("putMsgQueue: start (%d) ...\n",getWorkMsgNum(Qstream->qttsList));
			putMsgQueue(Qstream->qttsList,getdata,audio_len);	//添加到播放队列
#if 0
			//Qstream->cacheSize +=audio_len;
			if(Qstream->playState==PLAY_QTTS_START){
				Qstream->playState=PLAY_QTTS_ING;
				//DEBUG_QTTS("play_qtts_data=====%d=====\n",Qstream->cacheSize);
				pool_add_task(play_qtts_data,(void *)Qstream);		//启动播放线程
			}
#endif
		}
		usleep(100*1000);
		if (synth_status==2|| ret!= 0){		//退出
			DEBUG_QTTS("synth_status(%d)ret(%d)\n",synth_status,ret);
			Qstream->downState=DOWN_QTTS_QUIT;
			break;
		}
	}
	DEBUG_QTTS("text_to_speech :while end(ret=%d)...\n",Qstream->playState);
	while(Qstream->downState==DOWN_QTTS_QUIT){		//等待播放线程退出
		if(Qstream->playState!=PLAY_QTTS_ING)
			break;
		usleep(100*1000);
	}
	Qstream->playState=PLAY_QTTS_QUIT;
	DEBUG_QTTS("text_to_speech :TTS end(ret=%d)...\n\n",ret);
	ret = QTTSSessionEnd(sess_id, NULL);
	return ret;
}
/****************************************
@函数功能:	文本转换语音参数选择
@参数:	text 文本文件 
@		VINN_GBK	文本转换语音参数
******************************************/
int Qtts_voices_text(char *text,unsigned char type)
{
	if(type==QTTS_SYS){
		return text_to_speech(text,(const char *)VINN_GBK);//女童音
	}
	else if(type==QTTS_APP){
		return text_to_speech(text,(const char *)VINN_UTF8);//女童音;
	}
}

int init_iat_MSPLogin(void WritePcm(char *data,int size))
{
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
		DEBUG_QTTS("iat MSPLogin failed , Error code %d.\n",ret);
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

void iat_MSPLogout(void)
{
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
	Qtts_voices_text("1234566789",QTTS_SYS);
	return 0;
	FILE *fp =fopen(argv[1],"r");
	if(fp==NULL)
		return -1;
	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char *data = (char *)malloc(len+1);
	fread(data,len,1,fp);
	fclose(fp);
	Qtts_voices_text(data,QTTS_SYS);
	free(data);
	return 0;	
}
#endif
