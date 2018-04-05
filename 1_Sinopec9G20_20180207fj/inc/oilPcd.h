#ifndef _OIL_PCD_H_
#define _OIL_PCD_H_


#include "lstLib.h"

#define PCD_MBOARD_INFO		"������V5.00 "	//������Ϣ��12ASCII
#define PCD_VERSION				"V5.00"		      //PCD�汾
#define PCD_VERSION_DSP		"0500"		      //PCD�汾���ϴ�������Ϣ�д��ֶΣ�Ҫ����PCD�汾�ĳ�ֻ�����ֵ���ʽ
#define PCD_MSGNB					20				      //PCD������Ϣ���������յ���Ϣ����
#define PCD_MSGMAX				512			//PCD������Ϣ����һ����Ϣ��󳤶�
#define PCD_PCDATA_MAX		512			//PCD���պ�̨PC���ݵ���󳤶�
#define PCD_MBOARD_MAX		3				//PCD֧�ֵ����������
#define PCD_NOZZLE_MAX			6			//PCD֧�ֵ����������ǹ��
#define PCD_FP_OFFLINE			0			//֧���ն�����״̬
#define PCD_FP_ONLINE			1				//֧���ն�����״̬
#define PCD_PC_OFFLINE			0			//PC��̨����״̬
#define PCD_PC_ONLINE			1				//PC��̨����״̬
#define PCD_ZD_LEN					128		//�˵�����
#define PCD_ZDERRINFO_SIZE	16	  //�쳣�˵�������Ϣ����
#define PCD_ZD_INDEX_SIZE	16			//������ǹ�˵�������Ϣ����
#define PCD_DOWNLOAD_SEG	30			//���ͻ�����PC����ʱÿ�����ص����ݶ���
#define PCD_RECORD_MAX		50000		//PCD���洢�˵���Ŀ
#define PCD_ZD_INDEX_MAX	5000		//��ǹ���˵������ļ����쳣�˵�����δ�������Ŀ

//PC������������
#define PCD_DOWN_BASELIST			1		//���ػ���������
#define PCD_DOWN_ADDLIST			2		//��������������
#define PCD_DOWN_DELLIST			3		//������ɾ������
#define PCD_DOWN_WHITELIST		4		//���ذ�����
#define PCD_DOWN_OILINFO			5		//������Ʒ�ͼ۱�
#define PCD_DOWN_STATIONINFO	6		//������վͨ����Ϣ

//�˵�����
#define PCD_BILL_SIZE									128			//�˵�����
#define PCD_OFFSET_TTC								(0)			//POS_TTC	4bytes
#define PCD_OFFSET_T_TYPE						(4)			  //��������1byte
#define PCD_OFFSET_TIME							(5)			  //�������ڼ�ʱ��7bytes��Ӧ��
#define PCD_OFFSET_ASN								(12)		//��Ӧ�ú�10bytes
#define PCD_OFFSET_BALANCE						(22)		//���4bytes
#define PCD_OFFSET_AMN								(26)		//����3bytes
#define PCD_OFFSET_CTC								(29)		//���������2bytes
#define PCD_OFFSET_TAC								(31)		//����ǩ��4bytes
#define PCD_OFFSET_GMAC							(35)			//�����֤��4bytes
#define PCD_OFFSET_PSAM_TAC					(39)			//PSAM����ǩ��4bytes
#define PCD_OFFSET_PSAM_ASN					(43)			//PSAMӦ�ú�10bytes
#define PCD_OFFSET_TID								(53)		//PSAM���6bytes
#define PCD_OFFSET_PSAM_TTC					(59)			//PSAM�ն˽������4bytes
#define PCD_OFFSET_DS								(63)			//�ۿ���Դ1byte
#define PCD_OFFSET_UNIT							(64)			//���㵥λ/��ʽ1byte
#define PCD_OFFSET_C_TYPE						(65)			//����1byte
#define PCD_OFFSET_VER								(66)		//���汾1byte	b7~b4:����Կ�����ţ�b3~b0:����Կ�汾��
#define PCD_OFFSET_NZN								(67)		//ǹ��1byte
#define PCD_OFFSET_G_CODE						(68)			//��Ʒ����2bytes
#define PCD_OFFSET_VOL								(70)		//����3bytes
#define PCD_OFFSET_PRC								(73)		//�ɽ��۸�2bytes
#define PCD_OFFSET_EMP								(75)		//Ա����1byte
#define PCD_OFFSET_V_TOT							(76)		//���ۼ�4bytes
#define PCD_OFFSET_RFU								(80)		//���ò���11bytes
#define PCD_OFFSET_T_MAC							(91)		//�ն�������֤��4bytes........................................
#define PCD_OFFSET_PHYGUN						(95)			//����ǹ��1byte
#define PCD_OFFSET_STOPNO						(96)			//����ͣ������1byte
#define PCD_OFFSET_BEFOR_BAL				(97)			//��ǰ���4bytes
#define PCD_OFFSET_ZD_STATE					(101)		  //�˵�״̬0=������1=δ���
#define PCD_OFFSET_JLNOZZLE					(102)		  //����ǹ��1byte
#define PCD_OFFSET_ZDXOR							(127)		//�˵����У��,1byte
#define PCD_OFFSET_ZDBACKUP					(128)		  //128~255�˵����ݣ��ܳ�128�ֽ�

