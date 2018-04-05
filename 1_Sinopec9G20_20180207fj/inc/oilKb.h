#ifndef _OIL_KB_H_
#define _OIL_KB_H_

//#include <semLib.h>
//#include <pthread.h>



//宏定义
#define KB_CONTRAY_NO		    0								//不清屏
#define KB_CONTRAY				1								//清屏
#define KB_DSPCLR_NO			0								//显示不清屏
#define KB_DSPCLR				1								//显示清屏
#define KB_FONT16				1								//16*16，16*8点阵字库
#define KB_FONT14				2								//14*14，14*7点阵字库
#define KB_FONT8				3								//8*8点阵字库
#define KB_FONT12				4								//12*12，12*6点阵字库
//#define KB_MSG_MAX_LEN	512							//消息队列最大长度
#define KB_UART_MAX_SIZE		2048
#define KB_RNG_MAX_LEN		    2048						    //串口接收缓存最大长度
#define KB_IO_DEBOUNCE		    5								//键盘消抖次数
#define KB_USARTA1				1								//A1键盘扩展串口1__打印串口
#define KB_USARTA2				2								//A1扩展串口2__IC卡座
#define KB_USARTA3				3								//A1扩展串口3__主板接口
#define KB_USARTA4				4								//A1扩展串口4__条码串口
#define KB_USARTA5				5								//A1扩展串口5__PCD/卡机联动联网串口
#define KB_USARTB1				6								//B1键盘扩展串口1__打印串口
#define KB_USARTB2				7								//B1扩展串口2__IC卡座
#define KB_USARTB3				8								//B1扩展串口3__主板接口
#define KB_USARTB4				9								//B1扩展串口4__条码串口
#define KB_USARTB5				10								//B1扩展串口5__PCD/卡机联动联网串口
#define KB_REFRESH_TIME	(5*ONE_SECOND)		//键盘显示刷新时间间隔

//"9g20"<->"键盘"通讯数据端口定义
#define KB_DATA_HI				0							//高半字节
#define KB_DATA_LOW			    1							//低半字节
#define KB_PORT_SET			    0							//键盘，配置及查询用端口（卡机联接键盘与主板连接串口或CAN口）
#define KB_PORT_DSP			    1							//键盘显示端口
#define KB_PORT_BUTTON	        2							//按键端口
#define KB_PORT_IC				3							//读卡器端口__串口1
#define KB_PORT_PRN			    4							//打印端口__串口2
#define KB_PORT_CODE			5							//键盘条码扫描端口__串口4
#define KB_PORT_NET			    6							//电流环联网端口__串口5
#define KB_PORT_SWITCH	        7							//开关信号及复位状态报告端口

//按键值，公共部分
#define KB_BUTTON_NO			0xffffffff		//无按键值
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
#define KB_BUTTON_CZ			0x0000000a		//凑整/.
#define KB_BUTTON_BACK		    0x0000000b		//退卡
#define KB_BUTTON_ACK			0x0000000c		//确认
//按键值，普通石化机
#define KB_BUTTON_SET			0x0000000d		//设置
#define KB_BUTTON_UP			0x0000000e		//上
#define KB_BUTTON_DOWN		    0x0000000f		//下
#define KB_BUTTON_MON			0x00000010		//定金额
#define KB_BUTTON_VOL			0x00000011		//定升数
#define KB_BUTTON_CHG			0x00000012		//更改
#define KB_BUTTON_SEL			0x00000013		//选择
#define KB_BUTTON_BACK1		    0x00000014		//退卡+1
#define KB_BUTTON_BACK2		    0x00000015		//退卡+2
#define KB_BUTTON_BACK3		    0x00000016		//退卡+3
#define KB_BUTTON_BACK4		    0x00000017		//退卡+4
#define KB_BUTTON_BACK5		    0x00000018		//退卡+5
#define KB_BUTTON_BACK6		    0x00000019		//退卡+6
#define KB_BUTTON_BACK7		    0x0000001a		//退卡+7
#define KB_BUTTON_BACK8		    0x0000001b		//退卡+8
#define KB_BUTTON_BACK9		    0x0000001c		//退卡+9
#define KB_BUTTON_SET1			0x0000001e		//设置+1
#define KB_BUTTON_SET2			0x0000001f		//设置+2
#define KB_BUTTON_SET3			0x00000020		//设置+3
#define KB_BUTTON_SET4			0x00000021		//设置+4
#define KB_BUTTON_SET5			0x00000022		//设置+5
#define KB_BUTTON_SET6			0x00000023		//设置+6
#define KB_BUTTON_SET7			0x00000024		//设置+7
#define KB_BUTTON_SET8			0x00000025		//设置+8
#define KB_BUTTON_SET9			0x00000026		//设置+9
//按键值，石化新自助
#define KB_BUTTON_L1				0x00000020		//侧键左1
#define KB_BUTTON_L2				0x00000021		//侧键左2
#define KB_BUTTON_L3				0x00000022		//侧键左3
#define KB_BUTTON_L4				0x00000023		//侧键左4
#define KB_BUTTON_R1				0x00000024		//侧键右1
#define KB_BUTTON_R2				0x00000025		//侧键右2
#define KB_BUTTON_R3				0x00000026		//侧键右3
#define KB_BUTTON_R4				0x00000027		//侧键右4
#define KB_BUTTON_CLR			    0x00000028		//清除/退出
#define KB_BUTTON_SCORE		        0x00000029		//用积分
#define KB_BUTTON_PRESET		    0x0000002a		//定升/钱数
//油品选择按钮做按键值
#define KB_BUTTON_YP1			0x0000002b		//YP1
#define KB_BUTTON_YP2			0x0000002c		//YP2
#define KB_BUTTON_YP3			0x0000002d		//YP3

