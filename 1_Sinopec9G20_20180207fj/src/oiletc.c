//#include "oilCfg.h"
//#include "oilFram.h"
//#include "oilKb.h"
//#include "oilSpk.h"
//#include "oilDsp.h"
//#include "oilIpt.h"
//#include "oiletc.h"
#include "../inc/main.h"

//ETC���ܴ���
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
				EtcPidCarinfRead(ID);//ETC��ȡ������Ϣ
				break;
			case ETC_PID_SEL_PIN:	
				EtcPinSelPin(ID);//ETC������֤��
				break;
			case ETC_PID_TO_OBU:
				EtcPidToObu(ID);//ETCѡ��OBU
				break;
			case ETC_PID_INF_READ:
				EtcPidInfRead(ID); //ETCһ���Զ�ȡ����Ϣ
				break;
			case ETC_PID_OIL_SURE:
				EtcPidOilSure(ID); //��Ʒ��һ����Ҫȷ��
				break;
			case ETC_PID_PIN_CHECK:
				EtcCardObuPin(ID); //OBU����
				break;
			default:
				break;
		}
}

//ETC��ȡ������Ϣ
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
							dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�޳�����Ϣ", strlen("�޳�����Ϣ"));
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
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�����б���Ϣʧ��", strlen("�����б���Ϣʧ��"));
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
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "�����б���Ϣ��ʱ", strlen("�����б���Ϣ��ʱ"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}

//ETC������֤��
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
							iptparam->EtcFreeflag=1;//ѡ�������ͷ�OBU�����б�
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
				iptMainInterface(iptparam); //�˿����˿������ش�������
				break;
			default:
				break;
		}
}

//ETCѡ��OBU
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
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "OBUѡ��ʧ��", strlen("OBUѡ��ʧ��"));
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
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "OBUѡ����ʱ", strlen("OBUѡ����ʱ"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}

