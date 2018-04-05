/*************************************************************************
*	本文件实现串口的基本操作
*	使用者使用时只需调用comInit/comExit进行初始化/注销操作，并调动comWrite/comRead函数写/读数据
*	COM0~COM6为9g20自带串口，
*	COM3,COM4作为COM13~COM20母串口，一般的除本文件外不应对这两个木串口进行操作
*	以SPI1为母口通过stm32扩展得到COM7~COM12,所以此类串口的数据接收底层会添加到SPI1中
*/

//#include <ioLib.h>
//#include "oilCfg.h"
//#include "oilKb.h"
//#include "oilStmTransmit.h"
//#include "oilCom.h"

#include "../inc/main.h"
#include "../inc/uart.h"

pthread_mutex_t  g_comMutex = PTHREAD_MUTEX_INITIALIZER;


//串口设备符号
int gFdCom0=ERROR, gFdCom1=ERROR, gFdCom2=ERROR, gFdCom3=ERROR, gFdCom4=ERROR, gFdCom5=ERROR, gFdCom6=ERROR;

int gFdPcServerCom = ERROR;

#define FIOBAUDRATE 4
#define SIO_HW_OPTS_SET 0x1005
#define FIOFLUSH 2

//串口写数据互斥信号量
//SEM_ID SemIdWrCom0=NULL, SemIdWrCom1=NULL, SemIdWrCom2=NULL, SemIdWrCom3=NULL, SemIdWrCom4=NULL, SemIdWrCom5=NULL, SemIdWrCom6=NULL;


/*****************************************************************************
*Name				:comWrite
*Description		:串口设备数据写入，此函数因保护临界资源最大可能有500ms延时
*Input				:com_fd 		串口设备符号形如COMx参数
*						:buffer			发送数据缓存地址
*						:nbytes		发送数据长度
*Output			:None
*Return				:返回实际发送字数(不大于nbytes)，错误返回ERROR
*History			:2013-07-01
*/
int comWrite(int com_fd, char *buffer, int nbytes)
{
	int i=0;
	//STATUS state=0;
	//int TIME_OUT=sysClkRateGet()*500/1000;

	switch(com_fd)
	{
	case COM0:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom0, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM1:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom1, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM2:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom2, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM3:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom3, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM4: //fj:20171016
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom4, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);

		//printf("COM4 : i = %d\n",i);
		//PrintH(nbytes,buffer);
		//taskUnlock();
		break;
//	case COM4:
//		//taskLock();
//		pthread_mutex_lock(&g_comMutex);
//		i=write(gFdPcServerCom, buffer, nbytes);
//		pthread_mutex_unlock(&g_comMutex);
//		//taskUnlock();
//		break;
	case COM5:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom5, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM6:
		//taskLock();
		pthread_mutex_lock(&g_comMutex);
		i=write(gFdCom6, buffer, nbytes);
		pthread_mutex_unlock(&g_comMutex);
		//taskUnlock();
		break;
	case COM7:
		i=spi1UartWrite(STM_SPI1_PORT_USART1, buffer, nbytes);
		break;
	case COM8:
		i=spi1UartWrite(STM_SPI1_PORT_USART2, buffer, nbytes);
		break;
	case COM9:
		i=spi1UartWrite(STM_SPI1_PORT_USART3, buffer, nbytes);
		break;
	case COM10:
		i=spi1UartWrite(STM_SPI1_PORT_UART4, buffer, nbytes);
		break;
	case COM11:
		i=spi1UartWrite(STM_SPI1_PORT_UART5, buffer, nbytes);
		break;
	case COM12:
		i=spi1UartWrite(STM_SPI1_PORT_USART6, buffer, nbytes);
		break;
	case COM13:
		i=kbUartWrite(KB_USARTA1, buffer, nbytes);
		break;
	case COM14:
		//printf("IC COM14: \n");
		//PrintH(nbytes,buffer);
		i=kbUartWrite(KB_USARTA2, buffer, nbytes);
		break;
	case COM15:
		i=kbUartWrite(KB_USARTA4, buffer, nbytes);
		break;
//	case COM16:
//		i=kbUartWrite(KB_USARTA5, buffer, nbytes);
//		break;
//	case COM16: //fj:20170919,update
//		i = write(gFdPcServerCom,buffer,nbytes);
//		break;
	case COM16: //fj:20170923
        i = kbUartWrite(KB_USARTA5,buffer,nbytes);
		//printf("com16 ---\n");
		break;
	case COM17:
		i=kbUartWrite(KB_USARTB1, buffer, nbytes);
		break;
	case COM18:
		i=kbUartWrite(KB_USARTB2, buffer, nbytes);
		break;
	case COM19:
		i=kbUartWrite(KB_USARTB4, buffer, nbytes);
		break;
	case COM20:
		i=kbUartWrite(KB_USARTB5, buffer, nbytes);
		break;
	default:
		break;
	}
 
	return i; 
}


