#ifndef _OIL_IC_H_
#define _OIL_IC_H_

#include <semaphore.h>


#define ICDATA_LEN					512	//IPT����IC��������󳤶�											                                                                 
#define IC_OVERTIME					(5*1000)	//�����ʱʱ��,5��
#define ICSTATE_OK                  0  //IC��������ֵ���ɹ�
#define ICSTATE_FAILED				1   //IC��������ֵ����:ʧ��
#define ICSTATE_OVERTIME			2   //IC��������ֵ����:��ʱ
#define ICSTATE_NOZZLEERR			3   //IC��������ֵ����:ǹѡ�Ƿ�
#define ICSTATE_CACHEERR			4   //IC��������ֵ����:�������̫С
//IC������
#define IC_CARD_CPU					0x31    //����CPU��
#define IC_CARD_RF_TYPEB			0x32	//RF-- TYPE B CPU��
#define IC_CARD_RF_TYPEA			0x33	//RF-- TYPE A CPU��
//IC��Ӧ������
#define ICTYPE_USER					0x01	//�û���
#define ICTYPE_MANAGE				0x02	//����
#define ICTYPE_STAFF				0x04	//Ա����
#define ICTYPE_PUMP					0x05	//��ÿ�
#define ICTYPE_SERVICE				0x06	//ά�޿�
#define ICTYPE_PSAM					0x10	//PSAM��
//IC�����豸ǹ��
#define IC_NOZZLE_1					0		//1�ſ����豸
#define IC_NOZZLE_2					1		//2�ſ����豸

//IC���������������ݽṹ
typedef struct
{
	unsigned char DeckStateS1;						//S1����״̬=0x30��ʾ�п�,0x31��ʾ�޿���
	unsigned char IcTypeS2;							//S2:������ ,S2=0x30 �������޿� ,       S2=0x3f �޷�ʶ�� ,     S2=0x31 ����cpu��
															   //S2=0x32 RF-- TYPE B CPU�� ,S2=0x33 RF-TYPE A CPU��,S2=0x34 RF-M1��

	unsigned char IcStateS3;						//S3����״̬ ,S3=0x30 �µ�״̬ ,S3=0x31 ����״̬
																//S4=0x32 ����״̬ ,S5=0x33 æ̬��ͨ��״̬��

	unsigned char PowerStateS4;						//S4������״̬,S4=0x30 ��������,S4=0x31 ����																													
}IcStateType;


//��������ز���
typedef struct
{
	int nozzle; //ǹѡID
	IcStateType State; //��������
	unsigned char RxValid;	//������Чλ:0=��Ч֡��1=��Ч֡										
	unsigned char RxBuffer[ICDATA_LEN];	//���ݻ���				
	unsigned int RxLen;	 //���ݳ���

	//del SEM_ID semIdShoot;
	pthread_mutex_t semIdShoot;   //���������ź���

	char shootAsk; //�Ƿ���Ҫ����0=����Ҫ��1=��Ҫ
	char pollLimit;//�Ƿ��ֹ��ѯ���0=����1=��ֹ
	int tIdReceive; //�����ݽ�������ID
	int tIdShoot; //��������ID
	int tIdPoll; //��״̬��ѯ����ID
}IcStructType;



