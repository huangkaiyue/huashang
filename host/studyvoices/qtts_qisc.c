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

static char savebuf[1];
static unsigned char cacheFlag=0;	
void initputPcmdata(void){
	cacheFlag=0;
	memset(savebuf,0,1);
}

void setPlayAudioSize(int downSize){
}
//�����ص���pcm������������ӵ����е���
void putPcmStreamToQueue(const void *data,int size){
	int ret =0;
	void *newdata=NULL;
	if(size%2==0&&cacheFlag==0){		//ż����û��Ԥ��
		newdata= (char *)calloc(1,size);
		memcpy(newdata,(void *)data,size);
		ret=size;
		cacheFlag=0;
	}else if(size%2==0&&cacheFlag==1){	//ż������Ԥ��
		newdata= (char *)calloc(1,size);
		memcpy(newdata,savebuf,1);
		memcpy(savebuf,(void *)(data+size-1),1);
		memcpy(newdata+1,(void *)data,size-1);
		ret=size;
		cacheFlag=1;
	}else if(size%2==1&&cacheFlag==1){	//��������Ԥ��
		newdata= (char *)calloc(1,size+1);
		memcpy(newdata,savebuf,1);
		memcpy(newdata+1,(void *)data,size);
		ret=size+1;
		cacheFlag=0;
	}else if(size%2==1&&cacheFlag==0){	//������û��Ԥ��
		newdata= (char *)calloc(1,size-1);
		memcpy(savebuf,(void *)(data+size-1),1);
		memcpy(newdata,(void *)data,size-1);
		ret=size-1;
		cacheFlag=1;
	}
	putPcmDataToPlay((const void *)newdata,ret);	//��ӵ����Ŷ���	
}
static void putStreamVoicesForPlay(const void *userdata,const void *data,int audio_len){
	putPcmStreamToQueue(data,audio_len);

}
/***************************************************************************
@��������:	�ı�ת������
@����:	src_text �ı��ļ� 
@		params	�ı�ת����������
@����ֵ:	0 �ɹ�
@			����	������
***************************************************************************/
static int text_to_speech(const char* src_text,const void *userdata,unsigned char type,const char* playVoicesName,int playSpeed,unsigned int playEventNums,void GetXunFeiVoicesData(const void *userdata,const void *voicesdata,int size)){
	char* sess_id = NULL;
	int ret = -1;
	unsigned int text_len = 0;
	unsigned int audio_len = 0;
	int synth_status = 1;	
	char params[128]={0};
	if(type==QTTS_GBK){
		snprintf(params,128,"voice_name=%s,text_encoding=gbk,sample_rate=8000,speed=%d,volume=50,pitch=50,rdn =3",playVoicesName,playSpeed);
	}
	else if(type==QTTS_UTF8){
		snprintf(params,128,"voice_name=%s,text_encoding=utf8,sample_rate=8000,speed=%d,volume=50,pitch=50,rdn =3",playVoicesName,playSpeed);
	}	
	DEBUG_QTTS("begin to synth...\n");
	text_len = (unsigned int)strlen(src_text);
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSSessionBegin: qtts begin session failed Error code %d\n",ret);
		goto exit0;
	}
	DEBUG_QTTS(" start put text\n");
	ret = QTTSTextPut(sess_id, src_text, text_len, NULL);
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("QTTSTextPut: qtts put text failed Error code %d\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
		goto exit0;
	}
	DEBUG_QTTS(" start QTTSAudioGet\n");
	while(1){
		const void *data = QTTSAudioGet(sess_id, &audio_len, &synth_status, &ret);
		if (NULL != data){
			GetXunFeiVoicesData(userdata,(const void *)data,audio_len);
		}
		usleep(100*1000);
		if (synth_status==2|| ret!= 0){		//�����˳�
			ret=0;
			DEBUG_QTTS(" synth_status ok exit\n");
			break;
		}
		if(GetCurrentEventNums()!=playEventNums){	//��ǰ�¼����л���ֱ�Ӵ������
			ret=-1;
			DEBUG_QTTS(" event break exit\n");
			break;
		}
	}
	QTTSSessionEnd(sess_id, NULL);
exit0:
	DEBUG_QTTS(" exit qtts\n");
	return ret;
}
/****************************************
@��������:	�ı�ת����������ѡ��
@����:	text �ı��ļ� 
@		VINN_GBK	�ı�ת����������
******************************************/
int Qtts_voices_text(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums,int playSpeed){
	return text_to_speech(text,NULL,type,playVoicesName,playSpeed,playEventNums,putStreamVoicesForPlay);
}
typedef struct{
	int audio_len;
	FILE *fp;
}FileQtts_t;
static void WriteXunfei_speeckVoicesToFile(const void *userdata,const void *voicesdata,int size){
	FileQtts_t *fileQtts = (FileQtts_t *)userdata;
	if(fileQtts->fp){
		fwrite(voicesdata,1,size,fileQtts->fp);
		fileQtts->audio_len +=size;
	}
}
int QttsTextVoicesFile(const char *text,unsigned char type,const char *playVoicesName,unsigned int playEventNums,int playSpeed,const char *outFile){
	int ret =-1;
	FileQtts_t fileQtts;
	struct wave_pcm_hdr wavhdr;
	fileQtts.audio_len=0;
	fileQtts.fp = fopen(outFile,"w+");
	int wavHeadSize=sizeof(struct wave_pcm_hdr);	
	memcpy(&wavhdr,&pcmwavhdr,wavHeadSize);	
	if(fileQtts.fp){
		fwrite((void *)&wavhdr,1,wavHeadSize,fileQtts.fp);
		ret = text_to_speech(text,(const void *)&fileQtts,type,playVoicesName,playSpeed,playEventNums,WriteXunfei_speeckVoicesToFile);
		wavhdr.size_8 = (fileQtts.audio_len+36);
		wavhdr.data_size = fileQtts.audio_len;
		fseek(fileQtts.fp,0,SEEK_SET);	
		fwrite(&wavhdr,1,wavHeadSize,fileQtts.fp);
		fclose(fileQtts.fp);
	}
	return ret;
}
int Init_Iat_MSPLogin(void){
	//APPID��������Ķ�
	int ret = 0;
	char login_configs[100];
	strcpy(login_config.appid,"57909831");
	strcpy(login_config.dvc,"hpm10000");
	snprintf(login_configs,100,"appid=%s,dvc=%s, work_dir =   . ",login_config.appid,login_config.dvc);
	ret = MSPLogin(NULL, NULL, login_configs);//�û���¼
	if ( ret != MSP_SUCCESS ){
		DEBUG_QTTS("iat MSPLogin failed , Error code %d\n",ret);
		return -1;
	}
	
	DEBUG_QTTS("iat MSPLogin successfully \n");
	return 0;
}

void Iat_MSPLogout(void){
	MSPLogout();
}