/*****************************************************************************
*Name				:comRead()
*Description		:串口设备数据读取
*Input				:com_fd 		串口设备符号形如COMx参数
*Output			:buffer			数据输出缓存地址
*						:maxbytes	数据输出最大长度
*Return				:返回实际读取字数(不大于maxbytes)，错误返回ERROR
*History			:2013-07-01
*/
int comRead(int com_fd, char *buffer, int maxbytes)
{
	int i=0;

	switch(com_fd)
	{
	case COM0:
		i=read(gFdCom0, buffer, maxbytes);
		break;
	case COM1:
		i=read(gFdCom1, buffer, maxbytes);
		break;
	case COM2:
		i=read(gFdCom2, buffer, maxbytes);
		break;
	case COM3:
		i=read(gFdCom3, buffer, maxbytes);
		break;
	case COM4: //串口4
		i=read(gFdCom4, buffer, maxbytes);
		break;
//    case COM4:  //fj:20171016
//		i = read(gFdPcServerCom,buffer,maxbytes);
//		break;
	case COM5: //串口5
		i=read(gFdCom5, buffer, maxbytes);
		break;
	case COM6: //串口6
		i=read(gFdCom6, buffer, maxbytes);
		break;
	case COM7: //COM7
		i=spi1UartRead(STM_SPI1_PORT_USART1, buffer, maxbytes);
		break;
	case COM8: //COM8
		i=spi1UartRead(STM_SPI1_PORT_USART2, buffer, maxbytes);
		break;
	case COM9: //COM9	
		i=spi1UartRead(STM_SPI1_PORT_USART3, buffer, maxbytes);
		break;
	case COM10: //COM10
		i=spi1UartRead(STM_SPI1_PORT_UART4, buffer, maxbytes);
		break;
	case COM11: //COM11	
		i=spi1UartRead(STM_SPI1_PORT_UART5, buffer, maxbytes);
		break;
	case COM12: //COM12
		i=spi1UartRead(STM_SPI1_PORT_USART6, buffer, maxbytes);
		break;
	case COM13: //COM13	
		i=kbUartRead(KB_USARTA1, buffer, maxbytes);
		break;
	case COM14: //COM14
		i=kbUartRead(KB_USARTA2, buffer, maxbytes);
		break;
	case COM15: //COM15
		i=kbUartRead(KB_USARTA4, buffer, maxbytes);
		break;
//	case COM16: //COM16
//		i=kbUartRead(KB_USARTA5, buffer, maxbytes);
//		break;
//	case COM16: //fj:20170919,update
//		i = read(gFdPcServerCom,buffer,maxbytes);
//		break;
	case COM16: //fj:20170923,
        i = kbUartRead(KB_USARTA5,buffer,maxbytes);
		break;
	case COM17: //COM17
		i=kbUartRead(KB_USARTB1, buffer, maxbytes);
		break;
	case COM18:
		i=kbUartRead(KB_USARTB2, buffer, maxbytes);
		break;
	case COM19:
		i=kbUartRead(KB_USARTB4, buffer, maxbytes);
		break;
	case COM20:
		i=kbUartRead(KB_USARTB5, buffer, maxbytes);
		break;
	default:
		break;
	}

	return i; 
}


/*****************************************************************************
*Name				:comRead()
*Description		:串口设备数据读取
*Input				:com_fd 		串口设备符号形如COMx参数
*Output			:buffer			数据输出缓存地址
*						:maxbytes	数据输出最大长度
*						:overtime		等待发送超时的秒数
*Return			:返回实际读取字数(不大于maxbytes)，错误返回ERROR
*History			:2013-07-01
*/
int comWriteInTime(int comfd, char *inbuffer, int nbytes, int overtime)
{
	int ilength = 0;
	ilength = comWrite(comfd, inbuffer, nbytes);
	return ilength;
}