//ETCһ���Զ�ȡ����Ϣ
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
			//��������480
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
								dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " ����Ϣ���ȴ��� ", 16);								
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
								dsp(iptparam->DEVDsp,DSP_TEXT_INFO, " ����Ϣ���ȴ��� ", 16);
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
								dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " ����Ϣ���ȴ��� ", 16);								
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
								dsp(iptparam->DEVDsp,DSP_TEXT_INFO, " ����Ϣ���ȴ��� ", 16);
								IPT_DSP_WAIT();
								iptMainInterface(iptparam);
							}
							else
								memcpy(iptparam->etc_09_buff+iptparam->etc_09_len-(iptparam->etc_rec_len-4),iptparam->etc_rec_buff+4,iptparam->etc_rec_len-4);
						}
					//����Ϣ����
					istate=EtcCardInfJieXi(ID);
					if(istate==1 || istate==2 || istate==3)
						{
							//�������������
							memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
							iptparam->IcPasswordLen=0;

							iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC��Ӧ��Ĭ��Ϊ������Ʊ
							iptparam->DS=IPT_PAYUNIT_MONEY;				  //�ۿ���ԴĬ��Ϊʯ�Ϳ�������Ʊ
							iptparam->Payment=IPT_PAYMENT_MONEY;		//���㷽ʽĬ��Ϊ�ֽ�
							iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //������Ĭ��Ϊʯ���淶��
							iptparam->PriceDiscount = 0;						//�۸��ۿ۶�
							iptparam->UserElecFlag = 0;

							//��ʾ��ת�뿨Ԥ�������
							dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
							iptPidSet(iptparam, IPT_PID_IC_PRETREAT);
							iptparam->IcState.IcTypeS2=IC_CARD_CPU;
							iptparam->Step=1;
							iptparam->etc_touming_flg=0;

							//������֪ͨƽ���ͻ�״̬
							if(1 == paramPromotionGet())
							{
								iptparam->TaState = IPT_STATE_CARD_PRETREAT;	iptparam->TaStateParamLength = 0;
								pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
							}
						}
					else if(istate==0)
						{
							//������֪ͨƽ���ͻ�״̬
							if(1 == paramPromotionGet())
							{
								iptparam->TaState = IPT_STATE_CARD_PRETREAT;	iptparam->TaStateParamLength = 0;
								pcStateUpload(iptparam->TabletPanel, iptparam->LogicNozzle, iptparam->TaState, iptparam->TaStateParam, iptparam->TaStateParamLength);
							}
							
							if(iptparam->etc_card_num==7)//2017.07.25��һ��ʹ��δ����
								{
									//�������������
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC��Ӧ��Ĭ��Ϊ������Ʊ
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //�ۿ���ԴĬ��Ϊʯ�Ϳ�������Ʊ
									iptparam->Payment=IPT_PAYMENT_MONEY;		//���㷽ʽĬ��Ϊ�ֽ�
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //������Ĭ��Ϊʯ���淶��
									iptparam->PriceDiscount = 0;						//�۸��ۿ۶�
									iptparam->UserElecFlag = 0;

									//��ʾ��ת�뿨Ԥ�������
									dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
									iptPidSet(iptparam, IPT_PID_IC_PRETREAT);
									iptparam->IcState.IcTypeS2=IC_CARD_CPU;
									iptparam->Step=1;
									iptparam->etc_touming_flg=0;
								}
								else if(iptparam->etc_card_num==10)
								{
									//�������������
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC��Ӧ��Ĭ��Ϊ������Ʊ
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //�ۿ���ԴĬ��Ϊʯ�Ϳ�������Ʊ
									iptparam->Payment=IPT_PAYMENT_MONEY;		//���㷽ʽĬ��Ϊ�ֽ�
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //������Ĭ��Ϊʯ���淶��
									iptparam->PriceDiscount = 0;						//�۸��ۿ۶�
									iptparam->UserElecFlag = 0;

									//��ʾ��ת�뿨Ԥ�������
									dsp(iptparam->DEVDsp, DSP_CARD_PRETREAT, "\x00", 0);
									iptPidSet(iptparam, IPT_PID_PSAM_PRETREAT);
									iptparam->IcState.IcTypeS2=IC_CARD_CPU;
									iptparam->Step=0;
									iptparam->etc_touming_flg=1;
								}
								else
								{
									dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " ����Ϣ��ʽ���� ", 16);
									IPT_DSP_WAIT();
									//�������������
									memcpy(iptparam->IcPassword, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);	
									iptparam->IcPasswordLen=0;

									iptparam->PayUnit=IPT_PAYUNIT_MONEY;		//IC��Ӧ��Ĭ��Ϊ������Ʊ
									iptparam->DS=IPT_PAYUNIT_MONEY;				  //�ۿ���ԴĬ��Ϊʯ�Ϳ�������Ʊ
									iptparam->Payment=IPT_PAYMENT_MONEY;		//���㷽ʽĬ��Ϊ�ֽ�
									iptparam->C_TYPE=IPT_CARDTYPE_SINO;		  //������Ĭ��Ϊʯ���淶��
									iptparam->PriceDiscount = 0;						//�۸��ۿ۶�
									iptparam->UserElecFlag = 0;

									//��ʾ��ת�뿨Ԥ�������
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
					dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "��ȡ����Ϣʧ��", strlen("��ȡ����Ϣʧ��"));
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
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "��ȡ����Ϣ��ʱ", strlen("��ȡ����Ϣ��ʱ"));
			IPT_DSP_WAIT();
			iptMainInterface(iptparam);
		}
}


