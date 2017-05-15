#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/cJSON.h"

#define 	NOT_PLAY_CMD		0
#define 	NEXT_CHECK			1
#define 	CHECK_CMD_OK		2
#define 	CHECK_FILE			3

#define HUASHANG_MUSIC_FILE	"home/huashang.bnf"
#define HUASHANG_GUOXUE_DIR		"huashangedu"
#define MAX_PLAY_NUMBER			3819

//#define MAIN_TEST
//#define PASRE_UTF8_LOG(fmt, args...) printf("%s: " fmt,__func__, ## args)
#define PASRE_UTF8_LOG(fmt, args...) { }

typedef struct{
	unsigned char nextCheckFlag;	//下一个汉字检查标记
	unsigned char pasreState;		//解析出关键标记
	char result[64];				//得出解析结果
	char cmd[12];					//得出解析命令
	int playIndexNum;				//得出播放歌曲编号
	int cacheNums;					//查找数字的时候缓存数字
	int resultSize;					//待查找汉字个数
	int findPointNums;				//查找到的关键个数
	int maxPoint;					//得出最大的关键字数
	void *data;						//主要用于指向读取文件当中一行数据指针
	cJSON *resultJsonData;			//解析出来的关键字，json格式存放
}utf8Parse_t;
/*
utf8Str: uft8 中文汉字
user_point:用户可以额外传递数据到回调函数当中，就不必设置全局变量
GetBytechar_ch :提取出一个汉字回调函数，进行处理
*/
void pasre_utf8(const char *utf8Str,void *user_point,int GetBytechar_ch(void *user_point,const char *byteChar,int byteSize)){
	int i=0,nbytes=0;
	const char *str = utf8Str;
	char byteChar[5];//utf8Str中的一个汉字
	for(i = 0; str[i] != '\0'; ){
		memset(byteChar,0,5);
		char chr = str[i];
		if((chr & 0x80) == 0){ //chr是0xxx xxxx，即ascii码
			nbytes=1;	
		}else if((chr & 0xF8) == 0xF8){ //chr是1111 1xxx
			nbytes=5;
		}else if((chr & 0xF0) == 0xF0){ //chr是1111 xxxx
			nbytes=4;
		}else if((chr & 0xE0) == 0xE0){ //chr是111x xxxx
			nbytes=3;
		}else if((chr & 0xC0) == 0xC0){ //chr是11xx xxxx
			nbytes=2;
		}
	    memcpy(byteChar,str+i,nbytes);
		byteChar[nbytes]='\0';
		//PASRE_UTF8_LOG("%s\n",byteChar); 
		i+=nbytes;
		if(!strcmp(byteChar,"\n")||!strcmp(byteChar,""))
			continue;
		if(GetBytechar_ch(user_point,byteChar,nbytes)){
			break;
		}
	}
}

