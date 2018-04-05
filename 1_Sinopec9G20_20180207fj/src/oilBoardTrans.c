/********************************************************
*修改时间:	2015-11-16
*修改人:		SYJ
*功能:			本文件代码实现主板间数据转发功能
*
*主板间通过串口通讯，在此规定:
*	双主板连接，使用1号串口(stm32f205扩展的串口1)通讯；
*	三主板连接，1号主板1号串口连接2号主板1号串口、1号主板2号串口连接3号主板1号串口、2号主板2号串口连接3号主板2号串口；
*	主板间连接关系如图:
*___________________________________________________________________________
*
*
*								-------------------------------------------------
*								|																								|
*						----|----------------------		----------------------|----
*						|		|											|      |                                          |		|
*						|		|											|		|											|		|
*	-----------------					-----------------					-----------------
*	|				|	1	|	2	|					|				|	1	|	2	|					|				|	1	|	2	|
*	|				--------|					|				--------|					|				--------|
*	|主板1						|					|主板2						|					|主板3						|
*	-----------------					-----------------					-----------------
*							
*
*___________________________________________________________________________
*
*/
//#include <VxWorks.h>
//#include "oilCom.h"
//#include "oilCfg.h"
//#include "oilLog.h"
//#include "oilSpk.h"
//#include "oilBoardTrans.h"

#include "../inc/main.h"

struct BoardRxDataStruct{
	char step;
	char buffer[BDATA_SIZE_MAX+1];
	int length;
};

struct BoardRxDataStruct BoardRxData1, BoardRxData2;


void boardRxPackageProccess(struct BoardRxDataStruct *rxstruct);
void boardRxDataAnalyze(struct BoardRxDataStruct *rxstruct, char *intbuffer, int nbytes);
void tBoardRx1(void);
void tBoardRx2(void);


