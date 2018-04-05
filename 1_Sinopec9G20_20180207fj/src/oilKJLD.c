//#include <inetLib.h>
//#include <semLib.h>
//#include <rngLib.h>
//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilTcpip.h"
//#include "oilLog.h"
//#include "oilParam.h"
//#include "oilBoardTrans.h"
//#include "oilKJLD.h"

#include "../inc/main.h"

//����ͨѶ��ʽʱ����������ͨѶ����
//#define KJLD_COMX		COM16
#define KJLD_COMX DEV_COM16_NETA


//����������̨ͨѶ���ݽṹ
struct KJLDStruct
{
	unsigned int NetSinopecAddr;		//������ʯ����̨ͨѶ�ĺ�̨��ַ
	unsigned int NetSinopecPort;		//������ʯ����̨ͨѶ�ĺ�̨�˿ں�
	unsigned int NetSLocalPort;			//������ʯ����̨ͨѶ�ı��ض˿ں�

	unsigned int nNetSLocalIp;
	unsigned int nNetSLocalMask;
	unsigned int nNetSLocalGateway;

	char chNetSLocalIp[32];       //����IP��fj:20171214
	char chNetSLocalMask[32];     //�������룬fj:20171214
    char chNetSLocalGateway[32];  //���أ�fj:20171214
	char chNetSMacAddr[32];       //����MAC��ַ��fj:20171214

	unsigned int Channel;					 //������ʯ����̨ͨѶ�ķ�ʽ KJLD_CHANNEL_COM / KJLD_CHANNEL_UDP
	//SEM_ID SemID;								//ʯ����̨���ݻ�������ź���,fj
	RING_ID RngBuffer;						//���ݻ�����
};

struct KJLDStruct kjld;


/********************************************************************
*Name				:kjldDataIn
*Description		:�洢1������ת������ʯ����̨����
*Input				:inbuffer		ʯ����̨���ݻ���
*						:nbytes			ʯ����̨���ݻ��泤��
*Output			:None
*Return				:ʵ�ʴ洢�ĳ���
*History			:2015-12-11,modified by syj
*/
int kjldDataIn(char *inbuffer, int nbytes)
{
	int length = 0;

	//if(NULL == kjld.SemID)
	//{
	//	jljRunLog("Error, kjld.SemID is invalid!\n");
	//	return 0;
	//}

	if(NULL == kjld.RngBuffer)
	{
		jljRunLog("Error, kjld.RngBuffer is invalid!\n");
		return 0;
	}

	//semTake(kjld.SemID, WAIT_FOREVER);//fj:

	length = rngBufPut(kjld.RngBuffer, inbuffer, nbytes);
	
	//semGive(kjld.SemID);

	return length;
}


/********************************************************************
*Name				:kjldDataOut
*Description		:��ȡ1������ת������ʯ����̨����
*Input				:outbuffer		�洢��ȡʯ����̨��������
*						:maxbytes	ʯ����̨�������ݳ���
*Output			:None
*Return				:ʵ�ʴ洢�ĳ���
*History			:2015-12-11,modified by syj
*/
int kjldDataOut(char *outbuffer, int maxbytes)
{
	int length = 0;

	//if(NULL == kjld.SemID)
	//{
	//	jljRunLog("Error, kjld.SemID is invalid!\n");
	//	return 0;
	//}

	if(NULL == kjld.RngBuffer)
	{
		jljRunLog("Error, kjld.RngBuffer is invalid!\n");
		return 0;
	}

	while(0 == rngNBytes(kjld.RngBuffer))	
		//taskDelay(1);
		usleep(1000);

	//semTake(kjld.SemID, WAIT_FOREVER);

	length = rngBufGet(kjld.RngBuffer, outbuffer, maxbytes);

	//semGive(kjld.SemID);

	return length;
}


/********************************************************************
*Name				:kjldWrite
*Description		:��ʯ������������̨��������
*Input				:None
*Output			:None
*Return				:ʵ�ʷ��͵ĳ��ȣ�ʧ�ܷ���ERROR
*History			:2015-12-11,modified by syj
*/
int kjldWrite(char *inbuffer, int nbytes)
{
	int ilength = 0, local_board_id = 0;
	
	//���������1������ʱ���͸�1������ת������̨
	local_board_id = pcdMboardIDRead();
	if(1 != local_board_id)
	{
		boardSend(local_board_id, 1, BDATA_TYPE_TOSINOPEC, inbuffer, nbytes);
	}
	else if(KJLD_CHANNEL_COM== kjld.Channel) //����������ʽΪ����ʱͨ�����ڷ��͵�ʯ����̨
	{
		ilength = comWriteInTime(KJLD_COMX, inbuffer, nbytes, 2);
	}
	else if(KJLD_CHANNEL_NET == kjld.Channel) //����������ʽΪ����ʱͨ�����ڷ��͵�ʯ����̨
	{
		ilength = netSWrite(NETS_PROTOCOL_TCP, kjld.NetSinopecPort, inbuffer, nbytes);  //fj:
	}

	return ilength;
}


