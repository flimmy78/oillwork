#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h> 
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../inc/main.h"

char Dir_Current[100]={0};   //当前操作路径
char Dir_Working[100]={0};   //exe程序所在的路径
int can_recv(void);
int g_nWatchDogNumber = 0;

struct oilPowerStateStruct
{
	int state;
	int stateLast;
	unsigned int timer;
};

struct oilPowerStateStruct oilPower = {0,0,0};

int powerStateRead(void)
{
	return oilPower.state;
}

/*
void tWdgEntry()
{
	int istate=0;

	IptParamA.PcdTxTimer++;
	IptParamA.OilDspTimer++;
	IptParamA.WarnigBeepTimer++;
	IptParamA.PassTimer++;
	IptParamA.NousedTimer++;
	IptParamA.CPOSTimer++;
	IptParamA.PriceChgTimer++;
	IptParamA.PowerStateTimer++;
	IptParamA.PlayLongTimer++;
	IptParamA.OilEndTimer++;

	IptParamB.PcdTxTimer++;
	IptParamB.OilDspTimer++;
	IptParamB.WarnigBeepTimer++;
	IptParamB.PassTimer++;
	IptParamB.NousedTimer++;
	IptParamB.CPOSTimer++;
	IptParamB.PriceChgTimer++;
	IptParamB.PowerStateTimer++;
	IptParamB.PlayLongTimer++;
	IptParamB.OilEndTimer++;


	dspTimerA++;
	dspTimerB++;

	spi1TransTimer++;

	pcdWdgIsr();

	//税控SKL检测超时计数器
	//taxSKLTimer++;   /fj:

	//定时监测电源状态，状态持续500ms则为有效电源状态 //fj:
	oilPower.timer++;
	//istate=pioReadBit(AT91C_BASE_PIOC, BIT15);
	//istate = 0;
	istate = GET_GPIO_VAL(0x01);
	if(istate!=oilPower.stateLast)
	{
		oilPower.stateLast=istate;	oilPower.timer=0;
	}
	if(oilPower.timer>=500*ONE_MILLI_SECOND)
	{
		oilPower.state=istate;
	}

	//看门狗定时1毫秒
	//wdStart(wdgIdMain, ONE_MILLI_SECOND, (FUNCPTR)tWdgEntry, 0);

	//return;
}*/


void tWdgEntry()
{
	int istate=0;

	IptParamA.PcdTxTimer +=10;
	IptParamA.OilDspTimer +=10;
	IptParamA.WarnigBeepTimer+=10;
	IptParamA.PassTimer+=10;
	IptParamA.NousedTimer+=10;
	IptParamA.CPOSTimer+=10;
	IptParamA.PriceChgTimer+=10;
	IptParamA.PowerStateTimer+=10;
	IptParamA.PlayLongTimer+=10;
	IptParamA.OilEndTimer+=10;
	IptParamA.EtcTxTime+=10;    //szb_fj_20171120,add
	IptParamA.EtcFreeObuTime+=10;//szb_fj_20171120,add

	IptParamB.PcdTxTimer+=10;
	IptParamB.OilDspTimer+=10;
	IptParamB.WarnigBeepTimer+=10;
	IptParamB.PassTimer+=10;
	IptParamB.NousedTimer+=10;
	IptParamB.CPOSTimer+=10;
	IptParamB.PriceChgTimer+=10;
	IptParamB.PowerStateTimer+=10;
	IptParamB.PlayLongTimer+=10;
	IptParamB.OilEndTimer+=10;
	IptParamB.EtcTxTime+=10;    //szb_fj_20171120,add
	IptParamB.EtcFreeObuTime+=10;//szb_fj_20171120,add


	dspTimerA+=10;
	dspTimerB+=10;

	spi1TransTimer+=10;

	g_nWatchDogNumber += 10;
	if(g_nWatchDogNumber >= 1000)
	{
		TriggerDog();
	}

	pcdWdgIsr();

	//税控SKL检测超时计数器
	//taxSKLTimer++;   /fj:

	//定时监测电源状态，状态持续500ms则为有效电源状态 //fj:
	oilPower.timer++;
	//istate=pioReadBit(AT91C_BASE_PIOC, BIT15);
	//istate = 0;
	istate = GET_GPIO_VAL(0x01);
	if(istate!=oilPower.stateLast)
	{
		oilPower.stateLast=istate;	oilPower.timer=0;
	}
	if(oilPower.timer>=50*ONE_MILLI_SECOND)
	{
		oilPower.state=istate;
	}

	//看门狗定时1毫秒
	//wdStart(wdgIdMain, ONE_MILLI_SECOND, (FUNCPTR)tWdgEntry, 0);

	//return;
}

