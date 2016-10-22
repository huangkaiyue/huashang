#ifndef _QTTS_QISC_H
#define _QTTS_QISC_H

//extern int voices_single_to_stereo(char *dest_files,char *src_files);

#define QTTS_SYS	0
#define QTTS_APP	1

extern int Qtts_voices_text(char *text,unsigned char type);

extern int init_iat_MSPLogin(void WritePcm(char *data,int size));

extern void iat_MSPLogout(void);
//extern void play_qtts_data(char *data,int len);

#endif
