//#include "oilKb.h"
//#include "oilDsp.h"
//#include "oilFRam.h"
//#include "oilParam.h"
//#include "oilIpt.h"
//#include "oilJL.h"
//#include "oilSpk.h"
//#include "oilLog.h"
//#include "oilPC.h"
//#include "oilMain.h"
//#include "oilAuthorize.h"

#include "../inc/main.h"

//��Ȩ����
AuthorizeDataType authorizeDataA1, authorizeDataB1;


//˽�нӿ�����
void authBalanceDsp(int nozzle, char *inbuffer, int nbytes);
void authBalance(int nozzle, char *inbuffer, int nbytes);
void authOilCheck(int nozzle, char *inbuffer, int nbytes);
void authOilStart(int nozzle, char *inbuffer, int nbytes);
void authOilling(int nozzle, char *inbuffer, int nbytes);
void authOilFinish(int nozzle, char *inbuffer, int nbytes);
void authDebitApply(int nozzle, char *inbuffer, int nbytes);
void authBillSave(int nozzle, char *inbuffer, int nbytes);
void authETCPasswordInput(int nozzle, char *inbuffer, int nbytes);
void authOilStartError(int nozzle, char *inbuffer, int nbytes);



/********************************************************************
*Name				:authorizeWrite
*Description		:����������ʾ�����ת�뱾���̵ȴ�������
*						:���������ʱ�޲������˳���
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	��Ȩ����(��Ȩ��4HEX + ��Ȩ��ʽ1Bin + ����ID��1byte��+����MAC�ţ�4bytes��+ OBU��ͬ���к�(8bytes)+���ƺţ�12bytes��+���ţ�10bytes��)
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ��
*History			:2016-04-12,modified by syj
*/
int authorizeWrite(int nozzle, const char *inbuffer)
{
	int i = 0;
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return ERROR;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//������Ȩֵ
	//taskLock(); //fj:
	
	pAuth->Unit = 0;
	pAuth->Amount = (*(inbuffer+0)<<24)|(*(inbuffer+1)<<16)|(*(inbuffer+2)<<8)|(*(inbuffer+3)<<0);
	pAuth->Model = *(inbuffer+4);
	pAuth->AntID = *(inbuffer+5);
	memcpy(pAuth->OBUID, inbuffer+6, 4);
	memcpy(pAuth->ContractNo, inbuffer+10, 8);
	memcpy(pAuth->OBUPlate, inbuffer+18, 12);
	memcpy(pAuth->CardID, inbuffer+30, 10);
	
	//taskUnlock(); //fj:
	
	return 0;
}


/********************************************************************
*Name				:authProccessExit
*Description		:�˳���Ȩ���ʹ���
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ��
*History			:2016-04-12,modified by syj
*/
int authProccessExit(int nozzle, char *inbuffer, int nbytes)
{
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return ERROR;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	iptMainInterface(iptparam);

	return 0;
}


