#ifndef _HUASHANGMUSIC_H
#define _HUASHANGMUSIC_H

#define	HUASHANG_MUSIC_TOTAL_NUM 	3838

#define UNKOWN_AIFI_STATE	0	//��ʼ״̬
#define	TULING_AIFI_OK		1	//ͼ������ɹ�״̬
#define	XUNFEI_AIFI_OK		2	//Ѷ������ɹ�״̬
#define XUNFEI_AIFI_FAILED	3	//Ѷ��ʶ��ʧ��״̬
#define XUNFEI_AIFI_ING		4	//Ѷ������ʶ��״̬

#define ALLOW_TULING_PLAY	0
#define DISABLE_TULING_PLAY	-1
#define TIMEOUT_AIFI		-2
//�������ػ��Ͻ�������
extern void openSystemload_huashangData(void);
//��ȡsdard ���ݽ��в���
extern int GetScard_forPlayHuashang_Music(unsigned char playMode,unsigned char EventSource);
//�ػ����滪������
extern void closeSystemSave_huashangData(void);
//�������������б�
extern void CreatePlayListMuisc(const void *data,int musicType);

extern void Huashang_changePlayVoicesName(void);


#endif