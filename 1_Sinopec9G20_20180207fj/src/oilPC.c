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


/*平板通讯时所使用缓存的大小*/
#define PC_DATA_MAX_SIZE		1024

/*促销机型号 0=非促销机(普通机型)；1=单面单枪；2=单面双枪；*/
int pcModel = ERROR;

/*促销机平板电脑通讯的先关参数数据结构*/
struct PCStruct
{
	int Comx;												/*主板与平板电脑通讯的串口号*/
	//SEM_ID SemID;									        /*与平板电脑通讯的操作互斥信号量*/
    pthread_mutex_t SemID;	

	unsigned char Panel;								   /*平板电脑面板号 PC_PANEL_1/PC_PANEL_2*/
	unsigned char UserID;							       /*使用此平板电脑的用户ID，0表示空闲*/
	unsigned char TradeNO;						           /*通讯时的序号*/
	unsigned char Nozzle;							       /*对应的油枪枪号*/

	//MSG_Q_ID cmdBuffer;							        /*平板电脑主动发送的命令数据*/
	//MSG_Q_ID TxBuffer;							        /*油机主动发送的命令数据*/
	
	int cmdBuffer;
	int TxBuffer;
	
	char ackBuffer[PC_DATA_MAX_SIZE+10];	                /*加油机主动发送命令后平板电脑返回的数据*/
	int ackLength;												/*加油机主动发送命令后平板电脑返回的数据长度*/

	PCOilInfoType OilInfo;							/*加油机信息*/

	PCCardInfoType CardInfo;					/*卡信息*/

	unsigned char CardAppSelect;				/*应用选择 0=电子油票；1=积分应用*/
	unsigned char CardRecordNumber;		/*卡交易信息条数*/
	PCCardRecordType CardRecord[10];	/*卡交易信息*/
};

pthread_mutex_t g_mutexLockPc = PTHREAD_MUTEX_INITIALIZER;

struct PCStruct PC_1;
struct PCStruct PC_2;

/*内部函数声明*/
int  pcAckSend(int panel, char command, char tradeno, char *inbuffer, int nbytes);
int  pcSend(int panel, char command, char *inbuffer, int nbytes, char *outbuffer, int maxbytes);



