//#include <dirLib.h>

#include "oilCfg.h"
#include "oilStmTransmit.h"
#include "oilTax.h"
#include "oilDsp.h"
#include "oilCom.h"
#include "oilIpt.h"
#include "oilSpk.h"
#include "oilBoardTrans.h"
#include "oilGas.h"
#include "oilMain.h"


/*公用看门狗定时器ID*/
WDOG_ID wdgIdMain=NULL;

/*检测电源状态的参数*/
struct oilPowerStateStruct
{
	int state;								/*有效状态POWER_STATE_OK/POWER_STATE_ERR*/
	int stateLast;						/*前一刻即时状态*/
	unsigned int timer;				/*电源状态有效时间*/
};

struct oilPowerStateStruct oilPower={0,0,0};



/*为什么熬夜？「还有两集剧要追」、「晚上吃多了没消化」、「习惯了」…
这些不过是表象，是借口。当一个人度过充实而满足的一天后，
自然会不到十点就犯困，一沾枕头就睡得很香；夜夜熬、天天困、改也改不掉
想戒戒不了的人，不过是想把对白天的失望，在无尽的拖延中找补一些回来罢了。*/

/*就像费孝通在《乡土中国》中这样写道：“中国人的道德和法律不是一把放之四海而皆准的公平，客观的尺子。
它可以根据所施对象与自己的关系而加以程度上的伸缩，可谓双重标准。”*/


/*****************************************************************************
*Name				:tWdgEntry
*Description		:公用看门狗定时器响应入口函数
*Input				:None
*Output			:None
*Return				:None
*History			:2014-11-01,modified by syj
*/
static void tWdgEntry()
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

	/*税控SKL检测超时计数器*/
	taxSKLTimer++;

	/*定时监测电源状态，状态持续500ms则为有效电源状态*/
	oilPower.timer++;
	istate=pioReadBit(AT91C_BASE_PIOC, BIT15);
	if(istate!=oilPower.stateLast)
	{
		oilPower.stateLast=istate;	oilPower.timer=0;
	}
	if(oilPower.timer>=500*ONE_MILLI_SECOND)
	{
		oilPower.state=istate;
	}

	/*看门狗定时1毫秒*/
	wdStart(wdgIdMain, ONE_MILLI_SECOND, (FUNCPTR)tWdgEntry, 0);

	return;
}


/*****************************************************************************
*Name				:oilMain
*Description		:加油机应用主入口函数
*Input				:None
*Output			:None
*Return			:None
*History			:2014-11-01,modified by syj
*/
static void oilMain()
{
	/*语音设备初始化*/
	spkDevInit();

	/*铁电设备初始化*/
	framDevInit();

	/*文件操作初始化*/
	fileInit();

	/*SPI1(即stm32通讯)初始化*/
	spi1Init();

	/*终端设备通讯初始化(包括键盘显示等功能)*/
	//kbInit();
	tdInit();

	/*串口初始化(包括9g20串口及扩展串口,因为有SPI1扩展串口故最好先初始化SPI1模块)*/
	comInit();

	/*配置文件操作初始化*/
	paramSetupInit();

	/*计量初始化*/
	jlInit();

	/*主板间通讯模块功能初始化*/
	//boardTransInit();

	/*PCD联网模块初始化*/
	pcdInit();

	/*打印模块功能初始化*/
	printInit();

	/*显示功能初始化*/
	dspInit();

	/*卡操作功能初始化*/
	IcModuleInit();

	/*条码功能初始化*/
	barcodeInit();

	/*促销机，与平板电脑通讯的功能初始化*/
	pcInit();

	/*支付终端功能初始化*/
	iptInit();

	exit(0);
}


/*****************************************************************************
*Name				:entryPt
*Description		:主入口函数(main)
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void entryPt()
{
	unsigned int ttbuffer[10]={0};

	/*以下初始化顺序慎做改变，因上电时某些模块依赖前一模块的功能*/
	/*设置系统时钟频率为1000，即1个tick为1ms*/
	sysClkRateSet(1000);

	/*创建一个1tick的公共看门狗定时器*/
	wdgIdMain=wdCreate();
	if(NULL==wdgIdMain)	printf("Error! Create watch dog timer 'wgIdMain' failed!\n");
	else								wdStart(wdgIdMain, 1, (FUNCPTR)tWdgEntry, 0);

	/*创建加油机应用主入口函数任务*/
	taskSpawn("tOilMain", 200, 0, 0x4000, (FUNCPTR)oilMain, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
	
	return;
}


