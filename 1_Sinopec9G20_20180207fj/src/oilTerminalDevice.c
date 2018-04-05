
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
*Description		:������ʾ,ÿ��������ʾ8�����֣������ʾ����
*Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
*						:FONTx				�ֿ�����ѡ��	=1		16*8,16*16�ֿ� 
       																					=2    	14*7,14*14�ֿ� 
	   																						=3   	8*8�ֿ�
	   																						=4    	12*6,12*12�ֿ�
*						:IsContrary			�Ƿ���0=������ʾ��1=����
*						:Offsetx				������0~7
*						:Offsety				������0~127
*						:buffer					��ʾ����
*						:nbytes				��ʾ���ݳ���
*Output			:None
*Return				:0=�ɹ�������=����
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
		jljRunLog("[Function:%s]�ն��豸ѡ�����!\n", __FUNCTION__);
		printf("tdDspContent Function\n");
		return -1;
	}

	if(0 == tdstruct->Device)
	{
		return -1;
	}

	if(TD_DEVICE_LIANDI == tdstruct->Device)
	{
		ldDspContent(DEV_DSP_KEYx, FONTx, IsContrary, Offsetx, Offsety, buffer, nbytes); //FJ��
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
*Description		:�ն���ʾ��Ӧ���ȵ���tbDspContent�����ʾ���ݺ���ñ�����
*Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
*						:Contrast				�Աȶ�HEX��ʽ
*						:IsClr					�Ƿ����ȫ��0=����գ�1=���
*Output			:None
*Return			:0=�ɹ�������=ʧ��
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
		jljRunLog("[Function:%s]�ն��豸ѡ�����!\n", __FUNCTION__);
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
*Description		:���̰�����ȡ
*Input				:kb_dev_id	�豸ѡ��	DEV_BUTTON_KEYA=A1����	DEV_BUTTON_KEYB=B1����
*Output			:None
*Return			:����ֵ���ް���ʱ����KB_BUTTON_NO
*History			:2013-07-01,modified by syj
*/
unsigned int tdButtonRead(int kb_dev_id)
{
	TdStructParamType *tdstruct  = NULL;

	if(DEV_BUTTON_KEYA == kb_dev_id)			tdstruct = &tdStructA1;
	else if(DEV_BUTTON_KEYB == kb_dev_id)	tdstruct = &tdStructB1;
	else
	{
		jljRunLog("[Function:%s]�ն��豸ѡ�����!\n", __FUNCTION__);
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
*Description		:��ȡ��Ʒ��ťֵ
*Input				:DEVx		�豸�� 
*						:				DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3/
*						:				DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3/
*						:				DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB
*Output			:value		��ťֵ 0=�ް�ť������1=�а�ť����
*Return			:0=�ɹ�������=ʧ�ܣ�
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
		jljRunLog("[Function:%s]�ն��豸ѡ�����!\n", __FUNCTION__);
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
*Description		:���н��׽ӿ�
*Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*						:pSend			���͵�DATA������
*						:nSendLen	���͵�DATA�����򳤶�
*Output			:pRec			���ص�DATA������
*						:pnRecLen	���ص�DATA�����򳤶�
*Return			:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2016-09-08,modified by syj
*/
int tdBankPayment(int nozzle, const void *pSend, unsigned int nSendLen, void *pRec, int *pnRecLen)
{
	int dev_dsp_keyx = 0;

	if(0 != nozzle && 1 != nozzle)
	{
		jljRunLog("[Function:%s]�ն��豸ѡ�����!\n", __FUNCTION__);
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
*Description	:����
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output		:None
*Return		:�ն��豸����:	'1' = �������̣�'2' = �����ն�ģ�飻
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
*Description		:�ն˹��ܳ�ʼ��
*Input				:��
*Output			:��
*Return			:0=�ɹ�������=ʧ��
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


