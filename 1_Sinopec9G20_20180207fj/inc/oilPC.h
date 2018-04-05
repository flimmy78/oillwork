#ifndef _OIL_PC_H_
#define _OIL_PC_H_



//油机信息
typedef struct
{
	unsigned CardState:1;		//卡状态 0 无卡  1有卡
	unsigned OilState:1;		//加油状态 1  加油中
	unsigned OilEndState:1;		//加油结束状态  1  结束

	//有卡时
	char CardID[10];						//卡号 BCD
	char CardBalance[4];				//卡余额 HEX
	char Shopping;							//购物允许标志 0x01 允许
	
	//加油时
	char OilMoney[4];					//加油金额 HEX
	char OilVolume[4];					//加油升数 HEX
	char OilPrice[3];						//加油价格 HEX
	
	//加油结束时
	char EndMoney[4];					//加油金额 HEX
	char EndVolume[4];					//加油升数 HEX
	char EndPrice[3];						//加油价格 HEX
	
}PCOilInfoType;

//卡信息
typedef struct
{
	unsigned char IcValid;							   //是否有有效卡信息 0=无；1=有

	unsigned char IcAppSelect;							//应用选择 0=电子油票；1=积分应用
	unsigned char IcAppId[10];							//应用序列号
	unsigned char IcEnableTime[4];					//应用启用日期
	unsigned char IcInvalidTime[4];					//应用有效截止日期
	unsigned char IcUserName[20];					//持卡人姓名
	unsigned char IcUserIdeId[18];					//持卡人证件(identity)号码(ASCII)
	unsigned char IcUserIdeType;						//持卡人证件类型
	unsigned char IcOilLimit[2];							//油品限制
	unsigned char IcRegionTypeLimit;				//限地区,油站加油方式
	unsigned char IcRegionLimit[40];					//限地区,油站加油
	unsigned char IcVolumeLimit[2];					//限每次加油量
	unsigned char IcTimesLimit;						//限每天加油次数
	unsigned char IcMoneyDayLimit[4];			//限每天加油金额
	unsigned char IcCarIdLimit[16];					//车牌号限制(ASCII)

}PCCardInfoType;

//卡交易明细
typedef struct
{
	unsigned char TTC[2];						//ET联机或脱机交易序号
	unsigned char Limit[3];						//透支限额
	unsigned char Money[4];					//交易金额
	unsigned char Type;							//交易类型标识
	unsigned char TermID[6];				//终端机编号
	unsigned char Time[7];						//交易时间

}PCCardRecordType;

//连接平板电脑的串口
#define PC_COM_1					COM9		//1号面板平板电脑连接的串口
#define PC_COM_2					COM10		//2号面板平板电脑连接的串口


//平板电脑面板号
#define PC_PANEL_1				0				//1号面板平板电脑ID
#define PC_PANEL_2				1				//2号面板平板电脑ID

//是否允许购物
#define PC_SHOPPING_ALLOW				1			//0=不允许；1=允许

//平板电脑与主板通讯命令
#define PC_CMD_ACTION_UPLOAD		0x54		//(油机->平板)通知平板电脑加油机的操作动作
#define PC_CMD_STATE_UPLOAD			0x55		//(油机->平板)通知平板电脑加油机的状态
#define PC_CMD_VOICE_PLAY				0x56		//(油机->平板)语音播放
#define PC_CMD_OILINFO						0x57		//(油机->平板)加油机信息上送
#define PC_CMD_KEYUPLOAD					0x58		//(油机->平板)上送拍奖按键值
#define PC_CMD_SHAKEHANDS				0x59		//(平板->油机)平板电脑握手
#define PC_CMD_PCPARAM						0x5a		//(油机->平板)平板参数信息查询设置
#define PC_CMD_LIGHT_CONTROL		0x5b		//(平板->油机)平板电脑主动命令-包括部分平板电脑主动发送的其它命令
#define PC_CMD_LET_PC_DONE				0x5c		//(油机->平板)加油机触发平板申请抽奖
#define PC_CMD_DATA_REUP					0x5d		//(平板->油机)平板电脑转发后台数据给油机
#define PC_CMD_EACH_OTHER				0x5e		//(油机->平板)加油机与平板支付交互
#define PC_CMD_GET_VOLUME				0x5f		//(油机->平板)加油机获取平板电脑音量
#define PC_CMD_GET_CARDINFO			0x60		//(平板->油机)平板电脑读取油机卡片信息、卡交易记录
#define PC_CMD_OIL_ACK						0x61		//(油机->平板)油品确认界面
#define PC_CMD_BILL_UPLOAD				0x63		//(油机->平板)油机上传加油明细
#define PC_CMD_PASS_UPLOAD				0x64		//(油机->平板)油机上传密码
#define PC_CMD_DEBIT_RESULT			0x65		//(油机->平板)油机上传消费结果

