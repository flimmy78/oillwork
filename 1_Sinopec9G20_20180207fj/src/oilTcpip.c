
/*
*	��ģ���ṩ����ͨѶ���ܣ�ͨ������TCP/UDPͨѶ����ʵ�����ݵĶ�д������
*	������������ֻ�������ݡ��������յ������ݽ��л��棬�ͻ���ֻ�������ݡ������ͻ����е����ݷ��͸���������
*	��ģ�鹦�ܽӿڴ��¹�ϵͼ����:
*
*		_____ netSRead() ______														_____ netSWrite() ______
*		|										|														|										|
*		|										|														|										|
*	tTcpSerX()							|														|										|
*		|										|														|										|
*	tTcpSer()							tUdpSer()											tUdpSer()							tUdpSer()
*		|										|														|										|
*	netSTcpServerStart()		netSUdpServerStart()						netSUdpServerStart()		netSUdpServerStart()
*		|										|														|										|
*		|										|														|										|
*		|										|														|										|
*		___ netSServerStart() ___														____ netSClientStart() ____
*						|																										|
*						|																										|
*						|																										|
*						_____________________ netSINOPECStart() ________________
*
*
*/

//#include <socket.h>
//#include <stdio.h>
//#include <ioLib.h>
//#include <inetLib.h>
//#include <semLib.h>
//#include <rngLib.h>
//#include "oilCfg.h"
//#include "oilLog.h"
//#include "oilTcpip.h"


#include "../inc/main.h"


//�ͻ������ݽṹ
typedef struct
{
	char IsEnable;					//�Ƿ�����	0=δ���ã�1=������
	int  Protocol;					//ͨѶЭ������	NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
	int  Addr;						//��������ַ
	int  Port;						//�������˿ں�
	int  SocetFd;					//�׽���������
	int  RecvTid;					//���������

	//SEM_ID SemIDTx;				//���ͻ�������ź���
	pthread_mutex_t SemIDTx;        //���ͻ�������ź���
	RING_ID RngBufferTx;		    //���ͻ���
	//SEM_ID SemIDRx;				//���շ��ͻ�������ź���
	pthread_mutex_t SemIDRx;        //���ջ����ź���
	RING_ID RngBufferRx;		    //���ջ���

    bool bRecvThreadStart;          //�߳̿�ʼ���ձ�־
}NetsSClientStruct;

NetsSClientStruct NetSClient[NETS_PORT_MAX];


//�ӿ�����
//void tTcpRecv(NetsSClientStruct *param);
//void tTcpClient(int addr, int port, NetsSClientStruct *param);
static int  netSTcpClientStart(int addr, int port);
static int  netSClientStart(int protocol, int addr, int port);

/*****************************************************************************
*Name				:tTcpServer
*Description		:TCP�������������񣬼����ͻ��˵���������
*Input				:param		�˵�ַ�˶˿ں�ռ�õ����ݽṹ��ַ
*Output			:��
*Return			:��
*History			:2016-05-12,modified by syj
*/
//void tTcpRecv(NetsSClientStruct *param)
void tTcpRecv(void* argc)
{
	NetsSClientStruct* param = (NetsSClientStruct*)argc;
	char rxbuffer[NETS_BUFFER_SIZE] = {0};
	int rxlenght = 0;

	FOREVER
	{
		if(param->bRecvThreadStart == false)
		{
			return;
		}

		if(ERROR != param->SocetFd)
		{
			rxlenght = recv(param->SocetFd, rxbuffer, NETS_BUFFER_SIZE, 0);
		}
		
		if(rxlenght > 0)
		{
			//semTake(param->SemIDRx, WAIT_FOREVER);
			pthread_mutex_lock(&param->SemIDRx);
			rngBufPut(param->RngBufferRx, rxbuffer, rxlenght);
		    pthread_mutex_unlock(&param->SemIDRx);
			//semGive(param->SemIDRx);
	        //printf("tTcpRecv -- aaaaaa\n");
		}

		//taskDelay(1);
		usleep(1000);
	}

	exit(0);
}