//����Ϣ����
char EtcCardInfJieXi(unsigned char ID)
{
	unsigned int len=4,dsp_len=0,i=0,bao_len=0;
	unsigned char dsp_buffer[64]={0};
	IptParamStructType *iptparam=NULL;
	
	if(ID==IPT_NOZZLE_1)
		iptparam=&IptParamA;
	else if(ID==IPT_NOZZLE_2)
		iptparam=&IptParamB;
	
	//����  0����,1APDU״̬����,2ͨ�ų�ʱ,3�����ʽ����
	iptparam->etc_card_num=0;
	//�����1 ѡ��MF�ļ�
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
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, " IC��MFѡ��ʱ ", 16);	
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], " IC��MFѡ��ʧ�� ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
		
	//�����2 ѡ��Ӧ��
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
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC��Ӧ��ѡ��ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC��Ӧ��ѡ��ʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����3 ��ȡ21�ļ�
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
				//���濨��Ϣ
				memcpy(iptparam->IcIssuerMark, iptparam->etc_09_buff+len-bao_len, 8);		  //��������ʶ	
				iptparam->IcAppMatk=iptparam->etc_09_buff[len-bao_len+8];									//Ӧ�����ͱ�ʶ					
				iptparam->IcAppVersion=iptparam->etc_09_buff[len-bao_len+9];							//Ӧ�ð汾
				memcpy(iptparam->IcAppId, iptparam->etc_09_buff+len-bao_len+10, 10);			//Ӧ�����к�
				memcpy(iptparam->IcEnableTime, iptparam->etc_09_buff+len-bao_len+20, 4);	//Ӧ����������
				memcpy(iptparam->IcInvalidTime, iptparam->etc_09_buff+len-bao_len+24, 4);	//Ӧ����Ч��ֹ����
				iptparam->IcCodeVersion=iptparam->etc_09_buff[len-bao_len+28];						//ָ��汾			
				iptparam->IcFile21Unused=0;											                          //21�ļ���������
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC����21�ļ���ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC����21�ļ�ʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����4 ��ȡ22�ļ�
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
				//���濨��Ϣ
				iptparam->IcTypeMark=iptparam->etc_09_buff[len-bao_len];									//�����ͱ�ʶ
				iptparam->IcStaffMark = iptparam->etc_09_buff[len-bao_len+1];						  //��ϵͳְ����ʶ
				memcpy(iptparam->IcUserName, iptparam->etc_09_buff+len-bao_len+2, 20);		//�ֿ�������
				memcpy(iptparam->IcUserIdeId, iptparam->etc_09_buff+len-bao_len+22, 18);	//�ֿ���֤��(identity)����(ASCII)
				iptparam->IcUserIdeType=iptparam->etc_09_buff[len-bao_len+40];						//�ֿ���֤������
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC����22�ļ���ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC����22�ļ�ʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����5 ��ȡ27�ļ�
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
				//���濨��Ϣ
				iptparam->IcDefaultPassword=iptparam->etc_09_buff[len-bao_len];							//�Ƿ����Ĭ������,00=ʹ��Ĭ�����룬01=ʹ���û�����
				iptparam->IcStaffId=iptparam->etc_09_buff[len-bao_len+1];										//Ա����(�ڲ�����Ч)
				memcpy(iptparam->IcStaffPassword, iptparam->etc_09_buff+len-bao_len+2, 2);	//Ա������(�ڲ�����Ч)
				iptparam->IcDebitUnit = iptparam->etc_09_buff[len-bao_len+4];								//�ۿλ(00H=Ԫ��01H=�������վ���ṹ�д��ֶ�)
				iptparam->IcDiscountFlag = iptparam->etc_09_buff[len-bao_len+5];						//�Ƿ��ۿۿ�(00H=���ۿۿ���01H=�ۿۿ������վ���ṹ�д��ֶ�)
				iptparam->IcAppType = iptparam->etc_09_buff[len-bao_len+6];									//��ƬӦ������(���վ���ṹ�д��ֶ�)
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC����27�ļ���ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC����27�ļ�ʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����6 ��ȡ28�ļ�
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
				//���濨��Ϣ
				memcpy(iptparam->IcOilLimit, iptparam->etc_09_buff+len-bao_len, 2);					//��Ʒ����
				iptparam->IcRegionTypeLimit=iptparam->etc_09_buff[len-bao_len+2];						//�޵���,��վ���ͷ�ʽ
				memcpy(iptparam->IcRegionLimit, iptparam->etc_09_buff+len-bao_len+3, 40);		//�޵���,��վ����
				memcpy(iptparam->IcVolumeLimit, iptparam->etc_09_buff+len-bao_len+43, 2);		//��ÿ�μ�����
				iptparam->IcTimesLimit=iptparam->etc_09_buff[len-bao_len+45];								//��ÿ����ʹ���
				memcpy(iptparam->IcMoneyDayLimit, iptparam->etc_09_buff+len-bao_len+46, 4);	//��ÿ����ͽ��
				memcpy(iptparam->IcCarIdLimit, iptparam->etc_09_buff+len-bao_len+50, 16);		//���ƺ�����(ASCII)
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "IC����28�ļ���ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "IC����28�ļ�ʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����7 ���ܽ��
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
				
				return 0;//2017.07.25���ܳ�ʱ�����±�ǩ���뿨��һ�α���ֱ�Ӹ�Ϊ������ֱ����͸������
//				dsp(DSP_TEXT_INFO, "  IC�����ܳ�ʱ  ", 16);
//				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(dsp_buffer+3, "  IC������ʧ��  ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����8 ��ѯ�����׼�¼
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
				//���濨��Ϣ
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
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "����������ϸ��ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "����������ϸʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����9 ��ȡ��������Ϣ
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
				//������ʱ��״̬
				iptparam->ICStateFirst=iptparam->etc_09_buff[len-bao_len];
			
				//���������Ϣ
				iptparam->IcLockMark=iptparam->etc_09_buff[len-bao_len];								  //״̬��:0x00=�޻�����0x01=�ѻ�����0x10=TACδ��
				iptparam->IcLockType=iptparam->etc_09_buff[len-bao_len+1];								//�ϴη�����ۻ�������׵Ľ������ͱ�ʶ
				iptparam->IcLockET=iptparam->etc_09_buff[len-bao_len+2];									//�ϴη�����ۻ��������ΪET
				memcpy(iptparam->IcLockBalance, iptparam->etc_09_buff+len-bao_len+3, 4);	//�ϴη�����ۻ��������Ч���
				memcpy(iptparam->IcLockCTC, iptparam->etc_09_buff+len-bao_len+7, 2);			//�ϴη�����ۻ�����Ľ������
				memcpy(iptparam->IcLockTermId, iptparam->etc_09_buff+len-bao_len+9, 6);		//�ϴη�����ۻ�������ն˱��
				memcpy(iptparam->IcLockTime, iptparam->etc_09_buff+len-bao_len+15, 7);		//�ϴη�����ۻ����������ʱ��
				memcpy(iptparam->IcLockMoney, iptparam->etc_09_buff+len-bao_len+22, 4);		//�ϴη�����ۻ�����Ľ��׽��
				memcpy(iptparam->IcLockGTAC, iptparam->etc_09_buff+len-bao_len+26, 4);		//�ϴη�����ۻ������GTAC��TAC��MAC3
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\x69\x85",2)==0)
			{
				iptparam->IcLockMark=0;																			     //״̬��:0x00=�޻�����0x01=�ѻ�����0x10=TACδ��
				iptparam->IcLockType=0;																			     //�ϴη�����ۻ�������׵Ľ������ͱ�ʶ
				iptparam->IcLockET=0;																				     //�ϴη�����ۻ��������ΪET
				memcpy(iptparam->IcLockBalance, "\x00\x00\x00\x00", 4);				   //�ϴη�����ۻ��������Ч���
				memcpy(iptparam->IcLockCTC, "\x00\x00", 2);										   //�ϴη�����ۻ�����Ľ������
				memcpy(iptparam->IcLockTermId, "\x00\x00\x00\x00\x00\x00", 6);   //�ϴη�����ۻ�����ձ��
				memcpy(iptparam->IcLockTime, "\x00\x00\x00\x00\x00\x00\x00", 7); //�ϴη�����ۻ����������ʱ��
				memcpy(iptparam->IcLockMoney, "\x00\x00\x00\x00", 4);						 //�ϴη�����ۻ�����Ľ��׽��
				memcpy(iptparam->IcLockGTAC, "\x00\x00\x00\x00", 4);						 //�ϴη�����ۻ������GTAC��TAC��MAC3
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "����������Ϣ��ʱ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "����������Ϣʧ��", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);		
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	//�����10 ��ȡ�����
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
				//���Ѷ������
				iptparam->IcValid=1;

				//���㷽ʽĬ��Ϊ�ֽ�
				iptparam->Payment=IPT_PAYMENT_MONEY;

				//�������Ϣ
				memcpy(iptparam->IcBalance, iptparam->etc_09_buff+len-bao_len, 4);
			}
			else if(memcmp(iptparam->etc_09_buff+len-2,"\xFF\xFF",2)==0)
			{
				dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "  ������ʱ  ", 16);
				IPT_DSP_WAIT();
				return 2;
			}
			else
			{
				memcpy(&dsp_buffer[0], iptparam->etc_09_buff+len-2, 2);
				dsp_buffer[2]=16;
				memcpy(&dsp_buffer[3], "  �������ʧ��  ", 16);
				dsp_len=dsp_buffer[2]+3;
				dsp(iptparam->DEVDsp, DSP_CARD_ERR_INFO, dsp_buffer,dsp_len);	
				IPT_DSP_WAIT();
				return 1;
			}
		}
		else
		{
			dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ȴ���   ", 16);	
			IPT_DSP_WAIT();
			return 3;
		}
	}
	else
	{
		dsp(iptparam->DEVDsp, DSP_TEXT_INFO, "   �����ݴ���   ", 16);	
		IPT_DSP_WAIT();
		return 3;
	}
	
	return 0;
}

