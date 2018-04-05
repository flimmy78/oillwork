#ifndef _OIL_COM_H_
#define _OIL_COM_H_


//COMx设备符号列表，参见<系统图表>表1：   物理设备ID列表
#define COM0		DEV_COM0_DEBUG			//COM0(ARM9_DEBUG)	0	调试串口
#define COM1		DEV_COM1						//COM1(ARM9_UART0)	1	备用
#define COM2		DEV_COM2_ICNET			//COM2(ARM9_UART1)	2	卡机联动联网
#define COM3		DEV_COM3_CODENET		//COM3(ARM9_UART2)	3	授权联网
#define COM4		DEV_COM4_KEYA				//COM4(ARM9_UART3)	4	A键盘通讯/A支付模块
#define COM5		DEV_COM5_KEYB				//COM5(ARM9_UART4)	5	B键盘通讯/B支付模块
#define COM6		DEV_COM6_TAX				//COM6(ARM9_UART5)	6	与税控CPU通讯
#define COM7		DEV_COM7						//COM7(ARM7_UART1)	7	备用
#define COM8		DEV_COM8						//COM8(ARM7_UART2)	8	备用
#define COM9		DEV_COM9_CODEA			//COM9(ARM7_UART3)	9	A面条码扫描
#define COM10	DEV_COM10_CODEB			//COM10(ARM7_UART4)	10	B面条码扫描
#define COM11	DEV_COM11						//COM11(ARM7_UART5)	11	备用
#define COM12	DEV_COM12						//COM12(ARM7_UART6)	12	备用
#define COM13	DEV_COM13_PRNA			//COM13(A面键盘_扩展1)	13	A面打印
#define COM14	DEV_COM14_ICA				//COM14(A面键盘_扩展2)	14	A面读卡器
#define COM15	DEV_COM15_CODEA			//COM15(A面键盘_扩展3)	15	A面条码通讯
#define COM16	DEV_COM16_NETA			//COM16(A面键盘_扩展4)	16	A面支付模块/联网
#define COM17	DEV_COM17_PRNB			//COM17(B面键盘_扩展1)	17	B面打印
#define COM18	DEV_COM18_ICB				//COM18(B面键盘_扩展2)	18	B面读卡器
#define COM19	DEV_COM19_CODEB			//COM19(B面键盘_扩展3)	19	B面条码通讯
#define COM20	DEV_COM20_NETB			//COM20(B面键盘_扩展4)	20	B面支付模块/联网

//串口数据位
#define COM_WORDLENGTH_8B				0
#define COM_WORDLENGTH_9B				1

//停止位
#define COM_STOPBITS_1                    		0
#define COM_STOPBITS_0_5                		1
#define COM_STOPBITS_2                    		2
#define COM_STOPBITS_1_5                		3

//校验位
#define COM_PARITY_NO             0		//无校验
#define COM_PARITY_EVEN						1		//偶校验
#define COM_PARITY_ODD						2		//奇校验

//其它定义
#define COM_MAX_LEN		(1024)				  //COM串口数据最大长度
#define COM0_PATH			("/tyCo/0")			//串口0路径
#define COM1_PATH			("/tyCo/1")			//串口1路径
#define COM2_PATH			("/tyCo/2")			//串口2路径
#define COM3_PATH			("/tyCo/3")			//串口3路径
#define COM4_PATH			("/tyCo/4")			//串口4路径
#define COM5_PATH			("/tyCo/5")			//串口5路径
#define COM6_PATH			("/tyCo/6")			//串口6路径

//串口设备描述符
extern int gFdCom0, gFdCom1, gFdCom2, gFdCom3, gFdCom4, gFdCom5, gFdCom6;
extern int gFdPcServerCom;

//函数外部声明
extern int comWrite(int com_fd, char *buffer, int nbytes);
extern int comRead(int com_fd, char *buffer, int maxbytes);
extern int comWriteInTime(int comfd, char *inbuffer, int nbytes, int overtime);
extern int comSet(int com_fd, int baudrate, int databit, int stopbit, int paritybit);
extern int comGet(int com_fd, int *baudrate, int *databit, int *stopbit, int *paritybit);
extern void comInit(void);
extern void comExit(void);


#endif