/********************************************************************
*Name				:tPcReceive
*Description		:接收并处理平板电脑的数据
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:无
*Return				:无
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
		rdLength = comRead(myPC->Comx, rdBuffer, 64);//接收连接平板电脑的串口数据
		if(rdLength <= 0)
		{
			//taskDelay(1);
			usleep(10000);
			continue;
		}

		for(i = 0; i < rdLength; i++)
		{	
			if(rx_len >= PC_DATA_MAX_SIZE)//防止长度溢出
			{
				rx_len = 0;
			}

			//	如果长度为0，接收到的字节为0xfa则为报文头，非0xfa则认为数据有误，不存储；
			//	如果长度非0(即已接收数据)，本数据为0xfa时判断下一数据是否为0xfa，如果是则为转义字符，否则认为前一个0xfa为报文头；
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

			//判断是否接收到完整长度的一帧数据
			dataLength = ((rx_buffer[1]>>4)&0x0f)*1000 + ((rx_buffer[1]>>0)&0x0f)*100 +	((rx_buffer[2]>>4)&0x0f)*10 + ((rx_buffer[2]>>0)&0x0f)*1;
			if(!(rx_len >= 10 && rx_len >= dataLength + 5))
			{
				continue;
			}

			//校验并处理数据，平板电脑发送的主动数据发送给其处理任务，被动返回的数据存入返回数据缓存
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
*Description		:处理平板电脑主动发送的命令
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:无
*Return				:无
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
		/*获取平板电脑发送的主动操作命令*/
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

		//根据命令字处理平板电脑发送过来的数据，序列号也应与平板电脑一致
		memset(tx_buffer, 0, sizeof(tx_buffer));	
		tx_len = 0;
		command = *(rdBuffer + 7);	
		tradeno = *(rdBuffer + 3);
		ipanel = *(rdBuffer + 4);
		switch(command)
		{
		case PC_CMD_SHAKEHANDS:	//平板电脑向加油机发送的握手命令	
			*(tx_buffer + tx_len++) = 0;						//errcode
			if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
			{
				*(tx_buffer + tx_len++) = 0x03;
				*(tx_buffer + tx_len++) = 0;					    //1面板号
				myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
				*(tx_buffer + tx_len++) = myPC->Nozzle;				//1面板对应枪号
				memcpy(tx_buffer + tx_len, PC_SOFTVERSION, 2);	    //协议版本号
				tx_len += 2;
				memcpy(tx_buffer + tx_len, PC_OIL_SOFTVERSION, 3);	//油机软件版本号
				tx_len += 3;
				pcAckSend(myPC->Panel, PC_CMD_SHAKEHANDS, tradeno, tx_buffer, tx_len);
			}
			else
			{
				*(tx_buffer + tx_len++) = 0x04;
				*(tx_buffer + tx_len++) = PC_PANEL_1;				//1面板号
				PC_1.Nozzle = iptPhysicalNozzleRead(PC_1.Panel);
				*(tx_buffer + tx_len++) = PC_1.Nozzle;				//1面板对应枪号
				*(tx_buffer + tx_len++) = PC_PANEL_2;				//2面板号
				PC_2.Nozzle = iptPhysicalNozzleRead(PC_2.Panel);
				*(tx_buffer + tx_len++) = PC_2.Nozzle;				//2面板对应枪号
				memcpy(tx_buffer + tx_len, PC_SOFTVERSION, 2);	    //协议版本号
				tx_len += 2;
				memcpy(tx_buffer + tx_len, PC_OIL_SOFTVERSION, 3);	//油机软件版本号
				tx_len += 3;
				pcAckSend(0, PC_CMD_SHAKEHANDS, tradeno, tx_buffer, tx_len);
			}
			break;
			
		case PC_CMD_LIGHT_CONTROL://平板电脑向加油机发送的拍奖灯控制命令、打印数据命令、进入支付流程命令,控制拍奖灯，单面单枪时分别控制，单面双枪时共同控制A面油品按钮灯
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
			else if(1 == *(rdBuffer + 8))//打印数据，单面单枪时分别打印，单面双枪时共同使用A面打印机打印
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
			else if(2 == *(rdBuffer + 8))//进入/退出支付流程操作
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
			else if(3 == *(rdBuffer + 8))//非油消费扣款
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate = iptCardPayFor(myPC->Panel, rdBuffer + 9);
				else												
					istate = iptCardPayFor(ipanel, rdBuffer + 9);

				break;
			}	
			else if(4 == *(rdBuffer + 8))//暂停
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
			else if(5 == *(rdBuffer + 8))	//恢复
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
			else if(6 == *(rdBuffer + 8))//6	通知油机开始输入密码
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthETCPassInput(myPC->Panel, 1);
				else	
					istate2 = iptAuthETCPassInput(ipanel, 1);

				//非法的命令，返回失败
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
			else if(7 == *(rdBuffer + 8)) //7	通知油机取消输入密码
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthETCPassInput(myPC->Panel, 2);
				else	
					istate2 = iptAuthETCPassInput(ipanel, 2);

				//非法的命令，返回失败
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
			else if(8 == *(rdBuffer + 8))//8	向加油机授权
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptAuthorize(myPC->Panel, 0, rdBuffer + 9);
				else
					istate2 = iptAuthorize(ipanel, 0, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(9 == *(rdBuffer + 8))//9	取消加油机授权
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptAuthorize(myPC->Panel, 1, rdBuffer + 9);
				else	
					istate2 = iptAuthorize(ipanel, 1, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(0x0A == *(rdBuffer + 8))//通知油机预置量
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	 
					istate2 = iptExterPreset(myPC->Panel, rdBuffer + 9);
				else	
					istate2 = iptExterPreset(ipanel, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(0x0B == *(rdBuffer + 8))//通知油机加油卡应用选择
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterCardAppSet(myPC->Panel, rdBuffer + 9);
				else	
					istate2 = iptExterCardAppSet(ipanel, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(0x0C == *(rdBuffer + 8))//通知油机加油卡支付方式选择
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterCardPaymentSet(myPC->Panel, rdBuffer + 9);
				else
					istate2 = iptExterCardPaymentSet(ipanel, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(0x0D == *(rdBuffer + 8))	//通知油机加油卡退卡
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterCardShoot(myPC->Panel);
				else	
					istate2 = iptExterCardShoot(ipanel);

				//非法的命令，返回失败
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
			else if(0x0E == *(rdBuffer + 8))//通知油机启动条码扫描
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	 
					istate2 = iptExterScanHandle(myPC->Panel, 0);
				else	
					istate2 = iptExterScanHandle(ipanel, 0);

				//非法的命令，返回失败
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
			else if(0x0F == *(rdBuffer + 8))//通知油机退出条码扫描
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterScanHandle(myPC->Panel, 1);
				else	
					istate2 = iptExterScanHandle(ipanel, 1);

				//非法的命令，返回失败
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
			else if(0x10 == *(rdBuffer + 8))//通知油机扫描或输入的条码
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
					istate2 = iptExterBarcodeInput(myPC->Panel, 0, rdBuffer + 9);
				else	
					istate2 = iptExterBarcodeInput(ipanel, 0, rdBuffer + 9);

				//非法的命令，返回失败
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
			else if(0x11 == *(rdBuffer + 8))	//通知油机扫描或输入的条码完毕
			{
				if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
					istate2 = iptExterBarcodeInput(myPC->Panel, 1, rdBuffer + 9);
				else
					istate2 = iptExterBarcodeInput(ipanel, 1, rdBuffer + 9);

				//非法的命令，返回失败
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
			
			//非法的命令，返回失败
			*(tx_buffer + tx_len++) = *(rdBuffer + 8);		//handle1
			*(tx_buffer + tx_len++) = 1;					//errcode
			if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())	
				pcAckSend(myPC->Panel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			else	
				pcAckSend(ipanel, PC_CMD_LIGHT_CONTROL, tradeno, tx_buffer, tx_len);
			break;
		case PC_CMD_GET_CARDINFO:	//平板电脑向加油机发送的读取卡片信息、卡交易记录	
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

			if(0 == datatype)	//读卡片信息
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
			
			if(1 == datatype)//读卡交易信息
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
*Description		:处理油机主动发送的命令
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:无
*Return				:无
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
		//获取油机主动发送的数据
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
			usleep(10000); //fj:该arm下linux延时时间精度达不到1毫秒
			continue;
		}
	
		for(i = 0; i < 2; i++)//根据指令发送数据
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
*Description		:向平板电脑发送返回的数据
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:command	命令字
*						:tradeno		流水号
*						:inbuffer		发送的有效数据
*						:nbytes			发送的有效数据长度
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断长度
	if(nbytes + 16 > 512)
	{
		jljRunLog("Error, [nbytes = %d] data length is too large!\n", nbytes);
		return ERROR;
	}

	//判断面板选择并解析数据
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(databuffer + data_len++) = 0xfa;
	temp = hex2Bcd(5 + nbytes);
	*(databuffer + data_len++) = temp >> 8;
	*(databuffer + data_len++) = temp >> 0;
	*(databuffer + data_len++) = tradeno;
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())
		*(databuffer + data_len++) = 0;			//单枪时面板号固定为0
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
			
	//添加转义字符
	*(tx_buffer + tx_len++) = 0xfa;
	for(i = 1; i < data_len; i++)
	{
		*(tx_buffer + tx_len++) = *(databuffer + i);
		if(0xfa == *(databuffer + i))
		{
			*(tx_buffer + tx_len++) = 0xfa;
		}
	}

	//单面单枪时各自使用平板电脑、单面双枪时使用A面平板电脑
	if(PANEL_NOZZLE_DOUBLE == paramPanelNozzleGet())	
		comWriteInTime(PC_1.Comx, tx_buffer, tx_len, 2);
	else																						
		comWriteInTime(myPC->Comx, tx_buffer, tx_len, 2);

	return 0;
}


/********************************************************************
*Name				:pcSend
*Description		:向平板电脑发送数据
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:command	命令字
*						:inbuffer		发送的有效数据
*						:nbytes			发送的有效数据长度
*						:maxbytes	接收的数据缓存大小
*Output			:outbuffer		接收的数据缓存，包括数据头到校验码的全部数据
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断长度
	if(nbytes + 16 > 512)
	{
		jljRunLog("Error, [nbytes = %d] data length is too large!\n", nbytes);
		return ERROR;
	}

	//判断面板选择并解析数据
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
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
		*(databuffer + data_len++) = 0;			//单枪时面板号固定为0
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
			
	//添加转义字符
	*(tx_buffer + tx_len++) = 0xfa;
	for(i = 1; i < data_len; i++)
	{
		*(tx_buffer + tx_len++) = *(databuffer + i);
		if(0xfa == *(databuffer + i))
		{
			*(tx_buffer + tx_len++) = 0xfa;
		}
	}

	//判断需要操作的平板电脑 修改于20160507，by SYJ
	if(PANEL_NOZZLE_DOUBLE == paramPanelNozzleGet())	
		transPC = &PC_1;
	if(PANEL_NOZZLE_DOUBLE != paramPanelNozzleGet() && PC_PANEL_1 == panel)	
		transPC = &PC_1;
	if(PANEL_NOZZLE_DOUBLE != paramPanelNozzleGet() && PC_PANEL_2 == panel)	
		transPC = &PC_2;

	//等待设备准备好
	//semTake(transPC->SemID, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&transPC->SemID);

	//发送数据
	transPC->ackLength = 0;
	comWriteInTime(transPC->Comx, tx_buffer, tx_len, 2);

	//等待接收到完整的数据
	//	如果接收到的数据命令字或流水号与发送的数据不一致时打印错误信息并清除本次接收的数据；
	//	如果接收到的数据命令字和流水号与发送的数据一致则输出接收到的数据；
	//	如果超时2秒未接收到完整数据则返回超时；
	
	while(1)
	{
		if(transPC->ackLength>0 && (command!=*(transPC->ackBuffer + 7) || bcdTradeNO!=*(transPC->ackBuffer + 3)))
		{
/*/////////////////////////	
			jljRunLog("发送到平板数据与接收命令数据不一致，命令字或流水号不一致!\n");
			
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
			printf("输出缓存空间太小![缓存大小=%x][接收数据=%d]!\n", maxbytes, myPC->ackLength);
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
			printf("发送接收超时!\n");
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
*Description		:获取平板电脑是否通知油机暂停使用的状态
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:无
*Return				:0=正常；1=暂停使用；
*History			:2016-01-11,modified by syj
*/
int pcIsApplyPause(int panel, int phynozzle)
{
	return 0;
}


/********************************************************************
*Name				:pcNetInfoSet
*Description		:设置平板电脑网络配置信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:ip				IP地址，点分十进制
*						:mask		掩码，点分十进制
*						:gateway	默认网关个，点分十进制
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断输入的点分十进制地址是否合法
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0

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

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑网络配置信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:ip				IP地址，点分十进制
*						:mask		掩码，点分十进制
*						:gateway	默认网关个，点分十进制
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(tx_buffer + tx_len++) = 2;
	*(tx_buffer + tx_len++) = 0;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:设置平板电脑DNS
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:first			首选DNS地址，点分十进制
*						:second	备用DNS地址，点分十进制
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断输入的点分十进制地址是否合法
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0
	
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

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑DNS
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:first			首选DNS地址，点分十进制
*						:second	备用DNS地址，点分十进制
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(tx_buffer + tx_len++) = 3;
	*(tx_buffer + tx_len++) = 0;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:设置平板电脑FTP服务器地址信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:ftpIp		FTP服务器地址，点分十进制
*						:ftpPort		FTP服务器端口号，十进制ASCII
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断输入的点分十进制地址是否合法
	if(ERROR == myNetDotAddrCheck(ftpIp))
	{
		jljRunLog("Error, the ftp address \"%s\" is invalid!\n", ftpIp);
		return ERROR;
	}
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0
	
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

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑FTP服务器地址信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:ftpIp		FTP服务器IP地址，点分十进制
*						:ftpPort		FTP服务器端口号，十进制ASCII码
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(tx_buffer + tx_len++) = 4;
	*(tx_buffer + tx_len++) = 0;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:设置平板电脑后台服务器地址信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:serverIp	平板连接的服务器地址，点分十进制
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断输入的点分十进制地址是否合法
	if(ERROR == myNetDotAddrCheck(serverIp))
	{
		jljRunLog("Error, the serverIp address \"%s\" is invalid!\n", serverIp);
		return ERROR;
	}
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)serverIp);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑后台服务器地址信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:serverIp		平板连接的服务器地址，点分十进制
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 0;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:设置平板电脑音量
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:volume	平板电脑音量，HEX
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0
	
	*(tx_buffer + tx_len++) = 1;
	*(tx_buffer + tx_len++) = volume;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑音量
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:volume	平板电脑音量，HEX
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:设置平板电脑语音对讲后台IP地址
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:ip				平板电脑语音对讲后台IP地址，点分十进制
*Return				:成功返回0；超时返回1；失败返回ERROR
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

	//判断输入的点分十进制地址是否合法
	if(ERROR == myNetDotAddrCheck(ip))
	{
		jljRunLog("Error, the ip address \"%s\" is invalid!\n", ip);
		return ERROR;
	}
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	//注意:通过函数inet_addr转换获得的整数地址为高位在前，如"192.168.2.10"转换所得为0xa02a8c0
	
	*(tx_buffer + tx_len++) = 6;
	*(tx_buffer + tx_len++) = 1;
	myAddr = inet_addr((char*)ip);																				
	*(tx_buffer + tx_len++) = (char)(myAddr>>0);		
	*(tx_buffer + tx_len++) = (char)(myAddr>>8);
	*(tx_buffer + tx_len++) = (char)(myAddr>>16);
	*(tx_buffer + tx_len++) = (char)(myAddr>>24);

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:获取平板电脑语音对讲后台IP地址
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:ip				平板电脑语音对讲后台IP地址，点分十进制
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据
	*(tx_buffer + tx_len++) = 5;
	*(tx_buffer + tx_len++) = 0;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:加油机将本地输入的密码上传给PC平板
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	逻辑枪号
*						:inbuffer		数据(密码12ASCII + 是否输入完成标识1Bin)
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
*History			:2016-04-14,modified by syj
*/
int pcPsaawordUpload(int panel, int logicnozzle, char *inbuffer)
{
	char rx_buffer[128] = {0};
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:加油机将授权扣款结果上传到平板电脑
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	逻辑枪号
*						:inbuffer		数据(授权方式1byte + 扣款结果1byte + 数额3bytes + 升数3bytes + 余额4bytes)
*						:nbytes			数据长度
*Output			:无
*Return			:成功返回0；超时返回1；失败返回ERROR
*History			:2016-04-14,modified by syj
*/
int pcDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes)
{
	char rx_buffer[128] = {0};
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
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
*Description		:加油机将非油消费加油卡扣款结果上传到平板电脑
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:logicnozzle	逻辑枪号
*						:inbuffer		数据(结果1byte 0=成功；其它=失败 + 非油消费TTC号4bytes + 流水号16bytes)
*						:nbytes			数据长度
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
*History			:2016-04-14,modified by syj
*/
int pcCardDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes)
{
	char tx_buffer[128] = {0};
	int tx_len = 0;	
	struct PCStruct *myPC = NULL;
	int istate = 0;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//数据发送到平板并根据返回结果判断，如果返回正确且执行代码为正确则说明执行正确，否则认为执行失败或返回超时
	*(tx_buffer + 0) = 0x03;
	memcpy(tx_buffer + 1, inbuffer, nbytes);
	tx_len = 1 + nbytes;
	pcAckSend(panel, PC_CMD_LIGHT_CONTROL, 0, tx_buffer, tx_len);

	return istate;
}


/********************************************************************
*Name				:pcYPLightTurnOn
*Description		:点亮油品选择灯
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*Output			:无:
*Return				:成功返回0；失败返回ERROR
*History			:2016-01-11,modified by syj
*/
int pcYPLightTurnOn(int panel, int phynozzle)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//点亮油品灯
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())		
		YPLightTurnOn(myPC->Panel, 0);
	else	
		YPLightTurnOn(0, 0);

	return  0;
}


/********************************************************************
*Name				:pcYPLightTurnOff
*Description		:关闭油品选择灯
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*Output			:无
*Return				:成功返回0；失败返回ERROR
*History			:2016-01-11,modified by syj
*/
int pcYPLightTurnOff(int panel, int phynozzle)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//关闭油品灯
	if(PANEL_NOZZLE_SINGLE == paramPanelNozzleGet())		
		YPLightTurnOff(myPC->Panel, 0);
	else
		YPLightTurnOff(0, 0);

	return  0;
}


