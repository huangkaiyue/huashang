#ifndef _MUSICLIST_H
#define _MUSICLIST_H

typedef struct{
	char listname[24];
	int Nums;
	int DirTime;
	int playindex;
}List_t;

#define PLAY_NEXT	1		//������һ�׸���
#define PLAY_PREV	2 		//������һ�׸���

extern int  SysOnloadMusicList(const char *sdcard,const char *mp3Music,const char *story,const char *english);
extern int GetSdcardMusic(const char *sdcard,const char *musicDir,char *getMusicname,unsigned char Mode);
extern int InitMusicList(void);
extern void CleanMusicList(void);

#endif
