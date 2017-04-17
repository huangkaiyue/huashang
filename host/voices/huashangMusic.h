#ifndef _HUASHANGMUSIC_H
#define _HUASHANGMUSIC_H

#define	HUASHANG_MUSIC_TOTAL_NUM 	3819

#define UNKOWN_AIFI_STATE	0	//��ʼ״̬
#define	TULING_AIFI_OK		1	//ͼ������ɹ�״̬
#define	XUNFEI_AIFI_OK		2	//Ѷ������ɹ�״̬
#define XUNFEI_AIFI_FAILED	3	//Ѷ��ʶ��ʧ��״̬
#define XUNFEI_AIFI_ING		4	//Ѷ������ʶ��״̬

#define ALLOW_TULING_PLAY	0
#define DISABLE_TULING_PLAY	-1

//�������ػ��Ͻ�������
extern void openSystemload_huashangData(void);
//��ȡsdard ���ݽ��в���
extern int GetScard_forPlayHuashang_Music(unsigned char playMode,const void *playDir);
//�ػ����滪������
extern void closeSystemSave_huashangData(void);
//�������������б�
extern void CreatePlayListMuisc(const void *data,int musicType);
//���Ͻ����������£����Ű�����
extern void Huashang_keyDown_playkeyVoices(int state);
//����aifi ����ʶ��״̬
extern void SetAifi_voicesState(unsigned char aifiState);
//��ȡ����aifi����ʶ��״̬
extern int GetAifi_voicesState(void);
//���ͼ���������Ȩ��
extern int check_tuingAifiPermison(void);
//���ͻ�����������ʶ��
extern void Huashang_SendnotOnline_xunfeiVoices(const char *filename);
//��ȡ����Ѷ����������ʶ����
extern void GetHuashang_xunfei_aifiVoices(const char *xunfeiAifi);
//��ȡ����Ѷ����������ʶ��ʧ��
extern void GetHuashang_xunfei_aifiFailed(void);

#endif