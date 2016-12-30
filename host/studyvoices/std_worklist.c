#include "comshead.h"
#include "host/studyvoices/std_worklist.h"
#include "host/studyvoices/prompt_tone.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/queWorkCond.h"
#include "host/voices/callvoices.h"
#include "host/voices/message_wav.h"
#include "host/voices/wm8960i2s.h"
#include "tlvoice.h"
#include "qtts_qisc.h"
#include "config.h"
#include "StreamFile.h"
#include "../sdcard/musicList.h"

static char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
//static char *key = "21868a0cd8806ee2ba5eab6181f0add7";//tang : change 2016.4.26 for from chang key 

struct eventMsg{
	int  len:24,type:8;
};
static WorkQueue *evMsg;

#ifdef TEST_SAVE_MP3
static int test_mp3file=0;
static  void test_save_mp3file(char *mp3_data,int size)
{
	FILE * fp;	//tang :change 2016-2-22 add mp3 
	char buf[32]={0};
	sprintf(buf,"%s%d%s","/mnt/test/text",test_mp3file++,".amr");
	fp = fopen(buf,"w+");
	if(fp ==NULL){
		perror("test_save_mp3file: fopen failed ");
		return ;
	}
	fwrite(mp3_data,size,1,fp);
	fclose(fp);
}
#endif

/******************************************
@函数功能:	文本处理函数
@参数:	text 文本文字
******************************************/
static void handle_text(char *text)
{
	tolkLog("tolk_start\n");
	int ret=0;
	//检查关键词，做出相应的回答
	if(check_text_cmd(text)){
		//pause_record_audio();------请在check_text_cmd函数中添加
		return ;
	}
	tolkLog("tolk handle qtts start\n");
	ret = PlayQttsText(text,QTTS_UTF8);
	if(ret == 10202){
		//重连，语音播报
		playsysvoices(REQUEST_FAILED);
		//startServiceWifi();
	}
	tolkLog("tolk handle qtts end\n");
}
#define J_VOICES_1	1
#define J_TAIBEN_1	2
#define J_TAIBEN_2	3
#define J_TAIBEN_3	4
#define J_TAIBEN_4	5
#define J_TAIBEN_5	6
#define J_TAIBEN_6	7
#define J_TAIBEN_7	8
#define J_TAIBEN_8	9
#define J_TAIBEN_9	10
static void TaiBenToTulingJsonEr(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (10.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case J_VOICES_1:
			playsysvoices(ERROR_40002);
			break;
		case J_TAIBEN_1:
			PlayQttsText("小朋友你可以说，播放一首歌。",QTTS_GBK);
			break;
		case J_TAIBEN_2:
			PlayQttsText("小朋友你可以问我，李白是谁。",QTTS_GBK);
			break;
		case J_TAIBEN_3:
			PlayQttsText("小朋友你可以问我，明天天气怎么样。",QTTS_GBK);
			break;
		case J_TAIBEN_4:
			PlayQttsText("小朋友你可以跟我聊聊天，说一说悄悄话。",QTTS_GBK);
			break;
		case J_TAIBEN_5:
			PlayQttsText("小朋友你可以说，讲个故事。",QTTS_GBK);
			break;
		case J_TAIBEN_6:
			PlayQttsText("小朋友你可以说，讲个笑话。",QTTS_GBK);
			break;
		case J_TAIBEN_7:
			PlayQttsText("小朋友可以跟我说，音量减。",QTTS_GBK);
			break;
		case J_TAIBEN_8:
			PlayQttsText("小朋友你可以问我，2+3+4+5等于几。",QTTS_GBK);
			break;
		case J_TAIBEN_9:
			PlayQttsText("小朋友你可以对我说，翻译天空。",QTTS_GBK);
			break;
	}
}

