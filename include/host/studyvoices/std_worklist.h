#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("%s:" ,__func__,fmt, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

extern void ReqTulingServer(const char *voicesdata,int len,const char *voices_type,const char* asr,int rate);
extern int AddworkEvent(const char *databuf,int  len,int  type);
extern int getEventNum(void);
extern void InitEventMsgPthread(void);
extern void CleanEventMsgPthread(void);

#endif
