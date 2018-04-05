#include "../inc/main.h"
//#include "../inc/IO_Usart.h"

#define MAX_SOCK_FD 32

bool ParseUpdatePotocol_55AA(unsigned char* puchRecvData,int nRecvLen,int* pnDeleteLen);
void GetUpdateTcpData(unsigned char* puchSendData,int* pnSendLen);

int g_nUpdateFd = -1;

bool InitUpdateJLexe()
{
	char* dev[2] = {"/dev/ttyS5",""};
	g_nUpdateFd = open_port(dev[0],115200);
	if(g_nUpdateFd < 0)
	{
		printf("open jl update com is failure1\n");
		return false;
	}
	return true;
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
	int nDeleteLen = 0;
	unsigned char buff[1280] ;
	memset(buff,0,sizeof(buff));

	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
        perror("socket");
		return ;
		//exit(1);
	}

	printf("sockfd = %d\n",sockfd);

	bzero(&server_sockaddr,sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(g_stru972Ip.Port);
	//server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	server_sockaddr.sin_addr.s_addr = inet_addr(g_stru972Ip.Ip_Addr);

	//重复使用本地地址与套接字进行绑定
	int i = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));
	if(bind(sockfd,(struct sockaddr*)&server_sockaddr,sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		printf("bind ip error,ip = %s,port = %d\n",g_stru972Ip.Ip_Addr,g_stru972Ip.Port);
		//exit(1);
		return;
	}

    if(listen(sockfd,8) == -1)
	{
		perror("listen");
		//exit(1);
		return;
	}

	printf("listening update jlprocess is start..............\n");

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
			//exit(1);
			return;
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
						//exit(1);
						return;
					}
					FD_SET(client_fd,&inset);
					printf("New connection form%d(socket)\n",client_fd);
				}
				else //处理从客户端发来的数据
				{
					if((nread = recv(fd,buff,sizeof(buff),0)) > 0)
					{
						printf("--------tcp ip update :\n");
						PrintH(nread,buff);
						if(ParseUpdatePotocol_55AA(buff,nread,&nDeleteLen))
						{
							int nWriteLen = 0;
							nWriteLen = write(g_nUpdateFd,buff,nread);

							printf("com write:\n");
							PrintH(nWriteLen,buff);

							if(nWriteLen > 0)
							{
                                int nReadLen = 0;
								int nAllLen = 0;
								unsigned char uchReadBuff[128] = {0};
                                unsigned char uchAllBuff[256] = {0};
								while((nReadLen = read(g_nUpdateFd,uchReadBuff,128)) > 0)
								{
                                    memcpy(uchAllBuff + nAllLen,uchReadBuff,nReadLen);
									nAllLen += nReadLen;

									printf("com recv:\n");
									PrintH(nAllLen,uchAllBuff);
                                    if(ParseUpdatePotocol_55AA(uchAllBuff,nAllLen,&nDeleteLen))
									{
										if((nSend = send(fd,uchAllBuff,nAllLen,0)) == -1)
										{
											perror("send\n");
										}
										printf("--------tcp send\n");
										PrintH(nAllLen,uchAllBuff);
										break;
									}
									else
									{
										if(nDeleteLen == -1)
										{
											printf("xor error!\n");
											break;
										}
									}
								}
							}
						}
						else
						{
							if(nDeleteLen == -1)
							{
								memset(buff,0,sizeof(buff));
								nread = 0;
	                            printf("tcp xor error!\n");
							}
						}
					}
					else
					{
						close(fd);
						FD_CLR(fd,&inset);
						printf("Client %d(socket) has leave\n",fd);
					}
				}
			}
		}
	}

	close(sockfd);
}

bool ParseUpdatePotocol_55AA(unsigned char* puchRecvData,int nRecvLen,int* pnDeleteLen)
{
	if(nRecvLen < 7)
	{
		*pnDeleteLen = 0; //长度不够，继续接收
		return false;
	}
	else
	{
        int i;
		for( i = 0; i < nRecvLen; i++)
		{
			if( i < nRecvLen - 1)
			{
			    if((puchRecvData[i] == 0x55) && (puchRecvData[i+1] == 0xAA))
				{
                    if(i > nRecvLen - 5)
					{
                        *pnDeleteLen = 0;  //长度不够，继续接收
						return false;
					}
					else
					{
						unsigned short ushPDLen = puchRecvData[i+5];
						int nLen = (ushPDLen << 8) | (puchRecvData[i+4]);
						if(nRecvLen < i + nLen + 7)
						{
							*pnDeleteLen = 0;
							return false;  //长度不够，继续接收
						}
						else
						{
							unsigned char uchXor = puchRecvData[i+nLen+6];
							unsigned char uchResultXor = xorGet(&puchRecvData[i+4],nLen+2);
							if(uchXor == uchResultXor)
							{
								*pnDeleteLen = -1;
								return true;
							}
							else
							{
								*pnDeleteLen = -1;
								return false;
							}
						}
					}
				}
			}
		}
	}

	*pnDeleteLen = -1;
	return false;
}

void GetUpdateTcpData(unsigned char* puchSendData,int* pnSendLen)
{
	;
}
