//#include "oilCfg.h"
//#include "oilFram.h"
//#include "oilKb.h"
//#include "oilSpk.h"
//#include "oilDsp.h"
//#include "oilIpt.h"
//#include "oiletc.h"
#include "../inc/main.h"

//ETC功能处理
void etc_fun_process(unsigned char ID)
{
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	switch(iptparam->ProcessId)
		{
			case ETC_PID_CARINF_READ:
				EtcPidCarinfRead(ID);//ETC读取车辆信息
				break;
			case ETC_PID_SEL_PIN:	
				EtcPinSelPin(ID);//ETC输入验证码
				break;
			case ETC_PID_TO_OBU:
				EtcPidToObu(ID);//ETC选定OBU
				break;
			case ETC_PID_INF_READ:
				EtcPidInfRead(ID); //ETC一次性读取卡信息
				break;
			case ETC_PID_OIL_SURE:
				EtcPidOilSure(ID); //油品不一致需要确认
				break;
			case ETC_PID_PIN_CHECK:
				EtcCardObuPin(ID); //OBU验密
				break;
			default:
				break;
		}
}

//ETC读取车辆信息
void EtcPidCarinfRead(unsigned char ID)
{
	unsigned char tx_buff[50]={0};
	unsigned int tx_len=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->EtcTxFlg==0)
		{
			tx_buff[tx_len++]=ETC_CMD;
			tx_buff[tx_len++]=ETC_01;
			tx_buff[tx_len++]=iptparam->LogicNozzle;

			pcd2PcSend(tx_buff, tx_len);
			iptparam->etc_rec_len=0;
			memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
			
			iptparam->EtcTxCi++;
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=1;
		}
	else if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_01)
		{
			if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					if((iptparam->etc_rec_len-4)/ETCCARDLEN==0)
						{
							dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "无车辆信息", strlen("无车辆信息"));
							IPT_DSP_WAIT();
							iptMainInterface(iptparam);
						}
					else
						{
							iptparam->EtcListNum=(iptparam->etc_rec_len-4)/ETCCARDLEN;
							if(iptparam->EtcListNum>=20)
								{
									iptparam->EtcListNum=20;
									memcpy(iptparam->EtcListInf,iptparam->etc_rec_buff+4,iptparam->EtcListNum*ETCCARDLEN);
								}
							else
								{
									memcpy(iptparam->EtcListInf,iptparam->etc_rec_buff+4,iptparam->etc_rec_len-4);
								}
							memset(iptparam->IcPassword,0,sizeof(iptparam->IcPassword));
							iptparam->IcPasswordLen=0;
							if(iptparam->EtcListInf[25]!=0)
							{
								iptparam->IcPassword[1]=iptparam->EtcListInf[25];
								dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 3);
							}
							else	
								dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 1);
							iptPidSet(iptparam, ETC_PID_SEL_PIN);
						}
				}
			else
				{
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "车辆列表信息失败", strlen("车辆列表信息失败"));
					IPT_DSP_WAIT();
					iptMainInterface(iptparam);
				}
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;		
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "车辆列表信息超时", strlen("车辆列表信息超时"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}

//ETC输入验证码
void EtcPinSelPin(unsigned char ID)
{
	unsigned char dsp_buffer[64]={0}, dsp_len=0, button=0,i=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

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
				
				if(iptparam->IcPasswordLen<4)
				{
					iptparam->IcPassword[iptparam->IcPasswordLen++]=button;

					dsp_buffer[0]=iptparam->IcPasswordLen;
					dsp_len=1;
					if(iptparam->EtcListInf[25]!=0)
					{
						dsp_buffer[1]=iptparam->EtcListInf[25];
						dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,dsp_buffer , 3);
					}
					else
						dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT, dsp_buffer, dsp_len);
				}
				break;
			case KB_BUTTON_CHG:
				memset(iptparam->IcPassword,0,sizeof(iptparam->IcPassword));
				iptparam->IcPasswordLen=0;
				if(iptparam->EtcListInf[25]!=0)
				{
					iptparam->IcPassword[1]=iptparam->EtcListInf[25];
					dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 3);
				}
				else
					dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 1);
				break;
	
			case KB_BUTTON_ACK:
				if(iptparam->IcPasswordLen<4)
				{
					memset(iptparam->IcPassword,0,sizeof(iptparam->IcPassword));
					iptparam->IcPasswordLen=0;
					if(iptparam->EtcListInf[25]!=0)
					{
						iptparam->IcPassword[1]=iptparam->EtcListInf[25];
						dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 3);
					}
					else
						dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 1);
					break;
				}
				for(i=0;i<iptparam->EtcListNum;i++)
				{
					dsp_buffer[0]=(iptparam->IcPassword[0]-0x30)*0x10+(iptparam->IcPassword[1]-0x30)*1;
					dsp_buffer[1]=(iptparam->IcPassword[2]-0x30)*0x10+(iptparam->IcPassword[3]-0x30)*1;
					if(memcmp(dsp_buffer,&iptparam->EtcListInf[i*ETCCARDLEN+20],2)==0)
					{						
						if((iptparam->EtcListInf[22]&0x01)==0x01 || (iptparam->EtcListInf[22]&0x20)==0x20 || \
							(iptparam->EtcListInf[22]&0x40)==0x40 || (iptparam->EtcListInf[22]&0x80)==0x80)
						{
							memset(iptparam->IcPassword,0,sizeof(iptparam->IcPassword));
							iptparam->IcPasswordLen=0;
							dsp_buffer[0]=iptparam->IcPasswordLen;	
							dsp_buffer[1]=iptparam->EtcListInf[22];dsp_len=2;
							dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,dsp_buffer ,dsp_len);
							return;
						}
						else
						{
							memcpy(iptparam->EtcSelCardInf,&iptparam->EtcListInf[i*ETCCARDLEN],ETCCARDLEN);
							iptparam->EtcFreeflag=1;//选定车号释放OBU返回列表
							if(ID==IPT_NOZZLE_1)
								{
									dsp_buffer[0]=iptparam->EtcFreeflag;
									memcpy(dsp_buffer+1,iptparam->EtcSelCardInf+16,4);
									framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, dsp_buffer, 5);
								}
							else
								{
									dsp_buffer[0]=iptparam->EtcFreeflag;
									memcpy(dsp_buffer+1,iptparam->EtcSelCardInf+16,4);
									framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B,dsp_buffer, 5);
								}
							iptPidSet(iptparam, ETC_PID_TO_OBU);
							return;
						}
					}
				}
				memset(iptparam->IcPassword,0,sizeof(iptparam->IcPassword));
				iptparam->IcPasswordLen=0;
				if(iptparam->EtcListInf[25]!=0)
				{
					iptparam->IcPassword[1]=iptparam->EtcListInf[25];
					dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 3);
				}
				else
					dsp(iptparam->DEVDsp, DSP_ETC_PIN_INPUT,iptparam->IcPassword , 1);
				break;

			case KB_BUTTON_BACK:
				iptMainInterface(iptparam); //退卡键退卡并返回待机界面
				break;
			default:
				break;
		}
}