/********************************************************************
*Name				:authBalancePretreat
*Description		:ת����Ȩ���ǰ��Ԥ����
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authBalancePretreat(int nozzle, char *inbuffer, int nbytes)
{
	char dsp_buffer[64] = {0};
	int dsp_len = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//���Ԥ������
	memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));	iptparam->IntegerLen=0;
	iptparam->Point=0;
	memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
	iptparam->PresetMode=IPT_PRESET_NO;

	//��Ȩ״̬
	iptparam->TaState = IPT_STATE_AUTH_BALANCE;	iptparam->TaStateParamLength = 0;
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>24);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>16);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>8);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>0);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
	memcpy(iptparam->TaStateParam + iptparam->TaStateParamLength, 0, 16);
	iptparam->TaStateParamLength += 16;
	pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);

	//��ʾ��Ȩ������
	dsp_buffer[0] = (char)(pAuth->Amount>>24);	dsp_buffer[1] = (char)(pAuth->Amount>>16);
	dsp_buffer[2] = (char)(pAuth->Amount>>8);	dsp_buffer[3] = (char)(pAuth->Amount>>0);
	dsp_buffer[4] = pAuth->Unit;
	dsp(iptparam->DEVDsp, DSP_AUTH_BALANCE, dsp_buffer, dsp_len);
	iptPidSet(iptparam, IPT_PID_AUTH_BALANCE);

	return;
}


/********************************************************************
*Name				:authBalance
*Description		:��Ȩ����������
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes			���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authBalance(int nozzle, char *inbuffer, int nbytes)
{
	char dsp_buffer[128] = {0};
	int dsp_len = 0;

	unsigned char button = 0;
	unsigned int data = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//��ǹ����ʼ����
	if(IPT_GUN_PUTUP == iptparam->GunState && 0 != iptparam->GunStateChg)
	{
		//��Ȩ����������״̬
		iptparam->TaState = IPT_STATE_AUTH_OILSTART;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
	
		iptPidSet(iptparam, IPT_PID_AUTH_OIL_CHECK);
		return;
	}

	//��ETC��Ȩ(��΢�Ż�֧������Ȩ)������������Ԥ��
	if(AUTH_MODEL_ETC != pAuth->Model)
	{
		return;
	}

	//��������
	switch(iptparam->Button)
	{
	case KB_BUTTON_0:
		if(0==button)	button='0';
	case KB_BUTTON_1:
		if(0==button)	button='1';
	case KB_BUTTON_2:
		if(0==button)	button='2';
	case KB_BUTTON_3:
		if(0==button)	button='3';
	case KB_BUTTON_4:
		if(0==button)	button='4';
	case KB_BUTTON_5:
		if(0==button)	button='5';
	case KB_BUTTON_6:
		if(0==button)	button='6';
	case KB_BUTTON_7:
		if(0==button)	button='7';
	case KB_BUTTON_8:
		if(0==button)	button='8';
	case KB_BUTTON_9:
		if(0==button)	button='9';
	case KB_BUTTON_CZ:
		if(0==button)	button='.';

		//��¼С����
		if('.'==button)
		{
			iptparam->Point='.';
		}
		//ǰһ����С���㰴����������¼����ֵΪ����
		else if(0==iptparam->Point)
		{
			iptparam->IntegerBuffer[0]=iptparam->IntegerBuffer[1];	iptparam->IntegerBuffer[1]=iptparam->IntegerBuffer[2];
			iptparam->IntegerBuffer[2]=iptparam->IntegerBuffer[3];	iptparam->IntegerBuffer[3]=iptparam->IntegerBuffer[4];	
			iptparam->IntegerBuffer[4]=iptparam->IntegerBuffer[5];	iptparam->IntegerBuffer[5]=button;
		}
		//ǰһ����С���㰴����������¼����ֵΪС��
		else if(0!=iptparam->Point && iptparam->DecimalLen<2)
		{
			iptparam->DecimalBuffer[iptparam->DecimalLen++]=button;
		}

		//��Ԥ��ģʽĬ��ΪԤ�ý��
		if(IPT_PRESET_NO==iptparam->PresetMode)	iptparam->PresetMode=IPT_PRESET_MONEY;

		//����Ԥ��ֵ
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//��ʾԤ�ý���
		dsp_buffer[0]=(char)(data>>24);	dsp_buffer[1]=(char)(data>>16);
		dsp_buffer[2]=(char)(data>>8);	dsp_buffer[3]=(char)(data>>0);
		if(IPT_PRESET_VOLUME==iptparam->PresetMode)	dsp_buffer[4]=1;
		else																			dsp_buffer[4]=0;
		dsp_len=5;
		dsp(iptparam->DEVDsp, DSP_CARD_PRESET, dsp_buffer, dsp_len);
		break;

	case KB_BUTTON_MON:
	case KB_BUTTON_VOL:
		//����Ԥ��ֵ
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//����Ԥ�÷�ʽ
		if(KB_BUTTON_MON==iptparam->Button)	iptparam->PresetMode=IPT_PRESET_MONEY;
		else																iptparam->PresetMode=IPT_PRESET_VOLUME;
		dsp_buffer[0]=(char)(data>>24);	dsp_buffer[1]=(char)(data>>16);
		dsp_buffer[2]=(char)(data>>8);	dsp_buffer[3]=(char)(data>>0);
		if(IPT_PRESET_VOLUME==iptparam->PresetMode)	dsp_buffer[4]=1;
		else																			dsp_buffer[4]=0;
		dsp_len=5;
		dsp(iptparam->DEVDsp, DSP_CARD_PRESET, dsp_buffer, dsp_len);
		break;

	case KB_BUTTON_CHG:
		//����Ԥ��ֵ
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//���Ԥ����Ϊ0��ʾ�������棬�������Ԥ����
		if(0==data)
		{
			dsp_buffer[0] = (char)(pAuth->Amount>>24);	dsp_buffer[1] = (char)(pAuth->Amount>>16);
			dsp_buffer[2] = (char)(pAuth->Amount>>8);	dsp_buffer[3] = (char)(pAuth->Amount>>0);
			dsp_buffer[4] = pAuth->Unit;
			dsp(iptparam->DEVDsp, DSP_AUTH_BALANCE, dsp_buffer, dsp_len);
		}
		else
		{
			memset(dsp_buffer, 0, 4);
			if(IPT_PRESET_VOLUME==iptparam->PresetMode)	dsp_buffer[4]=1;
			else																			dsp_buffer[4]=0;
			dsp_len=5;
			dsp(iptparam->DEVDsp, DSP_CARD_PRESET, dsp_buffer, dsp_len);
		}

		//���Ԥ������
		memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));	iptparam->IntegerLen=0;
		iptparam->Point=0;
		memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
		iptparam->PresetMode=IPT_PRESET_NO;
		break;

	case KB_BUTTON_BACK:
		iptMainInterface(iptparam);
		break;
	default:
		break;
	}

	return;
}


/********************************************************************
*Name				:authOilCheck
*Description		:��Ȩ��������ǰ�����������Ƿ�Ϸ������ĳЩԤ�������
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authOilCheck(int nozzle, char *inbuffer, int nbytes)
{
	char tmp_buffer[32] = {0};
	char dsp_buffer[64] = {0};
	int dsp_len = 0;
	unsigned int data = 0;
	unsigned int money_max = 0, volume_max = 0, money_min = 0, volume_min = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//�жϵ�Դ״̬
	if(POWER_STATE_OK!=powerStateRead())
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�ͻ���Դ״̬�쳣", 16);
		IPT_DSP_WAIT();
		
		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ��ͻ���Դ״̬�쳣!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж��˵��Ƿ����
	if(iptparam->FuelUnloadNumber>=IPT_BILLUNLOAD_MAX)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�˵����", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ��˵����!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�жϻ�ȡ���۽��
	if(0!=jlPriceRead(iptparam->JlNozzle, &iptparam->OilPrice))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "��ȡ����ʧ��", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ���ȡ��������ʧ��!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	if(iptparam->JlErr_BianJia!=0)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "���ʧ��", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ����ʧ��!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�жϵ��ۺϷ���
	if((iptparam->OilPrice<IPT_PRICE_MIN)||(iptparam->OilPrice>IPT_PRICE_MAX))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "���۷Ƿ�", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ����۷Ƿ�!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж���������
	if(IPT_SELL_UNLOCK!=iptparam->SellLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "������������", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ�������������!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�ҹ������
	if(IPT_NIGHT_UNLOCK!=iptparam->NightLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "ҹ������", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ�ҹ������!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�Կ��״̬
	if(KB_KEYLOCK_OIL!=iptparam->KeyLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "���Կ�״򵽼���λ��", strlen("���Կ�״򵽼���λ��"));
		IPT_DSP_WAIT();

		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�ʱ��Ϸ���
	tmp_buffer[0]=iptparam->Time.century;	tmp_buffer[1]=iptparam->Time.year;		tmp_buffer[2]=iptparam->Time.month;	tmp_buffer[3]=iptparam->Time.date;
	tmp_buffer[4]=iptparam->Time.hour;		tmp_buffer[5]=iptparam->Time.minute;	tmp_buffer[6]=iptparam->Time.second;
	if(0!=timeVerification(tmp_buffer, 7))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�ͻ�ʱ��Ƿ�", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ��ͻ�ʱ��Ƿ�!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�����ǹ�źϷ���
	if(iptparam->PhysicalNozzle<1 || iptparam->PhysicalNozzle>6)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "����ǹ�ŷǷ�", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ�����ǹ�ŷǷ�!����ǹ��=%s.", iptparam->PhysicalNozzle);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�PCD����״̬
	if(1!=iptparam->PcdState)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "��PCD���ӶϿ�", 13);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ���PCD���ӶϿ�!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�PCD�쳣״̬
	if(0!=iptparam->PcdErrNO)
	{
		memcpy(&dsp_buffer[0], "PCD״̬�쳣", 11);
		dsp_buffer[11]=(iptparam->PcdErrNO>>4)&0x0f+0x30;
		dsp_buffer[12]=(iptparam->PcdErrNO>>0)&0x0f+0x30;
		dsp_len=13;
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, dsp_buffer, dsp_len);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ�PCD״̬�쳣!�������=%x.", iptparam->PcdErrNO);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�ͨ����Ϣ�汾�Ϸ���
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==IptPcInfo.SInfo.Version)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "ͨ����Ϣ�汾�Ƿ�", 16);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ�ͨ����Ϣ�汾�Ƿ�!�汾=%d.", IptPcInfo.SInfo.Version);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж���Ʒ�ͼ۰汾
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==IptPcInfo.OilInfo.Version)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "��Ʒ�ͼ۰汾�Ƿ�", 16);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ���Ʒ�ͼ۰汾�Ƿ�!�汾=%d.", IptPcInfo.OilInfo.Version);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж��߼�ǹ�źϷ���
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==iptparam->LogicNozzle)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�߼�ǹ�ŷǷ�", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "��������ʧ�ܣ��߼�ǹ�ŷǷ�!�߼�ǹ��=%d.", iptparam->LogicNozzle);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//������������������
	//ȡ��Ȩ���Ĭ������������Сֵ
	
	if(pAuth->Amount <= JL_MONEY_MAX)	money_max = pAuth->Amount;	
	else																money_max = JL_MONEY_MAX;
	volume_max = JL_VOLUME_MAX;

	//����Ԥ����
	//Ԥ����Ϊ����Ԥ�ü�Ĭ��Ϊ������ͣ�
	//���������ͣ�ͨ����������Ԥ�ý�
	//�������ͣ�ͨ��������Ԥ��������
	//������ͣ������������ΪԤ�ý�������Ԥ��������
	//��ע�⣬�������ж�������Ϣ�������ʱ���ܻ�����Ԥ�ã�
	
	data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;
	if(0 == data || (IPT_PRESET_VOLUME!=iptparam->PresetMode && IPT_PRESET_MONEY!=iptparam->PresetMode))
	{
		iptparam->PresetMoney = money_max;	iptparam->PresetVolume = volume_max;	iptparam->PresetMode = IPT_PRESET_NO;
	}
	else if(IPT_PRESET_VOLUME == iptparam->PresetMode)
	{
		iptparam->PresetMoney = 0;	iptparam->PresetVolume=data;	iptparam->PresetMode = IPT_PRESET_VOLUME;
	}
	else
	{
		iptparam->PresetMoney = 0;	iptparam->PresetMoney=data;		iptparam->PresetMode = IPT_PRESET_MONEY;
	}
	dsp_buffer[0] = (char)(money_max>>24);						dsp_buffer[1] = (char)(money_max>>16);
	dsp_buffer[2] = (char)(money_max>>8);							dsp_buffer[3] = (char)(money_max>>0);
	dsp_buffer[4] = (char)(volume_max>>24);						dsp_buffer[5] = (char)(volume_max>>16);
	dsp_buffer[6] = (char)(volume_max>>8);						dsp_buffer[7] = (char)(volume_max>>0);
	dsp_buffer[8] = (char)(iptparam->PresetMoney>>24);		dsp_buffer[9] = (char)(iptparam->PresetMoney>>16);
	dsp_buffer[10] = (char)(iptparam->PresetMoney>>8);		dsp_buffer[11] = (char)(iptparam->PresetMoney>>0);
	dsp_buffer[12] = (char)(iptparam->PresetVolume>>24);	dsp_buffer[13] = (char)(iptparam->PresetVolume>>16);
	dsp_buffer[14] = (char)(iptparam->PresetVolume>>8);	dsp_buffer[15] = (char)(iptparam->PresetVolume>>0);
	dsp_buffer[16] = (char)(iptparam->OilPrice>>24);			dsp_buffer[17] = (char)(iptparam->OilPrice>>16);
	dsp_buffer[18] = (char)(iptparam->OilPrice>>8);				dsp_buffer[19] = (char)(iptparam->OilPrice>>0);
	dsp_buffer[20] = iptparam->PresetMode;
	jlPresetAmountCalculate(iptparam->JlNozzle, dsp_buffer);
	iptparam->PresetMoney = (dsp_buffer[8]<<24)|(dsp_buffer[9]<<16)|(dsp_buffer[10]<<8)|(dsp_buffer[11]<<0);
	iptparam->PresetVolume = (dsp_buffer[12]<<24)|(dsp_buffer[13]<<16)|(dsp_buffer[14]<<8)|(dsp_buffer[15]<<0);

	//�ж�Ԥ�����Ƿ��С����С��1��
	if(iptparam->PresetMoney<iptparam->OilPrice || iptparam->PresetMoney<IPT_MONEY_MIN || iptparam->PresetVolume<IPT_VOLUME_MIN)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "Ԥ����̫С", 10);
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж�Ԥ�����Ƿ����
	if(iptparam->PresetMoney>JL_MONEY_MAX || iptparam->PresetVolume>JL_VOLUME_MAX)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "Ԥ����̫��", 10);
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//�ж����
	if(iptparam->PresetMoney > pAuth->Amount)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "����", strlen("����"));
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//��¼����ʱ�䣬��ʼ�����μ�������
	iptparam->OilTime[0]=iptparam->Time.century;	iptparam->OilTime[1]=iptparam->Time.year;
	iptparam->OilTime[2]=iptparam->Time.month;		iptparam->OilTime[3]=iptparam->Time.date;
	iptparam->OilTime[4]=iptparam->Time.hour;		iptparam->OilTime[5]=iptparam->Time.minute;
	iptparam->OilTime[6]=iptparam->Time.second;
	iptparam->OilMoney=0;	iptparam->OilVolume=0;		iptparam->OilPrice=iptparam->OilPrice;
	jlSumRead(iptparam->JlNozzle, &(iptparam->SumVolume), &(iptparam->SumMoney));
	//GetjlSumRead(iptparam->JlNozzle,&(iptparam->SumVolume),&(iptparam->SumMoney));
	iptparam->OilRound=0;

	//��������
	dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "���������У�           ���Ժ�...", strlen("���������У�           ���Ժ�..."));
	iptPidSet(iptparam, IPT_PID_AUTH_OIL_START);

	return;
}


/********************************************************************
*Name				:authOilStart
*Description		:��Ȩ������������
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return			:��
*History			:2016-04-12,modified by syj
*/
void authOilStart(int nozzle, char *inbuffer, int nbytes)
{
	char tmp_buffer[32] = {0};
	int voice[16] = {0};
	int i = 0;
	int istate = 0;

	char dsp_buffer[64] = {0};
	int dsp_len = 0;

	unsigned long long preset_amount = 0;
	unsigned int preset_price = 0,  preset_mode = 0;

	unsigned long long volume_sum = 0, money_sum = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//���������������
	if(IPT_PRESET_VOLUME==iptparam->PresetMode)	{preset_amount = iptparam->PresetVolume;	preset_price = iptparam->OilPrice;	preset_mode = IPT_PRESET_VOLUME;	}
	else																				{preset_amount = iptparam->PresetMoney;		preset_price = iptparam->OilPrice;	preset_mode = IPT_PRESET_MONEY;	}
	for(i=0; i<3; i++)
	{
		printf("auto oiling: preset_amount = %d,preset_price = %d\n",preset_amount,preset_price);
		istate=jlOilStart(iptparam->JlNozzle, preset_amount, preset_price, preset_mode);
		if(0==istate)	break;

		//taskDelay(1);
		usleep(1000);
	}

	//���������������ʧ�ܣ���ʾʧ����Ϣ����¼������־��������Ȩ������
	if(0 != istate)
	{
		memset(dsp_buffer, 0, sizeof(dsp_buffer));
		memcpy(dsp_buffer, "��������ʧ��    ", 16);
		i=((char)(istate>>4)&0x0f)*10+((char)(istate>>0)&0x0f)*1;
		memcpy(dsp_buffer+strlen(dsp_buffer), jlStartFiledReson[i], strlen(jlStartFiledReson[i]));
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, dsp_buffer, strlen(dsp_buffer));
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "%s", dsp_buffer);

		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//������ʾ"����ǹ��עXX�������ѻ��㣬��ȷ��"
	if(iptparam->VoiceVolume>0 && IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)
	{
		voice[0]=SPKW_OILFILL;															//����ǹ��ע
																										//��Ʒ��������
		if(0==memcmp(iptparam->OilVoice, "\x00\x00\x00\x00", 4))
			voice[1]=iptOilVoiceIdGet(iptparam, iptparam->OilCode);
		else
			voice[1]=((iptparam->OilVoice[0]&0x0f)<<12)|((iptparam->OilVoice[1]&0x0f)<<8)|\
							((iptparam->OilVoice[2]&0x0f)<<4)|((iptparam->OilVoice[3]&0x0f)<<0);
		voice[2]=SPKW_OILACK;															//�����ѻ��㣬��ȷ��
		iptSpk(iptparam, voice, 3);
	}
	else if(iptparam->VoiceVolume>0 && IPT_VOICE_TYPE_MAN==iptparam->VoiceType)
	{
		voice[0]=SPKM_OILFILL;															//����ǹ��ע
																										    //��Ʒ��������
		if(0==memcmp(iptparam->OilVoice, "\x00\x00\x00\x00", 4))
			voice[1]=iptOilVoiceIdGet(iptparam, iptparam->OilCode);
		else
			voice[1]=((iptparam->OilVoice[0]&0x0f)<<12)|((iptparam->OilVoice[1]&0x0f)<<8)|\
							((iptparam->OilVoice[2]&0x0f)<<4)|((iptparam->OilVoice[3]&0x0f)<<0);
		voice[2]=SPKM_OILACK;															 //�����ѻ��㣬��ȷ��
		iptSpk(iptparam, voice, 3);
	}

	//��������״̬��ʼ��
	iptparam->VoiceFlag=0;
	iptparam->OilState=IPT_OIL_FUELLING;

	//�����ۿ���ԴΪETC���ķǿ��˵�
	//�����:POS_TTC
	memcpy(&iptparam->OilBill[IPT_OFFSET_TTC], "\x00\x00\x00\x00", 4);

	//�����:����������Ϊ�޿�����̨/���غڰ��������ۿ�ǩ����Ч���ӿ�
	iptparam->OilBill[IPT_OFFSET_T_TYPE]=(0<<7)|(0<<6)|(0<<4)|(7<<0);

	//�����:�������ڼ�ʱ��
	iptparam->OilTime[0]=iptparam->Time.century;	iptparam->OilTime[1]=iptparam->Time.year;
	iptparam->OilTime[2]=iptparam->Time.month;		iptparam->OilTime[3]=iptparam->Time.date;
	iptparam->OilTime[4]=iptparam->Time.hour;		iptparam->OilTime[5]=iptparam->Time.minute;
	iptparam->OilTime[6]=iptparam->Time.second;
	memcpy(&iptparam->OilBill[IPT_OFFSET_TIME], iptparam->OilTime, 7);

	//�����:��Ӧ�ú�
	memcpy(&iptparam->OilBill[IPT_OFFSET_ASN], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

	//�����:���(��ǰ)��������ɺ���ۺ����
	*(tmp_buffer + 0) = (char)(pAuth->Amount>>24);	*(tmp_buffer + 1) = (char)(pAuth->Amount>>16);
	*(tmp_buffer + 2) = (char)(pAuth->Amount>>8);	*(tmp_buffer + 3) = (char)(pAuth->Amount>>0);
	memcpy(&iptparam->OilBill[IPT_OFFSET_BALANCE], tmp_buffer, 4);

	//�����:���������ɺ���ʵ������
	memcpy(&iptparam->OilBill[IPT_OFFSET_AMN], "\x00\x00\x00", 3);

	//�����:���������
	memcpy(&iptparam->OilBill[IPT_OFFSET_CTC], "\x00\x00", 2);

	//�����:����ǩ��������/����/����ʱδTAC���ӿ�ʱΪGTAC
	memcpy(&iptparam->OilBill[IPT_OFFSET_TAC], "\x00\x00\x00\x00", 4);

	//�����:�����֤��4bytes��������ɣ�PSAM����GMAC�����
	memcpy(&iptparam->OilBill[IPT_OFFSET_GMAC], "\x00\x00\x00\x00", 4);

	//�����:PSAM����ǩ��4bytes��������ɣ�PSAM����GMAC�����
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_TAC], "\x00\x00\x00\x00", 4);

	//�����:PSAMӦ�ú�10bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_ASN], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

	//�����:PSAM���6bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_TID], "\x00\x00\x00\x00\x00\x00", 6);
			
	//�����:PSAM�ն˽������4bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_TTC], "\x00\x00\x00\x00", 4);

	//�����:�ۿ���Դ1byte
	if(AUTH_MODEL_WECHAT == pAuth->Model)		iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_WECHAT;
	else if(AUTH_MODEL_ALIPAY == pAuth->Model)	iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_ALIPAY;
	else																			iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_ETC;

	//�����:���㵥λ/��ʽ1byte
	iptparam->OilBill[IPT_OFFSET_UNIT] = 0;
			
	//�����:����1byte
	iptparam->OilBill[IPT_OFFSET_C_TYPE] = 0;

	//�����:���汾1byte��b7~b4����Կ�����ţ�b3~b0����Կ�汾��
	iptparam->OilBill[IPT_OFFSET_VER] = 0;

	//�����:ǹ��1byte
	iptparam->OilBill[IPT_OFFSET_NZN] = iptparam->LogicNozzle;

	//�����:��Ʒ����2bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_G_CODE], iptparam->OilCode, 2);

	//����3bytes��������ɺ���ʵ������
	memcpy(&iptparam->OilBill[IPT_OFFSET_VOL], "\x00\x00\x00", 3);

	//�����:�ɽ��۸�2bytes�������ۼƽ���������
	iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);
	iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);

	//�����:Ա����1byte
	iptparam->OilBill[IPT_OFFSET_EMP]=iptparam->EMP;

	//�����:���ۼ�4bytes
	jlSumRead(iptparam->JlNozzle, &volume_sum, &money_sum);
	//GetjlSumRead(iptparam->JlNozzle,&volume_sum,&money_sum);
	iptparam->OilBill[IPT_OFFSET_V_TOT+0]=(char)(volume_sum>>24);
	iptparam->OilBill[IPT_OFFSET_V_TOT+1]=(char)(volume_sum>>16);
	iptparam->OilBill[IPT_OFFSET_V_TOT+2]=(char)(volume_sum>>8);	
	iptparam->OilBill[IPT_OFFSET_V_TOT+3]=(char)(volume_sum>>0);

	//�����:���ò���11bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_RFU], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 11);
	//�˵�״̬
	iptparam->OilBill[IPT_OFFSET_STATE] = 0x00;

	//�����:T-MAC	4bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], "\x00\x00\x00\x00", 4);
						
	//�����:����ǹ��PhysicsGunId
	iptparam->OilBill[IPT_OFFSET_PHYGUN]=iptparam->PhysicalNozzle;

	//�����:����ͣ��ԭ��
	iptparam->OilBill[IPT_OFFSET_STOPNO]=0;

	//�����:��ǰ���
	*(tmp_buffer + 0) = (char)(pAuth->Amount>>24);	*(tmp_buffer + 1) = (char)(pAuth->Amount>>16);
	*(tmp_buffer + 2) = (char)(pAuth->Amount>>8);	*(tmp_buffer + 3) = (char)(pAuth->Amount>>0);
	memcpy(&iptparam->OilBill[IPT_OFFSET_BEFOR_BAL], tmp_buffer, 4);

	//�˵�״̬:0=������1=δ���
	iptparam->OilBill[IPT_OFFSET_ZD_STATE]=1;

	//�����豸ID
	iptparam->OilBill[IPT_OFFSET_JLNOZZLE]=iptparam->JlNozzle;

	//�����˵�У��ֵ
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
		
	//��¼���͹���
	memset(dsp_buffer, 0, sizeof(dsp_buffer));
	sprintf(dsp_buffer+strlen(dsp_buffer), "[�����=%d]", pcdMboardIDRead());
	sprintf(dsp_buffer+strlen(dsp_buffer), "[����=%d]", iptparam->Id);
	sprintf(dsp_buffer+strlen(dsp_buffer), "IC����������:Ԥ�ý��=%d,", iptparam->PresetMoney);
	sprintf(dsp_buffer+strlen(dsp_buffer), "Ԥ������=%d,", iptparam->PresetVolume);
	sprintf(dsp_buffer+strlen(dsp_buffer), "Ԥ�÷�ʽ=%d.", iptparam->PresetMode);
	jljUserLog("%s\n", dsp_buffer);

	//��ʾ�����н���
	memset(dsp_buffer, 0, 4);	dsp_len=4;
	dsp(iptparam->DEVDsp, DSP_UNSELF_OILLING, dsp_buffer, dsp_len);

	//ת�������
	iptPidSet(iptparam, IPT_PID_AUTH_OILLING);

	return;
}


