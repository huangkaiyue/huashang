#include <sys/time.h>
#include <time.h>
#include <setjmp.h>
#include <string.h>

#include "comshead.h"
#include "base/demo_tcp.h"
#include "base/cJSON.h"
#include "config.h"
#ifndef MY_HTTP_REQ
#include <curl/curl.h>
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
    const char* wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char* mon[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
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
	cJSON_AddStringToObject(root,"key",key);
#ifdef AES_USER_ID	  
	char *aes_key = "1234567890123456";    
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
		
	for(i=0;out[i] != '\0';i++){		
		aseLen+=snprintf(outStr+aseLen,64,"%.2x",out[i]);	 
	}
	//printf("outStr = %s i=%d\n",outStr,i);
	cJSON_AddStringToObject(root,"userid",outStr);
#else
	cJSON_AddStringToObject(root,"userid",tulingUser->user_id);
#endif	

	cJSON_AddNumberToObject(root,"rate", rate);
	cJSON_AddStringToObject(root,"format", format);
	cJSON_AddNumberToObject(root,"len", len);
	cJSON_AddStringToObject(root,"asr", asr);
	cJSON_AddStringToObject(root,"tts", "0");
	cJSON_AddStringToObject(root,"token", tulingUser->token);
	cJSON_AddStringToObject(root,"elapsedtime", "true");
	char* str_js = cJSON_Print(root);
	cJSON_Delete(root);
	//printf("str_js = %s\n",str_js);
	//free(str_js);
	return str_js;
}

#ifdef MY_HTTP_REQ
#define HTTP_PORT	80
#define HOST_REQ
#ifdef HOST_REQ
static unsigned char reqNum=15;
static char ServerIp[20];
#endif
static char upload_head[] = 
	"POST /speechapi/speech/speechapi HTTP/1.1\r\n"
