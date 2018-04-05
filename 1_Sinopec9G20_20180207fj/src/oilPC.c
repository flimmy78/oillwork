//#include <VxWorks.h>
//#include <semLib.h>
//#include <inetLib.h>
//#include "oilCfg.h"
//#include "oilLog.h"
//#include "oilCom.h"
//#include "oilParam.h"
//#include "oilIpt.h"
//#include "oilPC.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "../inc/main.h"


/*ƽ��ͨѶʱ��ʹ�û���Ĵ�С*/
#define PC_DATA_MAX_SIZE		1024

/*�������ͺ� 0=�Ǵ�����(��ͨ����)��1=���浥ǹ��2=����˫ǹ��*/
int pcModel = ERROR;

/*������ƽ�����ͨѶ���ȹز������ݽṹ*/
struct PCStruct
{
	int Comx;												/*������ƽ�����ͨѶ�Ĵ��ں�*/
	//SEM_ID SemID;									        /*��ƽ�����ͨѶ�Ĳ��������ź���*/
    pthread_mutex_t SemID;	

	unsigned char Panel;								   /*ƽ��������� PC_PANEL_1/PC_PANEL_2*/
	unsigned char UserID;							       /*ʹ�ô�ƽ����Ե��û�ID��0��ʾ����*/
	unsigned char TradeNO;						           /*ͨѶʱ�����*/
	unsigned char Nozzle;							       /*��Ӧ����ǹǹ��*/

	//MSG_Q_ID cmdBuffer;							        /*ƽ������������͵���������*/
	//MSG_Q_ID TxBuffer;							        /*�ͻ��������͵���������*/
	
	int cmdBuffer;
	int TxBuffer;
	
	char ackBuffer[PC_DATA_MAX_SIZE+10];	                /*���ͻ��������������ƽ����Է��ص�����*/
	int ackLength;												/*���ͻ��������������ƽ����Է��ص����ݳ���*/

	PCOilInfoType OilInfo;							/*���ͻ���Ϣ*/

	PCCardInfoType CardInfo;					/*����Ϣ*/

	unsigned char CardAppSelect;				/*Ӧ��ѡ�� 0=������Ʊ��1=����Ӧ��*/
	unsigned char CardRecordNumber;		/*��������Ϣ����*/
	PCCardRecordType CardRecord[10];	/*��������Ϣ*/
};

pthread_mutex_t g_mutexLockPc = PTHREAD_MUTEX_INITIALIZER;

struct PCStruct PC_1;
struct PCStruct PC_2;

/*�ڲ���������*/
int  pcAckSend(int panel, char command, char tradeno, char *inbuffer, int nbytes);
int  pcSend(int panel, char command, char *inbuffer, int nbytes, char *outbuffer, int maxbytes);



/********************************************************************
*Name				:tPcReceive
*Description		:���ղ�����ƽ����Ե�����
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:��
*Return				:��
*History			:2015-12-23,modified by syj
*/
void tPcReceive(int panel)
{
	char rdBuffer[64] = {0};
	int rdLength = 0;
	
	char rx_buffer[PC_DATA_MAX_SIZE + 1] = {0};
	struct msg_struct msg_st;
	msg_st.msgType = 1;
	int rx_len = 0;
	
	int i = 0, dataLength = 0, dataCrc = 0, crcValue = 0, iMark = 0, j = 0;
	struct PCStruct *myPC = NULL;

	if(PC_PANEL_1 == panel)		
	{
		myPC = &PC_1;
        prctl(PR_SET_NAME,(unsigned long)"tPcReceiveA");
	}
	else if(PC_PANEL_2 == panel)	
	{
		myPC = &PC_2;
        prctl(PR_SET_NAME,(unsigned long)"tPcReceiveB");
	}
	else
	{
		jljRunLog("Function \"%s\" panel[%x] is invalid!\n", __FUNCTION__, panel);	
		return;
	}
	

	FOREVER
	{	
		rdLength = comRead(myPC->Comx, rdBuffer, 64);//��������ƽ����ԵĴ�������
		if(rdLength <= 0)
		{
			//taskDelay(1);
			usleep(10000);
			continue;
		}

		for(i = 0; i < rdLength; i++)
		{	
			if(rx_len >= PC_DATA_MAX_SIZE)//��ֹ�������
			{
				rx_len = 0;
			}

			//	�������Ϊ0�����յ����ֽ�Ϊ0xfa��Ϊ����ͷ����0xfa����Ϊ�������󣬲��洢��
			//	������ȷ�0(���ѽ�������)��������Ϊ0xfaʱ�ж���һ�����Ƿ�Ϊ0xfa���������Ϊת���ַ���������Ϊǰһ��0xfaΪ����ͷ��
			if(0 == rx_len && 0xfa == rdBuffer[i])
			{
				rx_buffer[rx_len++] = rdBuffer[i];
			}
			else if(0 == rx_len && 0xfa != rdBuffer[i])
			{
			}
			else if(0 != rx_len && 0 == iMark)
			{
				rx_buffer[rx_len++] = rdBuffer[i];
				if(0xfa == rdBuffer[i])	
					iMark = 1;
			}
			else if(0 != rx_len && 0 != iMark && 0xfa == rdBuffer[i])
			{
				iMark = 0;
			}
			else if(0 != rx_len && 0 != iMark && 0xfa != rdBuffer[i])
			{
				rx_buffer[rx_len++] = 0xfa;
				rx_buffer[rx_len++] = rdBuffer[i];
				iMark = 0;
			}

			//�ж��Ƿ���յ��������ȵ�һ֡����
			dataLength = ((rx_buffer[1]>>4)&0x0f)*1000 + ((rx_buffer[1]>>0)&0x0f)*100 +	((rx_buffer[2]>>4)&0x0f)*10 + ((rx_buffer[2]>>0)&0x0f)*1;
			if(!(rx_len >= 10 && rx_len >= dataLength + 5))
			{
				continue;
			}

			//У�鲢�������ݣ�ƽ����Է��͵��������ݷ��͸��䴦�����񣬱������ص����ݴ��뷵�����ݻ���
			crcValue = crc16Get(rx_buffer + 1, dataLength + 2);
			dataCrc = ((*(rx_buffer + dataLength + 3 + 0))<<8)|((*(rx_buffer + dataLength + 3 + 1))<<0);
			if(crcValue == dataCrc)
			{
                memcpy(msg_st.msgBuffer,rx_buffer,rx_len);
				//msgQSend(myPC->cmdBuffer, rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);
				msgsnd(myPC->cmdBuffer,&msg_st,rx_len,IPC_NOWAIT);
			}
			if(crcValue == dataCrc && 0 == myPC->ackLength)
			{
				memcpy(myPC->ackBuffer, rx_buffer, rx_len);
				myPC->ackLength = rx_len;
			}

			rx_len = 0;
		}

		//taskDelay(1);
		usleep(10000);
	}

	return;
}


