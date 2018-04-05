#ifndef _OIL_BARCODE_H_
#define _OIL_BARCODE_H_

//����������Ȩ ��̨����������󳤶�
#define CPOS_MAX_LEN				128

//����������Ȩ��̨���ݷ���
extern int CPOSWrite(int nozzle, unsigned char pos_p, unsigned char *buffer, int nbytes);


//���û�
#define BARCODE_USER_NO		0xee

//����ɨ��״̬
#define BARCODE_IDLE				0			//ɨ�����״̬
#define BARCODE_SCAN				1			//ɨ��״̬

//����ģ��Ʒ��
#define YUANJD_BRAND				'1'			//Զ�����άģ��
#define YUANJD_LV1000			    '2'			//Զ����һάģ��LV1000
#define HONEYWELL_BRAND		        '3'			//����Τ����άģ��
#define HONEYWELL_IS4125		    '4'			//����Τ��һάģ��IS4125

//Զ�������ʱ��1�룬���������ʱ��Ӧ�ٴη��ʹ�������
#define YUANJD_TIME				(1*ONE_SECOND)

//����Τ������ɨ��ʱ��30�룬���������ʱ��Ӧ�ٴη��ʹ�������
#define HONEYWELL_TIME			(3*ONE_SECOND)

//����ɨ���豸ǹ��
#define BARCODE_NOZZLE_1	0			//1�������豸
#define BARCODE_NOZZLE_2	1			//2�������豸

//�������������󳤶�
#define BAR_RXLEN_MAX			32


extern int barScanModuleInit(int nozzle, unsigned char brand); // ɨ��ͷ�ϵ��ʼ��
extern int barBrandWrite(int nozzle, unsigned char brand);     // ɨ��ͷƷ������
extern int barBrandRead(int nozzle, unsigned char *brand);     // ɨ��ͷƷ�ƻ�ȡ
extern int barScan(int nozzle, int UID);                       // ��ʼɨ��
extern int barStop(int nozzle, int UID);                       // ֹͣɨ��
extern int barUserIDSet(int nozzle, int id);                   // ���õ�ǰ�û�ID
extern int barUserIDGet(int nozzle);                           // ��ȡ��ǰ�û�ID
extern int barStateRead(int nozzle);                           // ����ɨ��״̬
extern int barRead(int nozzle, unsigned char *buffer, int maxbytes);// ����״̬��ȡ
extern bool barcodeInit(void);                                 // ���빦��ģ���ʼ��

extern void tCPOSRx(void);
extern void tBarcodeScan(int nozzle);
extern void tBarcodeReceive(int nozzle);

#endif

