#ifndef __FJLOG_H__
#define __FJLOG_H__

#define FJLOG_PATH     "../config/fjlog.txt"

typedef struct
{
	char chModuleName[32];
	char chFunctionName[32];
	char chTimes[32];
	char chEvent[256];
	bool bIsOpen;
	bool bIsWrite;
	bool (*InitLog)(void);
	void (*CloseLog)(void);
	void (*WriteLog)(char* pchModuleName,char* pchFunctionName,unsigned char* pchEvent,int nEventLen);
}fjLogClass;

extern fjLogClass g_fjLog;
bool   InitFJLog();





#endif
