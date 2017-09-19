#ifndef _ACK_WEIXIN_H
#define _ACK_WEIXIN_H

extern int uploadBaterry(const char *url,const char *mac,int battery);

extern int uploadVersion(const char *url,const char *mac,int battery,int rom);

#endif
