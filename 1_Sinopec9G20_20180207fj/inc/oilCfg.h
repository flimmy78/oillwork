#ifndef _OIL_CONFIG_H_
#define _OIL_CONFIG_H_


/*��Ʒ�������ƶ�Ӧ���ļ�·��*/
#define FILE_OILCODE_NAME			"../config/mboardFiles/OilCodeName.txt"

/*��ĿӦ����������ͷ�ļ�*/
//#include <vxWorks.h>
//#include <sysLib.h>
//#include <wdLib.h>
//#include <stdLib.h>
//#include <iv.h>
//#include <lstLib.h>
//#include <rngLib.h>
//#include <assert.h>
//#include <memLib.h>
//#include <taskLib.h>
//#include <string.h>
//#include <errnoLib.h>
//#include <timers.h>
//#include <unistd.h>
//#include <sigLib.h>
//#include <stdio.h>
//#include <msgQLib.h>


/*Ӧ��ѡ��ĳЩ����ͨ���������ж�Ϊ����Ӧ�û���ʯ��Ӧ��*/
#define APP_CNPC						0		/*��ʯ��Ӧ��*/
#define APP_SINOPEC				1		/*��ʯ��Ӧ��*/


#if APP_SINOPEC

/*�Ƿ���260���������򣬴˳���ĳЩ�������⣬˰�س���Ҳ��ר��*/
#define _TYPE_BIG260_			0

#endif	/*end #if APP_SINOPEC*/




#if APP_CNPC

/*��Ϣ���Ͷ���*/
#define OIL_MSG_TYPE_VPCD2IPT		1		/*VPCD��IPT�亯��ͨѶ�ӿ�Э����Ϣ����*/
#define OIL_MSG_TYPE_IPT2VPCD		2		/*IPT��VPCD��ͨѶ�ӿ�Э����Ϣ����*/

#endif	/*end #if APP_CNPC*/











/*���ʱ��*/

//#define ONE_MILLI_SECOND		(sysClkRateGet()/1000)			/*1����*/  //fj:
//#define ONE_SECOND					(sysClkRateGet())		/*1��*/


#define ONE_MILLI_SECOND   (1)              //fj:�޸�
#define ONE_SECOND         (1000) 


