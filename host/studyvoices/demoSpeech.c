#include <sys/time.h>
#include <time.h>
#include <setjmp.h>
#include <string.h>

#include "comshead.h"
//#define MAIN_TEST
#ifdef MAIN_TEST
#include "sock_fd.h"
#include "cJSON.h"
#else
#include "base/demo_tcp.h"
#include "base/cJSON.h"
#include "config.h"
#endif

#define AES_USER_ID
#ifdef AES_USER_ID
#include "aes.h"
#endif

//#define DBG_SPEEK
#ifdef  DBG_SPEEK
#define DEBUG_SPEEK(fmt, args...) printf("%s: " fmt,__func__, ## args)
#else
#define DEBUG_SPEEK(fmt, args...) { }
#endif

#define REQ_FAILED			-1
#define REQ_OK				0
#define REQ_NOT_RESPONSE	1

typedef struct{
	int sock ;
	char token[64];
	char user_id[17];
}TulingUser_t;
static TulingUser_t *tulingUser=NULL;

#ifdef TULING_FILE_LOG
static FILE *logfp=NULL;
static int requestLogNum=0;
static void GetDate(char *date){
    //const char* wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    //const char* mon[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    time_t timep;
    struct tm* p;
    time(&timep);
    p = gmtime(&timep);
    snprintf(date,128,"%d-%02d-%02d-%02d:%02d:%02d",(1900+p->tm_year),p->tm_mon+1,p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
    printf("%s\n", date);
}
#endif

static char *aifiJson(const char *key,int len,int rate,const char *format,const char* asr){
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"ak",key);
#ifdef AES_USER_ID	  
	char *aes_key = "fc8b8bf13ab56ae9";    
	uint8_t in[17]={0};    
	uint8_t out[64] = {'0'};	
	uint8_t aes_key_1[17]={0};	  
	uint8_t iv[17]={0}; 	  
	memcpy(in, tulingUser->user_id, strlen(tulingUser->user_id));	 
	printf("in =%s\n",in); 
	memcpy(aes_key_1, aes_key, strlen(aes_key));	
	memcpy(iv, key, 16);		
	AES128_CBC_encrypt_buffer(out, in, 16, aes_key_1, iv);
	
	uint8_t outStr[64] = {'0'};
	int i,aseLen=0;
		
	for(i=0;i<16;i++){		
		aseLen+=snprintf(outStr+aseLen,64,"%.2x",out[i]);	 
	}
	//printf("outStr = %s i=%d\n",outStr,i);
	cJSON_AddStringToObject(root,"uid",outStr);
#else
	cJSON_AddStringToObject(root,"uid",tulingUser->user_id);
#endif	

	cJSON_AddStringToObject(root,"asr", asr);
	cJSON_AddStringToObject(root,"tts", "0");
	cJSON_AddNumberToObject(root,"flag", 3);
	cJSON_AddStringToObject(root,"token", tulingUser->token);
	char* str_js = cJSON_Print(root);
	cJSON_Delete(root);
	//printf("str_js = %s\n",str_js);
	//free(str_js);
	return str_js;
}

#define HTTP_PORT	80
#define HOST_REQ
#ifdef HOST_REQ
static unsigned char reqNum=15;
static char ServerIp[20];
#endif
static char upload_head[] = 
	"POST /speech/chat HTTP/1.1\r\n"
	"Charset: UTF-8\r\n"
	"Content-Type: multipart/form-data;boundary=***\r\n"
	"Cache-Control: no-cache\r\n"
	"Pragma: no-cache\r\n"
	"User-Agent: Java/1.7.0_65\r\n"
	"Host: %s\r\n"
	"Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\n"
	"Connection: keep-alive\r\n"
	"Content-Length: %d\r\n\r\n";

static char upload_request[] = 
	"\r\n"
	"Content-Disposition: form-data; name=\"speech\";filename=\"speech.wav\"\r\n"
	"Content-Type: application/octet-stream\r\n\r\n";

static sigjmp_buf jmpbuf;

static void sigalrm_fn(int sig){
	printf("demospeeck alarm!\n");
	if(tulingUser->sock>0)
		close(tulingUser->sock);
	tulingUser->sock=-1;
	siglongjmp(jmpbuf,1);
	printf("......sigalrm_fn........\n");
}

struct resp_header{    
	int status_code;//HTTP/1.1 '200' OK    
	char content_type[128];//Content-Type: application/gzip    
	long content_length;//Content-Length: 11683079    
};