/********************************************************************
*Name				:pcCardInfoWrite
*Description		:写卡信息，当有有效卡信息时缓存，当平板电脑读取时向平板电脑返回该数据
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:info			卡信息
*Return				:成功返回0；超时返回1；失败返回ERROR
*History			:2016-01-11,modified by syj
*/
int pcCardInfoWrite(int panel, int phynozzle, PCCardInfoType info)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//保存信息
	//taskLock(); //fj:
	pthread_mutex_lock(&g_mutexLockPc);
	memcpy(&(myPC->CardInfo), &info, sizeof(PCCardInfoType));
	pthread_mutex_unlock(&g_mutexLockPc);
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcCardRecordWrite
*Description		:写卡交易记录信息，当有有效卡信息时缓存，当平板电脑读取时向平板电脑返回该数据
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:info			卡交易记录信息
*						:number	卡交易记录信息条数
*						:app			卡账户类型 0=电子油票；1=积分应用
*Return				:成功返回0；失败返回ERROR
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

	//保存信息
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
*Description		:使用平板电脑播放语音段，最多支持32段语音
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:voice		要播放的语音段
*						:number	要播放的语音段段数
*Output			:无
*Return				:0=成功；其它=失败；
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

	//判断语音段数
	if(number<1 || number>32)
	{
		jljRunLog("Error, voice number [number = %x] is invalid!\n", number);
		return ERROR;
	}
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//组织数据，将BCD码转为HEX码
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
*Description		:获取当前存储的油机通知平板电脑油机的信息
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:info			油机信息
*Return				:成功返回0；失败返回ERROR
*History			:2016-01-11,modified by syj
*/
int pcOilInfoRead(int panel, int phynozzle, PCOilInfoType *info)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//保存信息
	// taskLock(); //fj:
	pthread_mutex_lock(&g_mutexLockPc);
	memcpy(info, &(myPC->OilInfo), sizeof(PCOilInfoType));
	pthread_mutex_unlock(&g_mutexLockPc);
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcBillWrite
*Description		:缓存加油账单明细并向平板电脑发送
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*						:buffer			账单明细数据
*						:nbytes			账单明细数据长度
*Output			:无
*Return				:成功返回0；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//判断长度
	if(nbytes + 10 > sizeof(tx_buffer))
	{
		jljRunLog("[function=%s][line = %d], Error, parameter lenght [nbytes=%d] is wrong!\n", __FUNCTION__, __LINE__, nbytes);
		return ERROR;
	}

	//发送数据
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
*Description		:油机通知平板电脑显示或退出显示油品确认界面
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:confirm		油品确认操作
*						:oilname		油品名称，ASCII
*Output			:confirm		0=无操作；1=显示油品确认界面；2=取消并退出显示油品确认界面；3=确认并退出油品确认界面
*Return				:成功返回0；超时返回1；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	
	//发送数据
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
*Description		:通知平板电脑油机的动作
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*						:acton			动作代码 0 = 人体感应；1 = 拍奖按钮；2 = 油品选择；3 = 按键；4 = 提挂枪；
*Output			:无
*Return				:成功返回0；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//发送数据
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
*Description		:通知平板电脑油机的状态
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*						:state			状态
*						:inbuffer		参数
*						:nbytes			参数长度
*Output			:无
*Return				:成功返回0；失败返回ERROR
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
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//判断长度
	if(nbytes + 10 > sizeof(tx_buffer))
	{
		jljRunLog("[function=%s][line = %d], Error, parameter lenght [nbytes=%d] is wrong!\n", __FUNCTION__, __LINE__, nbytes);
		return ERROR;
	}

	//发送数据
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
*Description		:设置使用此平板电脑的用户ID
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:id				用户ID:	0表示设置此平板电脑为空闲；>=1表示实际用户ID
*Output			:无
*Return				:成功返回0；失败返回ERROR
*History			:2016-01-28,modified by syj
*/
int pcUserIDSet(int panel, char id)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
	if(!(PC_PANEL_1==panel || PC_PANEL_2==panel))
	{
		jljRunLog("Error, function \"%s\" parameter  panel [panel = %x] is wrong!\n", __FUNCTION__, panel);
		return ERROR;
	}
	if(PC_PANEL_1 == panel)	myPC = &PC_1;
	if(PC_PANEL_2 == panel)	myPC = &PC_2;

	//配置用户ID
	myPC->UserID = id;

	return 0;
}