//ETC选定OBU
void EtcPidToObu(unsigned char ID)
{
	unsigned char tx_buff[50]={0};
	unsigned int tx_len=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->EtcTxFlg==0)
		{
			tx_buff[tx_len++]=ETC_CMD;
			tx_buff[tx_len++]=ETC_02;
			tx_buff[tx_len++]=iptparam->LogicNozzle;
			memcpy(tx_buff+tx_len,iptparam->EtcSelCardInf+16,4);
			tx_len+=4;

			pcd2PcSend(tx_buff, tx_len);
			iptparam->etc_rec_len=0;
			memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
			
			iptparam->EtcTxCi++;
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=1;
		}
	else if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_02)
		{

			if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					iptparam->etc_09_num=1;
					memset(iptparam->etc_09_buff,0,sizeof(iptparam->etc_09_buff));
					iptparam->etc_09_len=0;
					iptPidSet(iptparam, ETC_PID_INF_READ);
				}
			else
				{
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "OBU选车失败", strlen("OBU选车失败"));
					IPT_DSP_WAIT();
					iptMainInterface(iptparam);
				}
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;		
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "OBU选车超时", strlen("OBU选车超时"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}

//ETC一次性读取卡信息
void EtcPidInfRead(unsigned char ID)
{
	unsigned char tx_buff[50]={0},istate=0;
	unsigned int tx_len=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->EtcTxFlg==0)
		{
			tx_buff[tx_len++]=ETC_CMD;
			tx_buff[tx_len++]=ETC_09;
			tx_buff[tx_len++]=iptparam->LogicNozzle;
			memcpy(tx_buff+tx_len,iptparam->EtcSelCardInf+16,4);
			tx_len+=4;
			//最大包长度480
			tx_buff[tx_len++]=0x01;
			tx_buff[tx_len++]=0xE0;
			tx_buff[tx_len++]=iptparam->etc_09_num;

			pcd2PcSend(tx_buff, tx_len);
			iptparam->etc_rec_len=0;
			memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
			
			iptparam->EtcTxCi++;
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=1;
		}
	else if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_09)
		{
			if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					if(iptparam->etc_09_num==1)
						{
							iptparam->etc_09_len+=iptparam->etc_rec_len;
							if(iptparam->etc_09_len>=ETC_CARD_MAX)
							{
								dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " 包信息长度错误 ", 16);								
								IPT_DSP_WAIT();
								iptMainInterface(iptparam);
							}
							else
								memcpy(iptparam->etc_09_buff,iptparam->etc_rec_buff,iptparam->etc_rec_len);
						}
					else
						{
							iptparam->etc_09_len+=(iptparam->etc_rec_len-4);
							if(iptparam->etc_09_len>=ETC_CARD_MAX)
							{
								dsp(iptparam->DEVDsp,DSP_TEXT_INFO, " 包信息长度错误 ", 16);
								IPT_DSP_WAIT();
								iptMainInterface(iptparam);
							}
							else
								memcpy(iptparam->etc_09_buff+iptparam->etc_09_len-(iptparam->etc_rec_len-4),iptparam->etc_rec_buff+4,iptparam->etc_rec_len-4);
						}
					iptparam->etc_09_num++;	
					iptparam->EtcTxCi=0;
					iptparam->EtcTxTime=0;
					iptparam->EtcTxFlg=0;
				}
			else if(iptparam->etc_rec_buff[2]==1 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					if(iptparam->etc_09_num==1)
						{
							iptparam->etc_09_len+=iptparam->etc_rec_len;
							if(iptparam->etc_09_len>=ETC_CARD_MAX)
							{
								dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " 包信息长度错误 ", 16);								
								IPT_DSP_WAIT();
								iptMainInterface(iptparam);
							}
							else
								memcpy(iptparam->etc_09_buff,iptparam->etc_rec_buff,iptparam->etc_rec_len);
						}
					else
						{
							iptparam->etc_09_len+=(iptparam->etc_rec_len-4);
							if(iptparam->etc_09_len>=ETC_CARD_MAX)
							{
								dsp(iptparam->DEVDsp,DSP_TEXT_INFO, " 包信息长度错误 ", 16);
								IPT_DSP_WAIT();
								iptMainInterface(iptparam);
							}
							else
								memcpy(iptparam->etc_09_buff+iptparam->etc_09_len-(iptparam->etc_rec_len-4),iptparam->etc_rec_buff+4,iptparam->etc_rec_len-4);
						}
					//卡信息解析
					istate=EtcCardInfJieXi(ID);
					if(istate==1 || istate==2 || istate==3)
						{
							//清除卡密码输入
							memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
							iptparam->IcPasswordLen=0;

							iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC卡应用默认为电子油票
							iptparam->DS=IPT_PAYUNIT_MONEY;				  //扣款来源默认为石油卡电子油票
							iptparam->Payment=IPT_PAYMENT_MONEY;		//结算方式默认为现金
							iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //卡类型默认为石化规范卡
							iptparam->PriceDiscount = 0;						//价格折扣额
							iptparam->UserElecFlag = 0;

							//显示并转入卡预处理过程
							dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
							iptPidSet(iptparam, IPT_PID_IC_PRETREAT);
							iptparam->IcState.IcTypeS2=IC_CARD_CPU;
							iptparam->Step=1;
							iptparam->etc_touming_flg=0;

							//促销机通知平板油机状态
							if(1 == paramPromotionGet())
							{
								iptparam->TaState = IPT_STATE_CARD_PRETREAT;	iptparam->TaStateParamLength = 0;
								pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
							}
						}
					else if(istate==0)
						{
							//促销机通知平板油机状态
							if(1 == paramPromotionGet())
							{
								iptparam->TaState = IPT_STATE_CARD_PRETREAT;	iptparam->TaStateParamLength = 0;
								pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
							}
							
							if(iptparam->etc_card_num==7)//2017.07.25第一次使用未验密
								{
									//清除卡密码输入
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC卡应用默认为电子油票
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //扣款来源默认为石油卡电子油票
									iptparam->Payment=IPT_PAYMENT_MONEY;		//结算方式默认为现金
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //卡类型默认为石化规范卡
									iptparam->PriceDiscount = 0;						//价格折扣额
									iptparam->UserElecFlag = 0;

									//显示并转入卡预处理过程
									dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
									iptPidSet(iptparam, IPT_PID_IC_PRETREAT);
									iptparam->IcState.IcTypeS2=IC_CARD_CPU;
									iptparam->Step=1;
									iptparam->etc_touming_flg=0;
								}
								else if(iptparam->etc_card_num==10)
								{
									//清除卡密码输入
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC卡应用默认为电子油票
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //扣款来源默认为石油卡电子油票
									iptparam->Payment=IPT_PAYMENT_MONEY;		//结算方式默认为现金
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //卡类型默认为石化规范卡
									iptparam->PriceDiscount = 0;						//价格折扣额
									iptparam->UserElecFlag = 0;

									//显示并转入卡预处理过程
									dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
									iptPidSet(iptparam, IPT_PID_PSAM_PRETREAT);
									iptparam->IcState.IcTypeS2=IC_CARD_CPU;
									iptparam->Step=0;
									iptparam->etc_touming_flg=1;
								}
								else
								{
									dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " 包信息格式错误 ", 16);
									IPT_DSP_WAIT();
									//清除卡密码输入
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC卡应用默认为电子油票
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //扣款来源默认为石油卡电子油票
									iptparam->Payment=IPT_PAYMENT_MONEY;		//结算方式默认为现金
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //卡类型默认为石化规范卡
									iptparam->PriceDiscount = 0;						//价格折扣额
									iptparam->UserElecFlag = 0;

									//显示并转入卡预处理过程
									dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
									iptPidSet(iptparam, IPT_PID_IC_PRETREAT);
									iptparam->IcState.IcTypeS2=IC_CARD_CPU;
									iptparam->Step=1;
									iptparam->etc_touming_flg=0;
								}
						}
				}
			else
				{
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "读取卡信息失败", strlen("读取卡信息失败"));
					IPT_DSP_WAIT();
					iptMainInterface(iptparam);
				}
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;		
		}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "读取卡信息超时", strlen("读取卡信息超时"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}


