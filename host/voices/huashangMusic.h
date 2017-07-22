#ifndef _HUASHANGMUSIC_H
#define _HUASHANGMUSIC_H

#define	HUASHANG_MUSIC_TOTAL_NUM 	3838

#define UNKOWN_AIFI_STATE	0	//初始状态
#define	TULING_AIFI_OK		1	//图灵请求成功状态
#define	XUNFEI_AIFI_OK		2	//讯飞请求成功状态
#define XUNFEI_AIFI_FAILED	3	//讯飞识别失败状态
#define XUNFEI_AIFI_ING		4	//讯飞正在识别状态

#define ALLOW_TULING_PLAY	0
#define DISABLE_TULING_PLAY	-1
#define TIMEOUT_AIFI		-2

typedef struct{
	unsigned char playVoicesNameNums;
	unsigned char dirMenu;
	int PlayHuashang_MusicIndex;	//播放华上教育歌曲下表编号 
	int Huashang_MusicTotal;
}HuashangUser_t;

//------------------------------------------------------------------------------
//更新当前目录,并播放
extern void Huahang_SelectDirMenu(void);
//根据当前播放索引，更新目录
extern int Huashang_Update_DirMenu(int PlayHuashang_MusicIndex);
//获取sdcard 歌曲编号进行播放
extern int Huashang_GetScard_forPlayMusic(unsigned char playMode,unsigned char EventSource);
extern void Huashang_updatePlayindex(int playIndex);
extern void Huashang_changePlayVoicesName(void);
extern int Huashang_WeiXinplayMusic(int playIndex);
/**获取播音人**/
extern void Huashang_GetPlayVoicesName(char *playVoicesName,int *speek);
//开机获取华上教育内容播放记录
extern void Huashang_loadSystemdata(void);
extern void Huashang_Init(void);
//关机保存华上教育内容播放记录数据
extern void Huashang_closeSystemSavedata(void);


#endif