/*
int main(int argc ,void *arg[])
{
	char chCurrentDir[100] = {0};
	char chWorkingDir[100] = {0};
	getcwd(chCurrentDir,sizeof(chCurrentDir));
	printf("Current path is: %s\n",chCurrentDir);
	chdir(dirname(arg[0]));
	getcwd(chWorkingDir,sizeof(chWorkingDir));
	printf("Working path is:%s\n",chWorkingDir);

	bcopy(chCurrentDir,chWorkingDir,100);

    InitAllAppParameter();

	//声明线程ID
	pthread_t tid1; 
	switch(tdStructB1.Device)
	{
		case TD_DEVICE_KEYBOARD:
			break;
		case TD_DEVICE_LIANDI:
			break;
		default:
			break;
	}
	pthread_t tid2;
	pthread_t tid3;
	pthread_t tid4;
	pthread_t tid5;
	pthread_t tid6;
	pthread_t tid7;

	pthread_create(&tid6,NULL,(void*)InitParam,0);
	pthread_create(&tid7,NULL,(void*)InitParam,0);


	pid_t child;
	child = fork();
	if(child < 0)
	{
        printf("error of fork!");
	}
	else if(child == 0)
	{
		int nProcessID = getpid();
		printf("create child1 process is success,ID = %d\n",nProcessID);
		char chSetPriorityCmd[64];
		sprintf(chSetPriorityCmd,"renice -n -1 -p %d",nProcessID);
		system(chSetPriorityCmd);

		pthread_create(&tid5,NULL,(void*)InitTestChildTd,0);
		while(1);
	}
	else
	{
		printf("parent process id = %d\n",getpid());
	}

	pid_t childEx;
	childEx = fork();
	if(childEx < 0)
	{
        printf("error of fork!");
	}
	else if(childEx == 0)
	{
		int nProcessID = getpid();
		printf("create child2 process is success,ID = %d\n",nProcessID); 
		char chSetPriorityCmd[64];
		sprintf(chSetPriorityCmd,"renice -n -1 -p %d",nProcessID);
		system(chSetPriorityCmd);
		while(1);
	}
	else
	{
		printf("parent process id = %d\n",getpid());
	}

    pthread_create(&tid6,NULL,(void*)InitTestParentTd,0);

	while(1);
	//wait(NULL);
}*/