static void get_resp_header(const char *response,struct resp_header *resp){    
	/*获取响应头的信息*/    
	char *pos = strstr(response, "HTTP/");    
	if (pos)        
		sscanf(pos, "%*s %d", &resp->status_code);//返回状态码    
		pos = strstr(response, "Content-Type:");//返回内容类型    
		if (pos)        
			sscanf(pos, "%*s %s", resp->content_type);    
		pos = strstr(response, "Content-Length:");//内容的长度(字节)    
		if (pos)        
			sscanf(pos, "%*s %ld", &resp->content_length);    
}
static int httpUploadData(int timeout,const char *key,const char *hostaddr,const void * audio,int len,int rate,const char *format,const char* asr,char **text,int *textSize){	
	char* end = "\r\n"; 			//结尾换行
	char* twoHyphens = "--";		//两个连字符
	char* boundary = "***"; 		// 每个post参数之间的分隔。随意设定，只要不会和其他的字符串重复即可。
	char send_date[4096] = {0};
	char endFile[128]={0};
	char endBoundary[128]={0};			
	int ContentLength = -1;
	struct hostent	 *host=NULL;
	int reuslt=REQ_FAILED;

	*text = NULL;
	*textSize = 0;

#if 1
	signal(SIGALRM,sigalrm_fn);
	if(sigsetjmp(jmpbuf,1)!=0){
		signal(SIGALRM,SIG_IGN);
		printf("..............\n");
		return reuslt;
	}
	alarm(timeout);
#endif	
	if(++reqNum>=10){
		if(reqNum<15){
			reqNum=0;
		}
		if((host=gethostbyname(hostaddr))==NULL){
			fprintf(stderr, "Gethostname   error,	%s\n ",   strerror(errno));
			goto exit0;
		}
		reqNum=0;
		snprintf(ServerIp,20,"%s",(char *)inet_ntoa(*((struct in_addr *)host->h_addr)));
	}
	DEBUG_SPEEK("hostaddr=%s ip=%s \n",hostaddr,ServerIp);
	tulingUser->sock = create_client(ServerIp,(int)HTTP_PORT);


	if(tulingUser->sock < 0 ){
		printf("connect server error!\n");
		goto exit0;
	}
	reuslt =REQ_NOT_RESPONSE;
	
	ContentLength = len;
	char str1[1024]={0};
	strcat(str1,twoHyphens);
	strcat(str1,boundary);
	strcat(str1,end);
	char s[] = "Content-Disposition: form-data; name=\"parameters\"";
	strcat(str1,s);
	strcat(str1,end);
	strcat(str1,end);

	char *str_js =aifiJson(key,len,rate,format,asr);
	strcat(str1,str_js);
	free(str_js);

	strcat(str1,end);
	strcat(str1,twoHyphens);
	strcat(str1,boundary);
	
	ContentLength += strlen(str1);
	ContentLength += strlen(upload_request);

	sprintf(endFile,"%s%s%s",boundary,twoHyphens,end);
	sprintf(endBoundary,"%s%s%s%s%s",end,twoHyphens,boundary,twoHyphens,end);
	
	ContentLength += strlen(boundary);
	ContentLength += strlen(endFile);
	ContentLength += strlen(endBoundary);
	
	//发送的http头和上传文件的描述
	int ret = snprintf(send_date,4096,upload_head,hostaddr,ContentLength);
	
	//发送http 请求头
	if(send(tulingUser->sock,send_date,ret,0) != ret){
		printf("send head error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",send_date);
	memset(send_date,0,ret);
	//json 数据构造发送
	ret = strlen(str1);
	if(send(tulingUser->sock,str1,ret,0) != ret){
		printf("send json error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",str1);
	ret=strlen(upload_request);
	if(send(tulingUser->sock,upload_request,ret,0) != ret){
		printf("send upload_request error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",upload_request);
	int w_size=0,pos=0,all_Size=0;
	while(1){
		//printf("ret = %d\n",ret);
		pos =send(tulingUser->sock,audio+w_size,len-w_size,0);
		w_size +=pos;
		all_Size +=len;
		if( w_size== len){
			w_size=0;
			break;
		}
	}
	send(tulingUser->sock,boundary,strlen(boundary),0);
	send(tulingUser->sock,endFile,strlen(endFile),0); 
	send(tulingUser->sock,endBoundary,strlen(endBoundary),0);
	DEBUG_SPEEK("........send request ok all_Size = %d..........\n",all_Size);
	struct resp_header resp;
	char *response=send_date;
	int length=0,mem_size=4096;
	while (1)	{	
		ret = recv(tulingUser->sock, response+length, 1,0);
		if(ret<=0)
			goto exit0;
		//找到响应头的头部信息, 两个"\n\r"为分割点		  
		int flag = 0;	
		int i;			
		for (i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		if (flag == 4)			  
			break;		  
		length += ret;	
		if(length>=mem_size-1){
			break;
		}
	}
	get_resp_header(response,&resp);	
	DEBUG_SPEEK("resp.content_length = %ld status_code = %d\n",resp.content_length,resp.status_code);
	//printf("response = %s\n",response);
	if(resp.status_code!=200||resp.content_length==0){
		goto exit0;
	}
	char *code = (char *)calloc(1,resp.content_length+1);
	if(code==NULL){
		goto exit0;
	}
	ret=0;
	length=0;
	while(1){
		ret = recv(tulingUser->sock, code+length, resp.content_length-length,0);
		if(ret<=0){
			free(code);
			goto exit0;
		}
		length+=ret;
		//printf("result = %s len=%d\n",code,len);		
		if(length==resp.content_length)
			break;
	}
	*text = code;
	*textSize = resp.content_length;
	reuslt =REQ_OK;
exit0:	
	//设置SIGALRM 为忽略信号，不执行信号函数
	alarm(0);
	signal(SIGALRM,SIG_IGN);
	if(tulingUser->sock>0)
		close(tulingUser->sock);
	tulingUser->sock=-1;
	return reuslt;	
}

/*
@ 请求图灵服务器识别接口
@ timeout 设置请求超时时间  key 请求的账号 audio 上传的语音 len语音长度
  rate 语音采样率 format 语音格式 text 返回的文本信息 textSize 文本长度
@ 无
*/
int reqTlVoices(int timeout,const char *key,const void * audio,int len,int rate,const char *format,const char *asr,char **text,int *textSize){
	int ret =0;
	struct timeval starttime,endtime;
    gettimeofday(&starttime,0); 
	//ret=httpUploadData(timeout,key,(const char *)"beta.app.tuling123.com",audio,len,rate,format,asr,text,textSize);
	ret=httpUploadData(timeout,key,(const char *)"smartdevice.ai.tuling123.com",audio,len,rate,format,asr,text,textSize);
	//printf("ret = %d\n",ret);
	gettimeofday(&endtime,0);
    double timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
    //除以1000则进行毫秒计时，如果除以1000000则进行秒级别计时，如果除以1则进行微妙级别计时
   	printf("reqTlVoices usr time:  %d.%d\n",(int)timeuse/1000000,(int)timeuse/1000%1000);  

#ifdef TULING_FILE_LOG
	if(logfp==NULL){
		char filelog[128]={0},timeStr[128]={0};
		GetDate(timeStr);
		sprintf(filelog,"log/%s",timeStr);
		logfp = fopen(filelog,"w+"); 
		if(logfp==NULL){
			printf("open failed \n");
			return -1;
		}
		fprintf(logfp,"mtk76xx upload file rate %d 16bit  type %s\n",16000,"pcm");
		printf("open log file %s ok\n",filelog);

	}
	fprintf(logfp,"--------------------%d--------------------\n",++requestLogNum);

	fprintf(logfp,"reqTlVoices usr time:  %d.%d\n",(int)timeuse/1000000,(int)timeuse/1000%1000);  
	if(text){
		fprintf(logfp,"%s\n",*text);
	}else{
		fprintf(logfp,"%s\n","requst failed");
	}
	fflush(logfp);
#endif			
	return 	ret;
}

/*
@ 更新token值，用于下一次上传语音的时候，服务器验证
@ token: 本次请求的时候服务器返回的
@ 无
*/
void updateTokenValue(const char *token){
//	Write_tulinglog((const char * )token);
	int size = sizeof(tulingUser->token);
	memset(tulingUser->token,0,size);
	snprintf(tulingUser->token,size,"%s",token);
}
/*
@ 获取token值，保存到路由表当中
@ 
*/
void GetuserTokenValue(char *userId,char *token){
	sprintf(userId,"%s",tulingUser->user_id);
	sprintf(token,"%s",tulingUser->token);
}
/*
@ 从runsystem 进程当中加载userId 和token值(考虑到第一加载和从路由表当中读取)
@ 
*/
int Load_useridAndToken(const char *userId,const char *token){
//	Write_tulinglog((const char * )"update tuling vaule");
//	Write_tulinglog((const char * )userId);
//	Write_tulinglog((const char * )token);
	if(!strcmp(userId,"12345678")){
		return -1;
	}
	memset(tulingUser->user_id,0,strlen(tulingUser->user_id));
	memset(tulingUser->token,0,strlen(tulingUser->token));
	memcpy(tulingUser->user_id,userId,strlen(userId));
	memcpy(tulingUser->token,token,strlen(token));
	return 0;
}

int InitTuling(const char *userId,const char *token){
//	Write_tulinglog((const char * )"init tuling vaule");
//	Write_tulinglog((const char * )userId);
//	Write_tulinglog((const char * )token);
	tulingUser = (TulingUser_t *)calloc(1,sizeof(TulingUser_t));
	if(tulingUser==NULL){
		return -1;
	}
	memcpy(tulingUser->user_id,userId,strlen(userId));
	memcpy(tulingUser->token,token,strlen(token));

	return 0;
}
void DestoryTuling(void){
	if(tulingUser){
		free(tulingUser);
		tulingUser=NULL;
	}
}


#ifdef MAIN_TEST
#define PCM
//#define AMR
typedef int SR_DWORD;
typedef short int SR_WORD ;
struct wave_pcm_hdr{
         char            riff[4];                        // = "RIFF"
         SR_DWORD        size_8;                         // = FileSize - 8
         char            wave[4];                        // = "WAVE"
         char            fmt[4];                         // = "fmt "
         SR_DWORD        dwFmtSize;     
         SR_WORD         format_tag;    
         SR_WORD         channels;       
         SR_DWORD        samples_per_sec; 
         SR_DWORD        avg_bytes_per_sec;
         SR_WORD         block_align;   
         SR_WORD         bits_per_sample;
 
         char            data[4];                        // = "data";
         SR_DWORD        data_size;
};
extern struct wave_pcm_hdr pcmwavhdr;

struct wave_pcm_hdr pcmwavhdr = {
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	8000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

/*******************************************
@函数功能:	json解析服务器数据
@参数:	pMsg	服务器数据
@		textString	解析后的数据
@返回值:	0	成功	其他整数都是错误码
***********************************************/
static int parseJson_string(const char * pMsg){
	int err=-1;
	if(NULL == pMsg){
		return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson){
       	return -1;
    }
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//获取token的值，用于下一次请求时上传的校验值
	if(pSub!=NULL){
		//暂时定义，用于临时存放校验值，每请求一次服务器都返回token
		printf(" TokenValue = %s\n",pSub->valuestring);
		updateTokenValue(pSub->valuestring);
	}
    pSub = cJSON_GetObjectItem(pJson, "code");
    if(NULL == pSub){
		goto exit;
	}
	switch(pSub->valueint){
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
			goto exit;
	}
	pSub = cJSON_GetObjectItem(pJson, "asr");		//返回结果
    if(NULL == pSub){

		goto exit;
    }
	printf("info: %s \n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "tts");		//返回结果---bug
	if(NULL == pSub){
		goto exit;
	}
	printf("text: %s \n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "func");		//返回结果
    if(NULL == pSub){
		goto exit;
    }
	cJSON *urlSub = cJSON_GetObjectItem(pSub, "url");	
	char *ttsURL= (char *)calloc(1,strlen(urlSub->valuestring)+1);
	if(ttsURL==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	sprintf(ttsURL,"%s",urlSub->valuestring);
	printf("url: %s \n",urlSub->valuestring);
	

exit:
	cJSON_Delete(pJson);
	return err;
}

int main(int argc,char **argv){
	if(argc<3){
		printf("please input pcm file and rate<8000/16000> \n");
		return -1;
	}

	char *filename= argv[1];
	if(access(filename,F_OK) != 0){
		printf("file %s not exsit!\n",filename);
		return -1;
	}

	FILE *fp = fopen(filename,"rb");
	if(fp == NULL){
		printf("open file %s error!\n",filename);
		return -1;
	}
	
	fseek(fp,0,SEEK_END);
	int file_len = ftell(fp);
	fseek(fp,0,SEEK_SET);

#ifdef PCM
	char *fileData = (char *)calloc(1,file_len+1);
	if(fileData==NULL)
		return -1;
	int len = fread(fileData,1,file_len,fp);
#endif	


	fclose(fp);
	int rate=atoi(argv[2]);

	char *text=NULL;
	int textSize=0;
	const char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
	//const char *key = "b1833040534a6bfd761215154069ea58";
	char token[64]={"cebb4b73259c4f8b83658bbd4f74b852"};
	char *user_id  = "ai22334455667780";
	InitTuling(user_id,token);
	
	while(1){	
		reqTlVoices(10,key,fileData,len,16000,"pcm","0",&text,&textSize);

		if(text){
			printf("textSize =%d text = %s\n",textSize,text);
			parseJson_string((const char * )text);
			free(text);
		}else{
			printf("req failed\n");
		}
		sleep(1);
	}
	return 0;
}
#endif
