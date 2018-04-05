/*************************************************************************
*	���ļ�ʵ�ִ��ڵĻ�������
*	ʹ����ʹ��ʱֻ�����comInit/comExit���г�ʼ��/ע��������������comWrite/comRead����д/������
*	COM0~COM6Ϊ9g20�Դ����ڣ�
*	COM3,COM4��ΪCOM13~COM20ĸ���ڣ�һ��ĳ����ļ��ⲻӦ��������ľ���ڽ��в���
*	��SPI1Ϊĸ��ͨ��stm32��չ�õ�COM7~COM12,���Դ��മ�ڵ����ݽ��յײ����ӵ�SPI1��
*/

//#include <ioLib.h>
//#include "oilCfg.h"
//#include "oilKb.h"
//#include "oilStmTransmit.h"
//#include "oilCom.h"

#include "../inc/main.h"
#include "../inc/uart.h"

pthread_mutex_t  g_comMutex = PTHREAD_MUTEX_INITIALIZER;


//�����豸����
int gFdCom0=ERROR, gFdCom1=ERROR, gFdCom2=ERROR, gFdCom3=ERROR, gFdCom4=ERROR, gFdCom5=ERROR, gFdCom6=ERROR;

int gFdPcServerCom = ERROR;

#define FIOBAUDRATE 4
#define SIO_HW_OPTS_SET 0x1005
#define FIOFLUSH 2

//����д���ݻ����ź���
//SEM_ID SemIdWrCom0=NULL, SemIdWrCom1=NULL, SemIdWrCom2=NULL, SemIdWrCom3=NULL, SemIdWrCom4=NULL, SemIdWrCom5=NULL, SemIdWrCom6=NULL;


/*****************************************************************************
*Name				:comWrite
*Description		:�����豸����д�룬�˺����򱣻��ٽ���Դ��������500ms��ʱ
*Input				:com_fd 		�����豸��������COMx����
*						:buffer			�������ݻ����ַ
*						:nbytes		�������ݳ���
*Output			:None
*Return				:����ʵ�ʷ�������(������nbytes)�����󷵻�ERROR
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
*Description		:�����豸���ݶ�ȡ
*Input				:com_fd 		�����豸��������COMx����
*Output			:buffer			������������ַ
*						:maxbytes	���������󳤶�
*Return				:����ʵ�ʶ�ȡ����(������maxbytes)�����󷵻�ERROR
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
	case COM4: //����4
		i=read(gFdCom4, buffer, maxbytes);
		break;
//    case COM4:  //fj:20171016
//		i = read(gFdPcServerCom,buffer,maxbytes);
//		break;
	case COM5: //����5
		i=read(gFdCom5, buffer, maxbytes);
		break;
	case COM6: //����6
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
*Description		:�����豸���ݶ�ȡ
*Input				:com_fd 		�����豸��������COMx����
*Output			:buffer			������������ַ
*						:maxbytes	���������󳤶�
*						:overtime		�ȴ����ͳ�ʱ������
*Return			:����ʵ�ʶ�ȡ����(������maxbytes)�����󷵻�ERROR
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
*Description		:�����豸��������
*Input				:com_fd 		�����豸��������COMx����	COM0 ~ COM20
*						:baudrate		������ HEX
*						:databit			����λ HEX
*						:stopbit			ֹͣλ HEX
*						:paritybit		У��λ HEX
*Output			:��
*Return				:��ȷ����0�����󷵻�ERROR
*History			:2016-03-21
*/
int comSet(int com_fd, int baudrate, int databit, int stopbit, int paritybit)
{
	return 0;
}


/*****************************************************************************
*Name				:comGet
*Description		:�����豸������ȡ
*Input				:com_fd 		�����豸��������COMx����	COM0 ~ COM20
*Output			:baudrate		������ HEX
*						:databit			����λ HEX
*						:stopbit			ֹͣλ HEX
*						:paritybit		У��λ HEX
*Return				:��ȷ����0�����󷵻�ERROR
*History			:2016-03-21
*/
int comGet(int com_fd, int *baudrate, int *databit, int *stopbit, int *paritybit)
{
	return 0;
}


