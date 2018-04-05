#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

#include "../inc/main.h"

#define MAX_SOCK_FD FD_SETSIZE

struct ARG
{
	int connfd;
	struct sockaddr_in client;
	int nIoIndex;
};
struct ARG* struArg[33]={0};

void* read_fun(void* arg);
void process_cli(int connfd, struct sockaddr_in client,int nIoIndex);

int g_nIEC103Len;
int g_nModbusLen;
int g_nIntlDtLen; //DC83内部的数据交互 
unsigned char g_ucIEC103Buf[512];
unsigned char g_ucModbusBuf[512];
unsigned char g_ucIntlDtBuf[512];

fd_set fds;
int maxfdp;

/*************通讯协议模块******************************************************************/
void iec103_deal(void* argc) //OCUS710的IEC103的通讯。
{
	int fd;
	int nread = 0;
	int nSendLen = 0;
	int nDeleteLen =0;
	g_nIEC103Len = 0;
	unsigned char buff[512] ;
	unsigned char chSendBuff[512];
	memset(buff,0,sizeof(buff));
	memset(chSendBuff,0,sizeof(chSendBuff));

	fd = Open_uart_port_485((char*)argc);
	Uart_set_485(fd,115200,8,1,'N');

	ReadDIO(g_sStateDIOOld_buf);
	ReadDIO(g_sStateDIONew_buf);

	while (1) 
	{ 
		while((nread = Uart_recv_485(fd,(char*)argc,buff,512)) > 0)
		{
			memcpy(g_ucIEC103Buf+g_nIEC103Len,buff,nread);
		    g_nIEC103Len += nread;
           
			if(ParseRecvData(g_ucIEC103Buf,g_nIEC103Len,&nDeleteLen))
			{
				bzero(buff,sizeof(buff));
				g_nIEC103Len = 0;
                bzero(g_ucIEC103Buf,sizeof(g_ucIEC103Buf));
				ReadDIO(g_sStateDIONew_buf);
				if(SendResponseData(chSendBuff,&nSendLen))
				{
					Uart_send_485(fd,(char*)argc,chSendBuff,nSendLen);
					bzero(chSendBuff,sizeof(chSendBuff));
					nSendLen = 0;
					g_bIsExternalComm = true;
				}
				else
				{
					g_bIsExternalComm = false;
				}
			}
		}
		g_bIsExternalComm = false;
	}
	close(fd);  
}

void Modbus_fun(void* argc) //OCUS710的Modbus通讯。
{
	int fd;
	int nread = 0;
	int nSendLen = 0;
	int nDeleteLen = 0;
	unsigned char buff[512] ;
	unsigned char chSendBuff[512];
	memset(buff,0,sizeof(buff));
	memset(chSendBuff,0,sizeof(chSendBuff));
	fd = Open_uart_port_485((char*)argc);
	Uart_set_485(fd,115200,8,1,'N');

	while (1) 
	{   
		while((nread = Uart_recv_485(fd,(char*)argc,buff,512)) > 0) //block read
		{
			if(g_nModbusLen >= 512)
			{
				g_nModbusLen = 0;
				bzero(g_ucModbusBuf,sizeof(g_ucModbusBuf));
			}
	    	memcpy(g_ucModbusBuf+g_nModbusLen,buff,nread);
	    	g_nModbusLen += nread;

			if(ParseModbusData(g_ucModbusBuf,g_nModbusLen,&nDeleteLen)) //parse recevice data
			{
			    bzero(buff,sizeof(buff));
				g_nModbusLen = 0;
				bzero(g_ucModbusBuf,sizeof(g_ucModbusBuf));
                if(SendModbusData(chSendBuff,&nSendLen)) //Send the modbus request data.
				{
					Uart_send_485(fd,(char*)argc,chSendBuff,nSendLen);
					bzero(chSendBuff,sizeof(chSendBuff));
					g_bIsExternalComm = true;
				}
				else
				{
					g_bIsExternalComm = false;
				}
			}			
		}
		g_bIsExternalComm = false;
	}
	close(fd);  
}