/********************************************************************
*Name				:authOilling
*Description		:��Ȩ���ͳ�
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authOilling(int nozzle, char *inbuffer, int nbytes)
{
	int istate = 0;
	unsigned int money = 0, volume = 0, price = 0, jl_stop_no = 0;

	char dsp_buffer[64] = {0};
	int dsp_len = 0;
	int i = 0;

	int voice[16] = {0};

	PCOilInfoType info;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//��ȡʵʱ������
	istate=jlOilRead(iptparam->JlNozzle, &money, &volume, &price, (char*)&jl_stop_no);
	printf("auto:istate=%d,volume=%d,iptparam->OilVolume=%d\n",istate,volume,iptparam->OilVolume);

	if(iptparam->OilDspTimer >= ONE_SECOND)
	{
		//��ȡʵʱ�������ݣ��������ݸı�ʱ���д洢
		if(0==istate && volume!=iptparam->OilVolume)
		{
			iptparam->OilVolume=volume;		iptparam->OilMoney=money;		iptparam->OilPrice=price;
			
			//����
			iptparam->OilBill[IPT_OFFSET_VOL+0]=(char)(iptparam->OilVolume>>16);	
			iptparam->OilBill[IPT_OFFSET_VOL+1]=(char)(iptparam->OilVolume>>8);	
			iptparam->OilBill[IPT_OFFSET_VOL+2]=(char)(iptparam->OilVolume>>0);
			//���
			iptparam->OilBill[IPT_OFFSET_AMN+0]=(char)(iptparam->OilMoney>>16);	
			iptparam->OilBill[IPT_OFFSET_AMN+1]=(char)(iptparam->OilMoney>>8);	
			iptparam->OilBill[IPT_OFFSET_AMN+2]=(char)(iptparam->OilMoney>>0);
			//����
			iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);		
			iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);
			
			//�����˵�У��ֵ
			iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);

			//�����˵�������
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);
		}

		//����
		*(dsp_buffer + 0) = (char)(iptparam->OilMoney>>24);	*(dsp_buffer + 1) = (char)(iptparam->OilMoney>>16);
		*(dsp_buffer + 2) = (char)(iptparam->OilMoney>>8);		*(dsp_buffer + 3) = (char)(iptparam->OilMoney>>0);
		dsp_len=4;
		dsp(iptparam->DEVDsp, DSP_UNSELF_OILLING, dsp_buffer, dsp_len);

		//֪ͨƽ����ԣ���Ȩ������״̬
		iptparam->TaState = IPT_STATE_AUTH_OILLING;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		
		iptparam->OilDspTimer=0;
	}

	//������Ĭ�Ͻ�����
	if(KB_BUTTON_CZ==iptparam->Button || KB_BUTTON_R4==iptparam->Button)
	{
		if(0==jlOilCZ(iptparam->Id, 0))	iptparam->OilRound=1;
	}

	//���ʹﵽԤ����δ��ǹʱ����������ʾ
	if(0==iptparam->VoiceFlag && 2==jl_stop_no && IPT_GUN_PUTUP==iptparam->GunState && iptparam->VoiceVolume>0)
	{
		if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_OILEND;
		else																				voice[0]=SPKM_OILEND;
		iptSpk(iptparam, voice, 1);

		iptparam->VoiceFlag=1;
	}

	//��ǹ������״̬�쳣(�ǴﵽԤ����)���ͻ����磬�������������״̬
	if((IPT_GUN_PUTDOWN == iptparam->GunState)	||\
		(0==istate && 0!=jl_stop_no && 2!=jl_stop_no)	||\
		POWER_STATE_OK!=powerStateRead()	||\
		iptparam->OilMoney>pAuth->Amount)
	{
		if(IPT_GUN_PUTDOWN==iptparam->GunState)		jljRunLog("����=%d:��Ȩ���ͽ���!��ǹ!\n", iptparam->Id);
	
		if(0==istate && 0!=jl_stop_no && 2!=jl_stop_no)
		{
			i=((char)(jl_stop_no>>4)&0x0f)*10+((char)(jl_stop_no>>0)&0x0f)*1;
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, (unsigned char *)jlStopReason[i], strlen(jlStopReason[i]));
			IPT_DSP_WAIT();
			jljOilErrLogWrite(iptparam->Id, "��Ȩ���ͽ���!����ֹͣ����[%s]!", jlStopReason[i]);
		}
		
		if(POWER_STATE_OK!=powerStateRead())			jljOilErrLogWrite(iptparam->Id, "��Ȩ���ͽ���!�ͻ�����!");
		if(iptparam->OilMoney>pAuth->Amount)					jljOilErrLogWrite(iptparam->Id, "��Ȩ���ͽ���!����![���=%d]!", iptparam->OilMoney);

		//֪ͨƽ����ԣ���Ȩ������״̬
		iptparam->TaState = IPT_STATE_AUTH_OILFINISH;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "���ͽ��������Ժ�", strlen("���ͽ��������Ժ�"));
		iptPidSet(iptparam, IPT_PID_AUTH_OIL_FINISH);
		return;
	}

	return;
}


/********************************************************************
*Name				:authOilFinish
*Description		:��Ȩ���ͽ�������
*Input				:nozzle		���� 0=A1�棻1=B1�棻
*						:inbuffer	����
*						:nbytes		���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authOilFinish(int nozzle, char *inbuffer, int nbytes)
{
	int istate = 0;

	unsigned long long money_sum = 0, volume_sum = 0;
	unsigned int money = 0, volume = 0, price = 0, stop_no = 0;

	int i = 0;
	unsigned int jlvolume = 0, jlmoney = 0;
	int voice[16] = {0}, voice_len = 0;
	int tmp_voice[16] = {0}, tmp_voice_len = 0;

	PCOilInfoType info;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	istate=jlOilFinish(iptparam->JlNozzle, &money_sum, &volume_sum, &money, &volume, &price, (char*)&stop_no);

	iptAbnormalStopHandle(iptparam,stop_no,money);

	if(0==istate)
	{
		//����״̬����
		iptparam->OilState=IPT_OIL_IDLE;

		//�����������
		iptparam->SumMoney=money_sum;	iptparam->SumVolume=volume_sum;
		iptparam->OilMoney=money;	iptparam->OilVolume=volume;	iptparam->OilPrice=price;	iptparam->JlStopNO=stop_no;

		//���濨�˵�����
		//����
		iptparam->OilBill[IPT_OFFSET_VOL+0]=(char)(iptparam->OilVolume>>16);	
		iptparam->OilBill[IPT_OFFSET_VOL+1]=(char)(iptparam->OilVolume>>8);	
		iptparam->OilBill[IPT_OFFSET_VOL+2]=(char)(iptparam->OilVolume>>0);
		//���
		iptparam->OilBill[IPT_OFFSET_AMN+0]=(char)(iptparam->OilMoney>>16);	
		iptparam->OilBill[IPT_OFFSET_AMN+1]=(char)(iptparam->OilMoney>>8);	
		iptparam->OilBill[IPT_OFFSET_AMN+2]=(char)(iptparam->OilMoney>>0);
		//����
		iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);		
		iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);
		//����
		iptparam->OilBill[IPT_OFFSET_V_TOT+0]=(char)(iptparam->SumVolume>>24);
		iptparam->OilBill[IPT_OFFSET_V_TOT+1]=(char)(iptparam->SumVolume>>16);
		iptparam->OilBill[IPT_OFFSET_V_TOT+2]=(char)(iptparam->SumVolume>>8);
		iptparam->OilBill[IPT_OFFSET_V_TOT+3]=(char)(iptparam->SumVolume>>0);
		//����ͣ������
		iptparam->OilBill[IPT_OFFSET_STOPNO]=(char)(iptparam->JlStopNO);
		//�����˵�У��ֵ
		iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
		//�����˵�������
		framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
		framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

		//������ʾ"���˴εļ�������XXXX.XX��,XXXX.XXԪ��ף��һ·ƽ������ӭ�´ι���"
		if(iptparam->VoiceVolume>0)
		{
			jlvolume = iptparam->OilVolume;	jlmoney = iptparam->OilMoney;
			//���˴εļ�������
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_THISVOLUME;	
			else																				voice[voice_len++]=SPKM_THISVOLUME;
			//XXXX.XX
			tmp_voice_len=iptHexVoiceIdGet(iptparam, jlvolume, tmp_voice);	
			for(i=0; i<tmp_voice_len; i++)	voice[voice_len++]=tmp_voice[i];
			//��
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_SHENG;	
			else																				voice[voice_len++]=SPKM_SHENG;
			//XXXX.XX
			tmp_voice_len=iptHexVoiceIdGet(iptparam, jlmoney, tmp_voice);			
			for(i=0; i<tmp_voice_len; i++)	voice[voice_len++]=tmp_voice[i];
			//Ԫ
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_YUAN;	
			else																				voice[voice_len++]=SPKM_YUAN;
			//ף��һ·ƽ������ӭ�´ι���
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_SEEYOU;		
			else																				voice[voice_len++]=SPKM_SEEYOU;

			iptSpk(iptparam, voice, voice_len);
		}

		//ETC��Ȩת������ۿ����
		//��ETC��Ȩת���˵��洢����
		
		if(AUTH_MODEL_ETC == pAuth->Model)
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�ۿ����������Ժ�", strlen("�ۿ����������Ժ�"));
			iptPidSet(iptparam, IPT_PID_AUTH_DEBIT_APPLY);
		}
		else
		{
			iptPidSet(iptparam, IPT_PID_AUTH_BILL_SAVE);
		}
	}

	return;
}


/********************************************************************
*Name				:authDebitApply
*Description		:��Ȩ���ͽ�������ۿ����
*Input				:nozzle			���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes			���ݳ���
*Output			:��
*Return			:��
*History			:2016-04-12,modified by syj
*/
void authDebitApply(int nozzle, char *inbuffer, int nbytes)
{
	int i = 0;
	int istate = 0;

	char tx_buffer[64] = {0}, rx_buffer[64] = {0};
	int tx_len = 0, rx_len = 0;

	unsigned int ibalance = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//����ۿ�
	for(i = 0; i < 3; i++)
	{
		*(tx_buffer + 0) = 0x71;													//������Handle
		*(tx_buffer + 1) = iptparam->LogicNozzle;						//ǹ�� Nzn 
		*(tx_buffer + 2) = pAuth->AntID;										//����ID AntID
		memcpy(tx_buffer + 3, pAuth->OBUID, 4);						//��ǩMAC�� OBUID
		memcpy(tx_buffer + 7, pAuth->ContractNo, 8);				//OBU��ͬ���к�ContractNo
		memcpy(tx_buffer + 15, pAuth->OBUPlate, 12);				//���ƺ� OBUPlate
		*(tx_buffer + 27) = (char)(iptparam->OilMoney>>24);		//�ۿ�� Fee
		*(tx_buffer + 28) = (char)(iptparam->OilMoney>>16);	
		*(tx_buffer + 29) = (char)(iptparam->OilMoney>>8);	
		*(tx_buffer + 30) = (char)(iptparam->OilMoney>>0);
		memcpy(tx_buffer + 31, iptparam->OilTime, 7);				//��ǰʱ�� Time
		tx_len = 38;
		istate = pcdApplyForAuthETCDebit(tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
		if(0 == istate)	break;

		//taskDelay(1);
		usleep(1000);
	}
	
	//�ɹ�����ʼ�����˵�
	//ʧ�ܣ���ʾ������Ϣ
	if(0 == istate && 0x71 == *(rx_buffer + 0) && 0 == *(rx_buffer + 2))	iptparam->OilBill[IPT_OFFSET_STATE] = 0x00;
	else																										iptparam->OilBill[IPT_OFFSET_STATE] = 0x01;
	//�����˵�У��ֵ
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
	//�����˵�������
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

	//��ʾ֧�������Ϣ
	ibalance = pAuth->Amount - iptparam->OilMoney;
	if(0 == istate && 0x71 == *(rx_buffer + 0) && 0 == *(rx_buffer + 2))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "֧���ɹ�", strlen("֧���ɹ�"));
		*(tx_buffer + 0) = pAuth->Model;	
		*(tx_buffer + 1) = 0;
		*(tx_buffer + 2) = (char)(iptparam->OilMoney>>16);
		*(tx_buffer + 3) = (char)(iptparam->OilMoney>>8);
		*(tx_buffer + 4) = (char)(iptparam->OilMoney>>0);
		*(tx_buffer + 5) = (char)(iptparam->OilVolume>>16);
		*(tx_buffer + 6) = (char)(iptparam->OilVolume>>8);
		*(tx_buffer + 7) = (char)(iptparam->OilVolume>>0);
		*(tx_buffer + 8) = (char)(ibalance>>24);
		*(tx_buffer + 9) = (char)(ibalance>>16);
		*(tx_buffer + 10) = (char)(ibalance>>8);
		*(tx_buffer + 11) = (char)(ibalance>>0);
		tx_len = 12;
		pcDebitResultUpload(iptparam->TabletPanel, iptparam->LogicNozzle, tx_buffer, tx_len);
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "֧��ʧ��", strlen("֧��ʧ��"));
		*(tx_buffer + 0) = pAuth->Model;	
		*(tx_buffer + 1) = 1;
		*(tx_buffer + 2) = (char)(iptparam->OilMoney>>16);
		*(tx_buffer + 3) = (char)(iptparam->OilMoney>>8);
		*(tx_buffer + 4) = (char)(iptparam->OilMoney>>0);
		*(tx_buffer + 5) = (char)(iptparam->OilVolume>>16);
		*(tx_buffer + 6) = (char)(iptparam->OilVolume>>8);
		*(tx_buffer + 7) = (char)(iptparam->OilVolume>>0);
		*(tx_buffer + 8) = (char)(pAuth->Amount>>24);
		*(tx_buffer + 9) = (char)(pAuth->Amount>>16);
		*(tx_buffer + 10) = (char)(pAuth->Amount>>8);
		*(tx_buffer + 11) = (char)(pAuth->Amount>>0);
		tx_len = 12;
		pcDebitResultUpload(iptparam->TabletPanel, iptparam->LogicNozzle, tx_buffer, tx_len);
	}

	IPT_DSP_WAIT();

	//ת���˵��������
	iptPidSet(iptparam, IPT_PID_AUTH_BILL_SAVE);

	return;
}