//���ƺŴ���
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

	//���Ϳ��ж��޳���ת��Ա������������̣�����ת�뿨��ϸ��ѯ����
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
		
		if(iptparam->EtcOilFlg==1)//ETC����
		{
			if(memcmp(CardInf,iptparam->EtcSelCardInf,16)==0)//����һ��ֱ������Ա����������
			{
				iptparam->etc_limit_car=1;
//				goto NoYiZhi;
				//����һ��ǿ������Ա������
				goto YG_input1;

			}
			else//����
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

//��Ʒ������ʾ����
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
	if(iptparam->voiceParam.voiceOil==0)//���̨�Զ�ƥ��
	{
		oilCode=(iptparam->OilCode[0]<<8)+(iptparam->OilCode[1]<<0);
		oilCodeConvertVoice(oilCode);
	}
#endif
	if(((oilCode>=0x0107)&&(oilCode<=0x0111)) ||((oilCode>=0x0307)&&(oilCode<=0x0311)))
		memcpy(iptparam->etc_now_oil,"89#����",strlen("89#����"));
	else if(((oilCode>=0x0011)&&(oilCode<=0x0013))||(oilCode==0x0112) ||((oilCode>=0x0211)&&(oilCode<=0x0213)) ||(oilCode==0x0312))
		memcpy(iptparam->etc_now_oil,"90#����",strlen("90#����"));
	else if(((oilCode>=0x0103)&&(oilCode<=0x0106))||(oilCode==0x0113) ||((oilCode>=0x0303)&&(oilCode<=0x0306)) ||(oilCode==0x0313))
		memcpy(iptparam->etc_now_oil,"92#����",strlen("92#����"));
	else if(((oilCode>=0x0014)&&(oilCode<=0x0017))||(oilCode==0x0114) ||((oilCode>=0x0214)&&(oilCode<=0x0217)) ||(oilCode==0x0314))
		memcpy(iptparam->etc_now_oil,"93#����",strlen("93#����"));
	else if(((oilCode>=0x0018)&&(oilCode<=0x0021))||(oilCode==0x0115) ||((oilCode>=0x0218)&&(oilCode<=0x0221)) ||(oilCode==0x0315))
		memcpy(iptparam->etc_now_oil,"95#����",strlen("95#����"));
	else if(((oilCode>=0x0022)&&(oilCode<=0x0025))||(oilCode==0x0116) ||((oilCode>=0x0222)&&(oilCode<=0x0225)) ||(oilCode==0x0316))
		memcpy(iptparam->etc_now_oil,"97#����",strlen("97#����"));
	else if(((oilCode>=0x0026)&&(oilCode<=0x0028))||(oilCode==0x0117) ||((oilCode>=0x0226)&&(oilCode<=0x0228)) ||(oilCode==0x0317))
		memcpy(iptparam->etc_now_oil,"98#����",strlen("98#����"));
	else if(oilCode==0x0029||oilCode==0x0229)
		memcpy(iptparam->etc_now_oil,"120#����",strlen("120#����"));
	else if(oilCode==0x0038 || oilCode==0x0123||oilCode==0x0238 || oilCode==0x0331)
		memcpy(iptparam->etc_now_oil,"0#����",strlen("0#����"));
	else if(oilCode==0x0047||oilCode==0x0247)
		memcpy(iptparam->etc_now_oil,"+5#����",strlen("+5#����"));
	else if(oilCode==0x0048||oilCode==0x0248)
		memcpy(iptparam->etc_now_oil,"+10#����",strlen("+10#����"));
	else if(oilCode==0x0049||oilCode==0x0249)
		memcpy(iptparam->etc_now_oil,"+15#����",strlen("+15#����"));
	else if(oilCode==0x0050||oilCode==0x0250)
		memcpy(iptparam->etc_now_oil,"+20#����",strlen("+20#����"));
	else if(oilCode==0x0039||oilCode==0x0239)
		memcpy(iptparam->etc_now_oil,"-5#����",strlen("-5#����"));
	else if(oilCode==0x0040||oilCode==0x0240)
		memcpy(iptparam->etc_now_oil,"-10#����",strlen("-10#����"));
	else if(oilCode==0x0043||oilCode==0x0243)
		memcpy(iptparam->etc_now_oil,"-20#����",strlen("-20#����"));
	else if(oilCode==0x0044||oilCode==0x0244)
		memcpy(iptparam->etc_now_oil,"-30#����",strlen("-30#����"));
	else if(oilCode==0x0045||oilCode==0x0245)
		memcpy(iptparam->etc_now_oil,"-35#����",strlen("-35#����"));
	else if(oilCode==0x0046||oilCode==0x0246)
		memcpy(iptparam->etc_now_oil,"-50#����",strlen("-50#����"));
	
	if((iptparam->EtcSelCardInf[ETCCARDLEN-3]==0 || iptparam->EtcSelCardInf[ETCCARDLEN-3]==0x30) && (iptparam->EtcSelCardInf[ETCCARDLEN-2]!=0))
		sprintf((char *)iptparam->etc_recnet_oil,"%d#����",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	else if((iptparam->EtcSelCardInf[ETCCARDLEN-3]==0 || iptparam->EtcSelCardInf[ETCCARDLEN-3]==0x30) && (iptparam->EtcSelCardInf[ETCCARDLEN-2]==0))/*0�Ų���*/
		sprintf((char *)iptparam->etc_recnet_oil,"%d#����",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	else if(iptparam->EtcSelCardInf[ETCCARDLEN-3]=='-' || iptparam->EtcSelCardInf[ETCCARDLEN-3]=='+')
	{
		iptparam->etc_recnet_oil[0]=iptparam->EtcSelCardInf[ETCCARDLEN-3];
		sprintf((char *)iptparam->etc_recnet_oil+1,"%d#����",iptparam->EtcSelCardInf[ETCCARDLEN-2]);
	}
}

//ETC����
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
			memcmp(iptparam->etc_now_oil,iptparam->etc_recnet_oil,10)==0)//��δ�ӹ��ͻ�����Ʒ����ͬ
		{	
			iptparam->IcValid=1; //���Ѷ������
			iptparam->Payment=IPT_PAYMENT_MONEY; //���㷽ʽĬ��Ϊ�ֽ�

			//��ʼ��Ԥ������
			memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));		iptparam->IntegerLen=0;
			iptparam->Point=0;
			memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
			iptparam->PresetMode=IPT_PRESET_NO;	iptparam->PresetVolume=0;	iptparam->PresetMoney=0;
		
			memset(iptparam->SetButton,0,sizeof(iptparam->SetButton));
			iptparam->SetButtonLen=0;
			if(iptparam->etc_limit_car==1)
				{
					//��ǹ��ʹ���޳��ſ�����
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
					//����ǹ���ͻ��Զ�����ʽ����
					if(iptparam->VoiceVolume > 0 && IPT_PAYUNIT_LOYALTY != iptparam->PayUnit)
						{
							if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_OILPLEASE;	/*Ů��*/
							else												voice[0]=SPKM_OILPLEASE;	/*����*/
							iptSpk(iptparam, voice, 1);
						}
				}
			iptIcBalanceDsp(iptparam->Id);
			iptPidSet(iptparam, IPT_PID_IC_BALANCE);

			//������֪ͨƽ���ͻ�״̬
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
		//�ҿ���ת��Ҽ�¼��ѯ����
		dsp(iptparam->DEVDsp, DSP_CARD_UNLOCK_FINISH, "\x00", 0);
		iptPidSet(iptparam, IPT_PID_IC_LOCKRECORD);
	}
	else if(0x10==iptparam->IcLockMark)
	{
		//ת��TAC�������
		dsp(iptparam->DEVDsp,DSP_TEXT_INFO, "  �����䴦����  ", strlen("  �����䴦����  "));
		iptPidSet(iptparam,IPT_PID_IC_TAC_CLEAR);
	}
	else
	{
		dsp(iptparam->DEVDsp,DSP_TEXT_INFO, "  ����״̬�ִ�  ", 16);
		IPT_DSP_WAIT();
		iptMainInterface(iptparam);
	}
}

