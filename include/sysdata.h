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
extern void get_paly_num(int *size,unsigned char str);
extern void set_paly_num(int size,unsigned char str);
extern void get_paly_sys_num(void);
extern void set_paly_sys_num(void);
extern void set_vol_size(unsigned char size);
extern void get_vol_size(unsigned char *size);
#ifdef VOICS_CH
extern void set_vol_ch(unsigned char ch);
extern void get_vol_ch(unsigned char *ch);
#endif
#endif
