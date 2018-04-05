#ifndef _OIL_BOARD_TRANS_H_
#define _OIL_BOARD_TRANS_H_


//#define BDATA_COM_1								COM7		//��һ��ͨѶ����
//#define BDATA_COM_2								COM8		//�ڶ���ͨѶ����

#define BDATA_COM_1   DEV_COM7
#define BDATA_COM_2   DEV_COM8

#define BDATA_SIZE_MAX							2048	//����һ֡������󳤶�

/*�����������͵Ķ���*/
#define BDATA_TYPE_TOPRINTE				0x0001		//���͸���ӡ��������			
#define BDATA_TYPE_FROMPRINTE			0x0002		//��ӡ�����ص�����
#define BDATA_TYPE_TOSPK						0x0003	//���͸�����������������
#define BDATA_TYPE_FROMSPK				0x0004		//�������������ص�����

#define BDATA_TYPE_TOSINOPEC			0x1001		//��ʯ��Ӧ��:����ģ�鷢�͸�ʯ����̨������
#define BDATA_TYPE_FROMSINOPEC		0x1002		//��ʯ��Ӧ��:ʯ����̨���͸�����ģ�������

#define BDATA_TYPE_TOVPCD					0x2001		//��ʯ��Ӧ��:֧��ģ�鷢�͸�VPCD��ͨѶ����
#define BDATA_TYPE_FROMVPCD				0x2002		//��ʯ��Ӧ��:VPCD���͸�֧��ģ���ͨѶ����
#define BDATA_TYPE_TOVPCD_FUN			0x2003		//��ʯ��Ӧ��:֧��ģ�鷢�͸�VPCD�ĺ����ӿ�����	
#define BDATA_TYPE_FROMVPCD_FUN	  0x2004		//��ʯ��Ӧ��:VPCD���͸�֧��ģ��ĺ����ӿ�����


extern int boardSend(char src, char dst, int type, char *inbuffer, int nbytes);
extern void boardTransInit(void);

#endif

