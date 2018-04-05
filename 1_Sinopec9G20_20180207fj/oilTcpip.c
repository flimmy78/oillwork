
/*
*	本模块提供网络通讯功能，通过创建TCP/UDP通讯任务实现数据的读写操作，
*	本处服务器端只接收数据、并将接收到的数据进行缓存，客户端只发送数据、将发送缓存中的数据发送给服务器；
*	本模块功能接口大致关系图如下:
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


//客户端数据结构
typedef struct
{
	char IsEnable;					//是否启用	0=未启用；1=已启用

	int Protocol;						//通讯协议类型	NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP

	int Addr;								//服务器地址

	int Port;								//服务器端口号

	int SocetFd;						//套接字描述符

	int RecvTid;						/*接收任务号*/

	SEM_ID SemIDTx;				/*发送缓存操作信号量*/

	RING_ID RngBufferTx;		/*发送缓存*/

	SEM_ID SemIDRx;				/*接收发送缓存操作信号量*/

	RING_ID RngBufferRx;		/*接收缓存*/
	
}NetsSClientStruct;

NetsSClientStruct NetSClient[NETS_PORT_MAX];


/*接口声明*/
LOCAL void tTcpRecv(NetsSClientStruct *param);
LOCAL void tTcpClient(int addr, int port, NetsSClientStruct *param);
LOCAL int netSTcpClientStart(int addr, int port);
LOCAL int netSClientStart(int protocol, int addr, int port);

/*****************************************************************************
*Name				:tTcpServer
*Description		:TCP服务器监听任务，监听客户端的连接请求
*Input				:param		此地址此端口号占用的数据结构地址
*Output			:无
*Return			:无
*History			:2016-05-12,modified by syj
*/
LOCAL void tTcpRecv(NetsSClientStruct *param)
{
	char rxbuffer[NETS_BUFFER_SIZE] = {0};
	int rxlenght = 0;

	FOREVER
	{
		if(ERROR != param->SocetFd)
		{
			rxlenght = recv(param->SocetFd, rxbuffer, NETS_BUFFER_SIZE, 0);
		}
		
		if(rxlenght > 0)
		{
			semTake(param->SemIDRx, WAIT_FOREVER);
			rngBufPut(param->RngBufferRx, rxbuffer, rxlenght);
			semGive(param->SemIDRx);
		}

		taskDelay(1);
	}

	exit(0);
}


/*****************************************************************************
*Name				:tTcpClient
*Description		:TCP客户端任务
*Input				:addr		服务器IP地址(HEX)
*						:port			服务器端口号(HEX)
*						:param		此地址此端口号占用的数据结构地址
*Output			:无
*Return			:无
*History			:2016-05-12,modified by syj
*/
LOCAL void tTcpClient(int addr, int port, NetsSClientStruct *param)
{
	char server_name[32] = {0};
	char taskname[32] = {0};
	struct sockaddr_in server_addr;
	int server_addr_size = 0;
	int istate = 0;

	char rdbuffer[NETS_BUFFER_SIZE] = {0};
	int rdlength = 0;

	/*服务器信息长度*/
	server_addr_size = sizeof(struct sockaddr_in);

	/*转为点分十进制*/
	sprintf(server_name, "%d.%d.%d.%d", (addr>>24)&0x000000ff, (addr>>16)&0x000000ff, (addr>>8)&0x000000ff, (addr>>0)&0x000000ff);

	/*创建socket*/
SOCKET_DO:
	jljRunLog("客户端正在创建SOCKET!\n");
	param->SocetFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ERROR == param->SocetFd)
	{
		taskDelay(5*sysClkRateGet());
		goto SOCKET_DO;
	}
	jljRunLog("客户端创建SOCKET成功!\n");

	/*设置服务器信息*/
PARAM_DO:
	memset(&server_addr, 0, server_addr_size);
	server_addr.sin_family = AF_INET;
	server_addr.sin_len = server_addr_size;
	server_addr.sin_addr.s_addr = inet_addr(server_name);
	server_addr.sin_port = htons(port);

	/*连接服务器*/