/********************************************************************
*Name				:kjldRead
*Description		:��ȡʯ������������̨���͵����ݣ�����ʽ
*Input				:None
*Output			:None
*Return				:ʵ�ʶ�ȡ�ĳ���
*History			:2015-12-11,modified by syj
*/
int kjldRead(char *outbuffer, int maxbytes)
{
	int ilength = 0, local_board_id = 0;

	//���������1������ʱ���͸�1������ת������̨
	local_board_id = pcdMboardIDRead();
	if(1 != local_board_id)
	{
		ilength = kjldDataOut(outbuffer, maxbytes);
	}
	else if(KJLD_CHANNEL_COM== kjld.Channel) //����������ʽΪ����ʱͨ�����ڽ���ʯ����̨����
	{
		ilength = comRead(KJLD_COMX, outbuffer, maxbytes);
	}
	else if(KJLD_CHANNEL_NET == kjld.Channel) //����������ʽΪ����ʱͨ�����ڽ���ʯ����̨����
	{
        ilength = netSRead(NETS_PROTOCOL_TCP, kjld.NetSinopecPort, outbuffer, maxbytes);
	}

	return ilength;
}


/********************************************************************
*Name				:kjldChannelSet
*Description		:����������ʯ������������̨ͨѶ��ʽ
*Input				:channel	ͨѶ��ʽ KJLD_CHANNEL_COM / KJLD_CHANNEL_NET
*Output			:None
*Return			:�ɹ�����0��ʧ�ܷ�������ֵ
*History			:2015-12-11,modified by syj
*/
int kjldChannelSet(int channel)
{
	char wrbuffer[8] = {0};

	if(KJLD_CHANNEL_COM != channel && KJLD_CHANNEL_NET != channel)
	{
		jljRunLog("���ú�̨���ӷ�ʽʧ�ܣ���������ȷ!\n");
		return ERROR;
	}

	wrbuffer[0] = channel;
	if(0 != paramSetupWrite(PRM_SINOPEC_CONNECT, wrbuffer, 1))
	{
		jljRunLog("���ú�̨���ӷ�ʽʧ�ܣ���������ʧ��!\n");
		return ERROR;
	}

	kjld.Channel = channel;

	return 0;
}


/********************************************************************
*Name				:kjldChannelGet
*Description		:��ȡ������ʯ������������̨ͨѶ��ʽ
*Input				:None
*Output			:None
*Return				:ͨѶ��ʽ KJLD_CHANNEL_COM / KJLD_CHANNEL_NET
*History			:2015-12-11,modified by syj
*/
int kjldChannelGet(void)
{
	return kjld.Channel;
}


/*****************************************************************************
*Name				:netServerSet
*Description		:���ü��ͻ����ӵ�ʯ����̨������IP��ַ
*Input				:ip		������IP��ַ�����ʮ����ASCII��
*						:port		�������˿ںţ�ʮ����ASCII��
*Output			:None
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-29,modified by syj
*/
int kjldServerNetSet(const char *ip, const char *port)
{
	unsigned int myIP = 0, myPort = 0;
	char wrbuffer[8] = {0};

	myIP = inet_addr((char*)ip);
	myIP = ntohl(myIP);
	myPort = atoi(port);

	wrbuffer[0] = (char)(myIP>>24);	wrbuffer[1] = (char)(myIP>>16);
	wrbuffer[2] = (char)(myIP>>8);		wrbuffer[3] = (char)(myIP>>0);
	wrbuffer[4] = (char)(myPort>>8);	wrbuffer[5] = (char)(myPort>>0);
	if(0 != paramSetupWrite(PRM_SINOPEC_ADDRESS, wrbuffer, 6))
	{
		jljRunLog("���÷�������ַʧ�ܣ���������ʧ��!\n");
		return ERROR;
	}

	kjld.NetSinopecAddr = myIP;
	kjld.NetSinopecPort = myPort;

	return 0;
}


/*****************************************************************************
*Name				:netServerGet
*Description		:��ȡ���ͻ����ӵ�ʯ����̨������IP��ַ
*Input				:��
*Output			:ip		������IP��ַ�����ʮ����ASCII��
*						:port		�������˿ںţ�ʮ����ASCII��
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-29,modified by syj
*/
int kjldServerNetGet(char *ip, char *port)
{
	unsigned int myIP = 0, myPort = 0;

	myIP = kjld.NetSinopecAddr;
	myPort = kjld.NetSinopecPort;

	sprintf(ip, "%d.%d.%d.%d", (unsigned char)(myIP>>24), (unsigned char)(myIP>>16), (unsigned char)(myIP>>8), (unsigned char)(myIP>>0));
	sprintf(port, "%d", myPort);

	return 0;
}