/********************************************************************
*Name				:authBillSave
*Description		:��Ȩ���ͽ����˵��洢����
*Input				:nozzle			���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes			���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authBillSave(int nozzle, char *inbuffer, int nbytes)
{
	int i = 0;
	int istate = 0;
	unsigned int pauth_balance = 0;
	unsigned int tmp_ttc = 0;
	char tmac_buffer[32] = {0}, apdu_buffer[32] = {0};

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//����TTC
	for(i = 0; i < 3; i++)
	{
		istate = pcdApplyForTTC(iptparam->PhysicalNozzle, iptparam->OilBill, IPT_BILL_SIZE, &tmp_ttc);
		if(0 == istate)	break;
		
		//taskDelay(100*sysClkRateGet()/1000);
		usleep(100000);
	}
	if(0 == istate)
	{
		iptparam->OilBill[IPT_OFFSET_TTC + 0] = (char)(tmp_ttc>>24);	iptparam->OilBill[IPT_OFFSET_TTC + 1] = (char)(tmp_ttc>>16);
		iptparam->OilBill[IPT_OFFSET_TTC + 2] = (char)(tmp_ttc>>8);	iptparam->OilBill[IPT_OFFSET_TTC + 3] = (char)(tmp_ttc>>0);
	}

	//����TMAC������ʧ��ʱ��Ĭ��TMAC���
	for(i=0; i<3; i++)
	{
		istate = iptTMACCalculate(iptparam, tmac_buffer, apdu_buffer, iptparam->OilBill, 95);
		if(0==istate)		break;

		//taskDelay(100*sysClkRateGet()/1000);
		usleep(100000);
	}
	if(0==istate)memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], tmac_buffer, 4);
	else				memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], IPT_TMAC_DEFUALT, 4);

	//�����˵�У��ֵ
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
	//���浽������
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
	//���汸��
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

	//�����˵�
	for(i = 0; i < 3; i++)
	{
		istate = pcdApplyForBillSave(iptparam->PhysicalNozzle, iptparam->OilBill, IPT_BILL_SIZE);
		if(0 == istate)	break;

		//taskDelay(100*sysClkRateGet()/1000);
		usleep(100000);
	}

	//��ETC��Ȩ(��΢�Ż�֧����)��Ҫ�ϴ����ͽ��
	if(AUTH_MODEL_ETC != pAuth->Model)
	{
		//������Ȩ���
		if(pAuth->Amount > iptparam->OilMoney)	pauth_balance = pAuth->Amount - iptparam->OilMoney;
		else																	pauth_balance = 0;
	
		//֪ͨƽ����ԣ���Ȩ������״̬
		//��Ȩ�4HEX��
		//��Ȩ��ʽ��1HEX��
		//��Ȩ��4HEX��
		//���ͽ�4HEX��
		//����������4HEX��
		//������ţ�4HEX��
		//ǹ�ţ�1HEX��
		//���ã�20bytes��
		
		iptparam->TaState = IPT_STATE_AUTH_OIL_DATA;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pauth_balance>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pauth_balance>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pauth_balance>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pauth_balance>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilMoney>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilVolume>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilVolume>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilVolume>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(iptparam->OilVolume>>0);
		memcpy(iptparam->TaStateParam + iptparam->TaStateParamLength, &iptparam->OilBill[IPT_OFFSET_TTC], 4);
		iptparam->TaStateParamLength += 4;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = iptparam->LogicNozzle;
		memset(iptparam->TaStateParam + iptparam->TaStateParamLength, 0, 20);
		iptparam->TaStateParamLength += 20;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		
	}

	//�˳���Ȩ����
	authProccessExit(iptparam->Id, inbuffer, nbytes);

	return;
}


/********************************************************************
*Name				:authETCPasswordInput
*Description		:��Ȩ����ETC�������������
*Input				:nozzle			���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes			���ݳ���
*Output			:��
*Return			:��
*History			:2016-04-12,modified by syj
*/
void authETCPasswordInput(int nozzle, char *inbuffer, int nbytes)
{
	char inbffer[32] = {0};
	char dsp_buffer[64] = {0};
	int dsp_len = 0;
	int button = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//���յ�ȡ����������˳�����
	if(2 == iptparam->PassInputAsk)
	{
		iptparam->PassInputAsk = 0;
		authProccessExit(nozzle, inbuffer, nbytes);
		return;
	}

	//����������ϴ�
	switch(iptparam->Button)
	{
	case KB_BUTTON_0:
		if(0==button)	button='0';
	case KB_BUTTON_1:
		if(0==button)	button='1';
	case KB_BUTTON_2:
		if(0==button)	button='2';
	case KB_BUTTON_3:
		if(0==button)	button='3';
	case KB_BUTTON_4:
		if(0==button)	button='4';
	case KB_BUTTON_5:
		if(0==button)	button='5';
	case KB_BUTTON_6:
		if(0==button)	button='6';
	case KB_BUTTON_7:
		if(0==button)	button='7';
	case KB_BUTTON_8:
		if(0==button)	button='8';
	case KB_BUTTON_9:
		if(0==button)	button='9';

		//�����ּ��������룬��������������ϴ���ƽ��
		if(iptparam->SetButtonLen>=6)	break;

		iptparam->SetButton[iptparam->SetButtonLen++] = button;
		
		dsp_buffer[0] = iptparam->SetButtonLen;	dsp_len = 1;
		dsp(iptparam->DEVDsp, DSP_PASSWORD_INPUT, dsp_buffer, dsp_len);

		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 0;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);
		break;

		
	case KB_BUTTON_CHG:
		//�������
		memset(iptparam->SetButton, 0, sizeof(iptparam->SetButton));	iptparam->SetButtonLen = 0;

		dsp_buffer[0] = iptparam->SetButtonLen;	dsp_len = 1;
		dsp(iptparam->DEVDsp, DSP_PASSWORD_INPUT, dsp_buffer, dsp_len);

		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 0;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);
		break;

		
	case KB_BUTTON_ACK:
		//�������������ϴ���ƽ�岢�˳�����
		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 1;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);

		authProccessExit(nozzle, inbuffer, nbytes);
		break;

		
	case KB_BUTTON_BACK:
		//�˳�����
		memset(iptparam->SetButton, 0, sizeof(iptparam->SetButton));	iptparam->SetButtonLen = 0;

		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 0;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);
		authProccessExit(nozzle, inbuffer, nbytes);
		break;
	default:
		break;
	}

	return;
}


