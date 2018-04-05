
//#include "oilTerminalDevice.h"
//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilKb.h"
//#include "oilIC.h"
//#include "oilLog.h"
//#include "oilLianDi.h"

#include "../inc/main.h"

TdStructParamType tdStructA1, tdStructB1;


/********************************************************************
*Name				:tdDspContent
*Description		:键盘显示,每行最多可显示8个汉字，最多显示四行
*Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
*						:FONTx				字库类型选择	=1		16*8,16*16字库 
       																					=2    	14*7,14*14字库 
	   																						=3   	8*8字库
	   																						=4    	12*6,12*12字库
*						:IsContrary			是否反显0=正常显示；1=反显
*						:Offsetx				行坐标0~7
*						:Offsety				列坐标0~127
*						:buffer					显示数据
*						:nbytes				显示数据长度
*Output			:None
*Return				:0=成功；其它=错误
*History			:2013-07-01,modified by syj
*/
int tdDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes)
{
	TdStructParamType *tdstruct  = NULL;

	//printf("tdDspContent: DEV_DSP_KEYx = %d\n",DEV_DSP_KEYx);

	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			tdstruct = &tdStructA1;
	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	tdstruct = &tdStructB1;
	else
	{
		jljRunLog("[Function:%s]终端设备选择错误!\n", __FUNCTION__);
		printf("tdDspContent Function\n");
		return -1;
	}

	if(0 == tdstruct->Device)
	{
		return -1;
	}

	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		ldDspContent(DEV_DSP_KEYx, FONTx, IsContrary, Offsetx, Offsety, buffer, nbytes); //FJ：
	}
	else
	{
		kbDspContent(DEV_DSP_KEYx, FONTx, IsContrary, Offsetx, Offsety, buffer, nbytes);  //fj:
		//printf("kbDspContent \n");
	}

	return 0;
}


/********************************************************************
*Name				:tdDsp
*Description		:终端显示，应首先调用tbDspContent添加显示内容后调用本函数
*Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
*						:Contrast				对比度HEX格式
*						:IsClr					是否清空全屏0=不清空；1=清空
*Output			:None
*Return			:0=成功；其它=失败
*History			:2013-07-01,modified by syj
*/
int tdDsp(int DEV_DSP_KEYx, int Contrast, int IsClr)
{
	TdStructParamType *tdstruct  = NULL;

	//printf("tdDsp: DEV_DSP_KEYx = %d\n",DEV_DSP_KEYx);

	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			tdstruct = &tdStructA1;
	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	tdstruct = &tdStructB1;
	else
	{
		jljRunLog("[Function:%s]终端设备选择错误!\n", __FUNCTION__);
		printf("tdDsp Function\n");
		return -1;
	}

	if(0 == tdstruct->Device)
	{
		return -1;
	}

	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		ldDsp(DEV_DSP_KEYx, Contrast, IsClr);   //fj:
	}
	else
	{
		kbDsp(DEV_DSP_KEYx, Contrast, IsClr);    //fj:
		//printf("kbDsp\n");
	}

	return 0;
}


/********************************************************************
*Name				:tdButtonRead
*Description		:键盘按键读取
*Input				:kb_dev_id	设备选择	DEV_BUTTON_KEYA=A1键盘	DEV_BUTTON_KEYB=B1键盘
*Output			:None
*Return			:按键值，无按键时返回KB_BUTTON_NO
*History			:2013-07-01,modified by syj
*/
unsigned int tdButtonRead(int kb_dev_id)
{
	TdStructParamType *tdstruct  = NULL;

	if(DEV_BUTTON_KEYA == kb_dev_id)			tdstruct = &tdStructA1;
	else if(DEV_BUTTON_KEYB == kb_dev_id)	tdstruct = &tdStructB1;
	else
	{
		jljRunLog("[Function:%s]终端设备选择错误!\n", __FUNCTION__);
		return -1;
	}

	if(0 == tdstruct->Device)
	{
		return -1;
	}

	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		return ldButtonRead(kb_dev_id);  //fj:
	}
	else
	{
		return kbButtonRead(kb_dev_id);   //fj:
	}

	return KB_BUTTON_NO;
}


