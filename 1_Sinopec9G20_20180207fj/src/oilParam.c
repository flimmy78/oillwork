//#include "oilCfg.h"
//#include "config.h"
//#include "oilStmTransmit.h"
//#include "oilParam.h"

#include "../inc/main.h"


//
const char *ModelParam[] = 
{
	"石化卡机联动加油机",
	"社会站加油机",
	"联达系统加油机",
};

/*
//配置文件描述符
static int fdSetup=ERROR;

//配置文件操作互斥信号量
static SEM_ID semIdSetup=NULL;

//异常记录文件描述符
static int FdErrInfo=ERROR;

//异常记录文件信号量
static SEM_ID SemIdErrInfo=NULL;

//日志文件描述符
static int FdRecord=ERROR;

//日志文件信号量
static SEM_ID SemIdRecord=NULL;*/

//fj:20170905
static int fdSetup=ERROR; //配置文件描述符
pthread_rwlock_t rwlock_Setup;//配置文件操作互斥信号量
static int FdErrInfo=ERROR; //异常记录文件描述符
pthread_rwlock_t rwLock_ErrInfo; //异常记录文件信号量
static int FdRecord=ERROR; //日志文件描述符
pthread_rwlock_t rwlock_Record; //日志文件信号量


/*主板机型*/
static int ParamModel = ERROR;

/*单面枪数 PANEL_NOZZLE_SINGLE / PANEL_NOZZLE_DOUBLE*/
static int ParamPanelNozzle = ERROR;

/*是否启用促销功能 0 = 不启用；1 = 启用；*/
static int ParamPromotion = ERROR;