/*************通讯方式模块，TCPIP通讯与CAN通讯*********************************************************************/
int servsock; 
void Server_Ethnet(void* argc) //TCPIP服务端程序
{
	int i=0;
	//int servsock; 
	int connfd;
	pid_t pid;
	struct sockaddr_in servaddr;
	struct sockaddr_in client;
	printf("\nphread--Server_Ethnet is starting\n");
	for(i=0;i<32;i++)
	{

		struArg[i] = (struct ARG*)malloc(sizeof(struct ARG));
		struArg[i]->connfd = 0;
	}
	if((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket\n");
		exit(1);
	}
	
	int reuse = 1;
	if(setsockopt(servsock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))<0)
	{
		printf("close sock\n"); 
		close(servsock);

	}
	//fcntl(servsock,F_SETFD,O_NONBLOCK); 
	int nIOAddr = struSysConFile.struBasic.IO_Addr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(struSysConFile.struSubList[nIOAddr].Ip_Addr);	
	servaddr.sin_port = htons(struSysConFile.struSubList[nIOAddr].Port);

    printf("nIOAddr is  %d\n",nIOAddr);
    //printf("the serverip is %s\n",inet_ntoa(servaddr.sin_addr));

	printf("server IpAddr:%s\n",struSysConFile.struSubList[32].Ip_Addr);
	if(bind(servsock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind\n");
		exit(1);
	}

	if (listen(servsock, 8) < 0)
	{
		perror("listen\n");
		exit(1);
	}
	int connected_client_num =0;
	while(1)
	{		
		socklen_t len =  sizeof(struct sockaddr);
		if((connfd = accept(servsock,(struct sockaddr*)&client,&len))<0)
		{
			printf("accept erro\n");
		}
		else
		{
			if(++connected_client_num >1000)
			{
				connected_client_num = 2;	
			}
			//printf("connected_client_num = %d \b",connected_client_num);
			int tid;
			int tt;
			char* client_addr = inet_ntoa(client.sin_addr);
			for(tt =0;tt<struSysConFile.IO_Num-1;tt++)
			{
				char* str1 =  struSysConFile.struSubList[tt].Ip_Addr;
				if(strcasecmp(struSysConFile.struSubList[tt].Ip_Addr,client_addr) == 0)
				{
					printf("you got a message from :%s\n",client_addr);
					struArg[tt]->connfd = connfd;
					struArg[tt]->nIoIndex = tt;
					memcpy((void *)&(struArg[tt]->client),&client,sizeof(client));
					//if(connected_client_num == 1)
					//{
						pthread_create(&tid, NULL, read_fun, (void*)struArg[tt]);	
					//}
					break;
				}
			}
		}
	}
} 

void process_cli(int connfd, struct sockaddr_in client,int nIoIndex) //解析所有所链接的DC所上送的TCPIP数据
{  
	int num;   
	int nread=0;
	unsigned char buff[512];
	while(1)
	{
		int tt =0;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&fds);
		for(tt=0;tt<struSysConFile.IO_Num-1;tt++)
		{
			FD_SET(struArg[tt]->connfd,&fds);
			if(maxfdp<struArg[tt]->connfd)
			{
				maxfdp = struArg[tt]->connfd;
			}
		}
		if(select(maxfdp+1,&fds,NULL,NULL,NULL)>0)
		for(tt =0;tt<struSysConFile.IO_Num-1;tt++)
		{
        	if(FD_ISSET(struArg[tt]->connfd,&fds))
			if(( nread=recv(struArg[tt]->connfd,buff,sizeof(buff),MSG_NOSIGNAL|MSG_DONTWAIT))>0)
			{
				int erro;
				ParseEthnetData(buff,nread,&erro);	
			}
			else
			{
				printf("server not recv\n");
				close(connfd);
			}
		}
	} 

	close(connfd);  
}

