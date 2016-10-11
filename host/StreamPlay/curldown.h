#ifndef __CURLDOWN_H
#define __CURLDOWN_H 

#ifdef __cplusplus
extern "C" {
#endif

#define DOWN_QUIT	0
#define DOWN_WAIT	1
#define DOWN_ING	2

extern void parse_url(const char *url, char *domain, int *port, char *file_name);

extern void progressBar(long cur_size, long total_size,float *getPercent);

extern void quitDownFile(void);

extern void setDowning(void);

extern int getDownState(void);

extern void demoDownFile(const char *url,int timeout,void StartDownFile(const char *filename,int streamLen),void GetStreamData(const char *data,int size),void EndDownFile(int endSize));

extern void initCurl(void);

extern void cleanCurl(void);

#ifdef __cplusplus
};
#endif

#endif
