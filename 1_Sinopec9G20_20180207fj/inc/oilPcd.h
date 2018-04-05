#ifndef _OIL_PCD_H_
#define _OIL_PCD_H_


#include "lstLib.h"

#define PCD_MBOARD_INFO		"佳力佳V5.00 "	//厂家信息，12ASCII
#define PCD_VERSION				"V5.00"		      //PCD版本
#define PCD_VERSION_DSP		"0500"		      //PCD版本，上传错误信息有此字段，要根据PCD版本改成只有数字的形式
#define PCD_MSGNB					20				      //PCD接收消息队列最多接收的消息条数
#define PCD_MSGMAX				512			//PCD接收消息队列一条信息最大长度
#define PCD_PCDATA_MAX		512			//PCD接收后台PC数据的最大长度
#define PCD_MBOARD_MAX		3				//PCD支持的最大主板数
#define PCD_NOZZLE_MAX			6			//PCD支持的最大物理油枪数
#define PCD_FP_OFFLINE			0			//支付终端离线状态
#define PCD_FP_ONLINE			1				//支付终端在线状态
#define PCD_PC_OFFLINE			0			//PC后台离线状态
#define PCD_PC_ONLINE			1				//PC后台在线状态
#define PCD_ZD_LEN					128		//账单长度
#define PCD_ZDERRINFO_SIZE	16	  //异常账单索引信息长度
#define PCD_ZD_INDEX_SIZE	16			//物理油枪账单索引信息长度
#define PCD_DOWNLOAD_SEG	30			//加油机下载PC数据时每次下载的数据段数
#define PCD_RECORD_MAX		50000		//PCD最大存储账单数目
#define PCD_ZD_INDEX_MAX	5000		//各枪号账单索引文件及异常账单索引未见最大数目

//PC数据下载内容
#define PCD_DOWN_BASELIST			1		//下载基础黑名单
#define PCD_DOWN_ADDLIST			2		//下载新增黑名单
#define PCD_DOWN_DELLIST			3		//下载新删黑名单
#define PCD_DOWN_WHITELIST		4		//下载白名单
#define PCD_DOWN_OILINFO			5		//下载油品油价表
#define PCD_DOWN_STATIONINFO	6		//下载油站通用信息

//账单数据
#define PCD_BILL_SIZE									128			//账单长度
#define PCD_OFFSET_TTC								(0)			//POS_TTC	4bytes
#define PCD_OFFSET_T_TYPE						(4)			  //交易类型1byte
#define PCD_OFFSET_TIME							(5)			  //交易日期及时间7bytes卡应用
#define PCD_OFFSET_ASN								(12)		//卡应用号10bytes
#define PCD_OFFSET_BALANCE						(22)		//余额4bytes
#define PCD_OFFSET_AMN								(26)		//数额3bytes
#define PCD_OFFSET_CTC								(29)		//卡交易序号2bytes
#define PCD_OFFSET_TAC								(31)		//电子签名4bytes
#define PCD_OFFSET_GMAC							(35)			//解灰认证码4bytes
#define PCD_OFFSET_PSAM_TAC					(39)			//PSAM灰锁签名4bytes
#define PCD_OFFSET_PSAM_ASN					(43)			//PSAM应用号10bytes
#define PCD_OFFSET_TID								(53)		//PSAM编号6bytes
#define PCD_OFFSET_PSAM_TTC					(59)			//PSAM终端交易序号4bytes
#define PCD_OFFSET_DS								(63)			//扣款来源1byte
#define PCD_OFFSET_UNIT							(64)			//结算单位/方式1byte
#define PCD_OFFSET_C_TYPE						(65)			//卡类1byte
#define PCD_OFFSET_VER								(66)		//卡版本1byte	b7~b4:卡密钥索引号；b3~b0:卡密钥版本号
#define PCD_OFFSET_NZN								(67)		//枪号1byte
#define PCD_OFFSET_G_CODE						(68)			//油品代码2bytes
#define PCD_OFFSET_VOL								(70)		//升数3bytes
#define PCD_OFFSET_PRC								(73)		//成交价格2bytes
#define PCD_OFFSET_EMP								(75)		//员工号1byte
#define PCD_OFFSET_V_TOT							(76)		//升累计4bytes
#define PCD_OFFSET_RFU								(80)		//备用部分11bytes
#define PCD_OFFSET_T_MAC							(91)		//终端数据认证码4bytes........................................
#define PCD_OFFSET_PHYGUN						(95)			//物理枪号1byte
#define PCD_OFFSET_STOPNO						(96)			//计量停机代码1byte
#define PCD_OFFSET_BEFOR_BAL				(97)			//扣前余额4bytes
#define PCD_OFFSET_ZD_STATE					(101)		  //账单状态0=正常；1=未完成
#define PCD_OFFSET_JLNOZZLE					(102)		  //计量枪号1byte
#define PCD_OFFSET_ZDXOR							(127)		//账单异或校验,1byte
#define PCD_OFFSET_ZDBACKUP					(128)		  //128~255账单备份，总长128字节