void SendToDC83_Tcp(void) //OCUS710通过TCPIP把数据发送给DC83
{
	int i;
	int len =0;
	for(i=0;i<struSysConFile.IO_Num-1;i++)
	{
		len = GetTcpChangedData_OCUS710(i);

		if(len >10 )//至少大于帧头长度
		{
			if(send(struArg[i]->connfd,g_uchSendPackBuff,len,MSG_NOSIGNAL) <0 )
			{
				printf("server was unconnected\n");	
			}
			pthread_rwlock_wrlock(&rwlock_WriteState);//加写锁（读区）
			memset(&DataBase_State[i][0],0,112);
			pthread_rwlock_unlock(&rwlock_WriteState);//解锁
		}
	}
	SaveCSVFile();
}

void* read_fun(void* arg) //TCPIP服务端读取数据线程函数
{  
	struct ARG *info;  
	info = (struct ARG*)arg;  
	process_cli(info->connfd, info->client,info->nIoIndex);  
	free(arg);  
	close(info->connfd);
	printf("close connfd");
	pthread_exit(NULL);
}

void Client_Ethnet(void *argc) //客户端程序只在DC83上运行
{ 
	signed char FlagConneted_Client;
	int client_socket;
	struct sockaddr_in server_addr;  
	int i=0;
	int len=0;
	int l_Num_DevOnLine = 1;
	FlagConneted_Client = -1;
	while(1)
	{
		sleep(1);
		if(FlagConneted_Client != 0)
		{
			client_socket = socket(AF_INET,SOCK_STREAM,0);  
			if( client_socket < 0)  
			{ 
				perror("socket");
	            printf("Create Socket Failed!\n");  
				break;
			}  
			bzero(&server_addr,sizeof(client_socket));  
			server_addr.sin_family = AF_INET;  
			server_addr.sin_addr.s_addr = inet_addr(struSysConFile.struSubList[32].Ip_Addr);
			server_addr.sin_port = htons(struSysConFile.struSubList[32].Port);  
			FlagConneted_Client= connect(client_socket,(struct sockaddr*)&server_addr, sizeof(server_addr)); 
			if(FlagConneted_Client==0)
			{
				fcntl(client_socket,F_SETFL,O_NONBLOCK); 
	            printf("OCUS710 ipaddr :%d\n",struSysConFile.struBasic.IO_Addr);
	            printf("connect success\n");
				sleep(1);
			}
			else
			{
                close(client_socket);
			}
		}
		else
		{
			while(1)
			{
				int nread = 0;
				char buff[512];
				if(( nread=read(client_socket,buff,sizeof(buff)))>0)
				{
					int erro;
					ParseEthnetData(buff,nread,&erro);
					sem_post(&sem_idSendToDSP);
					g_bIsInternalComm = true;
				}

				len = GetTcpChangedData_DC83();
				if(len >10 )//至少大于帧头长度
				{
					if( send(client_socket,g_uchSendPackBuff,len,MSG_NOSIGNAL) <0 )
					{
						FlagConneted_Client=-1;
						close(client_socket);
						printf("client socket disconnect!\n");
						g_bIsInternalComm = false;
						break;
					}
					pthread_rwlock_wrlock(&rwlock_WriteState);//加写锁（读区）
					memset(&DataBase_State[struSysConFile.struBasic.IO_Addr][0],0,102);
					pthread_rwlock_unlock(&rwlock_WriteState);//解锁
					g_bIsInternalComm = true;
				}
			}
		}
	}
}

void run_cansend(void) //can数据的发送,CAN数据的接收在IO_Can.c里
{
	if(struSysConFile.struBasic.IO_Type == OCUS710_DEV) //OCUS710
	{	
		while(1)
		{
			sem_wait(&sem_idSend);
			if(struSysConFile.struBasic.IO_Mode == SUBNET_CAN)
			{
			    SendCan_OCUS710();
			}
			else
			{
				SendToDC83_Tcp();
			}
		}			
	}
	else //DC83
	{
		while(1)
		{
			sem_wait(&sem_idSendToOCUS);
			int i;
			int nIoDevIndex = struSysConFile.struBasic.IO_Addr;
			bool bIsChanged = false;
			for(i = 0; i < 102 ; i++)
			{
				if(DataBase_State[nIoDevIndex][i])
				{
					bIsChanged = true;
					break;
				}
			}
			if(bIsChanged) 
			{
				SendCan_DC83();
			}
		}
	}
}