/*******************************************
@函数功能:	json解析服务器数据
@参数:	pMsg	服务器数据
@		handle_jsion	解析后处理函数
@		textString	解析后的数据
@返回值:	0	成功	其他整数都是错误码
***********************************************/
static int parseJson_string(const char * pMsg,void handle_jsion(char *textString))
{
	int err=-1;
	if(NULL == pMsg){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson){
       	return -1;
    }
    cJSON * pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		DEBUG_STD_MSG("get code failed\n");
		goto exit;
	}
	DEBUG_STD_MSG("code : %d\n", pSub->valueint);
	switch(pSub->valueint)
	{
		case 40001:
		case 40003:
		case 40004:
		case 40005:		
		case 40006:	
		case 40007:
		case 305000:
		case 302000:
		case 200000:
		case 40002:
#if 1
			TaiBenToTulingJsonEr();
#else
			playsysvoices(ERROR_40002);
#endif
			goto exit;
	}
#if 0	//文本
    pSub = cJSON_GetObjectItem(pJson, "text");		//返回结果
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=get text=%s=\n",pSub->valuestring);
	handle_jsion(pSub->valuestring);
#else	//语音
	pSub = cJSON_GetObjectItem(pJson, "fileUrl");		//返回结果
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=get url=%s=\n",pSub->valuestring);
	//http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3
	//AddDownEvent("http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3",TULING_URL_MAIN);
	char *URL= (char *)calloc(1,strlen(pSub->valuestring)+1);
	if(URL==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	sprintf(URL,"%s",pSub->valuestring);
	AddDownEvent(URL,TULING_URL_MAIN);
#endif
	pSub = cJSON_GetObjectItem(pJson, "info");		//语音识别
    if(NULL == pSub){
		DEBUG_STD_MSG("get text failed\n");
		goto exit;
    }
	printf("=upload=%s=\n",pSub->valuestring);
	err=0;
exit:
	cJSON_Delete(pJson);
	return err;
}

/*******************************************************
@函数功能:	上传数据到服务器并获取到回复
@参数:	voicesdata 上传数据
@		len	上传数据大小
@		voices_type	数据类型
********************************************************/
#if 0
jmp_buf env; 

void recvSignal(int sig) {
	tulingLog("recvSignal signal 11",0);
    printf("received signal %d !!!\n",sig);
	siglongjmp(env,1); 
} 

