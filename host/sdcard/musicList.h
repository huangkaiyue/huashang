#ifndef _MUSICLIST_H
#define _MUSICLIST_H

#include "config.h"

typedef struct{
	char listname[24];
	int Nums;
	int DirTime;
	int playindex;
}List_t;


#define PLAY_NEXT		1		//播放下一首歌曲
#define PLAY_PREV		2 		//播放上一首歌曲
#define PLAY_RANDOM		3		//随机播放

#ifdef QITUTU_SHI
#define XIMALA_MUSIC	"ximalaya"	//喜马拉雅收藏的歌曲		

//将喜马拉雅的歌曲下载到本地
extern int InsertXimalayaMusic(const char *musicDir,const char *musicName);
extern int DelXimalayaMusic(const char *musicDir,const char *musicName);
#endif
#define MUSIC_LIST	4
extern int SysOnloadMusicList(const char *sdcard,const char *mp3Music,const char *story,const char *english,const char *guoxue);
extern int GetSdcardMusic(const char *sdcard,const char *musicDir,char *getMusicname,unsigned char Mode);
extern int InitMusicList(void);
extern void CleanMusicList(void);

#endif