/*****************A8与DSP之间的数据通讯模块***************************************************************************/
void DC83InRecvData(void* argc) //DC83接收Dsp数据
{
	int fdRecv;
	int nread = 0;
	int nDeleteLen =0;
	g_nIntlDtLen = 0;
	unsigned char chRecvBuff[512];

	memset(chRecvBuff,0,sizeof(chRecvBuff));
	fdRecv = Open_uart_port((char*)argc);
	Uart_set(fdRecv,115200,8,1,'N');

	while (1) 
	{
		while((nread = read(fdRecv,chRecvBuff,512)) > 0) //block read
		{
			if(g_nIntlDtLen + nread > 512)
			{
                bzero(g_ucIntlDtBuf,sizeof(g_ucIntlDtBuf));
				g_nIntlDtLen = 0;
			}

	    	memcpy(g_ucIntlDtBuf+g_nIntlDtLen,chRecvBuff,nread);
	    	g_nIntlDtLen += nread;
           
			if(ParseDC83InData(g_ucIntlDtBuf,g_nIntlDtLen,&nDeleteLen)) //parse recevice data
			{
                bzero(chRecvBuff,sizeof(chRecvBuff));
				if(nDeleteLen > 0)
				{
				    unsigned char uchTemp[512];
				    bzero(uchTemp,sizeof(uchTemp));
				    memcpy(uchTemp,g_ucIntlDtBuf + (g_nIntlDtLen-nDeleteLen),nDeleteLen);
				    bzero(g_ucIntlDtBuf,sizeof(g_ucIntlDtBuf));
				    memcpy(g_ucIntlDtBuf,uchTemp,nDeleteLen);
				}
				else
				{
					bzero(g_ucIntlDtBuf,sizeof(g_ucIntlDtBuf));
				}
				g_nIntlDtLen = nDeleteLen;
                sem_post(&sem_idSendToOCUS); 
			}
			else 
			{
				if(nDeleteLen == CRC_ERROR)
				{
					bzero(chRecvBuff,sizeof(chRecvBuff));
					g_nIntlDtLen = 0;
					bzero(g_ucIntlDtBuf,sizeof(g_ucIntlDtBuf));
				}
			}
		}
	}
	close(fdRecv);
}


void DC83InSendData(void* argc) //DC83发送数据给Dsp
{
    int fdSend;
	int nSendLen = 0;
	int nDeleteLen =0;
	unsigned char chSendBuff[512];
	memset(chSendBuff,0,sizeof(chSendBuff));
	fdSend = Open_uart_port((char*)argc);
	Uart_set(fdSend,115200,8,1,'N');

	while (1) 
	{
		if(g_bIsCalibrate == true)
		{
	        unsigned short ushTemp = crc_check(g_uchCalibrateBuff,11);
			g_uchCalibrateBuff[11] = (ushTemp >> 8) & 0xff;
			g_uchCalibrateBuff[12] = ushTemp & 0xff;
			write(fdSend,g_uchCalibrateBuff,13);
			g_bIsCalibrate = false;
			memset(g_uchCalibrateBuff,0,sizeof(g_uchCalibrateBuff));
		}

		sem_wait(&sem_idSendToDSP);
		if(g_bIsCalibrate)
		{
			continue;
		}

		if(SendDC83InData(chSendBuff,&nSendLen))  
		{
			write(fdSend,chSendBuff,nSendLen);
			bzero(chSendBuff,sizeof(chSendBuff));
			pthread_rwlock_wrlock(&rwlock_ReadDataBase);//加写锁（读区）
			memset(&DataBase_State[struSysConFile.struBasic.IO_Addr][0],0,120); //数据最后发送完再把整个状态清0
			pthread_rwlock_unlock(&rwlock_ReadDataBase);//解锁
		}
	}
	close(fdSend);  
}