/********************************************************************
*Name				:boardRxPackageProccess
*Description		:处理接收到的其它主板的一包完整数据
*Input				:rxstruct	接收到的数据存储结构，每个串口的接收应有一个对应的结构 
*Output			:无
*Return				:无
*History			:2015-11-16,modified by syj
*/
void boardRxPackageProccess(struct BoardRxDataStruct *rxstruct)
{
	int data_src = 0, data_dst = 0, data_type = 0, data_length = 0, local_board_id = 0;
	unsigned char speaker = 0, volume = 0, printer = 0, panel = 0;
	int iintbuffer[128] = {0}, ilen = 0, i = 0;
	
	//MSG_Q_ID vpcd_msgid = NULL, ipt_msgid = NULL, dst_msgid = NULL; //fj:

	//获取源主板号、目的主板号、数据类型、数据长度
	data_src = rxstruct->buffer[1];
	data_dst = rxstruct->buffer[2];
	data_type = (rxstruct->buffer[3]<<8)|(rxstruct->buffer[4]<<0);
	data_length = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

	//目标主板非本主板则不执行后续操作,fj:先注释
	//local_board_id = pcdMboardIDRead();
	//if(data_dst != local_board_id){
	//	return;
	//}

	//根据数据类型处理相应数据
	switch(data_type)
	{
	
	case	BDATA_TYPE_TOPRINTE:					//发送给打印机的数据
		printer = (rxstruct->buffer[7])&0x0f;
		pprint(printer, rxstruct->buffer + 9, data_length-2);
		break;
		
	case	BDATA_TYPE_FROMPRINTE:				//打印机返回的数据
		break;
		
	case	BDATA_TYPE_TOSPK:						//发送给语音播放器的数据
		speaker = (rxstruct->buffer[7]) & 0x0f;
		volume = rxstruct->buffer[9];
		for(i = 0; i < (data_length-3)/4; i++){
			iintbuffer[i] = (rxstruct->buffer[10 + i*4 + 0]<<24) | (rxstruct->buffer[10 + i*4 + 1]<<16) |\
				(rxstruct->buffer[10 + i*4 + 2]<<8) | (rxstruct->buffer[10 + i*4 + 3]<<0);
		}
		ilen = i;
		spkPlay(speaker, volume, iintbuffer, ilen); //fj:
		break;
		
	case	BDATA_TYPE_FROMSPK	:					//语音播放器返回的数据
		break;



	
#if APP_SINOPEC
	case	BDATA_TYPE_TOSINOPEC:				  //中石化应用:联网模块发送给石化后台的数据
		break;
		
	case	BDATA_TYPE_FROMSINOPEC:			  //中石化应用:石化后台发送给联网模块的数据
		break;
#endif	/*end #if APP_SINOPEC*/


/*  FJ:先注释
#if APP_CNPC
	case	BDATA_TYPE_TOVPCD:						//中石油应用:支付模块发送给VPCD的通讯数据
		panel = rxstruct->buffer[7];
		vpcd_msgid = (MSG_Q_ID)getI2VMsgID();
		if(NULL != vpcd_msgid){
			
			msgQSend(vpcd_msgid, rxstruct->buffer + 8, data_length - 1, NO_WAIT, MSG_PRI_NORMAL);
		}
		break;

	case	BDATA_TYPE_FROMVPCD:					//中石油应用:VPCD发送给支付模块的通讯数据
	case	BDATA_TYPE_FROMVPCD_FUN:			//中石油应用:VPCD发送给支付模块的函数接口数据
		panel = rxstruct->buffer[7];
		ipt_msgid = (MSG_Q_ID)iptMsgIdRead(panel);
		if(NULL != ipt_msgid){

			msgQSend(ipt_msgid, rxstruct->buffer + 8, data_length - 1, NO_WAIT, MSG_PRI_NORMAL);
		}
		break;
		
	case	BDATA_TYPE_TOVPCD_FUN:				//中石油应用:支付模块发送给VPCD的函数接口数据
		panel = rxstruct->buffer[7];
		dst_msgid = (MSG_Q_ID)((rxstruct->buffer[8]<<24)|(rxstruct->buffer[9]<<16)|(rxstruct->buffer[10]<<8)|(rxstruct->buffer[11]<<0));
		if(NULL != vpcd_msgid && data_length - 5 > 0){
			
			msgQSend(dst_msgid, rxstruct->buffer + 12, data_length - 5, NO_WAIT, MSG_PRI_NORMAL);
		}
		break;
#endif  */	//end #if APP_CNPC

	default:
		break;
	}

	return;
}


