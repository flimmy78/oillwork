//#include "oilCfg.h"
//#include "config.h"
//#include "oilStmTransmit.h"
//#include "oilParam.h"

#include "../inc/main.h"


//
const char *ModelParam[] = 
{
	"ʯ�������������ͻ�",
	"���վ���ͻ�",
	"����ϵͳ���ͻ�",
};

/*
//�����ļ�������
static int fdSetup=ERROR;

//�����ļ����������ź���
static SEM_ID semIdSetup=NULL;

//�쳣��¼�ļ�������
static int FdErrInfo=ERROR;

//�쳣��¼�ļ��ź���
static SEM_ID SemIdErrInfo=NULL;

//��־�ļ�������
static int FdRecord=ERROR;

//��־�ļ��ź���
static SEM_ID SemIdRecord=NULL;*/

//fj:20170905
static int fdSetup=ERROR; //�����ļ�������
pthread_rwlock_t rwlock_Setup;//�����ļ����������ź���
static int FdErrInfo=ERROR; //�쳣��¼�ļ�������
pthread_rwlock_t rwLock_ErrInfo; //�쳣��¼�ļ��ź���
static int FdRecord=ERROR; //��־�ļ�������
pthread_rwlock_t rwlock_Record; //��־�ļ��ź���


/*�������*/
static int ParamModel = ERROR;

/*����ǹ�� PANEL_NOZZLE_SINGLE / PANEL_NOZZLE_DOUBLE*/
static int ParamPanelNozzle = ERROR;

/*�Ƿ����ô������� 0 = �����ã�1 = ���ã�*/
static int ParamPromotion = ERROR;


