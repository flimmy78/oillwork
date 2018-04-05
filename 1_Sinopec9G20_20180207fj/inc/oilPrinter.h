#ifndef _OIL_PRINTER_H_
#define _OIL_PRINTER_H_

#include "lstLib.h"



//打印机设备选择
#define PRINTER_A						0					//A打印机
#define PRINTER_B						1					//B打印机
//打印数据队列最大数目
#define PRINT_NODE_MAX		10


//打印数据节点
typedef struct{

	NODE	Ndptrs;
	char *buffer;					//需要打印的数据
	int length;						//需要打印的数据长度

}PrintListNode;


//打印相关数据结构
typedef struct{

	LIST list;							//打印数据队列

	int comFd;						//打印机连接串口
	int tPrintId;						//打印数据队列处理任务

}PrintParamStruct;


extern PrintParamStruct printParamA;
extern PrintParamStruct printParamB;

extern void tPrintProcess(char printer);
extern int pprint(char printer, char *inbuffer, int nbytes);
//extern int printInit(void);

#endif

