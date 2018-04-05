/********************************************************
*�޸�ʱ��:	2015-11-16
*�޸���:		SYJ
*����:			���ļ�����ʵ�����������ת������
*
*�����ͨ������ͨѶ���ڴ˹涨:
*	˫�������ӣ�ʹ��1�Ŵ���(stm32f205��չ�Ĵ���1)ͨѶ��
*	���������ӣ�1������1�Ŵ�������2������1�Ŵ��ڡ�1������2�Ŵ�������3������1�Ŵ��ڡ�2������2�Ŵ�������3������2�Ŵ��ڣ�
*	��������ӹ�ϵ��ͼ:
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
*	|����1						|					|����2						|					|����3						|
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
*Description		:������յ������������һ����������
*Input				:rxstruct	���յ������ݴ洢�ṹ��ÿ�����ڵĽ���Ӧ��һ����Ӧ�Ľṹ 
*Output			:��
*Return				:��
*History			:2015-11-16,modified by syj
*/
void boardRxPackageProccess(struct BoardRxDataStruct *rxstruct)
{
	int data_src = 0, data_dst = 0, data_type = 0, data_length = 0, local_board_id = 0;
	unsigned char speaker = 0, volume = 0, printer = 0, panel = 0;
	int iintbuffer[128] = {0}, ilen = 0, i = 0;
	
	//MSG_Q_ID vpcd_msgid = NULL, ipt_msgid = NULL, dst_msgid = NULL; //fj:

	//��ȡԴ����š�Ŀ������š��������͡����ݳ���
	data_src = rxstruct->buffer[1];
	data_dst = rxstruct->buffer[2];
	data_type = (rxstruct->buffer[3]<<8)|(rxstruct->buffer[4]<<0);
	data_length = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

	//Ŀ������Ǳ�������ִ�к�������,fj:��ע��
	//local_board_id = pcdMboardIDRead();
	//if(data_dst != local_board_id){
	//	return;
	//}

	//�����������ʹ�����Ӧ����
	switch(data_type)
	{
	
	case	BDATA_TYPE_TOPRINTE:					//���͸���ӡ��������
		printer = (rxstruct->buffer[7])&0x0f;
		pprint(printer, rxstruct->buffer + 9, data_length-2);
		break;
		
	case	BDATA_TYPE_FROMPRINTE:				//��ӡ�����ص�����
		break;
		
	case	BDATA_TYPE_TOSPK:						//���͸�����������������
		speaker = (rxstruct->buffer[7]) & 0x0f;
		volume = rxstruct->buffer[9];
		for(i = 0; i < (data_length-3)/4; i++){
			iintbuffer[i] = (rxstruct->buffer[10 + i*4 + 0]<<24) | (rxstruct->buffer[10 + i*4 + 1]<<16) |\
				(rxstruct->buffer[10 + i*4 + 2]<<8) | (rxstruct->buffer[10 + i*4 + 3]<<0);
		}
		ilen = i;
		spkPlay(speaker, volume, iintbuffer, ilen); //fj:
		break;
		
	case	BDATA_TYPE_FROMSPK	:					//�������������ص�����
		break;



	
#if APP_SINOPEC
	case	BDATA_TYPE_TOSINOPEC:				  //��ʯ��Ӧ��:����ģ�鷢�͸�ʯ����̨������
		break;
		
	case	BDATA_TYPE_FROMSINOPEC:			  //��ʯ��Ӧ��:ʯ����̨���͸�����ģ�������
		break;
#endif	/*end #if APP_SINOPEC*/


/*  FJ:��ע��
#if APP_CNPC
	case	BDATA_TYPE_TOVPCD:						//��ʯ��Ӧ��:֧��ģ�鷢�͸�VPCD��ͨѶ����
		panel = rxstruct->buffer[7];
		vpcd_msgid = (MSG_Q_ID)getI2VMsgID();
		if(NULL != vpcd_msgid){
			
			msgQSend(vpcd_msgid, rxstruct->buffer + 8, data_length - 1, NO_WAIT, MSG_PRI_NORMAL);
		}
		break;

	case	BDATA_TYPE_FROMVPCD:					//��ʯ��Ӧ��:VPCD���͸�֧��ģ���ͨѶ����
	case	BDATA_TYPE_FROMVPCD_FUN:			//��ʯ��Ӧ��:VPCD���͸�֧��ģ��ĺ����ӿ�����
		panel = rxstruct->buffer[7];
		ipt_msgid = (MSG_Q_ID)iptMsgIdRead(panel);
		if(NULL != ipt_msgid){

			msgQSend(ipt_msgid, rxstruct->buffer + 8, data_length - 1, NO_WAIT, MSG_PRI_NORMAL);
		}
		break;
		
	case	BDATA_TYPE_TOVPCD_FUN:				//��ʯ��Ӧ��:֧��ģ�鷢�͸�VPCD�ĺ����ӿ�����
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
*Description		:������������յ����������������
*Input				:rxstruct	���յ������ݴ洢�ṹ��ÿ�����ڵĽ���Ӧ��һ����Ӧ�Ľṹ 
*						:inbuffer	���յ����������������
*						:nbytes		���յ���������������ݳ���
*Output			:��
*Return				:��
*History			:2015-11-16,modified by syj
*/
void boardRxDataAnalyze(struct BoardRxDataStruct *rxstruct, char *inbuffer, int nbytes)
{
	int i = 0, datalen = 0, crc_result = 0, flag = 0;

	for(i=0; i<nbytes; i++){

		//��ֹ�������
		if(rxstruct->length >= BDATA_SIZE_MAX){
			
			rxstruct->length = 0;	rxstruct->step = 0;	datalen = 0;	crc_result = 0;	flag = 0;
		}

		//���ս�������
		rxstruct->buffer[rxstruct->length] = inbuffer[i];
		switch(rxstruct->step)
		{
		case 0:
			//�������ݿ�ʼ�ֽ�
			if(0xfa == rxstruct->buffer[rxstruct->length]){
				
				rxstruct->step++;	rxstruct->length++;	
			}
			else{	
				
				rxstruct->step=0;	rxstruct->length=0;				
			}
			break;
				
		case 1:
			//����Դ��ַ��Ŀ���ַ����������(������0xFA��
			if(0xfa == rxstruct->buffer[rxstruct->length]){

				rxstruct->step=0;	rxstruct->length=0;
			}
			else{	
					
				rxstruct->length++;
			}

			//��һ���Ľ���
			if(rxstruct->length>=5){	rxstruct->step++;}
				
			break;
				
		case 2:
			//����λ0xfaʱ������Ϊ���ֽ�Ϊת���ַ�ִ����һ���������򱣴���ֽ�����
			if(0xfa == rxstruct->buffer[rxstruct->length]){	
						
				rxstruct->step=3;
			}
			else{	

				rxstruct->length++;	
			}

			//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
			datalen = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

			/*�ж���Ч���ݳ��ȺϷ���*/
			if(datalen+9 >= BDATA_SIZE_MAX){

				rxstruct->step=0;	rxstruct->length=0;	
				break;
			}

			//�ж��Ƿ�������
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
			//�����0xfa����Ϊת�屣�浱ǰ���ݣ��������0xfa����Ϊǰһ��0xfaΪ��ͷ
			if(0xfa == rxstruct->buffer[rxstruct->length]){	
	
				rxstruct->step=2;	rxstruct->length++;
			}
			else{

				rxstruct->buffer[0] = 0xfa;	rxstruct->buffer[1] = inbuffer[i];	
				rxstruct->step = 1;	rxstruct->length = 2;
			}

			//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
			datalen = (rxstruct->buffer[5]<<8)|(rxstruct->buffer[6]<<0);

			//�ж���Ч���ݳ��ȺϷ���
			if(datalen+9 >= BDATA_SIZE_MAX){
		
				rxstruct->step=0;	rxstruct->length=0;
				break;
			}

			//�ж��Ƿ�������
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


		//���������ɵ�����֡
		if(1==flag){

			boardRxPackageProccess(rxstruct);
			rxstruct->length = 0;	rxstruct->step = 0;	datalen = 0;	crc_result = 0;	flag = 0;
		}
	}

	return;
}


/********************************************************************
*Name				:tBoardRx1
*Description		:�����������巢���������ݲ���������
*Input				:��
*Output			:��
*Return				:ʵ��д�볤�ȣ����󷵻�ERROR
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
*Description		:�����������巢���������ݲ���������
*Input				:��
*Output			:��
*Return				:ʵ��д�볤�ȣ����󷵻�ERROR
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
*Description		:����������ģ�鷢������
*Input				:src			Դ�����
*						:dst			Ŀ�������
*						:type		��������	BDATA_TYPE_TOPRINTE��BDATA_TYPE_TOSINO�ȶ���
*						:inbuffer	��Ҫ���͵�����
*						:nbytes		��Ҫ���͵����ݳ���
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2015-11-16,modified by syj
*/
int boardSend(char src, char dst, int type, char *inbuffer, int nbytes)
{
	char tx_buffer[BDATA_SIZE_MAX] = {0};
	//char tx_buffer[2048] = {0};
	int tx_len = 0, crc_result = 0, i = 0, j = 0, comFd = 0, local_id = 0;

	//��֯���ݣ����У����
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

	//���ת���ַ�
	for(i = 1; i < tx_len; ){

		if(0xfa == *(tx_buffer + i)){
			//memcpy(tx_buffer + i + 2, tx_buffer + i + 1, tx_len -1 - i);
			for(j = tx_len; j > i; j--)	*(tx_buffer + j) = *(tx_buffer + j - 1);
			*(tx_buffer + i + 1) = 0xfa;
			i++;	tx_len++;
		}

		i++;
	}

	/*�������ݸ���������
	*	������������ͼ����������Ϊ1������ʱ��1�Ŵ�����2���������ӣ�2�Ŵ�����3���������ӣ�
	*	���������1������ʱ��1�Ŵ�����1���������ӣ�2�Ŵ�������һ���������ӣ�
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
*Description		:�����ͨѶģ�鹦�ܳ�ʼ��
*Input				:��
*Output			:��
*Return				:��
*History			:2015-11-16,modified by syj
*/
void boardTransInit(void)
{
	int tid = 0;

	memset(&BoardRxData1, 0, sizeof(struct BoardRxDataStruct));
	memset(&BoardRxData2, 0, sizeof(struct BoardRxDataStruct));

	//�����������ڵ������������ݽ��մ�������fj:��ע��
	//tid = taskSpawn("tBoardRx1", 155, 0, 0x4000, (FUNCPTR)tBoardRx1, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		printf("Error!	Creat task 'tBoardRx1' failed!\n");

	//tid = taskSpawn("tBoardRx2", 155, 0, 0x4000, (FUNCPTR)tBoardRx2, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tid))		printf("Error!	Creat task 'tBoardRx2' failed!\n");

	return;
}




