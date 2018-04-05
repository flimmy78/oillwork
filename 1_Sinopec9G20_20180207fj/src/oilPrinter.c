//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilLog.h"
//#include "oilBoardTrans.h"
//#include "oilPrinter.h"

#include "../inc/main.h"

PrintParamStruct printParamA, printParamB;



/****************************************************************************
*Name				:tPrintProccess
*Description		:打印队列处理任务
*Input				:printer			0=A打印机(PRINTER_A)；1=B打印机(PRINTER_B)
*Output			:无
*Return				:无
*History			:2015-11-13,modified by syj
*/
void tPrintProcess(char printer)
{
	PrintParamStruct *param=NULL;
	PrintListNode *node=NULL;
	int lastbytes = 0;
	const PRINTER_MAX_SIZE_ONE_TIME = 1024;

	//判断设备选择
	if(PRINTER_A==printer)
	{
		param=&printParamA;
		param->comFd = COM13;  //fj:先注释
		prctl(PR_SET_NAME,(unsigned long)"tPrintProcessA");
	}
	else if(PRINTER_B==printer)
	{
		param=&printParamB;
		param->comFd = COM17;  //fj:先注释
        prctl(PR_SET_NAME,(unsigned long)"tPrintProcessB");
	}
	else
	{
		printf("%s:%d:printer select error!\n", __FUNCTION__, __LINE__);
		return;
	}

	//读取打印数据队列并完成打印数据的发送
	/*	为避免大数据量造成打印机缓存溢出，当打印数据超过单次允许发送的
	*	数据长度(PRINTER_MAX_SIZE_ONE_TIME)时分次打印并在中间增加一定的延时
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
				comWriteInTime(param->comFd, node->buffer + (node->length-lastbytes), PRINTER_MAX_SIZE_ONE_TIME, 2);  //fj:先注释
				lastbytes = lastbytes - PRINTER_MAX_SIZE_ONE_TIME;
			}
			else
			{
				printf("exe print,length = %d \n",lastbytes);
				comWriteInTime(param->comFd, node->buffer+ (node->length-lastbytes), lastbytes, 2); //fj:都在oilCom.c类里
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
*Description		:打印功能初始化
*Input				:printer		b7~b4为打印机主板号(1~3)；b3~b0为打印机选择(PRINTER_A/PRINTER_B)；
*						:inbuffer	需要打印的数据
*						:nbytes		需要打印的数据长度
*Output			:无
*Return				:成功返回OK；失败返回ERROR
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

	//打印机主板号和打印机面板号
	board = (printer>>4) & 0x0f;
	iprinter = printer & 0x0f;
	//判断设备选择
	if(PRINTER_A==iprinter)			
		param=&printParamA;
	else if(PRINTER_B==iprinter)	
		param=&printParamB;
	else
	{
		jljRunLog("printer select error!\n");
		return ERROR;
	}

	//获取本地主板号
	local_board_id = pcdMboardIDRead();  //fj:在PCD类里

	printf("local_board_id = %d,board = %d\n");

	//需要在其他主板连接的打印机打印数据的处理
	if(0!=board && local_board_id!=board)
	{
		for(i = 0; i < nbytes; i+=tx_len)
		{
			//fj:在oilBoardTrans里定义，先注释
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

	//在本地主板打印机打印的数据处理
	//判断未打印队列长度
	if(lstCount(&param->list) > PRINT_NODE_MAX)
	{
		jljRunLog("print list is full!\n");
		return ERROR;
	}
	//申请节点空间和节点打印数据并拷贝数据到申请的空间中
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
	lstAdd(&(param->list), (NODE*)node);	//将打印数据添加到打印队列中

	return OK;
}


/****************************************************************************
*Name				:printInit
*Description		:打印功能初始化
*Input				:无
*Output			:无
*Return				:成功返回0；失败返回其它值
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
//打印相关数据
static OilPrinterStruct OilPrinterA1, OilPrinterB1;

//私有函数列表
int tOilPrint(int printer);


//****************************************************************************
*Name				:tOilPrint
*Description		:打印小票任务入口函数
*Input				:printer		0=1号打印机；1=2号打印机
*Output			:None
*Return				:成功返回0；失败返回其它值
*History			:2014-12-15,modified by syj
*/
static int tOilPrint(int printer)
{
	OilPrinterStruct *istruct=NULL;
	char buffer[PRINT_MAX]={0};
	int len=0;

	//判断打印机选择
	if(0==printer)			istruct=&OilPrinterA1;
	else if(1==printer)	istruct=&OilPrinterB1;
	else							return ERROR;

	//打印任务处理,fj:
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
*Description		:打印小票
*Input				:printer		打印机0=1号打印机(A1)；1=2号打印机(B1)
*						:buffer		打印内容
*						:nbytes		打印内容长度
*Output			:None
*Return				:成功返回0；失败返回其它值
*History			:2014-12-15,modified by syj
*/
int oilPrint(int printer, char *buffer, int nbytes)
{
	OilPrinterStruct *istruct=NULL;
	int i=0;

	//判断打印机选择
	if(0==printer)			istruct=&OilPrinterA1;
	else if(1==printer)	istruct=&OilPrinterB1;
	else							return ERROR;

	//添加打印任务,fj:
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
*Description		:打印小票功能模块初始化
*Input				:None
*Output			:None
*Return				:成功返回0；失败返回其它值
*History			:2014-12-15,modified by syj
*/
int oilPrintModulesInit(void)
{
	//初始化1号打印数据
	/*OilPrinterA1.ComFd=COM13;
	//初始化1号打印机消息队列
	OilPrinterA1.msgId=msgQCreate(10, PRINT_MAX, MSG_Q_FIFO);
	if(NULL==OilPrinterA1.msgId)		printf("Error!	Creat messages  'OilPrinterA1.msgId' failed!\n");
	//初始化1号打印机打印任务
	OilPrinterA1.tId=taskSpawn("tPrintA1", 160, 0, 0x1000, (FUNCPTR)tOilPrint, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(OilPrinterA1.tId))		printf("Error!	Creat task 'tPrintA1' failed!\n");



	//初始化2号打印数据
	OilPrinterB1.ComFd=COM17;
	//初始化2号打印机消息队列
	OilPrinterB1.msgId=msgQCreate(10, PRINT_MAX, MSG_Q_FIFO);
	if(NULL==OilPrinterB1.msgId)		printf("Error!	Creat messages  'OilPrinterB1.msgId' failed!\n");
	//初始化2号打印机打印任务
	OilPrinterB1.tId=taskSpawn("tPrintA1", 160, 0, 0x1000, (FUNCPTR)tOilPrint, 1,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(OilPrinterB1.tId))		printf("Error!	Creat task 'tPrintB1' failed!\n");*/

	return 0;
}
#endif

