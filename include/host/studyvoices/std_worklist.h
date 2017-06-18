#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("%s: "fmt,__func__, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

#define TULING_TEXT			1
#define TULING_TEXT_MUSIC	2	//播放text 携带有音乐(语音点歌)


#define NORMAL_PLAY_PCM		0
#define MIX_PLAY_PCM		1	//对当前声音进行重采样


typedef struct{
	unsigned char mixMode;			//混音播放处理
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


#define START_PLAY_VOICES_LIST		1	//开始播放列表声音
#define INTERRUPT_PLAY_VOICES_LIST	2	//打断播放列表声音
#define CLEAN_PLAY_VOICES_LIST		3	//清除播放列表数据
#define END_PLAY_VOICES_LIST		4	//播放完

#define KEEP_RECORD_STATE			1	//保持当前录音状态
#define UPDATE_RECORD_STATE			0	//更新当前录音状态	

extern unsigned char getPlaylistVoicesSate(void);

extern void putPcmDataToPlay(const void * data,int size);
extern int getPlayVoicesQueueNums(void);
extern void SetPlayFinnishKeepRecodeState(int state);


#endif
