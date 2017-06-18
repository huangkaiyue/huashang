#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

#define TULING_TEXT			1
#define TULING_TEXT_MUSIC	2	//����text Я��������(�������)


#define NORMAL_PLAY_PCM		0
#define MIX_PLAY_PCM		1	//�Ե�ǰ���������ز���


typedef struct{
	unsigned char mixMode;			//�������Ŵ���
	unsigned char event;			//��ǰ�����¼�
	unsigned short playLocalVoicesIndex;	//���ű���¼�ƺõ�̨�����
	unsigned int EventNums;			//��ǰ��Ҫ���������¼����
	char *data;		//��Ҫ���������	(�ϴ����������������ݡ����ܵ����������������ı�or���ӵ�ַ ��Ҫ����)
	int dataSize;	//��������ݴ�С
}HandlerText_t;

extern void ReqTulingServer(HandlerText_t *handText,const char *voices_type,const char* asr,int rate);
extern int AddworkEvent(HandlerText_t *handText,int msgSize);
extern int getEventNum(void);
extern void InitEventMsgPthread(void);
extern void CleanEventMsgPthread(void);


#define START_PLAY_VOICES_LIST		1	//��ʼ�����б�����
#define INTERRUPT_PLAY_VOICES_LIST	2	//��ϲ����б�����
#define CLEAN_PLAY_VOICES_LIST		3	//��������б�����
#define END_PLAY_VOICES_LIST		4	//������

#define KEEP_RECORD_STATE			1	//���ֵ�ǰ¼��״̬
#define UPDATE_RECORD_STATE			0	//���µ�ǰ¼��״̬	

extern unsigned char getPlaylistVoicesSate(void);

extern void putPcmDataToPlay(const void * data,int size);
extern int getPlayVoicesQueueNums(void);
extern void SetPlayFinnishKeepRecodeState(int state);


#endif