//状态定义
#define KB_KEYLOCK_OIL			0						//键盘锁在加油位置
#define KB_KEYLOCK_SET			1						//键盘锁在设置位置


//定义显示缓存数据结构，包括当前显示数据及需要显示的数据标识
typedef struct
{
	unsigned char Buffer[128*8];	//显示缓存
	unsigned char Pointer[8];			//第n行待显示起始下标
	unsigned char Len[8];				//第n行待显示长度

}KbDspStructType;


//操作键盘返回数据
typedef struct
{
	//SEM_ID SemIdSet;			//此信号量用于维护键盘配置端口通讯
	pthread_mutex_t SemIdSet;   //fj:20170911
	char SetBuffer[256];			//查询/设置操作数据缓存
	int SetLen;							//查询/设置操作数据缓存长度
	char SetPack;						//查询/设置操作是否接受到一帧完整数据0=否；1=是

	char DspBuffer[128];			//查询/设置操作数据缓存
	int DspLen;							//查询/设置操作数据缓存长度
	char DspPack;						//查询/设置操作是否接受到一帧完整数据0=否；1=是

	char ButtonBuffer[128];		//按键操作数据缓存
	int ButtonLen;						//按键操作数据缓存长度
	char ButtonPack;					//按键操作是否接受到一帧完整数据0=否；1=是
	unsigned int ButtonLast;		//上一次按键

	char RstStateLast;				//键盘复位前一刻状态
	char RstState;						//键盘复位状态0=未复位完成；1=已复位完成
	char RstStateChg;				//键盘复位状态是否变化；0=无；1=有

	char KeyLockLast;				//LOCK1键盘锁电平前一刻状态
	char KeyLock;						//LOCK1键盘锁电平0=低电平；1=高电平
	char KeyLockChg;				//LOCK1键盘锁电平是否变化；0=无；1=有

	char OilLeftLast;					//YP1左油品选择前一刻状态
	char OilLeft;							//YP1左油品选择；0=低电平；1=高电平
	char OilLeftChg;					//YP1左油品选择是否变化；0=无；1=有

	char OilRightLast;				//YP2右油品选择前一刻状态
	char OilRight;						//YP2右油品选择；0=低电平；1=高电平
	char OilRightChg;					//YP2右油品选择是否变化；0=无；1=有

	char OilConfirmLast;			//YP3油品确认前一刻状态
	char OilConfirm;					//YP3油品确认；0=低电平；1=高电平
	char OilConfirmChg;			//YP3油品确认选择是否变化；0=无；1=有

	char PrinterBusyLast;			//打印机忙闲前一刻状态
	char PrinterBusyState;		//打印机忙闲；0=低电平；1=高电平
	char PrinterBusyChg;			//打印机忙闲；0=无；1=有

	char PIRStateLast;				//感应开关前一刻状态
	char PIRState;					//感应开关；0=低电平；1=高电平
	char PIRStateChg;				//感应开关；0=无；1=有

}KbParamStructType;



//函数外部声明
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
// //油品按钮相关接口
// extern int kbYPUserSet(int DEVx, int user);
// extern int kbYPUserGet(int DEVx, int *user);
extern int kbYPRead(int DEVx, int *value);
// //配置端口相关接口
extern int kbSetPsamTransmit(int KEYx, unsigned char *inbuffer, int nbytes, unsigned char *outbuffer, int *outbytes);

//与键盘之间的操作初始化
extern bool kbInit(void);
//extern void kbExit(void);

void tRxCom4();
void tRxCom5();
void kbTaskSwitchRead();
void tKbDsp(void* kbType);

#endif