/*****************************************************************************
*Name				:comSet
*Description		:串口设备参数设置
*Input				:com_fd 		串口设备符号形如COMx参数	COM0 ~ COM20
*						:baudrate		波特率 HEX
*						:databit			数据位 HEX
*						:stopbit			停止位 HEX
*						:paritybit		校验位 HEX
*Output			:无
*Return				:正确返回0，错误返回ERROR
*History			:2016-03-21
*/
int comSet(int com_fd, int baudrate, int databit, int stopbit, int paritybit)
{
	return 0;
}


/*****************************************************************************
*Name				:comGet
*Description		:串口设备参数获取
*Input				:com_fd 		串口设备符号形如COMx参数	COM0 ~ COM20
*Output			:baudrate		波特率 HEX
*						:databit			数据位 HEX
*						:stopbit			停止位 HEX
*						:paritybit		校验位 HEX
*Return				:正确返回0，错误返回ERROR
*History			:2016-03-21
*/
int comGet(int com_fd, int *baudrate, int *databit, int *stopbit, int *paritybit)
{
	return 0;
}


/*****************************************************************************
*Name				:comInit
*Description	:串口设备及任务初始化
*Input				:None
*Output			:None
*Return			:None
*History			:2013-07-01
*/
/*
void comInit()
{
	//串口1	初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom1)	
		gFdCom1=open(COM1_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom1))
	{
		ioctl(gFdCom1,FIOBAUDRATE,115200);
		ioctl(gFdCom1,SIO_HW_OPTS_SET,CS8);
		ioctl(gFdCom1,FIOFLUSH,0);


		//COM1发送信号量初始化
		//if(NULL==SemIdWrCom1)		SemIdWrCom1=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom1)		printf("Error!	Creat semaphore 'SemIdWrCom1' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM1' failed because %s can't open!\n", COM1_PATH);		
	}			

	//串口2初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom2)	gFdCom2=open(COM2_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom2))
	{
		ioctl(gFdCom2, FIOBAUDRATE, 9600);
		ioctl(gFdCom2, SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom2, FIOFLUSH, 0);

		//COM2发送信号量初始化
		//if(NULL==SemIdWrCom2)		SemIdWrCom2=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom2)		printf("Error!	Creat semaphore 'SemIdWrCom2' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM2' failed because %s can't open!\n", COM2_PATH);		
	}			


	//串口3初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom3)	gFdCom3=open(COM3_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom3))
	{
		ioctl(gFdCom3, FIOBAUDRATE, 9600);
		ioctl(gFdCom3,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom3, FIOFLUSH, 0);

		//COM3发送信号量初始化
		//if(NULL==SemIdWrCom3)		SemIdWrCom3=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom3)		printf("Error!	Creat semaphore 'SemIdWrCom3' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM3' failed because %s can't open!\n", COM3_PATH);		
	}			

	
	//串口4	(包括母串口COM4，子串口COM13~COM16)初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom4)	gFdCom4=open(COM4_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom4))
	{
		ioctl(gFdCom4, FIOBAUDRATE, 115200);
		ioctl(gFdCom4,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom4, FIOFLUSH, 0);

		//COM4发送信号量初始化
		//if(NULL==SemIdWrCom4)		SemIdWrCom4=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom4)		printf("Error!	Creat semaphore 'SemIdWrCom4' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM4&COM13&COM14&COM15&COM16' failed because %s can't open!\n", COM4_PATH);		
	}			


	//串口5	(包括母串口COM5，子串口COM17~COM20)初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom5)	gFdCom5=open(COM5_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom5))
	{
		ioctl(gFdCom5, FIOBAUDRATE, 115200);
		ioctl(gFdCom5,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom5, FIOFLUSH, 0);

		//COM5发送信号量初始化
		//if(NULL==SemIdWrCom5)		SemIdWrCom5=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom5)		printf("Error!	Creat semaphore 'SemIdWrCom5' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM5&COM17&COM18&COM19&COM20' failed because %s can't open!\n", COM5_PATH);		
	}			


	//串口6	初始化接收任务，此串口接收税控报税口数据
	//打开并设置数据位，校验位(偶校验)，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom6)	gFdCom6=open(COM6_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom6))
	{
		//printf("gFdCom6__FIOSETOPTIONS___%x\n", ioctl(gFdCom6,FIOSETOPTIONS,OPT_RAW));
		ioctl(gFdCom6, FIOFLUSH, 0);
		ioctl(gFdCom6, FIOBAUDRATE, 9600);
		ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB);
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,	SIO_HW_OPTS_SET, CS8|PARENB));
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB));

		//COM6发送信号量初始化
		//if(NULL==SemIdWrCom6)		SemIdWrCom6=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom6)		printf("Error!	Creat semaphore 'SemIdWrCom6' failed!\n");
	}
	else
	{
		printf("Error! Initialize 'COM6' failed because %s can't open!\n", COM6_PATH);		
	}			


	//fj:20170919,add
    if(ERROR == gFdPcServerCom)
	{
		char* dev[10] = {"/dev/ttyS9","/dev/ttyS1","/dev/ttyS3"};
		gFdPcServerCom = open_port(dev[0]);
		if(gFdPcServerCom < 0)
		{
			printf("Connect sinopec server is failed\n");
		}
		else
		{

			printf("Connect sinopec server is success!\n");
		}
	}


	return;	
}*/