void WriteValueToDsp()//把A8读到的配置信息然后下置给DSP，如果成功表示A8可以和DSP正常通讯，反之不能。
{
    int fd;
	int nSendLen = 0;
    int nRecvLen = 0;
	unsigned char chSendBuff[512];
	char chRecvBuff[512];
	memset(chSendBuff,0,sizeof(chSendBuff));
	memset(chRecvBuff,0,sizeof(chRecvBuff));
	fd = Open_uart_port(3);
	Uart_set(fd,115200,8,1,'N');

	fcntl(fd,F_SETFL,FNDELAY);

	bool bSendSuccess = false;
	int i;
	int nTimes = 0;
	do
	{  
		int nIndex = 0;
		chSendBuff[nIndex++] = 0xaa;
		chSendBuff[nIndex++] = 0xcc;
        chSendBuff[nIndex++] = 0;
		chSendBuff[nIndex++] = 0;
		for(i = 0; i < 16; i++)
		{
			chSendBuff[nIndex++] = i + 1;
			chSendBuff[nIndex++] = (g_struProtect[i].usDelayTime >> 8) & 0xff;
            chSendBuff[nIndex++] = g_struProtect[i].usDelayTime & 0xff;
			chSendBuff[nIndex++] = g_struProtect[i].usState;
			chSendBuff[nIndex++] = g_struProtect[i].usThresholdValue >> 8 & 0xff;
			chSendBuff[nIndex++] = g_struProtect[i].usThresholdValue & 0xff;
			chSendBuff[nIndex++] = g_struProtect[i].usStateDO1;
			chSendBuff[nIndex++] = g_struProtect[i].usStateDO2;
			chSendBuff[nIndex++] = g_struProtect[i].usStateDO3;
			chSendBuff[nIndex++] = g_struProtect[i].usStateDO4;	
		}
		chSendBuff[2] = ((nIndex+2) >> 8)&0xff;
		chSendBuff[3] = (nIndex+2)&0xff;
		chSendBuff[nIndex++] = 0xbb;
		chSendBuff[nIndex++] = 0xdd;
		nSendLen = nIndex;
		write(fd,chSendBuff,nSendLen);
        sleep(1);
		
        if(read(fd,chRecvBuff,512) > 0)
		{ 
			if(strcasecmp(chRecvBuff,"set is ok!") == 0)
			{
				printf("Send the config value is success!\n");
                break;
			}
		}
		nTimes++;
	}while(nTimes < 10);

	close(fd);  
}






/***************************************测试程序************************************************************/

void test1(void* arg) //用于测试
{
	unsigned char DI1_temp=0;
	unsigned char uchTemp[8];

	while(1)
	{
		sleep(3);
		DI1_temp = !DI1_temp;
		DataBaseWrite(struSysConFile.struBasic.IO_Addr,96,&DI1_temp,1,0);
        DataBaseWrite(struSysConFile.struBasic.IO_Addr,97,&DI1_temp,1,0);
        DataBaseWrite(struSysConFile.struBasic.IO_Addr,98,&DI1_temp,1,0);
        DataBaseWrite(struSysConFile.struBasic.IO_Addr,99,&DI1_temp,1,0);
		DataBaseUpdata();
	}
}

