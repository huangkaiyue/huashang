#include <sys/time.h>
#include <time.h>
#include <setjmp.h>

#include "comshead.h"

#include "base/demo_tcp.h"
#include "base/cJSON.h"

//#define DBG_SPEEK
#ifdef  DBG_SPEEK
#define DEBUG_SPEEK(fmt, args...) printf("%s: " fmt,__func__, ## args)
#else
#define DEBUG_SPEEK(fmt, args...) { }
#endif

#define REQ_FAILED			-1
#define REQ_OK				0
#define REQ_NOT_RESPONSE	1

#define HTTP_PORT	80

static int sock = -1;
#define HOST_REQ

#ifdef HOST_REQ
static unsigned char reqNum=15;
static char ServerIp[20];
#endif
static char upload_head[] = 
	"POST /openapi/speech/speechapi HTTP/1.1\r\n"
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
	if(sock>0)
		close(sock);
	sock=-1;
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

static int httpUploadData(int timeout,const char *key,const char *hostaddr,const void * audio,int len,int rate,const char *format,char **text,int *textSize){	
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

#ifdef HOST_REQ 
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
	sock = create_client(ServerIp,(int)HTTP_PORT);
#else
	if((host=timeGesthostbyname(hostaddr,5)) == NULL){
		fprintf(stderr, "Gethostname   error,	%s\n ",   strerror(errno));
		alarm(0);
		signal(SIGALRM,SIG_IGN);
		return reuslt;
	}
	DEBUG_SPEEK("hostaddr=%s ip=%s \n",hostaddr,(char *)inet_ntoa(*((struct in_addr *)host->h_addr)));
	sock = create_client((char *)inet_ntoa(*((struct in_addr *)host->h_addr)),(int)HTTP_PORT);
#endif
/*
	struct itimerval  tick;
	signal(SIGALRM, sigalrm_fn);
	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_sec = 10;
    tick.it_value.tv_usec = 0;

    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 10;
    tick.it_interval.tv_usec = 0;
	
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
            printf("Set timer failed!\n");
*/
	if(sock < 0 ){
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

	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"key",key);
	cJSON_AddStringToObject(root,"userid","1");
	cJSON_AddNumberToObject(root,"rate", rate);
	cJSON_AddStringToObject(root,"format", format);
	cJSON_AddNumberToObject(root,"len", len);
	char* str_js = cJSON_Print(root);
	strcat(str1,str_js);
	cJSON_Delete(root);

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
	if(send(sock,send_date,ret,0) != ret){
		printf("send head error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",send_date);
	memset(send_date,0,ret);
	//json 数据构造发送
	ret = strlen(str1);
	if(send(sock,str1,ret,0) != ret){
		printf("send json error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",str1);
	ret=strlen(upload_request);
	if(send(sock,upload_request,ret,0) != ret){
		printf("send upload_request error!\n");
		goto exit0;
	}
	DEBUG_SPEEK("%s\n",upload_request);
	int w_size=0,pos=0,all_Size=0;
	while(1){
		//printf("ret = %d\n",ret);
		pos =send(sock,audio+w_size,len-w_size,0);
		w_size +=pos;
		all_Size +=len;
		if( w_size== len){
			w_size=0;
			break;
		}
	}
	send(sock,boundary,strlen(boundary),0);
	send(sock,endFile,strlen(endFile),0); 
	send(sock,endBoundary,strlen(endBoundary),0);
	DEBUG_SPEEK("........send request ok all_Size = %d..........\n",all_Size);
	struct resp_header resp;
	char *response=send_date;
	int length=0,mem_size=4096;
	while (1)	{	
		ret = recv(sock, response+length, 1,0);
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
		ret = recv(sock, code+length, resp.content_length-length,0);
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
	if(sock>0)
		close(sock);
	sock=-1;
	return reuslt;	
}

int reqTlVoices(int timeout,const char *key,const void * audio,int len,int rate,const char *format,char **text,int *textSize){
	int ret =0;
	struct timeval starttime,endtime;
    gettimeofday(&starttime,0); 
	ret=httpUploadData(timeout,key,(const char *)"test79.tuling123.com",audio,len,rate,format,text,textSize);
	printf("ret = %d\n",ret);
	gettimeofday(&endtime,0);
    double timeuse = 1000000*(endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
    //除以1000则进行毫秒计时，如果除以1000000则进行秒级别计时，如果除以1则进行微妙级别计时
   	printf("reqTlVoices usr time:  %d.%d\n",(int)timeuse/1000000,(int)timeuse/1000%1000);  
	return 	ret;
}

//#define MAIN_TEST
#ifdef MAIN_TEST
int main(void){
	const char *filename="./40002_8k.amr";
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
	char *fileData = (char *)calloc(1,file_len+1);
	if(fileData==NULL)
		return -1;
	int len = fread(fileData,1,file_len,fp);
	char *text=NULL;
	int textSize=0;
	fclose(fp);
	const char *key = "a2f6808bf85a693e1bde2069c8b7fd79";
	while(1){
		reqTlVoices(10,key,fileData,len,8000,"amr",&text,&textSize);
		if(text){
			printf("textSize =%d text = %s\n",textSize,text);
			free(text);
		}else{
			printf("req failed\n");
		}
		sleep(1);
	}
	return 0;
}
#endif