/*****************************************************************************
*Name				:tTcpClient
*Description		:TCP�ͻ�������
*Input				:addr		������IP��ַ(HEX)
*						:port			�������˿ں�(HEX)
*						:param		�˵�ַ�˶˿ں�ռ�õ����ݽṹ��ַ
*Output			:��
*Return			:��
*History			:2016-05-12,modified by syj
*/
//void tTcpClient(int addr, int port, NetsSClientStruct *param)
void tTcpClient(void* argc)
{
	NetsSClientStruct* param = (NetsSClientStruct*)argc;
	int addr = param->Addr;
	int port = param->Port;
	char server_name[32] = {0};
	char taskname[32] = {0};
	struct sockaddr_in server_addr;
	int server_addr_size = 0;
	int istate = 0;

	char rdbuffer[NETS_BUFFER_SIZE] = {0};
	int rdlength = 0;

	//��������Ϣ����
	server_addr_size = sizeof(struct sockaddr_in);

	//תΪ���ʮ����
	sprintf(server_name, "%d.%d.%d.%d", (addr>>24)&0x000000ff, (addr>>16)&0x000000ff, (addr>>8)&0x000000ff, (addr>>0)&0x000000ff);

	printf("server_name = %s\n",server_name);
	printf("server_port = %d\n",port);

	//����socket
SOCKET_DO:
	jljRunLog("�ͻ������ڴ���SOCKET!\n");
	param->SocetFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ERROR == param->SocetFd)
	{
		//taskDelay(5*sysClkRateGet());
		sleep(5);
		goto SOCKET_DO;
	}
	jljRunLog("�ͻ��˴���SOCKET�ɹ�!\n");

	//printf("�ͻ��˴���SOCKET�ɹ�!\n");

	//���÷�������Ϣ
PARAM_DO:
	memset(&server_addr, 0, server_addr_size);
	server_addr.sin_family = AF_INET;
	//server_addr.sin_len = server_addr_size;
	server_addr.sin_addr.s_addr = inet_addr(server_name);
	server_addr.sin_port = htons(port);

	//printf("tTcpClient -- aaaaaa\n");

	//���ӷ�����
CONNECT_DO:
	jljRunLog("�ͻ����������ӷ�����[IP=%s][Port=%d]!\n", server_name, port);
	istate = connect(param->SocetFd, (struct sockaddr *)&server_addr, server_addr_size);
	if(ERROR == istate)
	{
		//taskDelay(sysClkRateGet());
		sleep(1);
		close(param->SocetFd);
		goto SOCKET_DO;
	}
	jljRunLog("�ͻ������ӷ������ɹ�[IP=%s][Port=%d]!\n", server_name, port);

	//printf("tTcpClient -- bbbbbbbb\n");
	
	param->bRecvThreadStart = true;
	pthread_t td_TcpRecv;
	pthread_create(&td_TcpRecv,NULL,(void*)tTcpRecv,(void*)param);

	//printf("tTcpClient -- ccccc\n");

	//������������
	/*if(OK != taskIdVerify(param->RecvTid))
	{
		jljRunLog("���ڴ������ݽ�������......\n");
		
		memset(taskname, 0,sizeof(taskname));
		strcpy(taskname + 0, "tTcpRx");
		sprintf(taskname + strlen(taskname), "%d", port);
		param->RecvTid = taskSpawn(taskname, NETS_TASK_PRIORITY, 0, NETS_TASK_STACK_SIZE, (FUNCPTR)tTcpRecv, (int)param,1,2,3,4,5,6,7,8,9);
		if(OK != taskIdVerify(param->RecvTid))
		{
			jljRunLog("������������ʧ��.\n");
			//taskDelay(sysClkRateGet());
			sleep(1);
			close(param->SocetFd);	
			param->SocetFd = ERROR;
			goto SOCKET_DO;
		}
		
		jljRunLog("�������ݽ�������ɹ�......\n");
	}*/
	
	//�������ݣ��ӷ��ͻ���ȡ�����ݣ����͵���̨
SEND_DO:
	while(1)
	{
		//semTake(param->SemIDTx, WAIT_FOREVER);
		pthread_mutex_lock(&param->SemIDTx);
		rdlength = rngBufGet(param->RngBufferTx, rdbuffer, NETS_BUFFER_SIZE);
		pthread_mutex_unlock(&param->SemIDTx);
		//semGive(param->SemIDTx);

		if(rdlength < 0)
		{
			//taskDelay(1);
			usleep(1000);
			continue;
		}
		//istate = send(param->SocetFd, rdbuffer, rdlength, 0, (struct sockaddr *)&server_addr, server_addr_size);
	/*	istate = send(param->SocetFd,rdbuffer,rdlength,0);
		if(ERROR == istate)
		{
			//taskDelay(sysClkRateGet());

			param->bRecvThreadStart = false;
			sleep(1);
			close(param->SocetFd);	
			param->SocetFd = ERROR;
			printf("bRecvThreadStart\n");
			goto SOCKET_DO;
		}*/

        istate = send(param->SocetFd,rdbuffer,rdlength,MSG_NOSIGNAL);
		if(istate < 0)
		{
			//taskDelay(sysClkRateGet());

			param->bRecvThreadStart = false;
			sleep(1);
			close(param->SocetFd);	
			param->SocetFd = ERROR;
			printf("bRecvThreadStart\n");
			goto SOCKET_DO;
		}

		//taskDelay(1);
		usleep(1000);
	}

	//while(1);

	exit(0);
}


