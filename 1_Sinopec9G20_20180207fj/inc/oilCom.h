#ifndef _OIL_COM_H_
#define _OIL_COM_H_


//COMx�豸�����б��μ�<ϵͳͼ��>��1��   �����豸ID�б�
#define COM0		DEV_COM0_DEBUG			//COM0(ARM9_DEBUG)	0	���Դ���
#define COM1		DEV_COM1						//COM1(ARM9_UART0)	1	����
#define COM2		DEV_COM2_ICNET			//COM2(ARM9_UART1)	2	������������
#define COM3		DEV_COM3_CODENET		//COM3(ARM9_UART2)	3	��Ȩ����
#define COM4		DEV_COM4_KEYA				//COM4(ARM9_UART3)	4	A����ͨѶ/A֧��ģ��
#define COM5		DEV_COM5_KEYB				//COM5(ARM9_UART4)	5	B����ͨѶ/B֧��ģ��
#define COM6		DEV_COM6_TAX				//COM6(ARM9_UART5)	6	��˰��CPUͨѶ
#define COM7		DEV_COM7						//COM7(ARM7_UART1)	7	����
#define COM8		DEV_COM8						//COM8(ARM7_UART2)	8	����
#define COM9		DEV_COM9_CODEA			//COM9(ARM7_UART3)	9	A������ɨ��
#define COM10	DEV_COM10_CODEB			//COM10(ARM7_UART4)	10	B������ɨ��
#define COM11	DEV_COM11						//COM11(ARM7_UART5)	11	����
#define COM12	DEV_COM12						//COM12(ARM7_UART6)	12	����
#define COM13	DEV_COM13_PRNA			//COM13(A�����_��չ1)	13	A���ӡ
#define COM14	DEV_COM14_ICA				//COM14(A�����_��չ2)	14	A�������
#define COM15	DEV_COM15_CODEA			//COM15(A�����_��չ3)	15	A������ͨѶ
#define COM16	DEV_COM16_NETA			//COM16(A�����_��չ4)	16	A��֧��ģ��/����
#define COM17	DEV_COM17_PRNB			//COM17(B�����_��չ1)	17	B���ӡ
#define COM18	DEV_COM18_ICB				//COM18(B�����_��չ2)	18	B�������
#define COM19	DEV_COM19_CODEB			//COM19(B�����_��չ3)	19	B������ͨѶ
#define COM20	DEV_COM20_NETB			//COM20(B�����_��չ4)	20	B��֧��ģ��/����

//��������λ
#define COM_WORDLENGTH_8B				0
#define COM_WORDLENGTH_9B				1

//ֹͣλ
#define COM_STOPBITS_1                    		0
#define COM_STOPBITS_0_5                		1
#define COM_STOPBITS_2                    		2
#define COM_STOPBITS_1_5                		3

//У��λ
#define COM_PARITY_NO             0		//��У��
#define COM_PARITY_EVEN						1		//żУ��
#define COM_PARITY_ODD						2		//��У��

//��������
#define COM_MAX_LEN		(1024)				  //COM����������󳤶�
#define COM0_PATH			("/tyCo/0")			//����0·��
#define COM1_PATH			("/tyCo/1")			//����1·��
#define COM2_PATH			("/tyCo/2")			//����2·��
#define COM3_PATH			("/tyCo/3")			//����3·��
#define COM4_PATH			("/tyCo/4")			//����4·��
#define COM5_PATH			("/tyCo/5")			//����5·��
#define COM6_PATH			("/tyCo/6")			//����6·��

//�����豸������
extern int gFdCom0, gFdCom1, gFdCom2, gFdCom3, gFdCom4, gFdCom5, gFdCom6;
extern int gFdPcServerCom;

//�����ⲿ����
extern int comWrite(int com_fd, char *buffer, int nbytes);
extern int comRead(int com_fd, char *buffer, int maxbytes);
extern int comWriteInTime(int comfd, char *inbuffer, int nbytes, int overtime);
extern int comSet(int com_fd, int baudrate, int databit, int stopbit, int paritybit);
extern int comGet(int com_fd, int *baudrate, int *databit, int *stopbit, int *paritybit);
extern void comInit(void);
extern void comExit(void);


#endif

