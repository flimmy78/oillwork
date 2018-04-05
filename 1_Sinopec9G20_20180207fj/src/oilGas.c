//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilGas.h"

#include "../inc/main.h"


void tGasPoll(void);

void tGasPollRecv(void)
{
	char rxbuffer[16 + 1] = {0};
	int plen = 0;
	int i = 0;

	FOREVER
	{
		memset(rxbuffer, 0, 16);	plen = 0;
		//plen = comRead(COM1, rxbuffer, 16);  //fj:先注释
		printf("[%s][%d]Recv[plen = %x]:", __FUNCTION__, __LINE__, plen);
		for(i = 0; i < plen; i++)	printf(":%x", rxbuffer[i]);
		printf(".\n");
	
		//taskDelay(1);
		usleep(1000);   //fj:
	}

	return;
}


/********************************************************************
*Name				:tGasPoll
*Description		:油气回收功能初始化
*Input				:None
*Output			:None
*Return			:None
*History			:2016-10-12,modified by syj
*/
void tGasPoll(void)
{
	char rxbuffer[16 + 1] = {0};
	int plen = 0;
	int i = 0;

	FOREVER
	{
		i++;
	
		//comWrite(COM1, "0123456789", 10); //fj:
		printf("[%s][%d][%x]Send Over.\n", __FUNCTION__, __LINE__, i);
		
		//taskDelay(sysClkRateGet());
		//taskDelay(1);
		sleep(1); //fj:
	}

	return;
}


/********************************************************************
*Name				:gasInit
*Description		:油气回收功能初始化
*Input				:None
*Output			:None
*Return			:None
*History			:2016-10-12,modified by syj
*/
void gasInit(void)
{
	int tid = 0;

	//tid = taskSpawn("tGas", 166, 0, 0x1000, (FUNCPTR)tGasPoll, 0,1,2,3,4,5,6,7,8,9);
	//if(OK != taskIdVerify(tid))		printf("Error!	Creat task 'tGas' failed!\n");

	return;
}


