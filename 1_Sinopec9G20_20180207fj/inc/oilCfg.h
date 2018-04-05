#ifndef _OIL_CONFIG_H_
#define _OIL_CONFIG_H_


/*油品代码名称对应表文件路径*/
#define FILE_OILCODE_NAME			"../config/mboardFiles/OilCodeName.txt"

/*项目应包含的所有头文件*/
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


/*应用选择，某些功能通过此条件判断为中油应用或中石化应用*/
#define APP_CNPC						0		/*中石油应用*/
#define APP_SINOPEC				1		/*中石化应用*/


#if APP_SINOPEC

/*是否是260大流量程序，此程序某些操作特殊，税控程序也是专用*/
#define _TYPE_BIG260_			0

#endif	/*end #if APP_SINOPEC*/




#if APP_CNPC

/*消息类型定义*/
#define OIL_MSG_TYPE_VPCD2IPT		1		/*VPCD与IPT间函数通讯接口协议消息类型*/
#define OIL_MSG_TYPE_IPT2VPCD		2		/*IPT与VPCD间通讯接口协议消息类型*/

#endif	/*end #if APP_CNPC*/











/*间隔时间*/

//#define ONE_MILLI_SECOND		(sysClkRateGet()/1000)			/*1毫秒*/  //fj:
//#define ONE_SECOND					(sysClkRateGet())		/*1秒*/


#define ONE_MILLI_SECOND   (1)              //fj:修改
#define ONE_SECOND         (1000) 