void TestModbusRS232(void* argc) //测试rs232
{
	int fd;
	int nread = 0;
	int nSendLen = 0;
	int nDeleteLen = 0;
	unsigned char buff[512] ;
	unsigned char chSendBuff[512] = "123456789";
	memset(buff,0,sizeof(buff));
	memset(chSendBuff,0,sizeof(chSendBuff));



	fd = Open_uart_port((char*)argc);
	Uart_set(fd,115200,8,1,'N');

	printf("ttttttttttttttttttt*******\n");
	//nSendLen = 5;

	//while(1)
	//{
	//    Uart_send(fd,chSendBuff,10);
	//    printf("send \n");
	//}

	while (1) 
	{   
		//PrintTime("--modbus run time sec is %d-- \n","msec is %d \n");
		//nread = read(fd,buff,512);
	    //printf("nread = %d\n",nread);
        //printf("Modbus nread : %s \n",buff);
		while((nread = Uart_recv(fd,buff,512)) > 0) //block read
		{
	    	memcpy(g_ucModbusBuf+g_nModbusLen,buff,nread);
	    	g_nModbusLen += nread;

			printf("nread = %d\n",nread);
			printf("Modbus nread : %s \n",buff);
			if(ParseModbusData(g_ucModbusBuf,g_nModbusLen,&nDeleteLen)) //parse recevice data
			{
				//PrintH(g_nModbusLen,g_ucModbusBuf);		
			    bzero(buff,sizeof(buff));
				g_nModbusLen = 0;
				bzero(g_ucModbusBuf,sizeof(g_ucModbusBuf));
                if(SendModbusData(chSendBuff,&nSendLen)) //Send the modbus request data.
				{
					//printf("chSendBuff:%s\n",chSendBuff);
					//printf("nSendLen = %d\n",nSendLen);
					//PrintH(nSendLen,chSendBuff);
					//write(fd,chSendBuff,nSendLen); 
					Uart_send(fd,chSendBuff,nSendLen);
					bzero(chSendBuff,sizeof(chSendBuff));
					g_bIsExternalComm = true;
				}
				else
				{
					g_bIsExternalComm = false;
				}
			}			
		}
		g_bIsExternalComm = false;
	}
	close(fd);  
}

void CreateTcpServer(void* argc)
{
	struct sockaddr_in server_sockaddr;
	struct sockaddr_in client_sockaddr;
	fd_set inset;
	fd_set tmp_inset;
    int sockfd;
	int client_fd;
	int fd;
	int sin_size;

	int nread = 0;
	int nSend;
	int nSendLen = 0;
	int nDeleteLen = 0;
	unsigned char buff[512] ;
	unsigned char chSendBuff[512];
	memset(buff,0,sizeof(buff));
	memset(chSendBuff,0,sizeof(chSendBuff));

	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
        perror("socket");
		exit(1);
	}

	bzero(&server_sockaddr,sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(struSysConFile.struCom.Port);
	//server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	server_sockaddr.sin_addr.s_addr = inet_addr(struSysConFile.struCom.Ip_Addr);

	//重复使用本地地址与套接字进行绑定
	int i = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));
	if(bind(sockfd,(struct sockaddr*)&server_sockaddr,sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}

    if(listen(sockfd,8) == -1)
	{
		perror("listen");
		exit(1);
	}

	//printf("listening..............20150202\n");

	//调用socket函数的描述符作为文件描述符
	FD_ZERO(&inset);
	FD_SET(sockfd,&inset);
	while(1)
	{
		tmp_inset = inset;
		sin_size = sizeof(struct sockaddr_in);
		
		//调用select函数
		if(!(select(MAX_SOCK_FD,&tmp_inset,NULL,NULL,NULL) > 0))
		{
			perror("select");
			close(sockfd);
			exit(1);
		}

		for(fd = 0; fd < MAX_SOCK_FD; fd++)
		{
			if(FD_ISSET(fd,&tmp_inset) > 0)
			{
				if(fd == sockfd)
				{   //服务端接收客户端的链接请求
                    if((client_fd = accept(sockfd,(struct sockaddr*)&client_sockaddr,&sin_size)) == -1)
					{
						perror("accept");
						exit(1);
					}
					FD_SET(client_fd,&inset);
					//printf("New connection form%d(socket)\n",client_fd);
				}
				else //处理从客户端发来的数据
				{
					if((nread = recv(fd,buff,sizeof(buff),0)) > 0)
					{
						if(ParseModbusTcpData(buff,nread,&nDeleteLen))
						{
							GetModbusTcpData(chSendBuff,&nSendLen);
                            if((nSend = send(fd,chSendBuff,nSendLen,0)) == -1)
							{
								perror("send\n");
							}
						}
					}
					else
					{
						close(fd);
						FD_CLR(fd,&inset);
						printf("Client %d(socket) has left\n",fd);
					}
				}
			}
		}

	}

	close(sockfd);
}