/*
int main(int argc ,void *arg[])
{
	char chCurrentDir[100] = {0};
	char chWorkingDir[100] = {0};
	getcwd(chCurrentDir,sizeof(chCurrentDir));
	printf("Current path is: %s\n",chCurrentDir);
	chdir(dirname(arg[0]));
	getcwd(chWorkingDir,sizeof(chWorkingDir));
	printf("Working path is:%s\n",chWorkingDir);

	bcopy(chCurrentDir,chWorkingDir,100);

    if(InitAllAppParameter() == false)
	{
		printf("error,init app failure!\n");
		return 0;
	}

	//声明线程ID
	pthread_t tid1; 
	switch(tdStructB1.Device)
	{
		case TD_DEVICE_KEYBOARD:
			break;
		case TD_DEVICE_LIANDI:
			break;
		default:
			break;
	}


//   pthread_t td_KbRxCom4;
//   pthread_create(&td_KbRxCom4,NULL,(void*)tRxCom4,NULL);  //A键盘相关通讯及数据处理
//   pthread_t td_KbRxCom5;
//   pthread_create(&td_KbRxCom5,NULL,(void*)tRxCom5,NULL);  //B键盘相关通讯及数据处理
//   pthread_t td_KbSwitchRead;
//   pthread_create(&td_KbSwitchRead,NULL,(void*)kbTaskSwitchRead,NULL); //定时IO状态读取:

   pthread_t td_spitrasf;
   pthread_create(&td_spitrasf,NULL,(void*)tSpi1Transmit,NULL);

   pthread_t td_KbRxCom4;
   pthread_create(&td_KbRxCom4,NULL,(void*)tRxCom4,NULL);  //A键盘相关通讯及数据处理
   pthread_t td_KbRxCom5;
   pthread_create(&td_KbRxCom5,NULL,(void*)tRxCom5,NULL);  //B键盘相关通讯及数据处理
   pthread_t td_KbSwitchRead;
   pthread_create(&td_KbSwitchRead,NULL,(void*)kbTaskSwitchRead,NULL); //定时IO状态读取:

//   pthread_t td_JLProcA;
//   pthread_create(&td_JLProcA,NULL,(void*)JL_Process,NULL);

   pthread_t td_IcReceiveA;
   pthread_create(&td_IcReceiveA,NULL,(void*)tICReceive,0);    //A卡槽数据处理，ICC卡，银行卡
   pthread_t td_IcReceiveB;
   pthread_create(&td_IcReceiveB,NULL,(void*)tICReceive,1); //B卡槽数据处理，ICC卡，银行卡

   pthread_t td_PcdRx;
   pthread_create(&td_PcdRx,NULL,(void*)tPcd2PcRx,NULL);        //物联网模块，来自管控数据接收解析等处理
   pthread_t td_PcdProcess;
   pthread_create(&td_PcdProcess,NULL,(void*)tPcdProcess,NULL); //物联网模块，处理PC数据消息转发，结果推送IPT支付模块等。

   pthread_t td_IptDealA;
   pthread_create(&td_IptDealA,NULL,(void*)iptTask,0);
   pthread_t td_IptDealB;
   pthread_create(&td_IptDealB,NULL,(void*)iptTask,1);

   //fj:20170926
   pthread_t td_KbDspA;
   pthread_create(&td_KbDspA,NULL,(void*)tKbDsp,DEV_DSP_KEYA);  //键盘A显示处理，处理显示消息队列
   pthread_t td_KbDspB;
   pthread_create(&td_KbDspB,NULL,(void*)tKbDsp,DEV_DSP_KEYB);  //键盘B显示处理，处理显示消息队列

   pthread_t td_IcStateScanA;
   pthread_create(&td_IcStateScanA,NULL,(void*)tICPoll,0);  //卡槽A状态轮询,IC卡槽
   pthread_t td_IcStateScanB;
   pthread_create(&td_IcStateScanB,NULL,(void*)tICPoll,1);  //卡槽B状态轮询,IC卡槽

   while(1)
   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
	   struct timeval tv;
	   tv.tv_sec = 0;
	   tv.tv_usec = 1000;
	   tWdgEntry();
	   select(0,NULL,NULL,NULL,&tv);
	   //PrintTime("  main end --sec=%d,  ","millsec=%d\n");
   }
		//wait(NULL);
}*/

void dogEntry(void* pData)
{
  while(1)
   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
	   struct timeval tv;
	   tv.tv_sec = 0;
	   tv.tv_usec = 1000;
	   tWdgEntry();
	   select(0,NULL,NULL,NULL,&tv);
	   //PrintTime("  main end --sec=%d,  ","millsec=%d\n");
   }
}