//卡信息解析
char EtcCardInfJieXi(unsigned char ID)
{
	unsigned int len=4,dsp_len=0,i=0,bao_len=0;
	unsigned char dsp_buffer[64]={0};
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	//返回  0正常,1APDU状态错误,2通信超时,3命令格式错误
	iptparam->etc_card_num=0;
	//命令号1 选择MF文件
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x01)
	{
		iptparam->etc_card_num=1;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]>=2)
		if(bao_len>=2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{;}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " IC卡MF选择超时 ", 16);	
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], " IC卡MF选择失败 ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
		
	//命令号2 选择卡应用
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x02)
	{
		iptparam->etc_card_num=2;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]>=2)
		if(bao_len>=2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{;}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC卡应用选择超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC卡应用选择失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号3 读取21文件
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x03)
	{
		iptparam->etc_card_num=3;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x1E+2)
		if(bao_len==(30+2))
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//保存卡信息
				memcpy(iptparam->IcIssuerMark, iptparam->etc_09_buff+len-bao_len, 8);		  //发卡方标识	
				iptparam->IcAppMatk=iptparam->etc_09_buff[len-bao_len+8];									//应用类型标识					
				iptparam->IcAppVersion=iptparam->etc_09_buff[len-bao_len+9];							//应用版本
				memcpy(iptparam->IcAppId, iptparam->etc_09_buff+len-bao_len+10, 10);			//应用序列号
				memcpy(iptparam->IcEnableTime, iptparam->etc_09_buff+len-bao_len+20, 4);	//应用启用日期
				memcpy(iptparam->IcInvalidTime, iptparam->etc_09_buff+len-bao_len+24, 4);	//应用有效截止日期
				iptparam->IcCodeVersion=iptparam->etc_09_buff[len-bao_len+28];						//指令集版本			
				iptparam->IcFile21Unused=0;											                          //21文件备用区域
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC卡读21文件超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC卡读21文件失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号4 读取22文件
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x04)
	{
		iptparam->etc_card_num=4;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x29+2)
		if(bao_len==0x29+2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//保存卡信息
				iptparam->IcTypeMark=iptparam->etc_09_buff[len-bao_len];									//卡类型标识
				iptparam->IcStaffMark = iptparam->etc_09_buff[len-bao_len+1];						  //本系统职工标识
				memcpy(iptparam->IcUserName, iptparam->etc_09_buff+len-bao_len+2, 20);		//持卡人姓名
				memcpy(iptparam->IcUserIdeId, iptparam->etc_09_buff+len-bao_len+22, 18);	//持卡人证件(identity)号码(ASCII)
				iptparam->IcUserIdeType=iptparam->etc_09_buff[len-bao_len+40];						//持卡人证件类型
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC卡读22文件超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC卡读22文件失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号5 读取27文件
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x05)
	{
		iptparam->etc_card_num=5;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x20+2)
		if(bao_len==0x20+2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//保存卡信息
				iptparam->IcDefaultPassword=iptparam->etc_09_buff[len-bao_len];							//是否采用默认密码,00=使用默认密码，01=使用用户密码
				iptparam->IcStaffId=iptparam->etc_09_buff[len-bao_len+1];										//员工号(内部卡有效)
				memcpy(iptparam->IcStaffPassword, iptparam->etc_09_buff+len-bao_len+2, 2);	//员工密码(内部卡有效)
				iptparam->IcDebitUnit = iptparam->etc_09_buff[len-bao_len+4];								//扣款单位(00H=元；01H=升；社会站卡结构有此字段)
				iptparam->IcDiscountFlag = iptparam->etc_09_buff[len-bao_len+5];						//是否折扣卡(00H=非折扣卡；01H=折扣卡；社会站卡结构有此字段)
				iptparam->IcAppType = iptparam->etc_09_buff[len-bao_len+6];									//卡片应用类型(社会站卡结构有此字段)
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC卡读27文件超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC卡读27文件失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号6 读取28文件
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x06)
	{
		iptparam->etc_card_num=6;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x42+2)
		if(bao_len==0x42+2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//保存卡信息
				memcpy(iptparam->IcOilLimit, iptparam->etc_09_buff+len-bao_len, 2);					//油品限制
				iptparam->IcRegionTypeLimit=iptparam->etc_09_buff[len-bao_len+2];						//限地区,油站加油方式
				memcpy(iptparam->IcRegionLimit, iptparam->etc_09_buff+len-bao_len+3, 40);		//限地区,油站加油
				memcpy(iptparam->IcVolumeLimit, iptparam->etc_09_buff+len-bao_len+43, 2);		//限每次加油量
				iptparam->IcTimesLimit=iptparam->etc_09_buff[len-bao_len+45];								//限每天加油次数
				memcpy(iptparam->IcMoneyDayLimit, iptparam->etc_09_buff+len-bao_len+46, 4);	//限每天加油金额
				memcpy(iptparam->IcCarIdLimit, iptparam->etc_09_buff+len-bao_len+50, 16);		//车牌号限制(ASCII)
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC卡读28文件超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC卡读28文件失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号7 验密结果
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x07)
	{
		iptparam->etc_card_num=7;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]>=2)
		if(bao_len>=2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{			
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				
				return 0;//2017.07.25验密超时或者新标签密码卡第一次报错，直接改为不报错直接走透明传输
//				dsp(DSP_TEXT_INFO, "  IC卡验密超时  ", 16);
//				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(dsp_buffer+3, "  IC卡验密失败  ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号8 查询卡交易记录
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x08)
	{
		iptparam->etc_card_num=8;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if((network.faceResult.checkbuff[len]%25)==2)
		if((bao_len%25)==2)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			iptparam->IcRecordNumber=bao_len/25;
			if(iptparam->IcRecordNumber>=10)
				iptparam->IcRecordNumber=10;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x6A\x83",2)==0)
			{
				//保存卡信息
				for(i=0;i<iptparam->IcRecordNumber;i++)
				{
					memcpy(iptparam->IcRecord[i].TTC, iptparam->etc_09_buff+len-bao_len+i*25, 2);
					memcpy(iptparam->IcRecord[i].Limit, iptparam->etc_09_buff+len-bao_len+2+i*25, 3);
					memcpy(iptparam->IcRecord[i].Money, iptparam->etc_09_buff+len-bao_len+5+i*25, 4);
					memcpy(&iptparam->IcRecord[i].Type, &iptparam->etc_09_buff[len-bao_len+9+i*25], 1);
					memcpy(iptparam->IcRecord[i].TermID, iptparam->etc_09_buff+len-bao_len+10+i*25, 6);
					memcpy(iptparam->IcRecord[i].Time, iptparam->etc_09_buff+len-bao_len+16+i*25, 7);
				}
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "读卡交易明细超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "读卡交易明细失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号9 读取卡灰锁信息
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==0x09)
	{
		iptparam->etc_card_num=9;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x1E+2 || network.faceResult.checkbuff[len]==2)
		if(bao_len==(30+2) || (bao_len>=2))
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//卡插入时的状态
				iptparam->ICStateFirst=iptparam->etc_09_buff[len-bao_len];
			
				//保存灰锁信息
				iptparam->IcLockMark=iptparam->etc_09_buff[len-bao_len];								  //状态字:0x00=无灰锁；0x01=已灰锁；0x10=TAC未读
				iptparam->IcLockType=iptparam->etc_09_buff[len-bao_len+1];								//上次发生解扣或灰锁交易的交易类型标识
				iptparam->IcLockET=iptparam->etc_09_buff[len-bao_len+2];									//上次发生解扣或灰锁交易为ET
				memcpy(iptparam->IcLockBalance, iptparam->etc_09_buff+len-bao_len+3, 4);	//上次发生解扣或灰锁的有效余额
				memcpy(iptparam->IcLockCTC, iptparam->etc_09_buff+len-bao_len+7, 2);			//上次发生解扣或灰锁的交易序号
				memcpy(iptparam->IcLockTermId, iptparam->etc_09_buff+len-bao_len+9, 6);		//上次发生解扣或灰锁的终端编号
				memcpy(iptparam->IcLockTime, iptparam->etc_09_buff+len-bao_len+15, 7);		//上次发生解扣或灰锁的日期时间
				memcpy(iptparam->IcLockMoney, iptparam->etc_09_buff+len-bao_len+22, 4);		//上次发生解扣或灰锁的交易金额
				memcpy(iptparam->IcLockGTAC, iptparam->etc_09_buff+len-bao_len+26, 4);		//上次发生解扣或灰锁的GTAC或TAC或MAC3
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\x69\x85",2)==0)
			{
				iptparam->IcLockMark=0;																			     //状态字:0x00=无灰锁；0x01=已灰锁；0x10=TAC未读
				iptparam->IcLockType=0;																			     //上次发生解扣或灰锁交易的交易类型标识
				iptparam->IcLockET=0;																				     //上次发生解扣或灰锁交易为ET
				memcpy(iptparam->IcLockBalance, "\x00\x00\x00\x00", 4);				   //上次发生解扣或灰锁的有效余额
				memcpy(iptparam->IcLockCTC, "\x00\x00", 2);										   //上次发生解扣或灰锁的交易序号
				memcpy(iptparam->IcLockTermId, "\x00\x00\x00\x00\x00\x00", 6);   //上次发生解扣或灰锁终编号
				memcpy(iptparam->IcLockTime, "\x00\x00\x00\x00\x00\x00\x00", 7); //上次发生解扣或灰锁的日期时间
				memcpy(iptparam->IcLockMoney, "\x00\x00\x00\x00", 4);						 //上次发生解扣或灰锁的交易金额
				memcpy(iptparam->IcLockGTAC, "\x00\x00\x00\x00", 4);						 //上次发生解扣或灰锁的GTAC或TAC或MAC3
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "读卡灰锁信息超时", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "读卡灰锁信息失败", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//命令号10 读取卡余额
	if((iptparam->etc_09_len>=len) && iptparam->etc_09_buff[len]==10)
	{
		iptparam->etc_card_num=10;
		len++;
		bao_len=(iptparam->etc_09_buff[len]<<8)+iptparam->etc_09_buff[len+1];
		len+=2;
//		if(network.faceResult.checkbuff[len]==0x04)
		if(bao_len==0x06)
		{
//			len=network.faceResult.checkbuff[len]+len;
			len=bao_len+len;
			if(memcmp(iptparam->etc_09_buff+len-2,"\x90\x00",2)==0)
			{
				//卡已读出余额
				iptparam->IcValid=1;

				//结算方式默认为现金
				iptparam->Payment=IPT_PAYMENT_MONEY;

				//卡余额信息
				memcpy(iptparam->IcBalance, iptparam->etc_09_buff+len-bao_len, 4);
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "  读卡余额超时  ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "  读卡余额失败  ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包长度错误   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   包数据错误   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	
	return 0;
}

//车牌号处理
void EtcCardCarHandle(unsigned char ID)
{
	int dsp_len=0,i=0;
	unsigned char dsp_buffer[64]={0};
	unsigned char CardInf[16]={0};
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	//加油卡判断限车号转入员工密码输入过程，否则转入卡明细查询界面
	if(0!=memcmp(iptparam->IcCarIdLimit, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 16))
	{
		memcpy(CardInf,iptparam->IcCarIdLimit,16);
		for(i=15;i>=0;i--)
		{
			if(CardInf[i]==0xFF)
				CardInf[i]=0x00;
			else
				break;
		}
		
		if(iptparam->EtcOilFlg==1)//ETC加油
		{
			if(memcmp(CardInf,iptparam->EtcSelCardInf,16)==0)//车牌一致直接跳过员工输入密码
			{
				iptparam->etc_limit_car=1;
//				goto NoYiZhi;
				//车牌一致强制输入员工密码
				goto YG_input1;

			}
			else//报错
			{
				dsp(iptparam->DEVDsp,DSP_CAR_NOYIZHI,dsp_buffer,0);
				IPT_DSP_WAIT();
				iptMainInterface(iptparam);
			}
		}
		else
		{
YG_input1:
			memset(iptparam->SetButton, 0, 16);	iptparam->SetButtonLen=0;
			iptPidSet(iptparam, IPT_PID_IC_STAF_PASSIN);

			memcpy(&dsp_buffer[0], iptparam->IcCarIdLimit, 16);
			dsp_buffer[16]=iptparam->SetButtonLen;	dsp_len=17;
			dsp(iptparam->DEVDsp,DSP_CARD_CARLIMIT, dsp_buffer, dsp_len);
		}
	}
	else
	{
//NoYiZhi:
		iptPidSet(iptparam, IPT_PID_IC_NOTES_CHECK);
	}
}

//油品换算显示函数
void EtcOilNameCourse(unsigned char ID)
{
	unsigned int oilCode=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	memset(iptparam->etc_recnet_oil,0,sizeof(iptparam->etc_recnet_oil));
	memset(iptparam->etc_now_oil,0,sizeof(iptparam->etc_now_oil));
	oilCode=((iptparam->OilVoice[0]&0x0f)<<12)|((iptparam->OilVoice[1]&0x0f)<<8)|\
		((iptparam->OilVoice[2]&0x0f)<<4)|((iptparam->OilVoice[3]&0x0f)<<0);
#if 0	
	if(iptparam->voiceParam.voiceOil==0)//与后台自动匹配
	{
		oilCode=(iptparam->OilCode[0]<<8)+(iptparam->OilCode[1]<<0);
		oilCodeConvertVoice(oilCode);
	}
#endif
	if(((oilCode>=0x0107)&&(oilCode<=0x0111)) ||((oilCode>=0x0307)&&(oilCode<=0x0311)))
		memcpy(iptparam->etc_now_oil,"89#汽油",strlen("89#汽油"));
	else if(((oilCode>=0x0011)&&(oilCode<=0x0013))||(oilCode==0x0112) ||((oilCode>=0x0211)&&(oilCode<=0x0213)) ||(oilCode==0x0312))
		memcpy(iptparam->etc_now_oil,"90#汽油",strlen("90#汽油"));
	else if(((oilCode>=0x0103)&&(oilCode<=0x0106))||(oilCode==0x0113) ||((oilCode>=0x0303)&&(oilCode<=0x0306)) ||(oilCode==0x0313))
		memcpy(iptparam->etc_now_oil,"92#汽油",strlen("92#汽油"));
	else if(((oilCode>=0x0014)&&(oilCode<=0x0017))||(oilCode==0x0114) ||((oilCode>=0x0214)&&(oilCode<=0x0217)) ||(oilCode==0x0314))
		memcpy(iptparam->etc_now_oil,"93#汽油",strlen("93#汽油"));
	else if(((oilCode>=0x0018)&&(oilCode<=0x0021))||(oilCode==0x0115) ||((oilCode>=0x0218)&&(oilCode<=0x0221)) ||(oilCode==0x0315))
		memcpy(iptparam->etc_now_oil,"95#汽油",strlen("95#汽油"));
	else if(((oilCode>=0x0022)&&(oilCode<=0x0025))||(oilCode==0x0116) ||((oilCode>=0x0222)&&(oilCode<=0x0225)) ||(oilCode==0x0316))
		memcpy(iptparam->etc_now_oil,"97#汽油",strlen("97#汽油"));
	else if(((oilCode>=0x0026)&&(oilCode<=0x0028))||(oilCode==0x0117) ||((oilCode>=0x0226)&&(oilCode<=0x0228)) ||(oilCode==0x0317))
		memcpy(iptparam->etc_now_oil,"98#汽油",strlen("98#汽油"));
	else if(oilCode==0x0029||oilCode==0x0229)
		memcpy(iptparam->etc_now_oil,"120#汽油",strlen("120#汽油"));
	else if(oilCode==0x0038 || oilCode==0x0123||oilCode==0x0238 || oilCode==0x0331)
		memcpy(iptparam->etc_now_oil,"0#柴油",strlen("0#柴油"));
	else if(oilCode==0x0047||oilCode==0x0247)
		memcpy(iptparam->etc_now_oil,"+5#柴油",strlen("+5#柴油"));
	else if(oilCode==0x0048||oilCode==0x0248)
		memcpy(iptparam->etc_now_oil,"+10#柴油",strlen("+10#柴油"));
	else if(oilCode==0x0049||oilCode==0x0249)
		memcpy(iptparam->etc_now_oil,"+15#柴油",strlen("+15#柴油"));
	else if(oilCode==0x0050||oilCode==0x0250)
		memcpy(iptparam->etc_now_oil,"+20#柴油",strlen("+20#柴油"));
	else if(oilCode==0x0039||oilCode==0x0239)
		memcpy(iptparam->etc_now_oil,"-5#柴油",strlen("-5#柴油"));
	else if(oilCode==0x0040||oilCode==0x0240)
		memcpy(iptparam->etc_now_oil,"-10#柴油",strlen("-10#柴油"));
	else if(oilCode==0x0043||oilCode==0x0243)
		memcpy(iptparam->etc_now_oil,"-20#柴油",strlen("-20#柴油"));
	else if(oilCode==0x0044||oilCode==0x0244)
		memcpy(iptparam->etc_now_oil,"-30#柴油",strlen("-30#柴油"));
	else if(oilCode==0x0045||oilCode==0x0245)
		memcpy(iptparam->etc_now_oil,"-35#柴油",strlen("-35#柴油"));
	else if(oilCode==0x0046||oilCode==0x0246)
		memcpy(iptparam->etc_now_oil,"-50#柴油",strlen("-50#柴油"));
	
	if((iptparam->EtcSelCardInf[ETCCARDLEN-3]==0 || iptparam->EtcSelCardInf[ETCCARDLEN-3]==0x30) && (iptparam->EtcSelCardInf[ETCCARDLEN-2]!=0))
		sprintf((char *)iptparam->etc_recnet_oil,"%d#汽油",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	else if((iptparam->EtcSelCardInf[ETCCARDLEN-3]==0 || iptparam->EtcSelCardInf[ETCCARDLEN-3]==0x30) && (iptparam->EtcSelCardInf[ETCCARDLEN-2]==0))/*0号柴油*/
		sprintf((char *)iptparam->etc_recnet_oil,"%d#柴油",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	else if(iptparam->EtcSelCardInf[ETCCARDLEN-3]=='-' || iptparam->EtcSelCardInf[ETCCARDLEN-3]=='+')
	{
		iptparam->etc_recnet_oil[0]=iptparam->EtcSelCardInf[ETCCARDLEN-3];
		sprintf((char *)iptparam->etc_recnet_oil+1,"%d#柴油",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	}
}

//ETC余额处理
void EtcCardYuehanlde(unsigned char ID)
{
	unsigned char dsp_buffer[64]={0},LockSt=0,dspbuff[20]={0};
	unsigned int voice[10]={0},voice_len=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(0x00==iptparam->IcLockMark)
	{
		EtcOilNameCourse(ID);
		if(memcmp(iptparam->EtcSelCardInf+ETCCARDLEN-3,"\xff\xff",2)==0 || \
			memcmp(iptparam->etc_now_oil,iptparam->etc_recnet_oil,10)==0)//还未加过油或者油品号相同
		{	
			iptparam->IcValid=1; //卡已读出余额
			iptparam->Payment=IPT_PAYMENT_MONEY; //结算方式默认为现金

			//初始化预置数据
			memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));		iptparam->IntegerLen=0;
			iptparam->Point=0;
			memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
			iptparam->PresetMode=IPT_PRESET_NO;	iptparam->PresetVolume=0;	iptparam->PresetMoney=0;
		
			memset(iptparam->SetButton,0,sizeof(iptparam->SetButton));
			iptparam->SetButtonLen=0;
			if(iptparam->etc_limit_car==1)
				{
					//号枪正使用限车号卡加油
					if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	
						{
							if(iptparam->LogicNozzle/10==0)
								voice[voice_len++]=iptparam->LogicNozzle;
							else if((iptparam->LogicNozzle/10)!=0)
								{
									if((iptparam->LogicNozzle%10)==0)
										{
											if((iptparam->LogicNozzle/10)==1)
												{
													voice[voice_len++]=SPKW_TEN;
												}
											else
												{
													voice[voice_len++]=(iptparam->LogicNozzle/10);
													voice[voice_len++]=SPKW_TEN;
												}
										}
									else
										{
											if((iptparam->LogicNozzle/10)==1)
												{
													voice[voice_len++]=SPKW_TEN;
													voice[voice_len++]=(iptparam->LogicNozzle%10);
												}
											else
												{
													voice[voice_len++]=(iptparam->LogicNozzle/10);
													voice[voice_len++]=SPKW_TEN;
													voice[voice_len++]=(iptparam->LogicNozzle%10);
												}
										}
								}
								voice[voice_len++]=SPKW_LIMIT_CAR;
								
						}
#if 0
					else	
						{
							if(iptparam->LogicNozzle/10==0)
								voice[voice_len++]=iptparam->LogicNozzle+0x0200;
							else if((iptparam->LogicNozzle/10)!=0)
								{
									if((iptparam->LogicNozzle%10)==0)
										{
											if((iptparam->LogicNozzle/10)==1)
												{
													voice[voice_len++]=SPKW_TEN+0x0200;
												}
											else
												{
													voice[voice_len++]=(iptparam->LogicNozzle/10)+0x0200;
													voice[voice_len++]=SPKW_TEN+0x0200;
												}
										}
									else
										{
											if((iptparam->LogicNozzle/10)==1)
												{
													voice[voice_len++]=SPKW_TEN+0x0200;
													voice[voice_len++]=(iptparam->LogicNozzle%10)+0x0200;
												}
											else
												{
													voice[voice_len++]=(iptparam->LogicNozzle/10)+0x0200;
													voice[voice_len++]=SPKW_TEN+0x200;
													voice[voice_len++]=(iptparam->LogicNozzle%10)+0x0200;
												}
										}
								}
								voice[voice_len++]=SPKW_LIMIT_CAR;
						}
#endif
					iptSpk(iptparam, voice, voice_len);
				}
			else
				{
					//请提枪加油或以定量方式加油
					if(iptparam->VoiceVolume > 0 && IPT_PAYUNIT_LOYALTY != iptparam->PayUnit)
						{
							if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_OILPLEASE;	/*女声*/
							else												voice[0]=SPKM_OILPLEASE;	/*男声*/
							iptSpk(iptparam, voice, 1);
						}
				}
			iptIcBalanceDsp(iptparam->Id);
			iptPidSet(iptparam, IPT_PID_IC_BALANCE);

			//促销机通知平板油机状态
			if(1 == paramPromotionGet())
			{
				iptparam->TaState = IPT_STATE_CARD_BALANCE;	iptparam->TaStateParamLength = 0;
				memcpy(iptparam->TaStateParam + 0, iptparam->IcAppId, 10);
				memcpy(iptparam->TaStateParam + 10, iptparam->IcBalance, 4);
				if(IPT_PAYUNIT_MONEY == iptparam->PayUnit)	*(iptparam->TaStateParam + 14) = 0;
				else																			*(iptparam->TaStateParam + 14) = 1;
				*(iptparam->TaStateParam + 15) = (char)(0>>24);	*(iptparam->TaStateParam + 16) = (char)(0>>16);
				*(iptparam->TaStateParam + 17) = (char)(0>>8);		*(iptparam->TaStateParam + 18) = (char)(0>>0);
				if(IPT_PRESET_VOLUME == iptparam->PresetMode)	*(iptparam->TaStateParam + 19) = 1;
				else																					*(iptparam->TaStateParam + 19) = 0;
				*(iptparam->TaStateParam + 20) = 0x01;
				iptparam->TaStateParamLength = 21;
				pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
			}
		}
		else
		{
			memcpy(dspbuff,iptparam->etc_recnet_oil,10);
			memcpy(dspbuff+10,iptparam->etc_now_oil,10);
			dsp(iptparam->DEVDsp,DSP_ETC_OILCR,dspbuff,20);
			iptPidSet(iptparam, ETC_PID_OIL_SURE);
		}
	}
	else if(0x01==iptparam->IcLockMark)
	{
		//灰卡，转入灰记录查询过程
		dsp(iptparam->DEVDsp, DSP_CARD_UNLOCK_FINISH, "\x00", 0);
		iptPidSet(iptparam, IPT_PID_IC_LOCKRECORD);
	}
	else if(0x10==iptparam->IcLockMark)
	{
		//转入TAC清除过程
		dsp(iptparam->DEVDsp,DSP_TEXT_INFO, "  卡补充处理中  ", strlen("  卡补充处理中  "));
		iptPidSet(iptparam,IPT_PID_IC_TAC_CLEAR);
	}
	else
	{
		dsp(iptparam->DEVDsp,DSP_TEXT_INFO, "  灰锁状态字错  ", 16);
		IPT_DSP_WAIT();
		iptMainInterface(iptparam);
	}
}

//油品不一致需要确认
void EtcPidOilSure(unsigned char ID)
{
	unsigned int voice[10]={0},voice_len=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	if(iptparam->Button==KB_BUTTON_ACK)
	{
		iptparam->IcValid=1; //卡已读出余额
		iptparam->Payment=IPT_PAYMENT_MONEY; //结算方式默认为现金

		//初始化预置数据
		memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));		iptparam->IntegerLen=0;
		iptparam->Point=0;
		memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
		iptparam->PresetMode=IPT_PRESET_NO;	iptparam->PresetVolume=0;	iptparam->PresetMoney=0;
	
		memset(iptparam->SetButton,0,sizeof(iptparam->SetButton));
		iptparam->SetButtonLen=0;
		if(iptparam->etc_limit_car==1)
			{
				//号枪正使用限车号卡加油
				if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	
					{
						if(iptparam->LogicNozzle/10==0)
							voice[voice_len++]=iptparam->LogicNozzle;
						else if((iptparam->LogicNozzle/10)!=0)
							{
								if((iptparam->LogicNozzle%10)==0)
									{
										if((iptparam->LogicNozzle/10)==1)
											{
												voice[voice_len++]=SPKW_TEN;
											}
										else
											{
												voice[voice_len++]=(iptparam->LogicNozzle/10);
												voice[voice_len++]=SPKW_TEN;
											}
									}
								else
									{
										if((iptparam->LogicNozzle/10)==1)
											{
												voice[voice_len++]=SPKW_TEN;
												voice[voice_len++]=(iptparam->LogicNozzle%10);
											}
										else
											{
												voice[voice_len++]=(iptparam->LogicNozzle/10);
												voice[voice_len++]=SPKW_TEN;
												voice[voice_len++]=(iptparam->LogicNozzle%10);
											}
									}
							}
							voice[voice_len++]=SPKW_LIMIT_CAR;
							
					}
#if 0
				else	
					{
						if(iptparam->LogicNozzle/10==0)
							voice[voice_len++]=iptparam->LogicNozzle+0x0200;
						else if((iptparam->LogicNozzle/10)!=0)
							{
								if((iptparam->LogicNozzle%10)==0)
									{
										if((iptparam->LogicNozzle/10)==1)
											{
												voice[voice_len++]=SPKW_TEN+0x0200;
											}
										else
											{
												voice[voice_len++]=(iptparam->LogicNozzle/10)+0x0200;
												voice[voice_len++]=SPKW_TEN+0x0200;
											}
									}
								else
									{
										if((iptparam->LogicNozzle/10)==1)
											{
												voice[voice_len++]=SPKW_TEN+0x0200;
												voice[voice_len++]=(iptparam->LogicNozzle%10)+0x0200;
											}
										else
											{
												voice[voice_len++]=(iptparam->LogicNozzle/10)+0x0200;
												voice[voice_len++]=SPKW_TEN+0x200;
												voice[voice_len++]=(iptparam->LogicNozzle%10)+0x0200;
											}
									}
							}
							voice[voice_len++]=SPKW_LIMIT_CAR;
					}
#endif
				iptSpk(iptparam, voice, voice_len);
			}
		else
			{
				//请提枪加油或以定量方式加油
				if(iptparam->VoiceVolume > 0 && IPT_PAYUNIT_LOYALTY != iptparam->PayUnit)
					{
						if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	
							voice[0]=SPKW_OILPLEASE;	//女声
						else
							voice[0]=SPKM_OILPLEASE;	//男声
						iptSpk(iptparam, voice, 1);
					}
			}

		iptIcBalanceDsp(iptparam->Id);
		iptPidSet(iptparam, IPT_PID_IC_BALANCE);

		//促销机通知平板油机状态
		if(1 == paramPromotionGet())
		{
			iptparam->TaState = IPT_STATE_CARD_BALANCE;	iptparam->TaStateParamLength = 0;
			memcpy(iptparam->TaStateParam + 0, iptparam->IcAppId, 10);
			memcpy(iptparam->TaStateParam + 10, iptparam->IcBalance, 4);
			if(IPT_PAYUNIT_MONEY == iptparam->PayUnit)	*(iptparam->TaStateParam + 14) = 0;
			else																			*(iptparam->TaStateParam + 14) = 1;
			*(iptparam->TaStateParam + 15) = (char)(0>>24);	*(iptparam->TaStateParam + 16) = (char)(0>>16);
			*(iptparam->TaStateParam + 17) = (char)(0>>8);		*(iptparam->TaStateParam + 18) = (char)(0>>0);
			if(IPT_PRESET_VOLUME == iptparam->PresetMode)	*(iptparam->TaStateParam + 19) = 1;
			else																					*(iptparam->TaStateParam + 19) = 0;
			*(iptparam->TaStateParam + 20) = 0x01;
			iptparam->TaStateParamLength = 21;
			pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
		}
	}
	else if(iptparam->Button==KB_BUTTON_BACK)
	{
		iptMainInterface(iptparam);
	}
}

//更新油品号
void EtcUpdateGrade(unsigned char ID)
{
	unsigned char txbuff[50]={0};
	unsigned char txlen=0,i=0,oil_num=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	if(iptparam->etc_update_flag==0 && iptparam->EtcTxFlg==0)
	{
		txlen=0;
		txbuff[txlen++]=ETC_CMD;
		txbuff[txlen++]=ETC_0A;
		txbuff[txlen++]=iptparam->LogicNozzle;
		memcpy(txbuff+txlen,iptparam->EtcSelCardInf+16,4);
		txlen+=4;
		if(iptparam->etc_now_oil[0]==0x30)/*0#柴油*/
		 memcpy(txbuff+txlen,"\x30\x00",2);
		else if((iptparam->etc_now_oil[0]>=0x31) && (iptparam->etc_now_oil[0]<=0x39))/*汽油*/
		{
			for(i=0;i<10;i++)
			{
				if(iptparam->etc_now_oil[i]=='#')
					break;
				oil_num=oil_num*10+(iptparam->etc_now_oil[i]-0x30)*1;
			}
			txbuff[txlen]=0x30;
			txbuff[txlen+1]=oil_num;
		}
		else if(iptparam->etc_now_oil[0]=='-' || iptparam->etc_now_oil[0]=='+')
		{
			for(i=1;i<10;i++)
			{
				if(iptparam->etc_now_oil[i]=='#')
					break;
				oil_num=oil_num*10+(iptparam->etc_now_oil[i]-0x30)*1;
			}
			txbuff[txlen]=iptparam->etc_now_oil[0];
			txbuff[txlen+1]=oil_num;
		}
		txlen+=2;
		pcd2PcSend(txbuff, txlen);
		iptparam->etc_rec_len=0;
		memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
		
		iptparam->EtcTxCi++;
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=1;		
		
	}
	else if(iptparam->etc_rec_len!=0 && iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_0A)
	{
		if(iptparam->etc_rec_buff[2]==0x00 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
		{
			iptparam->etc_update_flag=1;
		}
		else if(iptparam->etc_rec_buff[2]!=0x00)
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;	
		}
		else if(iptparam->etc_rec_buff[3]!=iptparam->LogicNozzle)
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;	
		}
	}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
	{
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=0;		
	}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
	{
		iptparam->etc_update_flag=1;
	}
}

//显示余额
void EtcYueDisHandle(unsigned char ID)
{
	unsigned char txbuff[50]={0};
	unsigned char txlen=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->etc_yue_dis_flag==0 && iptparam->EtcTxFlg==0)
	{
		txlen=0;
		txbuff[txlen++]=ETC_CMD;
		txbuff[txlen++]=ETC_0B;
		txbuff[txlen++]=iptparam->LogicNozzle;
		memcpy(txbuff+txlen,iptparam->EtcSelCardInf+16,4);
		txlen+=4;
		if(iptparam->ICStateFirst==0x00)
		{
			memcpy(txbuff+txlen,iptparam->IcBalance,4);
		}
		else if(iptparam->ICStateFirst==0x01)
		{
			memcpy(txbuff+txlen,iptparam->IcBalance,4);
		}
		else if(iptparam->ICStateFirst==0x10)
		{
			memcpy(txbuff+txlen,iptparam->IcLockBalance,4);
		}  
		txlen+=4;
		pcd2PcSend(txbuff, txlen);
		iptparam->etc_rec_len=0;
		memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
		
		iptparam->EtcTxCi++;
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=1;
	}
	else if(iptparam->etc_rec_len!=0 && iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_0B)
	{
		if(iptparam->etc_rec_buff[2]==0x00 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
		{
			iptparam->etc_yue_dis_flag=1;
		}
		else if(iptparam->etc_rec_buff[2]!=0x00)
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;	
		}
		else if(iptparam->etc_rec_buff[3]!=iptparam->LogicNozzle)
		{
			iptparam->EtcTxTime=0;
			iptparam->EtcTxFlg=0;	
		}
	}
	else if((iptparam->EtcTxTime>=2*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
	{
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=0;		
	}
	else if((iptparam->EtcTxTime>=2*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
	{
		iptparam->etc_yue_dis_flag=1;
	}
}

//OBU验密
void EtcCardObuPin(unsigned char ID)
{
	unsigned char txbuff[50]={0},dsp_buffer[64]={0};
	unsigned char txlen=0,dsp_len=0;
	unsigned char CardInf[16]={0};
	int i=0,voice[10]={0};
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->EtcTxFlg==0)
	{
		txlen=0;
		txbuff[txlen++]=ETC_CMD;
		txbuff[txlen++]=ETC_08;
		txbuff[txlen++]=iptparam->LogicNozzle;
		memcpy(txbuff+txlen,iptparam->EtcSelCardInf+16,4);
		txlen+=4;

		pcd2PcSend(txbuff, txlen);
		iptparam->etc_rec_len=0;
		memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
		
		iptparam->EtcTxCi++;
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=1;		
	}
	else if(iptparam->etc_rec_len!=0 && iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_08)
	{
		if(iptparam->etc_rec_buff[2]==0x00 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
		{	
			//加油卡判断限车号转入员工密码输入过程，否则转入卡明细查询界面
			if(0!=memcmp(iptparam->IcCarIdLimit, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 16))
			{
				memcpy(CardInf,iptparam->IcCarIdLimit,16);
					for(i=15;i>=0;i--)
					{
						if(CardInf[i]==0xFF)
							CardInf[i]=0x00;
						else
							break;
					}
					
					if(iptparam->EtcOilFlg==1)//ETC加油
					{
						if(memcmp(CardInf,iptparam->EtcSelCardInf,16)==0)//车牌一致直接跳过员工输入密码
						{
//							goto NoCard1;
							//车牌一致强制输入员工密码
							iptparam->etc_limit_car=1;
							goto YG_input2;
						}
						else//报错
						{
							dsp(iptparam->DEVDsp,DSP_CAR_NOYIZHI,dsp_buffer,0);
							IPT_DSP_WAIT();
							iptMainInterface(iptparam);
						}
					}
					else
					{
YG_input2:
						memset(iptparam->SetButton, 0, 16);	iptparam->SetButtonLen=0;
						iptPidSet(iptparam, IPT_PID_IC_STAF_PASSIN);

						memcpy(&dsp_buffer[0], iptparam->IcCarIdLimit, 16);
						dsp_buffer[16]=iptparam->SetButtonLen;	dsp_len=17;
						dsp(iptparam->DEVDsp,DSP_CARD_CARLIMIT, dsp_buffer, dsp_len);
					}
			}
			else
			{
//NoCard1:
				iptparam->IcRecordNumber=0;
//					dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
				iptPidSet(iptparam,IPT_PID_IC_NOTES);
			}	
		}
		else
		{
			if(0==iptparam->IcDefaultPassword)
			{
				memcpy(iptparam->IcPassword, "\x39\x39\x39\x39", 4);	iptparam->IcPasswordLen=4;
				iptPidSet(iptparam,IPT_PID_IC_PIN_CHECK);
			}		
			else
			{
				memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	iptparam->IcPasswordLen=0;
				iptPidSet(iptparam, IPT_PID_IC_PIN_INPUT);

				dsp_buffer[0]=iptparam->IcPasswordLen;	dsp_len=1;
				dsp(iptparam->DEVDsp, DSP_CARD_PASSIN, dsp_buffer, dsp_len);

				//播放语音"请输入卡密码"
				if(iptparam->VoiceVolume>0)
				{
					if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_PASSIN;	/*女声*/
					else																					voice[0]=SPKM_PASSIN;	/*男声*/
					iptSpk(iptparam, voice, 1);
				}

				//促销机通知平板电脑输入卡密码
				if(1 == paramPromotionGet())
				{
					iptparam->TaState = IPT_STATE_CARD_PASSIN;	iptparam->TaStateParamLength = 0;
					*(iptparam->TaStateParam + 0) = iptparam->IcPasswordLen;	
					iptparam->TaStateParamLength = 1;
					pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
				}
			}
		}
			
	}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi<ETC_SEND_TIMES))
	{
		iptparam->EtcTxTime=0;
		iptparam->EtcTxFlg=0;		
	}
	else if((iptparam->EtcTxTime>=5*ONE_SECOND) && (iptparam->EtcTxCi>=ETC_SEND_TIMES))
	{
		if(0==iptparam->IcDefaultPassword)
		{
			memcpy(iptparam->IcPassword, "\x39\x39\x39\x39", 4);	iptparam->IcPasswordLen=4;
			iptPidSet(iptparam,IPT_PID_IC_PIN_CHECK);
		}		
		else
		{
			memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	iptparam->IcPasswordLen=0;
			iptPidSet(iptparam, IPT_PID_IC_PIN_INPUT);

			dsp_buffer[0]=iptparam->IcPasswordLen;	dsp_len=1;
			dsp(iptparam->DEVDsp, DSP_CARD_PASSIN, dsp_buffer, dsp_len);

			//播放语音"请输入卡密码"
			if(iptparam->VoiceVolume>0)
			{
				if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_PASSIN;	/*女声*/
				else																					voice[0]=SPKM_PASSIN;	/*男声*/
				iptSpk(iptparam, voice, 1);
			}

			//促销机通知平板电脑输入卡密码
			if(1 == paramPromotionGet())
			{
				iptparam->TaState = IPT_STATE_CARD_PASSIN;	iptparam->TaStateParamLength = 0;
				*(iptparam->TaStateParam + 0) = iptparam->IcPasswordLen;	
				iptparam->TaStateParamLength = 1;
				pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
			}
		}
	}
}

//释放OBU
void EtcFreeObuCourse(unsigned char ID)
{
	unsigned char txbuff[50]={0};
	unsigned char txlen=0;
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;

	if(iptparam->EtcFreeObuflg==1)
	{
		txlen=0;
		txbuff[txlen++]=ETC_CMD;
		txbuff[txlen++]=ETC_07;
		txbuff[txlen++]=iptparam->LogicNozzle;
		memcpy(txbuff+txlen,iptparam->EtcSelCardInf+16,4);
		txlen+=4;

		//积分应用要强制释放标签
		if(IPT_PAYUNIT_LOYALTY==iptparam->PayUnit)
		{
			iptparam->EtcFreeflag=2;//释放OBU退出车辆列表
			if(iptparam->Id==IPT_NOZZLE_1)
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, &iptparam->EtcFreeflag, 1);
			else
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B, &iptparam->EtcFreeflag, 1);
		}
		
		if(iptparam->EtcFreeflag==1)//OBU返回车辆列表
			txbuff[txlen++]=0x00;
		else if(iptparam->EtcFreeflag==2)//OBU退出车辆列表
			txbuff[txlen++]=0x01;

		pcd2PcSend(txbuff, txlen);
		iptparam->etc_rec_len=0;
		memset(iptparam->etc_rec_buff,0,sizeof(iptparam->etc_rec_buff));
		
		iptparam->EtcFreeObuCi++;
		iptparam->EtcFreeObuTime=0;
		iptparam->EtcFreeObuflg=2;
	}
	else if(iptparam->EtcFreeObuflg==2 && (iptparam->etc_rec_len!=0) && iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_07)
	{
		if(iptparam->etc_rec_buff[2]==0x00 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
		{
			iptparam->EtcFreeObuflg=0;		
			iptparam->EtcFreeflag=0;		
			if(ID==IPT_NOZZLE_1)
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, &iptparam->EtcFreeflag, 1);
			else
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B, &iptparam->EtcFreeflag, 1);		
		}
		else
		{
			iptparam->EtcFreeObuflg=0;
			iptparam->EtcFreeflag=0;	
			if(ID==IPT_NOZZLE_1)
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, &iptparam->EtcFreeflag, 1);
			else
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B, &iptparam->EtcFreeflag, 1);		
		}
			
	}
	else if(iptparam->EtcFreeObuflg==2 && (iptparam->EtcFreeObuTime>=2*ONE_SECOND) && (iptparam->EtcFreeObuCi<ETC_SEND_TIMES))
	{
		iptparam->EtcFreeObuTime=0;
		iptparam->EtcFreeObuflg=1;			
	}
	else if(iptparam->EtcFreeObuflg==2 && (iptparam->EtcFreeObuTime>=2*ONE_SECOND) && (iptparam->EtcFreeObuCi>=ETC_SEND_TIMES))
	{
		iptparam->EtcFreeObuflg=0;
		iptparam->EtcFreeflag=0;	
		if(ID==IPT_NOZZLE_1)
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, &iptparam->EtcFreeflag, 1);
		else
			framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B, &iptparam->EtcFreeflag, 1);
	}
}







