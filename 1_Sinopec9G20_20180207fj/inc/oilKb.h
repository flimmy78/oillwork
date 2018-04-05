#ifndef _OIL_KB_H_
#define _OIL_KB_H_

//#include <semLib.h>
//#include <pthread.h>



//�궨��
#define KB_CONTRAY_NO		    0								//������
#define KB_CONTRAY				1								//����
#define KB_DSPCLR_NO			0								//��ʾ������
#define KB_DSPCLR				1								//��ʾ����
#define KB_FONT16				1								//16*16��16*8�����ֿ�
#define KB_FONT14				2								//14*14��14*7�����ֿ�
#define KB_FONT8				3								//8*8�����ֿ�
#define KB_FONT12				4								//12*12��12*6�����ֿ�
//#define KB_MSG_MAX_LEN	512							//��Ϣ������󳤶�
#define KB_UART_MAX_SIZE		2048
#define KB_RNG_MAX_LEN		    2048						    //���ڽ��ջ�����󳤶�
#define KB_IO_DEBOUNCE		    5								//������������
#define KB_USARTA1				1								//A1������չ����1__��ӡ����
#define KB_USARTA2				2								//A1��չ����2__IC����
#define KB_USARTA3				3								//A1��չ����3__����ӿ�
#define KB_USARTA4				4								//A1��չ����4__���봮��
#define KB_USARTA5				5								//A1��չ����5__PCD/����������������
#define KB_USARTB1				6								//B1������չ����1__��ӡ����
#define KB_USARTB2				7								//B1��չ����2__IC����
#define KB_USARTB3				8								//B1��չ����3__����ӿ�
#define KB_USARTB4				9								//B1��չ����4__���봮��
#define KB_USARTB5				10								//B1��չ����5__PCD/����������������
#define KB_REFRESH_TIME	(5*ONE_SECOND)		//������ʾˢ��ʱ����

//"9g20"<->"����"ͨѶ���ݶ˿ڶ���
#define KB_DATA_HI				0							//�߰��ֽ�
#define KB_DATA_LOW			    1							//�Ͱ��ֽ�
#define KB_PORT_SET			    0							//���̣����ü���ѯ�ö˿ڣ��������Ӽ������������Ӵ��ڻ�CAN�ڣ�
#define KB_PORT_DSP			    1							//������ʾ�˿�
#define KB_PORT_BUTTON	        2							//�����˿�
#define KB_PORT_IC				3							//�������˿�__����1
#define KB_PORT_PRN			    4							//��ӡ�˿�__����2
#define KB_PORT_CODE			5							//��������ɨ��˿�__����4
#define KB_PORT_NET			    6							//�����������˿�__����5
#define KB_PORT_SWITCH	        7							//�����źż���λ״̬����˿�

//����ֵ����������
#define KB_BUTTON_NO			0xffffffff		//�ް���ֵ
#define KB_BUTTON_0				0x00000000		// 0
#define KB_BUTTON_1				0x00000001		//1
#define KB_BUTTON_2				0x00000002		// 2
#define KB_BUTTON_3				0x00000003		//3
#define KB_BUTTON_4				0x00000004		//4
#define KB_BUTTON_5				0x00000005		//5
#define KB_BUTTON_6				0x00000006		//6
#define KB_BUTTON_7				0x00000007		//7
#define KB_BUTTON_8				0x00000008		//8
#define KB_BUTTON_9				0x00000009		//9
#define KB_BUTTON_CZ			0x0000000a		//����/.
#define KB_BUTTON_BACK		    0x0000000b		//�˿�
#define KB_BUTTON_ACK			0x0000000c		//ȷ��
//����ֵ����ͨʯ����
#define KB_BUTTON_SET			0x0000000d		//����
#define KB_BUTTON_UP			0x0000000e		//��
#define KB_BUTTON_DOWN		    0x0000000f		//��
#define KB_BUTTON_MON			0x00000010		//�����
#define KB_BUTTON_VOL			0x00000011		//������
#define KB_BUTTON_CHG			0x00000012		//����
#define KB_BUTTON_SEL			0x00000013		//ѡ��
#define KB_BUTTON_BACK1		    0x00000014		//�˿�+1
#define KB_BUTTON_BACK2		    0x00000015		//�˿�+2
#define KB_BUTTON_BACK3		    0x00000016		//�˿�+3
#define KB_BUTTON_BACK4		    0x00000017		//�˿�+4
#define KB_BUTTON_BACK5		    0x00000018		//�˿�+5
#define KB_BUTTON_BACK6		    0x00000019		//�˿�+6
#define KB_BUTTON_BACK7		    0x0000001a		//�˿�+7
#define KB_BUTTON_BACK8		    0x0000001b		//�˿�+8
#define KB_BUTTON_BACK9		    0x0000001c		//�˿�+9
#define KB_BUTTON_SET1			0x0000001e		//����+1
#define KB_BUTTON_SET2			0x0000001f		//����+2
#define KB_BUTTON_SET3			0x00000020		//����+3
#define KB_BUTTON_SET4			0x00000021		//����+4
#define KB_BUTTON_SET5			0x00000022		//����+5
#define KB_BUTTON_SET6			0x00000023		//����+6
#define KB_BUTTON_SET7			0x00000024		//����+7
#define KB_BUTTON_SET8			0x00000025		//����+8
#define KB_BUTTON_SET9			0x00000026		//����+9
//����ֵ��ʯ��������
#define KB_BUTTON_L1				0x00000020		//�����1
#define KB_BUTTON_L2				0x00000021		//�����2
#define KB_BUTTON_L3				0x00000022		//�����3
#define KB_BUTTON_L4				0x00000023		//�����4
#define KB_BUTTON_R1				0x00000024		//�����1
#define KB_BUTTON_R2				0x00000025		//�����2
#define KB_BUTTON_R3				0x00000026		//�����3
#define KB_BUTTON_R4				0x00000027		//�����4
#define KB_BUTTON_CLR			    0x00000028		//���/�˳�
#define KB_BUTTON_SCORE		        0x00000029		//�û���
#define KB_BUTTON_PRESET		    0x0000002a		//����/Ǯ��
//��Ʒѡ��ť������ֵ
#define KB_BUTTON_YP1			0x0000002b		//YP1
#define KB_BUTTON_YP2			0x0000002c		//YP2
#define KB_BUTTON_YP3			0x0000002d		//YP3