/********************************************************************
*Name				:tPcCommandProcess
*Description		:����ƽ������������͵�����
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:��
*Return				:��
*History			:2015-12-23,modified by syj
*/
void tPcCommandProcess(int panel)
{
	char rdBuffer[PC_DATA_MAX_SIZE] = {0};
	struct msg_struct msg_st;
	msg_st.msgType = 0;
	int rdLength = 0;

	char tx_buffer[512] = {0};
	int tx_len = 0;

	int command = 0, tradeno = 0, ipanel = 0;
	int datatype = 0, idata = 0, istate = 0, i = 0;
	int istate2 = 0;
	struct PCStruct *myPC = NULL;
	struct PCStruct *tmpPC = NULL;

	if(PC_PANEL_1 == panel)		
	{
		myPC = &PC_1;
		prctl(PR_SET_NAME,(unsigned long)"tPcCommandProcA");
	}
	else if(PC_PANEL_2 == panel)	
	{
		myPC = &PC_2;
	    prctl(PR_SET_NAME,(unsigned long)"tPcCommandProcB");
	}
	else			
	{
		jljRunLog("Function \"%s\" panel[%x] is invalid!\n", __FUNCTION__, panel);	
		return;
	}

	FOREVER
	{
		/*��ȡƽ����Է��͵�������������*/
		memset(rdBuffer, 0, PC_DATA_MAX_SIZE);	rdLength = 0;
		memset(msg_st.msgBuffer,0,PC_DATA_MAX_SIZE);
		//if(NULL != myPC->cmdBuffer)
		//{
		//	rdLength = msgQReceive(myPC->cmdBuffer, rdBuffer, PC_DATA_MAX_SIZE, WAIT_FOREVER);
		//}
		if(myPC->cmdBuffer != ERROR)
		{
			rdLength = msgrcv(myPC->cmdBuffer,&msg_st,PC_DATA_MAX_SIZE,0,IPC_NOWAIT);
			if(rdLength > 0)
			{
				memcpy(rdBuffer,msg_st.msgBuffer,rdLength);
				printf("tPcCommandProcess. rdLength = %d\n",rdLength);
			}
		}
		if(rdLength <= 0)
		{
			//taskDelay(1);
			usleep(1000);
			continue;
		}

		//���������ִ���ƽ����Է��͹��������ݣ����к�ҲӦ��ƽ�����һ��
		memset(tx_buffer, 0, sizeof(tx_buffer));	
		tx_len = 0;
		command = *(rdBuffer + 7);	
		tradeno = *(rdBuffer + 3);
		ipanel = *(rdBuffer + 4);
		switch(command)
		{
		case PC_CMD_SHAKEHANDS:	//ƽ���������ͻ����͵���������	
			*(tx_buffer + tx_len++) = 0;						//errcode
			if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
			{
				*(tx_buffer + tx_len++) = 0x03;
				*(tx_buffer + tx_len++) = 0;					    //1����
				myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
				*(tx_buffer + tx_len++) = myPC->Nozzle;				//1����Ӧǹ��
				memcpy(tx_buffer + tx_len, PC_SOFTVERSION, 2);	    //Э��汾��
				tx_len += 2;
				memcpy(tx_buffer + tx_len, PC_OIL_SOFTVERSION, 3);	//�ͻ�����汾��
				tx_len += 3;
				pcAckSend(myPC->Panel, PC_CMD_SHAKEHANDS, tradeno, tx_buffer, tx_len);
			}
			else
			{
				*(tx_buffer + tx_len++) = 0x04;
				*(tx_buffer + tx_len++) = PC_PANEL_1;				//1����
				PC_1.Nozzle = iptPhysicalNozzleRead(PC_1.Panel);
				*(tx_buffer + tx_len++) = PC_1.Nozzle;				//1����Ӧǹ��
				*(tx_buffer + tx_len++) = PC_PANEL_2;				//2����
				PC_2.Nozzle = iptPhysicalNozzleRead(PC_2.Panel);
				*(tx_buffer + tx_len++) = PC_2.Nozzle;				//2����Ӧǹ��
				memcpy(tx_buffer + tx_len, PC_SOFTVERSION, 2);	    //Э��汾��
				tx_len += 2;
				memcpy(tx_buffer + tx_len, PC_OIL_SOFTVERSION, 3);	//�ͻ�����汾��
				tx_len += 3;
				pcAckSend(0, PC_CMD_SHAKEHANDS, tradeno, tx_buffer, tx_len);
			}
			break;
			
		case PC_CMD_LIGHT_CONTROL://ƽ���������ͻ����͵��Ľ��ƿ��������ӡ�����������֧����������,�����Ľ��ƣ����浥ǹʱ�ֱ���ƣ�����˫ǹʱ��ͬ����A����Ʒ��ť��
			if(0 == *(rdBuffer + 8))
			{
				if(0 == *(rdBuffer + 9) && PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())			
					YPLightTurnOff(myPC->Panel, 0);
				else if(0 == *(rdBuffer + 9) && PANEL_NOZZLE_SINGLE != paramPanelNozzleGet())
					YPLightTurnOff(0, 0);
				else if(0 != *(rdBuffer + 9) && PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					YPLightTurnOn(myPC->Panel, 0);
				else if(0 != *(rdBuffer + 9) && PANEL_NOZZLE_SINGLE != paramPanelNozzleGet())	
					YPLightTurnOn(0, 0);

				*(tx_buffer + tx_len++) = *(rdBuffer + 8);	//handle1
				*(tx_buffer + tx_len++) = 0;				//errcode
				pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(1 == *(rdBuffer + 8))//��ӡ���ݣ����浥ǹʱ�ֱ��ӡ������˫ǹʱ��ͬʹ��A���ӡ����ӡ
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pprint(iptPrinterRead(myPC->Panel), rdBuffer + 9, rdLength - 11);
				else
					pprint(iptPrinterRead(0), rdBuffer + 9, rdLength - 11);

				*(tx_buffer + tx_len++) = *(rdBuffer + 8);	//handle1
				*(tx_buffer + tx_len++) = 0;				//errcode
				pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(2 == *(rdBuffer + 8))//����/�˳�֧�����̲���
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate = iptCardPayForAsk(myPC->Panel, rdBuffer + 9);
				else								
					istate = iptCardPayForAsk(ipanel, rdBuffer + 9);

				*(tx_buffer + tx_len++) = *(rdBuffer + 8);	//handle1
				*(tx_buffer + tx_len++) = istate ;			//errcode
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(3 == *(rdBuffer + 8))//�������ѿۿ�
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate = iptCardPayFor(myPC->Panel, rdBuffer + 9);
				else												
					istate = iptCardPayFor(ipanel, rdBuffer + 9);

				break;
			}	
			else if(4 == *(rdBuffer + 8))//��ͣ
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate = iptExterSuspendDo(myPC->Panel, 1, rdBuffer + 9);
				else																					
				{
					istate = iptExterSuspendDo(0, 1, rdBuffer + 9);
					istate = iptExterSuspendDo(1, 1, rdBuffer + 9);
				}

				*(tx_buffer + tx_len++) = *(rdBuffer + 8);	//handle1
				*(tx_buffer + tx_len++) = istate ;			//errcode
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			}	
			else if(5 == *(rdBuffer + 8))	//�ָ�
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate = iptExterSuspendDo(myPC->Panel, 0, rdBuffer + 9);
				else																					
				{
					istate = iptExterSuspendDo(0, 0, rdBuffer + 9);
					istate = iptExterSuspendDo(1, 0, rdBuffer + 9);
				}
				
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);	//handle1
				*(tx_buffer + tx_len++) = istate ;			//errcode
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			}	
			else if(6 == *(rdBuffer + 8))//6	֪ͨ�ͻ���ʼ��������
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthETCPassInput(myPC->Panel, 1);
				else	
					istate2 = iptAuthETCPassInput(ipanel, 1);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	
					*(tx_buffer + tx_len++) = 0;	//errcode
				else	
					*(tx_buffer + tx_len++) = 1;	//errcode
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}
			else if(7 == *(rdBuffer + 8)) //7	֪ͨ�ͻ�ȡ����������
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthETCPassInput(myPC->Panel, 2);
				else	
					istate2 = iptAuthETCPassInput(ipanel, 2);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)		//errcode
					*(tx_buffer + tx_len++) = 0;
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(8 == *(rdBuffer + 8))//8	����ͻ���Ȩ
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptAuthorize(myPC->Panel, 0, rdBuffer + 9);
				else
					istate2 = iptAuthorize(ipanel, 0, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	                        	//errcode
					*(tx_buffer + tx_len++) = 0;
				else	
					*(tx_buffer + tx_len++) = 1;		
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(9 == *(rdBuffer + 8))//9	ȡ�����ͻ���Ȩ
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthorize(myPC->Panel, 1, rdBuffer + 9);
				else	
					istate2 = iptAuthorize(ipanel, 1, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)                                //errcode
					*(tx_buffer + tx_len++) = 0;	
				else	
					*(tx_buffer + tx_len++) = 1;

				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0A == *(rdBuffer + 8))//֪ͨ�ͻ�Ԥ����
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	 
					istate2 = iptExterPreset(myPC->Panel, rdBuffer + 9);
				else	
					istate2 = iptExterPreset(ipanel, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	//errcode
					*(tx_buffer + tx_len++) = 0;
				else		
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0B == *(rdBuffer + 8))//֪ͨ�ͻ����Ϳ�Ӧ��ѡ��
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterCardAppSet(myPC->Panel, rdBuffer + 9);
				else	
					istate2 = iptExterCardAppSet(ipanel, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)                                //errcode	
					*(tx_buffer + tx_len++) = 0;	
				else
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0C == *(rdBuffer + 8))//֪ͨ�ͻ����Ϳ�֧����ʽѡ��
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterCardPaymentSet(myPC->Panel, rdBuffer + 9);
				else
					istate2 = iptExterCardPaymentSet(ipanel, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	                            //errcode
					*(tx_buffer + tx_len++) = 0;	
				else
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0D == *(rdBuffer + 8))	//֪ͨ�ͻ����Ϳ��˿�
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterCardShoot(myPC->Panel);
				else	
					istate2 = iptExterCardShoot(ipanel);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)		                        //errcode
					*(tx_buffer + tx_len++) = 0;
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0E == *(rdBuffer + 8))//֪ͨ�ͻ���������ɨ��
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	 
					istate2 = iptExterScanHandle(myPC->Panel, 0);
				else	
					istate2 = iptExterScanHandle(ipanel, 0);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)                             	//errcode
					*(tx_buffer + tx_len++) = 0;	  
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x0F == *(rdBuffer + 8))//֪ͨ�ͻ��˳�����ɨ��
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterScanHandle(myPC->Panel, 1);
				else	
					istate2 = iptExterScanHandle(ipanel, 1);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	                            //errcode
					*(tx_buffer + tx_len++) = 0;	
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x10 == *(rdBuffer + 8))//֪ͨ�ͻ�ɨ������������
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterBarcodeInput(myPC->Panel, 0, rdBuffer + 9);
				else	
					istate2 = iptExterBarcodeInput(ipanel, 0, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	
					*(tx_buffer + tx_len++) = 0;	//errcode
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}	
			else if(0x11 == *(rdBuffer + 8))	//֪ͨ�ͻ�ɨ���������������
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterBarcodeInput(myPC->Panel, 1, rdBuffer + 9);
				else
					istate2 = iptExterBarcodeInput(ipanel, 1, rdBuffer + 9);

				//�Ƿ����������ʧ��
				*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
				if(0 == istate2)	
					*(tx_buffer + tx_len++) = 0;	//errcode
				else	
					*(tx_buffer + tx_len++) = 1;	
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
				break;
			}
			
			//�Ƿ����������ʧ��
			*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
			*(tx_buffer + tx_len++) = 1;					//errcode
			if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
				pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			else	
				pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			break;
		case PC_CMD_GET_CARDINFO:	//ƽ���������ͻ����͵Ķ�ȡ��Ƭ��Ϣ�������׼�¼	
			datatype = *(rdBuffer + 8);
			if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
			{
				tmpPC = myPC;
			}
			else
			{
				if(PC_PANEL_1 == ipanel)	
					tmpPC = &PC_1;
				else
					tmpPC = &PC_2;
			}

			if(0 == datatype)	//����Ƭ��Ϣ
			{
				//taskLock(); //fj:
				pthread_mutex_lock(&g_mutexLockPc);
				*(tx_buffer + tx_len++) = 0;
				*(tx_buffer + tx_len++) = tmpPC->CardInfo.IcAppSelect;
				*(tx_buffer + tx_len++) = datatype;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcAppId, 10);		
				tx_len += 10;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcEnableTime, 4);			
				tx_len += 4;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcInvalidTime, 4);
				tx_len += 4;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcUserName, 20);
				tx_len += 20;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcUserIdeId, 18);
				tx_len += 18;
				memcpy(tx_buffer + tx_len, &(tmpPC->CardInfo.IcUserIdeType), 1);
				tx_len += 1;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcOilLimit, 2);
				tx_len += 2;
				memcpy(tx_buffer + tx_len, &(tmpPC->CardInfo.IcRegionTypeLimit), 1);	
				tx_len += 1;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcRegionLimit, 40);	
				tx_len += 40;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcVolumeLimit, 2);	
				tx_len += 2;
				memcpy(tx_buffer + tx_len, &(tmpPC->CardInfo.IcTimesLimit), 1);
				tx_len += 1;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcMoneyDayLimit, 4);	
				tx_len += 4;
				memcpy(tx_buffer + tx_len, tmpPC->CardInfo.IcCarIdLimit, 16);			
				tx_len += 16;
				memset(tx_buffer + tx_len, 0, 25);															
				tx_len += 25;
				pthread_mutex_unlock(&g_mutexLockPc);

				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					pcAckSend(myPC->Panel, PC_CMD_GET_CARDINFO, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_GET_CARDINFO, tradeno, tx_buffer, tx_len);
				break;
			}
			
			if(1 == datatype)//����������Ϣ
			{
				//taskLock();//fj:
				pthread_mutex_lock(&g_mutexLockPc);
				*(tx_buffer + tx_len++) = 0;
				*(tx_buffer + tx_len++) = tmpPC->CardAppSelect;
				*(tx_buffer + tx_len++) = datatype;
				*(tx_buffer + tx_len++) = tmpPC->CardRecordNumber;
				for(i = 0; i < tmpPC->CardRecordNumber && i < 10; i++)
				{
					memcpy(tx_buffer + tx_len, tmpPC->CardRecord[i].TTC, 2);		
					tx_len += 2;
					memcpy(tx_buffer + tx_len, tmpPC->CardRecord[i].Limit, 3);			
					tx_len += 3;
					memcpy(tx_buffer + tx_len, tmpPC->CardRecord[i].Money, 4);
					tx_len += 4;
					memcpy(tx_buffer + tx_len, &(tmpPC->CardRecord[i].Type), 1);		
					tx_len += 1;
					memcpy(tx_buffer + tx_len, tmpPC->CardRecord[i].TermID, 6);
					tx_len += 6;
					memcpy(tx_buffer + tx_len, tmpPC->CardRecord[i].Time, 7);
					tx_len += 7;
				}
				pthread_mutex_unlock(&g_mutexLockPc);

				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					pcAckSend(myPC->Panel, PC_CMD_GET_CARDINFO, tradeno, tx_buffer, tx_len);
				else	
					pcAckSend(ipanel, PC_CMD_GET_CARDINFO, tradeno, tx_buffer, tx_len);
				break;
			}
			
			break;
		default:
			break;
		}
		
		//taskDelay(1);
		//usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:tPcTxCommandProcess