//---------解析其他类型------------------------------------------------------------------
int GetBytechar_checkFile(void *user_point,const char *byteChar,int byteSize){
	utf8Parse_t *point =(utf8Parse_t *)user_point;
	//PASRE_UTF8_LOG("%s ： point data %s\n",__func__,point->data);
	if(strstr(point->data,byteChar)){
		point->findPointNums++;	//查找到关键字个数
		PASRE_UTF8_LOG("find =%s byteChar=%s\n",point->data,byteChar);
	}
	//PASRE_UTF8_LOG("%s ： end point data %s\n",__func__,point->data);
	return 0;
}
static int Get_Line(void *user_data,const char *lineStr,int lineNum){
	utf8Parse_t *point =(utf8Parse_t *)user_data;
	point->data = (char *)lineStr;
	//PASRE_UTF8_LOG("%s ： point data %s\n",__func__,point->data);
	//读取到一行，一个个汉字提取出来对比，找出关键字个数
	pasre_utf8(point->result,user_data,GetBytechar_checkFile);
	if(point->findPointNums>point->resultSize/2-1){    
		cJSON* pItem = cJSON_CreateObject();  
		cJSON_AddStringToObject(pItem, "line", lineStr);   
		cJSON_AddNumberToObject(pItem, "pointNums", point->findPointNums);  
		cJSON_AddItemToArray(point->resultJsonData, pItem); 	
	}
	point->findPointNums=0;
	return 0;
}
int ReadFile_Line(const char *filename,void *user_data,int Get_Line(void *user_data,const char *lineStr,int lineNum)){
     FILE * fp=NULL;  
     char * line = NULL;  
     size_t len = 0;  
     ssize_t read;  
     int nline=0;
     fp = fopen(filename, "r");  
     if (fp == NULL){ 
         return -1;
     }
   	    
     while ((read = getline(&line, &len, fp)) != -1) {  
        nline++;
		//PASRE_UTF8_LOG("line = %s \n",line);
		//PASRE_UTF8_LOG("line = %s \n",line);
		Get_Line(user_data,line,nline); 
    }  
    if (line)  
		free(line);
    fclose(fp);  
	return 0;
}
//实现解析一个汉字的回调函数
int GetBytechar_ch(void *user_point,const char *byteChar,int byteSize){
	//if(byteSize==1){	//不是汉字，直接退出
	//	return 0;
	//}
	int len =0;
	PASRE_UTF8_LOG("%s\n",byteChar);
	utf8Parse_t *point =(utf8Parse_t *)user_point;
	switch(point->nextCheckFlag){
		case NEXT_CHECK:
			if(!strcmp(byteChar,"放")){
				point->nextCheckFlag = CHECK_CMD_OK;
			}
			break;
		case NOT_PLAY_CMD:
			if(!strcmp(byteChar,"播")){
				PASRE_UTF8_LOG("get cmd \n");
				point->nextCheckFlag = NEXT_CHECK;
			}
			break;
		case CHECK_CMD_OK:	//已经过滤好播放这两个汉字
			len = strlen(point->result);
			snprintf(point->result+len,64-len,"%s",byteChar);
			point->resultSize++;		//需要识别 总汉字个数
			break;
	}
	return 0;
}
static void Parse_ResultLine_Id(const char *text,int *playIndex){
	int i=0,j=0;
	char nums[10]={0};
	char *str = (char *)text;
	for(i = 0; str[i] != '\0'; i++){
		if(str[i]=='('){
			break;
		}
	}
	for(; str[i] != '\0'; i++){
		if(str[i]==')'){
			break;
		}
		if(str[i]>='0'&&str[i]<='9')
			nums[j++]=str[i];
	}
	PASRE_UTF8_LOG("%s : text=%s nums=%s\n",__func__,text,nums);
	if(j>0){
		*playIndex = atoi(nums);
		PASRE_UTF8_LOG("%s : nums=%s playIndex=%d\n",__func__,nums,*playIndex);
	}
}
//解析识别出来播放歌曲的信息
static int Parse_TextJsonResult(const char *jsonData,int *playIndex){
	cJSON * pJson = cJSON_Parse(jsonData);
	if(NULL == pJson){
		return -1;
	}
	int iCount=0,i=0;
	int maxPoint=0;
	char Result[128]={0};
	cJSON * pArray =cJSON_GetObjectItem(pJson, "result");
    iCount = cJSON_GetArraySize(pArray);
    PASRE_UTF8_LOG("result iCount == %d \n",iCount);
    for (i=0; i < iCount; ++i) {
		cJSON* pItem = cJSON_GetArrayItem(pArray, i);
		if (NULL == pItem){
			continue;
		}
		cJSON* pj = cJSON_GetObjectItem(pItem,"pointNums");
		if(pj){
			int nums = pj->valueint;
			if(maxPoint<nums){
				pj = cJSON_GetObjectItem(pItem,"line");
				if(pj){
					snprintf(Result,128,"%s",pj->valuestring);
				}
				maxPoint=nums;
			}
		}
	}
	Parse_ResultLine_Id((const char *)Result,playIndex);
	cJSON_Delete(pJson);
	return 0;	
}