CONNECT_DO:
	jljRunLog("客户端正在连接服务器[IP=%s][Port=%d]!\n", server_name, port);
	istate = connect(param->SocetFd, (struct sockaddr *)&server_addr, server_addr_size);
	if(ERROR == istate)
	{
		taskDelay(sysClkRateGet());
		close(param->SocetFd);
		goto SOCKET_DO;
	}
	jljRunLog("客户端连接服务器成功[IP=%s][Port=%d]!\n", server_name, port);

	/*创建接收任务*/
	if(OK != taskIdVerify(param->RecvTid))
	{
		jljRunLog("正在创建数据接收任务......\n");
		
		memset(taskname, 0,sizeof(taskname));
		strcpy(taskname + 0, "tTcpRx");
		sprintf(taskname + strlen(taskname), "%d", port);
		param->RecvTid = taskSpawn(taskname, NETS_TASK_PRIORITY, 0, NETS_TASK_STACK_SIZE, (FUNCPTR)tTcpRecv, (int)param,1,2,3,4,5,6,7,8,9);
		if(OK != taskIdVerify(param->RecvTid))
		{
			jljRunLog("创建接收任务失败.\n");
			taskDelay(sysClkRateGet());
			close(param->SocetFd);	param->SocetFd = ERROR;
			goto SOCKET_DO;
		}
		
		jljRunLog("创建数据接收任务成功......\n");
	}
	
	/*发送数据，从发送缓存取出数据，发送到后台*/
SEND_DO:
	while(1)
	{
		semTake(param->SemIDTx, WAIT_FOREVER);
		rdlength = rngBufGet(param->RngBufferTx, rdbuffer, NETS_BUFFER_SIZE);
		semGive(param->SemIDTx);

		if(rdlength < 0)
		{
			taskDelay(1);
			continue;
		}
		istate = send(param->SocetFd, rdbuffer, rdlength, 0, (struct sockaddr *)&server_addr, server_addr_size);
		if(ERROR == istate)
		{
			taskDelay(sysClkRateGet());
			close(param->SocetFd);	param->SocetFd = ERROR;
			goto SOCKET_DO;
		}

		taskDelay(1);
	}

	exit(0);
}