//״̬����
#define KB_KEYLOCK_OIL			0						//�������ڼ���λ��
#define KB_KEYLOCK_SET			1						//������������λ��


//������ʾ�������ݽṹ��������ǰ��ʾ���ݼ���Ҫ��ʾ�����ݱ�ʶ
typedef struct
{
	unsigned char Buffer[128*8];	//��ʾ����
	unsigned char Pointer[8];			//��n�д���ʾ��ʼ�±�
	unsigned char Len[8];				//��n�д���ʾ����

}KbDspStructType;


//�������̷�������
typedef struct
{
	//SEM_ID SemIdSet;			//���ź�������ά���������ö˿�ͨѶ
	pthread_mutex_t SemIdSet;   //fj:20170911
	char SetBuffer[256];			//��ѯ/���ò������ݻ���
	int SetLen;							//��ѯ/���ò������ݻ��泤��
	char SetPack;						//��ѯ/���ò����Ƿ���ܵ�һ֡��������0=��1=��

	char DspBuffer[128];			//��ѯ/���ò������ݻ���
	int DspLen;							//��ѯ/���ò������ݻ��泤��
	char DspPack;						//��ѯ/���ò����Ƿ���ܵ�һ֡��������0=��1=��

	char ButtonBuffer[128];		//�����������ݻ���
	int ButtonLen;						//�����������ݻ��泤��
	char ButtonPack;					//���������Ƿ���ܵ�һ֡��������0=��1=��
	unsigned int ButtonLast;		//��һ�ΰ���

	char RstStateLast;				//���̸�λǰһ��״̬
	char RstState;						//���̸�λ״̬0=δ��λ��ɣ�1=�Ѹ�λ���
	char RstStateChg;				//���̸�λ״̬�Ƿ�仯��0=�ޣ�1=��

	char KeyLockLast;				//LOCK1��������ƽǰһ��״̬
	char KeyLock;						//LOCK1��������ƽ0=�͵�ƽ��1=�ߵ�ƽ
	char KeyLockChg;				//LOCK1��������ƽ�Ƿ�仯��0=�ޣ�1=��

	char OilLeftLast;					//YP1����Ʒѡ��ǰһ��״̬
	char OilLeft;							//YP1����Ʒѡ��0=�͵�ƽ��1=�ߵ�ƽ
	char OilLeftChg;					//YP1����Ʒѡ���Ƿ�仯��0=�ޣ�1=��

	char OilRightLast;				//YP2����Ʒѡ��ǰһ��״̬
	char OilRight;						//YP2����Ʒѡ��0=�͵�ƽ��1=�ߵ�ƽ
	char OilRightChg;					//YP2����Ʒѡ���Ƿ�仯��0=�ޣ�1=��

	char OilConfirmLast;			//YP3��Ʒȷ��ǰһ��״̬
	char OilConfirm;					//YP3��Ʒȷ�ϣ�0=�͵�ƽ��1=�ߵ�ƽ
	char OilConfirmChg;			//YP3��Ʒȷ��ѡ���Ƿ�仯��0=�ޣ�1=��

	char PrinterBusyLast;			//��ӡ��æ��ǰһ��״̬
	char PrinterBusyState;		//��ӡ��æ�У�0=�͵�ƽ��1=�ߵ�ƽ
	char PrinterBusyChg;			//��ӡ��æ�У�0=�ޣ�1=��

	char PIRStateLast;				//��Ӧ����ǰһ��״̬
	char PIRState;					//��Ӧ���أ�0=�͵�ƽ��1=�ߵ�ƽ
	char PIRStateChg;				//��Ӧ���أ�0=�ޣ�1=��

}KbParamStructType;



//�����ⲿ����
extern int kbBuzzerBeep(int KEYx, char type);
extern unsigned int kbButtonRead(int kb_dev_id);
extern int kbSwitchRead(int DEV_SWITCH_xx, int *ischg);
extern int kbDspContrastSet(int DEV_DSP_KEYx, unsigned char contrast);
extern void kbDspEmptyRefresh(int DEV_DSP_KEYx);
extern int kbDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes);
extern int kbDsp(int DEV_DSP_KEYx, int Contrast, int IsClr);
extern int kbIOWrite(int DEV_DSP_KEYx, unsigned char GPIOx, unsigned char state);
extern int kbPIRSateRead(int KEYx);
extern int kbUartRead(int KB_USARTxx, char *buffer, int maxbytes);
extern int kbUartWrite(int KB_USARTxx, char *buffer, int nbytes);
// //��Ʒ��ť��ؽӿ�
// extern int kbYPUserSet(int DEVx, int user);
// extern int kbYPUserGet(int DEVx, int *user);
extern int kbYPRead(int DEVx, int *value);
// //���ö˿���ؽӿ�
extern int kbSetPsamTransmit(int KEYx, unsigned char *inbuffer, int nbytes, unsigned char *outbuffer, int *outbytes);

//�����֮��Ĳ�����ʼ��
extern bool kbInit(void);
//extern void kbExit(void);

void tRxCom4();
void tRxCom5();
void kbTaskSwitchRead();
void tKbDsp(void* kbType);

#endif

