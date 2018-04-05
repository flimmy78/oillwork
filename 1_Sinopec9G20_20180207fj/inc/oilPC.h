#ifndef _OIL_PC_H_
#define _OIL_PC_H_



//�ͻ���Ϣ
typedef struct
{
	unsigned CardState:1;		//��״̬ 0 �޿�  1�п�
	unsigned OilState:1;		//����״̬ 1  ������
	unsigned OilEndState:1;		//���ͽ���״̬  1  ����

	//�п�ʱ
	char CardID[10];						//���� BCD
	char CardBalance[4];				//����� HEX
	char Shopping;							//���������־ 0x01 ����
	
	//����ʱ
	char OilMoney[4];					//���ͽ�� HEX
	char OilVolume[4];					//�������� HEX
	char OilPrice[3];						//���ͼ۸� HEX
	
	//���ͽ���ʱ
	char EndMoney[4];					//���ͽ�� HEX
	char EndVolume[4];					//�������� HEX
	char EndPrice[3];						//���ͼ۸� HEX
	
}PCOilInfoType;

//����Ϣ
typedef struct
{
	unsigned char IcValid;							   //�Ƿ�����Ч����Ϣ 0=�ޣ�1=��

	unsigned char IcAppSelect;							//Ӧ��ѡ�� 0=������Ʊ��1=����Ӧ��
	unsigned char IcAppId[10];							//Ӧ�����к�
	unsigned char IcEnableTime[4];					//Ӧ����������
	unsigned char IcInvalidTime[4];					//Ӧ����Ч��ֹ����
	unsigned char IcUserName[20];					//�ֿ�������
	unsigned char IcUserIdeId[18];					//�ֿ���֤��(identity)����(ASCII)
	unsigned char IcUserIdeType;						//�ֿ���֤������
	unsigned char IcOilLimit[2];							//��Ʒ����
	unsigned char IcRegionTypeLimit;				//�޵���,��վ���ͷ�ʽ
	unsigned char IcRegionLimit[40];					//�޵���,��վ����
	unsigned char IcVolumeLimit[2];					//��ÿ�μ�����
	unsigned char IcTimesLimit;						//��ÿ����ʹ���
	unsigned char IcMoneyDayLimit[4];			//��ÿ����ͽ��
	unsigned char IcCarIdLimit[16];					//���ƺ�����(ASCII)

}PCCardInfoType;

//��������ϸ
typedef struct
{
	unsigned char TTC[2];						//ET�������ѻ��������
	unsigned char Limit[3];						//͸֧�޶�
	unsigned char Money[4];					//���׽��
	unsigned char Type;							//�������ͱ�ʶ
	unsigned char TermID[6];				//�ն˻����
	unsigned char Time[7];						//����ʱ��

}PCCardRecordType;

//����ƽ����ԵĴ���
#define PC_COM_1					COM9		//1�����ƽ��������ӵĴ���
#define PC_COM_2					COM10		//2�����ƽ��������ӵĴ���


//ƽ���������
#define PC_PANEL_1				0				//1�����ƽ�����ID
#define PC_PANEL_2				1				//2�����ƽ�����ID

//�Ƿ�������
#define PC_SHOPPING_ALLOW				1			//0=������1=����

//ƽ�����������ͨѶ����
#define PC_CMD_ACTION_UPLOAD		0x54		//(�ͻ�->ƽ��)֪ͨƽ����Լ��ͻ��Ĳ�������
#define PC_CMD_STATE_UPLOAD			0x55		//(�ͻ�->ƽ��)֪ͨƽ����Լ��ͻ���״̬
#define PC_CMD_VOICE_PLAY				0x56		//(�ͻ�->ƽ��)��������
#define PC_CMD_OILINFO						0x57		//(�ͻ�->ƽ��)���ͻ���Ϣ����
#define PC_CMD_KEYUPLOAD					0x58		//(�ͻ�->ƽ��)�����Ľ�����ֵ
#define PC_CMD_SHAKEHANDS				0x59		//(ƽ��->�ͻ�)ƽ���������
#define PC_CMD_PCPARAM						0x5a		//(�ͻ�->ƽ��)ƽ�������Ϣ��ѯ����
#define PC_CMD_LIGHT_CONTROL		0x5b		//(ƽ��->�ͻ�)ƽ�������������-��������ƽ������������͵���������
#define PC_CMD_LET_PC_DONE				0x5c		//(�ͻ�->ƽ��)���ͻ�����ƽ������齱
#define PC_CMD_DATA_REUP					0x5d		//(ƽ��->�ͻ�)ƽ�����ת����̨���ݸ��ͻ�
#define PC_CMD_EACH_OTHER				0x5e		//(�ͻ�->ƽ��)���ͻ���ƽ��֧������
#define PC_CMD_GET_VOLUME				0x5f		//(�ͻ�->ƽ��)���ͻ���ȡƽ���������
#define PC_CMD_GET_CARDINFO			0x60		//(ƽ��->�ͻ�)ƽ����Զ�ȡ�ͻ���Ƭ��Ϣ�������׼�¼
#define PC_CMD_OIL_ACK						0x61		//(�ͻ�->ƽ��)��Ʒȷ�Ͻ���
#define PC_CMD_BILL_UPLOAD				0x63		//(�ͻ�->ƽ��)�ͻ��ϴ�������ϸ
#define PC_CMD_PASS_UPLOAD				0x64		//(�ͻ�->ƽ��)�ͻ��ϴ�����
#define PC_CMD_DEBIT_RESULT			0x65		//(�ͻ�->ƽ��)�ͻ��ϴ����ѽ��

//�ͻ�������������
#define PC_ACTION_SENSOR					0x00		//�����Ӧ
#define PC_ACTION_PLAY						0x01		//�Ľ���ť
#define PC_ACTION_YP							0x02		//��Ʒѡ��
#define PC_ACTION_BUTTON					0x03		//����
#define PC_ACTION_GUN							0x04		//���ǹ
#define PC_ACTION_PLAY_3SECOND		0x05		//3�볤��PLAY��ť
#define PC_ACTION_PLAY_4SECOND		0x06		//4�볤��PLAY��ť
#define PC_ACTION_PLAY_5SECOND		0x07		//5�볤��PLAY��ť

#define PC_FUN_GRADE_OK				0x30		//������Ʒȷ��
#define PC_FUN_GRADE_NO				0x31		//������Ʒȷ��


//��������
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



