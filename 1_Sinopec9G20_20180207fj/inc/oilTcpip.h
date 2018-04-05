#ifndef _OIL_TCPIP_H_
#define _OIL_TCPIP_H_


//网络通讯协议类型
#define NETS_PROTOCOL_UDP			'1'
#define NETS_PROTOCOL_TCP			'2'

//最多允许的服务器或客户端数目
#define NETS_PORT_MAX					6

//服务器端允许的最大连接数
#define NETS_CONNECT_MAX				1

//缓存大小
#define NETS_BUFFER_SIZE				2048

//网络通讯任务优先级
#define NETS_TASK_PRIORITY			155

//网络通讯任务堆栈空间大小
#define NETS_TASK_STACK_SIZE		0x8000

//函数声明
extern int netSWrite(int protocol, int port, char *inbuffer, int nbytes);
extern int netSRead(int protocol, int port, char *outbuffer, int maxbytes);
extern int netSKJLDInit(int protocol, unsigned int addr, unsigned int port, unsigned int localport);

//extern void tTcpClient(void* argc);
//extern void tTcpRecv(void* argc);
#endif

