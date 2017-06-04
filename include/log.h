#ifndef _LOG_H
#define _LOG_H

void WritePlayUrl_Log(const char *data1,const char *data2);
void PlaySystemAmrVoicesLog(const char *data);
void cleanplayLog(const char *data);
void WriteEventlockLog(const char *data,int lock);
void handleeventLog(const char *data,unsigned char msgSize);
void PlayQtts_log(const char *data);
void udpLog(const char *data);
void RecvTcp_dataLog(const char *data);
void systemTimeLog(const char *data);
void RequestTulingLog(const char *data);
void musicdbLog(const char *data,const char *filename);
void UartLog(const char *data,unsigned char number);
void GpioLog(const char *data,unsigned char number);
void WriteLocalserver_Version(const char *versionMessage);
void Write_Speekkeylog(const char *data,int num);
void test_Save_VoicesPackt_function_log(const char *data,int value);
void Write_tulinglog(const char *logStr);
void WiterSmartConifg_Log(const char *data1,const char *data2);
void Write_huashang_log(const char *data1,const char *data2,int val);
void Write_tulingTextLog(const char *txt);
void Write_huashangTextLog(const char *text);


#endif