/********************************************************************
*Name				:paramSetupWrite
*Description		:�޸������ļ�ѡ������ݵ�У��
*Input				:offset		��������
*						:buffer		��������
*						:nbytes	�������ȣ�������30
*Output			:None
*Return				:�ɹ�����0�����󷵻�����
*History			:2014-02-24,modified by syj
*/
int paramSetupWrite(off_t offset, const unsigned char *buffer, int nbytes)
{
	unsigned char wrbuffer[64]={0}, rdbuffer[64]={0};
	int istate=0, wrbytes=0, rdbytes=0;
	unsigned int file_offset=0, param_name_len=0;

	/*�жϲ�������*/
	if(nbytes>30)
	{
		return ERROR;
	}

	/*�����������������ʽΪ:"������"(���30�ֽ�)	+	"="	+	"��������"(���30�ֽ�)	+	"\r\n"*/
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
	case JL0_ALGORITHM:		//(A1ǹ)����������ʱ�䳬����ֵʱ�رմ�,4HEX
		file_offset=JL0_ALGORITHM;	param_name_len=strlen("JL0_ALGORITHM");
		memcpy(&wrbuffer[0], "JL0_ALGORITHM", param_name_len);
		break;
    case JL0_BIG_VOL_TIME://szb_fj_20171120,add,(A1ǹ)˫�����������������峬ʱʱ�����1BCD ��5-20Ĭ��8
		file_offset=JL0_BIG_VOL_TIME;	param_name_len=strlen("JL0_BIG_VOL_TIME");
		memcpy(&wrbuffer[0], "JL0_BIG_VOL_TIME", param_name_len);
		break;
	case JL0_BIG_VOL_SPEED://szb_fj_20171120,add,(A1ǹ)˫���������������ٿ���1BCD L/��10-80Ĭ��15
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
	case JL1_ALGORITHM:		//(B1ǹ)����������ʱ�䳬����ֵʱ�رմ�,4HEX
		file_offset=JL1_ALGORITHM;	param_name_len=strlen("JL1_ALGORITHM");
		memcpy(&wrbuffer[0], "JL1_ALGORITHM", param_name_len);
		break;
	case JL1_BIG_VOL_TIME:  //szb_fj_20171120,add,(B1ǹ)˫�����������������峬ʱʱ�����1BCD ��5-20Ĭ��8
		file_offset=JL1_BIG_VOL_TIME;	param_name_len=strlen("JL1_BIG_VOL_TIME");
		memcpy(&wrbuffer[0], "JL1_BIG_VOL_TIME", param_name_len);
		break;
	case JL1_BIG_VOL_SPEED:  //szb_fj_20171120,add,(B1ǹ)˫���������������ٿ���1BCD L/��10-80Ĭ��15
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
	case IPT0_BIND_TIME:					/*����Ϣ��ʱ�䣬7BCD*/
		file_offset=IPT0_BIND_TIME;	param_name_len=strlen("IPT0_BIND_TIME");
		memcpy(&wrbuffer[0], "IPT0_BIND_TIME", param_name_len);
		break;
	case IPT0_BIND_MBOARD_ID:		/*����Ϣ������ţ�8BCD*/
		file_offset=IPT0_BIND_MBOARD_ID;	param_name_len=strlen("IPT0_BIND_MBOARD_ID");
		memcpy(&wrbuffer[0], "IPT0_BIND_MBOARD_ID", param_name_len);
		break;
	case IPT0_BIND_ACT_APPID:		/*����Ϣ��ACT���ţ�10BCD*/
		file_offset=IPT0_BIND_ACT_APPID;	param_name_len=strlen("IPT0_BIND_ACT_APPID");
		memcpy(&wrbuffer[0], "IPT0_BIND_ACT_APPID", param_name_len);
		break;
	case IPT0_BIND_RID_APPID:			/*����Ϣ��RID���ţ�10BCD*/
		file_offset=IPT0_BIND_RID_APPID;	param_name_len=strlen("IPT0_BIND_RID_APPID");
		memcpy(&wrbuffer[0], "IPT0_BIND_RID_APPID", param_name_len);
		break;
	case IPT0_OIL_VOICE:					/*��Ʒ����������Ϣ���������ļ�������ǰ׺��4ASCII*/
		file_offset=IPT0_OIL_VOICE;	param_name_len=strlen("IPT0_OIL_VOICE");
		memcpy(&wrbuffer[0], "IPT0_OIL_VOICE", param_name_len);
		break;
	case IPT0_CONTRAST:					/*(A1ǹ)������ʾ�Աȶȣ�1HEX*/
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
	case IPT1_BIND_TIME:					/*����Ϣ��ʱ�䣬7BCD*/
		file_offset=IPT1_BIND_TIME;	param_name_len=strlen("IPT1_BIND_TIME");
		memcpy(&wrbuffer[0], "IPT1_BIND_TIME", param_name_len);
		break;
	case IPT1_BIND_MBOARD_ID:		/*����Ϣ������ţ�8BCD*/
		file_offset=IPT1_BIND_MBOARD_ID;	param_name_len=strlen("IPT1_BIND_MBOARD_ID");
		memcpy(&wrbuffer[0], "IPT1_BIND_MBOARD_ID", param_name_len);
		break;
	case IPT1_BIND_ACT_APPID:		/*����Ϣ��ACT���ţ�10BCD*/
		file_offset=IPT1_BIND_ACT_APPID;	param_name_len=strlen("IPT1_BIND_ACT_APPID");
		memcpy(&wrbuffer[0], "IPT1_BIND_ACT_APPID", param_name_len);
		break;
	case IPT1_BIND_RID_APPID:			/*����Ϣ��RID���ţ�10BCD*/
		file_offset=IPT1_BIND_RID_APPID;	param_name_len=strlen("IPT1_BIND_RID_APPID");
		memcpy(&wrbuffer[0], "IPT1_BIND_RID_APPID", param_name_len);
		break;
	case IPT1_OIL_VOICE:					/*��Ʒ����������Ϣ���������ļ�������ǰ׺��4ASCII*/
		file_offset=IPT1_OIL_VOICE;	param_name_len=strlen("IPT1_OIL_VOICE");
		memcpy(&wrbuffer[0], "IPT1_OIL_VOICE", param_name_len);
		break;
	case IPT1_CONTRAST:					/*(A1ǹ)������ʾ�Աȶȣ�1HEX*/
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
	case PRM_NOZZLE_NUMBER:	/*����ǹ���� 1HEX*/
		file_offset=PRM_NOZZLE_NUMBER;	param_name_len=strlen("PRM_NOZZLE_NUMBER");
		memcpy(&wrbuffer[0], "PRM_NOZZLE_NUMBER", param_name_len);
		break;
	case PRM_BARCODE_BRAND_A:	/*A������ɨ��ģ��Ʒ�ƣ�1ASCII*/
		file_offset=PRM_BARCODE_BRAND_A;	param_name_len=strlen("PRM_BARCODE_BRAND_A");
		memcpy(&wrbuffer[0], "PRM_BARCODE_BRAND_A", param_name_len);
		break;
	case PRM_BARCODE_BRAND_B:	/*B������ɨ��ģ��Ʒ�ƣ�1ASCII*/
		file_offset=PRM_BARCODE_BRAND_B;	param_name_len=strlen("PRM_BARCODE_BRAND_B");
		memcpy(&wrbuffer[0], "PRM_BARCODE_BRAND_B", param_name_len);
		break;
	case PRM_MODEL:					/*���� 4HEX*/
		file_offset=PRM_MODEL;	param_name_len=strlen("PRM_MODEL");
		memcpy(&wrbuffer[0], "PRM_MODEL", param_name_len);
		break;
	case PRM_PROMOTION:			/*�Ƿ����ô������� 4HEX*/
		file_offset=PRM_PROMOTION;	param_name_len=strlen("PRM_PROMOTION");
		memcpy(&wrbuffer[0], "PRM_PROMOTION", param_name_len);
		break;
	case PRM_SINOPEC_CONNECT:			/*����������̨���ӷ�ʽASCII '0'=���������ڣ�'1'=RJ45���ڣ�*/
		file_offset=PRM_SINOPEC_CONNECT;	param_name_len=strlen("PRM_SINOPEC_CONNECT");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_CONNECT", param_name_len);
		break;
	case PRM_SINOPEC_ADDRESS:			/*����������̨��������ַ��IP��ַ(4HEX) + �˿ں�(2HEX)*/
		file_offset=PRM_SINOPEC_ADDRESS;	param_name_len=strlen("PRM_SINOPEC_ADDRESS");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_ADDRESS", param_name_len);
		break;
	case PRM_SINOPEC_LOCAL_PORT:	/*����������̨ͨѶ���ط������˿ں�2HEX*/
		file_offset=PRM_SINOPEC_LOCAL_PORT;	param_name_len=strlen("PRM_SINOPEC_LOCAL_PORT");
		memcpy(&wrbuffer[0], "PRM_SINOPEC_LOCAL_PORT", param_name_len);
		break;
	case PRM_YuLe_Grade_OK:	/*�����������ֻ���Ʒȷ�Ϲ����Ƿ�����1HEX*/
		file_offset=PRM_YuLe_Grade_OK;	param_name_len=strlen("PRM_YuLe_Grade_OK");
		memcpy(&wrbuffer[0], "PRM_YuLe_Grade_OK", param_name_len);
		break;
	case PRM_OilLimit_Style_Set:	/*2017-02-13��Ʒ���Ʒ�ʽ����1HEX*/
		file_offset=PRM_OilLimit_Style_Set;	param_name_len=strlen("PRM_OilLimit_Style_Set");
		memcpy(&wrbuffer[0], "PRM_OilLimit_Style_Set", param_name_len);
		break;
	case PRM_ETC_FUN_SET:	//szb_fj_20171120,add,ETC��������
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
	//��ȡ�ź���
	semTake(semIdSetup, WAIT_FOREVER);
	//���������ļ�
	wrbytes=fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, wrbuffer, 64);
	//��������
	rdbytes=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, rdbuffer, 64);
	//�ͷ��ź���
	semGive(semIdSetup);*/
	
	//pthread_rwlock_wrlock(&rwlock_Setup); //fj:20170905

	wrbytes=fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, wrbuffer, 64); //���������ļ�
	rdbytes=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, file_offset, rdbuffer, 64); //��������

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

	/*У�鲢���ؽ��*/

	
	if(0==memcmp(wrbuffer, rdbuffer, 64))	
		return 0;
	else	
		return ERROR;
}


