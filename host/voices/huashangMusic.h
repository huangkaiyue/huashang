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
//开机加载华上教育内容
extern void openSystemload_huashangData(void);
//获取sdard 内容进行播放
extern int GetScard_forPlayHuashang_Music(unsigned char playMode,unsigned char EventSource);
//关机保存华上数据
extern void closeSystemSave_huashangData(void);
//创建播放音乐列表
extern void CreatePlayListMuisc(const void *data,int musicType);

extern void Huashang_changePlayVoicesName(void);


#endif