//�ļ�·��
#define PCD_FILE_BASELIST				"../config/mboardFiles/PcdIBList.txt"				 //������������¼�ļ�
#define PCD_FILE_ADDLIST				"../config/mboardFiles/PcdABList.txt"				 //������������¼�ļ�
#define PCD_FILE_DELLIST				"../config/mboardFiles/PcdDBList.txt"			 //��ɾ��������¼�ļ�
#define PCD_FILE_WLIST					"../config/mboardFiles/PcdWList.txt"					 //��������¼�ļ�
#define PCD_FILE_PRICEINFO			    "../config/mboardFiles/PcdPriceInfo.txt"			 //��Ʒ�ͼ۱��¼�ļ�
#define PCD_FILE_STATIONINFO		    "../config/mboardFiles/PcdStationInfo.txt"		 //��վͨ����Ϣ��¼�ļ�
#define PCD_FILE_OILRECORD			    "../config/mboardFiles/PcdOilRecord.txt"			 //�������ݼ�¼�ļ�
#define PCD_FILE_ZD_UNNORMAL	        "../config/mboardFiles/PcdZDUnnormal.txt"			 //�쳣�˵������ļ�
#define PCD_FILE_ZDINDEX_1			    "../config/mboardFiles/PcdZDIndex1.txt"			 //����ǹ��1�˵������ļ�
#define PCD_FILE_ZDINDEX_2			    "../config/mboardFiles/PcdZDIndex2.txt"				//����ǹ��2�˵������ļ�
#define PCD_FILE_ZDINDEX_3			    "../config/mboardFiles/PcdZDIndex3.txt"				//����ǹ��3�˵������ļ�
#define PCD_FILE_ZDINDEX_4			    "../config/mboardFiles/PcdZDIndex4.txt"				//����ǹ��4�˵������ļ�
#define PCD_FILE_ZDINDEX_5			    "../config/mboardFiles/PcdZDIndex5.txt"				//����ǹ��5�˵������ļ�
#define PCD_FILE_ZDINDEX_6			    "../config/mboardFiles/PcdZDIndex6.txt"				//����ǹ��6�˵������ļ�

//�����������洢λ��
#define PCD_FM_DATALEN			93				//������Ч���ݳ��ȣ�������У��
#define PCD_FM_TTC					(0*4)		//��ǰTTC��4bytes
#define PCD_FM_UNLOAD			(1*4)		//δ�ϴ��˵�������4bytes
#define PCD_FM_TTC1					(2*4)		//��ǹ��ǰTTC��1��ǹ��4bytes
#define PCD_FM_TTC2					(3*4)		//��ǹ��ǰTTC��2��ǹ��4bytes
#define PCD_FM_TTC3					(4*4)		//��ǹ��ǰTTC��3��ǹ��4bytes
#define PCD_FM_TTC4					(5*4)		//��ǹ��ǰTTC��4��ǹ��4bytes
#define PCD_FM_TTC5					(6*4)		//��ǹ��ǰTTC��5��ǹ��4bytes
#define PCD_FM_TTC6					(7*4)		//��ǹ��ǰTTC��6��ǹ��4bytes
#define PCD_FM_ZDNUM1			(8*4)		//��ǹ�˵���Ŀ��1��ǹ��4bytes
#define PCD_FM_ZDNUM2			(9*4)		//��ǹ�˵���Ŀ��2��ǹ��4bytes
#define PCD_FM_ZDNUM3			(10*4)		//��ǹ�˵���Ŀ��3��ǹ��4bytes
#define PCD_FM_ZDNUM4			(11*4)		//��ǹ�˵���Ŀ��4��ǹ��4bytes
#define PCD_FM_ZDNUM5			(12*4)		//��ǹ�˵���Ŀ��5��ǹ��4bytes
#define PCD_FM_ZDNUM6			(13*4)		//��ǹ�˵���Ŀ��6��ǹ��4bytes
#define PCD_FM_UNLOAD1			(14*4)		//��ǹδ�ϴ��˵���Ŀ��1��ǹ��4bytes
#define PCD_FM_UNLOAD2			(15*4)		//��ǹδ�ϴ��˵���Ŀ��2��ǹ��4bytes
#define PCD_FM_UNLOAD3			(16*4)		//��ǹδ�ϴ��˵���Ŀ��3��ǹ��4bytes
#define PCD_FM_UNLOAD4			(17*4)		//��ǹδ�ϴ��˵���Ŀ��4��ǹ��4bytes
#define PCD_FM_UNLOAD5			(18*4)		//��ǹδ�ϴ��˵���Ŀ��5��ǹ��4bytes
#define PCD_FM_UNLOAD6			(19*4)		//��ǹδ�ϴ��˵���Ŀ��6��ǹ��4bytes
#define PCD_FM_YCZDNUM			(20*4)		//�쳣�˵���Ŀ��4bytes
#define PCD_FM_DLEN					(21*4)		//���������ܳ��ȣ�4bytes
#define PCD_FM_DOFFSET			(22*4)		//��������ƫ�Ƴ��ȣ�4bytes
#define PCD_FM_DCONTENT		92				//������������0xff=���������أ�1byte
#define PCD_FM_CRC					93				//ǰN�ֽ�CRCУ�飬2bytes
																		//95~255����
#define PCD_FM_BACKUP			256			//256~511Ϊ���ݱ���


//PCD��PCͨѶ������
#define PCD2PC_CMD_POLL					0x30		//��ͨ��ѯ����
#define PCD2PC_CMD_REALINFO			0x31		//���ͻ�����ʵʱ��Ϣ����
#define PCD2PC_CMD_ZDUPLOAD			0x32		//���ͻ����ͳɽ���������
#define PCD2PC_CMD_DOWNSTART		0x33		//���ͻ�����������������
#define PCD2PC_CMD_DOWNLOAD		0x34		//���ͻ�������������
#define PCD2PC_CMD_GREYINFO			0x35		//���ͻ���PC��ѯ�Ҽ�¼����
#define PCD2PC_CMD_LISTINFO			0x36		//���ͻ���PC��ѯ��/��������Ϣ����
#define PCD2PC_CMD_SUMREAD			0x38		//PC��ȡ���ͻ��ۼ�������
#define PCD2PC_CMD_INFOREAD			0x3A		//PC��ȡ���ͻ���Ϣ����
#define PCD2PC_CMD_ERRINFO				0x3B		//���ͻ������ڲ�������Ϣ����
#define PCD2PC_CMD_ERRACK				0x3C		//PCͨѶ����ȷ������
#define PCD2PC_CMD_ZDREAD				0x3E		//PC��ȡ������������
#define PCD2PC_CMD_ZDNO					0x3F		//���ͻ���ӦPC������˵�����
#define PCD2PC_CMD_BAR_CHECK		0x60		//���ͻ����̨��ѯ��֤������
#define PCD2PC_CMD_BAR_RESULT		0x61		//��̨����ͻ����ص���֤���ѯ���
#define PCD2PC_CMD_BAR_ACK			0x62		//���ͻ����̨������֤���ѯȷ�Ͻ������
#define PCD2PC_CMD_BAR_ACKDONE	0x63		//��̨ȷ�������������
#define PCD2PC_CMD_DISCOUT_ASK	0x70		//���̨�����ۿ۶����ϵͳ���ͻ����20160401
#define PCD2PC_CMD_APPLY_DEBIT		0x71		//���̨����ETC���ۿ�
#define PCD2PC_CMD_ETC_FUN        0x80      //ETC��������


//PCD��IPTͨѶ������
#define PCD_CMD_POLL							0x01				//��ͨ��ѯ����
#define PCD_CMD_FORTTC						0x02				//IPT����TTC
#define PCD_CMD_ZDSAVE						0x03				//IPT���뱣���˵�
#define PCD_CMD_LIST							0x04				//IPT��ѯ��/������
#define PCD_CMD_GREYINFO					0x05				//IPT��ѯ������¼
#define PCD_CMD_PRINTE						0x06				//IPT��ӡ����
#define PCD_CMD_SPK								0x07				//IPT��������
#define PCD_CMD_ZDCHECK					0x08				//��ѯ�˵�
#define PCD_CMD_IDSET							0x09				//��������ID
#define PCD_CMD_BARCODE					0x0a				//��������ת��
#define PCD_CMD_FOR_TMAC					0x0b				//PCD����IPT����TMAC
#define PCD_CMD_ERRINFO_UPLOAD	0x0c				//IPTͨ��PCD�ϴ��ڲ�������Ϣ����̨
#define PCD_CMD_DISCOUNT_ASK		0x0d				//IPTͨ��PCD���̨�����ۿ۶�
#define PCD_CMD_APPLYFOR_DEBIT	0x0e				//����ۿ�

//PCD��IPT��Ϣ����
#define PCD_MSGTYPE_IPT					1					//PCD��IPT��ͨѶ��Ϣ����


//��/�������ṹ
typedef struct
{
	unsigned char Version[2];							//�汾��
	unsigned char TimeStart[4];						//��Ч����
	unsigned char TimeFinish[4];					//��ֹ����
	unsigned char Area[2];								//��Ч����
	unsigned char Number[4];						//��������
}PcdListInfoType;

//��վͨ����Ϣ�ṹ
typedef struct
{
	unsigned char Version;								//�汾��
	unsigned char Province;							//ʡ����
	unsigned char City;									//���д���
	unsigned char Superior[4];						//�ϼ���λ����
	unsigned char S_ID[4];								//����վID
	unsigned char POS_P;								//ͨѶ�ն��߼����
	unsigned char GUN_N;								//ǹ��1~6
	unsigned char NZN[PCD_NOZZLE_MAX];	//ǹ��
}PcdStationInfoType;

//��Ʒ�ͼۼ�¼�ṹ��ÿ��ǹ���洢�۸���ĿΪ8
typedef struct
{
	unsigned char NZN;									//ǹ�ţ�HEX
	unsigned char O_TYPE[2];						//��Ʒ���룬ѹ��BCD
	unsigned char Density[4];							//�ܶȣ�HEX
	unsigned char Price_n;								//�۸���Ŀ1~3��HEX
	unsigned char Price[6];								//�۸�HEX��2bytes��¼һ������
}PcdOilFiledType;

//��Ʒ�ͼ۱�ṹ�����洢����ǹ���ͼۼ�¼
typedef struct
{
	unsigned char Version;											//�汾��HEX
	unsigned char ValidTime[6];									//����Ʒ�ͼ���Чʱ�䣬ѹ��BCD
	unsigned char FieldNumber;									//��¼����HEX
	PcdOilFiledType Field[PCD_NOZZLE_MAX];			//��ǰ��Ʒ�ͼۼ�¼
	PcdOilFiledType FieldNew[PCD_NOZZLE_MAX];		//����Ʒ�ͼۼ�¼
}PcdOilRecordType;

//������ǹ��Ϣ
typedef struct
{
	unsigned char State;									//״̬��0=���У�1=�����룻2=̧ǹ�������
	unsigned char LogicNozzle;						//�߼�ǹ��
	unsigned char SumMoney[4];					//���۽��
	unsigned char SumVolume[4];					//��������
	unsigned char CardID[10];						//��Ӧ�ú�
	unsigned char CardState[2];					//��״̬
	unsigned char CardBalance[4];					//�����
	unsigned char PayUnit;								//���㵥λ/��ʽ
	unsigned char Money[3];							//����
	unsigned char Volume[3];							//����
	unsigned char Price[2];								//�۸�
}PcdNozzleInfoType;