//��Ʒ��һ����Ҫȷ��
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
		iptparam->IcValid=1; //���Ѷ������
		iptparam->Payment=IPT_PAYMENT_MONEY; //���㷽ʽĬ��Ϊ�ֽ�

		//��ʼ��Ԥ������
		memset(iptparam->IntegerBuffer, 0, sizeof(iptparam->IntegerBuffer));		iptparam->IntegerLen=0;
		iptparam->Point=0;
		memset(iptparam->DecimalBuffer, 0, sizeof(iptparam->DecimalBuffer));	iptparam->DecimalLen=0;
		iptparam->PresetMode=IPT_PRESET_NO;	iptparam->PresetVolume=0;	iptparam->PresetMoney=0;
	
		memset(iptparam->SetButton,0,sizeof(iptparam->SetButton));
		iptparam->SetButtonLen=0;
		if(iptparam->etc_limit_car==1)
			{
				//��ǹ��ʹ���޳��ſ�����
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
				//����ǹ���ͻ��Զ�����ʽ����
				if(iptparam->VoiceVolume > 0 && IPT_PAYUNIT_LOYALTY != iptparam->PayUnit)
					{
						if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	
							voice[0]=SPKW_OILPLEASE;	//Ů��
						else
							voice[0]=SPKM_OILPLEASE;	//����
						iptSpk(iptparam, voice, 1);
					}
			}

		iptIcBalanceDsp(iptparam->Id);
		iptPidSet(iptparam, IPT_PID_IC_BALANCE);

		//������֪ͨƽ���ͻ�״̬
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