//油机操作动作代码
#define PC_ACTION_SENSOR					0x00		//人体感应
#define PC_ACTION_PLAY						0x01		//拍奖按钮
#define PC_ACTION_YP							0x02		//油品选择
#define PC_ACTION_BUTTON					0x03		//按键
#define PC_ACTION_GUN							0x04		//提挂枪
#define PC_ACTION_PLAY_3SECOND		0x05		//3秒长按PLAY按钮
#define PC_ACTION_PLAY_4SECOND		0x06		//4秒长按PLAY按钮
#define PC_ACTION_PLAY_5SECOND		0x07		//5秒长按PLAY按钮

#define PC_FUN_GRADE_OK				0x30		//启用油品确认
#define PC_FUN_GRADE_NO				0x31		//禁用油品确认


//函数声明
extern int pcIsApplyPause(int panel, int phynozzle);
extern int pcNetInfoSet(int panel, int phynozzle, const char *ip, const char *mask, const char *gateway);
extern int pcNetInfoGet(int panel, int phynozzle, char *ip, char *mask, char *gateway);
extern int pcDNSInfoSet(int panel, int phynozzle, const char *first, const char *second);
extern int pcDNSInfoGet(int panel, int phynozzle, char *first, char *second);
extern int pcFtpServerInfoSet(int panel, int phynozzle, const char *ftpIp, const char *ftpPort);
extern int pcFtpServerInfoGet(int panel, int phynozzle, char *ftpIp, char *ftpPort);
extern int pcServerInfoSet(int panel, int phynozzle, const char *serverIp);
extern int pcServerInfoGet(int panel, int phynozzle, char *serverIp);
extern int pcVolumeSet(int panel, int phynozzle, unsigned char volume);
extern int pcVolumeGet(int panel, int phynozzle, unsigned char *volume);
extern int pcTelephoneIPSet(int panel, int phynozzle, const char *ip);
extern int pcTelephoneIPGet(int panel, int phynozzle, char *ip);

extern int pcPsaawordUpload(int panel, int logicnozzle, char *inbuffer);
extern int pcDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes);
extern int pcCardDebitResultUpload(int panel, int logicnozzle, char *inbuffer, int nbytes);
extern int pcYPLightTurnOn(int panel, int phynozzle);
extern int pcYPLightTurnOff(int panel, int phynozzle);

extern int pcCardInfoWrite(int panel, int phynozzle, PCCardInfoType info);
extern int pcCardRecordWrite(int panel, int phynozzle, PCCardRecordType info[], int number, char app);

extern int pcVoicePlay(int panel, int *voice, int number);
extern int pcOilInfoRead(int panel, int phynozzle, PCOilInfoType *info);
extern int pcBillWrite(int panel, int phynozzle, const char *buffer, int nbytes);
extern int pcOilConfirm(int panel, int phynozzle, int confirm, char *oilname);
extern int pcActionUpload(int panel, int phynozzle, int action);
extern int pcStateUpload(int panel, int phynozzle, int state, char *inbuffer, int nbytes);

extern int pcUserIDSet(int panel, char id);
extern int pcUserIDGet(int panel);
extern int pcLotteryStart(int panel, int phynozzle);
extern bool pcInit(void);

extern void tPcCommandProcess(int panel);
extern void tPcTxCommandProcess(int panel);
extern void tPcReceive(int panel);


#endif