/********************************************************************
*Name				:boardRxDataAnalyze
*Description		:解析并处理接收到的其它主板的数据
*Input				:rxstruct	接收到的数据存储结构，每个串口的接收应有一个对应的结构 
*						:inbuffer	接收到的其它主板的数据
*						:nbytes		接收到的其它主办的数据长度
*Output			:无
*Return				:无
*History			:2015-11-16,modified by syj
*/
void boardRxDataAnalyze(struct BoardRxDataStruct *rxstruct, char *inbuffer, int nbytes)
{
	int i = 0, datalen = 0, crc_result = 0, flag = 0;

	for(i=0; i<nbytes; i++){

		//防止数据溢出
		if(rxstruct->length >= BDATA_SIZE_MAX){
			
			rxstruct->length = 0;	rxstruct->step = 0;	datalen = 0;	crc_result = 0;	flag = 0;
		}

		//接收解析数据
		rxstruct->buffer[rxstruct->length] = inbuffer[i];
		switch(rxstruct->step)
		{
		case 0:
			//接收数据开始字节
			if(0xfa == rxstruct->buffer[rxstruct->length]){
				
				rxstruct->step++;	rxstruct->length++;	
			}
			else{	
				
				rxstruct->step=0;	rxstruct->length=0;				
			}
			break;
				
		case 1:
			//接收源地址、目标地址、数据类型(不能有0xFA）
			if(0xfa == rxstruct->buffer[rxstruct->length]){

				rxstruct->step=0;	rxstruct->length=0;
			}
			else{	
					
				rxstruct->length++;
			}

			//下一步的接收
			if(rxstruct->length>=5){	rxstruct->step++;}
				
			break;
				
		case 2:
			//数据位0xfa时初步认为该字节为转意字符执行下一步处理，否则保存该字节数据
			if(0xfa == rxstruct->buffer[rxstruct->length]){	
						
				rxstruct->step=3;
			}
			else{	

				rxstruct->length++;	
			}

			//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
			datalen = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

			/*判断有效数据长度合法性*/
			if(datalen+9 >= BDATA_SIZE_MAX){

				rxstruct->step=0;	rxstruct->length=0;	
				break;
			}

			//判断是否接收完成
			if(rxstruct->length>=9 && rxstruct->length>=(datalen+9)){
				
				crc_result = crc16Get(&rxstruct->buffer[1], 6+datalen);
				if(crc_result == (rxstruct->buffer[7+datalen]<<8) | (rxstruct->buffer[8+datalen]<<0)){
					flag=1;
				}
				else{

					rxstruct->step=0;	rxstruct->length=0;
				}
			}
				
			break;
				
		case 3:
			//如果是0xfa则作为转义保存当前数据，如果不是0xfa则认为前一个0xfa为包头
			if(0xfa == rxstruct->buffer[rxstruct->length]){	
	
				rxstruct->step=2;	rxstruct->length++;
			}
			else{

				rxstruct->buffer[0] = 0xfa;	rxstruct->buffer[1] = inbuffer[i];	
				rxstruct->step = 1;	rxstruct->length = 2;
			}

			//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
			datalen = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

			//判断有效数据长度合法性
			if(datalen+9 >= BDATA_SIZE_MAX){
		
				rxstruct->step=0;	rxstruct->length=0;
				break;
			}

			//判断是否接收完成
			if(rxstruct->length>=9 && rxstruct->length>=(datalen+9)){
					
				crc_result = crc16Get(&rxstruct->buffer[1], 6+datalen);
				if(crc_result == (rxstruct->buffer[7+datalen]<<8) | (rxstruct->buffer[8+datalen]<<0)){
					flag=1;
				}	
				else{

					rxstruct->step=0;	rxstruct->length=0;
				}
			}
			break;
		default:
			break;
		}


		//处理解析完成的数据帧
		if(1==flag){

			boardRxPackageProccess(rxstruct);
			rxstruct->length = 0;	rxstruct->step = 0;	datalen = 0;	crc_result = 0;	flag = 0;
		}
	}

	return;
}