//������Ʒ��
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
		if(iptparam->etc_now_oil[0]==0x30)/*0#����*/
		 memcpy(txbuff+txlen,"\x30\x00",2);
		else if((iptparam->etc_now_oil[0]>=0x31) && (iptparam->etc_now_oil[0]<=0x39))/*����*/
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

//��ʾ���
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

//OBU����
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
			//���Ϳ��ж��޳���ת��Ա������������̣�����ת�뿨��ϸ��ѯ����
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
					
					if(iptparam->EtcOilFlg==1)//ETC����
					{
						if(memcmp(CardInf,iptparam->EtcSelCardInf,16)==0)//����һ��ֱ������Ա����������
						{
//							goto NoCard1;
							//����һ��ǿ������Ա������
							iptparam->etc_limit_car=1;
							goto YG_input2;
						}
						else//����
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

				//��������"�����뿨����"
				if(iptparam->VoiceVolume>0)
				{
					if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_PASSIN;	/*Ů��*/
					else																					voice[0]=SPKM_PASSIN;	/*����*/
					iptSpk(iptparam, voice, 1);
				}

				//������֪ͨƽ��������뿨����
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

			//��������"�����뿨����"
			if(iptparam->VoiceVolume>0)
			{
				if(IPT_VOICE_TYPE_WOMAN==iptparam->VoiceType)	voice[0]=SPKW_PASSIN;	/*Ů��*/
				else																					voice[0]=SPKM_PASSIN;	/*����*/
				iptSpk(iptparam, voice, 1);
			}

			//������֪ͨƽ��������뿨����
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

//�ͷ�OBU
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

		//����Ӧ��Ҫǿ���ͷű�ǩ
		if(IPT_PAYUNIT_LOYALTY==iptparam->PayUnit)
		{
			iptparam->EtcFreeflag=2;//�ͷ�OBU�˳������б�
			if(iptparam->Id==IPT_NOZZLE_1)
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_A, &iptparam->EtcFreeflag, 1);
			else
				framWrite(FM_ADDR_IPT_SINO, iptparam->FMAddrBase+FM_ETC_FREE_FLG_B, &iptparam->EtcFreeflag, 1);
		}
		
		if(iptparam->EtcFreeflag==1)//OBU���س����б�
			txbuff[txlen++]=0x00;
		else if(iptparam->EtcFreeflag==2)//OBU�˳������б�
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







