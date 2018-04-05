#ifndef _OIL_KJLD_H_
#define _OIL_KJLD_H_

#define KJLD_DATA_MAX_SIZE		2048	//数据缓存长度

#define KJLD_CHANNEL_COM		'0'		//串口通讯
#define KJLD_CHANNEL_NET		'1'		//网口TCP通讯


extern int kjldDataIn(char *inbuffer, int nbytes);
extern int kjldDataOut(char *outbuffer, int maxbytes);
extern int kjldWrite(char *inbuffer, int nbytes);
extern int kjldRead(char *outbuffer, int maxbytes);
extern int kjldChannelSet(int channel);
extern int kjldChannelGet(void);
extern int kjldServerNetSet(const char *ip, const char *port);
extern int kjldServerNetGet(char *ip, char *port);
extern int kjldLocalPortSet(int port);
extern int kjldLocalPortGet(void);
extern int kjldInit(void);

//fj:20171214
extern int kjldLocalIpSet(char* pchLocalIp);
//extern int kjldLocalIpGet(char* pchLocalIp);
extern int kjldLocalMaskSet(int nLocalMask);
//extern int kjldLocalMaskGet(char* pchLocalMask);
extern int kjldLocalGatewaySet(char* pchLocalGateway);
//extern int kjldLocalGatewayGet(char* pchLocalGateway);
//extern int kjldLocalMacSet(char* pchMac);
//extern int kjldLocalMacGet(char* pchMac);
#endif