/*****************************************************************************
*Name				:kjldLocalPortSet
*Description		:���ÿ���������̨ͨѶ���ط������˿ں�
*Input				:port		�˿ں�HEX
*Output			:��
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-29,modified by syj
*/
int kjldLocalPortSet(int port)
{
	char wrbuffer[8] = {0};

	wrbuffer[0] = (char)(port>>8);	wrbuffer[1] = (char)(port>>0);
	if(0 != paramSetupWrite(PRM_SINOPEC_LOCAL_PORT, wrbuffer, 2))
	{
		return ERROR;
	}

	kjld.NetSLocalPort = port;

	return 0;
}

int kjldLocalIpSet(char* pchLocalIp)
{
	unsigned int nLocalIp = 0;
	char wrbuffer[8] = {0};

	nLocalIp = inet_addr((char*)pchLocalIp);
	nLocalIp = ntohl(nLocalIp);

	wrbuffer[0] = (char)(nLocalIp>>24);	wrbuffer[1] = (char)(nLocalIp>>16);
	wrbuffer[2] = (char)(nLocalIp>>8);		wrbuffer[3] = (char)(nLocalIp>>0);
	if(0 != paramSetupWrite(PRM_SINOPEC_LOCAL_IP, wrbuffer, 4))
	{
		jljRunLog("���÷�������ַʧ�ܣ���������ʧ��!\n");
		return ERROR;
	}

	kjld.nNetSLocalIp = nLocalIp;
	return 0;
}

/*
int kjldLocalIpGet(char* pchLocalIp)
{
	unsigned int nLocalIp = 0;
	nLocalIp = kjld.nNetSLocalIp;
	sprintf(pchLocalIp, "%d.%d.%d.%d", (unsigned char)(nLocalIp>>24), (unsigned char)(nLocalIp>>16), (unsigned char)(nLocalIp>>8), (unsigned char)(nLocalIp>>0));

	return 0;
}*/

int kjldLocalMaskSet(int nLocalMask)
{
	char wrbuffer[8] = {0};

	wrbuffer[0] = (char)(nLocalMask>>24);	wrbuffer[1] = (char)(nLocalMask>>16);
	wrbuffer[2] = (char)(nLocalMask>>8);		wrbuffer[3] = (char)(nLocalMask>>0);
	if(0 != paramSetupWrite(PRM_SINOPEC_LOCAL_MASK, wrbuffer, 4))
	{
		jljRunLog("���÷�������ַʧ�ܣ���������ʧ��!\n");
		return ERROR;
	}

	kjld.nNetSLocalMask = nLocalMask;
	return 0;
}

int kjldLocalGatewaySet(char* pchLocalGateway)
{
	unsigned int nLocalGateway = 0;
	char wrbuffer[8] = {0};

	nLocalGateway = inet_addr((char*)pchLocalGateway);
	nLocalGateway = ntohl(nLocalGateway);

	wrbuffer[0] = (char)(nLocalGateway>>24);	wrbuffer[1] = (char)(nLocalGateway>>16);
	wrbuffer[2] = (char)(nLocalGateway>>8);		wrbuffer[3] = (char)(nLocalGateway>>0);
	if(0 != paramSetupWrite(PRM_SINOPEC_LOCAL_GATEWAY, wrbuffer, 4))
	{
		jljRunLog("���÷�������ַʧ�ܣ���������ʧ��!\n");
		return ERROR;
	}

	kjld.nNetSLocalGateway = nLocalGateway;
	return 0;
}

/*****************************************************************************
*Name				:kjldLocalPortGet
*Description		:��ȡ����������̨ͨѶ���ط������˿ں�
*Input				:��
*Output			:��
*Return			:�˿ں�
*History			:2016-01-29,modified by syj
*/
int kjldLocalPortGet(void)
{
	return kjld.NetSLocalPort;
}