*Description		:�����ͻ��������͵�����
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:��
*Return				:��
*History			:2015-12-23,modified by syj
*/
void tPcTxCommandProcess(int panel)
{
	char rdBuffer[PC_DATA_MAX_SIZE] = {0};
	int rdLength = 0;
	struct msg_struct msg_st;
	msg_st.msgType = 0;

	char rxBuffer[PC_DATA_MAX_SIZE] = {0};
	int rxLength = 0;

	int i = 0;
	int istate = 0;
	struct PCStruct *myPC = NULL;
	
	if(PC_PANEL_1 == panel)			
	{
		myPC = &PC_1;
	    prctl(PR_SET_NAME,(unsigned long)"tPcTxCmdProcA");
	}
	else if(PC_PANEL_2 == panel)	
	{
		myPC = &PC_2;
        prctl(PR_SET_NAME,(unsigned long)"tPcTxCmdProcB");
	}
	else											
	{
		jljRunLog("Function \"%s\" panel[%x] is invalid!\n", __FUNCTION__, panel);	
		return;
	}

	
	FOREVER
	{
		//��ȡ�ͻ��������͵�����
		memset(rdBuffer, 0, PC_DATA_MAX_SIZE);	rdLength = 0;
		memset(msg_st.msgBuffer,0,PC_DATA_MAX_SIZE);
		//if(NULL != myPC->TxBuffer)
		//{
			//rdLength = msgQReceive(myPC->TxBuffer, rdBuffer, PC_DATA_MAX_SIZE, WAIT_FOREVER);

		//}

		if(myPC->TxBuffer != ERROR)
		{
			rdLength = msgrcv(myPC->TxBuffer,&msg_st,PC_DATA_MAX_SIZE,0,IPC_NOWAIT);
			if(rdLength > 0)
			{
				memcpy(rdBuffer,msg_st.msgBuffer,rdLength);
				printf("tPcTxCommandProcess,rdLength = %d\n",rdLength);
			}
		}

		if(rdLength <= 0)
		{
			//taskDelay(1);
			usleep(10000); //fj:��arm��linux��ʱʱ�侫�ȴﲻ��1����
			continue;
		}
	
		for(i = 0; i < 2; i++)//����ָ�������
		{
			istate = pcSend(panel, *(rdBuffer + 0), rdBuffer + 1, rdLength - 1, rxBuffer, PC_DATA_MAX_SIZE);
			if(1 != istate)
				break;
		}

		//taskDelay(1);
		//usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:pcAckSend
*Description		:��ƽ����Է��ͷ��ص�����
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:command	������
*						:tradeno		��ˮ��
*						:inbuffer		���͵���Ч����
*						:nbytes			���͵���Ч���ݳ���
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-28,modified by syj
*/
int pcAckSend(int panel, char command, char tradeno, char *inbuffer, int nbytes)
{
	char databuffer[512] = {0};
	int data_len = 0;
	char tx_buffer[512*2] = {0};
	int tx_len = 0;
	struct PCStruct *myPC = NULL;
	int temp = 0, i = 0, myCRC = 0;

	//�жϳ���
	if(nbytes + 16 > 512)
	{
		jljRunLog("Error, [nbytes = %d] data length is too large!\n", nbytes);
		return ERROR;
	}

	//�ж����ѡ�񲢽�������
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(databuffer + data_len++) = 0xfa;
	temp = hex2Bcd(5 + nbytes);
	*(databuffer + data_len++) = temp >> 8;
	*(databuffer + data_len++) = temp >> 0;
	*(databuffer + data_len++) = tradeno;
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
		*(databuffer + data_len++) = 0;			//��ǹʱ���Ź̶�Ϊ0
	else
		*(databuffer + data_len++) = panel;
	*(databuffer + data_len++) = myPC->Nozzle;
	*(databuffer + data_len++) = 0xff;
	*(databuffer + data_len++) = command;
	memcpy(databuffer + data_len, inbuffer, nbytes);
	data_len += nbytes;
	myCRC = crc16Get(databuffer + 1, data_len - 1);
	*(databuffer + data_len++) = myCRC>>8;
	*(databuffer + data_len++) = myCRC>>0;
			
	//���ת���ַ�
	*(tx_buffer + tx_len++) = 0xfa;
	for(i = 1; i < data_len; i++)
	{
		*(tx_buffer + tx_len++) = *(databuffer + i);
		if(0xfa == *(databuffer + i))
		{
			*(tx_buffer + tx_len++) = 0xfa;
		}
	}

	//���浥ǹʱ����ʹ��ƽ����ԡ�����˫ǹʱʹ��A��ƽ�����
	if(PANEL_NOZZLE_DOUBLE == paramPanelNozzleGet())	
		comWriteInTime(PC_1.Comx, tx_buffer, tx_len, 2);
	else																						
		comWriteInTime(myPC->Comx, tx_buffer, tx_len, 2);

	return 0;
}


/********************************************************************
*Name				:pcSend
*Description		:��ƽ����Է�������
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:command	������
*						:inbuffer		���͵���Ч����
*						:nbytes			���͵���Ч���ݳ���
*						:maxbytes	���յ����ݻ����С
*Output			:outbuffer		���յ����ݻ��棬��������ͷ��У�����ȫ������
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-28,modified by syj
*/
int pcSend(int panel, char command, char *inbuffer, int nbytes, char *outbuffer, int maxbytes)
{
	char databuffer[512] = {0};
	int data_len = 0;
	char tx_buffer[512*2] = {0};
	int tx_len = 0;

	int i = 0, timer = 0, istate = 0, myCRC = 0, temp = 0, myTradeNO = 0, bcdTradeNO = 0;
	struct PCStruct *myPC = NULL, *transPC = NULL;

	//�жϳ���
	if(nbytes + 16 > 512)
	{
		jljRunLog("Error, [nbytes = %d] data length is too large!\n", nbytes);
		return ERROR;
	}

	//�ж����ѡ�񲢽�������
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(databuffer + data_len++) = 0xfa;
	temp = hex2Bcd(5 + nbytes);
	*(databuffer + data_len++) = temp >> 8;
	*(databuffer + data_len++) = temp >> 0;
	myTradeNO = myPC->TradeNO;
	if(myTradeNO + 1 > 99)	
		myTradeNO = 0;
	else
		myTradeNO += 1;
	myPC->TradeNO = myTradeNO;
	bcdTradeNO = hex2Bcd(myTradeNO);
	*(databuffer + data_len++) = bcdTradeNO;
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
		*(databuffer + data_len++) = 0;			//��ǹʱ���Ź̶�Ϊ0
	else																					
		*(databuffer + data_len++) = panel;
	*(databuffer + data_len++) = myPC->Nozzle;
	*(databuffer + data_len++) = 0xff;
	*(databuffer + data_len++) = command;
	memcpy(databuffer + data_len, inbuffer, nbytes);
	data_len += nbytes;
	myCRC = crc16Get(databuffer + 1, data_len - 1);
	*(databuffer + data_len++) = myCRC>>8;
	*(databuffer + data_len++) = myCRC>>0;
			
	//���ת���ַ�
	*(tx_buffer + tx_len++) = 0xfa;
	for(i = 1; i < data_len; i++)
	{
		*(tx_buffer + tx_len++) = *(databuffer + i);
		if(0xfa == *(databuffer + i))
		{
			*(tx_buffer + tx_len++) = 0xfa;
		}
	}

	//�ж���Ҫ������ƽ����� �޸���20160507��by SYJ
	if(PANEL_NOZZLE_DOUBLE == paramPanelNozzleGet())	
		transPC = &PC_1;
	if(PANEL_NOZZLE_DOUBLE != paramPanelNozzleGet() && PC_PANEL_1 == panel)	
		transPC = &PC_1;
	if(PANEL_NOZZLE_DOUBLE != paramPanelNozzleGet() && PC_PANEL_2 == panel)	
		transPC = &PC_2;

	//�ȴ��豸׼����
	//semTake(transPC->SemID, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&transPC->SemID);

	//��������
	transPC->ackLength = 0;
	comWriteInTime(transPC->Comx, tx_buffer, tx_len, 2);

	//�ȴ����յ�����������
	//	������յ������������ֻ���ˮ���뷢�͵����ݲ�һ��ʱ��ӡ������Ϣ��������ν��յ����ݣ�
	//	������յ������������ֺ���ˮ���뷢�͵�����һ����������յ������ݣ�
	//	�����ʱ2��δ���յ����������򷵻س�ʱ��
	
	while(1)
	{
		if(transPC->ackLength>0 && (command!=*(transPC->ackBuffer + 7) || bcdTradeNO!=*(transPC->ackBuffer + 3)))
		{
/*/////////////////////////	
			jljRunLog("���͵�ƽ������������������ݲ�һ�£������ֻ���ˮ�Ų�һ��!\n");
			
			printf("%s:%d[Send][timer =%x]:", __FUNCTION__, __LINE__, timer);
			for(i = 0; i < tx_len; i++)	printf("%2x:", *(tx_buffer + i));
			printf("\n");

			printf("%s:%d[Recv][timer =%x]:", __FUNCTION__, __LINE__, timer);
			for(i = 0; i < transPC->ackLength; i++)	printf("%2x:", *(transPC->ackBuffer + i));
			printf("\n");
////////////////////////*/

			transPC->ackLength = 0;
		}

		if(transPC->ackLength>0 && command==*(transPC->ackBuffer + 7) && bcdTradeNO==*(transPC->ackBuffer + 3))
		{
/*/////////////////////////
			printf("%s:%d[SendDone][timer =%x]:", __FUNCTION__, __LINE__, timer);
			for(i = 0; i < tx_len; i++)	printf("%2x:", *(tx_buffer + i));
			printf("\n");

			printf("%s:%d[RecvDone][timer =%x]:", __FUNCTION__, __LINE__, timer);
			for(i = 0; i < transPC->ackLength; i++)	printf("%2x:", *(transPC->ackBuffer + i));
			printf("\n");
////////////////////////*/

			
			if(transPC->ackLength > maxbytes)
			{
/*/////////////////////////
			printf("�������ռ�̫С![�����С=%x][��������=%d]!\n", maxbytes, myPC->ackLength);
			for(i = 0; i < tx_len; i++)	printf("%2x:", *(transPC->ackBuffer + i));
			printf("\n");
////////////////////////*/

				goto DONE;
			}

			memcpy(outbuffer, transPC->ackBuffer, transPC->ackLength);
			goto DONE;
		}

		//if(timer >= 2*sysClkRateGet())
		if(timer >= 2000) //fj:
		{
/*/////////////////////////
			printf("���ͽ��ճ�ʱ!\n");
			printf("%s:%d[SendDoneOuttime]:", __FUNCTION__, __LINE__);
			for(i = 0; i < tx_len; i++)	printf("%2x:", *(tx_buffer + i));
			printf("\n");
////////////////////////*/
			istate = 1;
			goto DONE;
		}

		timer += 10;
		usleep(10000);
	}

DONE:
	//semGive(transPC->SemID);
	pthread_mutex_unlock(&transPC->SemID);

	return istate;
}


/********************************************************************
*Name				:pcIsApplyPause
*Description		:��ȡƽ������Ƿ�֪ͨ�ͻ���ͣʹ�õ�״̬
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:��
*Return				:0=������1=��ͣʹ�ã�
*History			:2016-01-11,modified by syj
*/
int pcIsApplyPause(int panel, int phynozzle)
{
	return 0;
}


/********************************************************************
*Name				:pcNetInfoSet
*Description		:����ƽ���������������Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:ip				IP��ַ�����ʮ����
*						:mask		���룬���ʮ����
*						:gateway	Ĭ�����ظ������ʮ����
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcNetInfoSet(int panel, int phynozzle, const char *ip, const char *mask, const char *gateway)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	unsigned long myAddr = 0;
	int istate = 0, myCRC = 0, i = 0, data = 0;

	//�ж�����ĵ��ʮ���Ƶ�ַ�Ƿ�Ϸ�
	if(ERROR == myNetDotAddrCheck(ip))
	{
		jljRunLog("Error, the ip address \"%s\" is invalid!\n", ip);
		return ERROR;
	}
	if(ERROR == myNetDotAddrCheck(mask))
	{
		jljRunLog("Error, the mask address \"%s\" is invalid!\n", mask);
		return ERROR;
	}
	if(ERROR == myNetDotAddrCheck(gateway))
	{
		jljRunLog("Error, the gateway address \"%s\" is invalid!\n", gateway);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0

	*(tx_buffer + tx_len++) = 2;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)ip);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);
	myAddr = inet_addr((char*)mask);
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);
	myAddr = inet_addr((char*)gateway);
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcNetInfoGet
*Description		:��ȡƽ���������������Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:ip				IP��ַ�����ʮ����
*						:mask		���룬���ʮ����
*						:gateway	Ĭ�����ظ������ʮ����
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcNetInfoGet(int panel, int phynozzle, char *ip, char *mask, char *gateway)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0, myCRC = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(tx_buffer + tx_len++) = 2;
	*(tx_buffer + tx_len++) = 0;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		iAddr.s_addr = (rx_buffer[11]<<0) | (rx_buffer[12]<<8) | (rx_buffer[13]<<16) | (rx_buffer[14]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(ip, myAddr);
		iAddr.s_addr = (rx_buffer[15]<<0) | (rx_buffer[16]<<8) | (rx_buffer[17]<<16) | (rx_buffer[18]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(mask, myAddr);
		iAddr.s_addr = (rx_buffer[19]<<0) | (rx_buffer[20]<<8) | (rx_buffer[21]<<16) | (rx_buffer[22]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(gateway, myAddr);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcDNSInfoSet
*Description		:����ƽ�����DNS
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:first			��ѡDNS��ַ�����ʮ����
*						:second	����DNS��ַ�����ʮ����
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcDNSInfoSet(int panel, int phynozzle, const char *first, const char *second)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	unsigned long myAddr = 0;
	int istate = 0, myCRC = 0, i = 0, data = 0;

	//�ж�����ĵ��ʮ���Ƶ�ַ�Ƿ�Ϸ�
	if(ERROR == myNetDotAddrCheck(first))
	{
		jljRunLog("Error, the first DNS address \"%s\" is invalid!\n", first);
		return ERROR;
	}
	if(ERROR == myNetDotAddrCheck(second))
	{
		jljRunLog("Error, the second DNS address \"%s\" is invalid!\n", second);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0
	
	*(tx_buffer + tx_len++) = 3;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)first);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);
	myAddr = inet_addr((char*)second);
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcDNSInfoGet
*Description		:��ȡƽ�����DNS
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:first			��ѡDNS��ַ�����ʮ����
*						:second	����DNS��ַ�����ʮ����
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcDNSInfoGet(int panel, int phynozzle, char *first, char *second)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0, myCRC = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(tx_buffer + tx_len++) = 3;
	*(tx_buffer + tx_len++) = 0;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		iAddr.s_addr = (rx_buffer[11]<<0) | (rx_buffer[12]<<8) | (rx_buffer[13]<<16) | (rx_buffer[14]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(first, myAddr);
		iAddr.s_addr = (rx_buffer[15]<<0) | (rx_buffer[16]<<8) | (rx_buffer[17]<<16) | (rx_buffer[18]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(second, myAddr);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcFtpServerInfoSet
*Description		:����ƽ�����FTP��������ַ��Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:ftpIp		FTP��������ַ�����ʮ����
*						:ftpPort		FTP�������˿ںţ�ʮ����ASCII
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcFtpServerInfoSet(int panel, int phynozzle, const char *ftpIp, const char *ftpPort)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	unsigned long myAddr = 0;
	int istate = 0, myCRC = 0, i = 0, data = 0;

	//�ж�����ĵ��ʮ���Ƶ�ַ�Ƿ�Ϸ�
	if(ERROR == myNetDotAddrCheck(ftpIp))
	{
		jljRunLog("Error, the ftp address \"%s\" is invalid!\n", ftpIp);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0
	
	*(tx_buffer + tx_len++) = 4;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)ftpIp);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);
	myAddr = atoi(ftpPort);
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcFtpServerInfoGet
*Description		:��ȡƽ�����FTP��������ַ��Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:ftpIp		FTP������IP��ַ�����ʮ����
*						:ftpPort		FTP�������˿ںţ�ʮ����ASCII��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcFtpServerInfoGet(int panel, int phynozzle, char *ftpIp, char *ftpPort)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0, myCRC = 0, data = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(tx_buffer + tx_len++) = 4;
	*(tx_buffer + tx_len++) = 0;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		iAddr.s_addr = (rx_buffer[11]<<0) | (rx_buffer[12]<<8) | (rx_buffer[13]<<16) | (rx_buffer[14]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(ftpIp, myAddr);
		myItoa((rx_buffer[15]<<8) | (rx_buffer[16]<<0), ftpPort, 10);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcServerInfoSet
*Description		:����ƽ����Ժ�̨��������ַ��Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:serverIp	ƽ�����ӵķ�������ַ�����ʮ����
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcServerInfoSet(int panel, int phynozzle, const char *serverIp)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	unsigned long myAddr = 0;
	int istate = 0, myCRC = 0, i = 0, data = 0;

	//�ж�����ĵ��ʮ���Ƶ�ַ�Ƿ�Ϸ�
	if(ERROR == myNetDotAddrCheck(serverIp))
	{
		jljRunLog("Error, the serverIp address \"%s\" is invalid!\n", serverIp);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)serverIp);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcServerInfoGet
*Description		:��ȡƽ����Ժ�̨��������ַ��Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:serverIp		ƽ�����ӵķ�������ַ�����ʮ����
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcServerInfoGet(int panel, int phynozzle, char *serverIp)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0, myCRC = 0, data = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 0;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		iAddr.s_addr = (rx_buffer[11]<<0) | (rx_buffer[12]<<8) | (rx_buffer[13]<<16) | (rx_buffer[14]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(serverIp, myAddr);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcVolumeSet
*Description		:����ƽ���������
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:volume	ƽ�����������HEX
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcVolumeSet(int panel, int phynozzle, unsigned char volume)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	int istate = 0, myCRC = 0, i = 0, data = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0
	
	*(tx_buffer + tx_len++) = 1;
	*(tx_buffer + tx_len++) = volume;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 9))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcVolumeGet
*Description		:��ȡƽ���������
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:volume	ƽ�����������HEX
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcVolumeGet(int panel, int phynozzle, unsigned char *volume)
{
	char rx_buffer[128] = {0};
	int rx_len = 0;

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0, myCRC = 0, data = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_GET_VOLUME, "\x00", 0, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 8))
	{
		*volume = *(rx_buffer + 9);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 8))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcTelephoneIPSet
*Description		:����ƽ����������Խ���̨IP��ַ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:ip				ƽ����������Խ���̨IP��ַ�����ʮ����
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-05-07,modified by syj
*/
int pcTelephoneIPSet(int panel, int phynozzle, const char *ip)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;
	char rx_buffer[128] = {0};
	struct PCStruct *myPC = NULL;
	unsigned long myAddr = 0;
	int istate = 0;

	//�ж�����ĵ��ʮ���Ƶ�ַ�Ƿ�Ϸ�
	if(ERROR == myNetDotAddrCheck(ip))
	{
		jljRunLog("Error, the ip address \"%s\" is invalid!\n", ip);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	//ע��:ͨ������inet_addrת����õ�������ַΪ��λ��ǰ����"192.168.2.10"ת������Ϊ0xa02a8c0
	
	*(tx_buffer + tx_len++) = 6;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)ip);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcTelephoneIPGet
*Description		:��ȡƽ����������Խ���̨IP��ַ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:ip				ƽ����������Խ���̨IP��ַ�����ʮ����
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-05-07,modified by syj
*/
int pcTelephoneIPGet(int panel, int phynozzle, char *ip)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};

	struct PCStruct *myPC = NULL;
	char *myAddr = NULL;
	struct in_addr iAddr;
	int istate = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯����
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 0;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PCPARAM, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 10))
	{
		iAddr.s_addr = (rx_buffer[11]<<0) | (rx_buffer[12]<<8) | (rx_buffer[13]<<16) | (rx_buffer[14]<<24);
		myAddr = inet_ntoa(iAddr);
		strcpy(ip, myAddr);
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 10))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcPsaawordUpload
*Description		:���ͻ�����������������ϴ���PCƽ��
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	�߼�ǹ��
*						:inbuffer		����(����12ASCII + �Ƿ�������ɱ�ʶ1Bin)
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-04-14,modified by syj
*/
int pcPsaawordUpload(int panel, int logicnozzle, char *inbuffer)
{
	char rx_buffer[128] = {0};
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_PASS_UPLOAD, inbuffer, 13, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 8))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 8))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcDebitResultUpload
*Description		:���ͻ�����Ȩ�ۿ����ϴ���ƽ�����
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	�߼�ǹ��
*						:inbuffer		����(��Ȩ��ʽ1byte + �ۿ���1byte + ����3bytes + ����3bytes + ���4bytes)
*						:nbytes			���ݳ���
*Output			:��
*Return			:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-04-14,modified by syj
*/
int pcDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes)
{
	char rx_buffer[128] = {0};
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	istate = pcSend(panel, PC_CMD_DEBIT_RESULT, inbuffer, nbytes, rx_buffer, sizeof(rx_buffer));
	if(0 == istate && 0 == *(rx_buffer + 8))
	{
		istate = 0;
	}
	else if(0 == istate && 0 != *(rx_buffer + 8))
	{
		istate = ERROR;
	}
	else
	{
		istate = istate;
	}

	return istate;
}