void comInit()
{
	char* dev[10] = {"/dev/ttyS9","/dev/ttyS4","/dev/ttyS3"};
	
    if(ERROR == gFdCom4)//fj:20170919,add
	{	
		gFdCom4 = open_port(dev[0],115200);
		if(gFdCom4 < 0)
		{
			printf("Connect sinopec server is failed,Open ttyS9 is failed\n");
		}
		else
		{

			printf("Connect sinopec server is success,ttyS9!\n");
		}
	}

    if(ERROR == gFdCom5)//fj:20170919,add
	{	
		gFdCom5 = open_port(dev[1],115200);
		if(gFdCom5 < 0)
		{
			printf("Open ttyS4 is failed!\n");
		}
		else
		{
			printf("Open ttyS4 is success!\n");
		}
	}

	//串口1	初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
/*	if(ERROR==gFdCom1)	
		gFdCom1=open(COM1_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom1))
	{
		ioctl(gFdCom1,FIOBAUDRATE,115200);
		ioctl(gFdCom1,SIO_HW_OPTS_SET,CS8);
		ioctl(gFdCom1,FIOFLUSH,0);


		//COM1发送信号量初始化
		//if(NULL==SemIdWrCom1)		SemIdWrCom1=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom1)		printf("Error!	Creat semaphore 'SemIdWrCom1' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM1' failed because %s can't open!\n", COM1_PATH);		
	}			

	//串口2初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom2)	gFdCom2=open(COM2_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom2))
	{
		ioctl(gFdCom2, FIOBAUDRATE, 9600);
		ioctl(gFdCom2, SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom2, FIOFLUSH, 0);

		//COM2发送信号量初始化
		//if(NULL==SemIdWrCom2)		SemIdWrCom2=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom2)		printf("Error!	Creat semaphore 'SemIdWrCom2' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM2' failed because %s can't open!\n", COM2_PATH);		
	}			


	//串口3初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom3)	gFdCom3=open(COM3_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom3))
	{
		ioctl(gFdCom3, FIOBAUDRATE, 9600);
		ioctl(gFdCom3,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom3, FIOFLUSH, 0);

		//COM3发送信号量初始化
		//if(NULL==SemIdWrCom3)		SemIdWrCom3=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom3)		printf("Error!	Creat semaphore 'SemIdWrCom3' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM3' failed because %s can't open!\n", COM3_PATH);		
	}			

	
	//串口4	(包括母串口COM4，子串口COM13~COM16)初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom4)	gFdCom4=open(COM4_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom4))
	{
		ioctl(gFdCom4, FIOBAUDRATE, 115200);
		ioctl(gFdCom4,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom4, FIOFLUSH, 0);

		//COM4发送信号量初始化
		//if(NULL==SemIdWrCom4)		SemIdWrCom4=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom4)		printf("Error!	Creat semaphore 'SemIdWrCom4' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM4&COM13&COM14&COM15&COM16' failed because %s can't open!\n", COM4_PATH);		
	}			


	//串口5	(包括母串口COM5，子串口COM17~COM20)初始化接收任务
	//打开并设置数据位，校验位，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom5)	gFdCom5=open(COM5_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom5))
	{
		ioctl(gFdCom5, FIOBAUDRATE, 115200);
		ioctl(gFdCom5,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom5, FIOFLUSH, 0);

		//COM5发送信号量初始化
		//if(NULL==SemIdWrCom5)		SemIdWrCom5=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom5)		printf("Error!	Creat semaphore 'SemIdWrCom5' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM5&COM17&COM18&COM19&COM20' failed because %s can't open!\n", COM5_PATH);		
	}			


	//串口6	初始化接收任务，此串口接收税控报税口数据
	//打开并设置数据位，校验位(偶校验)，停止位，波特率，清空收发缓存
	if(ERROR==gFdCom6)	gFdCom6=open(COM6_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom6))
	{
		//printf("gFdCom6__FIOSETOPTIONS___%x\n", ioctl(gFdCom6,FIOSETOPTIONS,OPT_RAW));
		ioctl(gFdCom6, FIOFLUSH, 0);
		ioctl(gFdCom6, FIOBAUDRATE, 9600);
		ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB);
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,	SIO_HW_OPTS_SET, CS8|PARENB));
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB));

		//COM6发送信号量初始化
		//if(NULL==SemIdWrCom6)		SemIdWrCom6=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom6)		printf("Error!	Creat semaphore 'SemIdWrCom6' failed!\n");
	}
	else
	{
		printf("Error! Initialize 'COM6' failed because %s can't open!\n", COM6_PATH);		
	}*/			


	//fj:20170919,add
   // if(ERROR == gFdPcServerCom)
//	{
//	
//		gFdPcServerCom = open_port(dev[0]);
//		if(gFdPcServerCom < 0)
//		{
//			printf("Connect sinopec server is failed\n");
//		}
//		else
//		{
//
//			printf("Connect sinopec server is success!\n");
//		}
//	}




	return;	
}
/*****************************************************************************
*Name				:comExit
*Description	:串口设备及任务注销
*Input				:None
*Output			:None
*Return			:None
*History			:2013-07-01
*/
void comExit()
{

	/****************************************关闭串口*/
	//关闭COM0
	if(!(ERROR==gFdCom0))
	{
		 close(gFdCom0);	
		 gFdCom0=ERROR;
	}

	//COM0信号量注销
	//if(!(NULL==SemIdWrCom0))
	//{
	//	semDelete(SemIdWrCom0);	SemIdWrCom0=NULL;
	//}
	
	//关闭COM1
	if(!(ERROR==gFdCom1))
	{
		close(gFdCom1);	gFdCom1=ERROR;
	}
	
	//COM1信号量注销
	//if(!(NULL==SemIdWrCom1))
	//{
	//	semDelete(SemIdWrCom1);	SemIdWrCom1=NULL;
	//}
	
	//关闭COM2
	if(!(ERROR==gFdCom2))
	{
		close(gFdCom2);	gFdCom2=ERROR;
	}

	//COM2信号量注销
	//if(!(NULL==SemIdWrCom2))
	//{
	//	semDelete(SemIdWrCom2);	SemIdWrCom2=NULL;
	//}
	
	//关闭COM3
	if(!(ERROR==gFdCom3))
	{
		close(gFdCom3);	gFdCom3=ERROR;
	}
	
	//COM3信号量注销
	//if(!(NULL==SemIdWrCom3))
	//{
	//	semDelete(SemIdWrCom3);	SemIdWrCom3=NULL;
	//}
	
	//关闭COM4
	if(!(ERROR==gFdCom4))
	{
		close(gFdCom4);	gFdCom4=ERROR;
	}

	//COM3信号量注销
	//if(!(NULL==SemIdWrCom4))
	//{
	//	semDelete(SemIdWrCom4);	SemIdWrCom4=NULL;
	//}
	
	//关闭COM5
	if(!(ERROR==gFdCom5))
	{
		close(gFdCom5);	gFdCom5=ERROR;
	}
	
	//COM5信号量注销
	//if(!(NULL==SemIdWrCom5))
	//{
	//	semDelete(SemIdWrCom5);	SemIdWrCom5=NULL;
	//}
	
	//关闭COM6
	if(!(ERROR==gFdCom6))
	{
		close(gFdCom6);	gFdCom6=ERROR;
	}

	//COM6信号量注销
	//if(!(NULL==SemIdWrCom6))
	//{
	//	semDelete(SemIdWrCom6);	SemIdWrCom6=NULL;
	//}

	return;
}





