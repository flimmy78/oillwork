#ifndef _OIL_LIANDI_H_
#define _OIL_LIANDI_H_

//#include <msgQLib.h>

#include "lstLib.h"

#define LD_BUFFER_SIZE				512			    //终端数据缓存大小
#define LD_ICDEV_USERCARD			0x00			//外接卡座
#define LD_ICDEV_SAM1CARD			0x01			//SAM1卡座
#define LD_ICDEV_SAM2CARD			0x02			//SAM2卡座
#define LD_ICDEV_SAM3CARD			0x03			//SAM3卡座

#define LD_CMD_SHAKE_HANDS		    0x0001		//主板发送发送握手命令
#define LD_CMD_INTERFACE_DO		    0x0002		//主板发送发送执行对应的接口函数
#define LD_CMD_DSP				    0x0003		//主板发送发送执行显示命令
#define LD_CMD_2					0x0004		//读取IO状态
#define LD_CMD_4					0x0005		//配置串口
#define LD_CMD_3					0x0006		//读取串口配置
#define LD_CMD_5					0x0007		//主板通过联迪模块串口转发数据
#define LD_CMD_6					0x0008		//联迪串口数据转发给主板
#define LD_CMD_ICMODULE_INIT		0x0010		//加油卡设备初始化
#define LD_CMD_ICDEVICE_OPEN		0x0011		//打开加油卡设备
#define LD_CMD_ICDEVICE_CLOSE	    0x0012		//关闭加油卡设备
#define LD_CMD_IC_POWER_UP		    0x0013		//加油卡设备上电
#define LD_CMD_IC_POWER_DOWN	    0x0014		//加油卡设备下电
#define LD_CMD_IC_APDU				0x0015		//加油卡设备APDU通讯
#define LD_CMD_ESEJECT_CARD		    0x0016		//加油卡退卡
#define LD_CMD_CARD_POLL		    0x0017		//寻卡
#define LD_CMD_BANK_JYJFREE		    0x0020		//加油机空闲
#define LD_CMD_BANK_PAYMENT		    0x0021		//银行卡交易数据交互
#define LD_CMD_BUTTON			    0x1001		//联迪终端上传按键值

//按键节点
typedef struct
{
	NODE Ndptrs;				//节点指针
	unsigned int Value; 	//按键

}LDButtonNodeStruct;

typedef struct
{
	unsigned char BitMap;				//位图b0~b7分别代表第0~7行，=0无此域，=1有此域
	unsigned char Font[8];				//各行显示字库类型=0		16*8,16*16字库 
       										            //=1    	14*7,14*14字库 
	   												    //=2    	8*8字库
	   												    //=3    	12*6,12*12字库
	unsigned char IsContrary[8];	    //各行是否反显0=正常显示；1=该行反显
	unsigned char OffsetY[8];			//行内偏移即坐标
	unsigned char Buffer[8][32+1];		//显示数据，8行，各行最多32字节
	unsigned char Nbytes[8];			//显示数据长度

}LDDspContentType;


typedef struct
{
	int ComX;								//主板与终端设备通讯的串口号
	unsigned char Device;			        //设备类型:'1' = 金属键盘；'2' = 联迪终端模块；
	unsigned char SendFrame;	//发送数据的帧号
	unsigned char AckBuffer[LD_BUFFER_SIZE + 1];		//接收的油机主动命令应答数据
	int AckLenght;															//接收的油机主动命令应答数据长度
	int Contrast;							//对比度
	LDDspContentType Content;	            //显示数据
	LIST ButtonList;						//按键链表
	unsigned char DeckStateS1;			    //S1卡座状态=0x30表示有卡,0x31表示无卡。
	unsigned char IcTypeS2;					//S2:卡类型
																		//S2=0x30 卡机内无卡
																		//S2=0x3f 无法识别
																		//S2=0x31 触点cpu卡
																		//S2=0x32 RF-- TYPE B CPU卡
																		//S2=0x33 RF-TYPE A CPU卡
																		//S2=0x34 RF-M1卡
	unsigned char IcStateS3;							                //S3：卡状态
																		//S3=0x30 下电状态
																		//S3=0x31 休眠状态
																		//S4=0x32 激活状态
																		//S5=0x33 忙态（通信状态）
	
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

