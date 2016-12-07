#ifndef _SQL_SQLITE_
#define _SQL_SQLITE_

#include "config.h"

typedef struct {
	char list[24];
	char passwd[24];
}HOST;
extern HOST host;

extern int updateSysList(char *list,char *passwd);
extern void InitSysList(char *frist_camlist,char *camlist_passwd);
extern unsigned char get_System_Lock(void);
extern void set_System_Lock(int size);
extern void set_vol_size(unsigned char size);
extern void get_vol_size(unsigned char *size);
#ifdef VOICS_CH
extern void set_vol_ch(unsigned char ch);
extern void get_vol_ch(unsigned char *ch);
#endif
#endif
