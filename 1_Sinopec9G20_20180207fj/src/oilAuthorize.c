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

//授权数据
AuthorizeDataType authorizeDataA1, authorizeDataB1;


//私有接口声明
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
*Description		:其它操作显示结果后转入本过程等待操作，
*						:按任意键或超时无操作则退出到
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	授权数据(授权额4HEX + 授权方式1Bin + 天线ID（1byte）+天线MAC号（4bytes）+ OBU合同序列号(8bytes)+车牌号（12bytes）+卡号（10bytes）)
*Output			:无
*Return				:成功返回0；失败返回其它值；
*History			:2016-04-12,modified by syj
*/
int authorizeWrite(int nozzle, const char *inbuffer)
{
	int i = 0;
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return ERROR;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//保存授权值
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
*Description		:退出授权加油处理
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return				:成功返回0；失败返回其它值；
*History			:2016-04-12,modified by syj
*/
int authProccessExit(int nozzle, char *inbuffer, int nbytes)
{
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return ERROR;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	iptMainInterface(iptparam);

	return 0;
}


/********************************************************************
*Name				:authBalancePretreat
*Description		:转入授权余额前的预处理
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return				:无
*History			:2016-04-12,modified by syj
*/
void authBalancePretreat(int nozzle, char *inbuffer, int nbytes)
{
	char dsp_buffer[64] = {0};
	int dsp_len = 0;

	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//清空预置数据
	memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));	iptparam->IntegerLen=0;
	iptparam->Point=0;
	memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
	iptparam->PresetMode=IPT_PRESET_NO;

	//授权状态
	iptparam->TaState = IPT_STATE_AUTH_BALANCE;	iptparam->TaStateParamLength = 0;
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>24);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>16);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>8);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = (char)(pAuth->Amount>>0);
	*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
	memcpy(iptparam->TaStateParam + iptparam->TaStateParamLength, 0, 16);
	iptparam->TaStateParamLength += 16;
	pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);

	//显示授权余额界面
	dsp_buffer[0] = (char)(pAuth->Amount>>24);	dsp_buffer[1] = (char)(pAuth->Amount>>16);
	dsp_buffer[2] = (char)(pAuth->Amount>>8);	dsp_buffer[3] = (char)(pAuth->Amount>>0);
	dsp_buffer[4] = pAuth->Unit;
	dsp(iptparam->DEVDsp, DSP_AUTH_BALANCE, dsp_buffer, dsp_len);
	iptPidSet(iptparam, IPT_PID_AUTH_BALANCE);

	return;
}


