#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("%s:" ,__func__,fmt, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

typedef struct{
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

#endif