int main(int argc ,void *arg[])
{
	system("insmod /pwm0-nuc972.ko");
	system("insmod /pwm2-nuc972.ko");
	system("insmod /FM25CL64.ko");

	char chCurrentDir[100] = {0};
	char chWorkingDir[100] = {0};
	getcwd(chCurrentDir,sizeof(chCurrentDir));
	printf("Current path is: %s\n",chCurrentDir);
	chdir(dirname(arg[0]));
	getcwd(chWorkingDir,sizeof(chWorkingDir));
	printf("Working path is:%s\n",chWorkingDir);

	bcopy(chCurrentDir,chWorkingDir,100);

	pthread_t td_dog;
	pthread_create(&td_dog,NULL,(void*)dogEntry,NULL);

    if(InitAllAppParameter() == false)
	{
		printf("error,init app failure!\n");
		return 0;
	}

	//unsigned char uchMacAddr[20];
	//if_updown("eth0",0);
	//ether_atoe(g_stru972Ip.Mac_Addr,uchMacAddr);
	//uchMacAddr[0] = 0x02;
	//set_mac_addr("eth0",uchMacAddr);
	//if_updown("eth0",1);
	//SetIfAddr("eth0",g_stru972Ip.Ip_Addr,g_stru972Ip.Mask_Addr,g_stru972Ip.Gateway_Addr);

//   while(1)
//   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
//	   struct timeval tv;
//	   tv.tv_sec = 2;
//	   tv.tv_usec = 0;
//	   select(0,NULL,NULL,NULL,&tv);
//	   break;
//   }

	//声明线程ID
	pthread_t tid1; 
	switch(tdStructB1.Device)
	{
		case TD_DEVICE_KEYBOARD:
			break;
		case TD_DEVICE_LIANDI:
			break;
		default:
			break;
	}

	pthread_t td_logSave;
	pthread_create(&td_logSave,NULL,(void*)jljLogSaveTask,NULL);

//   pthread_t td_KbRxCom4;
//   pthread_create(&td_KbRxCom4,NULL,(void*)tRxCom4,NULL);  //A键盘相关通讯及数据处理
//   pthread_t td_KbRxCom5;
//   pthread_create(&td_KbRxCom5,NULL,(void*)tRxCom5,NULL);  //B键盘相关通讯及数据处理
//   pthread_t td_KbSwitchRead;
//   pthread_create(&td_KbSwitchRead,NULL,(void*)kbTaskSwitchRead,NULL); //定时IO状态读取:

   pthread_t td_spitrasf;
   pthread_create(&td_spitrasf,NULL,(void*)tSpi1Transmit,NULL);

   pthread_t td_KbRxCom4;
   pthread_create(&td_KbRxCom4,NULL,(void*)tRxCom4,NULL);  //A键盘相关通讯及数据处理
   pthread_t td_KbRxCom5;
   pthread_create(&td_KbRxCom5,NULL,(void*)tRxCom5,NULL);  //B键盘相关通讯及数据处理
   pthread_t td_KbSwitchRead;
   pthread_create(&td_KbSwitchRead,NULL,(void*)kbTaskSwitchRead,NULL); //定时IO状态读取:

   //fj:20170926
   pthread_t td_KbDspA;
   pthread_create(&td_KbDspA,NULL,(void*)tKbDsp,DEV_DSP_KEYA);  //键盘A显示处理，处理显示消息队列
   pthread_t td_KbDspB;
   pthread_create(&td_KbDspB,NULL,(void*)tKbDsp,DEV_DSP_KEYB);  //键盘B显示处理，处理显示消息队列

   pthread_t td_JLProcess;
   pthread_create(&td_JLProcess,NULL,(void*)JL_Process,NULL);  //计量0x10轮询,A面板相关计量操作

   pthread_t td_PcdRx;
   pthread_create(&td_PcdRx,NULL,(void*)tPcd2PcRx,NULL);        //pcd物联网模块，来自管控数据接收解析等处理
   pthread_t td_PcdProcess;
   pthread_create(&td_PcdProcess,NULL,(void*)tPcdProcess,NULL); //pcd物联网模块，处理PC数据消息转发，结果推送IPT支付模块等。

   pthread_t td_IcReceiveA;
   pthread_create(&td_IcReceiveA,NULL,(void*)tICReceive,0);    //A卡槽数据处理，ICC卡，银行卡
   pthread_t td_IcStateScanA;
   pthread_create(&td_IcStateScanA,NULL,(void*)tICPoll,0);  //卡槽A状态轮询,IC卡槽
   //ResetIcPack(0);
   pthread_t td_IcReceiveB;
   pthread_create(&td_IcReceiveB,NULL,(void*)tICReceive,1); //B卡槽数据处理，ICC卡，银行卡
   pthread_t td_IcStateScanB;
   pthread_create(&td_IcStateScanB,NULL,(void*)tICPoll,1);  //卡槽B状态轮询,IC卡槽
   //ResetIcPack(1);

    while(1)
   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
	   struct timeval tv;
	   tv.tv_sec = 3;
	   tv.tv_usec = 0;
	   select(0,NULL,NULL,NULL,&tv);
	   break;
   } 