/********************************************************************
*Name				:kjldInit
*Description		:��������ͨѶ���ܳ�ʼ��
*Input				:None
*Output			:None
*Return				:�������Ű󶨵Ķ˿ں�
*History			:2015-12-11,modified by syj
*/
int kjldInit(void)
{
	char rdbuffer[32] = {0};
	int rdlenght = 0;
	int istate = 0;

	kjld.NetSLocalPort = 8000;
	memset(kjld.chNetSLocalIp,0,32);
	memset(kjld.chNetSLocalMask,0,32);
	memset(kjld.chNetSLocalGateway,0,32);
	memset(kjld.chNetSMacAddr,0,32);

	char chTempIp[32] = "192.168.0.100";
	memcpy(kjld.chNetSLocalIp,chTempIp,strlen(chTempIp));
	char chTempMask[32] = "255.255.255.0";
	memcpy(kjld.chNetSLocalMask,chTempMask,strlen(chTempMask));
	char chTempGateway[32] = "192.168.0.1";
	memcpy(kjld.chNetSLocalGateway,chTempGateway,strlen(chTempGateway));
	char chTempMac[32] = "02-00-00-00-00-01";
	memcpy(kjld.chNetSMacAddr,chTempMac,strlen(chTempMac));
	
	unsigned char uchMacAddr[20];
	if_updown("eth0",0);
	ether_atoe(kjld.chNetSMacAddr,uchMacAddr);
	uchMacAddr[0] = 0x02;
	set_mac_addr("eth0",uchMacAddr);
	if_updown("eth0",1);
	SetIfAddr("eth0",kjld.chNetSLocalIp,kjld.chNetSLocalMask,kjld.chNetSLocalGateway);

	//��ȡ������ʽ
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_CONNECT, rdbuffer, 1);
	if(0 == istate && (KJLD_CHANNEL_COM == rdbuffer[0] || KJLD_CHANNEL_NET == rdbuffer[0]))
	{
		kjld.Channel = rdbuffer[0];
	}
	else
	{
		jljRunLog("���󣬻�ȡ������ʽʧ�ܣ�Ĭ��Ϊ���������ڷ�ʽ!\n");
		kjld.Channel = KJLD_CHANNEL_COM;
	}

	//��ȡ��̨��������ַ���˿ں���Ϣ
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_ADDRESS, rdbuffer, 6);
	kjld.NetSinopecAddr = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	kjld.NetSinopecPort = (rdbuffer[4]<<8)|(rdbuffer[5]<<0);

	//��ȡ��̨ͨѶ���ض˿ں�
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_LOCAL_PORT, rdbuffer, 2);
	kjld.NetSLocalPort = (rdbuffer[0]<<8)|(rdbuffer[1]<<0);
	if(istate == 0)
	{
		g_stru972Ip.Port = kjld.NetSLocalPort;
	}

	//��ȡ����IP
    memset(rdbuffer,0,sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_LOCAL_IP,rdbuffer,4);
	printf("local ip: istate = %d\r\n",istate);
	PrintH(4,rdbuffer);
	kjld.nNetSLocalIp = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	if( istate == 0)
	{
		char chTemp[32] = {0};
	    sprintf(chTemp, "%d.%d.%d.%d",rdbuffer[0],rdbuffer[1],rdbuffer[2],rdbuffer[3]); 
		//usrNetIpSet("eth0",chTemp);
		if(usrNetIpSet("eth0",chTemp) == OK)
		{
		    memcpy(kjld.chNetSLocalIp,chTemp,strlen(chTemp));
			memset(g_stru972Ip.Ip_Addr,0,32);
			memcpy(g_stru972Ip.Ip_Addr,chTemp,strlen(chTemp));
		}
	}

	//��ȡ������������
    memset(rdbuffer,0,sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_LOCAL_MASK,rdbuffer,4);
	printf("local mask:\r\n");
	PrintH(4,rdbuffer);
	kjld.nNetSLocalMask = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	if( istate == 0)
	{
		//char chTemp[32] = {0};
	    //sprintf(chTemp, "%d.%d.%d.%d",rdbuffer[0],rdbuffer[1],rdbuffer[2],rdbuffer[3]); 
		usrNetMaskSet("eth0",kjld.nNetSLocalMask);
	}

	//��ȡ��������
    memset(rdbuffer,0,sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_LOCAL_GATEWAY,rdbuffer,4);
	printf("local gateway:\r\n");
	PrintH(4,rdbuffer);
	kjld.nNetSLocalGateway = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	if( istate == 0)
	{
		char chTemp[32] = {0};
	    sprintf(chTemp, "%d.%d.%d.%d",rdbuffer[0],rdbuffer[1],rdbuffer[2],rdbuffer[3]); 
		usrNetHostGatewaySet("eth0",chTemp);
	}

	//������ʽΪRJ45����ʱ����ͨѶ����
	if(KJLD_CHANNEL_NET == kjld.Channel)
	{
		 netSKJLDInit(NETS_PROTOCOL_TCP, kjld.NetSinopecAddr, kjld.NetSinopecPort, kjld.NetSLocalPort);
	}

	return 0;
}