/*�����豸ID���μ�<ϵͳͼ��>��1��   �����豸ID�б�*/
#define DEV_COM0_DEBUG		(0	)			//(ARM9_DEBUG)	���Դ���
#define DEV_COM1						(1	)			//(ARM9_UART0)	����
#define DEV_COM2_ICNET			(2	)			//(ARM9_UART1)	������������
#define DEV_COM3_CODENET	(3	)			//(ARM9_UART2)	��Ȩ����
#define DEV_COM4_KEYA			(4	)			//(ARM9_UART3)	A����ͨѶ/A֧��ģ��
#define DEV_COM5_KEYB			(5	)			//(ARM9_UART4)	B����ͨѶ/B֧��ģ��
#define DEV_COM6_TAX				(6	)			//(ARM9_UART5)	��˰��CPUͨѶ
#define DEV_COM7						(7	)			//(ARM7_UART1)	����
#define DEV_COM8						(8	)			//(ARM7_UART2)	����
#define DEV_COM9_CODEA		(9	)			//(ARM7_UART3)	A������ɨ��
#define DEV_COM10_CODEB		(10)			//(ARM7_UART4)	A������ɨ��
#define DEV_COM11					(11)			//(ARM7_UART5)	����
#define DEV_COM12					(12)			//(ARM7_UART6)	����
#define DEV_COM13_PRNA		(13)			//(A�����_��չ1)	A���ӡ
#define DEV_COM14_ICA			(14)			//(A�����_��չ2)	A�������
#define DEV_COM15_CODEA		(15)			//(A�����_��չ3)	A��ɨ��ģ��ͨѶ
#define DEV_COM16_NETA		(16)			//(A�����_��չ4)	A��֧��ģ��/����
#define DEV_COM17_PRNB		(17)			//(B�����_��չ1)	B���ӡ
#define DEV_COM18_ICB			(18)			//(B�����_��չ2)	B�������
#define DEV_COM19_CODEB		(19)			//(B�����_��չ3)	B��ɨ��ģ��ͨѶ
#define DEV_COM20_NETB			(20)			//(B�����_��չ4)	B��֧��ģ��/����
#define DEV_CAN1						(21)			//(ARM7_CAN1)		��������
#define DEV_CAN2						(22)			//(ARM7_CAN2)		��������
#define DEV_CAN3						(23)			//( A�����_��չ)����
#define DEV_CAN4						(24)			//( B�����_��չ)����
#define DEV_PWM1						(25)			//A����������
#define DEV_PWM2						(26)			//B����������
#define DEV_NET							(27)			//(����)TCP/IP����
#define DEV_USB							(28)			//��USB
#define DEV_SD							(29)			//SD��
#define DEV_TIMER						(30)			//(ARM7��չ)ʵʱʱ��
#define DEV_BUTTON_KEYA		(31)			//(A�����_��չ)A�水��
#define DEV_BUTTON_KEYB		(32)			//(B�����_��չ)B�水��
#define DEV_DSP_KEYA				(33)			//( A�����_��չ)A�������ʾ
#define DEV_DSP_KEYB				(34)			//( B�����_��չ)B�������ʾ
#define DEV_PULSEA1				(35)			//(A1������)��������ɼ�
#define DEV_PULSEA2				(36)			//(A2������)��������ɼ�
#define DEV_PULSEB1				(37)			//(B1������)��������ɼ�
#define DEV_PULSEB2				(38)			//(B2������)��������ɼ�
#define DEV_SET1						(39)			//��������ã�����1(ARM7��չ)��IO�ɼ�
#define DEV_SET2						(40)			//��������ã�����2(ARM7��չ)��IO�ɼ�
#define DEV_SET3						(41)			//��������ã�����3(ARM7��չ)��IO�ɼ�
#define DEV_SWITCH_LOCKA	(42)			//A���������ARM7��չ��,IO�ɼ�
#define DEV_SWITCH_LOCKB	(43)			//B���������ARM7��չ��,IO�ɼ�
#define DEV_LOCKBOARD			(44)			//���������,IO�ɼ�
#define DEV_POWER					(45)			//��Դ״̬��IO�ɼ�
#define DEV_GUNA1					(46)			//A��1ǹ��ARM7��չ��,IO�ɼ�
#define DEV_GUNB1					(47)			//B��1ǹ��ARM7��չ��,IO�ɼ�
#define DEV_GUNA2					(48)			//A��2ǹ��ARM7��չ��,IO�ɼ�
#define DEV_GUNB2					(49)			//B��2ǹ��ARM7��չ��,IO�ɼ�
#define DEV_PUMP_PERMITA	(50)			//A�����������źţ�ARM7��չ��,IO�ɼ�
#define DEV_PUMP_PERMITB	(51)			//B�����������źţ�ARM7��չ��,IO�ɼ�
#define DEV_SWITCH_SELA1		(52)			//A����Ʒѡ��1( A�����_��չ),IO�ɼ�
#define DEV_SWITCH_SELA2		(53)			//A����Ʒѡ��2( A�����_��չ),IO�ɼ�
#define DEV_SWITCH_SELA3		(54)			//A����Ʒѡ��3( A�����_��չ),IO�ɼ�
#define DEV_SWITCH_SELB1		(55)			//B����Ʒѡ��1( A�����_��չ),IO�ɼ�
#define DEV_SWITCH_SELB2		(56)			//B����Ʒѡ��2( A�����_��չ),IO�ɼ�
#define DEV_SWITCH_SELB3		(57)			//B����Ʒѡ��3( A�����_��չ),IO�ɼ�
#define DEV_GUN5						(58)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_GUN6						(59)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_GUN7						(60)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_GUN8						(61)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_GUN9						(62)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_GUN10					(63)			//���ÿ����źţ�ARM7��չ����IO�ɼ�
#define DEV_TAX_RESET			(64)			//˰�ظ�λ��IO���
#define DEV_LIGHT						(65)			//������ƣ�IO���
#define DEV_TAX_DOWN			(66)			//֪ͨ˰�ص��磬IO���
#define DEV_MOTORA1				(67)			//�������A1��IO���
#define DEV_MOTORB1				(68)			//�������B1��IO���
#define DEV_MOTORA2				(69)			//�������A2��IO���
#define DEV_MOTORB2				(70)			//�������B2��IO���
#define DEV_VALVE_AM				(71)			//��ŷ�������A��1/2���󷧣�IO���
#define DEV_VALVE_AL				(72)			//��ŷ�������A��1/2��С����IO���
#define DEV_VALVE_AS				(73)			//��ŷ�������A��1/2��ѡ��IO���
#define DEV_VALVE_BM				(74)			//��ŷ�������B��1/2���󷧣�IO���
#define DEV_VALVE_BL				(75)			//��ŷ�������B��1/2��С����IO���
#define DEV_VALVE_BS				(76)			//��ŷ�������B��1/2��ѡ��IO���
#define DEV_ENCODE_SIGA		(77)			//��������Ч�ź�,A���������Ч�źţ�ARM7��չ��,IO���	
#define DEV_ENCODE_SIGB		(78)			//��������Ч�ź�,A���������Ч�źţ�ARM7��չ��,IO���


/*�Զ���⺯��*/
extern char *myItoa(int num, char*str, int radix);
extern int myNetDotAddrCheck(const char *addr);
extern int bcdSum(const unsigned char *in_buffer1, int in_nbytes1, const unsigned char *in_buffer2, int in_nbytes2, unsigned char *out_buffer, int out_maxbytes);
extern unsigned char xorGet(unsigned char *buffer, int nbytes);
extern unsigned int crc16Get(unsigned char *buffer, int nbytes);
extern long long hex2Bcd(unsigned int hex_data);
extern int hexbcd2int(int nData); //fj:
extern long long hexbcd2longlong(unsigned long long llData);
extern int hex2Ascii(const unsigned char *hex_buffer, int hex_nbytes, unsigned char *ascii_buffer, int ascii_maxbytes);
extern int timeVerification(const unsigned char *buffer, int nbytes);

extern bool JudgeFFAndCrcProtocol(unsigned char* puchRecvData,int nRecvLen,unsigned char* puchReturnData,int* pnReturnLen,int* pnDeleteLen);

#endif



