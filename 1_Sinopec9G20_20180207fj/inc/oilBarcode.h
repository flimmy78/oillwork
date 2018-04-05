#ifndef _OIL_BARCODE_H_
#define _OIL_BARCODE_H_

//条码自助授权 后台接收数据最大长度
#define CPOS_MAX_LEN				128

//条码自助授权后台数据发送
extern int CPOSWrite(int nozzle, unsigned char pos_p, unsigned char *buffer, int nbytes);


//无用户
#define BARCODE_USER_NO		0xee

//条码扫描状态
#define BARCODE_IDLE				0			//扫描空闲状态
#define BARCODE_SCAN				1			//扫描状态

//条码模块品牌
#define YUANJD_BRAND				'1'			//远景达二维模块
#define YUANJD_LV1000			    '2'			//远景达一维模块LV1000
#define HONEYWELL_BRAND		        '3'			//霍尼韦尔二维模块
#define HONEYWELL_IS4125		    '4'			//霍尼韦尔一维模块IS4125

//远景达持续时间1秒，间隔超过此时间应再次发送触发命令
#define YUANJD_TIME				(1*ONE_SECOND)

//霍尼韦尔持续扫描时间30秒，间隔超过此时间应再次发送触发命令
#define HONEYWELL_TIME			(3*ONE_SECOND)

//条码扫描设备枪号
#define BARCODE_NOZZLE_1	0			//1号条码设备
#define BARCODE_NOZZLE_2	1			//2号条码设备

//定义接收数据最大长度
#define BAR_RXLEN_MAX			32


extern int barScanModuleInit(int nozzle, unsigned char brand); // 扫描头上电初始化
extern int barBrandWrite(int nozzle, unsigned char brand);     // 扫描头品牌设置
extern int barBrandRead(int nozzle, unsigned char *brand);     // 扫描头品牌获取
extern int barScan(int nozzle, int UID);                       // 开始扫描
extern int barStop(int nozzle, int UID);                       // 停止扫描
extern int barUserIDSet(int nozzle, int id);                   // 设置当前用户ID
extern int barUserIDGet(int nozzle);                           // 获取当前用户ID
extern int barStateRead(int nozzle);                           // 条码扫描状态
extern int barRead(int nozzle, unsigned char *buffer, int maxbytes);// 条码状态读取
extern bool barcodeInit(void);                                 // 条码功能模块初始化

extern void tCPOSRx(void);
extern void tBarcodeScan(int nozzle);
extern void tBarcodeReceive(int nozzle);

#endif

