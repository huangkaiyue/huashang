#ifndef _SQL_SQLITE_
#define _SQL_SQLITE_

#include "config.h"

#define OPEN_TIME	1
#define CLOSE_TIME	0


extern void SaveVol_toRouteTable(unsigned char vol);
extern void GetVol_formRouteTable(unsigned char *vol);
extern void Save_OpenCloseTime_toRouteTable(int type,unsigned char *time);
extern void Get_OpenCloseTime_formRouteTable(int type, char *time);

#ifdef VOICS_CH
extern void SaveVolCh_toRouteTable(unsigned char ch);
extern void GetVolCh_formRouteTable(unsigned char *ch);
#endif

//关机时候保存图灵的token值到路由表当中
extern void Save_TulingToken_toRouteTable(const char *tokenVal);

#endif
