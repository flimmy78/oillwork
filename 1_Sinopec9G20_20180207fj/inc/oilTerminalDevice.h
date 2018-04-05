#ifndef _OIL_TERMINAL_DEVICE_H_
#define _OIL_TERMINAL_DEVICE_H_

#define TD_PANEL_1							0			//终端面板号，A1面
#define TD_PANEL_2							1			//终端面板号，B1面

#define TD_DEVICE_KEYBOARD			       '1'			//终端设备类型，金属键盘
#define TD_DEVICE_LIANDI				   '2'			//终端设备类型，联迪支付模块


//终端数据结构
typedef struct
{
	unsigned char Device;			//设备类型:'1' = 金属键盘；'2' = 联迪终端模块；
}TdStructParamType;

extern TdStructParamType tdStructA1,tdStructB1;

extern int tdDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes);
extern int tdDsp(int DEV_DSP_KEYx, int Contrast, int IsClr);
extern unsigned int tdButtonRead(int kb_dev_id);
extern int tdYPRead(int DEVx, int *value);
extern int tdDeviceGet(int nozzle);
extern int tdBankPayment(int nozzle, const void *pSend, unsigned int nSendLen, void *pRec, int *pnRecLen);
extern bool tdInit(void);

#endif