//֧���ն���Ϣ
typedef struct
{
	unsigned int  OfflineTimer;						//֧���ն�����ʱ��
	unsigned char IsOnline;							//֧���ն��Ƿ�����(���Ƿ���PCDͨѶ����)��0=���ߣ�1=����
	unsigned char PhysicalNozzle;					//����ǹ��
	unsigned char NozzleNumber;					//����ǹ��Ŀ

	PcdNozzleInfoType Nozzle[3];					//����ǹ��Ϣ
}PcdFuellingPointInfoType;

//PC�����Ϣ
typedef struct
{
	PcdStationInfoType SInfo;		//ͨ����Ϣ

	PcdListInfoType BList;				//����������
	
	PcdListInfoType ABList;			//����������
	
	PcdListInfoType DBList;			//��ɾ������

	PcdListInfoType WList;				//������

	PcdOilRecordType OilInfo;			//��Ʒ�ͼ���Ϣ
	
	unsigned char Time[7];				//ʱ��
}PcdPcInfoType;

typedef struct
{
	NODE	Ndptrs;						      //�ڵ�ָ��
	unsigned char msgType;			  //IPT��Ϣ����
	unsigned char mboardId;			  //IPT֧���ն������
	unsigned char fuellingPoint;	//IPT֧���ն�����
	unsigned char phynozzle;			//IPT����ǹ��
	unsigned char command;			  //IPT��Ϣ������
	unsigned char Buffer[128];		//��Ҫ�����IPT����
													      //��ѯ��/��������Ϣʱ:
													      //����(10BCD);
													      //��ѯ�ҽ��׼�¼ʱ:
													      //����(10BCD);���(4HEX);CTC(2HEX);�ۿ���Դ(1HEX);ʱ��(7BCD);
													      //���̨�����ۿ۶�ʱ:
													      //ǹ��(1HEX);����(10BCD)
	unsigned char Nbytes;				 //��Ҫ�����IPT���ݳ���
}PcdIpt2PcNode;

