#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("%s:" ,__func__,fmt, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

typedef struct{
	unsigned char event;			//当前处理事件
	unsigned short playLocalVoicesIndex;	//播放本地录制好的台本编号
	unsigned int EventNums;			//当前需要处理数据事件编号
	char *data;		//需要处理的数据	(上传语音到服务器数据、接受到服务器返回来的文本or链接地址 需要播放)
	int dataSize;	//处理的数据大小
}HandlerText_t;


extern void ReqTulingServer(HandlerText_t *handText,const char *voices_type,const char* asr,int rate);
extern int AddworkEvent(HandlerText_t *handText,int msgSize);
extern int getEventNum(void);
extern void InitEventMsgPthread(void);
extern void CleanEventMsgPthread(void);

#endif
