#ifndef _OIL_TCPIP_H_
#define _OIL_TCPIP_H_


//����ͨѶЭ������
#define NETS_PROTOCOL_UDP			'1'
#define NETS_PROTOCOL_TCP			'2'

//�������ķ�������ͻ�����Ŀ
#define NETS_PORT_MAX					6

//����������������������
#define NETS_CONNECT_MAX				1

//�����С
#define NETS_BUFFER_SIZE				2048

//����ͨѶ�������ȼ�
#define NETS_TASK_PRIORITY			155

//����ͨѶ�����ջ�ռ��С
#define NETS_TASK_STACK_SIZE		0x8000

//��������
extern int netSWrite(int protocol, int port, char *inbuffer, int nbytes);
extern int netSRead(int protocol, int port, char *outbuffer, int maxbytes);
extern int netSKJLDInit(int protocol, unsigned int addr, unsigned int port, unsigned int localport);

//extern void tTcpClient(void* argc);
//extern void tTcpRecv(void* argc);
#endif