/********************************************************************
*Name				:pcCardDebitResultUpload
*Description		:���ͻ����������Ѽ��Ϳ��ۿ����ϴ���ƽ�����
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	�߼�ǹ��
*						:inbuffer		����(���1byte 0=�ɹ�������=ʧ�� + ��������TTC��4bytes + ��ˮ��16bytes)
*						:nbytes			���ݳ���
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-04-14,modified by syj
*/
int pcCardDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;	
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//���ݷ��͵�ƽ�岢���ݷ��ؽ���жϣ����������ȷ��ִ�д���Ϊ��ȷ��˵��ִ����ȷ��������Ϊִ��ʧ�ܻ򷵻س�ʱ
	*(tx_buffer + 0) = 0x03;
	memcpy(tx_buffer + 1, inbuffer, nbytes);
	tx_len = 1 + nbytes;
	pcAckSend(panel, PC_CMD_LIGHT_CONTROL, 0, tx_buffer, tx_len);

	return istate;
}


/********************************************************************
*Name				:pcYPLightTurnOn
*Description		:������Ʒѡ���
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*Output			:��:
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcYPLightTurnOn(int panel, int phynozzle)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//������Ʒ��
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())		
		YPLightTurnOn(myPC->Panel, 0);
	else	
		YPLightTurnOn(0, 0);

	return  0;
}


/********************************************************************
*Name				:pcYPLightTurnOff
*Description		:�ر���Ʒѡ���
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcYPLightTurnOff(int panel, int phynozzle)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//�ر���Ʒ��
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())		
		YPLightTurnOff(myPC->Panel, 0);
	else
		YPLightTurnOff(0, 0);

	return  0;
}


/********************************************************************
*Name				:pcCardInfoWrite
*Description		:д����Ϣ��������Ч����Ϣʱ���棬��ƽ����Զ�ȡʱ��ƽ����Է��ظ�����
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:info			����Ϣ
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcCardInfoWrite(int panel, int phynozzle, PCCardInfoType info)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//������Ϣ
	//taskLock(); //fj:
	pthread_mutex_lock(&g_mutexLockPc);
	memcpy(&(myPC->CardInfo), &info, sizeof(PCCardInfoType));
	pthread_mutex_unlock(&g_mutexLockPc);
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcCardRecordWrite
*Description		:д�����׼�¼��Ϣ��������Ч����Ϣʱ���棬��ƽ����Զ�ȡʱ��ƽ����Է��ظ�����
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:info			�����׼�¼��Ϣ
*						:number	�����׼�¼��Ϣ����
*						:app			���˻����� 0=������Ʊ��1=����Ӧ��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcCardRecordWrite(int panel, int phynozzle, PCCardRecordType info[], int number, char app)
{
	int i = 0;
	struct PCStruct *myPC = NULL;
	
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//������Ϣ
	//taskLock(); //fj:
	pthread_mutex_lock(&g_mutexLockPc);
	myPC->CardAppSelect = app;
	myPC->CardRecordNumber = number;
	for(i = 0; i < number && i < 10; i++)
	{
		memcpy(&(myPC->CardRecord[i]), &info[i], sizeof(PCCardRecordType));
	}
	for(; i < 10; i++)
	{
		memset(&(myPC->CardRecord[i]), 0, sizeof(PCCardRecordType));
	}
	pthread_mutex_unlock(&g_mutexLockPc);
	//taskUnlock(); 

	return 0;
}


/********************************************************************
*Name				:pcVoicePlay
*Description		:ʹ��ƽ����Բ��������Σ����֧��32������
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:voice		Ҫ���ŵ�������
*						:number	Ҫ���ŵ������ζ���
*Output			:��
*Return				:0=�ɹ�������=ʧ�ܣ�
*History			:2016-01-11,modified by syj
*/
int pcVoicePlay(int panel, int *voice, int number)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;

	char rx_buffer[128] = {0};
	int rx_len = 0;

	int voiceNo = 0;

	struct PCStruct *myPC = NULL;
	int istate = 0, crcdata = 0, i = 0;
	struct msg_struct msg_st;
	msg_st.msgType = 1;

	//�ж���������
	if(number<1 || number>32)
	{
		jljRunLog("Error, voice number [number = %x] is invalid!\n", number);
		return ERROR;
	}
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��֯���ݣ���BCD��תΪHEX��
	*(tx_buffer + tx_len++) = PC_CMD_VOICE_PLAY;
	*(tx_buffer + tx_len++) = number;
	for(i = 0; i < number; i++)
	{
		voiceNo = ((voice[i]>>12)&0x000f)*1000 + ((voice[i]>>8)&0x000f)*100 + ((voice[i]>>4)&0x000f)*10 + ((voice[i]>>0)&0x000f)*1;
		*(tx_buffer + tx_len++) = voiceNo>>8;
		*(tx_buffer + tx_len++) = voiceNo>>0;
	}

	//fj:
	//if(NULL != myPC->TxBuffer)	istate = msgQSend(myPC->TxBuffer, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);

	if(ERROR != myPC->TxBuffer)	
	{
		memcpy(msg_st.msgBuffer,tx_buffer,tx_len);
		istate = msgsnd(myPC->TxBuffer,&msg_st,tx_len,IPC_NOWAIT);
	}
	return istate;
}

