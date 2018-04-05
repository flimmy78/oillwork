#ifndef _OIL_IC_H_
#define _OIL_IC_H_

#include <semaphore.h>


#define ICDATA_LEN					512	//IPT接收IC卡数据最大长度											                                                                 
#define IC_OVERTIME					(5*1000)	//卡命令超时时间,5秒
#define ICSTATE_OK                  0  //IC操作返回值，成功
#define ICSTATE_FAILED				1   //IC操作返回值代码:失败
#define ICSTATE_OVERTIME			2   //IC操作返回值代码:超时
#define ICSTATE_NOZZLEERR			3   //IC操作返回值代码:枪选非法
#define ICSTATE_CACHEERR			4   //IC操作返回值代码:输出缓存太小
//IC卡类型
#define IC_CARD_CPU					0x31    //触电CPU卡
#define IC_CARD_RF_TYPEB			0x32	//RF-- TYPE B CPU卡
#define IC_CARD_RF_TYPEA			0x33	//RF-- TYPE A CPU卡
//IC卡应用类型
#define ICTYPE_USER					0x01	//用户卡
#define ICTYPE_MANAGE				0x02	//管理卡
#define ICTYPE_STAFF				0x04	//员工卡
#define ICTYPE_PUMP					0x05	//验泵卡
#define ICTYPE_SERVICE				0x06	//维修卡
#define ICTYPE_PSAM					0x10	//PSAM卡
//IC卡座设备枪号
#define IC_NOZZLE_1					0		//1号卡座设备
#define IC_NOZZLE_2					1		//2号卡座设备

//IC卡座返回命令数据结构
typedef struct
{
	unsigned char DeckStateS1;						//S1卡座状态=0x30表示有卡,0x31表示无卡。
	unsigned char IcTypeS2;							//S2:卡类型 ,S2=0x30 卡机内无卡 ,       S2=0x3f 无法识别 ,     S2=0x31 触点cpu卡
															   //S2=0x32 RF-- TYPE B CPU卡 ,S2=0x33 RF-TYPE A CPU卡,S2=0x34 RF-M1卡

	unsigned char IcStateS3;						//S3：卡状态 ,S3=0x30 下电状态 ,S3=0x31 休眠状态
																//S4=0x32 激活状态 ,S5=0x33 忙态（通信状态）

	unsigned char PowerStateS4;						//S4：供电状态,S4=0x30 供电正常,S4=0x31 掉电																													
}IcStateType;


//卡操作相关参数
typedef struct
{
	int nozzle; //枪选ID
	IcStateType State; //接收数据
	unsigned char RxValid;	//数据有效位:0=无效帧；1=有效帧										
	unsigned char RxBuffer[ICDATA_LEN];	//数据缓存				
	unsigned int RxLen;	 //数据长度

	//del SEM_ID semIdShoot;
	pthread_mutex_t semIdShoot;   //弹卡操作信号量

	char shootAsk; //是否需要弹卡0=不需要；1=需要
	char pollLimit;//是否禁止轮询命令，0=允许；1=禁止
	int tIdReceive; //卡数据接收任务ID
	int tIdShoot; //弹卡任务ID
	int tIdPoll; //卡状态轮询任务ID
}IcStructType;