/********************************************************************
*Name				:tBoardRx1
*Description		:接收其它主板发送来的数据并解析处理
*Input				:无
*Output			:无
*Return				:实际写入长度；错误返回ERROR
*History			:2015-11-16,modified by syj
*/
void tBoardRx1(void)
{
	char rdbuffer[128 + 1]={0};
	int rdbytes=0;

	int i = 0;

	FOREVER{
		
		//rdbytes = comRead(BDATA_COM_1, rdbuffer, 128); //fj:
/*		
printf("%s:%d:rdbytes=%x:\n", __FUNCTION__, __LINE__, rdbytes);
for(i = 0; i < rdbytes; i++)	printf(":%x", *(rdbuffer + i));
printf("\n");*/

		boardRxDataAnalyze(&BoardRxData1, rdbuffer, rdbytes);

		//taskDelay(1);
		usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:tBoardRx2
*Description		:接收其它主板发送来的数据并解析处理
*Input				:无
*Output			:无
*Return				:实际写入长度；错误返回ERROR
*History			:2015-11-16,modified by syj
*/
void tBoardRx2(void)
{
	char rdbuffer[64+1]={0};
	int rdbytes=0;

	FOREVER{
		
		//rdbytes = comRead(BDATA_COM_2, rdbuffer, 64); //fj:
		boardRxDataAnalyze(&BoardRxData2, rdbuffer, rdbytes);

		//taskDelay(1);
		usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:boardSend
*Description		:向其他主板模块发送数据
*Input				:src			源主板号
*						:dst			目的主板号
*						:type		数据类型	BDATA_TYPE_TOPRINTE、BDATA_TYPE_TOSINO等定义
*						:inbuffer	需要发送的数据
*						:nbytes		需要发送的数据长度
*Output			:无
*Return				:成功返回0；失败返回其它值
*History			:2015-11-16,modified by syj
*/
int boardSend(char src, char dst, int type, char *inbuffer, int nbytes)
{
	char tx_buffer[BDATA_SIZE_MAX] = {0};
	//char tx_buffer[2048] = {0};
	int tx_len = 0, crc_result = 0, i = 0, j = 0, comFd = 0, local_id = 0;

	//组织数据，添加校验码
	*(tx_buffer + 0) = 0xfa;
	*(tx_buffer + 1) = src;
	*(tx_buffer + 2) = dst;
	*(tx_buffer + 3) = (char)(type>>8);
	*(tx_buffer + 4) = (char)(type>>0);
	*(tx_buffer + 5) = (char)(nbytes>>8);
	*(tx_buffer + 6) = (char)(nbytes>>0);
	memcpy(tx_buffer + 7, inbuffer, nbytes);
	crc_result = crc16Get(tx_buffer + 1, nbytes + 6);
	*(tx_buffer + 7 + nbytes) = (char)(crc_result>>8);
	*(tx_buffer + 8 + nbytes) = (char)(crc_result>>0);
	tx_len = nbytes + 9;

	//添加转义字符
	for(i = 1; i < tx_len; ){

		if(0xfa == *(tx_buffer + i)){
			//memcpy(tx_buffer + i + 2, tx_buffer + i + 1, tx_len -1 - i);
			for(j = tx_len; j > i; j--)	*(tx_buffer + j) = *(tx_buffer + j - 1);
			*(tx_buffer + i + 1) = 0xfa;
			i++;	tx_len++;
		}

		i++;
	}

	/*发送数据给其它主板
	*	根据主板连接图，本地主板为1号主板时，1号串口与2号主板连接，2号串口与3号主板连接；
	*	本地主板非1号主板时，1号串口与1号主板连接，2号串口与另一块主板连接；
	*/
	local_id = pcdMboardIDRead();

	if(1 == local_id)
	{
		if(2 == dst)
			comWriteInTime(BDATA_COM_1, tx_buffer, tx_len, 2); //fj:
		if(3 == dst)	
			comWriteInTime(BDATA_COM_2, tx_buffer, tx_len, 2); //fj:
	}
	else
	{
		if(1 == dst)						
			comWriteInTime(BDATA_COM_1, tx_buffer, tx_len, 2); //fj:
		if(2 == dst || 3 == dst)	
			comWriteInTime(BDATA_COM_2, tx_buffer, tx_len, 2); //fj:
	}

	return 0;
}


/********************************************************************
*Name				:boardTransInit
*Description		:主板间通讯模块功能初始化
*Input				:无
*Output			:无
*Return				:无
*History			:2015-11-16,modified by syj
*/
void boardTransInit(void)
{
	int tid = 0;

	memset(&BoardRxData1, 0, sizeof(struct BoardRxDataStruct));
	memset(&BoardRxData2, 0, sizeof(struct BoardRxDataStruct));

	//创建两个串口的其它主板数据接收处理任务，fj:先注释
	//tid = taskSpawn("tBoardRx1", 155, 0, 0x4000, (FUNCPTR)tBoardRx1, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		printf("Error!	Creat task 'tBoardRx1' failed!\n");

	//tid = taskSpawn("tBoardRx2", 155, 0, 0x4000, (FUNCPTR)tBoardRx2, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		printf("Error!	Creat task 'tBoardRx2' failed!\n");

	return;
}