/********************************************************************
*Name				:pcOilInfoRead
*Description		:��ȡ��ǰ�洢���ͻ�֪ͨƽ������ͻ�����Ϣ
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:info			�ͻ���Ϣ
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcOilInfoRead(int panel, int phynozzle, PCOilInfoType *info)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//������Ϣ
	// taskLock(); //fj:
	pthread_mutex_lock(&g_mutexLockPc);
	memcpy(info, &(myPC->OilInfo), sizeof(PCOilInfoType));
	pthread_mutex_unlock(&g_mutexLockPc);
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcBillWrite
*Description		:��������˵���ϸ����ƽ����Է���
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*						:buffer			�˵���ϸ����
*						:nbytes			�˵���ϸ���ݳ���
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcBillWrite(int panel, int phynozzle, const char *buffer, int nbytes)
{
	char tx_buffer[256] = {0};
	int tx_len = 0;
	int istate = 0;
	struct PCStruct *myPC = NULL;
	struct msg_struct msg_st;
	msg_st.msgType = 1;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//�жϳ���
	if(nbytes + 10 > sizeof(tx_buffer))
	{
		jljRunLog("[function=%s][line = %d], Error, parameter lenght [nbytes=%d] is wrong!\n", __FUNCTION__, __LINE__, nbytes);
		return ERROR;
	}

	//��������
	*(tx_buffer + tx_len++) = PC_CMD_BILL_UPLOAD;
	*(tx_buffer + tx_len++) = nbytes;
	memcpy(tx_buffer + tx_len, buffer, nbytes);
	tx_len += nbytes;

	//fj:
	//if(NULL != myPC->TxBuffer)	istate = msgQSend(myPC->TxBuffer, tx_buffer, tx_len, 2*sysClkRateGet(), MSG_PRI_URGENT);
	
	if(myPC->TxBuffer != ERROR)
	{
		memcpy(msg_st.msgBuffer,tx_buffer,tx_len);
		istate = msgsnd(myPC->TxBuffer,&msg_st,tx_len,IPC_NOWAIT);
	}
  
	return 0;
}


/********************************************************************
*Name				:pcOilConfirm
*Description		:�ͻ�֪ͨƽ�������ʾ���˳���ʾ��Ʒȷ�Ͻ���
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:confirm		��Ʒȷ�ϲ���
*						:oilname		��Ʒ���ƣ�ASCII
*Output			:confirm		0=�޲�����1=��ʾ��Ʒȷ�Ͻ��棻2=ȡ�����˳���ʾ��Ʒȷ�Ͻ��棻3=ȷ�ϲ��˳���Ʒȷ�Ͻ���
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2016-01-11,modified by syj
*/
int pcOilConfirm(int panel, int phynozzle, int confirm, char *oilname)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;
	int istate = 0;
	int name_len = 0;
	struct PCStruct *myPC = NULL;
	struct msg_struct msg_st;
	msg_st.msgType = 1;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	
	//��������
	*(tx_buffer + tx_len++) = PC_CMD_OIL_ACK;
	if(1 == confirm)			
		*(tx_buffer + tx_len++) = 0;
	else if(2 == confirm)		
		*(tx_buffer + tx_len++) = 1;
	else	
		*(tx_buffer + tx_len++) = 2;

	if(1 == confirm)
	{
		name_len = strlen(oilname);
		if(name_len < 32)	
		{
			strncpy(tx_buffer + tx_len, oilname, name_len);	
			tx_len += name_len;
		}
		else	
		{
			strncpy(tx_buffer + tx_len, oilname, 32);			
			tx_len += 32;
		}
	}
	//fj:
	//if(NULL != myPC->TxBuffer)	istate = msgQSend(myPC->TxBuffer, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
	
	if(myPC->TxBuffer != ERROR)
	{
        memcpy(msg_st.msgBuffer,tx_buffer,tx_len);
		istate = msgsnd(myPC->TxBuffer,&msg_st,tx_len,IPC_NOWAIT);
	}

	return 0;
}