/********************************************************************
*Name				:paramSetupWrite
*Description		:修改配置文件选项，有数据的校验
*Input				:offset		参数内容
*						:buffer		参数内容
*						:nbytes	参数长度，不大于30
*Output			:None
*Return				:成功返回0；错误返回其它
*History			:2014-02-24,modified by syj
*/
int paramSetupWrite(off_t offset, const unsigned char *buffer, int nbytes)
{
	unsigned char wrbuffer[64]={0}, rdbuffer[64]={0};
	int istate=0, wrbytes=0, rdbytes=0;
	unsigned int file_offset=0, param_name_len=0;

	/*判断参数长度*/
	if(nbytes>30)
	{
		return ERROR;
	}

	/*参数组包，参数包格式为:"参数名"(最大30字节)	+	"="	+	"参数内容"(最大30字节)	+	"\r\n"*/
	switch(offset)
	{
	case JL0_EQUIVALENT:////////////////////////////////////////////////////////////////////////////////////
		file_offset=JL0_EQUIVALENT;	param_name_len=14;
		memcpy(&wrbuffer[0], "JL0_EQUIVALENT", param_name_len);
		break;
	case JL0_UNPULSE_TIME:
		file_offset=JL0_UNPULSE_TIME;	param_name_len=16;
		memcpy(&wrbuffer[0], "JL0_UNPULSE_TIME", param_name_len);
		break;
	case JL0_ADVANCE:
		file_offset=JL0_ADVANCE;	param_name_len=11;
		memcpy(&wrbuffer[0], "JL0_ADVANCE", param_name_len);
		break;
	case JL0_SHIELD:
		file_offset=JL0_SHIELD;	param_name_len=10;
		memcpy(&wrbuffer[0], "JL0_SHIELD", param_name_len);
		break;
	case JL0_OVER_SHIELD	:
		file_offset=JL0_OVER_SHIELD;	param_name_len=15;
		memcpy(&wrbuffer[0], "JL0_OVER_SHIELD", param_name_len);
		break;
	case JL0_TYPE:
		file_offset=JL0_TYPE;	param_name_len=8;
		memcpy(&wrbuffer[0], "JL0_TYPE", param_name_len);
		break;
	case JL0_PRICE:
		file_offset=JL0_PRICE;	param_name_len=9;
		memcpy(&wrbuffer[0], "JL0_PRICE", param_name_len);
		break;
	case JL0_VAVLE_VOLUME:
		file_offset=JL0_VAVLE_VOLUME;	param_name_len=16;
		memcpy(&wrbuffer[0], "JL0_VAVLE_VOLUME", param_name_len);
		break;
	case JL0_VAVLE_STOP:
		file_offset=JL0_VAVLE_STOP;	param_name_len=14;
		memcpy(&wrbuffer[0], "JL0_VAVLE_STOP", param_name_len);
		break;
	case JL0_ALGORITHM:		//(A1枪)计量无脉冲时间超过此值时关闭大阀,4HEX
		file_offset=JL0_ALGORITHM;	param_name_len=strlen("JL0_ALGORITHM");
		memcpy(&wrbuffer[0], "JL0_ALGORITHM", param_name_len);
		break;
    case JL0_BIG_VOL_TIME://szb_fj_20171120,add,(A1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8
		file_offset=JL0_BIG_VOL_TIME;	param_name_len=strlen("JL0_BIG_VOL_TIME");
		memcpy(&wrbuffer[0], "JL0_BIG_VOL_TIME", param_name_len);
		break;
	case JL0_BIG_VOL_SPEED://szb_fj_20171120,add,(A1枪)双编码器大流量流速控制1BCD L/分10-80默认15
		file_offset=JL0_BIG_VOL_SPEED;	param_name_len=strlen("JL0_BIG_VOL_SPEED");
		memcpy(&wrbuffer[0], "JL0_BIG_VOL_SPEED", param_name_len);
		break;


	case JL1_EQUIVALENT://////////////////////////////////////////////////////////////////////////////////////
		file_offset=JL1_EQUIVALENT;	param_name_len=14;
		memcpy(&wrbuffer[0], "JL1_EQUIVALENT", param_name_len);
		break;
	case JL1_UNPULSE_TIME:
		file_offset=JL1_UNPULSE_TIME;	param_name_len=16;
		memcpy(&wrbuffer[0], "JL1_UNPULSE_TIME", param_name_len);
		break;
	case JL1_ADVANCE:
		file_offset=JL1_ADVANCE;	param_name_len=11;
		memcpy(&wrbuffer[0], "JL1_ADVANCE", param_name_len);
		break;
	case JL1_SHIELD:
		file_offset=JL1_SHIELD;	param_name_len=10;
		memcpy(&wrbuffer[0], "JL1_SHIELD", param_name_len);
		break;
	case JL1_OVER_SHIELD	:
		file_offset=JL1_OVER_SHIELD;	param_name_len=15;
		memcpy(&wrbuffer[0], "JL1_OVER_SHIELD", param_name_len);
		break;
	case JL1_TYPE:
		file_offset=JL1_TYPE;	param_name_len=8;
		memcpy(&wrbuffer[0], "JL1_TYPE", param_name_len);
		break;
	case JL1_PRICE:
		file_offset=JL1_PRICE;	param_name_len=9;
		memcpy(&wrbuffer[0], "JL1_PRICE", param_name_len);
		break;
	case JL1_VAVLE_VOLUME:
		file_offset=JL1_VAVLE_VOLUME;	param_name_len=16;
		memcpy(&wrbuffer[0], "JL1_VAVLE_VOLUME", param_name_len);
		break;
	case JL1_VAVLE_STOP:
		file_offset=JL1_VAVLE_STOP;	param_name_len=14;
		memcpy(&wrbuffer[0], "JL1_VAVLE_STOP", param_name_len);
		break;
	case JL1_ALGORITHM:		//(B1枪)计量无脉冲时间超过此值时关闭大阀,4HEX
		file_offset=JL1_ALGORITHM;	param_name_len=strlen("JL1_ALGORITHM");
		memcpy(&wrbuffer[0], "JL1_ALGORITHM", param_name_len);
		break;
	case JL1_BIG_VOL_TIME:  //szb_fj_20171120,add,(B1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8
		file_offset=JL1_BIG_VOL_TIME;	param_name_len=strlen("JL1_BIG_VOL_TIME");
		memcpy(&wrbuffer[0], "JL1_BIG_VOL_TIME", param_name_len);
		break;
	case JL1_BIG_VOL_SPEED:  //szb_fj_20171120,add,(B1枪)双编码器大流量流速控制1BCD L/分10-80默认15
		file_offset=JL1_BIG_VOL_SPEED;	param_name_len=strlen("JL1_BIG_VOL_SPEED");
		memcpy(&wrbuffer[0], "JL1_BIG_VOL_SPEED", param_name_len);
		break;


	case IPT0_DUTY_INFO://////////////////////////////////////////////////////////////////////////////////
		file_offset=IPT0_DUTY_INFO;	param_name_len=14;
		memcpy(&wrbuffer[0], "IPT0_DUTY_INFO", param_name_len);
		break;
	case IPT0_VOICE_SPEAKER:
		file_offset=IPT0_VOICE_SPEAKER;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT0_VOICE_SPEAKER", param_name_len);
		break;
	case IPT0_VOICE_TYPE:
		file_offset=IPT0_VOICE_TYPE;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT0_VOICE_TYPE", param_name_len);
		break;
	case  IPT0_VOICE_VOLUME:
		file_offset=IPT0_VOICE_VOLUME;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT0_VOICE_VOLUME", param_name_len);
		break;
	case IPT0_PRINTER:
		file_offset=IPT0_PRINTER;	param_name_len=12;
		memcpy(&wrbuffer[0], "IPT0_PRINTER", param_name_len);
		break;
	case IPT0_PRINT_AUTO:
		file_offset=IPT0_PRINT_AUTO;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT0_PRINT_AUTO", param_name_len);
		break;
	case IPT0_PRINT_UNION	:
		file_offset=IPT0_PRINT_UNION;	param_name_len=16;
		memcpy(&wrbuffer[0], "IPT0_PRINT_UNION", param_name_len);
		break;
	case IPT0_PRN_CARD_USER:
		file_offset=IPT0_PRN_CARD_USER;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT0_PRN_CARD_USER", param_name_len);
		break;
	case IPT0_PRN_CARD_MANAGE:
		file_offset=IPT0_PRN_CARD_MANAGE;	param_name_len=20;
		memcpy(&wrbuffer[0], "IPT0_PRN_CARD_MANAGE", param_name_len);
		break;
	case IPT0_PRN_CARD_STAFF:
		file_offset=IPT0_PRN_CARD_STAFF;	param_name_len=19;
		memcpy(&wrbuffer[0], "IPT0_PRN_CARD_STAFF", param_name_len);
		break;
	case IPT0_PRN_CARD_PUMP:
		file_offset=IPT0_PRN_CARD_PUMP;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT0_PRN_CARD_PUMP", param_name_len);
		break;
	case IPT0_PRN_CARD_SERVICE:
		file_offset=IPT0_PRN_CARD_SERVICE;	param_name_len=21;
		memcpy(&wrbuffer[0], "IPT0_PRN_CARD_SERVICE", param_name_len);
		break;
	case IPT0_NIGHT_LOCK:
		file_offset=IPT0_NIGHT_LOCK;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT0_NIGHT_LOCK", param_name_len);
		break;
	case IPT0_LOGIC_NOZZLE:
		file_offset=IPT0_LOGIC_NOZZLE;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT0_LOGIC_NOZZLE", param_name_len);
		break;
	case IPT0_PHYSICAL_NOZZLE:
		file_offset=IPT0_PHYSICAL_NOZZLE;	param_name_len=20;
		memcpy(&wrbuffer[0], "IPT0_PHYSICAL_NOZZLE", param_name_len);
		break;
	case IPT0_PASSWORD:
		file_offset=IPT0_PASSWORD;	param_name_len=13;
		memcpy(&wrbuffer[0], "IPT0_PASSWORD", param_name_len);
		break;
	case IPT0_WORKMODE:
		file_offset=IPT0_WORKMODE;	param_name_len=13;
		memcpy(&wrbuffer[0], "IPT0_WORKMODE", param_name_len);
		break;
	case IPT0_SERVICE_PASS:
		file_offset=IPT0_SERVICE_PASS;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT0_SERVICE_PASS", param_name_len);
		break;
	case IPT0_AUTHEN:
		file_offset=IPT0_AUTHEN;	param_name_len=11;
		memcpy(&wrbuffer[0], "IPT0_AUTHEN", param_name_len);
		break;
	case IPT0_PRICE_INFO:
		file_offset=IPT0_PRICE_INFO;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT0_PRICE_INFO", param_name_len);
		break;
	case IPT0_STAFF_LIMIT:
		file_offset=IPT0_STAFF_LIMIT;	param_name_len=16;
		memcpy(&wrbuffer[0], "IPT0_STAFF_LIMIT", param_name_len);
		break;
	case IPT0_BIND_TIME:					/*绑定信息，时间，7BCD*/
		file_offset=IPT0_BIND_TIME;	param_name_len=strlen("IPT0_BIND_TIME");
		memcpy(&wrbuffer[0], "IPT0_BIND_TIME", param_name_len);
		break;
	case IPT0_BIND_MBOARD_ID:		/*绑定信息，主板号，8BCD*/
		file_offset=IPT0_BIND_MBOARD_ID;	param_name_len=strlen("IPT0_BIND_MBOARD_ID");
		memcpy(&wrbuffer[0], "IPT0_BIND_MBOARD_ID", param_name_len);
		break;
	case IPT0_BIND_ACT_APPID:		/*绑定信息，ACT卡号，10BCD*/
		file_offset=IPT0_BIND_ACT_APPID;	param_name_len=strlen("IPT0_BIND_ACT_APPID");
		memcpy(&wrbuffer[0], "IPT0_BIND_ACT_APPID", param_name_len);
		break;
	case IPT0_BIND_RID_APPID:			/*绑定信息，RID卡号，10BCD*/
		file_offset=IPT0_BIND_RID_APPID;	param_name_len=strlen("IPT0_BIND_RID_APPID");
		memcpy(&wrbuffer[0], "IPT0_BIND_RID_APPID", param_name_len);
		break;
	case IPT0_OIL_VOICE:					/*油品语音代码信息，即语音文件的数字前缀，4ASCII*/
		file_offset=IPT0_OIL_VOICE;	param_name_len=strlen("IPT0_OIL_VOICE");
		memcpy(&wrbuffer[0], "IPT0_OIL_VOICE", param_name_len);
		break;
	case IPT0_CONTRAST:					/*(A1枪)键盘显示对比度，1HEX*/
		file_offset=IPT0_CONTRAST;	param_name_len=strlen("IPT0_CONTRAST");
		memcpy(&wrbuffer[0], "IPT0_CONTRAST", param_name_len);
		break;
	case IPT1_DUTY_INFO:///////////////////////////////////////////////////////////////////////////////////
		file_offset=IPT1_DUTY_INFO;	param_name_len=14;
		memcpy(&wrbuffer[0], "IPT1_DUTY_INFO", param_name_len);
		break;
	case IPT1_VOICE_SPEAKER:
		file_offset=IPT1_VOICE_SPEAKER;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT1_VOICE_SPEAKER", param_name_len);
		break;
	case IPT1_VOICE_TYPE:
		file_offset=IPT1_VOICE_TYPE;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT1_VOICE_TYPE", param_name_len);
		break;
	case  IPT1_VOICE_VOLUME:
		file_offset=IPT1_VOICE_VOLUME;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT1_VOICE_VOLUME", param_name_len);
		break;
	case IPT1_PRINTER:
		file_offset=IPT1_PRINTER;	param_name_len=12;
		memcpy(&wrbuffer[0], "IPT1_PRINTER", param_name_len);
		break;
	case IPT1_PRINT_AUTO:
		file_offset=IPT1_PRINT_AUTO;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT1_PRINT_AUTO", param_name_len);
		break;
	case IPT1_PRINT_UNION	:
		file_offset=IPT1_PRINT_UNION;	param_name_len=16;
		memcpy(&wrbuffer[0], "IPT1_PRINT_UNION", param_name_len);
		break;
	case IPT1_PRN_CARD_USER:
		file_offset=IPT1_PRN_CARD_USER;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT1_PRN_CARD_USER", param_name_len);
		break;
	case IPT1_PRN_CARD_MANAGE:
		file_offset=IPT1_PRN_CARD_MANAGE;	param_name_len=20;
		memcpy(&wrbuffer[0], "IPT1_PRN_CARD_MANAGE", param_name_len);
		break;
	case IPT1_PRN_CARD_STAFF:
		file_offset=IPT1_PRN_CARD_STAFF;	param_name_len=19;
		memcpy(&wrbuffer[0], "IPT1_PRN_CARD_STAFF", param_name_len);
		break;
	case IPT1_PRN_CARD_PUMP:
		file_offset=IPT1_PRN_CARD_PUMP;	param_name_len=18;
		memcpy(&wrbuffer[0], "IPT1_PRN_CARD_PUMP", param_name_len);
		break;
	case IPT1_PRN_CARD_SERVICE:
		file_offset=IPT1_PRN_CARD_SERVICE;	param_name_len=21;
		memcpy(&wrbuffer[0], "IPT1_PRN_CARD_SERVICE", param_name_len);
		break;
	case IPT1_NIGHT_LOCK:
		file_offset=IPT1_NIGHT_LOCK;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT1_NIGHT_LOCK", param_name_len);
		break;
	case IPT1_LOGIC_NOZZLE:
		file_offset=IPT1_LOGIC_NOZZLE;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT1_LOGIC_NOZZLE", param_name_len);
		break;
	case IPT1_PHYSICAL_NOZZLE:
		file_offset=IPT1_PHYSICAL_NOZZLE;	param_name_len=20;
		memcpy(&wrbuffer[0], "IPT1_PHYSICAL_NOZZLE", param_name_len);
		break;
	case IPT1_PASSWORD:
		file_offset=IPT1_PASSWORD;	param_name_len=13;
		memcpy(&wrbuffer[0], "IPT1_PASSWORD", param_name_len);
		break;
	case IPT1_WORKMODE:
		file_offset=IPT1_WORKMODE;	param_name_len=13;
		memcpy(&wrbuffer[0], "IPT1_WORKMODE", param_name_len);
		break;
	case IPT1_SERVICE_PASS:
		file_offset=IPT1_SERVICE_PASS;	param_name_len=17;
		memcpy(&wrbuffer[0], "IPT1_SERVICE_PASS", param_name_len);
		break; 
	case IPT1_AUTHEN:
		file_offset=IPT1_AUTHEN;	param_name_len=11;
		memcpy(&wrbuffer[0], "IPT1_AUTHEN", param_name_len);
		break;
	case IPT1_PRICE_INFO:
		file_offset=IPT1_PRICE_INFO;	param_name_len=15;
		memcpy(&wrbuffer[0], "IPT1_PRICE_INFO", param_name_len);
		break;
	case IPT1_STAFF_LIMIT:
		file_offset=IPT1_STAFF_LIMIT;	param_name_len=16;
		memcpy(&wrbuffer[0], "IPT1_STAFF_LIMIT", param_name_len);
		break;
	case IPT1_BIND_TIME:					/*绑定信息，时间，7BCD*/
		file_offset=IPT1_BIND_TIME;	param_name_len=strlen("IPT1_BIND_TIME");
		memcpy(&wrbuffer[0], "IPT1_BIND_TIME", param_name_len);
		break;
	case IPT1_BIND_MBOARD_ID:		/*绑定信息，主板号，8BCD*/
		file_offset=IPT1_BIND_MBOARD_ID;	param_name_len=strlen("IPT1_BIND_MBOARD_ID");
		memcpy(&wrbuffer[0], "IPT1_BIND_MBOARD_ID", param_name_len);
		break;
	case IPT1_BIND_ACT_APPID:		/*绑定信息，ACT卡号，10BCD*/
		file_offset=IPT1_BIND_ACT_APPID;	param_name_len=strlen("IPT1_BIND_ACT_APPID");
		memcpy(&wrbuffer[0], "IPT1_BIND_ACT_APPID", param_name_len);
		break;
	case IPT1_BIND_RID_APPID:			/*绑定信息，RID卡号，10BCD*/
		file_offset=IPT1_BIND_RID_APPID;	param_name_len=strlen("IPT1_BIND_RID_APPID");
		memcpy(&wrbuffer[0], "IPT1_BIND_RID_APPID", param_name_len);
		break;
	case IPT1_OIL_VOICE:					/*油品语音代码信息，即语音文件的数字前缀，4ASCII*/
		file_offset=IPT1_OIL_VOICE;	param_name_len=strlen("IPT1_OIL_VOICE");
		memcpy(&wrbuffer[0], "IPT1_OIL_VOICE", param_name_len);
		break;
	case IPT1_CONTRAST:					/*(A1枪)键盘显示对比度，1HEX*/
		file_offset=IPT1_CONTRAST;	param_name_len=strlen("IPT1_CONTRAST");
		memcpy(&wrbuffer[0], "IPT1_CONTRAST", param_name_len);
		break;
	case PRM_MBOARD_ID://////////////////////////////////////////////////////////////
		file_offset=PRM_MBOARD_ID;	param_name_len=13;
		memcpy(&wrbuffer[0], "PRM_MBOARD_ID", param_name_len);
		break; 
	case PRM_SELL_LOCK:
		file_offset=PRM_SELL_LOCK;	param_name_len=13;
		memcpy(&wrbuffer[0], "PRM_SELL_LOCK", param_name_len);
		break;
	case PRM_BACKLIT:
		file_offset=PRM_BACKLIT;	param_name_len=11;
		memcpy(&wrbuffer[0], "PRM_BACKLIT", param_name_len);
		break;
	case PRM_IP_ADDR:
		file_offset=PRM_IP_ADDR;	param_name_len=11;
		memcpy(&wrbuffer[0], "PRM_IP_ADDR", param_name_len);
		break;
	case PRM_EPS_ADDR:
		file_offset=PRM_EPS_ADDR;	param_name_len=12;
		memcpy(&wrbuffer[0], "PRM_EPS_ADDR", param_name_len);
		break;
	case PRM_BANK_ADDR:
		file_offset=PRM_BANK_ADDR;	param_name_len=13;
		memcpy(&wrbuffer[0], "PRM_BANK_ADDR", param_name_len);
		break;
	case PRM_PRC_INFO_1:
		file_offset=PRM_PRC_INFO_1;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_1", param_name_len);
		break;
	case PRM_PRC_INFO_2:
		file_offset=PRM_PRC_INFO_2;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_2", param_name_len);
		break;
	case PRM_PRC_INFO_3:
		file_offset=PRM_PRC_INFO_3;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_3", param_name_len);
		break;
	case PRM_PRC_INFO_4:
		file_offset=PRM_PRC_INFO_4;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_4", param_name_len);
		break;
	case PRM_PRC_INFO_5:
		file_offset=PRM_PRC_INFO_5;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_5", param_name_len);
		break;
	case PRM_PRC_INFO_6:
		file_offset=PRM_PRC_INFO_6;	param_name_len=14;
		memcpy(&wrbuffer[0], "PRM_PRC_INFO_6", param_name_len);
		break;
	case PRM_NOZZLE_INFO_1:
		file_offset=PRM_NOZZLE_INFO_1;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_1", param_name_len);
		break;
	case PRM_NOZZLE_INFO_2:
		file_offset=PRM_NOZZLE_INFO_2;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_2", param_name_len);
		break;
	case PRM_NOZZLE_INFO_3:
		file_offset=PRM_NOZZLE_INFO_3;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_3", param_name_len);
		break;
	case PRM_NOZZLE_INFO_4:
		file_offset=PRM_NOZZLE_INFO_4;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_4", param_name_len);
		break;
	case PRM_NOZZLE_INFO_5:
		file_offset=PRM_NOZZLE_INFO_5;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_5", param_name_len);
		break;
	case PRM_NOZZLE_INFO_6:
		file_offset=PRM_NOZZLE_INFO_6;	param_name_len=17;
		memcpy(&wrbuffer[0], "PRM_NOZZLE_INFO_6", param_name_len);
		break;
	case PRM_VOLUME_SPEAKER0:
		file_offset=PRM_VOLUME_SPEAKER0;	param_name_len=strlen("PRM_VOLUME_SPEAKER0");
		memcpy(&wrbuffer[0], "PRM_VOLUME_SPEAKER0", param_name_len);
		break;
	case PRM_VOLUME_SPEAKER1:
		file_offset=PRM_VOLUME_SPEAKER1;	param_name_len=strlen("PRM_VOLUME_SPEAKER1");
		memcpy(&wrbuffer[0], "PRM_VOLUME_SPEAKER1", param_name_len);
		break;
	case PRM_SELL_LOCK_TIME:
		file_offset=PRM_SELL_LOCK_TIME;	param_name_len=strlen("PRM_SELL_LOCK_TIME");
		memcpy(&wrbuffer[0], "PRM_SELL_LOCK_TIME", param_name_len);
		break;
	case PRM_NOZZLE_NUMBER:	/*单面枪数， 1HEX*/
		file_offset=PRM_NOZZLE_NUMBER;	param_name_len=strlen("PRM_NOZZLE_NUMBER");
		memcpy(&wrbuffer[0], "PRM_NOZZLE_NUMBER", param_name_len);
		break;
	case PRM_BARCODE_BRAND_A:	/*A面条码扫描模块品牌，1ASCII*/
		file_offset=PRM_BARCODE_BRAND_A;	param_name_len=strlen("PRM_BARCODE_BRAND_A");
		memcpy(&wrbuffer[0], "PRM_BARCODE_BRAND_A", param_name_len);
		break;
	case PRM_BARCODE_BRAND_B:	/*B面条码扫描模块品牌，1ASCII*/
		file_offset=PRM_BARCODE_BRAND_B;	param_name_len=strlen("PRM_BARCODE_BRAND_B");
		memcpy(&wrbuffer[0], "PRM_BARCODE_BRAND_B", param_name_len);
		break;
	case PRM_MODEL:					/*机型 4HEX*/
		file_offset=PRM_MODEL;	param_name_len=strlen("PRM_MODEL");
		memcpy(&wrbuffer[0], "PRM_MODEL", param_name_len);
		break;
	case PRM_PROMOTION:			/*是否启用促销功能 4HEX*/
		file_offset=PRM_PROMOTION;	param_name_len=strlen("PRM_PROMOTION");
		memcpy(&wrbuffer[0], "PRM_PROMOTION", param_name_len);
		break;
	case PRM_SINOPEC_CONNECT:			/*卡机联动后台连接方式ASCII '0'=电流环串口；'1'=RJ45网口；*/
		file_offset=PRM_SINOPEC_CONNECT;	param_name_len=strlen("PRM_SINOPEC_CONNECT");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_CONNECT", param_name_len);
		break;
	case PRM_SINOPEC_ADDRESS:			/*卡机联动后台服务器地址，IP地址(4HEX) + 端口号(2HEX)*/
		file_offset=PRM_SINOPEC_ADDRESS;	param_name_len=strlen("PRM_SINOPEC_ADDRESS");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_ADDRESS", param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_PORT:	/*卡机联动后台通讯本地服务器端口号2HEX*/
		file_offset=PRM_SINOPEC_LOCAL_PORT;	param_name_len=strlen("PRM_SINOPEC_LOCAL_PORT");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_LOCAL_PORT", param_name_len);
		break;
	case PRM_YuLe_Grade_OK:	/*卡机联动娱乐机油品确认功能是否启用1HEX*/
		file_offset=PRM_YuLe_Grade_OK;	param_name_len=strlen("PRM_YuLe_Grade_OK");
		memcpy(&wrbuffer[0], "PRM_YuLe_Grade_OK", param_name_len);
		break;
	case PRM_OilLimit_Style_Set:	/*2017-02-13油品限制方式设置1HEX*/
		file_offset=PRM_OilLimit_Style_Set;	param_name_len=strlen("PRM_OilLimit_Style_Set");
		memcpy(&wrbuffer[0], "PRM_OilLimit_Style_Set", param_name_len);
		break;
	case PRM_ETC_FUN_SET:	//szb_fj_20171120,add,ETC功能设置
		file_offset=PRM_ETC_FUN_SET;	param_name_len=strlen("PRM_ETC_FUN_SET");
		memcpy(&wrbuffer[0], "PRM_ETC_FUN_SET", param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_IP:  //fj:20171214
		file_offset = PRM_SINOPEC_LOCAL_IP; 
		param_name_len = strlen("PRM_SINOPEC_LOCAL_IP");
		memcpy(&wrbuffer[0],"PRM_SINOPEC_LOCAL_IP",param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_MASK:
		file_offset = PRM_SINOPEC_LOCAL_MASK; 
		param_name_len = strlen("PRM_SINOPEC_LOCAL_MASK");
		memcpy(&wrbuffer[0],"PRM_SINOPEC_LOCAL_MASK",param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_GATEWAY:
		file_offset = PRM_SINOPEC_LOCAL_GATEWAY; 
		param_name_len = strlen("PRM_SINOPEC_LOCAL_GATEWAY");
		memcpy(&wrbuffer[0],"PRM_SINOPEC_LOCAL_GATEWAY",param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_MAC:
        file_offset = PRM_SINOPEC_LOCAL_MAC; 
		param_name_len = strlen("PRM_SINOPEC_LOCAL_MAC");
		memcpy(&wrbuffer[0],"PRM_SINOPEC_LOCAL_MAC",param_name_len);
		break;
	default:
		return ERROR;
		break;
	}
	wrbuffer[31]='=';
	memcpy(&wrbuffer[32], buffer, nbytes);
	memcpy(&wrbuffer[62], "\r\n", 2);

/*
	//获取信号量
	semTake(semIdSetup, WAIT_FOREVER);
	//存入配置文件
	wrbytes=fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, wrbuffer, 64);
	//读出数据
	rdbytes=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, rdbuffer, 64);
	//释放信号量
	semGive(semIdSetup);*/
	
	//pthread_rwlock_wrlock(&rwlock_Setup); //fj:20170905

	wrbytes=fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, wrbuffer, 64); //存入配置文件
	rdbytes=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, rdbuffer, 64); //读出数据

	//pthread_rwlock_unlock(&rwlock_Setup);

/*	printf("wrbytes = %d,rdbytes = %d\n",wrbytes,rdbytes);

	if(wrbytes > 0)
	{
		//PrintH(wrbytes,wrbuffer);
		printf("wrbuffer =%s\n",wrbuffer);

		if(rdbytes > 0)
		{
			//PrintH(rdbytes,rdbuffer);
			printf("rdbuffer = %s\n",rdbuffer);
		}
	}*/
	
	//return 0;

	/*校验并返回结果*/

	
	if(0==memcmp(wrbuffer, rdbuffer, 64))	
		return 0;
	else	
		return ERROR;
}


/********************************************************************
*Name				:paramSetupRead
*Description		:获取配置文件设置值，有临界保护
*Input				:offset				配置项目
*						:buffer				配置项目内容
*						:maxbytes		配置项目值最大获取长度
*Output			:param				配置项目值
*Return				:成功返回0；错误返回ERROR
*History			:2014-02-24,modified by syj
*/
int paramSetupRead(off_t offset, unsigned char *buffer, int maxbytes)
{
	int readlen=0;

	/*//获取信号量
	semTake(semIdSetup, WAIT_FOREVER);

	//读取参数
	readlen=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, offset+32, buffer, maxbytes);

	//释放信号量
	semGive(semIdSetup);*/
	
    pthread_rwlock_wrlock(&rwlock_Setup);//fj:20170905
	readlen=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, offset+32, buffer, maxbytes);
	pthread_rwlock_unlock(&rwlock_Setup);

	if(maxbytes==readlen)	{return 0;}
	else									{return ERROR;}
}


/********************************************************************
*Name				:paramModelSet
*Description		:配置机型，为了防止更改后处理错误，此处并不实时更新内存中的数据
*Input				:model	机型	MODEL_SINOPEC、MODEL_UNSELF ......
*Output			:无
*Return				:0=成功；其它=失败
*History			:2016-03-28,modified by syj
*/
int paramModelSet(int model) //fj:
{
	char wrbuffer[8] = {0};

	wrbuffer[0] = (char)(model>>24);	wrbuffer[1] = (char)(model>>16);
	wrbuffer[2] = (char)(model>>8);		wrbuffer[3] = (char)(model>>0);
	if(0 != paramSetupWrite(PRM_MODEL, wrbuffer, 4))
	{
		return ERROR;
	}

	return 0;
}


/********************************************************************
*Name				:paramModelGet
*Description		:配置机型
*Input				:无
*Output			:无
*Return				:机型	MODEL_SINOPEC ......
*History			:2016-03-28,modified by syj
*/
int paramModelGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*已获取了配置信息直接返回*/
	if(ERROR != ParamModel)
	{
		return ParamModel;
	}

	/*取配置信息*/
	if(0 == paramSetupRead(PRM_MODEL, rdbuffer, 4))
	{
		ParamModel = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*取到的配置信息非法则默认为石化卡机联动加油机*/
	if(ERROR == ParamModel)
	{
		ParamModel = MODEL_SINOPEC;
	}

	return ParamModel;
}


/********************************************************************
*Name				:paramPanelNozzleSet
*Description		:panel_nozzle		配置单面枪数 0=单面单枪(PANEL_NOZZLE_SINGLE)；1=单面双枪(PANEL_NOZZLE_DOUBLE)；
*Input				:无
*Output			:无
*Return				:0=成功；其它=失败
*History			:2016-03-28,modified by syj
*/
int paramPanelNozzleSet(int panel_nozzle) //fj:
{
	char wrbuffer[8] = {0};

	wrbuffer[0] = (char)(panel_nozzle>>24);	wrbuffer[1] = (char)(panel_nozzle>>16);
	wrbuffer[2] = (char)(panel_nozzle>>8);		wrbuffer[3] = (char)(panel_nozzle>>0);
	if(0 != paramSetupWrite(PRM_NOZZLE_NUMBER, wrbuffer, 4))
	{
		return ERROR;
	}

	ParamPanelNozzle = panel_nozzle;

	return 0;
}


/********************************************************************
*Name				:paramPanelNozzleGet
*Description		:读取单面枪数 
*Input				:无
*Output			:无
*Return				:单面枪数 0=单面单枪(PANEL_NOZZLE_SINGLE)；1=单面双枪(PANEL_NOZZLE_DOUBLE)；
*History			:2016-03-28,modified by syj
*/
int paramPanelNozzleGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*已获取了配置信息直接返回*/
	if(ERROR != ParamPanelNozzle)
	{
		return ParamPanelNozzle;
	}

	/*取配置信息*/
	if(0 == paramSetupRead(PRM_NOZZLE_NUMBER, rdbuffer, 4))
	{
		ParamPanelNozzle = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*取到的配置信息非法则默认为单面单枪*/
	if(ERROR == ParamPanelNozzle)
	{
		ParamPanelNozzle = 0;
	}

	return ParamPanelNozzle;
}


/********************************************************************
*Name				:paramPromotionGet
*Description		:获取是否支持促销功能
*Input				:promoton	0 = 不启用促销功能；1 = 启用促销功能；
*Output			:无
*Return			:0 = 成功；其它 = 失败；
*History			:2016-06-14,modified by syj
*/
int paramPromotionSet(int promoton) //fj:
{
	char wrbuffer[8] = {0};

	wrbuffer[0] = (char)(promoton>>24);	wrbuffer[1] = (char)(promoton>>16);
	wrbuffer[2] = (char)(promoton>>8);	wrbuffer[3] = (char)(promoton>>0);
	if(0 != paramSetupWrite(PRM_PROMOTION, wrbuffer, 4))
	{
		return ERROR;
	}

	return 0;
}


/********************************************************************
*Name				:paramPromotionGet
*Description		:获取是否支持促销功能
*Input				:无
*Output			:无
*Return			:是否启用促销功能 0=无促销；1=有促销
*History			:2016-03-28,modified by syj
*/
int paramPromotionGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*已获取了配置信息直接返回*/
	if(ERROR != ParamPromotion)
	{
		return ParamPromotion;
	}

	/*取配置信息*/
	if(0 == paramSetupRead(PRM_PROMOTION, rdbuffer, 4))
	{
		ParamPromotion = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*取到的配置信息非法则默认为 不启用*/
	if(ERROR == ParamPromotion)
	{
		ParamPromotion = 0;
	}

	return ParamPromotion;

#if 0
	int data = 0;

	/*中石化促销机、社会站促销机、联达系统油机启用促销功能*/
	data = paramModelGet();
	if(MODEL_SINOPEC_PROMOTION == data ||\
		MODEL_PRIVATE_PROMOTION == data ||\
		MODEL_LIANDA == data)
	{
		return 1;
	}

	return 0;
#endif
}


/********************************************************************
*Name				:paramBSPVersionGet
*Description		:获取BSP版本号
*Input				:无
*Output			:outbuffer	版本号字符串
*						:maxbytes	缓存长度
*Return			:0 = 成功；其它 = 失败；
*History			:2016-03-28,modified by syj
*/
int paramBSPVersionGet(char *outbuffer, int maxbytes) //fj:先注视掉获取版本的
{
	char ibuffer[64] = {0};
	int ilength = 0;

	//strcpy(ibuffer, BSP_VERSION);
	//strcpy(ibuffer + strlen(ibuffer), BSP_REV);
	ilength = strlen(ibuffer);

	printf("\n The BSP version: %x \n", ibuffer);

	if(maxbytes >= ilength)	strncpy(outbuffer, ibuffer, ilength);
	else										strncpy(outbuffer, ibuffer, maxbytes);

	return 0;
}


/********************************************************************
*Name				:paramSetupClr
*Description		:配置文件清空
*Input				:None
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-02-24,modified by syj
*/
int paramSetupClr()
{
	int istate=0;

  /*
	//获取信号量
	semTake(semIdSetup, WAIT_FOREVER);

	//清空配置信息文件
	if(0==fileTruncateForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4))
	{
		fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	}
	else
	{
		istate=1;
	}
	
	//释放信号量
	semGive(semIdSetup);*/
	
  //fj:20190905
	pthread_rwlock_wrlock(&rwlock_Setup);

	//清空配置信息文件
	if(0==fileTruncateForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4))
	{
		fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	}
	else
	{
		istate=1;
	}
		
	pthread_rwlock_unlock(&rwlock_Setup);
	

	return istate;
}


