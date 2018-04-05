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


/*���ÿ��Ź���ʱ��ID*/
WDOG_ID wdgIdMain=NULL;

/*����Դ״̬�Ĳ���*/
struct oilPowerStateStruct
{
	int state;								/*��Ч״̬POWER_STATE_OK/POWER_STATE_ERR*/
	int stateLast;						/*ǰһ�̼�ʱ״̬*/
	unsigned int timer;				/*��Դ״̬��Чʱ��*/
};

struct oilPowerStateStruct oilPower={0,0,0};



/*Ϊʲô��ҹ��������������Ҫ׷���������ϳԶ���û����������ϰ���ˡ���
��Щ�����Ǳ����ǽ�ڡ���һ���˶ȹ���ʵ�������һ���
��Ȼ�᲻��ʮ��ͷ�����һմ��ͷ��˯�ú��㣻ҹҹ��������������Ҳ�Ĳ���
���䲻�˵��ˣ���������Ѷ԰����ʧ�������޾����������Ҳ�һЩ�������ˡ�*/

/*�����Тͨ�ڡ������й���������д�������й��˵ĵ��ºͷ��ɲ���һ�ѷ�֮�ĺ�����׼�Ĺ�ƽ���͹۵ĳ��ӡ�
�����Ը�����ʩ�������Լ��Ĺ�ϵ�����Գ̶��ϵ���������ν˫�ر�׼����*/


/*****************************************************************************
*Name				:tWdgEntry
*Description		:���ÿ��Ź���ʱ����Ӧ��ں���
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

	/*˰��SKL��ⳬʱ������*/
	taxSKLTimer++;

	/*��ʱ����Դ״̬��״̬����500ms��Ϊ��Ч��Դ״̬*/
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

	/*���Ź���ʱ1����*/
	wdStart(wdgIdMain, ONE_MILLI_SECOND, (FUNCPTR)tWdgEntry, 0);

	return;
}


/*****************************************************************************
*Name				:oilMain
*Description		:���ͻ�Ӧ������ں���
*Input				:None
*Output			:None
*Return			:None
*History			:2014-11-01,modified by syj
*/
static void oilMain()
{
	/*�����豸��ʼ��*/
	spkDevInit();

	/*�����豸��ʼ��*/
	framDevInit();

	/*�ļ�������ʼ��*/
	fileInit();

	/*SPI1(��stm32ͨѶ)��ʼ��*/
	spi1Init();

	/*�ն��豸ͨѶ��ʼ��(����������ʾ�ȹ���)*/
	//kbInit();
	tdInit();

	/*���ڳ�ʼ��(����9g20���ڼ���չ����,��Ϊ��SPI1��չ���ڹ�����ȳ�ʼ��SPI1ģ��)*/
	comInit();

	/*�����ļ�������ʼ��*/
	paramSetupInit();

	/*������ʼ��*/
	jlInit();

	/*�����ͨѶģ�鹦�ܳ�ʼ��*/
	//boardTransInit();

	/*PCD����ģ���ʼ��*/
	pcdInit();

	/*��ӡģ�鹦�ܳ�ʼ��*/
	printInit();

	/*��ʾ���ܳ�ʼ��*/
	dspInit();

	/*���������ܳ�ʼ��*/
	IcModuleInit();

	/*���빦�ܳ�ʼ��*/
	barcodeInit();

	/*����������ƽ�����ͨѶ�Ĺ��ܳ�ʼ��*/
	pcInit();

	/*֧���ն˹��ܳ�ʼ��*/
	iptInit();

	exit(0);
}


/*****************************************************************************
*Name				:entryPt
*Description		:����ں���(main)
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void entryPt()
{
	unsigned int ttbuffer[10]={0};

	/*���³�ʼ��˳�������ı䣬���ϵ�ʱĳЩģ������ǰһģ��Ĺ���*/
	/*����ϵͳʱ��Ƶ��Ϊ1000����1��tickΪ1ms*/
	sysClkRateSet(1000);

	/*����һ��1tick�Ĺ������Ź���ʱ��*/
	wdgIdMain=wdCreate();
	if(NULL==wdgIdMain)	printf("Error! Create watch dog timer 'wgIdMain' failed!\n");
	else								wdStart(wdgIdMain, 1, (FUNCPTR)tWdgEntry, 0);

	/*�������ͻ�Ӧ������ں�������*/
	taskSpawn("tOilMain", 200, 0, 0x4000, (FUNCPTR)oilMain, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
	
	return;
}


/*****************************************************************************
*Name				:oilMainExit
*Description		:����ں����˳�
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void oilMainExit()
{
	/*IPTģ��ע��*/
	//iptExit();

	/*����ģ��ע��*/
	//jlExit();

	/*˰��ģ��ע��*/
	taxExit();

	/*����ͨѶע��(����������ʾ�ȹ���)*/
	kbExit();

	/*����ע��(����9g20���ڼ���չ����,��Ϊ��SPI1��չ���ڹ�����ȳ�ʼ��SPI1ģ��)*/
	comExit();

	/*SPI1(��stm32ͨѶ)ע��*/
	spi1Exit();

	return;
}


/*****************************************************************************
*Name				:powerStateRead
*Description		:��ȡ��Դ״̬
*Input				:None
*Output			:None
*Return				:��Դ״̬0=����������=��Դ����
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

	/*��ȡ����������*/
	if(ERROR==usrBootlineGet(bootbuffer, 128)){
		logMsg("%s: get the bootline failed!\n", (int)__FUNCTION__, 2, 3,4,5,6);
		return ERROR;
	}

	/*��������������*/
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