//文件路径
#define PCD_FILE_BASELIST				"../config/mboardFiles/PcdIBList.txt"				 //基础黑名单记录文件
#define PCD_FILE_ADDLIST				"../config/mboardFiles/PcdABList.txt"				 //新增黑名单记录文件
#define PCD_FILE_DELLIST				"../config/mboardFiles/PcdDBList.txt"			 //新删黑名单记录文件
#define PCD_FILE_WLIST					"../config/mboardFiles/PcdWList.txt"					 //白名单记录文件
#define PCD_FILE_PRICEINFO			    "../config/mboardFiles/PcdPriceInfo.txt"			 //油品油价表记录文件
#define PCD_FILE_STATIONINFO		    "../config/mboardFiles/PcdStationInfo.txt"		 //油站通用信息记录文件
#define PCD_FILE_OILRECORD			    "../config/mboardFiles/PcdOilRecord.txt"			 //加油数据记录文件
#define PCD_FILE_ZD_UNNORMAL	        "../config/mboardFiles/PcdZDUnnormal.txt"			 //异常账单索引文件
#define PCD_FILE_ZDINDEX_1			    "../config/mboardFiles/PcdZDIndex1.txt"			 //物理枪号1账单索引文件
#define PCD_FILE_ZDINDEX_2			    "../config/mboardFiles/PcdZDIndex2.txt"				//物理枪号2账单索引文件
#define PCD_FILE_ZDINDEX_3			    "../config/mboardFiles/PcdZDIndex3.txt"				//物理枪号3账单索引文件
#define PCD_FILE_ZDINDEX_4			    "../config/mboardFiles/PcdZDIndex4.txt"				//物理枪号4账单索引文件
#define PCD_FILE_ZDINDEX_5			    "../config/mboardFiles/PcdZDIndex5.txt"				//物理枪号5账单索引文件
#define PCD_FILE_ZDINDEX_6			    "../config/mboardFiles/PcdZDIndex6.txt"				//物理枪号6账单索引文件

