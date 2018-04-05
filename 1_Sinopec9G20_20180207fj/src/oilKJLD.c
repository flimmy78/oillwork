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

//串口通讯方式时，卡机联动通讯串口
//#define KJLD_COMX		COM16
#define KJLD_COMX DEV_COM16_NETA


//卡机联动后台通讯数据结构
struct KJLDStruct
{
	unsigned int NetSinopecAddr;		//主板与石化后台通讯的后台地址
	unsigned int NetSinopecPort;		//主板与石化后台通讯的后台端口号
	unsigned int NetSLocalPort;			//主板与石化后台通讯的本地端口号

	unsigned int nNetSLocalIp;
	unsigned int nNetSLocalMask;
	unsigned int nNetSLocalGateway;

	char chNetSLocalIp[32];       //本地IP，fj:20171214
	char chNetSLocalMask[32];     //子网掩码，fj:20171214
    char chNetSLocalGateway[32];  //网关，fj:20171214
	char chNetSMacAddr[32];       //网卡MAC地址，fj:20171214

	unsigned int Channel;					 //主板与石化后台通讯的方式 KJLD_CHANNEL_COM / KJLD_CHANNEL_UDP
	//SEM_ID SemID;								//石化后台数据缓存操作信号量,fj
	RING_ID RngBuffer;						//数据缓冲区
};

struct KJLDStruct kjld;


/********************************************************************
*Name				:kjldDataIn
*Description		:存储1号主板转发来的石化后台数据
*Input				:inbuffer		石化后台数据缓存
*						:nbytes			石化后台数据缓存长度
*Output			:None
*Return				:实际存储的长度
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
*Description		:读取1号主板转发来的石化后台数据
*Input				:outbuffer		存储读取石化后台缓存数据
*						:maxbytes	石化后台缓存数据长度
*Output			:None
*Return				:实际存储的长度
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
*Description		:向石化卡机联动后台发送数据
*Input				:None
*Output			:None
*Return				:实际发送的长度；失败返回ERROR
*History			:2015-12-11,modified by syj
*/
int kjldWrite(char *inbuffer, int nbytes)
{
	int ilength = 0, local_board_id = 0;
	
	//本地主板非1号主板时发送给1号主板转发到后台
	local_board_id = pcdMboardIDRead();
	if(1 != local_board_id)
	{
		boardSend(local_board_id, 1, BDATA_TYPE_TOSINOPEC, inbuffer, nbytes);
	}
	else if(KJLD_CHANNEL_COM== kjld.Channel) //本地联网方式为串口时通过串口发送到石化后台
	{
		ilength = comWriteInTime(KJLD_COMX, inbuffer, nbytes, 2);
	}
	else if(KJLD_CHANNEL_NET == kjld.Channel) //本地联网方式为网口时通过网口发送到石化后台
	{
		ilength = netSWrite(NETS_PROTOCOL_TCP, kjld.NetSinopecPort, inbuffer, nbytes);  //fj:
	}

	return ilength;
}


/********************************************************************
*Name				:kjldRead
*Description		:读取石化卡机联动后台发送的数据，阻塞式
*Input				:None
*Output			:None
*Return				:实际读取的长度
*History			:2015-12-11,modified by syj
*/
int kjldRead(char *outbuffer, int maxbytes)
{
	int ilength = 0, local_board_id = 0;

	//本地主板非1号主板时发送给1号主板转发到后台
	local_board_id = pcdMboardIDRead();
	if(1 != local_board_id)
	{
		ilength = kjldDataOut(outbuffer, maxbytes);
	}
	else if(KJLD_CHANNEL_COM== kjld.Channel) //本地联网方式为串口时通过串口接收石化后台数据
	{
		ilength = comRead(KJLD_COMX, outbuffer, maxbytes);
	}
	else if(KJLD_CHANNEL_NET == kjld.Channel) //本地联网方式为网口时通过网口接收石化后台数据
	{
        ilength = netSRead(NETS_PROTOCOL_TCP, kjld.NetSinopecPort, outbuffer, maxbytes);
	}

	return ilength;
}