/*****************************************************************************
*Name				:netSTcpClientStart
*Description		:创建TCP客户端任务
*Input				:addr			IP地址(HEX)
*						:port				端口号(HEX，1~65535)
*Output			:无
*Return			:成功返回0；失败返回ERROR
*History			:2016-05-12,modified by syj
*/
LOCAL int netSTcpClientStart(int addr, int port)
{
	int protocol = NETS_PROTOCOL_TCP;
	int i = 0;
	int client_number = 0;
	int ipointer = 0;
	int fd = 0;
	int istate = 0;
	char taskname[32] = {0};
	int taskid = 0;

	/*判断连接指定地址指定端口的客户端任务是否已创建，已创建则直接返回成功*/
	for(i = 0; i < NETS_PORT_MAX; i++)
	{
		if(1 == NetSClient[i].IsEnable && protocol == NetSClient[i].Protocol && addr == NetSClient[i].Addr && port == NetSClient[i].Port)
		{
			jljRunLog("错误，相同地址、端口号和协议的客户端任务已创建，不可重复创建!\n");
			return ERROR;
		}

		if(1 == NetSClient[i].IsEnable)
		{
			client_number++;
		}
	}

	/*判断是否可以创建更多的客户端任务*/
	if(client_number >= NETS_PORT_MAX)
	{
		jljRunLog("错误，无法创建更多的客户端任务!\n");
		return ERROR;
	}

	/*查找未使用的数据结构，用以处理创建客户端任务的数据*/
	for(i = 0; i < NETS_PORT_MAX; i++)
	{
		if(1 != NetSClient[i].IsEnable)
		{
			ipointer = i;
			break;
		}
	}

	/*标记为已启用*/
	NetSClient[ipointer].IsEnable = 1;

	/*记录协议类型和端口号*/
	NetSClient[ipointer].Protocol = protocol;
	NetSClient[ipointer].Port = port;

	/*创建发送信号量*/
	NetSClient[ipointer].SemIDTx = semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL == NetSClient[ipointer].SemIDTx)
	{
		jljRunLog("错误，创建信号量\"NetSClient[ipointer].SemIDTx\"失败，退出客户端!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	/*分配发送缓存*/
	NetSClient[ipointer].RngBufferTx = (RING_ID)rngCreate(NETS_BUFFER_SIZE);
	if(NULL == NetSClient[ipointer].RngBufferTx)
	{
		jljRunLog("错误，分配缓存\"NetSClient[ipointer].RngBufferTx\"失败，退出客户端!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	/*创建接收信号量*/
	NetSClient[ipointer].SemIDRx = semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL == NetSClient[ipointer].SemIDRx)
	{
		jljRunLog("错误，创建信号量\"NetSClient[ipointer].SemIDRx\"失败，退出客户端!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	/*分配接收缓存*/
	NetSClient[ipointer].RngBufferRx = (RING_ID)rngCreate(NETS_BUFFER_SIZE);
	if(NULL == NetSClient[ipointer].RngBufferRx)
	{
		jljRunLog("错误，分配缓存\"NetSClient[ipointer].RngBufferRx\"失败，退出客户端!\n", ipointer);
		istate = ERROR;
		goto DONE;
	}

	/*创建客户端任务*/
	strcpy(taskname + 0, "tTcpCl");
	sprintf(taskname + strlen(taskname), "%d", port);
	taskid = taskSpawn(taskname, NETS_TASK_PRIORITY, 0, NETS_TASK_STACK_SIZE, (FUNCPTR)tTcpClient, addr, port, (int)&NetSClient[ipointer],3,4,5,6,7,8,9);
	if(OK != taskIdVerify(taskid))
	{
		jljRunLog("错误，创建客户端任务失败，退出客户端!\n");
		istate = ERROR;
		goto DONE;
	}

DONE:
	if(0 != istate && OK == taskIdVerify(taskid))
	{
		taskDelete(taskid);
		taskid = 0;
	}
	if(0 != istate && NULL != NetSClient[ipointer].RngBufferTx)
	{
		free(NetSClient[ipointer].RngBufferTx);
	}
	if(0 != istate && NULL != NetSClient[ipointer].SemIDTx)
	{
		semDelete(NetSClient[ipointer].SemIDTx);
		NetSClient[ipointer].SemIDTx = NULL;
	}
	if(0 != istate)
	{
		NetSClient[ipointer].IsEnable = 0;
		NetSClient[ipointer].Protocol = 0;
		NetSClient[ipointer].Addr = 0;
		NetSClient[ipointer].Port = 0;
	}

	return 0;
}


/*****************************************************************************
*Name				:netSUdpClientStart
*Description		:创建客户端任务
*Input				:protocol		通讯协议 NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:addr			IP地址(HEX)
*						:port				端口号(HEX，1~65535)
*Output			:None
*Return				:成功返回0；失败返回ERROR
*History			:2016-05-12,modified by syj
*/
LOCAL int netSClientStart(int protocol, int addr, int port)
{
	int istate = 0;

	/*判断协议类型*/
	if(NETS_PROTOCOL_UDP != protocol && NETS_PROTOCOL_TCP != protocol)
	{
		jljRunLog("[function=%s][line=%d]the param 'protocol = %x' is invalid!\n", __FUNCTION__, __LINE__, protocol);
		return ERROR;
	}

	/*启用UDP服务器*/
	if(NETS_PROTOCOL_UDP == protocol)
	{
		jljRunLog("[function=%s][line=%d]暂时不实现UDP服务!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	/*启用TCP服务器*/
	if(NETS_PROTOCOL_TCP == protocol)
	{
		istate = netSTcpClientStart(addr, port);
	}

	return istate;
}


/*****************************************************************************
*Name				:netSWrite
*Description		:发送网络数据
*Input				:protocol		通讯协议 NETS_PROTOCOL_UDP / NETS_PROTOCOL_TCP
*						:port				端口号 HEX
*						:inbuffer		发送数据缓存
*						:nbytes			发送数据缓存长度
*Output			:None
*Return				:实际发送长度；失败返回ERROR
*History			:2016-05-12,modified by syj
*/
int netSWrite(int protocol, int port, char *inbuffer, int nbytes)
{
	int ipointer = 0, ilength = 0, i = 0;

	/*判断此端口是否已启用*/
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

	/*判断是否已启用*/
	if(0 == NetSClient[ipointer].IsEnable)
	{
		jljRunLog("NetSSend[%x] is not enable!\n", ipointer);
		return ERROR;
	}

	/*判断信号量*/
	if(NULL == NetSClient[ipointer].SemIDRx)
	{
		jljRunLog("NetSClient[%x] semaphore is invalid!\n", ipointer);
		return ERROR;
	}

	/*判断缓冲区是否合法*/
	if(NULL == NetSClient[ipointer].RngBufferTx)
	{
		jljRunLog("NetSClient[%x] data cache is invalid!\n", ipointer);
		return ERROR;
	}

	/*发送数据*/
	semTake(NetSClient[ipointer].SemIDTx, WAIT_FOREVER);
	ilength = rngBufPut(NetSClient[ipointer].RngBufferTx, inbuffer, nbytes);
	semGive(NetSClient[ipointer].SemIDTx);

	return ilength;
}


/*****************************************************************************
*Name				:netSRead
*Description		:读取网络数据
*Input				:protocol		通讯协议 NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:port				端口号 HEX
*						:maxbytes	输出数据缓存长度
*Output			:outbuffer		输出数据缓存
*Return				:实际获取到的数据长度
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

	/*判断是否已启用*/
	if(0 == NetSClient[ipointer].IsEnable)
	{
		jljRunLog("NetSClient[%x] is not enable!\n", ipointer);
		return ERROR;
	}

	/*判断信号量*/
	if(NULL == NetSClient[ipointer].SemIDRx)
	{
		jljRunLog("NetSClient[%x] semaphore is invalid!\n", ipointer);
		return ERROR;
	}

	/*判断缓冲区是否合法*/
	if(NULL == NetSClient[ipointer].RngBufferRx)
	{
		jljRunLog("NetSClient[%x] data cache is invalid!\n", ipointer);
		return ERROR;
	}

	/*等待数据*/
	while(0 == rngNBytes(NetSClient[ipointer].RngBufferRx))	taskDelay(1);

	/*读取数据*/
	semTake(NetSClient[ipointer].SemIDRx, WAIT_FOREVER);
	ilength = rngBufGet(NetSClient[ipointer].RngBufferRx, outbuffer, maxbytes);
	semGive(NetSClient[ipointer].SemIDRx);

	return ilength;
}


/*****************************************************************************
*Name				:netSKJLDInit
*Description		:网口连接卡机联动后台功能初始化，后台与油机
*						:采用TCP通讯，加油机为客户端，后台为服务器端
*Input				:protocol		通讯协议 NETS_PROTOCOL_UDP/NETS_PROTOCOL_TCP
*						:addr			服务器IP地址
*						:port				服务器端口号(HEX，1~65535)
*						:localport		本地服务器端口号(HEX，1~65535)
*Output			:None
*Return			:成功返回0；失败返回ERROR
*History			:2016-05-12,modified by syj
*/
int netSKJLDInit(int protocol, unsigned int addr, unsigned int port, unsigned int localport)
{
	int istate = 0;

	istate = netSClientStart(NETS_PROTOCOL_TCP, addr, port);

	return istate;
}
