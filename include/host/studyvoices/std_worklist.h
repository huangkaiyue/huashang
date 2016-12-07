#ifndef _STD_WORKLIST_H_
#define _STD_WORKLIST_H_


#define DBG_STD_MSG
#ifdef DBG_STD_MSG
#define DEBUG_STD_MSG(fmt, args...) printf("std worklist: " fmt, ## args)
#else   
#define DEBUG_STD_MSG(fmt, args...) { }
#endif	//end DBG_STD_MSG

extern void send_voices_server(const char *voicesdata,int len,char *voices_type);
extern int add_event_msg(const char *databuf,int  len,int  type);
extern int getEventNum(void);
extern void init_stdvoices_pthread(void);
extern void clean_stdvoices_pthread(void);

#endif