//	"POST /openapi/speech/speechapi HTTP/1.1\r\n"
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
	"Content-Disposition: form-data; name=\"speech\";filename=\"./iflytek01.wav\"\r\n"
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
	/*»ñÈ¡ÏìÓ¦Í·µÄÐÅÏ¢*/    
	char *pos = strstr(response, "HTTP/");    
	if (pos)        
		sscanf(pos, "%*s %d", &resp->status_code);//·µ»Ø×´Ì¬Âë    
		pos = strstr(response, "Content-Type:");//·µ»ØÄÚÈÝÀàÐÍ    
		if (pos)        
			sscanf(pos, "%*s %s", resp->content_type);    
		pos = strstr(response, "Content-Length:");//ÄÚÈÝµÄ³¤¶È(×Ö½Ú)    
		if (pos)        
			sscanf(pos, "%*s %ld", &resp->content_length);    
}
static int httpUploadData(int timeout,const char *key,const char *hostaddr,const void * audio,int len,int rate,const char *format,const char* asr,char **text,int *textSize){	
	char* end = "\r\n"; 			//½áÎ²»»ÐÐ
	char* twoHyphens = "--";		//Á½¸öÁ¬×Ö·û
	char* boundary = "***"; 		// Ã¿¸öpost²ÎÊýÖ®¼äµÄ·Ö¸ô¡£ËæÒâÉè¶¨£¬Ö»Òª²»»áºÍÆäËûµÄ×Ö·û´®ÖØ¸´¼´¿É¡£
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
//	if(++reqNum>=10){
		if(reqNum<15){
			reqNum=0;
		}
		if((host=gethostbyname(hostaddr))==NULL){
			fprintf(stderr, "Gethostname   error,	%s\n ",   strerror(errno));
			goto exit0;
		}
		reqNum=0;
		snprintf(ServerIp,20,"%s",(char *)inet_ntoa(*((struct in_addr *)host->h_addr)));
//	}
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
	
	//·¢ËÍµÄhttpÍ·ºÍÉÏ´«ÎÄ¼þµÄÃèÊö
	int ret = snprintf(send_date,4096,upload_head,hostaddr,ContentLength);
	
	//·¢ËÍhttp ÇëÇóÍ·
	if(send(tulingUser->sock,send_date,ret,0) != ret){
		printf("send head error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",send_date);
	memset(send_date,0,ret);
	//json Êý¾Ý¹¹Ôì·¢ËÍ
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
		//ÕÒµ½ÏìÓ¦Í·µÄÍ·²¿ÐÅÏ¢, Á½¸ö"\n\r"Îª·Ö¸îµã		  
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
	//ÉèÖÃSIGALRM ÎªºöÂÔÐÅºÅ£¬²»Ö´ÐÐÐÅºÅº¯Êý
	alarm(0);
	signal(SIGALRM,SIG_IGN);
	if(tulingUser->sock>0)
		close(tulingUser->sock);
	tulingUser->sock=-1;
	return reuslt;	
}

/*
@ ÇëÇóÍ¼Áé·þÎñÆ÷Ê¶±ð½Ó¿Ú
@ timeout ÉèÖÃÇëÇó³¬Ê±Ê±¼ä  key ÇëÇóµÄÕËºÅ audio ÉÏ´«µÄÓïÒô lenÓïÒô³¤¶È
  rate ÓïÒô²ÉÑùÂÊ format ÓïÒô¸ñÊ½ text ·µ»ØµÄÎÄ±¾ÐÅÏ¢ textSize ÎÄ±¾³¤¶È
@ ÎÞ
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
    //³ýÒÔ1000Ôò½øÐÐºÁÃë¼ÆÊ±£¬Èç¹û³ýÒÔ1000000Ôò½øÐÐÃë¼¶±ð¼ÆÊ±£¬Èç¹û³ýÒÔ1Ôò½øÐÐÎ¢Ãî¼¶±ð¼ÆÊ±
   	printf("reqTlVoices usr time:  %d.%d\n",(int)timeuse/1000000,(int)timeuse/1000%1000);  

#ifdef TULING_FILE_LOG
	if(logfp==NULL){
		char filelog[128]={0},timeStr[128]={0};
		GetDate(timeStr);
		mkdir("/log/",0777);
		sprintf(filelog,"log/%s",timeStr);
		logfp = fopen(filelog,"w+"); 
		if(logfp==NULL){
			printf("open failed \n");
			return -1;
		}
	#ifndef AMR16K_DATA	
		fprintf(logfp,"mtk76xx upload file rate %d 16bit  type %s\n",16000,"pcm");
	#else
		fprintf(logfp,"mtk76xx file rate %d 16bit  type %s\n",16000,"amr");
	#endif	
		printf("open log file %s ok\n",filelog);
	#endif	
	}

#ifdef TULING_FILE_LOG
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
#else
typedef struct{
	char *text;
	int len;
	int ret;
}RequestData_t;
static void process_data(void *buffer, size_t size, size_t nmemb, void *user_p){
    printf("%s\n\n",buffer);
    char *data = (char *)calloc(1,nmemb*size+1); 
    if(data==NULL){
       user_p=NULL;
       return ;
    }
    memcpy(data,buffer,nmemb*size);
    RequestData_t *request = (RequestData_t *)user_p;
    request->text = data;
    request->len=nmemb*size;
	request->ret=0;
}


int reqTlVoices(int timeout,const char *key,const char *audiofile,int audiolen,int rate,const char *format,const char *tts,char **result,int *res_len){ 	
	RequestData_t requestData;
	requestData.ret=1;
#if 1
	CURLcode code;
#else
	CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(code != CURLE_OK){
        printf("init libcurl failed.");
    }
#endif
    struct curl_slist *http_headers = NULL;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    CURL *easy_handle = curl_easy_init();
    
    http_headers = curl_slist_append(http_headers, "Expect:");
    http_headers = curl_slist_append(http_headers, "Charset: UTF-8");
    
	char *upjson = aifiJson(key,audiolen,rate,format,tts);
	printf("upjson = %s\n",upjson);
#if 0
    curl_formadd(&post,&last,
                 CURLFORM_COPYNAME,"parameters",
                 CURLFORM_COPYCONTENTS,"{\"key\":\"059b24d6e4ab9267f4728690711daa54\",\"userid\":\"FC2DDF1DF77BD9451E63C922B45FA779\",\"rate\":16000,\"format\":\"asr\",\"len\":90400,\"asr\":\"0\",\"tts\":\"0\",\"token\":\"0203f029-3d94-44ff-8429-bcc7c6250105\",\"elapsedtime\":\"true\"}",
                 CURLFORM_END);
#else
    curl_formadd(&post,&last,
                 CURLFORM_COPYNAME,"parameters",
                 CURLFORM_COPYCONTENTS,upjson,CURLFORM_END);
	
#endif    

    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME,"speech",
                 CURLFORM_FILE,audiofile,
                 CURLFORM_CONTENTTYPE,"application/octet-stream",
                 CURLFORM_END);
    
    //curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);
    
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, http_headers);
    //curl_easy_setopt(easy_handle, CURLOPT_URL, "http://test79.ai.tuling123.com/speechapi/speech/speechapi");
    curl_easy_setopt(easy_handle, CURLOPT_URL, "http://beta.app.tuling123.com/speechapi/speech/speechapi");
    //curl_easy_setopt(easy_handle, CURLOPT_URL, "http://smartdevice.ai.tuling123.com/speechapi/speech/speechapi");
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &process_data);
    curl_easy_setopt(easy_handle, CURLOPT_HTTPPOST,post);
//    curl_easy_setopt(easy_handle, CURLOPT_PROXY, "localhost:8888");
	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(easy_handle,CURLOPT_TIMEOUT,timeout);
    curl_easy_setopt(easy_handle,CURLOPT_CONNECTTIMEOUT,5);

	

    curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(easy_handle,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(easy_handle,CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(easy_handle, CURLOPT_FORBID_REUSE, 1L);

    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &requestData); //å°†è¿”å›žçš„httpå¤´è¾“å‡ºåˆ°fpæŒ‡å‘çš„æ–‡ä»¶
    code = curl_easy_perform(easy_handle);
    *result= requestData.text;
	*res_len = requestData.len;
    //curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, fp); //å°†è¿”å›žçš„htmlä¸»ä½“æ•°æ®è¾“å‡ºåˆ°fpæŒ‡å‘çš„æ–‡ä»¶
    //printf("len ret.data = %s \n",requestData.len,requestData.text);
    
    curl_slist_free_all(http_headers);
    curl_formfree(post);
    curl_easy_cleanup(easy_handle);
    //curl_global_cleanup();
    free(upjson);
	return requestData.ret;
}

#endif

/*
@ ¸üÐÂtokenÖµ£¬ÓÃÓÚÏÂÒ»´ÎÉÏ´«ÓïÒôµÄÊ±ºò£¬·þÎñÆ÷ÑéÖ¤
@ token: ±¾´ÎÇëÇóµÄÊ±ºò·þÎñÆ÷·µ»ØµÄ
@ ÎÞ
*/
void updateTokenValue(const char *token){
	int size = sizeof(tulingUser->token);
	memset(tulingUser->token,0,size);
	snprintf(tulingUser->token,size,"%s",token);
	//writeLog((const char * )"/home/tuling_log.txt",(const char * )token);
}
/*
@ »ñÈ¡tokenÖµ£¬±£´æµ½Â·ÓÉ±íµ±ÖÐ
@ 
*/
void GetTokenValue(char *token){
	sprintf(token,"%s",tulingUser->token);
}
/*
@ ´Órunsystem ½ø³Ìµ±ÖÐ¼ÓÔØuserId ºÍtokenÖµ(¿¼ÂÇµ½µÚÒ»¼ÓÔØºÍ´ÓÂ·ÓÉ±íµ±ÖÐ¶ÁÈ¡)
@ 
*/
int Load_useridAndToken(const char *userId,const char *token){
	writeLog((const char * )"/home/tuling_log.txt",(const char * )userId);
	writeLog((const char * )"/home/tuling_log.txt",(const char * )token);
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
	writeLog((const char * )"/home/tuling_log.txt",(const char * )"init val");
	writeLog((const char * )"/home/tuling_log.txt",(const char * )userId);
	writeLog((const char * )"/home/tuling_log.txt",(const char * )token);
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

//#define MAIN_TEST
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
@º¯Êý¹¦ÄÜ:	json½âÎö·þÎñÆ÷Êý¾Ý
@²ÎÊý:	pMsg	·þÎñÆ÷Êý¾Ý
@		textString	½âÎöºóµÄÊý¾Ý
@·µ»ØÖµ:	0	³É¹¦	ÆäËûÕûÊý¶¼ÊÇ´íÎóÂë
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
	cJSON *pSub = cJSON_GetObjectItem(pJson, "token");//»ñÈ¡tokenµÄÖµ£¬ÓÃÓÚÏÂÒ»´ÎÇëÇóÊ±ÉÏ´«µÄÐ£ÑéÖµ
	if(pSub!=NULL){
		//ÔÝÊ±¶¨Òå£¬ÓÃÓÚÁÙÊ±´æ·ÅÐ£ÑéÖµ£¬Ã¿ÇëÇóÒ»´Î·þÎñÆ÷¶¼·µ»Øtoken
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
	pSub = cJSON_GetObjectItem(pJson, "info");		//·µ»Ø½á¹û
    if(NULL == pSub){

		goto exit;
    }
	printf("info: %s \n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "text");		//·µ»Ø½á¹û---bug
	if(NULL == pSub){
		goto exit;
	}
	printf("text: %s \n",pSub->valuestring);
	pSub = cJSON_GetObjectItem(pJson, "ttsUrl");		//·µ»Ø½á¹û
    if(NULL == pSub){
		goto exit;
    }
	char *ttsURL= (char *)calloc(1,strlen(pSub->valuestring)+1);
	if(ttsURL==NULL){
		perror("calloc error !!!");
		goto exit;
	}
	sprintf(ttsURL,"%s",pSub->valuestring);
	printf("url: %s \n",pSub->valuestring);
	
	pSub = cJSON_GetObjectItem(pJson, "fileUrl"); 	//·µ»Ø½á¹û
	if(NULL == pSub){
		err=0;
		goto exit;
	}else{
		err=0;
		goto exit;
	}
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

#ifdef AMR
		static char amr_data[12*1024];
		char *buf_voices = (char *)calloc(1,file_len+sizeof(struct wave_pcm_hdr));
		int len = fread(buf_voices+sizeof(struct wave_pcm_hdr),1,file_len,fp);
		int amr_size=0;
		pcmwavhdr.size_8 = (len+36);
		pcmwavhdr.data_size = len;
		pcmwavhdr.samples_per_sec=16000;
		memcpy(buf_voices,&pcmwavhdr,sizeof(struct wave_pcm_hdr));
		printf("==================================\n");
		if(WavAmr16k((const char *)buf_voices,amr_data,&amr_size)){
			printf("enc failed \n");
			return 0;
		}
		printf("==================================\n");
#endif
	fclose(fp);
	int rate=atoi(argv[2]);

	char *text=NULL;
	int textSize=0;
	//const char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
	const char *key = "b1833040534a6bfd761215154069ea58";
	char token[64]={"772f32e9-1a8a-46fe-95ed-76b405e71fca"};
	char *user_id  = "ai22334455667780";
	InitTuling(user_id,token);
	
	while(1){
#ifdef PCM		
		reqTlVoices(10,key,fileData,len,16000,"pcm","0",&text,&textSize);
#endif
#ifdef AMR
		reqTlVoices(10,key,amr_data,len,16000,"amr","3",&text,&textSize);
#endif
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
