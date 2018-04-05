#include "../inc/main.h"

fjLogClass g_fjLog;
FILE* g_pFile = NULL;

bool InitLog()
{
     g_pFile = fopen(FJLOG_PATH,"a+");
	 if(g_pFile != NULL)
	 {
		  memset(g_fjLog.chModuleName,0,sizeof(g_fjLog.chModuleName));
		  memset(g_fjLog.chFunctionName,0,sizeof(g_fjLog.chFunctionName));
		  memset(g_fjLog.chTimes,0,sizeof(g_fjLog.chTimes));
		  memset(g_fjLog.chEvent,0,sizeof(g_fjLog.chEvent));
          g_fjLog.bIsOpen = true;
		  g_fjLog.bIsWrite = true;
	 }
	 else
	 {
		 return false;
	 }
	 return true;
}

void CloseLog()
{
	if(g_pFile != NULL)
	{
        fclose(g_pFile);
		g_fjLog.bIsOpen = false;
		g_fjLog.bIsWrite = false;
	}
}

void WriteLog(char* pchModuleName,char* pchFunctionName,unsigned char* pchEvent,int nEventLen)
{
	if(g_pFile == NULL || g_fjLog.bIsWrite == false)
	{
		return;
	}
	char chLog[512] = {0};
	int i;
	int nStrLen = 0;
	for(i = 0; i < strlen(pchModuleName); i++)
	{
		chLog[i] = pchModuleName[i];
	}
	nStrLen += strlen(pchModuleName);

	for(i = 0; i < strlen(pchFunctionName); i++)
	{
		chLog[i+nStrLen] = pchFunctionName[i];
	}
	nStrLen += strlen(pchFunctionName);

	char chTimes[32] = {0};
    time_t tmNow;
	struct tm* ptmNow;
	struct timespec tmSpec;
	time(&tmNow);
	ptmNow = localtime(&tmNow);
	clock_gettime(CLOCK_REALTIME,&tmSpec);
    sprintf(chTimes,"%d-%02d-%02d%c%02d:%02d:%02d.%03d;",1900+ptmNow->tm_year,1+ptmNow->tm_mon,ptmNow->tm_mday,'\040',ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec,tmSpec.tv_nsec / 1000000);
    for(i = 0; i < strlen(chTimes); i++)
	{
		chLog[i+nStrLen] = chTimes[i];
	}
	nStrLen += strlen(chTimes);

    if(nEventLen == 0)
	{
        //printf("------ char nEventLen = %d \n",nEventLen);
		for(i = 0; i < strlen(pchEvent); i++)
	    {
            chLog[i+nStrLen] = pchEvent[i];
		}
		nStrLen += strlen(pchEvent);
		sprintf(&chLog[nStrLen],"%s","\r\n");
	}
	else
	{
		//unsigned char uchTemp[256] = {0};
		//memcpy(uchTemp,pchEvent,nEventLen);
		for(i = 0; i < nEventLen; i++)
		{
			sprintf(&chLog[i*3+nStrLen],"%02x ",pchEvent[i]&0xff);
            //sprintf(&chLog[i+nStrLen],"%02x ",uchTemp[i]);
		}
        nStrLen += (nEventLen*3);
		sprintf(&chLog[nStrLen],"%s","\r\n");
	}
    rewind(g_pFile);
	fwrite(chLog,strlen(chLog),1,g_pFile);
	fflush(g_pFile);
}

bool InitFJLog()
{
	g_fjLog.InitLog = InitLog;
	g_fjLog.CloseLog = CloseLog;
	g_fjLog.WriteLog = WriteLog;

	if(g_fjLog.InitLog())
	{
		return true;
	}
	else
	{
		return false;
	}
}
