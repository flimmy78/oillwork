#ifndef _OIL_PRINTER_H_
#define _OIL_PRINTER_H_

#include "lstLib.h"



//��ӡ���豸ѡ��
#define PRINTER_A						0					//A��ӡ��
#define PRINTER_B						1					//B��ӡ��
//��ӡ���ݶ��������Ŀ
#define PRINT_NODE_MAX		10


//��ӡ���ݽڵ�
typedef struct{

	NODE	Ndptrs;
	char *buffer;					//��Ҫ��ӡ������
	int length;						//��Ҫ��ӡ�����ݳ���

}PrintListNode;


//��ӡ������ݽṹ
typedef struct{

	LIST list;							//��ӡ���ݶ���

	int comFd;						//��ӡ�����Ӵ���
	int tPrintId;						//��ӡ���ݶ��д�������

}PrintParamStruct;


extern PrintParamStruct printParamA;
extern PrintParamStruct printParamB;

extern void tPrintProcess(char printer);
extern int pprint(char printer, char *inbuffer, int nbytes);
//extern int printInit(void);

#endif