/********************************************************************
*Name				:paramSetupInit
*Description		:配置信息获取
*Input				:None
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-02-24,modified by syj
*/
bool  paramSetupInit()
{
	char rdbuffer[32] = {0};
	int istate = 0;

/*
	//配置文件操作互斥信号量
	semIdSetup=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==semIdSetup)
	{
		printf("Error!Create semaphore 'semIdSetup' failed!\n");
	}

	//异常记录文件操作互斥信号量
	SemIdErrInfo=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==SemIdErrInfo)
	{
		printf("Error!Create semaphore 'SemIdErrInfo' failed!\n");
	}

	//日志文件操作互斥信号量
	SemIdRecord=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==SemIdRecord)
	{
		printf("Error!Create semaphore 'SemIdRecord' failed!\n");
	}*/
	
	int nSetupRet = -1;
	nSetupRet = pthread_rwlock_init(&rwlock_Setup,NULL);
	if(nSetupRet != 0)
	{
		printf("Error!Create semaphore 'semIdSetup' failed!\n");
		return false;
	}
	
	int nErrorInfoRet = -1;
	nErrorInfoRet = pthread_rwlock_init(&rwLock_ErrInfo,NULL);
	if(nErrorInfoRet != 0)
	{
		printf("Error!Create semaphore 'SemIdErrInfo' failed!\n");
		return false;
	}
	
	int nRecordRet = -1;
	nRecordRet = pthread_rwlock_init(&rwlock_Record,NULL);
	if(nRecordRet != 0)
	{
		printf("Error!Create semaphore 'SemIdErrInfo' failed!\n");
		return false;
	}

	unsigned char buffer[2] = {0x01,0x02};
	int nLen = 2;
    int nRet = paramSetupWrite(JL0_EQUIVALENT,buffer,nLen);
	if(nRet < 0)
	{
		return false;
	}
	
	return true;
}