//   pthread_t td_PcdRx;
//   pthread_create(&td_PcdRx,NULL,(void*)tPcd2PcRx,NULL);        //pcd物联网模块，来自管控数据接收解析等处理
//   pthread_t td_PcdProcess;
//   pthread_create(&td_PcdProcess,NULL,(void*)tPcdProcess,NULL); //pcd物联网模块，处理PC数据消息转发，结果推送IPT支付模块等。


//  pthread_t td_IptDealA;
//   pthread_create(&td_IptDealA,NULL,(void*)iptTask,0);  //ipt支付终端模块，主轮询模块处理
//   pthread_t td_IptDealB;
//   pthread_create(&td_IptDealB,NULL,(void*)iptTask,1);   //ipt支付终端模块，主轮询模块处理

   //fj:20170926
  // pthread_t td_KbDspA;
  // pthread_create(&td_KbDspA,NULL,(void*)tKbDsp,DEV_DSP_KEYA);  //键盘A显示处理，处理显示消息队列
  // pthread_t td_KbDspB;
  // pthread_create(&td_KbDspB,NULL,(void*)tKbDsp,DEV_DSP_KEYB);  //键盘B显示处理，处理显示消息队列
  
/*   while(1)
   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
	   struct timeval tv;
	   tv.tv_sec = 2;
	   tv.tv_usec = 0;
	   select(0,NULL,NULL,NULL,&tv);
	   break;
   } */

   pthread_t td_voiceSpkA;
   pthread_create(&td_voiceSpkA,NULL,(void*)tSpeaker,0);
   pthread_t td_voiceSpkB;
   pthread_create(&td_voiceSpkB,NULL,(void*)tSpeaker,1);

   pthread_t td_printA;
   pthread_create(&td_printA,NULL,(void*)tPrintProcess,0);
   pthread_t td_printB;
   pthread_create(&td_printB,NULL,(void*)tPrintProcess,1);

   pthread_t td_BarcodeScanA;
   pthread_create(&td_BarcodeScanA,NULL,(void*)tBarcodeScan,0);
   pthread_t td_BarcodeReceiveA;
   pthread_create(&td_BarcodeReceiveA,NULL,(void*)tBarcodeReceive,0);
   pthread_t td_BarcodeScanB;
   pthread_create(&td_BarcodeScanB,NULL,(void*)tBarcodeScan,1);
   pthread_t td_BarcodeReceiveB;
   pthread_create(&td_BarcodeReceiveB,NULL,(void*)tBarcodeReceive,1);  
   pthread_t td_BcPosRx;
   pthread_create(&td_BcPosRx,NULL,(void*)tCPOSRx,NULL);

   pthread_t td_PcCommandA;
   pthread_create(&td_PcCommandA,NULL,(void*)tPcCommandProcess,0);
   pthread_t td_PcTxCommandA;
   pthread_create(&td_PcTxCommandA,NULL,(void*)tPcTxCommandProcess,0);
   pthread_t td_PcReceiveA;
   pthread_create(&td_PcReceiveA,NULL,(void*)tPcReceive,0);

   pthread_t td_PcCommandB;
   pthread_create(&td_PcCommandB,NULL,(void*)tPcCommandProcess,1);
   pthread_t td_PcTxCommandB;
   pthread_create(&td_PcTxCommandB,NULL,(void*)tPcTxCommandProcess,1);
   pthread_t td_PcReceiveB;
   pthread_create(&td_PcReceiveB,NULL,(void*)tPcReceive,1);

   pthread_t td_IptDealA;
   pthread_create(&td_IptDealA,NULL,(void*)iptTask,0);  //ipt支付终端模块，主轮询模块处理
   pthread_t td_IptDealB;
   pthread_create(&td_IptDealB,NULL,(void*)iptTask,1);   //ipt支付终端模块，主轮询模块处理

   pthread_t td_updatejlexe;
   pthread_create(&td_updatejlexe,NULL,(void*)CreateTcpServer,NULL);

   //while(1);

   while(1)
   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
	   struct timeval tv;
	   tv.tv_sec = 0;
	   tv.tv_usec = 1000;
	   tWdgEntry();
	   select(0,NULL,NULL,NULL,&tv);
	   //PrintTime("  main end --sec=%d,  ","millsec=%d\n");
   }
}