//IC������
extern int ICShoot(int nozzle);
//IC����λ
extern int ICReset(int nozzle, int sam);
//IC��MFѡ��
extern int ICMFSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��Ӧ��ѡ��
extern int ICAppSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char app);
//IC��21�ļ���ȡ
extern int ICFile21Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��22�ļ���ȡ
extern int ICFile22Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��27�ļ���ȡ
extern int ICFile27Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��28�ļ���ȡ
extern int ICFile28Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��������ϸ��ȡ
extern int ICNotesRead(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char notes_id);
//IC������У��
extern int ICPinCheck(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *pin, int pin_len);
//IC��������Ϣ��ȡ
extern int ICGreyInfoRead(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC������ȡ
extern int ICBalanceRead(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��������ʼ��
extern int ICLockInit(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char keyIndex, unsigned char *termId);
//IC������
extern int ICGreyLock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *psamTTC, unsigned char *psamRandom, unsigned char *time, unsigned char *psamMAC1);
//IC�����
extern int ICGreyUnlock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned int money,unsigned char *ICLockInitCTC, unsigned char *PsamTermId, unsigned char *ICPsamTTC,unsigned char *time, unsigned char *ICPsamGMAC);
//IC�����TAC
extern int ICTacClr(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��26�ļ���ȡ
extern int ICFile26Read(int nozzle, int sam, unsigned char *buffer, int maxbytes);
//IC��ר��DES����(DES CRYPT)
extern int ICDESCrypt(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *PSAMRandom);
//IC�������־��¼
extern int ICAppendLog(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardId, unsigned char *RIDMAC);


//PSAM����λ
extern int PsamReset(int nozzle,int sam);
//PSAM��MFѡ��
extern int PsamMFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM��21�ļ���ȡ
extern int PsamFile21Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM��22�ļ���ȡ
extern int PsamFile22Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM��DFѡ��ʯ��Ӧ��
extern int PsamDFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes, int DF);
//PSAM��23�ļ���ȡ
extern int PsamFile23Read(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM����ȡ��ȫ����״̬
extern int PsamGetAPProof(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM����ȡ�����
extern int PsamGetRandom(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM����ȫ������֤
extern int PsamAPAuthen(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *Ciphertext);
//PSAM������ʼ��������MAC1	
extern int PsamLockInit(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICLockInitRandom, unsigned char *ICLockInitCTC, unsigned char *ICLockInitBalance, unsigned char *time, unsigned char ICLockInitKeysVersion, unsigned char ICLockInitArithmetic, unsigned char *ICAppId, unsigned char *ICIssuerMark);
//PSAM��֤MAC2
extern int PsamMAC2Check(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *MAC2);
//PSAM����GMAC
extern int PsamGMAC(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICAppId, unsigned char *ICLockInitCTC, unsigned int money);
//PSAM��ȡGMAC
extern int PsamGMACRead(int nozzle,int sam, unsigned char *buffer, int maxbytes, const unsigned char *ICPsamTTC);
//PSAM��TMAC�����ʼ��
extern int PsamTMACInit(int nozzle,int sam, unsigned char *buffer, int maxbytes);
//PSAM������TMAC
extern int PsamTMACOperat(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *inbuffer, int len, int follow, int initvalue);
//PSAM����������ע�Ṧ��
extern int PsamStartBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *Ciphertext);
//PSAM����ʼ������ע�Ṧ��
extern int PsamInitBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *CoID, unsigned char *Ciphertext);
//PSAM������ע��
extern int PsamBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char const *MboardID, unsigned char *CoID);
//PSAM��ר��DES�����ʼ��
extern int PsamInitDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char CalKeyIndex);
//PSAM��ר��DES����
extern int PsamDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardId);


//���������������
//���뿨�������Բ���
extern int IcReaderTestElectrical(int nozzle, int type);
//���뿨��Э�����
extern int IcReaderTestProtocol(int nozzle, int type);
//���뿨����Ƶ����
extern int IcReaderTestRadiofrequency(int nozzle);
//����Э������������ѯʱ��
extern int IcReaderTestProtocolTime(int nozzle, int time);
//�˳�����ģʽ
extern int IcReaderTestExit(int nozzle);
//��ֹ��ѯ����
extern int IcPollLimit(int nozzle);
//������Ѱ����
extern int IcPollStart(int nozzle);


//��Կ���ؿ�ר������
//IC��Կ��ADFѡ��
extern int ICKeyADFSelect(int nozzle, unsigned char *buffer, int maxbytes);
//IC��Կ��ѡ��EF 01�ļ�
extern int ICKeyEF01Select(int nozzle, unsigned char *buffer, int maxbytes);
//IC��Կ��EF 01�ļ���ȡ
extern int ICKeyEF01Read(int nozzle, unsigned char *buffer, int maxbytes);
//IC��Կ��EF 01�ļ�д��
extern int ICKeyEF01Write(int nozzle, unsigned char *buffer, int maxbytes, unsigned int number);
//IC��Կ��ѡ��EF 02�ļ�
extern int ICKeyEF02Select(int nozzle, unsigned char *buffer, int maxbytes);
//IC��Կ��EF 02�ļ���ȡ
extern int ICKeyEF02Read(int nozzle, unsigned char *buffer, int maxbytes, unsigned int offset, int readbytes);


extern int ICStateRead(int nozzle, IcStateType *state);//IC״̬��ȡ
extern bool IcModuleInit(void);//ICģ���ʼ�� 

extern void ResetIcPack(int nozzle);

void tICReceive(void* pNozzleType);
void tICPoll(void* pNozzleType);

#endif