/*****************************************************************************
*Name				:netSTcpClientStart
*Description		:����TCP�ͻ�������
*Input				:addr			IP��ַ(HEX)
*						:port				�˿ں�(HEX��1~65535)
*Output			:��
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-05-12,modified by syj
*/
static int netSTcpClientStart(int addr, int port)
{
	//printf("netSTcpClientStart --aaaaaaa\n");
	int protocol = NETS_PROTOCOL_TCP;
	int i = 0;
	int client_number = 0;
	int ipointer = 0;
	int fd = 0;
	int istate = 0;
	char taskname[32] = {0};
	int taskid = 0;

	//�ж�����ָ����ַָ���˿ڵĿͻ��������Ƿ��Ѵ������Ѵ�����ֱ�ӷ��سɹ�
	for(i = 0; i < NETS_PORT_MAX; i++)
	{
		if(1 == NetSClient[i].IsEnable && protocol == NetSClient[i].Protocol && addr == NetSClient[i].Addr && port == NetSClient[i].Port)
		{
			jljRunLog("������ͬ��ַ���˿ںź�Э��Ŀͻ��������Ѵ����������ظ�����!\n");
			return ERROR;
		}

		if(1 == NetSClient[i].IsEnable)
		{
			client_number++;
		}
	}

	//printf("netSTcpClientStart --bbbbbbb\n");

	//�ж��Ƿ���Դ�������Ŀͻ�������
	if(client_number >= NETS_PORT_MAX)
	{
		jljRunLog("�����޷���������Ŀͻ�������!\n");
		return ERROR;
	}

	//printf("netSTcpClientStart --ccccccc\n");

	//����δʹ�õ����ݽṹ�����Դ������ͻ������������
	for(i = 0; i < NETS_PORT_MAX; i++)
	{
		if(1 != NetSClient[i].IsEnable)
		{
			ipointer = i;
			break;
		}
	}

	//printf("ipointer = %d\n",ipointer);

	//���Ϊ������
	NetSClient[ipointer].IsEnable = 1;

	//��¼Э�����ͺͶ˿ں�
	NetSClient[ipointer].Protocol = protocol;
	NetSClient[ipointer].Port = port;
	NetSClient[ipointer].Addr = addr;

	//���������ź���
	//NetSClient[ipointer].SemIDTx = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == NetSClient[ipointer].SemIDTx)
	//{
	//	jljRunLog("���󣬴����ź���\"NetSClient[ipointer].SemIDTx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
	//	istate = ERROR;
	//	goto DONE;
	//}
	
	int nMutexTxRet = pthread_mutex_init(&(NetSClient[ipointer].SemIDTx),NULL);
	if(nMutexTxRet != 0)
	{
		jljRunLog("���󣬴����ź���\"NetSClient[ipointer].SemIDTx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}
	//printf("netSTcpClientStart --ddddddd\n");

	//���䷢�ͻ���
	NetSClient[ipointer].RngBufferTx = (RING_ID)rngCreate(NETS_BUFFER_SIZE);
	if(NULL == NetSClient[ipointer].RngBufferTx)
	{
		jljRunLog("���󣬷��仺��\"NetSClient[ipointer].RngBufferTx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	//printf("netSTcpClientStart --eeeeeee\n");

	//���������ź���
	//NetSClient[ipointer].SemIDRx = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == NetSClient[ipointer].SemIDRx)
	//{
	//	jljRunLog("���󣬴����ź���\"NetSClient[ipointer].SemIDRx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
	//	istate = ERROR;
	//	goto DONE;
	//}
	
	int nMutexRxRet = pthread_mutex_init(&(NetSClient[ipointer].SemIDRx),NULL);
	if(nMutexRxRet != 0)
	{
		jljRunLog("���󣬴����ź���\"NetSClient[ipointer].SemIDRx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	//printf("netSTcpClientStart --fffffff\n");
	
	//������ջ���
	NetSClient[ipointer].RngBufferRx = (RING_ID)rngCreate(NETS_BUFFER_SIZE);
	if(NULL == NetSClient[ipointer].RngBufferRx)
	{
		jljRunLog("���󣬷��仺��\"NetSClient[ipointer].RngBufferRx\"ʧ�ܣ��˳��ͻ���!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	//printf("netSTcpClientStart --gggggg\n");

	//�����ͻ�������
	//strcpy(taskname + 0, "tTcpCl");
	//sprintf(taskname + strlen(taskname), "%d", port);
	//taskid = taskSpawn(taskname, NETS_TASK_PRIORITY, 0, NETS_TASK_STACK_SIZE, (FUNCPTR)tTcpClient, addr, port, (int)&NetSClient[ipointer],3,4,5,6,7,8,9);
	//if(OK != taskIdVerify(taskid))
	//{
	//	jljRunLog("���󣬴����ͻ�������ʧ�ܣ��˳��ͻ���!\n");
	//	istate = ERROR;
	//	goto DONE;
	//}
	
	pthread_t td_TcpClient;
	pthread_create(&td_TcpClient,NULL,(void*)tTcpClient,(void*)&NetSClient[ipointer]);


	//pthread_t td_TcpRecv;
	//pthread_create(&td_TcpRecv,NULL,(void*)tTcpRecv,(void*)&NetSClient[ipointer]);

	//printf("netSTcpClientStart --hhhhhhhh\n");

DONE:
	//if(0 != istate && OK == taskIdVerify(taskid))
	//{
	//	taskDelete(taskid);
	//	taskid = 0;
	//}
	if(0 != istate && NULL != NetSClient[ipointer].RngBufferTx)
	{
		free(NetSClient[ipointer].RngBufferTx);
	}
	//if(0 != istate && NULL != NetSClient[ipointer].SemIDTx)
	//{
	//	semDelete(NetSClient[ipointer].SemIDTx);
	//	NetSClient[ipointer].SemIDTx = NULL;
	//}
	if(0 != istate)
	{
		NetSClient[ipointer].IsEnable = 0;
		NetSClient[ipointer].Protocol = 0;
		NetSClient[ipointer].Addr = 0;
		NetSClient[ipointer].Port = 0;
	}

	//printf("netSTcpClientStart --iiiiiiiiiiiii\n");

	return 0;
}


/*****************************************************************************
*Name				:netSUdpClientStart
*Description		:�����ͻ�������
*Input				:protocol		ͨѶЭ�� NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:addr			IP��ַ(HEX)
*						:port				�˿ں�(HEX��1~65535)
*Output			:None
*Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-05-12,modified by syj
*/
static int netSClientStart(int protocol, int addr, int port)
{
	int istate = 0;

	//�ж�Э������
	if(NETS_PROTOCOL_UDP != protocol && NETS_PROTOCOL_TCP != protocol)
	{
		jljRunLog("[function=%s][line=%d]the param 'protocol = %x' is invalid!\n", __FUNCTION__, __LINE__, protocol);
		return ERROR;
	}

	//����UDP������
	if(NETS_PROTOCOL_UDP == protocol)
	{
		jljRunLog("[function=%s][line=%d]��ʱ��ʵ��UDP����!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//����TCP������
	if(NETS_PROTOCOL_TCP == protocol)
	{
		istate = netSTcpClientStart(addr, port);
	}

	return istate;
}


/*****************************************************************************
*Name				:netSWrite
*Description		:������������
*Input				:protocol		ͨѶЭ�� NETS_PROTOCOL_UDP / NETS_PROTOCOL_TCP
*						:port				�˿ں� HEX
*						:inbuffer		�������ݻ���
*						:nbytes			�������ݻ��泤��
*Output			:None
*Return				:ʵ�ʷ��ͳ��ȣ�ʧ�ܷ���ERROR
*History			:2016-05-12,modified by syj
*/
int netSWrite(int protocol, int port, char *inbuffer, int nbytes)
{
	int ipointer = 0, ilength = 0, i = 0;

	//�жϴ˶˿��Ƿ�������
	for(i = 0; i < NETS_PORT_MAX;)
	{
		if(protocol == NetSClient[i].Protocol && port == NetSClient[i].Port)
		{
			ipointer = i;
			break;
		}

		i++;

		if(i >= NETS_PORT_MAX)
		{
			jljRunLog("Error!%s protocol %x port %d communication is invalid!\n", __FUNCTION__, protocol, port);
			return ERROR;
		}
	}

	//�ж��Ƿ�������
	if(0 == NetSClient[ipointer].IsEnable)
	{
		jljRunLog("NetSSend[%x] is not enable!\n", ipointer);
		return ERROR;
	}

	//�ж��ź���
	//if(NULL == NetSClient[ipointer].SemIDRx)
	//{
	//	jljRunLog("NetSClient[%x] semaphore is invalid!\n", ipointer);
	//	return ERROR;
	//}

	//�жϻ������Ƿ�Ϸ�
	if(NULL == NetSClient[ipointer].RngBufferTx)
	{
		jljRunLog("NetSClient[%x] data cache is invalid!\n", ipointer);
		return ERROR;
	}

	//��������
	//semTake(NetSClient[ipointer].SemIDTx, WAIT_FOREVER);
	pthread_mutex_lock(&NetSClient[ipointer].SemIDTx);
	ilength = rngBufPut(NetSClient[ipointer].RngBufferTx, inbuffer, nbytes);
	pthread_mutex_unlock(&NetSClient[ipointer].SemIDTx);
	//semGive(NetSClient[ipointer].SemIDTx);

	return ilength;
}


/*****************************************************************************
*Name				:netSRead
*Description		:��ȡ��������
*Input				:protocol		ͨѶЭ�� NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:port				�˿ں� HEX
*						:maxbytes	������ݻ��泤��
*Output			:outbuffer		������ݻ���
*Return				:ʵ�ʻ�ȡ�������ݳ���
*History			:2016-05-12,modified by syj
*/
int netSRead(int protocol, int port, char *outbuffer, int maxbytes)
{
	int ipointer = 0, ilength = 0, i = 0;

	for(i = 0; i < NETS_PORT_MAX;)
	{
		if(protocol == NetSClient[i].Protocol && port == NetSClient[i].Port)
		{
			ipointer = i;
			break;
		}

		i++;

		if(i >= NETS_PORT_MAX)
		{
			jljRunLog("Error!	%s protocol %x port %d communication is invalid!\n", __FUNCTION__, protocol, port);
			return ERROR;
		}
	}

	//�ж��Ƿ�������
	if(0 == NetSClient[ipointer].IsEnable)
	{
		jljRunLog("NetSClient[%x] is not enable!\n", ipointer);
		return ERROR;
	}

	//�ж��ź���
	//if(NULL == NetSClient[ipointer].SemIDRx)
	//{
	//	jljRunLog("NetSClient[%x] semaphore is invalid!\n", ipointer);
	//	return ERROR;
	//}

	//�жϻ������Ƿ�Ϸ�
	if(NULL == NetSClient[ipointer].RngBufferRx)
	{
		jljRunLog("NetSClient[%x] data cache is invalid!\n", ipointer);
		return ERROR;
	}

	//�ȴ�����
	while(0 == rngNBytes(NetSClient[ipointer].RngBufferRx))
	{
		//taskDelay(1);
		usleep(1000);
	}

	//��ȡ����
	//semTake(NetSClient[ipointer].SemIDRx, WAIT_FOREVER);
	pthread_mutex_lock(&NetSClient[ipointer].SemIDRx);
	ilength = rngBufGet(NetSClient[ipointer].RngBufferRx, outbuffer, maxbytes);
	pthread_mutex_unlock(&NetSClient[ipointer].SemIDRx);
	//semGive(NetSClient[ipointer].SemIDRx);

	return ilength;
}


/*****************************************************************************
*Name				:netSKJLDInit
*Description		:�������ӿ���������̨���ܳ�ʼ������̨���ͻ�
*						:����TCPͨѶ�����ͻ�Ϊ�ͻ��ˣ���̨Ϊ��������
*Input				:protocol		ͨѶЭ�� NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:addr			������IP��ַ
*						:port				�������˿ں�(HEX��1~65535)
*						:localport		���ط������˿ں�(HEX��1~65535)
*Output			:None
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-05-12,modified by syj
*/
int netSKJLDInit(int protocol, unsigned int addr, unsigned int port, unsigned int localport)
{
	int istate = 0;

	istate = netSClientStart(NETS_PROTOCOL_TCP, addr, port);

	return istate;
}
