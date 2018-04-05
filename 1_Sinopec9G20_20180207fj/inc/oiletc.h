#ifndef _OIL_ETC_H_
#define _OIL_ETC_H_

#define ETC_05_H_LEN				6			//05命令头长度

#define ETC_DATA_MAX				600			//最大数据长度
#define ETC_CARD_MAX				1024		//卡信息最大缓存
#define ETCCARDLEN				  26			//车辆信息长度	
#define ETC_SEND_TIMES			3			  //ETC发送次数限制

#define ETC_FUN_NO				30			  //禁止ETC功能	
#define ETC_FUN_OK				31		  	//允许ETC功能	

#define ETC_CMD					0x80							//主命令
#define ETC_01					0x01							//油机读取车辆列表信息（油机发起）
#define ETC_02					0x02							//油机选定车号（油机发起）
#define ETC_03					0x03							//油机读取OBU信息（油机发起）
#define ETC_04					0x04							//IC卡上电（油机发起）
#define ETC_05					0x05							//IC卡操作（油机发起）
#define ETC_06					0x06							//IC卡下电（油机发起）
#define ETC_07					0x07							//释放OBU（油机发起）
#define ETC_08					0x08							//IC卡密码校验
#define ETC_09					0x09							//一次性读取卡信息
#define ETC_0A					0x0A							//保存油品号
#define ETC_0B					0x0B							//通知OBU余额


#define ETC_PID_CARINF_READ		0x00000801					//ETC读取车辆信息
#define ETC_PID_SEL_PIN			0x00000802					//ETC输入验证码
#define ETC_PID_TO_OBU			0x00000803					//ETC选定OBU
#define ETC_PID_INF_READ	    0x00000804					//ETC一次性读取卡信息
#define ETC_PID_OIL_SURE	    0x00000805					//油品不一致需要确认
#define ETC_PID_PIN_CHECK		0x00000806					//ETC验密


extern void etc_fun_process(unsigned char ID);//ETC功能处理
void EtcPidCarinfRead(unsigned char ID);      //ETC读取车辆信息
void EtcPinSelPin(unsigned char ID);          //ETC输入验证码
void EtcPidToObu(unsigned char ID);           //ETC选定OBU
void EtcPidInfRead(unsigned char ID);         //ETC一次性读取卡信息
char EtcCardInfJieXi(unsigned char ID);       //卡信息解析
void EtcCardCarHandle(unsigned char ID);      //车牌号处理
void EtcOilNameCourse(unsigned char ID);      //油品换算显示函数
void EtcCardYuehanlde(unsigned char ID);      //ETC余额处理
void EtcPidOilSure(unsigned char ID);         //油品不一致需要确认
void EtcUpdateGrade(unsigned char ID);        //更新油品号
void EtcYueDisHandle(unsigned char ID);       //显示余额
void EtcCardObuPin(unsigned char ID);         //OBU验密
extern void EtcFreeObuCourse(unsigned char ID);//释放OBU



#endif




