#ifndef _OIL_AUTHORIZE_H_
#define _OIL_AUTHORIZE_H_

//��Ȩ��ʽ
#define AUTH_MODEL_ETC				0x00	//ETC
#define AUTH_MODEL_WECHAT		0x01	//΢��
#define AUTH_MODEL_ALIPAY		0x02	//֧����

//�ۿ���Դ
#define AUTH_DS_ETC						0x91	//ETC
#define AUTH_DS_WECHAT				0x92	//΢��
#define AUTH_DS_ALIPAY				0x93	//֧����


//��Ȩ�������
typedef struct
{
	unsigned char Model;								//��Ȩ��ʽ00H=ETC; 01H=΢��; 02H=֧����
	unsigned char Unit;								//��Ȩ��λ 0=Ԫ��1=��
	unsigned int Amount;								//��Ȩ�� HEX
	unsigned char AntID;								//ETC��Ȩ������ID AntID
	unsigned char OBUID[4];						//ETC��Ȩ����ǩMAC�� OBUID
	unsigned char ContractNo[8];				//ETC��Ȩ��OBU��ͬ���к� ContractNo
	unsigned char OBUPlate[12];					//ETC��Ȩ�����ƺ� OBUPlate
	unsigned char CardID[10];						//ETC��Ȩ��ETC����

}AuthorizeDataType;




//�ӿ�
extern int authorizeWrite(int nozzle, const char *inbuffer);
extern void authProccess(int nozzle, char *inbuffer, int nbytes);

#endif