void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	time_t t;
	int endtime=0,starttime=0;
	int resSize = 0, textSize=0, err=0;
	char *result=NULL, *text=NULL, *audio=NULL;
	starttime=time(&t);
	start_event_play_wav();//暂停录音
	audio = base64_encode(voicesdata, len);
	if(!audio){
		create_event_system_voices(5);
		goto exit1;
	}
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
	
	tulingLog("tuling_start",err);
	int r = sigsetjmp(env,1);  //保存一下上下文
	tulingLog("tuling env start...",err);
	if(r== 0){
		signal(SIGSEGV, recvSignal);  //出现段错误回调函数
		DEBUG_STD_MSG("set signal ok \n");
		///图灵服务器上，内部可能会发生错误的代码 
		if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize)))
		{
			tulingLog("tl_req_voice error",err);
			if(err==5||err==6){
				endtime=time(&t);
				if((endtime-starttime)<15)//重连，语音播报
				{
					if((err=tl_req_voice(audio, len, RECODE_RATE, voices_type,\
								key, &result,&resSize,&text,&textSize))){
						DEBUG_STD_MSG("up voices data failed err =%d\n",err);
						if(err==5||err==6){
							create_event_system_voices(5);
							startServiceWifi();
							free(audio);
							goto exit1;
						}
					}
				}else{
					create_event_system_voices(5);
					startServiceWifi();
					free(audio);
					goto exit1;
				}
			}
		}
	}
	else{
		DEBUG_STD_MSG("jump this code bug!!\n");
		tulingLog("save up down",err);
		create_event_system_voices(5);
		free(audio);
		goto exit1;
	}
	tulingLog("tuling_end",err);
	free(audio);
	audio=NULL;
	if(result){
		free(result);
		result=NULL;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#else
#define SPEEK_1 	1
#define SPEEK_2 	2
#define SPEEK_3 	3
#define SPEEK_4 	4
#define JOKE_1		5
#define JOKE_2		6
#define JOKE_3		7
#define MUSIC_1		8
#define MUSIC_2		9
#define SONG_1		10
#define EDUCARE_1	11
#define SPEEK_5		12
#define EDUCARE_2	13
#define EDUCARE_3	14
#define EDUCARE_4	15
#define EDUCARE_5	16
#define EDUCARE_6	17
#define MUSIC_3		18
#define MUSIC_4		19
#define SONG_2		20
#define SONG_3		21
#define SONG_4		22
#define SONG_5		23
#define SONG_6		24
#define SONG_7		25
#define SONG_8		26
#define SONG_9		27
#define SONG_10		28
#define MUSIC_5		29
#define MUSIC_6		30
#define EDUCARE_7	31
#define SPEEK_6		32

static void TaiwanToTulingError(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (30.0*rand()/(RAND_MAX+1.0)));
	switch(i){
//---------------------------------对答-------------------------------------------------------
	case SPEEK_1:
		QttsPlayEvent("我先想想，再回答你好啦。",QTTS_GBK);
		break;
	case SPEEK_2:
		QttsPlayEvent("我也不知道哟，你不会怪我吧。",QTTS_GBK);
		break;
	case SPEEK_3:
		QttsPlayEvent("啦啦啦，今天天气真不错，心情也是棒棒哒。",QTTS_GBK);
		break;
	case SPEEK_4:
		QttsPlayEvent("不要老是问我这样的问题。",QTTS_GBK);
		break;
	case SPEEK_5:
		QttsPlayEvent("不要理我，我在思考呢。",QTTS_GBK);
		break;
	case SPEEK_6:
		QttsPlayEvent("这是悄悄话，我要贴在你耳朵边才能告诉你的啦。",QTTS_GBK);
		break;
//---------------------------------笑话-------------------------------------------------------
	case JOKE_1:
		QttsPlayEvent("孙子问，爷爷，水牛是啥样子?爷爷说：水牛跟普通牛长的差不多，不同的是它喜欢在水中生活。孙子说：噢，我懂啦，它一定是喜欢吃鱼吧。",QTTS_GBK);
		break;
	case JOKE_2:
		QttsPlayEvent("父亲吩咐儿子说：你到西服店去取为爸爸订做的衣服。如果老板问你要钱，你就告诉他，因为你太小，爸爸不让你带钱出门。儿子离开后不久，又空着手回来了，并告诉父亲说：爸爸，西服店老板说，等我长大了再去拿。",QTTS_GBK);
		break;
	case JOKE_3:
		QttsPlayEvent("儿子说：妈妈，我得了一百分，您奖给我什么呀？妈妈说：十块钱。儿子说：那就先奖我一半吧，我得了五十分。",QTTS_GBK);
		break;
//---------------------------------儿歌-------------------------------------------------------
	case MUSIC_1:
		QttsPlayEvent("你拍一,我拍一,一个小孩坐飞机。",QTTS_GBK);
		break;
	case MUSIC_2:
		QttsPlayEvent("找呀找呀找朋友，找到一个好朋友，敬个礼呀握握手，你是我的好朋友。",QTTS_GBK);
		break;
	case MUSIC_3:
		QttsPlayEvent("娃哈哈，娃哈哈，我们的生活多愉快。",QTTS_GBK);
		break;
	case MUSIC_4:
		QttsPlayEvent("小小少年，很少烦恼，无忧无虑乐陶陶。",QTTS_GBK);
		break;
	case MUSIC_5:
		QttsPlayEvent("丢，丢，丢手绢，轻轻地放在小朋友的后面，大家不要告诉他，快点快点捉住他，快点快点捉住他。快点快点捉住他。",QTTS_GBK);
		break;
	case MUSIC_6:
		QttsPlayEvent("夜夜想起妈妈的话，闪闪的泪光鲁冰花。",QTTS_GBK);
		break;
//---------------------------------诗歌-------------------------------------------------------
	case SONG_1:
		QttsPlayEvent("我给你读首诗吧，静夜思。作者：李白，床前明月光，疑是地上霜。举头望明月，低头思故乡。",QTTS_GBK);
		break;
	case SONG_2:
		QttsPlayEvent("我给你读首诗吧，咏鹅，唐·骆宾王。鹅，鹅，鹅，曲项向天歌。白毛浮绿水，红掌拨清波。",QTTS_GBK);
		break;
	case SONG_3:
		QttsPlayEvent("我给你读首诗吧，登鹳雀楼，唐·王之涣。白日依山尽，黄河入海流。欲穷千里目，更上一层楼。",QTTS_GBK);
		break;
	case SONG_4:
		QttsPlayEvent("我给你读首诗吧，《春晓》唐·孟浩然。春眠不觉晓，处处闻啼鸟。夜来风雨声，花落知多少。",QTTS_GBK);
		break;
	case SONG_5:
		QttsPlayEvent("我给你读首诗吧，《九月九日忆山东兄弟》唐·王维。独在异乡为异客，每逢佳节倍思亲。遥知兄弟登高处，遍插茱萸少一人。",QTTS_GBK);
		break;
	case SONG_6:
		QttsPlayEvent("我给你读首诗吧，《望庐山瀑布》唐·李白。日照香炉生紫烟，遥看瀑布挂前川。飞流直下三千尺，疑是银河落九天。",QTTS_GBK);
		break;
	case SONG_7:
		QttsPlayEvent("我给你读首诗吧，《赠汪伦》唐·李白。李白乘舟将欲行，忽闻岸上踏歌声。桃花潭水深千尺，不及汪伦送我情。",QTTS_GBK);
		break;
	case SONG_8:
		QttsPlayEvent("我给你读首诗吧，《黄鹤楼送孟浩然之广陵》唐·李白。故人西辞黄鹤楼，烟花三月下扬州。孤帆远影碧空尽，唯见长江天际流。",QTTS_GBK);
		break;
	case SONG_9:
		QttsPlayEvent("我给你读首诗吧，《早发白帝城》唐·李白。朝辞白帝彩云间，千里江陵一日还。两岸猿声啼不住，轻舟已过万重山。",QTTS_GBK);
		break;
	case SONG_10:
		QttsPlayEvent("我给你读首诗吧，《回乡偶书》唐·贺知章。少小离家老大回，乡音无改鬓毛衰。儿童相见不相识，笑问客从何处来。",QTTS_GBK);
		break;
//---------------------------------安全教育的-------------------------------------------------------
	case EDUCARE_1:
		QttsPlayEvent("安全小知识，夏天中暑别着急，清凉油擦效果好。",QTTS_GBK);
		break;
	case EDUCARE_2:
		QttsPlayEvent("安全小知识，走路要走人行道，不准追逐和打闹。一慢二看三通过，横穿马路忘不了。",QTTS_GBK);
		break;
	case EDUCARE_3:
		QttsPlayEvent("安全小知识，饮食一定讲卫生，不喝生水零食少。",QTTS_GBK);
		break;
	case EDUCARE_4:
		QttsPlayEvent("安全小知识，上下楼梯靠右行，不准拥挤和抢道。",QTTS_GBK);
		break;
	case EDUCARE_5:
		QttsPlayEvent("安全小知识，乘车要等车停稳，先下后上不急躁。手头不能出车窗，扶紧把手很重要。",QTTS_GBK);
		break;
	case EDUCARE_6:
		QttsPlayEvent("安全小知识，小学生，要牢记，不登高，不下低。不追逐，不投掷，安全意识放第一。",QTTS_GBK);
		break;
	case EDUCARE_7:
		QttsPlayEvent("安全小知识，红灯停，绿灯行，黄绿灯亮快快行，行停停行看灯明。",QTTS_GBK);
		break;
	}
}
#define Q_VOICES_1	1
#define Q_TAIBEN_1	2
#define Q_TAIBEN_2	3
#define Q_TAIBEN_3	4
#define Q_TAIBEN_4	5
#define Q_TAIBEN_5	6
#define Q_TAIBEN_6	7
#define Q_TAIBEN_7	8
#define Q_TAIBEN_8	9
#define Q_TAIBEN_9	10
static void TaiBenToTulingQuestEr(void){
	//srand( (unsigned)time( NULL ) );
	int i=(1+(int) (5.0*rand()/(RAND_MAX+1.0)));
	switch(i){
		case Q_VOICES_1:
			create_event_system_voices(5);
			break;
#ifdef QITUTU_SHI
		case Q_TAIBEN_1:
			createPlayEvent((const void *)"xiai",PLAY_NEXT);
			break;
#else
		case Q_TAIBEN_2:
			createPlayEvent((const void *)"mp3",PLAY_NEXT);
			break;
		case Q_TAIBEN_3:
			createPlayEvent((const void *)"story",PLAY_NEXT);
			break;
		case Q_TAIBEN_4:
			createPlayEvent((const void *)"guoxue",PLAY_NEXT);
			break;
		case Q_TAIBEN_5:
			createPlayEvent((const void *)"english",PLAY_NEXT);
			break;
#endif
#if 0
		case Q_TAIBEN_6:
			QttsPlayEvent("小朋友你可以说，讲个笑话。",QTTS_GBK);
			break;
		case Q_TAIBEN_7:
			QttsPlayEvent("小朋友跟我说，音量减。",QTTS_GBK);
			break;
		case Q_TAIBEN_8:
			QttsPlayEvent("小朋友你问我，2+3+4+5等于几。",QTTS_GBK);
			break;
		case Q_TAIBEN_9:
			QttsPlayEvent("小朋友你问我，翻译天空。",QTTS_GBK);
			break;
#endif
	}
}
void send_voices_server(const char *voicesdata,int len,char *voices_type)
{
	int textSize=0, err=0;
	char *text=NULL;
	start_event_play_wav();//暂停录音
	DEBUG_STD_MSG("up voices data ...(len=%d)\n",len);
#if 0
	char *URL= (char *)calloc(1,strlen("http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3")+1);
	if(URL==NULL){
		perror("calloc error !!!");
		return -1;
	}
	sprintf(URL,"%s","http://opentest.tuling123.com/file/d3a38a1e-7318-4837-be91-43642ae93842.mp3");
	AddDownEvent(URL,TULING_URL_MAIN);
#else
	err=reqTlVoices(10,key,(const void *)voicesdata,len,RECODE_RATE,voices_type,&text,&textSize);
	if(err==-1){
#if 1
		create_event_system_voices(5);	
#else
		TaiBenToTulingQuestEr();
#endif
		goto exit1;
	}else if(err==1){
		TaiwanToTulingError();
		goto exit1;
	}
	if(text){
		add_event_msg(text,0,STUDY_WAV_EVENT);
	}
#endif
	return ;
exit1:
#ifdef QITUTU_SHI
	Led_System_vigue_close();
#endif
	pause_record_audio();
	return;
}
#endif
/******************************************************
@函数功能:	学习音事件处理函数
@参数:	data 数据 len 数据大小
*******************************************************/
static void runJsonEvent(const char *data)
{
	parseJson_string(data,handle_text);
	free(data);
}
int event_lock=0;
/*******************************************************
@函数功能:	添加事件到链表
@参数:	databuf 数据	len 数据长度 type事件类型
@返回值:	-1 添加失败 其它添加成功
********************************************************/
int add_event_msg(const char *databuf,int  len,int  type)
{
	int msgSize=0;
	if(event_lock){
		DEBUG_STD_MSG("add_event_msg event_lock =%d\n",event_lock); // 写入 type event_lock a+
		eventlockLog("event_lock add error\n",event_lock);
		return ;
	}
	if(type!=LOCAL_MP3_EVENT)	//不为本地播放清理播放上下曲
		sysMes.localplayname=0;
	eventlockLog("event_lock add ok\n",event_lock);
	struct eventMsg *msg =(struct eventMsg *)(&msgSize);
	msg->len = len;
	msg->type = type;
	printf("add end ..\n");
	return putMsgQueue(evMsg,databuf,msgSize);
}
int getEventNum(void)
{
	return getWorkMsgNum(evMsg);
}
void cleanEvent(void){
char *msg;
int msgSize;
event_lock=1;	//受保护状态事件
while(getWorkMsgNum(evMsg)){
	getMsgQueue(evMsg,&msg,&msgSize);
	if(msg!=NULL){
		free(msg);
		usleep(100);
	}
}
event_lock=0;
}
/*******************************************************
@函数功能:	事件处理函数
@参数:	data 数据	msgSize事件类型以及数据大小结构体
********************************************************/
static void handle_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize); 
	DEBUG_STD_MSG("=====================================================================================\n");
	DEBUG_STD_MSG("handle_event_msg cur->type = %d\n",cur->type);
	DEBUG_STD_MSG("=====================================================================================\n");
	handleeventLog("handleevent_start\n",cur->type);
	switch(cur->type){
		case STUDY_WAV_EVENT:		//会话事件
#ifdef QITUTU_SHI
			Led_System_vigue_close();
#endif
			runJsonEvent(data);
			//pause_record_audio();
			break;
			
		case SYS_VOICES_EVENT:		//系统音事件
			start_event_play_wav();
			handle_event_system_voices(cur->len);
			if(cur->len==2)
				break;
			pause_record_audio();
			break;
			
		case SET_RATE_EVENT:		//URL清理事件
			event_lock=1;	//受保护状态事件
			eventlockLog("eventlock_start\n",event_lock);
			cleanplayEvent(1);
			//cleanEvent();
			NetStreamExitFile();
			i2s_start_play(RECODE_RATE);
			event_lock=0;
			sysMes.localplayname=0;
			pause_record_audio();
			eventlockLog("eventlock end\n",event_lock);
			break;
			
		case URL_VOICES_EVENT:		//URL网络播放事件
			playurlLog("url play\n");
			cleanplayEvent(0);		
			NetStreamExitFile();
			start_event_play_url();
			playurlLog("NetStreamExitFile\n");
			AddDownEvent(data,URL_VOICES_EVENT);
			sleep(3);
			break;
			
#ifdef 	LOCAL_MP3
		case LOCAL_MP3_EVENT:		//本地音乐播放事件
			cleanplayEvent(0);		//去除清理锁
			NetStreamExitFile();
			start_event_play_url();
			AddDownEvent(data,LOCAL_MP3_EVENT);
			DEBUG_STD_MSG("handle_event_msg LOCAL_MP3_EVENT add end\n");
			break;
#endif
			
		case QTTS_PLAY_EVENT:		//QTTS事件
			PlayTuLingTaibenQtts(data,cur->len);
			free((void *)data);
			break;
			
#ifdef SPEEK_VOICES	
		case SPEEK_VOICES_EVENT:	//接收到语音消息	
			playspeekVoices(data);
			pause_record_audio();
			remove(data);
			usleep(1000);
			free((void *)data);
			break;
			
		case TALK_EVENT_EVENT:		//对讲事件
			handle_voices_key_event(cur->len);
			break;
#endif
		default:
			DEBUG_STD_MSG("not event msg !!!\n");
			break;
	}
	handleeventLog("handleevent end\n",cur->type);
}

static void clean_event_msg(const char *data,int msgSize)
{
	struct eventMsg *cur =(struct eventMsg *)(&msgSize);
	if(cur->type==STUDY_WAV_EVENT){
		free(data);
	}else if(cur->type==SYS_VOICES_EVENT){

	}
}


void init_stdvoices_pthread(void)
{
	evMsg = InitCondWorkPthread(handle_event_msg);
	init_iat_MSPLogin(WriteqttsPcmData);
}
void clean_stdvoices_pthread(void)
{
	CleanCondWorkPthread(evMsg,clean_event_msg);
	iat_MSPLogout();
}