//解析播放指令
void Parse_playCmdMenu(const char *txt,int *playIndex){
	utf8Parse_t user_point;
	PASRE_UTF8_LOG("check Parse_playCmdMenu cmd  \n");
	memset(&user_point,0,sizeof(utf8Parse_t));
	pasre_utf8(txt,(void *)&user_point,GetBytechar_ch);
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON* pArray = cJSON_CreateArray();  
	cJSON_AddStringToObject(root,"srcdata",user_point.result);	
    cJSON_AddItemToObject(root, "result", pArray);
	user_point.resultJsonData = pArray;
	
	ReadFile_Line(HUASHANG_MUSIC_FILE,&user_point,Get_Line);
	char* szOut = cJSON_Print(root); 
	PASRE_UTF8_LOG("szOut =%s\n",szOut);
	Parse_TextJsonResult(szOut,playIndex);
	PASRE_UTF8_LOG("%s: get playIndex = %d\n",__func__,*playIndex);
	free(szOut);
	cJSON_Delete(root);
}
//---------------------------------------------------------------------- end 解析其他类型
//---------解析数字篇------------------------------------------------------------------
static int NumberMenu_GetBytechar_ch(void *user_point,const char *byteChar,int byteSize){
	int i=0; 
	int found =0;
	char matchStr[][10] = {"零","一","二","三","四","五","六","七","八","九"};
	//char matchAscii[][10] = {"0","1","2","3","4","5","6","7","8","9"};
	utf8Parse_t *point =(utf8Parse_t *)user_point;
	PASRE_UTF8_LOG("byteChar = %s\n",byteChar);
	for(i=0;i<10;i++){
		if(!strcmp((const char *)matchStr[i],byteChar)){
			point->cacheNums = i;	
			PASRE_UTF8_LOG("point->cacheNums %d \n",point->cacheNums);
			found=1;
			break;
		}
	}
	if(found==0){	//不在0-9范围,找十、百、千、万位
		if(!strcmp("十",byteChar)){
			if(point->playIndexNum==0){
				point->playIndexNum +=10;
			}else{
				point->playIndexNum +=point->cacheNums*10;
				point->cacheNums=0;
			}
		}else if(!strcmp("百",byteChar)){
			point->playIndexNum +=point->cacheNums*100;
			point->cacheNums=0;
		}else if(!strcmp("千",byteChar)){
			point->playIndexNum +=point->cacheNums*1000;
			point->cacheNums=0;
		}else if(!strcmp("万",byteChar)){
				point->playIndexNum +=point->cacheNums*10000;
				point->cacheNums=0;
		}
	}
	return 0;
}
//解析数字篇
static void Parse_NumberMenu(const char *txt,int *playIndex){
	utf8Parse_t user_point;
	memset(&user_point,0,sizeof(utf8Parse_t));
	pasre_utf8(txt,(void *)&user_point,NumberMenu_GetBytechar_ch);
	if(user_point.cacheNums!=0){
		user_point.playIndexNum +=user_point.cacheNums;
	}
	*playIndex= user_point.playIndexNum;
	PASRE_UTF8_LOG("user_point.playIndexNum = %d \n",user_point.playIndexNum);

}
//---------------------------------------------------------------------- end 解析数字篇
int Huashang_Checkutf8(const char *txt,char *playName){
	int playIndex=0;
#ifdef MAIN_TEST	
	if(strstr(txt,"播放"))
#endif		
	{
		PASRE_UTF8_LOG("check play cmd  \n");
		if(strstr(txt,"第")||strstr(txt,"的")||strstr(txt,"首")||strstr(txt,"手")){	//直接进入数字篇
			Parse_NumberMenu(txt,&playIndex);
		}else if(strstr(txt,"系列")){	//直接进入系列篇
			
		}else{	//其他项，采用二分查找
			Parse_playCmdMenu(txt,&playIndex);
		}
	}
		
	printf("%s: playIndex = %d\n",__func__,playIndex);
	if(playIndex==0){
		float maxplay =MAX_PLAY_NUMBER;
		int randIndex=(1+(int) (maxplay*rand()/(RAND_MAX+1.0)));
		playIndex=randIndex;
		printf("randIndex = %d\n",randIndex);
	}else if(playIndex>MAX_PLAY_NUMBER){
		playIndex =MAX_PLAY_NUMBER;
	}
	snprintf(playName,128,"/media/mmcblk0p1/%s/%d.mp3",HUASHANG_GUOXUE_DIR,playIndex);
	return 0;
}
//100111101100000
#ifdef MAIN_TEST
int main(void){  
    char str[]="播放第五首";//一串包含汉字的字串 	
	PASRE_UTF8_LOG("str len = %d\n",strlen(str));
	char playName[128]={0};
	char inputStr[128]={0};

	while(1){
		memset(inputStr,0,sizeof(inputStr));
		PASRE_UTF8_LOG("statrt input:\n");
		fgets(inputStr,sizeof(inputStr),stdin);
		if(!strcmp(inputStr,"q\n")){
			break;
		}
		PASRE_UTF8_LOG("\nintput : %s\n",inputStr);
		Huashang_Checkutf8(inputStr,playName);
		printf("playName = %s\n",playName);
	}
	return 0;  
}
#endif