//IC卡弹卡
extern int ICShoot(int nozzle);
//IC卡复位
extern int ICReset(int nozzle, int sam);
//IC卡MF选择
extern int ICMFSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡应用选择
extern int ICAppSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char app);
//IC卡21文件读取
extern int ICFile21Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡22文件读取
extern int ICFile22Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡27文件读取
extern int ICFile27Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡28文件读取
extern int ICFile28Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡交易明细读取
extern int ICNotesRead(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char notes_id);
//IC卡密码校验
extern int ICPinCheck(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *pin, int pin_len);
//IC卡灰锁信息读取
extern int ICGreyInfoRead(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡余额读取
extern int ICBalanceRead(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡灰锁初始化
extern int ICLockInit(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char keyIndex, unsigned char *termId);
//IC卡灰锁
extern int ICGreyLock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *psamTTC, unsigned char *psamRandom, unsigned char *time, unsigned char *psamMAC1);
//IC卡解灰
extern int ICGreyUnlock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned int money,unsigned char *ICLockInitCTC, unsigned char *PsamTermId, unsigned char *ICPsamTTC,unsigned char *time, unsigned char *ICPsamGMAC);
//IC卡清除TAC
extern int ICTacClr(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡26文件读取
extern int ICFile26Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC卡专用DES计算(DES CRYPT)
extern int ICDESCrypt(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *PSAMRandom);
//IC卡添加日志记录
extern int ICAppendLog(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardId, unsigned char *RIDMAC);


//PSAM卡复位
extern int PsamReset(int nozzle,int sam);
//PSAM卡MF选择
extern int PsamMFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡21文件读取
extern int PsamFile21Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡22文件读取
extern int PsamFile22Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡DF选择石化应用
extern int PsamDFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes, int DF);
//PSAM卡23文件读取
extern int PsamFile23Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡获取安全提升状态
extern int PsamGetAPProof(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡获取随机数
extern int PsamGetRandom(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡安全提升认证
extern int PsamAPAuthen(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *Ciphertext);
//PSAM灰锁初始化，计算MAC1	
extern int PsamLockInit(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICLockInitRandom, unsigned char *ICLockInitCTC, unsigned char *ICLockInitBalance, unsigned char *time, unsigned char ICLockInitKeysVersion, unsigned char ICLockInitArithmetic, unsigned char *ICAppId, unsigned char *ICIssuerMark);
//PSAM验证MAC2
extern int PsamMAC2Check(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *MAC2);
//PSAM计算GMAC
extern int PsamGMAC(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICAppId, unsigned char *ICLockInitCTC, unsigned int money);
//PSAM读取GMAC
extern int PsamGMACRead(int nozzle,int sam, unsigned char *buffer, int maxbytes, const unsigned char *ICPsamTTC);
//PSAM卡TMAC计算初始化
extern int PsamTMACInit(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM卡计算TMAC
extern int PsamTMACOperat(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *inbuffer, int len, int follow, int initvalue);
//PSAM卡启动计量注册功能
extern int PsamStartBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *Ciphertext);
//PSAM卡初始化计量注册功能
extern int PsamInitBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *CoID, unsigned char *Ciphertext);
//PSAM卡计量注册
extern int PsamBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char const *MboardID, unsigned char *CoID);
//PSAM卡专用DES计算初始化
extern int PsamInitDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char CalKeyIndex);
//PSAM卡专用DES计算
extern int PsamDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardId);


//卡座测试相关命令
//进入卡座电特性测试
extern int IcReaderTestElectrical(int nozzle, int type);
//进入卡座协议测试
extern int IcReaderTestProtocol(int nozzle, int type);
//进入卡座射频测试
extern int IcReaderTestRadiofrequency(int nozzle);
//设置协议连续测试轮询时间
extern int IcReaderTestProtocolTime(int nozzle, int time);
//退出测试模式
extern int IcReaderTestExit(int nozzle);
//禁止轮询命令
extern int IcPollLimit(int nozzle);
//启动轮寻命令
extern int IcPollStart(int nozzle);


//密钥下载卡专有命令
//IC密钥卡ADF选择
extern int ICKeyADFSelect(int nozzle, unsigned char *buffer, int maxbytes);
//IC密钥卡选择EF 01文件
extern int ICKeyEF01Select(int nozzle, unsigned char *buffer, int maxbytes);
//IC密钥卡EF 01文件读取
extern int ICKeyEF01Read(int nozzle, unsigned char *buffer, int maxbytes);
//IC密钥卡EF 01文件写入
extern int ICKeyEF01Write(int nozzle, unsigned char *buffer, int maxbytes, unsigned int number);
//IC密钥卡选择EF 02文件
extern int ICKeyEF02Select(int nozzle, unsigned char *buffer, int maxbytes);
//IC密钥卡EF 02文件读取
extern int ICKeyEF02Read(int nozzle, unsigned char *buffer, int maxbytes, unsigned int offset, int readbytes);


extern int ICStateRead(int nozzle, IcStateType *state);//IC状态读取
extern bool IcModuleInit(void);//IC模块初始化 

extern void ResetIcPack(int nozzle);

void tICReceive(void* pNozzleType);
void tICPoll(void* pNozzleType);

#endif


