//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilLog.h"
//#include "oilBoardTrans.h"
//#include "oilPrinter.h"

#include "../inc/main.h"

PrintParamStruct printParamA, printParamB;



/****************************************************************************
*Name				:tPrintProccess
*Description		:��ӡ���д�������
*Input				:printer			0=A��ӡ��(PRINTER_A)��1=B��ӡ��(PRINTER_B)
*Output			:��
*Return				:��
*History			:2015-11-13,modified by syj
*/
void tPrintProcess(char printer)
{
	PrintParamStruct *param=NULL;
	PrintListNode *node=NULL;
	int lastbytes = 0;
	const PRINTER_MAX_SIZE_ONE_TIME = 1024;

	//�ж��豸ѡ��
	if(PRINTER_A==printer)
	{
		param=&printParamA;
		param->comFd = COM13;  //fj:��ע��
		prctl(PR_SET_NAME,(unsigned long)"tPrintProcessA");
	}
	else if(PRINTER_B==printer)
	{
		param=&printParamB;
		param->comFd = COM17;  //fj:��ע��
        prctl(PR_SET_NAME,(unsigned long)"tPrintProcessB");
	}
	else
	{
		printf("%s:%d:printer select error!\n", __FUNCTION__, __LINE__);
		return;
	}

	//��ȡ��ӡ���ݶ��в���ɴ�ӡ���ݵķ���
	/*	Ϊ�������������ɴ�ӡ���������������ӡ���ݳ������������͵�
	*	���ݳ���(PRINTER_MAX_SIZE_ONE_TIME)ʱ�ִδ�ӡ�����м�����һ������ʱ
	*/

	FOREVER
	{
		node = (PrintListNode*)lstGet(&(param->list));
		if(NULL == node)
		{
			//taskDelay(10*sysClkRateGet()/1000);
			usleep(10000); //fj:
			continue;
		}

		lastbytes = node->length;
		while(lastbytes>0)
		{
			if(lastbytes > PRINTER_MAX_SIZE_ONE_TIME)
			{
				comWriteInTime(param->comFd, node->buffer + (node->length-lastbytes), PRINTER_MAX_SIZE_ONE_TIME, 2);  //fj:��ע��
				lastbytes = lastbytes - PRINTER_MAX_SIZE_ONE_TIME;
			}
			else
			{
				printf("exe print,length = %d \n",lastbytes);
				comWriteInTime(param->comFd, node->buffer+ (node->length-lastbytes), lastbytes, 2); //fj:����oilCom.c����
				lastbytes = lastbytes - lastbytes;
			}

			//taskDelay(200*ONE_MILLI_SECOND);
			usleep(200000);  //fj:
		}

		free(node->buffer);	node->buffer = NULL;
		free(node);	
		node = NULL;

		//taskDelay(10*sysClkRateGet()/1000);
		usleep(10000); //fj:
	}

	return;
}


/****************************************************************************
*Name				:printInit
*Description		:��ӡ���ܳ�ʼ��
*Input				:printer		b7~b4Ϊ��ӡ�������(1~3)��b3~b0Ϊ��ӡ��ѡ��(PRINTER_A/PRINTER_B)��
*						:inbuffer	��Ҫ��ӡ������
*						:nbytes		��Ҫ��ӡ�����ݳ���
*Output			:��
*Return				:�ɹ�����OK��ʧ�ܷ���ERROR
*History			:2015-11-13,modified by syj
*/
int pprint(char printer, char *inbuffer, int nbytes)
{
	printf("----start pprint\n");
	PrintParamStruct *param=NULL;
	PrintListNode *node=NULL;
	int board = 0, iprinter = 0, i = 0, local_board_id = 0;
	//char txbuffer[BDATA_SIZE_MAX] = {0};
	char txbuffer[2048];
	int tx_len = 0; 

	//��ӡ������źʹ�ӡ������
	board = (printer>>4) & 0x0f;
	iprinter = printer & 0x0f;
	//�ж��豸ѡ��
	if(PRINTER_A==iprinter)			
		param=&printParamA;
	else if(PRINTER_B==iprinter)	
		param=&printParamB;
	else
	{
		jljRunLog("printer select error!\n");
		return ERROR;
	}

	//��ȡ���������
	local_board_id = pcdMboardIDRead();  //fj:��PCD����

	printf("local_board_id = %d,board = %d\n");

	//��Ҫ�������������ӵĴ�ӡ����ӡ���ݵĴ���
	if(0!=board && local_board_id!=board)
	{
		for(i = 0; i < nbytes; i+=tx_len)
		{
			//fj:��oilBoardTrans�ﶨ�壬��ע��
			if(nbytes - i > BDATA_SIZE_MAX - 32)	
				tx_len = BDATA_SIZE_MAX - 32;
			else	
				tx_len = nbytes - i;

			*(txbuffer + 0) = printer & 0x0f;
			*(txbuffer + 1) = 0;
			memcpy(txbuffer + 2, inbuffer + i, tx_len);
			boardSend(local_board_id, board, BDATA_TYPE_TOPRINTE, txbuffer, tx_len + 2);  //fj:
		}

		return OK;
	}

	//�ڱ��������ӡ����ӡ�����ݴ���
	//�ж�δ��ӡ���г���
	if(lstCount(&param->list) > PRINT_NODE_MAX)
	{
		jljRunLog("print list is full!\n");
		return ERROR;
	}
	//����ڵ�ռ�ͽڵ��ӡ���ݲ��������ݵ�����Ŀռ���
	node = (PrintListNode*)malloc(sizeof(PrintListNode));
	if(NULL == node)
	{
		jljRunLog("print node malloc fail!\n");
		return ERROR;
	}
	node->buffer = (char*)malloc(nbytes);
	if(NULL == node->buffer)
	{
		jljRunLog("print node->buffer malloc fail!\n");
		free(node);
		return ERROR;
	}
	memcpy(node->buffer, inbuffer, nbytes);
	node->length = nbytes;
	lstAdd(&(param->list), (NODE*)node);	//����ӡ������ӵ���ӡ������

	return OK;
}