/********************************************************************
*Name				:authBalance
*Description		:授权加油余额界面
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes			数据长度
*Output			:无
*Return				:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//提枪，开始加油
	if(IPT_GUN_PUTUP == iptparam->GunState && 0 != iptparam->GunStateChg)
	{
		//授权加油启动中状态
		iptparam->TaState = IPT_STATE_AUTH_OILSTART;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
	
		iptPidSet(iptparam, IPT_PID_AUTH_OIL_CHECK);
		return;
	}

	//非ETC授权(即微信或支付宝授权)，不允许再行预置
	if(AUTH_MODEL_ETC != pAuth->Model)
	{
		return;
	}

	//按键处理
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

		//记录小数点
		if('.'==button)
		{
			iptparam->Point='.';
		}
		//前一刻无小数点按键操作，记录按键值为整数
		else if(0==iptparam->Point)
		{
			iptparam->IntegerBuffer[0]=iptparam->IntegerBuffer[1];	iptparam->IntegerBuffer[1]=iptparam->IntegerBuffer[2];
			iptparam->IntegerBuffer[2]=iptparam->IntegerBuffer[3];	iptparam->IntegerBuffer[3]=iptparam->IntegerBuffer[4];	
			iptparam->IntegerBuffer[4]=iptparam->IntegerBuffer[5];	iptparam->IntegerBuffer[5]=button;
		}
		//前一刻有小数点按键操作，记录按键值为小数
		else if(0!=iptparam->Point && iptparam->DecimalLen<2)
		{
			iptparam->DecimalBuffer[iptparam->DecimalLen++]=button;
		}

		//非预置模式默认为预置金额
		if(IPT_PRESET_NO==iptparam->PresetMode)	iptparam->PresetMode=IPT_PRESET_MONEY;

		//计算预置值
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//显示预置界面
		dsp_buffer[0]=(char)(data>>24);	dsp_buffer[1]=(char)(data>>16);
		dsp_buffer[2]=(char)(data>>8);	dsp_buffer[3]=(char)(data>>0);
		if(IPT_PRESET_VOLUME==iptparam->PresetMode)	dsp_buffer[4]=1;
		else																			dsp_buffer[4]=0;
		dsp_len=5;
		dsp(iptparam->DEVDsp, DSP_CARD_PRESET, dsp_buffer, dsp_len);
		break;

	case KB_BUTTON_MON:
	case KB_BUTTON_VOL:
		//计算预置值
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//更改预置方式
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
		//计算预置值
		data=(iptparam->IntegerBuffer[0]&0x0f)*10000000+(iptparam->IntegerBuffer[1]&0x0f)*1000000+\
			(iptparam->IntegerBuffer[2]&0x0f)*100000+(iptparam->IntegerBuffer[3]&0x0f)*10000+\
			(iptparam->IntegerBuffer[4]&0x0f)*1000+(iptparam->IntegerBuffer[5]&0x0f)*100+\
			(iptparam->DecimalBuffer[0]&0x0f)*10+(iptparam->DecimalBuffer[1]&0x0f)*1;

		//如果预置量为0显示卡余额界面，否则清空预置量
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

		//清空预置数据
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
*Description		:授权加油启动前检查各项数据是否合法，完成某些预处理操作
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return				:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//判断电源状态
	if(POWER_STATE_OK!=powerStateRead())
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "油机电源状态异常", 16);
		IPT_DSP_WAIT();
		
		jljOilErrLogWrite(iptparam->Id, "加油启动失败，油机电源状态异常!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断账单是否溢出
	if(iptparam->FuelUnloadNumber>=IPT_BILLUNLOAD_MAX)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "账单溢出", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，账单溢出!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断获取单价结果
	if(0!=jlPriceRead(iptparam->JlNozzle, &iptparam->OilPrice))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "获取单价失败", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，获取计量单价失败!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	if(iptparam->JlErr_BianJia!=0)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "变价失败", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，变价失败!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断单价合法性
	if((iptparam->OilPrice<IPT_PRICE_MIN)||(iptparam->OilPrice>IPT_PRICE_MAX))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "单价非法", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，单价非法!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断销售锁定
	if(IPT_SELL_UNLOCK!=iptparam->SellLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "加油启动锁定", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，加油启动锁定!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断夜间锁定
	if(IPT_NIGHT_UNLOCK!=iptparam->NightLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "夜间锁定", 8);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，夜间锁定!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断钥匙状态
	if(KB_KEYLOCK_OIL!=iptparam->KeyLock)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "请把钥匙打到加油位置", strlen("请把钥匙打到加油位置"));
		IPT_DSP_WAIT();

		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断时间合法性
	tmp_buffer[0]=iptparam->Time.century;	tmp_buffer[1]=iptparam->Time.year;		tmp_buffer[2]=iptparam->Time.month;	tmp_buffer[3]=iptparam->Time.date;
	tmp_buffer[4]=iptparam->Time.hour;		tmp_buffer[5]=iptparam->Time.minute;	tmp_buffer[6]=iptparam->Time.second;
	if(0!=timeVerification(tmp_buffer, 7))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "油机时间非法", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，油机时间非法!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断物理枪号合法性
	if(iptparam->PhysicalNozzle<1 || iptparam->PhysicalNozzle>6)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "物理枪号非法", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，物理枪号非法!物理枪号=%s.", iptparam->PhysicalNozzle);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断PCD连接状态
	if(1!=iptparam->PcdState)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "与PCD连接断开", 13);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，与PCD连接断开!");
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断PCD异常状态
	if(0!=iptparam->PcdErrNO)
	{
		memcpy(&dsp_buffer[0], "PCD状态异常", 11);
		dsp_buffer[11]=(iptparam->PcdErrNO>>4)&0x0f+0x30;
		dsp_buffer[12]=(iptparam->PcdErrNO>>0)&0x0f+0x30;
		dsp_len=13;
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, dsp_buffer, dsp_len);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，PCD状态异常!错误代码=%x.", iptparam->PcdErrNO);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断通用信息版本合法性
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==IptPcInfo.SInfo.Version)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "通用信息版本非法", 16);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，通用信息版本非法!版本=%d.", IptPcInfo.SInfo.Version);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断油品油价版本
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==IptPcInfo.OilInfo.Version)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "油品油价版本非法", 16);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，油品油价版本非法!版本=%d.", IptPcInfo.OilInfo.Version);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断逻辑枪号合法性
	if(IPT_MODE_UNSELF!=iptparam->Mode && 0==iptparam->LogicNozzle)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "逻辑枪号非法", 12);
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "加油启动失败，逻辑枪号非法!逻辑枪号=%d.", iptparam->LogicNozzle);
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//计算最大允许金额和升数
	//取授权额和默认最大允许金额较小值
	
	if(pAuth->Amount <= JL_MONEY_MAX)	money_max = pAuth->Amount;	
	else																money_max = JL_MONEY_MAX;
	volume_max = JL_VOLUME_MAX;

	//计算预置量
	//预置量为零或非预置即默认为任意加油；
	//定升数加油，通过升数计算预置金额；
	//定金额加油，通过金额计算预置升数；
	//任意加油，以最大允许金额为预置金额，并计算预置升数；
	//需注意，在以下判断限制信息及卡余额时可能会重新预置；
	
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

	//判断预置量是否过小，不小于1升
	if(iptparam->PresetMoney<iptparam->OilPrice || iptparam->PresetMoney<IPT_MONEY_MIN || iptparam->PresetVolume<IPT_VOLUME_MIN)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "预置量太小", 10);
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断预置量是否过大
	if(iptparam->PresetMoney>JL_MONEY_MAX || iptparam->PresetVolume>JL_VOLUME_MAX)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "预置量太大", 10);
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//判断余额
	if(iptparam->PresetMoney > pAuth->Amount)
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "余额不足", strlen("余额不足"));
		IPT_DSP_WAIT();
		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//记录加油时间，初始化本次加油数据
	iptparam->OilTime[0]=iptparam->Time.century;	iptparam->OilTime[1]=iptparam->Time.year;
	iptparam->OilTime[2]=iptparam->Time.month;		iptparam->OilTime[3]=iptparam->Time.date;
	iptparam->OilTime[4]=iptparam->Time.hour;		iptparam->OilTime[5]=iptparam->Time.minute;
	iptparam->OilTime[6]=iptparam->Time.second;
	iptparam->OilMoney=0;	iptparam->OilVolume=0;		iptparam->OilPrice=iptparam->OilPrice;
	jlSumRead(iptparam->JlNozzle, &(iptparam->SumVolume), &(iptparam->SumMoney));
	//GetjlSumRead(iptparam->JlNozzle,&(iptparam->SumVolume),&(iptparam->SumMoney));
	iptparam->OilRound=0;

	//启动加油
	dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "加油启动中，           请稍候...", strlen("加油启动中，           请稍候..."));
	iptPidSet(iptparam, IPT_PID_AUTH_OIL_START);

	return;
}


/********************************************************************
*Name				:authOilStart
*Description		:授权加油启动过程
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return			:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//申请计量加油启动
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

	//申请计量加油启动失败，提示失败信息，记录错误日志，返回授权余额界面
	if(0 != istate)
	{
		memset(dsp_buffer, 0, sizeof(dsp_buffer));
		memcpy(dsp_buffer, "计量启动失败    ", 16);
		i=((char)(istate>>4)&0x0f)*10+((char)(istate>>0)&0x0f)*1;
		memcpy(dsp_buffer+strlen(dsp_buffer), jlStartFiledReson[i], strlen(jlStartFiledReson[i]));
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, dsp_buffer, strlen(dsp_buffer));
		IPT_DSP_WAIT();

		jljOilErrLogWrite(iptparam->Id, "%s", dsp_buffer);

		//iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
		iptPidSet(iptparam, IPT_PID_AUTH_START_ERR);
		return;
	}

	//语音提示"此油枪加注XX，泵码已回零，请确认"
	if(iptparam->VoiceVolume>0 && IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)
	{
		voice[0]=SPKW_OILFILL;															//此油枪加注
																										//油品语音代码
		if(0==memcmp(iptparam->OilVoice, "\x00\x00\x00\x00", 4))
			voice[1]=iptOilVoiceIdGet(iptparam, iptparam->OilCode);
		else
			voice[1]=((iptparam->OilVoice[0]&0x0f)<<12)|((iptparam->OilVoice[1]&0x0f)<<8)|\
							((iptparam->OilVoice[2]&0x0f)<<4)|((iptparam->OilVoice[3]&0x0f)<<0);
		voice[2]=SPKW_OILACK;															//泵码已回零，请确认
		iptSpk(iptparam, voice, 3);
	}
	else if(iptparam->VoiceVolume>0 && IPT_VOICE_TYPE_MAN==iptparam->VoiceType)
	{
		voice[0]=SPKM_OILFILL;															//此油枪加注
																										    //油品语音代码
		if(0==memcmp(iptparam->OilVoice, "\x00\x00\x00\x00", 4))
			voice[1]=iptOilVoiceIdGet(iptparam, iptparam->OilCode);
		else
			voice[1]=((iptparam->OilVoice[0]&0x0f)<<12)|((iptparam->OilVoice[1]&0x0f)<<8)|\
							((iptparam->OilVoice[2]&0x0f)<<4)|((iptparam->OilVoice[3]&0x0f)<<0);
		voice[2]=SPKM_OILACK;															 //泵码已回零，请确认
		iptSpk(iptparam, voice, 3);
	}

	//参数根据状态初始化
	iptparam->VoiceFlag=0;
	iptparam->OilState=IPT_OIL_FUELLING;

	//产生扣款来源为ETC卡的非卡账单
	//待填充:POS_TTC
	memcpy(&iptparam->OilBill[IPT_OFFSET_TTC], "\x00\x00\x00\x00", 4);

	//待填充:交易类型设为无卡错，后台/本地黑白名单，扣款签名无效，逃卡
	iptparam->OilBill[IPT_OFFSET_T_TYPE]=(0<<7)|(0<<6)|(0<<4)|(7<<0);

	//已完成:交易日期及时间
	iptparam->OilTime[0]=iptparam->Time.century;	iptparam->OilTime[1]=iptparam->Time.year;
	iptparam->OilTime[2]=iptparam->Time.month;		iptparam->OilTime[3]=iptparam->Time.date;
	iptparam->OilTime[4]=iptparam->Time.hour;		iptparam->OilTime[5]=iptparam->Time.minute;
	iptparam->OilTime[6]=iptparam->Time.second;
	memcpy(&iptparam->OilBill[IPT_OFFSET_TIME], iptparam->OilTime, 7);

	//已完成:卡应用号
	memcpy(&iptparam->OilBill[IPT_OFFSET_ASN], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

	//待填充:余额(扣前)，交易完成后填扣后余额
	*(tmp_buffer + 0) = (char)(pAuth->Amount>>24);	*(tmp_buffer + 1) = (char)(pAuth->Amount>>16);
	*(tmp_buffer + 2) = (char)(pAuth->Amount>>8);	*(tmp_buffer + 3) = (char)(pAuth->Amount>>0);
	memcpy(&iptparam->OilBill[IPT_OFFSET_BALANCE], tmp_buffer, 4);

	//待填充:数额，交易完成后填实际数额
	memcpy(&iptparam->OilBill[IPT_OFFSET_AMN], "\x00\x00\x00", 3);

	//已完成:卡交易序号
	memcpy(&iptparam->OilBill[IPT_OFFSET_CTC], "\x00\x00", 2);

	//待填充:电子签名，加油/补扣/补充时未TAC，逃卡时为GTAC
	memcpy(&iptparam->OilBill[IPT_OFFSET_TAC], "\x00\x00\x00\x00", 4);

	//待填充:解灰认证码4bytes，加油完成，PSAM计算GMAC后添加
	memcpy(&iptparam->OilBill[IPT_OFFSET_GMAC], "\x00\x00\x00\x00", 4);

	//待填充:PSAM会所签名4bytes，加油完成，PSAM计算GMAC后添加
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_TAC], "\x00\x00\x00\x00", 4);

	//已完成:PSAM应用号10bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_ASN], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

	//已完成:PSAM编号6bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_TID], "\x00\x00\x00\x00\x00\x00", 6);
			
	//已完成:PSAM终端交易序号4bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_PSAM_TTC], "\x00\x00\x00\x00", 4);

	//已完成:扣款来源1byte
	if(AUTH_MODEL_WECHAT == pAuth->Model)		iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_WECHAT;
	else if(AUTH_MODEL_ALIPAY == pAuth->Model)	iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_ALIPAY;
	else																			iptparam->OilBill[IPT_OFFSET_DS] = AUTH_DS_ETC;

	//已完成:结算单位/方式1byte
	iptparam->OilBill[IPT_OFFSET_UNIT] = 0;
			
	//已完成:卡类1byte
	iptparam->OilBill[IPT_OFFSET_C_TYPE] = 0;

	//已完成:卡版本1byte，b7~b4卡密钥索引号；b3~b0卡密钥版本号
	iptparam->OilBill[IPT_OFFSET_VER] = 0;

	//已完成:枪号1byte
	iptparam->OilBill[IPT_OFFSET_NZN] = iptparam->LogicNozzle;

	//已完成:油品代码2bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_G_CODE], iptparam->OilCode, 2);

	//升数3bytes，交易完成后填实际升数
	memcpy(&iptparam->OilBill[IPT_OFFSET_VOL], "\x00\x00\x00", 3);

	//已完成:成交价格2bytes，根据累计金额及油量计算
	iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);
	iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);

	//已完成:员工号1byte
	iptparam->OilBill[IPT_OFFSET_EMP]=iptparam->EMP;

	//已完成:升累计4bytes
	jlSumRead(iptparam->JlNozzle, &volume_sum, &money_sum);
	//GetjlSumRead(iptparam->JlNozzle,&volume_sum,&money_sum);
	iptparam->OilBill[IPT_OFFSET_V_TOT+0]=(char)(volume_sum>>24);
	iptparam->OilBill[IPT_OFFSET_V_TOT+1]=(char)(volume_sum>>16);
	iptparam->OilBill[IPT_OFFSET_V_TOT+2]=(char)(volume_sum>>8);	
	iptparam->OilBill[IPT_OFFSET_V_TOT+3]=(char)(volume_sum>>0);

	//已完成:备用部分11bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_RFU], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 11);
	//账单状态
	iptparam->OilBill[IPT_OFFSET_STATE] = 0x00;

	//待填充:T-MAC	4bytes
	memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], "\x00\x00\x00\x00", 4);
						
	//已完成:物理枪号PhysicsGunId
	iptparam->OilBill[IPT_OFFSET_PHYGUN]=iptparam->PhysicalNozzle;

	//已完成:计量停机原因
	iptparam->OilBill[IPT_OFFSET_STOPNO]=0;

	//已完成:扣前余额
	*(tmp_buffer + 0) = (char)(pAuth->Amount>>24);	*(tmp_buffer + 1) = (char)(pAuth->Amount>>16);
	*(tmp_buffer + 2) = (char)(pAuth->Amount>>8);	*(tmp_buffer + 3) = (char)(pAuth->Amount>>0);
	memcpy(&iptparam->OilBill[IPT_OFFSET_BEFOR_BAL], tmp_buffer, 4);

	//账单状态:0=正常；1=未完成
	iptparam->OilBill[IPT_OFFSET_ZD_STATE]=1;

	//计量设备ID
	iptparam->OilBill[IPT_OFFSET_JLNOZZLE]=iptparam->JlNozzle;

	//计算账单校验值
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
		
	//记录加油过程
	memset(dsp_buffer, 0, sizeof(dsp_buffer));
	sprintf(dsp_buffer+strlen(dsp_buffer), "[主板号=%d]", pcdMboardIDRead());
	sprintf(dsp_buffer+strlen(dsp_buffer), "[面板号=%d]", iptparam->Id);
	sprintf(dsp_buffer+strlen(dsp_buffer), "IC卡加油启动:预置金额=%d,", iptparam->PresetMoney);
	sprintf(dsp_buffer+strlen(dsp_buffer), "预置升数=%d,", iptparam->PresetVolume);
	sprintf(dsp_buffer+strlen(dsp_buffer), "预置方式=%d.", iptparam->PresetMode);
	jljUserLog("%s\n", dsp_buffer);

	//显示加油中界面
	memset(dsp_buffer, 0, 4);	dsp_len=4;
	dsp(iptparam->DEVDsp, DSP_UNSELF_OILLING, dsp_buffer, dsp_len);

	//转入加油中
	iptPidSet(iptparam, IPT_PID_AUTH_OILLING);

	return;
}


/********************************************************************
*Name				:authOilling
*Description		:授权加油程
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return				:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//读取实时加油量
	istate=jlOilRead(iptparam->JlNozzle, &money, &volume, &price, (char*)&jl_stop_no);
	printf("auto:istate=%d,volume=%d,iptparam->OilVolume=%d\n",istate,volume,iptparam->OilVolume);

	if(iptparam->OilDspTimer >= ONE_SECOND)
	{
		//获取实时加油数据，加油数据改变时进行存储
		if(0==istate && volume!=iptparam->OilVolume)
		{
			iptparam->OilVolume=volume;		iptparam->OilMoney=money;		iptparam->OilPrice=price;
			
			//油量
			iptparam->OilBill[IPT_OFFSET_VOL+0]=(char)(iptparam->OilVolume>>16);	
			iptparam->OilBill[IPT_OFFSET_VOL+1]=(char)(iptparam->OilVolume>>8);	
			iptparam->OilBill[IPT_OFFSET_VOL+2]=(char)(iptparam->OilVolume>>0);
			//金额
			iptparam->OilBill[IPT_OFFSET_AMN+0]=(char)(iptparam->OilMoney>>16);	
			iptparam->OilBill[IPT_OFFSET_AMN+1]=(char)(iptparam->OilMoney>>8);	
			iptparam->OilBill[IPT_OFFSET_AMN+2]=(char)(iptparam->OilMoney>>0);
			//单价
			iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);		
			iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);
			
			//计算账单校验值
			iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);

			//保存账单及备份
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);
		}

		//送显
		*(dsp_buffer + 0) = (char)(iptparam->OilMoney>>24);	*(dsp_buffer + 1) = (char)(iptparam->OilMoney>>16);
		*(dsp_buffer + 2) = (char)(iptparam->OilMoney>>8);		*(dsp_buffer + 3) = (char)(iptparam->OilMoney>>0);
		dsp_len=4;
		dsp(iptparam->DEVDsp, DSP_UNSELF_OILLING, dsp_buffer, dsp_len);

		//通知平板电脑，授权加油中状态
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

	//凑整，默认金额凑整
	if(KB_BUTTON_CZ==iptparam->Button || KB_BUTTON_R4==iptparam->Button)
	{
		if(0==jlOilCZ(iptparam->Id, 0))	iptparam->OilRound=1;
	}

	//加油达到预置量未挂枪时播报语音提示
	if(0==iptparam->VoiceFlag && 2==jl_stop_no && IPT_GUN_PUTUP==iptparam->GunState && iptparam->VoiceVolume>0)
	{
		if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_OILEND;
		else																				voice[0]=SPKM_OILEND;
		iptSpk(iptparam, voice, 1);

		iptparam->VoiceFlag=1;
	}

	//挂枪，计量状态异常(非达到预置量)，油机掉电，过冲则结束加油状态
	if((IPT_GUN_PUTDOWN == iptparam->GunState)	||\
		(0==istate && 0!=jl_stop_no && 2!=jl_stop_no)	||\
		POWER_STATE_OK!=powerStateRead()	||\
		iptparam->OilMoney>pAuth->Amount)
	{
		if(IPT_GUN_PUTDOWN==iptparam->GunState)		jljRunLog("面板号=%d:授权加油结束!挂枪!\n", iptparam->Id);
	
		if(0==istate && 0!=jl_stop_no && 2!=jl_stop_no)
		{
			i=((char)(jl_stop_no>>4)&0x0f)*10+((char)(jl_stop_no>>0)&0x0f)*1;
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, (unsigned char *)jlStopReason[i], strlen(jlStopReason[i]));
			IPT_DSP_WAIT();
			jljOilErrLogWrite(iptparam->Id, "授权加油结束!计量停止加油[%s]!", jlStopReason[i]);
		}
		
		if(POWER_STATE_OK!=powerStateRead())			jljOilErrLogWrite(iptparam->Id, "授权加油结束!油机掉电!");
		if(iptparam->OilMoney>pAuth->Amount)					jljOilErrLogWrite(iptparam->Id, "授权加油结束!过冲![金额=%d]!", iptparam->OilMoney);

		//通知平板电脑，授权加油中状态
		iptparam->TaState = IPT_STATE_AUTH_OILFINISH;	iptparam->TaStateParamLength = 0;
		*(iptparam->TaStateParam + iptparam->TaStateParamLength++) = pAuth->Model;
		pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "加油结束中请稍候", strlen("加油结束中请稍候"));
		iptPidSet(iptparam, IPT_PID_AUTH_OIL_FINISH);
		return;
	}

	return;
}


/********************************************************************
*Name				:authOilFinish
*Description		:授权加油结束过程
*Input				:nozzle		面板号 0=A1面；1=B1面；
*						:inbuffer	数据
*						:nbytes		数据长度
*Output			:无
*Return				:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	istate=jlOilFinish(iptparam->JlNozzle, &money_sum, &volume_sum, &money, &volume, &price, (char*)&stop_no);

	iptAbnormalStopHandle(iptparam,stop_no,money);

	if(0==istate)
	{
		//加油状态结束
		iptparam->OilState=IPT_OIL_IDLE;

		//保存加油数据
		iptparam->SumMoney=money_sum;	iptparam->SumVolume=volume_sum;
		iptparam->OilMoney=money;	iptparam->OilVolume=volume;	iptparam->OilPrice=price;	iptparam->JlStopNO=stop_no;

		//保存卡账单数据
		//油量
		iptparam->OilBill[IPT_OFFSET_VOL+0]=(char)(iptparam->OilVolume>>16);	
		iptparam->OilBill[IPT_OFFSET_VOL+1]=(char)(iptparam->OilVolume>>8);	
		iptparam->OilBill[IPT_OFFSET_VOL+2]=(char)(iptparam->OilVolume>>0);
		//金额
		iptparam->OilBill[IPT_OFFSET_AMN+0]=(char)(iptparam->OilMoney>>16);	
		iptparam->OilBill[IPT_OFFSET_AMN+1]=(char)(iptparam->OilMoney>>8);	
		iptparam->OilBill[IPT_OFFSET_AMN+2]=(char)(iptparam->OilMoney>>0);
		//单价
		iptparam->OilBill[IPT_OFFSET_PRC+0]=(char)(iptparam->OilPrice>>8);		
		iptparam->OilBill[IPT_OFFSET_PRC+1]=(char)(iptparam->OilPrice>>0);
		//总累
		iptparam->OilBill[IPT_OFFSET_V_TOT+0]=(char)(iptparam->SumVolume>>24);
		iptparam->OilBill[IPT_OFFSET_V_TOT+1]=(char)(iptparam->SumVolume>>16);
		iptparam->OilBill[IPT_OFFSET_V_TOT+2]=(char)(iptparam->SumVolume>>8);
		iptparam->OilBill[IPT_OFFSET_V_TOT+3]=(char)(iptparam->SumVolume>>0);
		//计量停机代码
		iptparam->OilBill[IPT_OFFSET_STOPNO]=(char)(iptparam->JlStopNO);
		//计算账单校验值
		iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
		//保存账单及备份
		framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
		framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

		//语音提示"您此次的加油量是XXXX.XX升,XXXX.XX元，祝您一路平安，欢迎下次光临"
		if(iptparam->VoiceVolume>0)
		{
			jlvolume = iptparam->OilVolume;	jlmoney = iptparam->OilMoney;
			//您此次的加油量是
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_THISVOLUME;	
			else																				voice[voice_len++]=SPKM_THISVOLUME;
			//XXXX.XX
			tmp_voice_len=iptHexVoiceIdGet(iptparam, jlvolume, tmp_voice);	
			for(i=0; i<tmp_voice_len; i++)	voice[voice_len++]=tmp_voice[i];
			//升
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_SHENG;	
			else																				voice[voice_len++]=SPKM_SHENG;
			//XXXX.XX
			tmp_voice_len=iptHexVoiceIdGet(iptparam, jlmoney, tmp_voice);			
			for(i=0; i<tmp_voice_len; i++)	voice[voice_len++]=tmp_voice[i];
			//元
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_YUAN;	
			else																				voice[voice_len++]=SPKM_YUAN;
			//祝您一路平安，欢迎下次光临
			if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[voice_len++]=SPKW_SEEYOU;		
			else																				voice[voice_len++]=SPKM_SEEYOU;

			iptSpk(iptparam, voice, voice_len);
		}

		//ETC授权转入申请扣款过程
		//非ETC授权转入账单存储过程
		
		if(AUTH_MODEL_ETC == pAuth->Model)
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "扣款申请中请稍候", strlen("扣款申请中请稍候"));
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
*Description		:授权加油结束申请扣款过程
*Input				:nozzle			面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes			数据长度
*Output			:无
*Return			:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//申请扣款
	for(i = 0; i < 3; i++)
	{
		*(tx_buffer + 0) = 0x71;													//命令字Handle
		*(tx_buffer + 1) = iptparam->LogicNozzle;						//枪号 Nzn 
		*(tx_buffer + 2) = pAuth->AntID;										//天线ID AntID
		memcpy(tx_buffer + 3, pAuth->OBUID, 4);						//标签MAC号 OBUID
		memcpy(tx_buffer + 7, pAuth->ContractNo, 8);				//OBU合同序列号ContractNo
		memcpy(tx_buffer + 15, pAuth->OBUPlate, 12);				//车牌号 OBUPlate
		*(tx_buffer + 27) = (char)(iptparam->OilMoney>>24);		//扣款额 Fee
		*(tx_buffer + 28) = (char)(iptparam->OilMoney>>16);	
		*(tx_buffer + 29) = (char)(iptparam->OilMoney>>8);	
		*(tx_buffer + 30) = (char)(iptparam->OilMoney>>0);
		memcpy(tx_buffer + 31, iptparam->OilTime, 7);				//当前时间 Time
		tx_len = 38;
		istate = pcdApplyForAuthETCDebit(tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
		if(0 == istate)	break;

		//taskDelay(1);
		usleep(1000);
	}
	
	//成功，开始保存账单
	//失败，提示错误信息
	if(0 == istate && 0x71 == *(rx_buffer + 0) && 0 == *(rx_buffer + 2))	iptparam->OilBill[IPT_OFFSET_STATE] = 0x00;
	else																										iptparam->OilBill[IPT_OFFSET_STATE] = 0x01;
	//计算账单校验值
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
	//保存账单及备份
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

	//提示支付结果信息
	ibalance = pAuth->Amount - iptparam->OilMoney;
	if(0 == istate && 0x71 == *(rx_buffer + 0) && 0 == *(rx_buffer + 2))
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "支付成功", strlen("支付成功"));
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
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "支付失败", strlen("支付失败"));
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

	//转入账单保存过程
	iptPidSet(iptparam, IPT_PID_AUTH_BILL_SAVE);

	return;
}


/********************************************************************
*Name				:authBillSave
*Description		:授权加油结束账单存储过程
*Input				:nozzle			面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes			数据长度
*Output			:无
*Return				:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//申请TTC
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

	//计算TMAC，计算失败时以默认TMAC填充
	for(i=0; i<3; i++)
	{
		istate = iptTMACCalculate(iptparam, tmac_buffer, apdu_buffer, iptparam->OilBill, 95);
		if(0==istate)		break;

		//taskDelay(100*sysClkRateGet()/1000);
		usleep(100000);
	}
	if(0==istate)memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], tmac_buffer, 4);
	else				memcpy(&iptparam->OilBill[IPT_OFFSET_T_MAC], IPT_TMAC_DEFUALT, 4);

	//计算账单校验值
	iptparam->OilBill[IPT_OFFSET_ZDXOR]=xorGet(iptparam->OilBill, IPT_BILL_SIZE-1);
	//保存到铁电中
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZD, iptparam->OilBill, IPT_BILL_SIZE);
	//保存备份
	framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+IPT_FM_ZDBACKUP, iptparam->OilBill, IPT_BILL_SIZE);

	//保存账单
	for(i = 0; i < 3; i++)
	{
		istate = pcdApplyForBillSave(iptparam->PhysicalNozzle, iptparam->OilBill, IPT_BILL_SIZE);
		if(0 == istate)	break;

		//taskDelay(100*sysClkRateGet()/1000);
		usleep(100000);
	}

	//非ETC授权(即微信或支付宝)需要上传加油结果
	if(AUTH_MODEL_ETC != pAuth->Model)
	{
		//计算授权余额
		if(pAuth->Amount > iptparam->OilMoney)	pauth_balance = pAuth->Amount - iptparam->OilMoney;
		else																	pauth_balance = 0;
	
		//通知平板电脑，授权加油中状态
		//授权额（4HEX）
		//授权方式（1HEX）
		//授权余额（4HEX）
		//加油金额（4HEX）
		//加油升数（4HEX）
		//交易序号（4HEX）
		//枪号（1HEX）
		//备用（20bytes）
		
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

	//退出授权加油
	authProccessExit(iptparam->Id, inbuffer, nbytes);

	return;
}


/********************************************************************
*Name				:authETCPasswordInput
*Description		:授权加油ETC卡密码输入界面
*Input				:nozzle			面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes			数据长度
*Output			:无
*Return			:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//接收到取消输入命令，退出输入
	if(2 == iptparam->PassInputAsk)
	{
		iptparam->PassInputAsk = 0;
		authProccessExit(nozzle, inbuffer, nbytes);
		return;
	}

	//密码输入和上传
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

		//按数字键输入密码，并将输入的密码上传给平板
		if(iptparam->SetButtonLen>=6)	break;

		iptparam->SetButton[iptparam->SetButtonLen++] = button;
		
		dsp_buffer[0] = iptparam->SetButtonLen;	dsp_len = 1;
		dsp(iptparam->DEVDsp, DSP_PASSWORD_INPUT, dsp_buffer, dsp_len);

		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 0;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);
		break;

		
	case KB_BUTTON_CHG:
		//清除输入
		memset(iptparam->SetButton, 0, sizeof(iptparam->SetButton));	iptparam->SetButtonLen = 0;

		dsp_buffer[0] = iptparam->SetButtonLen;	dsp_len = 1;
		dsp(iptparam->DEVDsp, DSP_PASSWORD_INPUT, dsp_buffer, dsp_len);

		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 0;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);
		break;

		
	case KB_BUTTON_ACK:
		//将完整的密码上传给平板并退出操作
		memset(inbffer, 0xff, 12);
		memcpy(inbffer, iptparam->SetButton, iptparam->SetButtonLen);	*(inbffer + 12) = 1;
		pcPsaawordUpload(iptparam->TabletPanel, iptparam->LogicNozzle, inbffer);

		authProccessExit(nozzle, inbuffer, nbytes);
		break;

		
	case KB_BUTTON_BACK:
		//退出操作
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
*Description		:授权加油启动加油失败时的处理
*Input				:nozzle			面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes		数据长度
*Output			:无
*Return			:无
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

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}

	//如果是ETC授权则启动失败后直接返回授权结果界面即可
	//如果是微信或支付宝则取消授权后返回授权结果界面
	
	if(AUTH_MODEL_ETC == pAuth->Model)	iptPidSet(iptparam, IPT_PID_AUTH_PRETREAT);
	else																
	{
		//通知平板电脑授权取消
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
*Description		:授权加油操作处理
*Input				:nozzle			面板号 0=A1面；1=B1面；
*						:inbuffer		数据
*						:nbytes			数据长度
*Output			:无
*Return				:无
*History			:2016-04-12,modified by syj
*/
void authProccess(int nozzle, char *inbuffer, int nbytes)
{
	AuthorizeDataType *pAuth = NULL;
	IptParamStructType *iptparam = NULL;

	//判断操作的支付终端数据结构
	if(0 != nozzle && 1 != nozzle)	return;
	if(0 == nozzle)	{iptparam = &IptParamA;	pAuth = &authorizeDataA1;}
	if(1 == nozzle)	{iptparam = &IptParamB;	pAuth = &authorizeDataB1;}


	switch(iptparam->ProcessId)
	{
	case IPT_PID_AUTH_PRETREAT:			//授权预处理
		authBalancePretreat(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_BALANCE:			//授权余额处理
		authBalance(nozzle, inbuffer, nbytes);
		break;
		
	case	IPT_PID_AUTH_OIL_CHECK:	//授权加油启动前数据自检过程
		authOilCheck(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OIL_START:		//授权加油启动过程
		authOilStart(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OILLING:			//授权加油中
		authOilling(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_OIL_FINISH:		//授权加油结束过程
		authOilFinish(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_DEBIT_APPLY:	//(ETC)授权加油申请扣款过程
		authDebitApply(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_BILL_SAVE:		//授权加油账单存储过程
		authBillSave(nozzle, inbuffer, nbytes);
		break;

	case IPT_PID_AUTH_START_ERR:		//授权加油启动失败
		authOilStartError(nozzle, inbuffer, nbytes);
		break;
		
	case IPT_PID_AUTH_PASS_INPUT:		//(ETC)授权加油ETC卡密码输入界面
		authETCPasswordInput(nozzle, inbuffer, nbytes);
		break;
		
	default:
		authProccessExit(nozzle, inbuffer, nbytes);
		break;
	}

	return;
}









