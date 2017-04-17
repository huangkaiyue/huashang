#ifndef _HUASHANGMUSIC_H
#define _HUASHANGMUSIC_H

#define	HUASHANG_MUSIC_TOTAL_NUM 	3819

#define UNKOWN_AIFI_STATE	0	//初始状态
#define	TULING_AIFI_OK		1	//图灵请求成功状态
#define	XUNFEI_AIFI_OK		2	//讯飞请求成功状态
#define XUNFEI_AIFI_FAILED	3	//讯飞识别失败状态
#define XUNFEI_AIFI_ING		4	//讯飞正在识别状态

#define ALLOW_TULING_PLAY	0
#define DISABLE_TULING_PLAY	-1

//开机加载华上教育内容
extern void openSystemload_huashangData(void);
//获取sdard 内容进行播放
extern int GetScard_forPlayHuashang_Music(unsigned char playMode,const void *playDir);
//关机保存华上数据
extern void closeSystemSave_huashangData(void);
//创建播放音乐列表
extern void CreatePlayListMuisc(const void *data,int musicType);
//华上教育按键按下，播放按键音
extern void Huashang_keyDown_playkeyVoices(int state);
//设置aifi 语音识别状态
extern void SetAifi_voicesState(unsigned char aifiState);
//获取设置aifi语音识别状态
extern int GetAifi_voicesState(void);
//检查图灵语音添加权限
extern int check_tuingAifiPermison(void);
//发送华上离线语音识别
extern void Huashang_SendnotOnline_xunfeiVoices(const char *filename);
//获取华上讯飞离线语音识别结果
extern void GetHuashang_xunfei_aifiVoices(const char *xunfeiAifi);
//获取华上讯飞离线语音识别失败
extern void GetHuashang_xunfei_aifiFailed(void);

#endif