/****************************************************************************
*Name				:printInit
*Description		:��ӡ���ܳ�ʼ��
*Input				:��
*Output			:��
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2015-11-13,modified by syj
*/

/*
int printInit(void)
{
	PrintParamStruct *param=NULL;

	//fj:
	//param = &printParamA;
	//param->tPrintId = taskSpawn("tPrintA", 250, 0, 0x8000, (FUNCPTR)tPrintProccess, PRINTER_A,1,2,3,4,5,6,7,8,9);
	//if(OK != taskIdVerify(param->tPrintId))		printf("%s: create task tPrintA fail!\n", __FUNCTION__);

	//param = &printParamB;
	/param->tPrintId = taskSpawn("tPrintB", 250, 0, 0x8000, (FUNCPTR)tPrintProccess, PRINTER_B,1,2,3,4,5,6,7,8,9);
	//if(OK != taskIdVerify(param->tPrintId))		printf("%s: create task tPrintB fail!\n", __FUNCTION__);


	//return 0
}*/



#if 0
//��ӡ�������
static OilPrinterStruct OilPrinterA1, OilPrinterB1;

//˽�к����б�
int tOilPrint(int printer);


//****************************************************************************
*Name				:tOilPrint
*Description		:��ӡСƱ������ں���
*Input				:printer		0=1�Ŵ�ӡ����1=2�Ŵ�ӡ��
*Output			:None
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2014-12-15,modified by syj
*/
static int tOilPrint(int printer)
{
	OilPrinterStruct *istruct=NULL;
	char buffer[PRINT_MAX]={0};
	int len=0;

	//�жϴ�ӡ��ѡ��
	if(0==printer)			istruct=&OilPrinterA1;
	else if(1==printer)	istruct=&OilPrinterB1;
	else							return ERROR;

	//��ӡ������,fj:
	/*FOREVER
	{
		len=msgQReceive(istruct->msgId, buffer, PRINT_MAX, WAIT_FOREVER);
		if(len>0)
		{
			
		}

		taskDelay(1);
	}*/

	return 0;
}


//****************************************************************************
*Name				:oilPrint
*Description		:��ӡСƱ
*Input				:printer		��ӡ��0=1�Ŵ�ӡ��(A1)��1=2�Ŵ�ӡ��(B1)
*						:buffer		��ӡ����
*						:nbytes		��ӡ���ݳ���
*Output			:None
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2014-12-15,modified by syj
*/
int oilPrint(int printer, char *buffer, int nbytes)
{
	OilPrinterStruct *istruct=NULL;
	int i=0;

	//�жϴ�ӡ��ѡ��
	if(0==printer)			istruct=&OilPrinterA1;
	else if(1==printer)	istruct=&OilPrinterB1;
	else							return ERROR;

	//��Ӵ�ӡ����,fj:
	/*for(i=0; i<nbytes;)
	{
		if(i+PRINT_MAX<=nbytes)
		{
			msgQSend(istruct->msgId, &buffer[i], PRINT_MAX, NO_WAIT, MSG_PRI_NORMAL);
			i+=PRINT_MAX;
		}
		else
		{
			msgQSend(istruct->msgId, &buffer[i], nbytes-i, NO_WAIT, MSG_PRI_NORMAL);
			i+=(nbytes-i);
		}
	}*/

	return 0;
}


//****************************************************************************
*Name				:oilPrintModulesInit
*Description		:��ӡСƱ����ģ���ʼ��
*Input				:None
*Output			:None
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2014-12-15,modified by syj
*/
int oilPrintModulesInit(void)
{
	//��ʼ��1�Ŵ�ӡ����
	/*OilPrinterA1.ComFd=COM13;
	//��ʼ��1�Ŵ�ӡ����Ϣ����
	OilPrinterA1.msgId=msgQCreate(10, PRINT_MAX, MSG_Q_FIFO);
	if(NULL==OilPrinterA1.msgId)		printf("Error!	Creat messages  'OilPrinterA1.msgId' failed!\n");
	//��ʼ��1�Ŵ�ӡ����ӡ����
	OilPrinterA1.tId=taskSpawn("tPrintA1", 160, 0, 0x1000, (FUNCPTR)tOilPrint, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(OilPrinterA1.tId))		printf("Error!	Creat task 'tPrintA1' failed!\n");



	//��ʼ��2�Ŵ�ӡ����
	OilPrinterB1.ComFd=COM17;
	//��ʼ��2�Ŵ�ӡ����Ϣ����
	OilPrinterB1.msgId=msgQCreate(10, PRINT_MAX, MSG_Q_FIFO);
	if(NULL==OilPrinterB1.msgId)		printf("Error!	Creat messages  'OilPrinterB1.msgId' failed!\n");
	//��ʼ��2�Ŵ�ӡ����ӡ����
	OilPrinterB1.tId=taskSpawn("tPrintA1", 160, 0, 0x1000, (FUNCPTR)tOilPrint, 1,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(OilPrinterB1.tId))		printf("Error!	Creat task 'tPrintB1' failed!\n");*/

	return 0;
}
#endif

