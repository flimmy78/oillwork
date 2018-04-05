#ifndef _OIL_TERMINAL_DEVICE_H_
#define _OIL_TERMINAL_DEVICE_H_

#define TD_PANEL_1							0			//�ն����ţ�A1��
#define TD_PANEL_2							1			//�ն����ţ�B1��

#define TD_DEVICE_KEYBOARD			       '1'			//�ն��豸���ͣ���������
#define TD_DEVICE_LIANDI				   '2'			//�ն��豸���ͣ�����֧��ģ��


//�ն����ݽṹ
typedef struct
{
	unsigned char Device;			//�豸����:'1' = �������̣�'2' = �����ն�ģ�飻
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