/*物理设备ID表，参见<系统图表>表1：   物理设备ID列表*/
#define DEV_COM0_DEBUG		(0	)			//(ARM9_DEBUG)	调试串口
#define DEV_COM1						(1	)			//(ARM9_UART0)	备用
#define DEV_COM2_ICNET			(2	)			//(ARM9_UART1)	卡机联动联网
#define DEV_COM3_CODENET	(3	)			//(ARM9_UART2)	授权联网
#define DEV_COM4_KEYA			(4	)			//(ARM9_UART3)	A键盘通讯/A支付模块
#define DEV_COM5_KEYB			(5	)			//(ARM9_UART4)	B键盘通讯/B支付模块
#define DEV_COM6_TAX				(6	)			//(ARM9_UART5)	与税控CPU通讯
#define DEV_COM7						(7	)			//(ARM7_UART1)	备用
#define DEV_COM8						(8	)			//(ARM7_UART2)	备用
#define DEV_COM9_CODEA		(9	)			//(ARM7_UART3)	A面条码扫描
#define DEV_COM10_CODEB		(10)			//(ARM7_UART4)	A面条码扫描
#define DEV_COM11					(11)			//(ARM7_UART5)	备用
#define DEV_COM12					(12)			//(ARM7_UART6)	备用
#define DEV_COM13_PRNA		(13)			//(A面键盘_扩展1)	A面打印
#define DEV_COM14_ICA			(14)			//(A面键盘_扩展2)	A面读卡器
#define DEV_COM15_CODEA		(15)			//(A面键盘_扩展3)	A面扫描模块通讯
#define DEV_COM16_NETA		(16)			//(A面键盘_扩展4)	A面支付模块/联网
#define DEV_COM17_PRNB		(17)			//(B面键盘_扩展1)	B面打印
#define DEV_COM18_ICB			(18)			//(B面键盘_扩展2)	B面读卡器
#define DEV_COM19_CODEB		(19)			//(B面键盘_扩展3)	B面扫描模块通讯
#define DEV_COM20_NETB			(20)			//(B面键盘_扩展4)	B面支付模块/联网
#define DEV_CAN1						(21)			//(ARM7_CAN1)		主板连接
#define DEV_CAN2						(22)			//(ARM7_CAN2)		外设连接
#define DEV_CAN3						(23)			//( A面键盘_扩展)备用
#define DEV_CAN4						(24)			//( B面键盘_扩展)备用
#define DEV_PWM1						(25)			//A面语音播放
#define DEV_PWM2						(26)			//B面语音播放
#define DEV_NET							(27)			//(网口)TCP/IP联网
#define DEV_USB							(28)			//主USB
#define DEV_SD							(29)			//SD卡
#define DEV_TIMER						(30)			//(ARM7扩展)实时时钟
#define DEV_BUTTON_KEYA		(31)			//(A面键盘_扩展)A面按键
#define DEV_BUTTON_KEYB		(32)			//(B面键盘_扩展)B面按键
#define DEV_DSP_KEYA				(33)			//( A面键盘_扩展)A面键盘显示
#define DEV_DSP_KEYB				(34)			//( B面键盘_扩展)B面键盘显示
#define DEV_PULSEA1				(35)			//(A1编码器)加油脉冲采集
#define DEV_PULSEA2				(36)			//(A2编码器)加油脉冲采集
#define DEV_PULSEB1				(37)			//(B1编码器)加油脉冲采集
#define DEV_PULSEB2				(38)			//(B2编码器)加油脉冲采集
#define DEV_SET1						(39)			//主板号配置，跳线1(ARM7扩展)，IO采集
#define DEV_SET2						(40)			//主板号配置，跳线2(ARM7扩展)，IO采集
#define DEV_SET3						(41)			//主板号配置，跳线3(ARM7扩展)，IO采集
#define DEV_SWITCH_LOCKA	(42)			//A面键盘锁（ARM7扩展）,IO采集
#define DEV_SWITCH_LOCKB	(43)			//B面键盘锁（ARM7扩展）,IO采集
#define DEV_LOCKBOARD			(44)			//主板键盘锁,IO采集
#define DEV_POWER					(45)			//电源状态，IO采集
#define DEV_GUNA1					(46)			//A面1枪（ARM7扩展）,IO采集
#define DEV_GUNB1					(47)			//B面1枪（ARM7扩展）,IO采集
#define DEV_GUNA2					(48)			//A面2枪（ARM7扩展）,IO采集
#define DEV_GUNB2					(49)			//B面2枪（ARM7扩展）,IO采集
#define DEV_PUMP_PERMITA	(50)			//A面启泵允许信号（ARM7扩展）,IO采集
#define DEV_PUMP_PERMITB	(51)			//B面启泵允许信号（ARM7扩展）,IO采集
#define DEV_SWITCH_SELA1		(52)			//A面油品选择1( A面键盘_扩展),IO采集
#define DEV_SWITCH_SELA2		(53)			//A面油品选择2( A面键盘_扩展),IO采集
#define DEV_SWITCH_SELA3		(54)			//A面油品选择3( A面键盘_扩展),IO采集
#define DEV_SWITCH_SELB1		(55)			//B面油品选择1( A面键盘_扩展),IO采集
#define DEV_SWITCH_SELB2		(56)			//B面油品选择2( A面键盘_扩展),IO采集
#define DEV_SWITCH_SELB3		(57)			//B面油品选择3( A面键盘_扩展),IO采集
#define DEV_GUN5						(58)			//备用开关信号（ARM7扩展），IO采集
#define DEV_GUN6						(59)			//备用开关信号（ARM7扩展），IO采集
#define DEV_GUN7						(60)			//备用开关信号（ARM7扩展），IO采集
#define DEV_GUN8						(61)			//备用开关信号（ARM7扩展），IO采集
#define DEV_GUN9						(62)			//备用开关信号（ARM7扩展），IO采集
#define DEV_GUN10					(63)			//备用开关信号（ARM7扩展），IO采集
#define DEV_TAX_RESET			(64)			//税控复位，IO输出
#define DEV_LIGHT						(65)			//背光控制，IO输出
#define DEV_TAX_DOWN			(66)			//通知税控掉电，IO输出
#define DEV_MOTORA1				(67)			//电机驱动A1，IO输出
#define DEV_MOTORB1				(68)			//电机驱动B1，IO输出
#define DEV_MOTORA2				(69)			//电机驱动A2，IO输出
#define DEV_MOTORB2				(70)			//电机驱动B2，IO输出
#define DEV_VALVE_AM				(71)			//电磁阀驱动，A面1/2阀大阀，IO输出
#define DEV_VALVE_AL				(72)			//电磁阀驱动，A面1/2阀小阀，IO输出
#define DEV_VALVE_AS				(73)			//电磁阀驱动，A面1/2阀选择，IO输出
#define DEV_VALVE_BM				(74)			//电磁阀驱动，B面1/2阀大阀，IO输出
#define DEV_VALVE_BL				(75)			//电磁阀驱动，B面1/2阀小阀，IO输出
#define DEV_VALVE_BS				(76)			//电磁阀驱动，B面1/2阀选择，IO输出
#define DEV_ENCODE_SIGA		(77)			//编码器有效信号,A面编码器有效信号（ARM7扩展）,IO输出	
#define DEV_ENCODE_SIGB		(78)			//编码器有效信号,A面编码器有效信号（ARM7扩展）,IO输出


/*自定义库函数*/
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