/*****************************************************************************
*Name				:comInit
*Description	:�����豸�������ʼ��
*Input				:None
*Output			:None
*Return			:None
*History			:2013-07-01
*/
/*
void comInit()
{
	//����1	��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom1)	
		gFdCom1=open(COM1_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom1))
	{
		ioctl(gFdCom1,FIOBAUDRATE,115200);
		ioctl(gFdCom1,SIO_HW_OPTS_SET,CS8);
		ioctl(gFdCom1,FIOFLUSH,0);


		//COM1�����ź�����ʼ��
		//if(NULL==SemIdWrCom1)		SemIdWrCom1=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom1)		printf("Error!	Creat semaphore 'SemIdWrCom1' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM1' failed because %s can't open!\n", COM1_PATH);		
	}			

	//����2��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom2)	gFdCom2=open(COM2_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom2))
	{
		ioctl(gFdCom2, FIOBAUDRATE, 9600);
		ioctl(gFdCom2, SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom2, FIOFLUSH, 0);

		//COM2�����ź�����ʼ��
		//if(NULL==SemIdWrCom2)		SemIdWrCom2=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom2)		printf("Error!	Creat semaphore 'SemIdWrCom2' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM2' failed because %s can't open!\n", COM2_PATH);		
	}			


	//����3��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom3)	gFdCom3=open(COM3_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom3))
	{
		ioctl(gFdCom3, FIOBAUDRATE, 9600);
		ioctl(gFdCom3,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom3, FIOFLUSH, 0);

		//COM3�����ź�����ʼ��
		//if(NULL==SemIdWrCom3)		SemIdWrCom3=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom3)		printf("Error!	Creat semaphore 'SemIdWrCom3' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM3' failed because %s can't open!\n", COM3_PATH);		
	}			

	
	//����4	(����ĸ����COM4���Ӵ���COM13~COM16)��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom4)	gFdCom4=open(COM4_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom4))
	{
		ioctl(gFdCom4, FIOBAUDRATE, 115200);
		ioctl(gFdCom4,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom4, FIOFLUSH, 0);

		//COM4�����ź�����ʼ��
		//if(NULL==SemIdWrCom4)		SemIdWrCom4=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom4)		printf("Error!	Creat semaphore 'SemIdWrCom4' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM4&COM13&COM14&COM15&COM16' failed because %s can't open!\n", COM4_PATH);		
	}			


	//����5	(����ĸ����COM5���Ӵ���COM17~COM20)��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom5)	gFdCom5=open(COM5_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom5))
	{
		ioctl(gFdCom5, FIOBAUDRATE, 115200);
		ioctl(gFdCom5,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom5, FIOFLUSH, 0);

		//COM5�����ź�����ʼ��
		//if(NULL==SemIdWrCom5)		SemIdWrCom5=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom5)		printf("Error!	Creat semaphore 'SemIdWrCom5' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM5&COM17&COM18&COM19&COM20' failed because %s can't open!\n", COM5_PATH);		
	}			


	//����6	��ʼ���������񣬴˴��ڽ���˰�ر�˰������
	//�򿪲���������λ��У��λ(żУ��)��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom6)	gFdCom6=open(COM6_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom6))
	{
		//printf("gFdCom6__FIOSETOPTIONS___%x\n", ioctl(gFdCom6,FIOSETOPTIONS,OPT_RAW));
		ioctl(gFdCom6, FIOFLUSH, 0);
		ioctl(gFdCom6, FIOBAUDRATE, 9600);
		ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB);
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,	SIO_HW_OPTS_SET, CS8|PARENB));
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB));

		//COM6�����ź�����ʼ��
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

	//����1	��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
/*	if(ERROR==gFdCom1)	
		gFdCom1=open(COM1_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom1))
	{
		ioctl(gFdCom1,FIOBAUDRATE,115200);
		ioctl(gFdCom1,SIO_HW_OPTS_SET,CS8);
		ioctl(gFdCom1,FIOFLUSH,0);


		//COM1�����ź�����ʼ��
		//if(NULL==SemIdWrCom1)		SemIdWrCom1=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom1)		printf("Error!	Creat semaphore 'SemIdWrCom1' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM1' failed because %s can't open!\n", COM1_PATH);		
	}			

	//����2��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom2)	gFdCom2=open(COM2_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom2))
	{
		ioctl(gFdCom2, FIOBAUDRATE, 9600);
		ioctl(gFdCom2, SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom2, FIOFLUSH, 0);

		//COM2�����ź�����ʼ��
		//if(NULL==SemIdWrCom2)		SemIdWrCom2=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom2)		printf("Error!	Creat semaphore 'SemIdWrCom2' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM2' failed because %s can't open!\n", COM2_PATH);		
	}			


	//����3��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom3)	gFdCom3=open(COM3_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom3))
	{
		ioctl(gFdCom3, FIOBAUDRATE, 9600);
		ioctl(gFdCom3,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom3, FIOFLUSH, 0);

		//COM3�����ź�����ʼ��
		//if(NULL==SemIdWrCom3)		SemIdWrCom3=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom3)		printf("Error!	Creat semaphore 'SemIdWrCom3' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM3' failed because %s can't open!\n", COM3_PATH);		
	}			

	
	//����4	(����ĸ����COM4���Ӵ���COM13~COM16)��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom4)	gFdCom4=open(COM4_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom4))
	{
		ioctl(gFdCom4, FIOBAUDRATE, 115200);
		ioctl(gFdCom4,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom4, FIOFLUSH, 0);

		//COM4�����ź�����ʼ��
		//if(NULL==SemIdWrCom4)		SemIdWrCom4=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom4)		printf("Error!	Creat semaphore 'SemIdWrCom4' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM4&COM13&COM14&COM15&COM16' failed because %s can't open!\n", COM4_PATH);		
	}			


	//����5	(����ĸ����COM5���Ӵ���COM17~COM20)��ʼ����������
	//�򿪲���������λ��У��λ��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom5)	gFdCom5=open(COM5_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom5))
	{
		ioctl(gFdCom5, FIOBAUDRATE, 115200);
		ioctl(gFdCom5,	 SIO_HW_OPTS_SET, CS8);
		ioctl(gFdCom5, FIOFLUSH, 0);

		//COM5�����ź�����ʼ��
		//if(NULL==SemIdWrCom5)		SemIdWrCom5=semMCreate(SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
		//if(NULL==SemIdWrCom5)		printf("Error!	Creat semaphore 'SemIdWrCom5' failed!\n");
	}
	else 
	{
		printf("Error! Initialize 'COM5&COM17&COM18&COM19&COM20' failed because %s can't open!\n", COM5_PATH);		
	}			


	//����6	��ʼ���������񣬴˴��ڽ���˰�ر�˰������
	//�򿪲���������λ��У��λ(żУ��)��ֹͣλ�������ʣ�����շ�����
	if(ERROR==gFdCom6)	gFdCom6=open(COM6_PATH,	O_RDWR,	0);
	if(!(ERROR==gFdCom6))
	{
		//printf("gFdCom6__FIOSETOPTIONS___%x\n", ioctl(gFdCom6,FIOSETOPTIONS,OPT_RAW));
		ioctl(gFdCom6, FIOFLUSH, 0);
		ioctl(gFdCom6, FIOBAUDRATE, 9600);
		ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB);
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,	SIO_HW_OPTS_SET, CS8|PARENB));
		//printf("gFdCom6__SIO_HW_OPTS_SET___%x\n", ioctl(gFdCom6,SIO_HW_OPTS_SET, CLOCAL|CREAD|CS8|PARENB));

		//COM6�����ź�����ʼ��
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
*Description	:�����豸������ע��
*Input				:None
*Output			:None
*Return			:None
*History			:2013-07-01
*/
void comExit()
{

	/****************************************�رմ���*/
	//�ر�COM0
	if(!(ERROR==gFdCom0))
	{
		 close(gFdCom0);	
		 gFdCom0=ERROR;
	}

	//COM0�ź���ע��
	//if(!(NULL==SemIdWrCom0))
	//{
	//	semDelete(SemIdWrCom0);	SemIdWrCom0=NULL;
	//}
	
	//�ر�COM1
	if(!(ERROR==gFdCom1))
	{
		close(gFdCom1);	gFdCom1=ERROR;
	}
	
	//COM1�ź���ע��
	//if(!(NULL==SemIdWrCom1))
	//{
	//	semDelete(SemIdWrCom1);	SemIdWrCom1=NULL;
	//}
	
	//�ر�COM2
	if(!(ERROR==gFdCom2))
	{
		close(gFdCom2);	gFdCom2=ERROR;
	}

	//COM2�ź���ע��
	//if(!(NULL==SemIdWrCom2))
	//{
	//	semDelete(SemIdWrCom2);	SemIdWrCom2=NULL;
	//}
	
	//�ر�COM3
	if(!(ERROR==gFdCom3))
	{
		close(gFdCom3);	gFdCom3=ERROR;
	}
	
	//COM3�ź���ע��
	//if(!(NULL==SemIdWrCom3))
	//{
	//	semDelete(SemIdWrCom3);	SemIdWrCom3=NULL;
	//}
	
	//�ر�COM4
	if(!(ERROR==gFdCom4))
	{
		close(gFdCom4);	gFdCom4=ERROR;
	}

	//COM3�ź���ע��
	//if(!(NULL==SemIdWrCom4))
	//{
	//	semDelete(SemIdWrCom4);	SemIdWrCom4=NULL;
	//}
	
	//�ر�COM5
	if(!(ERROR==gFdCom5))
	{
		close(gFdCom5);	gFdCom5=ERROR;
	}
	
	//COM5�ź���ע��
	//if(!(NULL==SemIdWrCom5))
	//{
	//	semDelete(SemIdWrCom5);	SemIdWrCom5=NULL;
	//}
	
	//�ر�COM6
	if(!(ERROR==gFdCom6))
	{
		close(gFdCom6);	gFdCom6=ERROR;
	}

	//COM6�ź���ע��
	//if(!(NULL==SemIdWrCom6))
	//{
	//	semDelete(SemIdWrCom6);	SemIdWrCom6=NULL;
	//}

	return;
}