//PCD�����������
typedef struct
{
	int tIdProcess;											//PCD���ݴ��������
	int tIdPcReceive;										//PCD���պ�̨PC���ݲ���������
	int comFdPc;											//���̨�����ͨѶ����
	int comFdMboard1;									//��չ������1������֮��ͨѶ����
	int comFdMboard2;									//1��������2����չ����֮��ͨѶ����
	int comFdMboard3;									//1��������3����չ����֮��ͨѶ�ӿ�
	int comFdPrint1;										//������1�Ŵ�ӡ�����Ӵ���
	int comFdPrint2;										//������2�Ŵ�ӡ�����Ӵ���
	unsigned char mboardID;							//���������1~3
	char PcTime[7];										//��̨PCʱ��
	
	//WDOG_ID wdgId;									//���Ź���ʱ��ID

	//֧���ն���Ϣ
	// 	���ֻ֧��6��֧��֧���նˣ���Ӧÿ�����̣���Ӧ������ǹ��
	//	ÿ��֧���ն����֧��3���߼���ǹ������Ӧ��̨���õ��߼����ţ�
	
	PcdFuellingPointInfoType FP_1;				//����ǹ��1��Ϣ
	PcdFuellingPointInfoType FP_2;				//����ǹ��2��Ϣ
	PcdFuellingPointInfoType FP_3;				//����ǹ��3��Ϣ
	PcdFuellingPointInfoType FP_4;				//����ǹ��4��Ϣ
	PcdFuellingPointInfoType FP_5;				//����ǹ��5��Ϣ
	PcdFuellingPointInfoType FP_6;				//����ǹ��6��Ϣ

	//PCD���������
	//MSG_Q_ID msgIdRx;								//PCD���������

	//PCD��IPT��ͨѶ�������
	//MSG_Q_ID msgIdFromIpt;						//PCD����IPT���ݵ���Ϣ����     
    
	int msgIdFromIpt;     //fj:PCD����IPT���ݵ���Ϣ����
	int msgIdFromPc;      //fj:PCD����PC���ݵ���Ϣ����
	int msgIdRx;          //fj:PCD�������Ժ�̨�������������


	LIST ipt2PcList;										//IPT��PC��ѯ���ݵ�����
	PcdIpt2PcNode *ipt2PcNode;					//IPT��PC��ѯ���ݵ����ݽڵ�

	//PCD��PC��ͨѶ�������
	//MSG_Q_ID msgIdFromPc;						//PCD����PC���ݵ���Ϣ����
	unsigned char pcFrame;							//PCD��PC��ͨѶ֡��
	unsigned char pcCommand;						//��PC���͵���������
	unsigned int pcSendTimer;						//��PC���͵�ʱ����
	unsigned int pcOvertimer;						//��PC���͵����ݷ��س�ʱʱ��
	unsigned char pcOverTimes;					//��PC���͵����ݷ��س�ʱ����
	unsigned char PcOnline;							//PC��̨����/����״̬��0=���ߣ�1=����

	//����������ز���
	unsigned char PcDownloadContent;					//�����������ݣ�0=������������1=������������
																			//	2=��ɾ��������3=��������4=��Ʒ�ͼ���Ϣ��5=ͨ����Ϣ
																			//	����(0xff)��ʾ����������
	unsigned int PcDownloadLen;							//�������������ܳ���
	unsigned int PcOffsetLen;								//�����������������س���
	unsigned char PcBaseListTimes;						//�����������жϵ����̨�汾��һ�µĴ���
	unsigned char PcAddListTimes;						//�����������жϵ����̨�汾��һ�µĴ���
	unsigned char PcDelListTimes;							//��ɾ�������жϵ����̨�汾��һ�µĴ���
	unsigned char PcWListTimes;							//�׺������жϵ����̨�汾��һ�µĴ���
	unsigned char PcStaionInfoTimes;					//ͨ����Ϣ�жϵ����̨�汾��һ�µĴ���
	unsigned char PcPriceInfoTimes;						//��Ʒ�ͼ۱��жϵ����̨�汾��һ�µĴ���
	
	
	//�˵��洢���ϴ���ز���
	unsigned int TTC;												//TTC�����������ݵ�ǰTTC
	unsigned int UnloadNumber;							//δ�ϴ��˵�����
	unsigned int UploadTTC;									//������Ҫ�ϴ����˵�TTC
	unsigned int UploadTimer;								//�˵�δ��ɳ�ʱ��ʱ��
	unsigned char NewZD;										//0=δ�������˵���1=�����˵�
	unsigned char UploadZD[PCD_ZD_LEN];			//���ϴ��˵�

	//�ӿ��˵���Ŀ
	unsigned int AbnormalNumber;						//�쳣�˵���Ŀ

	//��������ǹ�ű���ĸ���ǹ����
	unsigned int TTC1;											//1��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber1;									//1��ǹ���˵�����
	unsigned int UnloadNumber1;							//1��ǹ��δ�ϴ��˵�����
	unsigned int TTC2;											//2��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber2;									//2��ǹ��δ�ϴ��˵�����
	unsigned int UnloadNumber2;							//2��ǹ��δ�ϴ��˵�����
	unsigned int TTC3;											//3��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber3;									//3��ǹ��δ�ϴ��˵�����
	unsigned int UnloadNumber3;							//3��ǹ��δ�ϴ��˵�����
	unsigned int TTC4;											//4��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber4;									//4��ǹ��δ�ϴ��˵�����
	unsigned int UnloadNumber4;							//4��ǹ��δ�ϴ��˵�����
	unsigned int TTC5;											//5��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber5;									//5��ǹ��δ�ϴ��˵�����
	unsigned int UnloadNumber5;							//5��ǹ��δ�ϴ��˵�����
	unsigned int TTC6;											//6��ǹ��TTC�����������ݵ�ǰTTC
	unsigned int ZDNumber6;									//6��ǹ��δ�ϴ��˵�����
	unsigned int UnloadNumber6;							//6��ǹ��δ�ϴ��˵�����

	unsigned char UnloadFlag;                           //���ر�ʶ����,szb_fj20171120
}PcdParamStructType;


//�ⲿ��������
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

