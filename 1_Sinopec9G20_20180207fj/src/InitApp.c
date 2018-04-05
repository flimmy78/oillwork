#include "../inc/main.h"
#include <linux/watchdog.h>

int g_nfdDog = -1;

bool InitDog()
{
	g_nfdDog = open("/dev/watchdog0",O_RDWR);
	if(g_nfdDog == -1)
	{
		perror("watchdog");
		return false;
	}
	int i = 8;
	ioctl(g_nfdDog,WDIOC_SETTIMEOUT,&i);
	return true;
}

void TriggerDog()
{
	if(g_nfdDog > 0)
	{
	    ioctl(g_nfdDog,WDIOC_KEEPALIVE,0);
	}
}

bool InitAllAppParameter()
{
	g_stru972Ip.Port = 8000;
	char chTemp[32] = "192.168.0.100";
	memcpy(g_stru972Ip.Ip_Addr,chTemp,strlen(chTemp));
    //g_stru972Ip.Ip_Addr = "192.168.1.200";
	//g_stru972Ip.Mask_Addr = "255.255.255.0";
	//g_stru972Ip.Gateway_Addr = "192.168.1.255";
	//g_stru972Ip.Mac_Addr = "02-00-00-00-00-01";

	if(false == InitDog())
	{
		printf("init dog failure\n");
		return false;
	}

	if(false == InitFJLog())
	{
		printf("init fjlog failure\n");
		return false;
	}
	g_fjLog.bIsWrite = false;

	if(false == initLog())
	{
		printf("init long failure\n");
		return false;
	}

	if(false == spkDevInit())
	{
		printf("init spk failure\n");
		return false;
	}

	if(false == framDevInit())
	{
		printf("init fram failure!\n");
		return false;
	}

	fileInit();

	if(false == spi1Init())
	{
		printf("init spi1 failure!\n");
		return false;
	}

	if(false == tdInit())
	{
        printf("init keyboard or liandi pos dev failure!\n");
		return false;
	}

//  if(false == kbInit())
//	{
//      printf("init keyboard dev failure!\n");
//	    return false;
//	}
	
    comInit();

	if(false == paramSetupInit())
	{
		printf("init param failure!\n");
		return false;
	}

	if(false == InitJL())
	{
		printf("init jl failure\n");
	   return false;
	}

	//printf("1111111111\n");

	if(false == pcdInit())
	{
		printf("init pcd failure!\n");
		return false;
	}

	//printf("2222222222\n");

	dspInit();

	if(false == IcModuleInit())
	{
		printf("init ic card failure!\n");
		return false;
	}

	//if(false == pcdInit())
	//{
	//	printf("init pcd failure!\n");
	//	return false;
	//}

	//printf("3333333333\n");

	//sleep(2);
	
	if(false == pcInit())
	{
		printf("init pc failure!\n");
		return false;
	}

	if(false == barcodeInit())
	{
		printf("init barcode failure\n");
		return false;
	}

	if(false == iptInit())
	{
		printf("init ipt failure!\n");
		return false;
	}

	//printf("4444444444\n");
//	if(false == InitFJLog())
//	{
//		printf("init fjlog failure");
//		return false;
//	}

	if(false == InitUpdateJLexe())
	{
		printf("init updatejlexe failure!\n");
		return false;
	}

	printf("Init is success!\n");
	return true;
}
