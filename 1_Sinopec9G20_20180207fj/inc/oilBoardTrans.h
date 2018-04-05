#ifndef _OIL_BOARD_TRANS_H_
#define _OIL_BOARD_TRANS_H_


//#define BDATA_COM_1								COM7		//第一个通讯串口
//#define BDATA_COM_2								COM8		//第二个通讯串口

#define BDATA_COM_1   DEV_COM7
#define BDATA_COM_2   DEV_COM8

#define BDATA_SIZE_MAX							2048	//接收一帧数据最大长度

/*报文数据类型的定义*/
#define BDATA_TYPE_TOPRINTE				0x0001		//发送给打印机的数据			
#define BDATA_TYPE_FROMPRINTE			0x0002		//打印机返回的数据
#define BDATA_TYPE_TOSPK						0x0003	//发送给语音播放器的数据
#define BDATA_TYPE_FROMSPK				0x0004		//语音播放器返回的数据

#define BDATA_TYPE_TOSINOPEC			0x1001		//中石化应用:联网模块发送给石化后台的数据
#define BDATA_TYPE_FROMSINOPEC		0x1002		//中石化应用:石化后台发送给联网模块的数据

#define BDATA_TYPE_TOVPCD					0x2001		//中石油应用:支付模块发送给VPCD的通讯数据
#define BDATA_TYPE_FROMVPCD				0x2002		//中石油应用:VPCD发送给支付模块的通讯数据
#define BDATA_TYPE_TOVPCD_FUN			0x2003		//中石油应用:支付模块发送给VPCD的函数接口数据	
#define BDATA_TYPE_FROMVPCD_FUN	  0x2004		//中石油应用:VPCD发送给支付模块的函数接口数据


extern int boardSend(char src, char dst, int type, char *inbuffer, int nbytes);
extern void boardTransInit(void);

#endif