/********************************************************************
*Name				:pcActionUpload
*Description		:֪ͨƽ������ͻ��Ķ���
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*						:acton			�������� 0 = �����Ӧ��1 = �Ľ���ť��2 = ��Ʒѡ��3 = ������4 = ���ǹ��
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-06-02,modified by syj
*/
int pcActionUpload(int panel, int phynozzle, int action)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;
	int istate = 0;
	struct PCStruct *myPC = NULL;
	struct msg_struct msg_st;
	msg_st.msgType = 1;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//��������
	*(tx_buffer + tx_len++) = PC_CMD_ACTION_UPLOAD;
	*(tx_buffer + tx_len++) = action;
	//fj:
	//if(NULL != myPC->TxBuffer)	istate = msgQSend(myPC->TxBuffer, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);

	if(myPC->TxBuffer != ERROR)
	{
		memcpy(msg_st.msgBuffer,tx_buffer,tx_len);
		istate = msgsnd(myPC->TxBuffer,&msg_st,tx_len,IPC_NOWAIT);
	}

	return 0;
}


/********************************************************************
*Name				:pcStateUpload
*Description		:֪ͨƽ������ͻ���״̬
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*						:state			״̬
*						:inbuffer		����
*						:nbytes			��������
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-06-02,modified by syj
*/
int pcStateUpload(int panel, int phynozzle, int state, char *inbuffer, int nbytes)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;
	int istate = 0;
	struct PCStruct *myPC = NULL;
	struct msg_struct msg_st;
	msg_st.msgType = 1;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//�жϳ���
	if(nbytes + 10 > sizeof(tx_buffer))
	{
		jljRunLog("[function=%s][line = %d], Error, parameter lenght [nbytes=%d] is wrong!\n", __FUNCTION__, __LINE__, nbytes);
		return ERROR;
	}

	//��������
	*(tx_buffer + tx_len++) = PC_CMD_STATE_UPLOAD;
	*(tx_buffer + tx_len++) = (char)(state>>8);
	*(tx_buffer + tx_len++) = (char)(state>>0);
	memcpy(tx_buffer + tx_len, inbuffer, nbytes);
	tx_len += nbytes;
	//fj:
	//if(NULL != myPC->TxBuffer)	istate = msgQSend(myPC->TxBuffer, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
    
	if(myPC->TxBuffer != ERROR)
	{
		memcpy(msg_st.msgBuffer,tx_buffer,tx_len);
		istate = msgsnd(myPC->TxBuffer,&msg_st,tx_len,IPC_NOWAIT);
	}

	return 0;
}