/********************************************************************
*Name				:tdYPRead
*Description		:获取油品按钮值
*Input				:DEVx		设备号 
*						:				DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3/
*						:				DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3/
*						:				DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB
*Output			:value		按钮值 0=无按钮操作；1=有按钮操作
*Return			:0=成功；其它=失败；
*History			:2016-05-10,modified by syj
*/
int tdYPRead(int DEVx, int *value)
{
	TdStructParamType *tdstruct  = NULL;

	if(DEV_SWITCH_SELA1 == DEVx || DEV_SWITCH_SELA2 == DEVx || DEV_SWITCH_SELA3 == DEVx || DEV_SWITCH_LOCKA == DEVx)			
		tdstruct = &tdStructA1;
	else if(DEV_SWITCH_SELB1 == DEVx || DEV_SWITCH_SELB2 == DEVx || DEV_SWITCH_SELB3 == DEVx || DEV_SWITCH_LOCKB == DEVx)	
		tdstruct = &tdStructB1;
	else
	{
		jljRunLog("[Function:%s]终端设备选择错误!\n", __FUNCTION__);
		return -1;
	}

	if(0 == tdstruct->Device)
	{
		return -1;
	}

	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		;
	}
	else
	{
		return kbYPRead(DEVx, value);  //fj:
	}

	return -1;
}


/********************************************************************
*Name				:tdBankPayment
*Description		:银行交易接口
*Input				:nozzle			枪选0=A1枪，1=B1枪
*						:pSend			发送的DATA数据域
*						:nSendLen	发送的DATA数据域长度
*Output			:pRec			返回的DATA数据域
*						:pnRecLen	返回的DATA数据域长度
*Return			:成功返回0；失败返回其它值
*History			:2016-09-08,modified by syj
*/
int tdBankPayment(int nozzle, const void *pSend, unsigned int nSendLen, void *pRec, int *pnRecLen)
{
	int dev_dsp_keyx = 0;

	if(0 != nozzle && 1 != nozzle)
	{
		jljRunLog("[Function:%s]终端设备选择错误!\n", __FUNCTION__);
		return -1;
	}

	if(0 == nozzle)	
		dev_dsp_keyx = DEV_DSP_KEYA;
	else	
		dev_dsp_keyx = DEV_DSP_KEYB;

	//return ldBankPayment(dev_dsp_keyx, pSend, nSendLen, pRec, pnRecLen);  //fj:
	return 0;  //fj:
}


/********************************************************************
*Name			:tdCardShoot
*Description	:弹卡
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output		:None
*Return		:终端设备类型:	'1' = 金属键盘；'2' = 联迪终端模块；
*History		:2014-10-17,modified by syj
*/
int tdDeviceGet(int nozzle)
{
	TdStructParamType *tdstruct  = NULL;

	if(0 == nozzle)			tdstruct = &tdStructA1;
	else if(1 == nozzle)	tdstruct = &tdStructB1;
	else
	{
		return -1;
	}

	return tdstruct->Device;
}


/********************************************************************
*Name				:tdInit
*Description		:终端功能初始化
*Input				:无
*Output			:无
*Return			:0=成功；其它=失败
*History			:2013-07-01,modified by syj
*/
bool tdInit(void)
{
/*	int pdev = TD_DEVICE_KEYBOARD;
//	int pdev = TD_DEVICE_LIANDI;
	TdStructParamType *tdstruct = NULL;

	tdstruct = &tdStructA1;
	tdstruct->Device = 0;
	tdstruct->Device = pdev;
	
	tdstruct = &tdStructB1;
	tdstruct->Device = 0;
	tdstruct->Device = pdev;
	
	if(TD_DEVICE_KEYBOARD == tdstruct->Device)
	{
		//kbInit();  //fj:
	}
	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		//ldInit(DEV_DSP_KEYA);  //fj:
		//ldInit(DEV_DSP_KEYB);  //fj:
	}*/

	int nDev = TD_DEVICE_KEYBOARD;
	//int nDev = TD_DEVICE_LIANDI;
	tdStructA1.Device = nDev;
	tdStructB1.Device = nDev;

	return kbInit();

  /*  if(tdStructB1.Device == TD_DEVICE_KEYBOARD)
	{
        kbInit();
	}
	else if(tdStructB1.Device == TD_DEVICE_LIANDI)
	{	
		ldInit(DEV_DSP_KEYA);  
		ldInit(DEV_DSP_KEYB);  
	}*/
	
	return true;
}


