#ifndef _OIL_LIANDI_H_
#define _OIL_LIANDI_H_

//#include <msgQLib.h>

#include "lstLib.h"

#define LD_BUFFER_SIZE				512			    //�ն����ݻ����С
#define LD_ICDEV_USERCARD			0x00			//��ӿ���
#define LD_ICDEV_SAM1CARD			0x01			//SAM1����
#define LD_ICDEV_SAM2CARD			0x02			//SAM2����
#define LD_ICDEV_SAM3CARD			0x03			//SAM3����

#define LD_CMD_SHAKE_HANDS		    0x0001		//���巢�ͷ�����������
#define LD_CMD_INTERFACE_DO		    0x0002		//���巢�ͷ���ִ�ж�Ӧ�Ľӿں���
#define LD_CMD_DSP				    0x0003		//���巢�ͷ���ִ����ʾ����
#define LD_CMD_2					0x0004		//��ȡIO״̬
#define LD_CMD_4					0x0005		//���ô���
#define LD_CMD_3					0x0006		//��ȡ��������
#define LD_CMD_5					0x0007		//����ͨ������ģ�鴮��ת������
#define LD_CMD_6					0x0008		//���ϴ�������ת��������
#define LD_CMD_ICMODULE_INIT		0x0010		//���Ϳ��豸��ʼ��
#define LD_CMD_ICDEVICE_OPEN		0x0011		//�򿪼��Ϳ��豸
#define LD_CMD_ICDEVICE_CLOSE	    0x0012		//�رռ��Ϳ��豸
#define LD_CMD_IC_POWER_UP		    0x0013		//���Ϳ��豸�ϵ�
#define LD_CMD_IC_POWER_DOWN	    0x0014		//���Ϳ��豸�µ�
#define LD_CMD_IC_APDU				0x0015		//���Ϳ��豸APDUͨѶ
#define LD_CMD_ESEJECT_CARD		    0x0016		//���Ϳ��˿�
#define LD_CMD_CARD_POLL		    0x0017		//Ѱ��
#define LD_CMD_BANK_JYJFREE		    0x0020		//���ͻ�����
#define LD_CMD_BANK_PAYMENT		    0x0021		//���п��������ݽ���
#define LD_CMD_BUTTON			    0x1001		//�����ն��ϴ�����ֵ

//�����ڵ�
typedef struct
{
	NODE Ndptrs;				//�ڵ�ָ��
	unsigned int Value; 	//����

}LDButtonNodeStruct;

typedef struct
{
	unsigned char BitMap;				//λͼb0~b7�ֱ�����0~7�У�=0�޴���=1�д���
	unsigned char Font[8];				//������ʾ�ֿ�����=0		16*8,16*16�ֿ� 
       										            //=1    	14*7,14*14�ֿ� 
	   												    //=2    	8*8�ֿ�
	   												    //=3    	12*6,12*12�ֿ�
	unsigned char IsContrary[8];	    //�����Ƿ���0=������ʾ��1=���з���
	unsigned char OffsetY[8];			//����ƫ�Ƽ�����
	unsigned char Buffer[8][32+1];		//��ʾ���ݣ�8�У��������32�ֽ�
	unsigned char Nbytes[8];			//��ʾ���ݳ���

}LDDspContentType;


typedef struct
{
	int ComX;								//�������ն��豸ͨѶ�Ĵ��ں�
	unsigned char Device;			        //�豸����:'1' = �������̣�'2' = �����ն�ģ�飻
	unsigned char SendFrame;	//�������ݵ�֡��
	unsigned char AckBuffer[LD_BUFFER_SIZE + 1];		//���յ��ͻ���������Ӧ������
	int AckLenght;															//���յ��ͻ���������Ӧ�����ݳ���
	int Contrast;							//�Աȶ�
	LDDspContentType Content;	            //��ʾ����
	LIST ButtonList;						//��������
	unsigned char DeckStateS1;			    //S1����״̬=0x30��ʾ�п�,0x31��ʾ�޿���
	unsigned char IcTypeS2;					//S2:������
																		//S2=0x30 �������޿�
																		//S2=0x3f �޷�ʶ��
																		//S2=0x31 ����cpu��
																		//S2=0x32 RF-- TYPE B CPU��
																		//S2=0x33 RF-TYPE A CPU��
																		//S2=0x34 RF-M1��
	unsigned char IcStateS3;							                //S3����״̬
																		//S3=0x30 �µ�״̬
																		//S3=0x31 ����״̬
																		//S4=0x32 ����״̬
																		//S5=0x33 æ̬��ͨ��״̬��
	
}LDStructParamType;


extern int ldDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes);
extern int ldDsp(int DEV_DSP_KEYx, int Contrast, int IsClr);
extern unsigned int ldButtonRead(int kb_dev_id);
extern int ldICPowerUp(int nozzle, unsigned char dev_x);
// extern int ldICPowerDown(int nozzle, unsigned char dev_x);
extern int ldICExchangAPDU(int nozzle, unsigned char dev_x, char *inbuffer, int nbytes, char *outbuffer, int maxbytes);
extern int ldESEjectCard(int nozzle);
// extern int ldICStateRead(int nozzle, char *outbuffer);
extern int ldBankPayment(int DEV_DSP_KEYx, const void *pSend, unsigned int nSendLen, void *pRec, int *pnRecLen);
extern int ldInit(int DEV_DSP_KEYx);

#endif