//铁电参数铁电存储位置
#define PCD_FM_DATALEN			93				//铁电有效数据长度，不包括校验
#define PCD_FM_TTC					(0*4)		//当前TTC，4bytes
#define PCD_FM_UNLOAD			(1*4)		//未上传账单数量，4bytes
#define PCD_FM_TTC1					(2*4)		//油枪当前TTC，1号枪，4bytes
#define PCD_FM_TTC2					(3*4)		//油枪当前TTC，2号枪，4bytes
#define PCD_FM_TTC3					(4*4)		//油枪当前TTC，3号枪，4bytes
#define PCD_FM_TTC4					(5*4)		//油枪当前TTC，4号枪，4bytes
#define PCD_FM_TTC5					(6*4)		//油枪当前TTC，5号枪，4bytes
#define PCD_FM_TTC6					(7*4)		//油枪当前TTC，6号枪，4bytes
#define PCD_FM_ZDNUM1			(8*4)		//油枪账单数目，1号枪，4bytes
#define PCD_FM_ZDNUM2			(9*4)		//油枪账单数目，2号枪，4bytes
#define PCD_FM_ZDNUM3			(10*4)		//油枪账单数目，3号枪，4bytes
#define PCD_FM_ZDNUM4			(11*4)		//油枪账单数目，4号枪，4bytes
#define PCD_FM_ZDNUM5			(12*4)		//油枪账单数目，5号枪，4bytes
#define PCD_FM_ZDNUM6			(13*4)		//油枪账单数目，6号枪，4bytes
#define PCD_FM_UNLOAD1			(14*4)		//油枪未上传账单数目，1号枪，4bytes
#define PCD_FM_UNLOAD2			(15*4)		//油枪未上传账单数目，2号枪，4bytes
#define PCD_FM_UNLOAD3			(16*4)		//油枪未上传账单数目，3号枪，4bytes
#define PCD_FM_UNLOAD4			(17*4)		//油枪未上传账单数目，4号枪，4bytes
#define PCD_FM_UNLOAD5			(18*4)		//油枪未上传账单数目，5号枪，4bytes
#define PCD_FM_UNLOAD6			(19*4)		//油枪未上传账单数目，6号枪，4bytes
#define PCD_FM_YCZDNUM			(20*4)		//异常账单数目，4bytes
#define PCD_FM_DLEN					(21*4)		//下载数据总长度，4bytes
#define PCD_FM_DOFFSET			(22*4)		//下载数据偏移长度，4bytes
#define PCD_FM_DCONTENT		92				//下载数据内容0xff=无数据下载，1byte
#define PCD_FM_CRC					93				//前N字节CRC校验，2bytes
																		//95~255备用
#define PCD_FM_BACKUP			256			//256~511为数据备份


//PCD与PC通讯命令字
#define PCD2PC_CMD_POLL					0x30		//普通查询命令
#define PCD2PC_CMD_REALINFO			0x31		//加油机发送实时信息命令
#define PCD2PC_CMD_ZDUPLOAD			0x32		//加油机上送成交数据命令
#define PCD2PC_CMD_DOWNSTART		0x33		//加油机启动下载数据命令
#define PCD2PC_CMD_DOWNLOAD		0x34		//加油机下载数据命令
#define PCD2PC_CMD_GREYINFO			0x35		//加油机向PC查询灰记录命令
#define PCD2PC_CMD_LISTINFO			0x36		//加油机向PC查询黑/白名单信息命令
#define PCD2PC_CMD_SUMREAD			0x38		//PC读取加油机累计数命令
#define PCD2PC_CMD_INFOREAD			0x3A		//PC读取加油机信息命令
#define PCD2PC_CMD_ERRINFO				0x3B		//加油机上送内部出错信息命令
#define PCD2PC_CMD_ERRACK				0x3C		//PC通讯接受确认命令
#define PCD2PC_CMD_ZDREAD				0x3E		//PC读取加油数据命令
#define PCD2PC_CMD_ZDNO					0x3F		//加油机回应PC无相关账单命令
#define PCD2PC_CMD_BAR_CHECK		0x60		//加油机向后台查询验证码命令
#define PCD2PC_CMD_BAR_RESULT		0x61		//后台向加油机返回的验证码查询结果
#define PCD2PC_CMD_BAR_ACK			0x62		//加油机向后台发送验证码查询确认结果命令
#define PCD2PC_CMD_BAR_ACKDONE	0x63		//后台确认锁定结果命令
#define PCD2PC_CMD_DISCOUT_ASK	0x70		//向后台申请折扣额，联达系统加油机命令，20160401
#define PCD2PC_CMD_APPLY_DEBIT		0x71		//向后台申请ETC卡扣款
#define PCD2PC_CMD_ETC_FUN        0x80      //ETC交易命令