/********************************************************************
*Name				:pcUserIDGet
*Description		:获取使用此平板电脑的用户ID
*Input				:panel		平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*Output			:无
*Return				:用户ID:	0表示设置此平板电脑为空闲；>=1表示实际用户ID
*History			:2016-01-28,modified by syj
*/
int pcUserIDGet(int panel)
{
	struct PCStruct *myPC = NULL;
	
	//判断面板选择
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
*Description		:加油机触发平板电脑抽奖
*Input				:panel			平板电脑面板号 PC_PANEL_1/PC_PANEL_2
*						:phynozzle	逻辑枪号
*Output			:无
*Return				:成功返回0；超时返回1；失败返回ERROR
*History			:2015-12-23,modified by syj
*/
int pcLotteryStart(int panel, int phynozzle)
{
	return;
}


/********************************************************************
*Name				:pcInit
*Description		:平板电脑通讯模块功能
*Input				:无
*Output			:无
*Return				:无
*History			:2015-12-23,modified by syj
*/
bool pcInit(void)
{
	int data = 0;
	int tid = 0;
	char tName[16] = {0};
	char rdbuffer[16] = {0};
	struct PCStruct *myPC = NULL;


	//获取油机枪数
	data = paramPanelNozzleGet();
	if(PANEL_NOZZLE_SINGLE == data)
	{
		pcModel = 1;
	}
	else
	{
		pcModel = 2;
	}

	//1号平板通讯相关数据初始化
	myPC = &PC_1;
	myPC->Comx = COM9;
	myPC->Panel = PC_PANEL_1;
	myPC->UserID = 0;

	//获取逻辑枪号
	myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
	
	/*创建一个信号量，用以保护加油机主动发送数据时对平板电脑的使用*/
	//myPC->SemID = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == myPC->SemID)	jljRunLog("Error, create semaphore fail!\n");
	
	int initMutex = pthread_mutex_init(&myPC->SemID,NULL);
	if(initMutex != 0)
	{
	    jljRunLog("Error, create semaphore fail!\n");
		return false;
	}

	
	//创建一个消息队列，用以存储平板电脑主动发送的命令
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

	//创建一个任务，用以处理平板电脑主动发送的命令
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdRx", strlen("tPcCmdRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);
	
	//创建一个任务，用以接收连接平板电脑的串口的数据并解析
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcRx", strlen("tPcRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcReceive, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	/*创建一个消息队列，用以存储油机主动发送的命令*/
	myPC->TxBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->TxBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "TxBuffer");
		return false;
	}
	
	/*创建一个任务，用以处理加油机主动发送给平板电脑的数据*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdTx", strlen("tPcCmdTx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcTxCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);




	
	/*2号平板通讯相关数据初始化*/
	myPC = &PC_2;
	myPC->Comx = COM10;
	myPC->Panel = PC_PANEL_2;
	myPC->UserID = 0;

	/*获取逻辑枪号*/
	myPC->Nozzle = iptPhysicalNozzleRead(myPC->Panel);
	
	/*创建一个信号量，用以保护加油机主动发送数据时对平板电脑的使用*/
	//myPC->SemID = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == myPC->SemID)	jljRunLog("Error, create semaphore fail!\n");
	initMutex = pthread_mutex_init(&myPC->SemID,NULL);
	if(initMutex != 0)
	{
	   jljRunLog("Error, create semaphore fail!\n");
	   return false;
	}
	
	/*创建一个消息队列，用以存储平板电脑主动发送的命令*/
	myPC->cmdBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->cmdBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "cmdBuffer");
		return false;
	}
	/*创建一个任务，用以处理平板电脑主动发送的命令*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmd", strlen("tPcCmd"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);
	
	/*创建一个任务，用以接收连接平板电脑的串口的数据并解析*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcRx", strlen("tPcRx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcReceive, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	/*创建一个消息队列，用以存储油机主动发送的命令*/
	myPC->TxBuffer = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(myPC->TxBuffer == ERROR)
	{
		jljRunLog("Error, create message queue \"%s\" fail!\n", "TxBuffer");
		return false;
	}

	/*创建一个任务，用以处理加油机主动发送给平板电脑的数据*/
	//memset(tName, 0, sizeof(tName));
	//memcpy(tName, "tPcCmdTx", strlen("tPcCmdTx"));
	//sprintf(tName + strlen(tName), "%d", myPC->Panel);
	//tid = taskSpawn(tName, 165, 0, 0x4000, (FUNCPTR)tPcTxCommandProcess, myPC->Panel,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		jljRunLog("Error, 	create task \"%s\" fail!\n", tName);

	return true;
}