/********************************************************************
*Name				:authOilStartError
*Description		:��Ȩ������������ʧ��ʱ�Ĵ���
*Input				:nozzle			���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes		���ݳ���
*Output			:��
*Return			:��
*History			:2016-04-12,modified by syj
*/
void authOilStartError(int nozzle, char *inbuffer, int nbytes)
{
	char inbffer[32] = {0};
	char dsp_buffer[64] = {0};
	int dsp_len = 0;
	int button = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//�����ETC��Ȩ������ʧ�ܺ�ֱ�ӷ�����Ȩ������漴��
	//�����΢�Ż�֧������ȡ����Ȩ�󷵻���Ȩ�������
	
	if(AUTH_MODEL_ETC == pAuth->Model)	iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
	else																
	{
		//֪ͨƽ�������Ȩȡ��
		iptparam->TaState = IPT_STATE_AUTH_CANCEL;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>24);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>16);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>8);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>0);
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		memcpy(iptparam->TaStateParam + iptparam->TaStateParamLength, 0, 16);
		iptparam->TaStateParamLength += 16;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		
		authProccessExit(iptparam->Id, inbuffer, nbytes);
	}

	return;
}


/********************************************************************
*Name				:authProccess
*Description		:��Ȩ���Ͳ�������
*Input				:nozzle			���� 0=A1�棻1=B1�棻
*						:inbuffer		����
*						:nbytes			���ݳ���
*Output			:��
*Return				:��
*History			:2016-04-12,modified by syj
*/
void authProccess(int nozzle, char *inbuffer, int nbytes)
{
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//�жϲ�����֧���ն����ݽṹ
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}


	switch(iptparam->ProcessId)
	{
	case IPT_PID_AUTH_PRETREAT:			//��ȨԤ����
		authBalancePretreat(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_BALANCE:			//��Ȩ����
		authBalance(nozzle, inbuffer, nbytes);
		break;
		
	case	IPT_PID_AUTH_OIL_CHECK:	//��Ȩ��������ǰ�����Լ����
		authOilCheck(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OIL_START:		//��Ȩ������������
		authOilStart(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OILLING:			//��Ȩ������
		authOilling(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OIL_FINISH:		//��Ȩ���ͽ�������
		authOilFinish(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_DEBIT_APPLY:	//(ETC)��Ȩ��������ۿ����
		authDebitApply(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_BILL_SAVE:		//��Ȩ�����˵��洢����
		authBillSave(nozzle, inbuffer, nbytes);
		break;

	case IPT_PID_AUTH_START_ERR:		//��Ȩ��������ʧ��
		authOilStartError(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_PASS_INPUT:		//(ETC)��Ȩ����ETC�������������
		authETCPasswordInput(nozzle, inbuffer, nbytes);
		break;
		
	default:
		authProccessExit(nozzle, inbuffer, nbytes);
		break;
	}

	return;
}