//PCD与IPT通讯命令字
#define PCD_CMD_POLL							0x01				//普通查询命令
#define PCD_CMD_FORTTC						0x02				//IPT申请TTC
#define PCD_CMD_ZDSAVE						0x03				//IPT申请保存账单
#define PCD_CMD_LIST							0x04				//IPT查询黑/白名单
#define PCD_CMD_GREYINFO					0x05				//IPT查询灰锁记录
#define PCD_CMD_PRINTE						0x06				//IPT打印数据
#define PCD_CMD_SPK								0x07				//IPT语音数据
#define PCD_CMD_ZDCHECK					0x08				//查询账单
#define PCD_CMD_IDSET							0x09				//设置主板ID
#define PCD_CMD_BARCODE					0x0a				//条码数据转发
#define PCD_CMD_FOR_TMAC					0x0b				//PCD申请IPT计算TMAC
#define PCD_CMD_ERRINFO_UPLOAD	0x0c				//IPT通过PCD上传内部出错信息到后台
#define PCD_CMD_DISCOUNT_ASK		0x0d				//IPT通过PCD向后台申请折扣额
#define PCD_CMD_APPLYFOR_DEBIT	0x0e				//申请扣款

//PCD与IPT消息类型
#define PCD_MSGTYPE_IPT					1					//PCD与IPT间通讯消息类型


//黑/白名单结构
typedef struct
{
	unsigned char Version[2];							//版本号
	unsigned char TimeStart[4];						//生效日期
	unsigned char TimeFinish[4];					//截止日期
	unsigned char Area[2];								//有效区域
	unsigned char Number[4];						//名单数量
}PcdListInfoType;

//油站通用信息结构
typedef struct
{
	unsigned char Version;								//版本号
	unsigned char Province;							//省代码
	unsigned char City;									//地市代码
	unsigned char Superior[4];						//上级单位代码
	unsigned char S_ID[4];								//加油站ID
	unsigned char POS_P;								//通讯终端逻辑编号
	unsigned char GUN_N;								//枪数1~6
	unsigned char NZN[PCD_NOZZLE_MAX];	//枪号
}PcdStationInfoType;

//油品油价记录结构，每条枪最多存储价格数目为8
typedef struct
{
	unsigned char NZN;									//枪号，HEX
	unsigned char O_TYPE[2];						//油品代码，压缩BCD
	unsigned char Density[4];							//密度，HEX
	unsigned char Price_n;								//价格数目1~3，HEX
	unsigned char Price[6];								//价格，HEX，2bytes记录一个单价
}PcdOilFiledType;

//油品油价表结构，最多存储六条枪的油价纪录
typedef struct
{
	unsigned char Version;											//版本，HEX
	unsigned char ValidTime[6];									//新油品油价生效时间，压缩BCD
	unsigned char FieldNumber;									//记录数，HEX
	PcdOilFiledType Field[PCD_NOZZLE_MAX];			//当前油品油价记录
	PcdOilFiledType FieldNew[PCD_NOZZLE_MAX];		//新油品油价记录
}PcdOilRecordType;

//物理油枪信息
typedef struct
{
	unsigned char State;									//状态；0=空闲；1=卡插入；2=抬枪或加油中
	unsigned char LogicNozzle;						//逻辑枪号
	unsigned char SumMoney[4];					//总累金额
	unsigned char SumVolume[4];					//总累油量
	unsigned char CardID[10];						//卡应用号
	unsigned char CardState[2];					//卡状态
	unsigned char CardBalance[4];					//卡余额
	unsigned char PayUnit;								//结算单位/方式
	unsigned char Money[3];							//数额
	unsigned char Volume[3];							//升数
	unsigned char Price[2];								//价格
}PcdNozzleInfoType;

//支付终端信息
typedef struct
{
	unsigned int  OfflineTimer;						//支付终端离线时间
	unsigned char IsOnline;							//支付终端是否在线(即是否与PCD通讯正常)，0=离线；1=在线
	unsigned char PhysicalNozzle;					//物理枪号
	unsigned char NozzleNumber;					//加油枪数目

	PcdNozzleInfoType Nozzle[3];					//加油枪信息
}PcdFuellingPointInfoType;

