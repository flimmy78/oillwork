#ifndef _OIL_AUTHORIZE_H_
#define _OIL_AUTHORIZE_H_

//授权方式
#define AUTH_MODEL_ETC				0x00	//ETC
#define AUTH_MODEL_WECHAT		0x01	//微信
#define AUTH_MODEL_ALIPAY		0x02	//支付宝

//扣款来源
#define AUTH_DS_ETC						0x91	//ETC
#define AUTH_DS_WECHAT				0x92	//微信
#define AUTH_DS_ALIPAY				0x93	//支付宝


//授权相关数据
typedef struct
{
	unsigned char Model;								//授权方式00H=ETC; 01H=微信; 02H=支付宝
	unsigned char Unit;								//授权单位 0=元；1=升
	unsigned int Amount;								//授权额 HEX
	unsigned char AntID;								//ETC授权，天线ID AntID
	unsigned char OBUID[4];						//ETC授权，标签MAC号 OBUID
	unsigned char ContractNo[8];				//ETC授权，OBU合同序列号 ContractNo
	unsigned char OBUPlate[12];					//ETC授权，车牌号 OBUPlate
	unsigned char CardID[10];						//ETC授权，ETC卡号

}AuthorizeDataType;




//接口
extern int authorizeWrite(int nozzle, const char *inbuffer);
extern void authProccess(int nozzle, char *inbuffer, int nbytes);

#endif