/********************************************************************
*Name				:pcUserIDSet
*Description		:����ʹ�ô�ƽ����Ե��û�ID
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:id				�û�ID:	0��ʾ���ô�ƽ�����Ϊ���У�>=1��ʾʵ���û�ID
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-28,modified by syj
*/
int pcUserIDSet(int panel, char id)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//�����û�ID
	myPC->UserID = id;

	return 0;
}


/********************************************************************
*Name				:pcUserIDGet
*Description		:��ȡʹ�ô�ƽ����Ե��û�ID
*Input				:panel		ƽ��������� PC_PANEL_1/PC_PANEL_2
*Output			:��
*Return				:�û�ID:	0��ʾ���ô�ƽ�����Ϊ���У�>=1��ʾʵ���û�ID
*History			:2016-01-28,modified by syj
*/
int pcUserIDGet(int panel)
{
	struct PCStruct *myPC = NULL;
	
	//�ж����ѡ��
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	return myPC->UserID;
}


/********************************************************************
*Name				:pcLotteryStart
*Description		:���ͻ�����ƽ����Գ齱
*Input				:panel			ƽ��������� PC_PANEL_1/PC_PANEL_2
*						:phynozzle	�߼�ǹ��
*Output			:��
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ���ERROR
*History			:2015-12-23,modified by syj
*/
int pcLotteryStart(int panel, int phynozzle)
{
	return;
}