//PC相关信息
typedef struct
{
	PcdStationInfoType SInfo;		//通用信息

	PcdListInfoType BList;				//基础黑名单
	
	PcdListInfoType ABList;			//新增黑名单
	
	PcdListInfoType DBList;			//新删黑名单

	PcdListInfoType WList;				//白名单

	PcdOilRecordType OilInfo;			//油品油价信息
	
	unsigned char Time[7];				//时间
}PcdPcInfoType;

typedef struct
{
	NODE	Ndptrs;						      //节点指针
	unsigned char msgType;			  //IPT消息类型
	unsigned char mboardId;			  //IPT支付终端主板号
	unsigned char fuellingPoint;	//IPT支付终端面板号
	unsigned char phynozzle;			//IPT物理枪号
	unsigned char command;			  //IPT消息命令字
	unsigned char Buffer[128];		//需要处理的IPT数据
													      //查询黑/白名单信息时:
													      //卡号(10BCD);
													      //查询灰交易记录时:
													      //卡号(10BCD);余额(4HEX);CTC(2HEX);扣款来源(1HEX);时间(7BCD);
													      //向后台申请折扣额时:
													      //枪号(1HEX);卡号(10BCD)
	unsigned char Nbytes;				 //需要处理的IPT数据长度
}PcdIpt2PcNode;