/*****************************************************************************
*Name				:oilMainExit
*Description		:主入口函数退出
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void oilMainExit()
{
	/*IPT模块注销*/
	//iptExit();

	/*计量模块注销*/
	//jlExit();

	/*税控模块注销*/
	taxExit();

	/*键盘通讯注销(包括键盘显示等功能)*/
	kbExit();

	/*串口注销(包括9g20串口及扩展串口,因为有SPI1扩展串口故最好先初始化SPI1模块)*/
	comExit();

	/*SPI1(即stm32通讯)注销*/
	spi1Exit();

	return;
}


/*****************************************************************************
*Name				:powerStateRead
*Description		:获取电源状态
*Input				:None
*Output			:None
*Return				:电源状态0=正常；其他=电源掉电
*History			:2014-12-10,modified by syj
*/
int powerStateRead(void)
{
	return oilPower.state;
}


#if 0
int iitest11()
{
	char buffer[64]={0}, ppbuffer[64]={0};
	int istate=0, iidata=0;

	
	memset(buffer,0,64);
	istate=usrNetEMACGet("eth0", buffer);
	sprintf(ppbuffer, "%x:%x:%x:%x:%x:%x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6]);
	printf("line=%d:istate=%x:%s\n", __LINE__, istate, ppbuffer);

	memset(buffer,0,64);
	istate=usrNetHostGatewayGet("eth0", buffer);
	printf("line=%d:istate=%x:%s\n", __LINE__, istate, buffer);

	memset(buffer,0,64);
	istate=usrNetHostAddrGet("eth0", buffer);
	printf("line=%d:istate=%x:%s\n", __LINE__, istate, buffer);

	memset(buffer,0,64);
	istate=usrNetBroadcastGet("eth0", buffer);
	printf("line=%d:istate=%x:%s\n", __LINE__, istate, buffer);

	istate=usrNetMaskGet("eth0", &iidata);
	printf("line=%d:istate=%x:%x\n", __LINE__, istate, iidata);

	memset(buffer,0,64);
	istate=usrNetIpGet("eth0", buffer);
	printf("line=%d:istate=%x:%s\n", __LINE__, istate, buffer);

	return 0;
}

int iitest()
{
	int istate=0;
	
	istate=usrNetEMACSet("eth0", "\x10\x22\x33\x44\x55\x66");
	printf("line=%d:istate=%x\n", __LINE__, istate);

	istate=usrNetHostGatewaySet("eth0", "192.168.2.150");
	printf("line=%d:istate=%x\n", __LINE__, istate);
	
	istate=usrNetHostAddrSet("eth0", "192.168.3.244");
	printf("line=%d:istate=%x\n", __LINE__, istate);

	istate=usrNetBroadcastSet("eth0", "192.168.2.233");
	printf("line=%d:istate=%x\n", __LINE__, istate);

	istate=usrNetMaskSet("eth0", 0xfff00000);
	printf("line=%d:istate=%x\n", __LINE__, istate);
	
	istate=usrNetIpSet("eth0", "192.168.2.11");
	printf("line=%d:istate=%x\n", __LINE__, istate);
	

#if 0
	char buffer[64]={0};

	usrNetEMACGet("eth0", buffer);
	printf("222------%x-%x-%x-%x-%x-%x\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);

	memcpy(buffer, "\x01\x01\x03\x03\x06\x06", 6);
	usrNetEMACSet("eth0", buffer);
	printf("33333333333\n");


	char bootbuffer[128+1]={0};
	char ipbuffer[128+1]={0};
	char numString[64]={0};
	char *ipointer=NULL;
	BOOT_PARAMS params;

	/*获取启动项数据*/
	if(ERROR==usrBootlineGet(bootbuffer, 128)){
		logMsg("%s: get the bootline failed!\n", (int)__FUNCTION__, 2, 3,4,5,6);
		return ERROR;
	}

	/*解析启动项数据*/
	ipointer=bootStringToStruct(bootbuffer, &params);
	if(EOS!=*ipointer){
		logMsg("%s: bootStringToStruct failed!\n", (int)__FUNCTION__, 2, 3,4,5,6);
		return ERROR;
	}

	printf("params.had=%s\nparams.gad=%s\nline=%d\n",params.had, params.gad, __LINE__);

	inet_netof_string (params.had, numString);

	printf("params.had=%s\nparams.gad=%s\nnumString=%s\nline=%d\n",params.had, params.gad, numString, __LINE__);
	
	if (routeAdd (numString, params.gad) == ERROR)	printf("111111111111111\n");
#endif
	

	
	

	//istate=usrNetIpGet("eth0", buffer);
	//printf("[istate=%x]--%s\n", istate, buffer);

	//istate=usrNetIpSet("eth0", "192.168.2.20");
	//printf("[istate=%x]--%s\n", istate, "192.168.2.20");
	
	//istate=usrNetDefaultGatewaySet("eth0", "192.168.2.1");
	//printf("[istate=%x]--%s\n", istate, "192.168.2.20");

	return 0;
}

#endif







