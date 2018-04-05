/*
本文件是多方或多次使用的公共变量和被调函数的声明文件 
 

file name:
modify date:201407
Author:fj
*/

#ifndef _PUBLIC_
#define _PUBLIC_

#define COM_SIGNALNUM 2

typedef struct	
{
	unsigned char IO_Addr; //内网设备地址
	unsigned char IO_Type; //内网设备类型
	unsigned char IO_Mode; //内网数据传输方式
}structBasic;

typedef struct	
{
	float LoseVoltage_No1;//12,24,96,192,394,1152
	float LoseVoltage_No2;//12,24,96,192,394,1152
	float LightLoad;      //12,24,96,192,394,1152
	float OverVoltage_No1;//12,24,96,192,394,1152
	float OverVoltage_No2;//12,24,96,192,394,1152
	float OverCurrent;    //12,24,96,192,394,1152
}structThreshold;

typedef struct	
{
	unsigned char Com_Addr;
	unsigned char Modbus_Bps;
	unsigned char Com103_Bps;
	char* Ip_Addr;
	char* Mask_Addr;
	char* Gateway_Addr;
	char* Mac_Addr;
	unsigned short Port;
	unsigned char DHCP;
	unsigned char Protocol_Type;
}structCom;


typedef struct	
{
	unsigned char IO_Addr;
	//char* Ip_Addr;
	char Ip_Addr[32];
	char* Mask_Addr;
	char* Gateway_Addr;
	char* Mac_Addr;
	unsigned short Port;
}structSubList;

extern structSubList g_stru972Ip;

typedef struct	
{
	structBasic struBasic;
	structThreshold struThreshold;
	structCom struCom;
	unsigned char IO_Num;
	structSubList struSubList[33];
}structSysConFile;

extern structSysConFile struSysConFile;

typedef struct	
{
	unsigned char Baud;//12,24,96,192,394,1152
}structCom103;


typedef struct	 
{
	unsigned char Baud;//12,24,96,192,394,1152
}structComModbus;
typedef struct	 
{
	unsigned char Ip_addr[4];
	unsigned char Netmask[4];//
	unsigned char Gw_addr[4];//
	unsigned char Mac_addr[6];//
	unsigned char Dhcp;//
	unsigned short Port;//
}structTCP;


typedef struct	 
{
	unsigned char Addr;
	unsigned char Type;
	unsigned char Chain;//SUBNET_CAN,SUBNET_ETHNET
	unsigned char com_addr;
	structCom103 struCom103;
	structComModbus struModbus;
	structTCP struTCP;

}structMyInformation;

typedef struct 
{
	unsigned short usDelayTime;
    unsigned short usState;
	unsigned short usThresholdValue;
	unsigned short usStateDO1;
	unsigned short usStateDO2;
	unsigned short usStateDO3;
	unsigned short usStateDO4;
}structProtect;

typedef struct
{
	char* m_pchMenu;
	int m_nOperationType;
	int m_nFlowNum;
	char* m_pchFlow[16];
}structItem;

typedef struct
{
	char* m_pchName;
	char* m_pchMenu;
    int m_nItemNum;
	structItem m_struItem[8];
}structSession;

typedef struct
{
	char* m_pchName;
	char* m_pchMenu;
	int m_nFunctionType;
	int m_nSessionNum;
	structSession m_struSession[2];
}structFunction;

typedef struct
{
	char* m_pchName;
	char* m_pchMenu;
	int m_nFunctionNum;
	structFunction m_struFunction[8];
}structFunctionList;

/*
typedef struct
{
	//char m_chIndex[8];
	//char m_chDevAddr[8];
	//char m_chOffsetAddr[8];
	//char m_chSOETime[32];   //Sequence of event
	//char m_chSOEState[8];
	int m_nIndex;            //报警索引
	int m_nDevAddr;          //设备地址，哪个设备报警   
	int m_nOffsetAddr;       //偏移地址，哪个项报警
	char m_chAlarmTime[32];    //发生报警的时间
	int m_nAlarmState;       //报警的状态
	char m_chAlarmRecord[64];
}structAlarmSOE;*/

extern structFunctionList g_struFunctionList;

extern structProtect g_struProtect[16];

extern structMyInformation struMyInf;
extern char* config_gate[ ][21];
extern unsigned short g_ushLogicGateNum;

extern bool g_bIsCalibrate;
extern unsigned char g_uchCalibrateBuff[8];

extern bool g_bIsInternalComm; //内部通讯，DC83与ocus710的通讯
extern bool g_bIsExternalComm; //外部通讯，后台软件与ocus710的通讯

unsigned short acsiitodec(char* c);
unsigned short crc_check( unsigned char  *puchMsg,unsigned short usDataLen);
unsigned char acsiitohex(char c);
void PrintTime(char* chSecond,char* chMillisecond);  //打印时间
void PrintH(int nLength,unsigned char* puchBuff);    //打印16进制数据表示

int SetIfAddr(char* ifname,char *Ipaddr,char *mask,char *gateway);


int if_updown(char* ifname,int flag);
int set_mac_addr(char* ifname,char* mac);
int ether_atoe(const char* a,unsigned char *e);
char* ether_atoa(const unsigned char* e,char* a);

char* GetSystemTime(int nType); //nType = 0;表示获得的系统时间精确到s,nType = 1,表示精确到ms,默认精确到s.
int   SetSystemTime(char* pchDt);

#endif

/*H:16进制数
  D:10进制数
  O:8进制数
  B:2进制数*/