//PCD处理相关数据
typedef struct
{
	int tIdProcess;											//PCD数据处理任务号
	int tIdPcReceive;										//PCD接收后台PC数据并解析处理
	int comFdPc;											//与后台主板见通讯串口
	int comFdMboard1;									//扩展主板与1号主板之间通讯串口
	int comFdMboard2;									//1号主板与2号扩展主板之间通讯串口
	int comFdMboard3;									//1号主板与3号扩展主板之间通讯接口
	int comFdPrint1;										//本主板1号打印机连接串口
	int comFdPrint2;										//本主板2号打印机连接串口
	unsigned char mboardID;							//本地主板号1~3
	char PcTime[7];										//后台PC时间
	
	//WDOG_ID wdgId;									//看门狗定时器ID

	//支付终端信息
	// 	最多只支持6各支付支付终端，对应每个键盘，对应物理油枪；
	//	每个支付终端最多支持3条逻辑油枪，即对应后台配置的逻辑机号；
	
	PcdFuellingPointInfoType FP_1;				//物理枪号1信息
	PcdFuellingPointInfoType FP_2;				//物理枪号2信息
	PcdFuellingPointInfoType FP_3;				//物理枪号3信息
	PcdFuellingPointInfoType FP_4;				//物理枪号4信息
	PcdFuellingPointInfoType FP_5;				//物理枪号5信息
	PcdFuellingPointInfoType FP_6;				//物理枪号6信息

	//PCD处理的数据
	//MSG_Q_ID msgIdRx;								//PCD处理的数据

	//PCD与IPT间通讯相关数据
	//MSG_Q_ID msgIdFromIpt;						//PCD接收IPT数据的消息队列     
    
	int msgIdFromIpt;     //fj:PCD接收IPT数据的消息队列
	int msgIdFromPc;      //fj:PCD接收PC数据的消息队列
	int msgIdRx;          //fj:PCD接收来自后台的数据请求队列


	LIST ipt2PcList;										//IPT向PC查询数据的链表
	PcdIpt2PcNode *ipt2PcNode;					//IPT向PC查询数据的数据节点

	//PCD与PC间通讯相关数据
	//MSG_Q_ID msgIdFromPc;						//PCD接收PC数据的消息队列
	unsigned char pcFrame;							//PCD与PC间通讯帧号
	unsigned char pcCommand;						//向PC发送的数据命令
	unsigned int pcSendTimer;						//向PC发送的时间间隔
	unsigned int pcOvertimer;						//向PC发送的数据返回超时时间
	unsigned char pcOverTimes;					//向PC发送的数据返回超时次数
	unsigned char PcOnline;							//PC后台在线/离线状态，0=离线；1=在线

	//数据下载相关参数
	unsigned char PcDownloadContent;					//下载数据内容，0=基础黑名单，1=新增黑名单，
																			//	2=新删黑名单，3=白名单，4=油品油价信息，5=通用信息
																			//	其它(0xff)表示无数据下载
	unsigned int PcDownloadLen;							//下载数据内容总长度
	unsigned int PcOffsetLen;								//下载数据内容已下载长度
	unsigned char PcBaseListTimes;						//基础黑名单判断到与后台版本不一致的次数
	unsigned char PcAddListTimes;						//新增黑名单判断到与后台版本不一致的次数
	unsigned char PcDelListTimes;							//新删黑名单判断到与后台版本不一致的次数
	unsigned char PcWListTimes;							//白黑名单判断到与后台版本不一致的次数
	unsigned char PcStaionInfoTimes;					//通用信息判断到与后台版本不一致的次数
	unsigned char PcPriceInfoTimes;						//油品油价表判断到与后台版本不一致的次数
	
	
	//账单存储及上传相关参数
	unsigned int TTC;												//TTC，即加油数据当前TTC
	unsigned int UnloadNumber;							//未上传账单数量
	unsigned int UploadTTC;									//本次需要上传的账单TTC
	unsigned int UploadTimer;								//账单未完成超时定时器
	unsigned char NewZD;										//0=未读到新账单，1=有新账单
	unsigned char UploadZD[PCD_ZD_LEN];			//待上传账单

	//逃卡账单数目
	unsigned int AbnormalNumber;						//异常账单数目

	//根据物理枪号保存的各油枪数据
	unsigned int TTC1;											//1号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber1;									//1号枪，账单数量
	unsigned int UnloadNumber1;							//1号枪，未上传账单数量
	unsigned int TTC2;											//2号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber2;									//2号枪，未上传账单数量
	unsigned int UnloadNumber2;							//2号枪，未上传账单数量
	unsigned int TTC3;											//3号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber3;									//3号枪，未上传账单数量
	unsigned int UnloadNumber3;							//3号枪，未上传账单数量
	unsigned int TTC4;											//4号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber4;									//4号枪，未上传账单数量
	unsigned int UnloadNumber4;							//4号枪，未上传账单数量
	unsigned int TTC5;											//5号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber5;									//5号枪，未上传账单数量
	unsigned int UnloadNumber5;							//5号枪，未上传账单数量
	unsigned int TTC6;											//6号枪，TTC，即加油数据当前TTC
	unsigned int ZDNumber6;									//6号枪，未上传账单数量
	unsigned int UnloadNumber6;							//6号枪，未上传账单数量

	unsigned char UnloadFlag;                           //下载标识控制,szb_fj20171120
}PcdParamStructType;


//外部函数声明
extern bool pcdInit(void);
extern void pcdExit(void);
//extern MSG_Q_ID pcdMsgIdRead(void);
int pcdMsgIdRead(void);

extern int pcdMboardIDWrite(unsigned char mboard_id);
extern int pcdFmWrite(off_t param_offset, unsigned char *buffer, int nbytes);
extern unsigned char pcdMboardIDRead(void);
extern int pcdApplyForTTC(int phynozzle, const char *inbuffer, int nbytes, unsigned int *ttc);
extern int pcdApplyForBillSave(int phynozzle, const  char *inbuffer, int nbytes);
extern int pcdApplyForAuthETCDebit(char *inbuffer, int nbytes, char *outbuffer, int maxbytes);
extern int pcdParamInit(void);

extern int pcdOilRecordRead(PcdOilRecordType *oilinfo);
extern int pcdStationInfoRead(PcdStationInfoType *stationinfo);

extern int pcd2PcSend(unsigned char *buffer,int nbytes); //szb_fj20171120

void tPcd2PcRx();
void tPcdProcess();

void pcdWdgIsr();



#endif

