#include "comshead.h"
#include "sysdata.h"
#include "config.h"
#include "nvram.h"
#include "host/voices/callvoices.h"
#include "../host/studyvoices/qtts_qisc.h"


//----------------------------Ȩ�޴���-------------------------------------
#ifdef	SYSTEMLOCK
//����Ȩ�޴���
void setSystemLock(int size){
	char buf_s[64]={0};
	sprintf(buf_s,"%s %d", "nvram_set 2860 SystemLock", size);
	system(buf_s);
	//nvram_bufset(RT2860_NVRAM, "VoiceSIZE",buf_s);
}
//��ȡȨ�޴���,����Ƿ񵽴�����
void checkSystemLock(void){
	char *number = nvram_bufget(RT2860_NVRAM, "SystemLock");
	int opennumber = (unsigned char)atoi(number);
	if(opennumber>SYSTEMLOCKNUM){	//��鿪������
		Create_PlayQttsEvent("Ȩ�޴�������������ϵ���������˾������������Ѷ���޹�˾���������������ƹ� QQ ��121109281��",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("Ȩ�޴�������������ϵ���������˾������������Ѷ���޹�˾���������������ƹ� QQ ��121109281��",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("Ȩ�޴�������������ϵ���������˾������������Ѷ���޹�˾���������������ƹ� QQ ��121109281��",QTTS_GBK);
		sleep(10);
		Create_PlayQttsEvent("Ȩ�޴�������������ϵ���������˾������������Ѷ���޹�˾���������������ƹ� QQ ��121109281��",QTTS_GBK);
		sleep(10);
		SetMucClose_Time(1);
	}
	setSystemLock((opennumber+1));
}
#endif

//----------------------------����-------------------------------------
//��·�ɱ��л�ȡ����
void GetVol_formRouteTable(unsigned char *size){
	char *vol = nvram_bufget(RT2860_NVRAM, "VoiceSIZE");
	if(!strcmp(vol,"")){
		*size=105;
	}else{
		*size = (unsigned char)atoi(vol);
	}
}

//����������·�ɱ���
void SaveVol_toRouteTable(unsigned char vol){
	char buf_s[64]={0};
	sprintf(buf_s,"nvram_set 2860 VoiceSIZE %d", vol);
	system(buf_s);
}
//----------------------------���ػ�-------------------------------------
//���ÿ��ػ���·�ɱ���
void Save_OpenCloseTime_toRouteTable(int type,unsigned char *time){
	char buf_s[64]={0};
	if(type==0)	//�ػ�ʱ��
		sprintf(buf_s,"nvram_set 2860 Closetime %s", time);
	else if (type==1)//����ʱ��
		sprintf(buf_s,"nvram_set 2860 Opentime %s", time);
	system(buf_s);
}

//��ȡ���ػ�ʱ��
void Get_OpenCloseTime_formRouteTable(int type, char *time){
	char *buf=NULL;
	if(type==0){
		buf= nvram_bufget(RT2860_NVRAM, "Closetime");
	}
	else if (type==1){
		buf= nvram_bufget(RT2860_NVRAM, "Opentime");
	}
	memcpy(time,buf,strlen(buf));
}

//�ػ�ʱ�򱣴�ͼ���tokenֵ��·�ɱ���
void Save_TulingToken_toRouteTable(const char *tokenVal){
	char buf_s[128]={0};
	sprintf(buf_s,"nvram_set 2860 tokenVal %s", tokenVal);
	system(buf_s);
}
//----------------------------end-------------------------------------