/********************************************************************
*Name				:paramSetupRead
*Description		:��ȡ�����ļ�����ֵ�����ٽ籣��
*Input				:offset				������Ŀ
*						:buffer				������Ŀ����
*						:maxbytes		������Ŀֵ����ȡ����
*Output			:param				������Ŀֵ
*Return				:�ɹ�����0�����󷵻�ERROR
*History			:2014-02-24,modified by syj
*/
int paramSetupRead(off_t offset, unsigned char *buffer, int maxbytes)
{
	int readlen=0;

	/*//��ȡ�ź���
	semTake(semIdSetup, WAIT_FOREVER);

	//��ȡ����
	readlen=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, offset+32, buffer, maxbytes);

	//�ͷ��ź���
	semGive(semIdSetup);*/
	
    pthread_rwlock_wrlock(&rwlock_Setup);//fj:20170905
	readlen=fileReadForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, offset+32, buffer, maxbytes);
	pthread_rwlock_unlock(&rwlock_Setup);

	if(maxbytes==readlen)	{return 0;}
	else									{return ERROR;}
}


/********************************************************************
*Name				:paramModelSet
*Description		:���û��ͣ�Ϊ�˷�ֹ���ĺ�����󣬴˴�����ʵʱ�����ڴ��е�����
*Input				:model	����	MODEL_SINOPEC��MODEL_UNSELF ......
*Output			:��
*Return				:0=�ɹ�������=ʧ��
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
*Description		:���û���
*Input				:��
*Output			:��
*Return				:����	MODEL_SINOPEC ......
*History			:2016-03-28,modified by syj
*/
int paramModelGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*�ѻ�ȡ��������Ϣֱ�ӷ���*/
	if(ERROR != ParamModel)
	{
		return ParamModel;
	}

	/*ȡ������Ϣ*/
	if(0 == paramSetupRead(PRM_MODEL, rdbuffer, 4))
	{
		ParamModel = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*ȡ����������Ϣ�Ƿ���Ĭ��Ϊʯ�������������ͻ�*/
	if(ERROR == ParamModel)
	{
		ParamModel = MODEL_SINOPEC;
	}

	return ParamModel;
}