/********************************************************************
*Name				:pcInit
*Description		:ƽ�����ͨѶģ�鹦��
*Input				:��
*Output			:��
*Return				:��
*History			:2015-12-23,modified by syj
*/
bool pcInit(void)
{
	int data = 0;
	int tid = 0;
	char tName[16] = {0};
	char rdbuffer[16] = {0};
	struct PCStruct *myPC = NULL;


	//��ȡ�ͻ�ǹ��
	data = paramPanelNozzleGet();
	if(PANEL_NOZZLE_SINGLE == data)
	{
		pcModel = 1;
	}
	else
	{
		pcModel = 2;
	}

	//1��ƽ��ͨѶ������ݳ�ʼ��
	myPC = &PC_1;
	myPC->Comx = COM9;
	myPC->Panel = PC_PANEL_1;
	myPC->UserID = 0;

	//��ȡ�߼�ǹ��
	myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
	
	/*����һ���ź��������Ա������ͻ�������������ʱ��ƽ����Ե�ʹ��*/
	//myPC->SemID = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == myPC->SemID)	jljRunLog("Error, create semaphore fail!\n");
	
	int initMutex = pthread_mutex_init(&myPC->SemID,NULL);
	if(initMutex != 0)
	{
	    jljRunLog("Error, create semaphore fail!\n");
		return false;
	}

	
	//����һ����Ϣ���У����Դ洢ƽ������������͵�����
	//myPC->cmdBuffer = msgQCreate(10, PC_DATA_MAX_SIZE, MSG_Q_FIFO);
	//if(NULL == myPC->cmdBuffer)
	//{
	//	jljRunLog("Error, create message queue \"%s\" fail!\n", "cmdBuffer");
	//}
    myPC->cmdBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->cmdBuffer == ERROR)
	{
	    jljRunLog("Error, create message queue \"%s\" fail!\n", "cmdBuffer");
		return false;
	}

	//����һ���������Դ���ƽ������������͵�����
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdRx", strlen("tPcCmdRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);
	
	//����һ���������Խ�������ƽ����ԵĴ��ڵ����ݲ�����
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcRx", strlen("tPcRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcReceive, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	/*����һ����Ϣ���У����Դ洢�ͻ��������͵�����*/
	myPC->TxBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->TxBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "TxBuffer");
		return false;
	}
	
	/*����һ���������Դ�����ͻ��������͸�ƽ����Ե�����*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdTx", strlen("tPcCmdTx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcTxCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);




	
	/*2��ƽ��ͨѶ������ݳ�ʼ��*/
	myPC = &PC_2;
	myPC->Comx = COM10;
	myPC->Panel = PC_PANEL_2;
	myPC->UserID = 0;

	/*��ȡ�߼�ǹ��*/
	myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
	
	/*����һ���ź��������Ա������ͻ�������������ʱ��ƽ����Ե�ʹ��*/
	//myPC->SemID = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == myPC->SemID)	jljRunLog("Error, create semaphore fail!\n");
	initMutex = pthread_mutex_init(&myPC->SemID,NULL);
	if(initMutex != 0)
	{
	   jljRunLog("Error, create semaphore fail!\n");
	   return false;
	}
	
	/*����һ����Ϣ���У����Դ洢ƽ������������͵�����*/
	myPC->cmdBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->cmdBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "cmdBuffer");
		return false;
	}
	/*����һ���������Դ���ƽ������������͵�����*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmd", strlen("tPcCmd"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);
	
	/*����һ���������Խ�������ƽ����ԵĴ��ڵ����ݲ�����*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcRx", strlen("tPcRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcReceive, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	/*����һ����Ϣ���У����Դ洢�ͻ��������͵�����*/
	myPC->TxBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->TxBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "TxBuffer");
		return false;
	}

	/*����һ���������Դ�����ͻ��������͸�ƽ����Ե�����*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdTx", strlen("tPcCmdTx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcTxCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	return true;
}