/********************************************************************
*Name				:kjldChannelSet
*Description		:设置主板与石化卡机联动后台通讯方式
*Input				:channel	通讯方式 KJLD_CHANNEL_COM / KJLD_CHANNEL_NET
*Output			:None
*Return			:成功返回0；失败返回其它值
*History			:2015-12-11,modified by syj
*/
int kjldChannelSet(int channel)
{
	char wrbuffer[8] = {0};

	if(KJLD_CHANNEL_COM != channel && KJLD_CHANNEL_NET != channel)
	{
		jljRunLog("设置后台连接方式失败，参数不正确!\n");
		return ERROR;
	}

	wrbuffer[0] = channel;
	if(0 != paramSetupWrite(PRM_SINOPEC_CONNECT, wrbuffer, 1))
	{
		jljRunLog("设置后台连接方式失败，参数保存失败!\n");
		return ERROR;
	}

	kjld.Channel = channel;

	return 0;
}


/********************************************************************
*Name				:kjldChannelGet
*Description		:获取主板与石化卡机联动后台通讯方式
*Input				:None
*Output			:None
*Return				:通讯方式 KJLD_CHANNEL_COM / KJLD_CHANNEL_NET
*History			:2015-12-11,modified by syj
*/
int kjldChannelGet(void)
{
	return kjld.Channel;
}


/*****************************************************************************
*Name				:netServerSet
*Description		:配置加油机连接的石化后台服务器IP地址
*Input				:ip		服务器IP地址，点分十进制ASCII码
*						:port		服务器端口号，十进制ASCII码
*Output			:None
*Return			:成功返回0；失败返回ERROR
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
		jljRunLog("设置服务器地址失败，参数保存失败!\n");
		return ERROR;
	}

	kjld.NetSinopecAddr = myIP;
	kjld.NetSinopecPort = myPort;

	return 0;
}


/*****************************************************************************
*Name				:netServerGet
*Description		:获取加油机连接的石化后台服务器IP地址
*Input				:无
*Output			:ip		服务器IP地址，点分十进制ASCII码
*						:port		服务器端口号，十进制ASCII码
*Return			:成功返回0；失败返回ERROR
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
*Description		:设置卡机联动后台通讯本地服务器端口号
*Input				:port		端口号HEX
*Output			:无
*Return			:成功返回0；失败返回ERROR
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
		jljRunLog("设置服务器地址失败，参数保存失败!\n");
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
		jljRunLog("设置服务器地址失败，参数保存失败!\n");
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
		jljRunLog("设置服务器地址失败，参数保存失败!\n");
		return ERROR;
	}

	kjld.nNetSLocalGateway = nLocalGateway;
	return 0;
}

/*****************************************************************************
*Name				:kjldLocalPortGet
*Description		:获取卡机联动后台通讯本地服务器端口号
*Input				:无
*Output			:无
*Return			:端口号
*History			:2016-01-29,modified by syj
*/
int kjldLocalPortGet(void)
{
	return kjld.NetSLocalPort;
}

/********************************************************************
*Name				:kjldInit
*Description		:卡机联动通讯功能初始化
*Input				:None
*Output			:None
*Return				:与该主板号绑定的端口号
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

	//读取联网方式
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_CONNECT, rdbuffer, 1);
	if(0 == istate && (KJLD_CHANNEL_COM == rdbuffer[0] || KJLD_CHANNEL_NET == rdbuffer[0]))
	{
		kjld.Channel = rdbuffer[0];
	}
	else
	{
		jljRunLog("错误，获取联网方式失败，默认为电流环串口方式!\n");
		kjld.Channel = KJLD_CHANNEL_COM;
	}

	//读取后台服务器地址、端口号信息
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_ADDRESS, rdbuffer, 6);
	kjld.NetSinopecAddr = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	kjld.NetSinopecPort = (rdbuffer[4]<<8)|(rdbuffer[5]<<0);

	//读取后台通讯本地端口号
	memset(rdbuffer, 0, sizeof(rdbuffer));
	istate = paramSetupRead(PRM_SINOPEC_LOCAL_PORT, rdbuffer, 2);
	kjld.NetSLocalPort = (rdbuffer[0]<<8)|(rdbuffer[1]<<0);
	if(istate == 0)
	{
		g_stru972Ip.Port = kjld.NetSLocalPort;
	}

	//读取本地IP
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

	//读取本地子网掩码
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

	//读取本地网关
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

	//联网方式为RJ45网口时启动通讯任务
	if(KJLD_CHANNEL_NET == kjld.Channel)
	{
		 netSKJLDInit(NETS_PROTOCOL_TCP, kjld.NetSinopecAddr, kjld.NetSinopecPort, kjld.NetSLocalPort);
	}

	return 0;
}