/********************************************************************
*Name				:paramPanelNozzleSet
*Description		:panel_nozzle		���õ���ǹ�� 0=���浥ǹ(PANEL_NOZZLE_SINGLE)��1=����˫ǹ(PANEL_NOZZLE_DOUBLE)��
*Input				:��
*Output			:��
*Return				:0=�ɹ�������=ʧ��
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
*Description		:��ȡ����ǹ�� 
*Input				:��
*Output			:��
*Return				:����ǹ�� 0=���浥ǹ(PANEL_NOZZLE_SINGLE)��1=����˫ǹ(PANEL_NOZZLE_DOUBLE)��
*History			:2016-03-28,modified by syj
*/
int paramPanelNozzleGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*�ѻ�ȡ��������Ϣֱ�ӷ���*/
	if(ERROR != ParamPanelNozzle)
	{
		return ParamPanelNozzle;
	}

	/*ȡ������Ϣ*/
	if(0 == paramSetupRead(PRM_NOZZLE_NUMBER, rdbuffer, 4))
	{
		ParamPanelNozzle = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*ȡ����������Ϣ�Ƿ���Ĭ��Ϊ���浥ǹ*/
	if(ERROR == ParamPanelNozzle)
	{
		ParamPanelNozzle = 0;
	}

	return ParamPanelNozzle;
}


/********************************************************************
*Name				:paramPromotionGet
*Description		:��ȡ�Ƿ�֧�ִ�������
*Input				:promoton	0 = �����ô������ܣ�1 = ���ô������ܣ�
*Output			:��
*Return			:0 = �ɹ������� = ʧ�ܣ�
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
*Description		:��ȡ�Ƿ�֧�ִ�������
*Input				:��
*Output			:��
*Return			:�Ƿ����ô������� 0=�޴�����1=�д���
*History			:2016-03-28,modified by syj
*/
int paramPromotionGet(void) //fj:
{
	char rdbuffer[8] = {0};

	/*�ѻ�ȡ��������Ϣֱ�ӷ���*/
	if(ERROR != ParamPromotion)
	{
		return ParamPromotion;
	}

	/*ȡ������Ϣ*/
	if(0 == paramSetupRead(PRM_PROMOTION, rdbuffer, 4))
	{
		ParamPromotion = (rdbuffer[0]<<24)|(rdbuffer[1]<<16)|(rdbuffer[2]<<8)|(rdbuffer[3]<<0);
	}

	/*ȡ����������Ϣ�Ƿ���Ĭ��Ϊ ������*/
	if(ERROR == ParamPromotion)
	{
		ParamPromotion = 0;
	}

	return ParamPromotion;

#if 0
	int data = 0;

	/*��ʯ�������������վ������������ϵͳ�ͻ����ô�������*/
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
*Description		:��ȡBSP�汾��
*Input				:��
*Output			:outbuffer	�汾���ַ���
*						:maxbytes	���泤��
*Return			:0 = �ɹ������� = ʧ�ܣ�
*History			:2016-03-28,modified by syj
*/
int paramBSPVersionGet(char *outbuffer, int maxbytes) //fj:��ע�ӵ���ȡ�汾��
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
*Description		:�����ļ����
*Input				:None
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-02-24,modified by syj
*/
int paramSetupClr()
{
	int istate=0;

  /*
	//��ȡ�ź���
	semTake(semIdSetup, WAIT_FOREVER);

	//���������Ϣ�ļ�
	if(0==fileTruncateForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4))
	{
		fileWriteForPath(FILE_SETUP, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	}
	else
	{
		istate=1;
	}
	
	//�ͷ��ź���
	semGive(semIdSetup);*/
	
  //fj:20190905
	pthread_rwlock_wrlock(&rwlock_Setup);

	//���������Ϣ�ļ�
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
*Description		:������Ϣ��ȡ
*Input				:None
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-02-24,modified by syj
*/
bool  paramSetupInit()
{
	char rdbuffer[32] = {0};
	int istate = 0;

/*
	//�����ļ����������ź���
	semIdSetup=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==semIdSetup)
	{
		printf("Error!Create semaphore 'semIdSetup' failed!\n");
	}

	//�쳣��¼�ļ����������ź���
	SemIdErrInfo=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==SemIdErrInfo)
	{
		printf("Error!Create semaphore 'SemIdErrInfo' failed!\n");
	}

	//��־�ļ����������ź���
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










