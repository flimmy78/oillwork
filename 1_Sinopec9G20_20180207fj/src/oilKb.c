#include "../inc/main.h"
 
//��չCOM���ݽ��ջ��λ���
 RING_ID RngRxUsartA1=NULL, RngRxUsartA2=NULL, RngRxUsartA4=NULL, RngRxUsartA5=NULL;
 RING_ID RngRxUsartB1=NULL, RngRxUsartB2=NULL, RngRxUsartB4=NULL, RngRxUsartB5=NULL;

pthread_rwlock_t rwlock_write = PTHREAD_RWLOCK_INITIALIZER;
 
//ĸ���ڽ��ղ��������Ӵ������ݵ�����ID
 int TaskIdRxCom4=0, TaskIdRxCom5=0;
 
//��ʱ��ȡ����IO״̬������IsContrarystatic int TaskIdIORead=0;
int TaskIdIORead=0;
//���̷��ص����ݼ�ĳЩ״̬
KbParamStructType  KbParamA1, KbParamB1;
 
//��ʾ�˿ڷ���������Ϣ����
int msgIdDspA1;
int msgIdDspB1;
 
//��ʾ�˿ڷ��������ź���
pthread_mutex_t  semIdDspA1=PTHREAD_MUTEX_INITIALIZER;                                                                                                                                                                            
pthread_mutex_t  semIdDspB1=PTHREAD_MUTEX_INITIALIZER;
 

//�����ڵ�
typedef struct
{
 	NODE Ndptrs;				//�ڵ�ָ��
 	unsigned int Value; 		//����
 }KbButtonNodeStruct;
 
LIST KbButtonListA1, KbButtonListB1;//��������
KbDspStructType KbDspA1, KbDspB1;//������ʾ����
 
//��ť���ݽṹ
struct kbSwitchStruct
{
 	int ValueYPA1;
 	int UserYPA1;
 
 	int ValueYPA2;
 	int UserYPA2;
 
 	int ValueYPA3;
 	int UserYPA3;
 
 	int ValueYPB1;
 	int UserYPB1;
 
 	int ValueYPB2;
 	int UserYPB2;
 
 	int ValueYPB3;
 	int UserYPB3;
 
 	int ValueLockA;
 	int UserLockA;
 
 	int ValueLockB;
 	int UserLockB;
 	
}KbSwitch;
 
//����ֵ��ȡ
int TaskIdSwitchRead=0;
 
//��ʾ���ݻ���
typedef struct
{
    unsigned char BitMap;				//λͼb0~b7�ֱ�����0~7�У�=0�޴���=1�д���
	unsigned char Font[8];				//������ʾ�ֿ�����=0		16*8,16*16�ֿ� 
//        												=1    	14*7,14*14�ֿ� 
// 	   													=2    	8*8�ֿ�
// 			    										=3    	12*6,12*12�ֿ�
	unsigned char Contrast;				//��ʾ�Աȶ�
	unsigned char IsContrary[8];	    //�����Ƿ���0=������ʾ��1=���з��� 	unsigned char OffsetY[8];			//����ƫ�Ƽ�����
    unsigned char OffsetY[8];           //����ƫ�Ƽ�����
	unsigned char Buffer[8][32+1];		//��ʾ���ݣ�8�У��������32�ֽ�
 	unsigned char Nbytes[8];			//��ʾ���ݳ���
}DspContentType;
 
DspContentType DspContentA, DspContentB;
 
int kbDspSend(int DEV_DSP_KEYx, int ackWait, char *inbuffer, int nbytes, char *outbuffer, int maxbytes);
int kbSetTransmit(int KEYx, unsigned char *inbuffer, int nbytes, unsigned char *outbuffer, int *outbytes);
// 
// //*******************************************************************
// *Name				:kbSetPars
// *Description		:�������ݽ�������������ִ�����ò����ķ������
// *Input				:kb_id	�豸ѡ��	0=A1����	1=B1����
// *						:data	���̷��ص�����
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
// 
static void kbSetPars(int kb_id, char data)
{
 	KbParamStructType *kbparam=NULL;
 
 	if(0==kb_id)			kbparam=&KbParamA1;
 	else if(1==kb_id)	kbparam=&KbParamB1;
 	else							return;
 
 	if(0==kbparam->SetPack){
 
 		//��ֹ���
 		if(kbparam->SetLen>=sizeof(kbparam->SetBuffer)){
 	
 			kbparam->SetPack=0;	kbparam->SetLen=0;	
 		}
 
 		//��������
 		kbparam->SetBuffer[kbparam->SetLen]=data;
 		if(0==kbparam->SetLen && 0xf9==kbparam->SetBuffer[kbparam->SetLen])	kbparam->SetLen++;
 		else	if(0!=kbparam->SetLen)																			kbparam->SetLen++;
 
 		//����һ֡����
 		if(kbparam->SetLen>=4 && kbparam->SetLen>=(kbparam->SetBuffer[1]+2)){
 			
 			if(kbparam->SetBuffer[kbparam->SetLen-1]==xorGet(&kbparam->SetBuffer[1], kbparam->SetLen-2)){
 				kbparam->SetPack=1;
 			}
 			else{
 				kbparam->SetPack=0;	kbparam->SetLen=0;
 			}
 		}
 	}
 
 	return;
}
 
 
// //*******************************************************************
// *Name				:kbButtonPars
// *Description		:�������ս����������յ��İ������ݴ��������ð���ֵ
// *Input				:kb_dev_id	�豸ѡ��	DEV_BUTTON_KEYA=A1����	DEV_BUTTON_KEYB=B1����
// *						:data			���̷��ص�����
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
static void kbButtonPars(int kb_dev_id, char data)
{
 	KbParamStructType *kbparam=NULL;
 	LIST *kblist=NULL;
 	KbButtonNodeStruct *node=NULL;
 	unsigned int button=0;
 
 	//�жϼ���ѡ��
 	if(DEV_BUTTON_KEYA==kb_dev_id)			
 	{
 		kbparam=&KbParamA1;	
		kblist=&KbButtonListA1;
 	}
 	else if(DEV_BUTTON_KEYB==kb_dev_id)	
 	{
 		kbparam=&KbParamB1;	
		kblist=&KbButtonListB1;
 	}
 	else	
		return;
 
 	//��ֹ�������
 	if(kbparam->ButtonLen>=16)	
		kbparam->ButtonLen=0;
 	//��������
 	kbparam->ButtonBuffer[kbparam->ButtonLen]=data;
 	if(kbparam->ButtonLen==0)
 	{
 		if(0xfb==kbparam->ButtonBuffer[kbparam->ButtonLen])	kbparam->ButtonLen++;
 	}
 	else	
		kbparam->ButtonLen++;
 	//�ж�һ֡
 	if((kbparam->ButtonLen>=3)&&(kbparam->ButtonLen>=(kbparam->ButtonBuffer[1]+2)))
 	{
 		//У��������������֡����
 		if(kbparam->ButtonBuffer[kbparam->ButtonLen-1]!=xorGet(&kbparam->ButtonBuffer[1], kbparam->ButtonLen-2))
 		{
 			kbparam->ButtonLen=0;
 			return;
 		}
 
 		//��������
 		kbparam->ButtonLen=0;
 
 		//������Ч��������������ֱ��̧��һ������һ����Ч������ĳЩ��ť�������ⰴ��ֵ����
 		button=(kbparam->ButtonBuffer[3]<<24)|(kbparam->ButtonBuffer[4]<<16)|(kbparam->ButtonBuffer[5]<<8)|(kbparam->ButtonBuffer[6]);
 		if(0x00000001==button || 0x00000002==button || 0x00000003==button || 0x00000004==button)
 		{
 			//��ֹ�������
 			//taskLock();
			pthread_rwlock_wrlock(&rwlock_write);
 			
 			//YP1
 			if(0x00000001==button)
 			{
 				kbparam->OilLeftChg=1;			
				kbparam->OilLeft=0;
 			}
 			if(0x00000001==button && DEV_BUTTON_KEYA==kb_dev_id)
 			{
 				KbSwitch.ValueYPA1 = 1;
 			}
 			if(0x00000001==button && DEV_BUTTON_KEYB==kb_dev_id)
 			{
 				KbSwitch.ValueYPB1 = 1;
 			}
 
 			//YP2
 			if(0x00000002==button)
 			{
 				kbparam->OilRightChg=1;		
				kbparam->OilRight=0;
 			}
 			if(0x00000002==button && DEV_BUTTON_KEYA==kb_dev_id)
 			{
 				KbSwitch.ValueYPA2 = 1;
 			}
 			if(0x00000002==button && DEV_BUTTON_KEYB==kb_dev_id)
 			{
 				KbSwitch.ValueYPB2 = 1;
 			}
 
 			//YP3
 			if(0x00000003==button)
 			{
 				kbparam->OilConfirmChg=1;	
				kbparam->OilConfirm=0;
 			}
 			if(0x00000003==button && DEV_BUTTON_KEYA==kb_dev_id)
 			{
 				KbSwitch.ValueYPA3 = 1;
 			}
 			if(0x00000003==button && DEV_BUTTON_KEYB==kb_dev_id)
 			{
 				KbSwitch.ValueYPB3 = 1;
 			}
 
 			//������״̬
 			if(0x00000004==button)
 			{
 				kbparam->KeyLockChg=1;		
				kbparam->KeyLock=0;
 			}
 			if(0x00000004==button && DEV_BUTTON_KEYA==kb_dev_id)
 			{
 				KbSwitch.ValueLockA = 1;
 			}
 			if(0x00000004==button && DEV_BUTTON_KEYB==kb_dev_id)
 			{
 				KbSwitch.ValueLockB = 1;
 			}
 
 			//�ָ��������
 			//taskUnlock();
			pthread_rwlock_unlock(&rwlock_write);
 		}
 		else if(KB_BUTTON_NO!=kbparam->ButtonLast && KB_BUTTON_NO==button)
 		{
 			//��ֹ�������
 			//taskLock();
			pthread_rwlock_wrlock(&rwlock_write);
 
 			//�ڵ�������10����ɾ����һ��
 			if(lstCount(kblist)>=10)
 			{
 				node=(KbButtonNodeStruct*)lstGet(kblist);
 				if(NULL!=node)
 				{
 					free(node);
 				}
 			}
 
 			//����һ���ڵ㲢��ӵ�β��
 			node=(KbButtonNodeStruct*)malloc(sizeof(KbButtonNodeStruct));
 			if(NULL!=node)
 			{
 				node->Value=kbparam->ButtonLast;
 				lstAdd(kblist, (NODE*)node);
 			}
 
 			//�ָ��������
 			//taskUnlock();
			pthread_rwlock_unlock(&rwlock_write);
 		}
 
 		//����Ϊǰһ�ΰ���
 		kbparam->ButtonLast=button;

		printf("kbButtonPars\n");
 	}
 
 	return;
}
 
 
// //*******************************************************************
// *Name				:kbDspDataPars
// *Description		:��ʾ���ݽ��ս�������������ִ����ʾ����ķ�������
// *Input				:kb_id	�豸ѡ��	0=A1����	1=B1����
// *						:data	���̷��ص�����
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
static void kbDspDataPars(int kb_id, char data)
{
 	KbParamStructType *kbparam=NULL;
 	int len=0;
 
 	if(0==kb_id)			kbparam=&KbParamA1;
 	else if(1==kb_id)	kbparam=&KbParamB1;
 	else							return;
 
 	if(0==kbparam->DspPack)
 	{	
 		//��������
 		kbparam->DspBuffer[kbparam->DspLen]=data;
 		if(0==kbparam->DspLen)
 		{
 			if(0xfa==kbparam->DspBuffer[kbparam->DspLen])		kbparam->DspLen++;
 		}
 		else	kbparam->DspLen++;
 		//����һ֡����
 		len=(kbparam->DspBuffer[1]<<8)|(kbparam->DspBuffer[2]<<0);
 		if((kbparam->DspLen>=5)&&(kbparam->DspLen>=(len+3)))
 		{
 			if(kbparam->DspBuffer[kbparam->DspLen-1]==xorGet(&kbparam->DspBuffer[1], kbparam->DspLen-2))
 			{
 				kbparam->DspPack=1;
 			}
 		}
 		else if(kbparam->DspLen>=128)
 		{
 			kbparam->DspPack=0;
 
 			kbparam->DspLen=0;
 		}
 	}
 
 	return;
}
 
 
// //*******************************************************************
// *Name				:kbSwitchDataPars
// *Description		:�������ݽ�����������������̱���Ŀ���״̬��
// *Input				:kb_id	�豸ѡ��	0=A1����	1=B1����
// *						:data	���̷��ص�����
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
static void kbSwitchDataPars(int kb_id, char data)
{
 	KbParamStructType *kbparam=NULL;
 	char state=0;
 
 	if(0==kb_id)			
		kbparam=&KbParamA1;
 	else if(1==kb_id)	
		kbparam=&KbParamB1;
 	else							
		return;
 
 	//���̸�λ״̬
 	state=(data>>0)&1;
 	if(state!=kbparam->RstState)	
		kbparam->RstStateChg=1;
 	kbparam->RstState=state;
 
 	//������״̬
 	state=(data>>1)&1;
 	if(state!=kbparam->KeyLock)	
		kbparam->KeyLockChg=1;
 	kbparam->KeyLock=state;
 
 	//��ǹ��Ʒѡ��״̬
 	state=(data>>2)&1;
 	if(state!=kbparam->OilLeft)
		kbparam->OilLeftChg=1;
 	kbparam->OilLeft=state;
 
 	//��ǹ��Ʒѡ��״̬
 	state=(data>>3)&1;
 	if(state!=kbparam->OilRight)	
		kbparam->OilRightChg=1;
 	kbparam->OilRight=state;
 
 	//��Ʒȷ��״̬
 	state=(data>>4)&1;
 	if(state!=kbparam->OilConfirm)	
		kbparam->OilConfirmChg=1;
 	kbparam->OilConfirm=state;
 
 	//��ӡ��æ��״̬
 	state=(data>>5)&1;
 	if(state!=kbparam->PrinterBusyState)	
		kbparam->PrinterBusyChg=1;
 	kbparam->PrinterBusyState=state;
 
 	//��Ӧ����״̬
 	state=(data>>6)&1;
 	if(state!=kbparam->PIRState)	
		kbparam->PIRStateChg=1;
 	kbparam->PIRState=state;
 
 	return;
}
 
 
// //*******************************************************************
// *Name				:kbTaskSwitchRead
// *Description		:����״̬��ʱ��ȡ
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
void kbTaskSwitchRead()
{
	prctl(PR_SET_NAME,(unsigned long)"kbSwitchRead");
 	char tx_buffer[16]={0};
 	int tx_len=0;
 
 	//���̿���״̬��ʱ200�����ȡ
 	tx_buffer[0]=(KB_PORT_SWITCH<<5)|(KB_DATA_HI<<4)|((0x01>>4)&0x0f);
 	tx_buffer[1]=(KB_PORT_SWITCH<<5)|(KB_DATA_LOW<<4)|((0x01>>0)&0x0f);
 	tx_len=2;
 
 	FOREVER
 	{
 		comWrite(COM4, tx_buffer, tx_len);
 
 		comWrite(COM5, tx_buffer, tx_len);

       // PrintH(tx_len,tx_buffer);

        usleep(200000);
 
 		//taskDelay(200*ONE_MILLI_SECOND);
 	}
 
 	return;
}
// 
// 
// //*******************************************************************
// *Name				:tKbDsp
// *Description		:����״̬��ʱ��ȡ
// *Input				:DEV_DSP_KEYx	�豸ѡ�� DEV_DSP_KEYA / DEV_DSP_KEYB
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
//void tKbDsp(int DEV_DSP_KEYx)
void tKbDsp(void* kbType)
{
	int DEV_DSP_KEYx = (int*)kbType;
 	char msg_buffer[512] = {0};
 	int msg_len = 0;
 
 	char rxbuffer[512] = {0};
 	int rx_len = 0;
 
 	int timer = 0, i = 0, istate = 0, ackWait = 0;
// 	MSG_Q_ID msgId = NULL;
    int msgId = 0;

	struct msg_struct msg_stKbDspRev;
	msg_stKbDspRev.msgType = 1;
	memset(msg_stKbDspRev.msgBuffer,0,512);

 
 	//�ж��豸ѡ��
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)
	{
		msgId = msgIdDspA1;
		prctl(PR_SET_NAME,(unsigned long)"tKbDspA");
	}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	
	{
		msgId = msgIdDspB1;
        prctl(PR_SET_NAME,(unsigned long)"tKbDspB");
	}
 	else																	
		return;
 
 	//�����յ����͵����̵���ʾ�˿�����ʱ�����������ж���δ���
 	FOREVER
 	{
 		//msg_len = msgQReceive(msgId, msg_buffer, 512, WAIT_FOREVER);
 		//if(msg_len > 0)
 		//{
 		//	istate = kbDspSend(DEV_DSP_KEYx, 0, msg_buffer, msg_len, rxbuffer, 512);
 		//}
 		//taskDelay(1);

		msg_len = msgrcv(msgId,&msg_stKbDspRev,512,0,IPC_NOWAIT);
		if(msg_len > 0)
		{
			memcpy(msg_buffer,msg_stKbDspRev.msgBuffer,msg_len);
			istate = kbDspSend(DEV_DSP_KEYx, 0, msg_buffer, msg_len, rxbuffer, 512); 
			//printf("keyboard dsp ,msg_len = %d\n",msg_len);
			//PrintH(msg_len,msg_buffer);
		}

		usleep(1000);
 	}
 
 	return;
}
 
 
// //****************************************************************************
// *Name				:tRxCom4
// *Description		:����4���ݽ��ղ�����
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-07-01
 
void tRxCom4()
{
	prctl(PR_SET_NAME,(unsigned long)"tRxCom4");
 	char read_buffer[128]={0};
 	int read_len=0, i=0;
 	char port=0, data_bit=0, data=0;
 	char data_hi[8]={0};	//���ݸ�λ��bit7~4����1����洢�˱��˿ڸ߰��Լ����ݣ�bit3~0�洢����
 	char data_low[8]={0};//���ݵ�λ��bit7~4����1����洢�˱��˿ڵͰ��Լ����ݣ�bit3~0�洢����
 
 	FOREVER
 	{
 		read_len=comRead(COM4, read_buffer, 128);
		//read_len = comRead(gFdPcServerCom,read_buffer,128);

		/*if(read_len > 0)
		{
            printf("************com4 to keyboard is data: \n");
            PrintH(read_len,read_buffer);
		}*/
		//else
		//{
		//	printf("ssssssssssssssssssssssssssssssssss\n");
		//	printf("\s");
		//}
		
 
 		for(i=0; i<read_len; i++)
 		{
 			port=(read_buffer[i]>>5)&0x07;	data_bit=(read_buffer[i]>>4)&0x01;	data=(read_buffer[i]>>0)&0x0f;
 
 			switch(port)
 			{
 			case KB_PORT_SET: //�������ü���ѯ�˿�
 				if(KB_DATA_HI==data_bit)
 				{
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0); 
 					//�����������Ч���ݴ���
 					kbSetPars(0, data);
 				}
 
 				break;
			case KB_PORT_DSP: //fj:������ʾ�˿�
 				if(KB_DATA_HI==data_bit)//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 				{		
 					data_hi[port]=0;	
					data_low[port]=0;	
					data_hi[port]=(1<<4)|data;
 				}
 				else//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 				{	
 					if(1==((data_hi[port]>>4)&1))
					{
						data_low[port]=(1<<4)|data;
					}
 					else									
					{
						data_hi[port]=0;	
						data_low[port]=0;
					}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);	
 					kbDspDataPars(0, data);	//�����������Ч���ݴ���
 				}
 				break;	
			case KB_PORT_BUTTON: //fj:�����˿�
 				if(KB_DATA_HI==data_bit)//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 				{		
 					data_hi[port]=0;	
					data_low[port]=0;	
					data_hi[port]=(1<<4)|data;
 				}
 				else//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 				{	
 					if(1==((data_hi[port]>>4)&1))
					{
						data_low[port]=(1<<4)|data;
					}
 					else
					{
						data_hi[port]=0;	
						data_low[port]=0;
					}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 					//�����������Ч���ݴ���!
 					kbButtonPars(DEV_BUTTON_KEYA, data);
 				}
 				break;
			case KB_PORT_IC: //fj:�������˿ڣ�����1
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1))
					{data_low[port]=(1<<4)|data;}
 					else												
					{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					if(rngNBytes(RngRxUsartA2)<KB_RNG_MAX_LEN)
 					{
 						rngBufPut(RngRxUsartA2, &data, 1);
 					}
 				}
 				break;
			case KB_PORT_PRN: //fj:��ӡ�˿�,����2
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					if(rngNBytes(RngRxUsartA1)<KB_RNG_MAX_LEN)
 					{
 						rngBufPut(RngRxUsartA1, &data, 1);
 					}
 				}
 				break;
			case KB_PORT_CODE: //fj:����ɨ��˿ڴ���4
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					if(rngNBytes(RngRxUsartA4)<KB_RNG_MAX_LEN)
 					{
 						rngBufPut(RngRxUsartA4, &data, 1);
 					}
 				}
 				break;
			case KB_PORT_NET: //fj:�����������˿ڣ�����5
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					if(rngNBytes(RngRxUsartA5)<KB_RNG_MAX_LEN)
 					{
 						rngBufPut(RngRxUsartA5, &data, 1);
 					}
 				}
 				break;
			case KB_PORT_SWITCH: //fj:�����źż���λ״̬����˿�
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 					
 					//�����������Ч���ݴ���
 					kbSwitchDataPars(0, data);
 				}
 				break;
 			default:
 				break;
 			}
 		}
 
 		//taskDelay(1);
		usleep(1000);
 	}
 
 	return;
}
 
 
// //****************************************************************************
// *Name				:tRxCom5
// *Description		:����5���ݽ��ղ�����
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-07-01
 
void tRxCom5()
{
	prctl(PR_SET_NAME,(unsigned long)"tRxCom5");
 	char read_buffer[128]={0}; 
 	int read_len=0, i=0;
 	char port=0, data_bit=0, data=0;
 	char data_hi[8]={0};	//���ݸ�λ��bit7~4����1����洢�˱��˿ڸ߰��Լ����ݣ�bit3~0�洢����
 	char data_low[8]={0};//���ݵ�λ��bit7~4����1����洢�˱��˿ڵͰ��Լ����ݣ�bit3~0�洢����
 
 	FOREVER
 	{
 		read_len=comRead(COM5, read_buffer, 128);
 
 		for(i=0; i<read_len; i++)
 		{
 			port=(read_buffer[i]>>5)&0x07;	data_bit=(read_buffer[i]>>4)&0x01;	data=(read_buffer[i]>>0)&0x0f;
 
 			switch(port)
 			{
 			case KB_PORT_SET:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					kbSetPars(1, data);
 				}
 
 				break;
 			case KB_PORT_DSP:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					kbDspDataPars(1, data);
 				}
 				break;	
 			case KB_PORT_BUTTON:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���!
 					kbButtonPars(DEV_BUTTON_KEYB, data);
 				}
 				break;
 			case KB_PORT_IC:
 				if(KB_DATA_HI==data_bit)
 				{
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					rngBufPut(RngRxUsartB2, &data, 1);
 				}
 				break;
 			case KB_PORT_PRN:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					rngBufPut(RngRxUsartB1, &data, 1);
 				}
 				break;
 			case KB_PORT_CODE:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					rngBufPut(RngRxUsartB4, &data, 1);
 				}
 				break;
 			case KB_PORT_NET:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					rngBufPut(RngRxUsartB5, &data, 1);
 				}
 				break;
 			case KB_PORT_SWITCH:
 				if(KB_DATA_HI==data_bit)
 				{	
 					//����Ϊ��λ����ʱ������ǰһ�̴洢״̬��ξ����´洢���ֽ�����
 					data_hi[port]=0;	data_low[port]=0;	data_hi[port]=(1<<4)|data;
 				}
 				else
 				{
 					//����Ϊ��λ����ʱ���Ѵ洢��Ч��λ������õ�λ���ݶԣ�������Ϊ���ݴ�λ
 					if(1==((data_hi[port]>>4)&1)){data_low[port]=(1<<4)|data;}
 					else												{data_hi[port]=0;	data_low[port]=0;}
 				}
 
 				if((1==((data_hi[port]>>4)&1))&&(1==((data_low[port]>>4)&1)))
 				{
 					data=((data_hi[port]&0x0f)<<4)|((data_low[port]&0x0f)<<0);
 
 					//�����������Ч���ݴ���
 					kbSwitchDataPars(1, data);
 				}
 				break;
 			default:
 				break;
 			}
 		}
 
 		//taskDelay(1);
		usleep(1000);
 	}
 
 	return;
}

// //*******************************************************************
// *Name				:kbBuzzerBeep
// *Description		:��������
// *Input				:KEYx	�豸ѡ��	0=A1����	1=B1����
// *						:type	������������������(0=������������ͣ������������)
// *Output			:None
// *Return				:0=�ɹ�������=ʧ��
// *History			:2015-05-24,modified by syj*/

int kbBuzzerBeep(int KEYx, char type)
{
 	unsigned char tx_buffer[256]={0};
 	int tx_len=0, len=0;
 	unsigned char msg_buffer[512]={0};
 	int msg_len=0;
 	DspContentType *dsp=NULL;
 	int istate=0, i=0, j=0;

	struct msg_struct msg_stKbBB;
	msg_stKbBB.msgType = 1;
 
 	//��ʱ������ʾ����ʵ�ֹ���
 	if(0==KEYx)			
		dsp=&DspContentA;
 	else if(1==KEYx)		
		dsp=&DspContentB;
 	else							
		return ERROR;
 
 	//�������
 	tx_buffer[0]=0xfa;	tx_buffer[1]=3>>8;	tx_buffer[2]=3>>0;	tx_buffer[3]=0x10;
 	tx_buffer[4]=type;	tx_buffer[5]=xorGet(&tx_buffer[1], 4);
 	tx_len=6;
    memcpy(msg_stKbBB.msgBuffer,tx_buffer,tx_len); 
 
	//fj:
 	//if(0==KEYx)			msgQSend(msgIdDspA1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
 	//else	if(1==KEYx)	msgQSend(msgIdDspB1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
  	
	if(0==KEYx)
	{
		msgsnd(msgIdDspA1,&msg_stKbBB,tx_len,IPC_NOWAIT);
		printf("kbBuzzerBeep,tx_len = %d\n",tx_len);
	}
 	else if(1==KEYx)
	{
		//msgsnd(msgIdDspB1,tx_buffer,tx_len,IPC_NOWAIT);
		msgsnd(msgIdDspB1,&msg_stKbBB,tx_len,IPC_NOWAIT);
	}
 
 
 #if 0	//20150119 by SYJ
 	//���ݷ�װ����
 	for(i=0; i<tx_len; i++)
 	{
 		msg_buffer[2*i]=(KB_PORT_DSP<<5)|(KB_DATA_HI<<4)|((tx_buffer[i]>>4)&0x0f);
 		msg_buffer[2*i+1]=(KB_PORT_DSP<<5)|(KB_DATA_LOW<<4)|((tx_buffer[i]>>0)&0x0f);
 	}
 	msg_len=2*i;
 
 	//��������
 	if(0==KEYx)	comWrite(COM4, msg_buffer, msg_len);
 	else					comWrite(COM5, msg_buffer, msg_len);
 #endif
 	
	return 0;
}
 
 
//*******************************************************************
// *Name				:kbButtonRead
// *Description		:���̰�����ȡ
// *Input				:kb_dev_id	�豸ѡ��	DEV_BUTTON_KEYA=A1����	DEV_BUTTON_KEYB=B1����
// *Output			:None
// *Return				:����ֵ���ް���ʱ����KB_BUTTON_NO
// *History			:2013-07-01,modified by syj*/
 
unsigned int kbButtonRead(int kb_dev_id)
{
 	unsigned int button=KB_BUTTON_NO, value=0;
 	KbButtonNodeStruct *node=NULL;
 
 	//�жϰ����豸
 	if(DEV_BUTTON_KEYA==kb_dev_id)			
		node=(KbButtonNodeStruct*)lstGet(&KbButtonListA1);
 	else if(DEV_BUTTON_KEYB==kb_dev_id)	
		node=(KbButtonNodeStruct*)lstGet(&KbButtonListB1);
 	else																
		return KB_BUTTON_NO;
 
 	//�ж��Ƿ��а���
 	if(NULL==node)
 	{
 		return KB_BUTTON_NO;
 	}

 	//��������
 	value=node->Value;
 	free(node);
 	switch(value)
 	{
 	case 0xffffffff:		//�ް���ֵ
 		button=KB_BUTTON_NO;
 		break;
 	case 0xfeffffff:		// 1
 		button=KB_BUTTON_1;
 		break;
	case 0xfdffffff:		// 4
 		button=KB_BUTTON_4;
 		break;
 	case 0xfbffffff:		// 7
 		button=KB_BUTTON_7;
 		break;
 	case 0xf7ffffff:		// ����/.
 		button=KB_BUTTON_CZ;
 		break;
 	case 0xefffffff:		// ����
 		button=KB_BUTTON_SET;
 		break;
 	case 0xfffeffff:		// 2
 		button=KB_BUTTON_2;
		break;
 	case 0xfffdffff:		// 5
 		button=KB_BUTTON_5;
 		break;
 	case 0xfffbffff:		// 8
 		button=KB_BUTTON_8;
 		break;
 	case 0xfff7ffff:		// 0
 		button=KB_BUTTON_0;
 		break;
 	case 0xffefffff:		// ��
 		button=KB_BUTTON_UP;
 		break;
 	case 0xfffffeff:		// 3
 		button=KB_BUTTON_3;
 		break;
 	case 0xfffffdff:		// 6
 		button=KB_BUTTON_6;
 		break;
 	case 0xfffffbff:		// 9
 		button=KB_BUTTON_9;
 		break;
 	case 0xfffff7ff:		// ȷ��
 		button=KB_BUTTON_ACK;
 		break;
 	case 0xffffefff:		// ��
 		button=KB_BUTTON_DOWN;
 		break;
 	case 0xfffffffe:		// �����
 		button=KB_BUTTON_MON;
 		break;
 	case 0xfffffffd:		// ������
 		button=KB_BUTTON_VOL;
 		break;
 	case 0xfffffffb:		// �˿�
 		button=KB_BUTTON_BACK; 
 		break;
 	case 0xfffffff7:		// ����
 		button=KB_BUTTON_CHG;
 		break;
 	case 0xffffffef:		// ѡ��
 		button=KB_BUTTON_SEL;
 		break;
 	case 0xfefffffb:		// �˿�+1
 		button=KB_BUTTON_BACK1;
 		break;
 	case 0xfffefffb:		// �˿�+2
 		button=KB_BUTTON_BACK2;
 		break;
 	case 0xfffffefb:		// �˿�+3
 		button=KB_BUTTON_BACK3; 
 		break;
 	case 0xfdfffffb:		// �˿�+4
 		button=KB_BUTTON_BACK4;
 		break;
 	case 0xfffdfffb:		// �˿�+5
 		button=KB_BUTTON_BACK5;
 		break;
 	case 0xfffffdfb:		// �˿�+6
 		button=KB_BUTTON_BACK6;
 		break;
 	case 0xfbfffffb:		// �˿�+7
 		button=KB_BUTTON_BACK7;
 		break;
 	case 0xfffbfffb:		// �˿�+8
 		button=KB_BUTTON_BACK8;
 		break;
 	case 0xfffffbfb:		// �˿�+9
 		button=KB_BUTTON_BACK9;
 		break;
 	case 0xeeffffff:		// ����+1
 		button=KB_BUTTON_SET1;
 		break;
 	case 0xeffeffff:		// ����+2
 		button=KB_BUTTON_SET2;
 		break;
 	case 0xeffffeff:		// ����+3
 		button=KB_BUTTON_SET3;
 		break;
 	case 0xedffffff:		// ����+4
 		button=KB_BUTTON_SET4;
 		break;
 	case 0xeffdffff:		// ����+5
 		button=KB_BUTTON_SET5;
 		break;
 	case 0xeffffdff:		// ����+6
 		button=KB_BUTTON_SET6;
 		break;
 	case 0xebffffff:		// ����+7
 		button=KB_BUTTON_SET7;
 		break;
 	case 0xeffbffff:		// ����+8
 		button=KB_BUTTON_SET8;
 		break;
 	case 0xeffffbff:		// ����+9
 		button=KB_BUTTON_SET9;
 		break; 
 	case 0x00000001:		//YP1
		button = KB_BUTTON_YP1;
 		break;
 	case 0x00000002:		//YP2
 		button = KB_BUTTON_YP2;
 		break;
 	case 0x00000003:		//YP3
 		button = KB_BUTTON_YP3;
 		break;
 
 	default:
 		button=KB_BUTTON_NO;
 		break;
 	}
 
 	return button;
} 
 
// //*******************************************************************
// *Name				:kbSwitchConfirmRead
// *Description		:������չ����״̬��ѯ��������������ŷƷѡ��
// *Input				:kb_dev_id	�豸ID��	DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB/
// *						:											DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3
// *						:											DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3
// *Output			:ischg			״̬���ϴ��Ƿ����仯0=�ޱ仯��1=�б仯
// *Return				:��ǰ��Ч״̬�����󷵻�ERROR
// *History			:2013-07-01,modified by syj*/
 
int kbSwitchRead(int DEV_SWITCH_xx, int *ischg)
{
 	int ireturn=ERROR;
 
 	switch(DEV_SWITCH_xx)
 	{
 	case DEV_SWITCH_LOCKA:
 		ireturn=KbParamA1.KeyLock;
 		*ischg=KbParamA1.KeyLockChg;	KbParamA1.KeyLockChg=0;
 		break;
 	case DEV_SWITCH_LOCKB:
 		ireturn=KbParamB1.KeyLock;
 		*ischg=KbParamB1.KeyLockChg;	KbParamB1.KeyLockChg=0;
 		break;
 	case DEV_SWITCH_SELA1:
 		ireturn=KbParamA1.OilLeft;
 		*ischg=KbParamA1.OilLeftChg;	KbParamA1.OilLeftChg=0;
 		break;
 	case DEV_SWITCH_SELB1:
 		ireturn=KbParamB1.OilLeft;
 		*ischg=KbParamB1.OilLeftChg;	KbParamB1.OilLeftChg=0;
 		break;
 	case DEV_SWITCH_SELA2:
 		ireturn=KbParamA1.OilRight;
 		*ischg=KbParamA1.OilRightChg;	KbParamA1.OilRightChg=0;
 		break;
 	case DEV_SWITCH_SELB2:
 		ireturn=KbParamB1.OilRight;
 		*ischg=KbParamB1.OilRightChg;	KbParamB1.OilRightChg=0;
 		break;
 	case DEV_SWITCH_SELA3:
 		ireturn=KbParamA1.OilConfirm;
 		*ischg=KbParamA1.OilConfirmChg;	KbParamA1.OilConfirmChg=0;
 		break;
 	case DEV_SWITCH_SELB3:
 		ireturn=KbParamB1.OilConfirm;
 		*ischg=KbParamB1.OilConfirmChg;	KbParamB1.OilConfirmChg=0;
 		break;
 	default:
 		break;
 	}
 
 	return ireturn;
}
 

// //*******************************************************************
// *Name				:kbDspSend
// *Description		:����̷�����ʾ�˿����ݲ��ȴ�����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����/DEV_DSP_KEYB=B1����
// *						:waitAck				�Ƿ�ȴ��������ݣ�0=�ǣ�1=��
// *						:inbuffer				Ҫ���͵����ݻ��棬����Э�������һ֡����
// *						:nbytes					Ҫ���͵����ݻ��泤��
// *						:maxbytes			Ҫ�������ݵĻ��泤�ȣ�>=128
// *Output			:outbuffer				Ҫ�������ݵĻ��棬����Э�������һ֡����
// *Return				:0=�ɹ���1=��ʱ������=ʧ��
// *History			:2016-01-18,modified by syj*/

int kbDspSend(int DEV_DSP_KEYx, int ackWait, char *inbuffer, int nbytes, char *outbuffer, int maxbytes)
{
 	char tx_buffer[1024] = {0};
 	int tx_len = 0;
 	int comX = 0, i = 0, istate = 0, timer = 0;
 	//SEM_ID semId = NULL;
	//int semId = 0;
	pthread_mutex_t semId;
 	KbParamStructType *param = NULL;
 
 	//�ж��豸ѡ��
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			
	{
		semId = semIdDspA1;	
		comX = COM4;	
		param = &KbParamA1;
	}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	
	{
		semId = semIdDspB1;	
		comX = COM5;	
		param = &KbParamB1;
	}
 	else	
		return ERROR;
 
 	//��֯����
 	for(i = 0; (i < nbytes) && (i < 1024/2); i++)
 	{
 		*(tx_buffer + i*2 + 0) = (KB_PORT_DSP<<5)|(KB_DATA_HI<<4)|(((*(inbuffer + i)) >> 4) & 0x0f);
 		*(tx_buffer + i*2 + 1) = (KB_PORT_DSP<<5)|(KB_DATA_LOW<<4)|(((*(inbuffer + i)) >> 0) & 0x0f);;
 	}
 	tx_len = i*2;
 
 
 	//��ȡ�ź���,fj:
// 	semTake(semId, WAIT_FOREVER);
    pthread_mutex_lock(&semId);
 
// 	//��������
 	memset(param->DspBuffer, 0, sizeof(param->DspBuffer));	param->DspLen = 0;	param->DspPack= 0;
 	comWrite(comX, tx_buffer, tx_len);
 
 	//�ж��Ƿ�ȴ�����
 	if(0 != ackWait)
 	{
 		goto DSP_SNED_DONE;
 	}
 
// 	//�ȴ����أ���ʱ2��
 	while(1)
 	{
 		if(1 == param->DspPack && *(inbuffer + 3) == *(param->DspBuffer + 3))
 		{
 			memcpy(outbuffer, param->DspBuffer, param->DspLen);
 			goto DSP_SNED_DONE;
 		}
 
 		if(timer >= 2*ONE_SECOND)
		//if(timer >= 6*ONE_SECOND)
 		{
			istate = 1;
 			goto DSP_SNED_DONE;
 		}
 	
 		timer+=(10*ONE_MILLI_SECOND);
 		//taskDelay(10*ONE_MILLI_SECOND);
		usleep(10000);
 	}
 
 
 DSP_SNED_DONE:
 
 	//�ͷ��ź���
// 	semGive(semId);
    pthread_mutex_unlock(&semId);
 
 	return istate;
}
 
 
// //*******************************************************************
// *Name				:kbDspContrastSet
// *Description		:������ʾ�Աȶ�����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����/DEV_DSP_KEYB=B1����
// *						:contrast				�Աȶ�(��Ч��Χ 25~40)
// *Output			:None
// *Return				:0=�ɹ���1=��ʱ������=ʧ��
// *History			:2016-01-18,modified by syj*/
 
int kbDspContrastSet(int DEV_DSP_KEYx, unsigned char contrast)
{
 	char data_buffer[32] = {0}, outbuffer[256] = {0};
 	int data_len = 0, istate = 0, i = 0;
 
 	*(data_buffer + data_len) = 0xfa;	data_len++;
 	*(data_buffer + data_len) = 0x00;	data_len++;
 	*(data_buffer + data_len) = 0x03;	data_len++;
 	*(data_buffer + data_len) = 0x02;	data_len++;
 	*(data_buffer + data_len) = contrast;	data_len++;
 	*(data_buffer + data_len) = xorGet(data_buffer + 1, 4);	data_len++;
 
 	istate = kbDspSend(DEV_DSP_KEYx, 0, data_buffer, data_len, outbuffer, sizeof(outbuffer));
 	if(0==istate && 0x02==*(outbuffer+3) && 0x00==*(outbuffer+4)){
 		return 0;
 	}
 	if(1==istate)
	{
 		return 1;
 	}
 
 	return ERROR;
}
 
 
// //*******************************************************************
// *Name				:kbDspEmptyRefresh
// *Description		:������ʾ��ղ�ˢ����ʾ
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����/DEV_DSP_KEYB=B1����
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj*/
 
void kbDspEmptyRefresh(int DEV_DSP_KEYx)
{
 	char tx_buffer[16]={0};
 	int tx_len=0;
 	int i=0;
	struct msg_struct msg_stKbDER;
	msg_stKbDER.msgType = 1;
 	
 	//�������
 	tx_buffer[0]=0xfa;	tx_buffer[1]=2>>8;	tx_buffer[2]=2>>0;	tx_buffer[3]=0x04;
 	tx_buffer[4]=xorGet(&tx_buffer[1], 3);
 	tx_len=5;
	memcpy(msg_stKbDER.msgBuffer,tx_buffer,tx_len);
 
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)	
 	{
 		memset(KbDspA1.Buffer, 0, 128*8);	
		memset(KbDspA1.Len, 0, 8);	
		memset(KbDspA1.Pointer, 0, 8);
	    msgsnd(msgIdDspA1,&msg_stKbDER,tx_len,IPC_NOWAIT); //fj:20170925
        printf("kbDspEmptyRefresh,tx_len = %d\n",tx_len);

 		//fj:
		//msgQSend(msgIdDspA1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
 	}
 	else
 	if(DEV_DSP_KEYB==DEV_DSP_KEYx)	
 	{
 		memset(KbDspB1.Buffer, 0, 128*8);	
		memset(KbDspB1.Len, 0, 8);	
		memset(KbDspB1.Pointer, 0, 8);
        msgsnd(msgIdDspB1,&msg_stKbDER,tx_len,IPC_NOWAIT); //fj:20170925
// 		msgQSend(msgIdDspB1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
 	}
 
#if 0	//20160119 by SYJ
 	//���ݷ�װ����
 	for(i=0; i<tx_len; i++)
 	{
 		msg_buffer[2*i]=(KB_PORT_DSP<<5)|(KB_DATA_HI<<4)|((tx_buffer[i]>>4)&0x0f);
 		msg_buffer[2*i+1]=(KB_PORT_DSP<<5)|(KB_DATA_LOW<<4)|((tx_buffer[i]>>0)&0x0f);
 	}
 	msg_len=2*i;
 
 	//����
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)	
 	{
 		memset(KbDspA1.Buffer, 0, 128*8);	memset(KbDspA1.Len, 0, 8);	memset(KbDspA1.Pointer, 0, 8);
// 		msgQSend(msgIdDspA1, msg_buffer, msg_len, NO_WAIT, MSG_PRI_NORMAL);
 	}
 	else
 	if(DEV_DSP_KEYB==DEV_DSP_KEYx)	
 	{
 		memset(KbDspB1.Buffer, 0, 128*8);	memset(KbDspB1.Len, 0, 8);	memset(KbDspB1.Pointer, 0, 8);
// 		msgQSend(msgIdDspB1, msg_buffer, msg_len, NO_WAIT, MSG_PRI_NORMAL);
 	}
 #endif	
 	return;
 }

// //*******************************************************************
// *Name				:kbDsp
// *Description		:������ʾ,ÿ��������ʾ8�����֣������ʾ����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:FONTx				�ֿ�����ѡ��	=1		16*8,16*16�ֿ� 
//        																					=2    	14*7,14*14�ֿ� 
// 	   																						=3   	8*8�ֿ�
// 	   																						=4    	12*6,12*12�ֿ�
// *						:IsContrary			�Ƿ���0=������ʾ��1=����
// *						:Offsetx				������0~7
// *						:Offsety				������0~127
// *						:buffer					��ʾ����
// *						:nbytes				��ʾ���ݳ���
// *Output			:None
// *Return				:0=�ɹ�������=����
// *History			:2013-07-01,modified by syj*/
// 
int kbDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes)
{
 	DspContentType *dsp=NULL;
 	int istate=0;
 
 	//�жϼ���ѡ��
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)			dsp=&DspContentA;
 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx)	dsp=&DspContentB;
 	else		
		return 1;
 	
 	//�ж��ֿ�ѡ��
 	if(KB_FONT16!=FONTx && KB_FONT14!=FONTx && KB_FONT12!=FONTx && KB_FONT8!=FONTx)
 	{
 		return ERROR;
 	}
 
 	//�ж�������
 	if(Offsetx>7)
 	{
 		return ERROR;
 	}
 
 	//����ʾ������ӵ�����
 	dsp->BitMap|=(1<<Offsetx);
 	dsp->Font[Offsetx]=FONTx;
 	dsp->IsContrary[Offsetx]=IsContrary;
 	dsp->OffsetY[Offsetx]=Offsety;
 	if(nbytes<=32)	dsp->Nbytes[Offsetx]=nbytes;
 	else						dsp->Nbytes[Offsetx]=32;
 	memcpy(dsp->Buffer[Offsetx], buffer, dsp->Nbytes[Offsetx]);
 	
 	return 0;
}
 
// //*******************************************************************
// *Name				:kbDsp
// *Description		:���ԣ�Ӧ���ȵ���kbDspContent�����ʾ���ݺ���ñ�����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:Contrast				�Աȶ�HEX��ʽ
// *						:IsClr					�Ƿ����ȫ��0=����գ�1=���
// *Output			:None
// *Return				:0=�ɹ�������=ʧ��
// *History			:2013-07-01,modified by syj*/
 
int kbDsp(int DEV_DSP_KEYx, int Contrast, int IsClr)
{
 	unsigned char tx_buffer[256]={0};
 	int tx_len=0, len=0;
 	unsigned char msg_buffer[512]={0};
 	int msg_len=0;
 	DspContentType *dsp=NULL;
 	int istate=0, i=0, j=0;
	struct msg_struct msg_stKbDsp;
	msg_stKbDsp.msgType = 1;
 
 	//�жϼ���ѡ��
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)			
	{
		dsp=&DspContentA;	
		DspContentA.Contrast=Contrast;
	}
 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx)	
	{
		dsp=&DspContentB;	
		DspContentB.Contrast=Contrast;
	}
 	else
		return ERROR;
 
 	//��ʾ������֡
 	tx_buffer[0]=0xfa;
 	tx_buffer[1]=0;
 	tx_buffer[2]=0;
 	tx_buffer[3]=0x0e;
 	tx_buffer[4]=dsp->Contrast;
 	tx_buffer[5]=IsClr;
 	tx_buffer[6]=dsp->BitMap;
 	for(i=0; i<8; i++)
 	{
 		if(1==((dsp->BitMap>>i)&1))
 		{
 			tx_buffer[7+len]=dsp->Font[i];
 			tx_buffer[8+len]=dsp->IsContrary[i];
 			tx_buffer[9+len]=dsp->OffsetY[i];
 			if(dsp->Nbytes[i] <= 32)
 			{
 				tx_buffer[10+len]=dsp->Nbytes[i];
 				memcpy(&tx_buffer[11+len], dsp->Buffer[i], dsp->Nbytes[i]); 
 				len=len+4+dsp->Nbytes[i];
 			}
 			else
 			{
 				tx_buffer[10+len]=32;
 				memcpy(&tx_buffer[11+len], dsp->Buffer[i], 32);
 				len=len+4+32;
 			}
 		}
 	}
 	tx_buffer[1]=(len+5)>>8;
 	tx_buffer[2]=(len+5)>>0;
 	tx_buffer[7+len]=xorGet(&tx_buffer[1], len+6);
 	tx_len=len+8;

	memcpy(msg_stKbDsp.msgBuffer,tx_buffer,tx_len);
 
 	//�������
 	dsp->BitMap=0;
 	for(i=0; i<8; i++)
 	{
 		dsp->Font[i]=0;
 		dsp->IsContrary[i]=0;
 		dsp->OffsetY[i]=0;
 		dsp->Nbytes[i]=0;
 		memset(dsp->Buffer[i], 0, 16);
 	}


  	if(DEV_DSP_KEYA==DEV_DSP_KEYx && NULL != msgIdDspA1)//fj:20170925
	{
		istate = msgsnd(msgIdDspA1,&msg_stKbDsp,tx_len,IPC_NOWAIT);
		//printf("kbDsp function ,tx_len = %d\n",tx_len); //fj:20171012
	}
 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx && NULL != msgIdDspB1)
	{
		istate = msgsnd(msgIdDspB1,&msg_stKbDsp,tx_len,IPC_NOWAIT);
	}
	//fj:
// 	if(DEV_DSP_KEYA==DEV_DSP_KEYx && NULL != msgIdDspA1)			
//		istate = msgQSend(msgIdDspA1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
// 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx && NULL != msgIdDspB1)	
//		istate = msgQSend(msgIdDspB1, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL); 
 
 	return 0;
 }
 
/*******************************************************************
 *Name				:kbDspContrary
 *Description		:������ʾָ��������
 *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
 *						:GPIOx					������IO���
 *						:state					IO����0=���õͣ�1=���ø�
 *Output			:None
 *Return				:0=�ɹ�������=ʧ��
 *History			:2014-11-22,modified by syj
 */
 
 int kbIOWrite(int DEV_DSP_KEYx, unsigned char GPIOx, unsigned char state)
 {
 	unsigned char tx_buffer[32]={0};
 	int tx_len=0;
 	DspContentType *dsp=NULL;
	struct msg_struct msg_stKbIW;
	msg_stKbIW.msgType = 1;
 
 	//��ʱ������ʾ����ʵ�ֹ���
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)
		dsp=&DspContentA;
 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx)
		dsp=&DspContentB;
 	else	
		return ERROR;
 
 	//�������
 	tx_buffer[0]=0xfa;	tx_buffer[1]=4>>8;	tx_buffer[2]=4>>0;	tx_buffer[3]=0x0f;
 	tx_buffer[4]=GPIOx;	tx_buffer[5]=state;	tx_buffer[6]=xorGet(&tx_buffer[1], 5);
 	tx_len=7;

	memcpy(msg_stKbIW.msgBuffer,tx_buffer,tx_len);
 
 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)
	{
		msgsnd(msgIdDspA1,&msg_stKbIW,tx_len,IPC_NOWAIT);
		printf("kbIOWrite,tx_len = %d\n",tx_len);  //fj:20170926
	}
 	else if(DEV_DSP_KEYB==DEV_DSP_KEYx)	
	{
		msgsnd(msgIdDspB1,&msg_stKbIW,tx_len,IPC_NOWAIT);
	}
 
 
// #if 0	//20160119 by SYJ
// 	//���ݷ�װ����
// 	for(i=0; i<tx_len; i++)
// 	{
// 		msg_buffer[2*i]=(KB_PORT_DSP<<5)|(KB_DATA_HI<<4)|((tx_buffer[i]>>4)&0x0f);
// 		msg_buffer[2*i+1]=(KB_PORT_DSP<<5)|(KB_DATA_LOW<<4)|((tx_buffer[i]>>0)&0x0f);
// 	}
// 	msg_len=2*i;
// 	if(DEV_DSP_KEYA==DEV_DSP_KEYx)			msgQSend(msgIdDspA1, msg_buffer, msg_len, NO_WAIT, MSG_PRI_NORMAL);
// 	else	if(DEV_DSP_KEYB==DEV_DSP_KEYx)	msgQSend(msgIdDspB1, msg_buffer, msg_len, NO_WAIT, MSG_PRI_NORMAL);
// #endif
 
 	return 0;
 }
 
 
//*******************************************************************
// *Name				:kbPIRSateRead
// *Description		:��Ӧ����״̬��ȡ
// *Input				:KEYx	����ѡ��0=A����	1=B����
// *Output			:None
// *Return				:��Ӧ����ֵ
// *History			:2014-11-22,modified by syj*/
 
 int kbPIRSateRead(int KEYx)
 {
 	if(0==KEYx)			return KbParamA1.PIRState;
 	else if(1==KEYx)	return KbParamB1.PIRState;
 	else							return ERROR;
 }
 
// //*******************************************************************
// *Name				:kbUartRead
// *Description		:������չ�������ݶ�ȡ������ʽ��ȡ
// *Input				:KB_USARTxx	Ŀ���Ӵ���KB_USARTA1/KB_USARTA2/KB_USARTA4/KB_USARTA5/KB_USARTB1/KB_USARTB2/KB_USARTB4/KB_USARTB5
// *						:buffer				��ȡĿ�껺��
// *						:maxbytes		��ȡĿ�껺����󳤶�
// *Output			:None
// *Return			:ʵ�ʶ�ȡ�����ֽ��������󷵻�ERROR
// *History			:2013-07-01,modified by syj
// */
 
int kbUartRead(int KB_USARTxx, char *buffer, int maxbytes)
{
 	int i=0, len=0;
 
 	switch(KB_USARTxx)
 	{
 	case KB_USARTA1:
 		if(NULL == RngRxUsartA1)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartA1)))
			usleep(1000);
 		i=rngBufGet(RngRxUsartA1, buffer, maxbytes);
 		break;
 	case KB_USARTA2:
 		if(NULL == RngRxUsartA2)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartA2)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartA2, buffer, maxbytes);
 		break;		
 	case KB_USARTA4:
 		if(NULL == RngRxUsartA4)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartA4)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartA4, buffer, maxbytes);
 		break;
 		
 	case KB_USARTA5:
 		if(NULL == RngRxUsartA5)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartA5)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartA5, buffer, maxbytes);
 		break;
 		
 	case KB_USARTB1:
 		if(NULL == RngRxUsartB1)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartB1)))
			usleep(1000);
 		i=rngBufGet(RngRxUsartB1, buffer, maxbytes);
 		break;
 		
 	case KB_USARTB2:
 		if(NULL == RngRxUsartB2)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartB2)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartB2, buffer, maxbytes);
 		break;
 		
 	case KB_USARTB4:
 		if(NULL == RngRxUsartB4)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartB4)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartB4, buffer, maxbytes);
 		break;
 		
 	case KB_USARTB5:
 		if(NULL == RngRxUsartB5)	
			return ERROR;
 		
 		while(!(len=rngNBytes(RngRxUsartB5)))	
			usleep(1000);
 		i=rngBufGet(RngRxUsartB5, buffer, maxbytes);
 		break;
 		
 	default:
 		break;
 	}
 
 	return i;
 }
 
 
// //*******************************************************************
// *Name				:kbUartWrite
// *Description	:������չ��������д��
// *Input				:KB_USARTxx	Ŀ���Ӵ���KB_USARTA1/KB_USARTA2/KB_USARTA4/KB_USARTA5/KB_USARTB1/KB_USARTB2/KB_USARTB4/KB_USARTB5
// *						:buffer				���ݻ���
// *						:maxbytes		���ݻ��泤��
// *Output			:None
// *Return			:ʵ��д����ֽ��������󷵻�ERROR
// *History			:2013-07-01,modified by syj
// */
 
int kbUartWrite(int KB_USARTxx, char *buffer, int nbytes)
{
 	char tx_buffer[KB_UART_MAX_SIZE]={0};
 	int tx_len=0;
 	int i=0, offset=0;

	//printf("com5,KB_USARTxx = %d",KB_USARTxx);
 
 	//���ݷ��ͣ����ݸ���Э����з�װ��һ�ֽ����ݲ��Ϊ2�ֽڷ���
 	switch(KB_USARTxx)
 	{
 	case KB_USARTA1:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_PRN<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_PRN<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 			
 			i+=comWrite(COM4, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTA2:	
 		while(offset<nbytes)
 		{		
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_IC<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_IC<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 
 			i+=comWrite(COM4, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTA3:
 		break;
 	case KB_USARTA4:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_CODE<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_CODE<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 			
 			i+=comWrite(COM4, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTA5:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_NET<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_NET<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 
 			i+=comWrite(COM4, tx_buffer, tx_len);
            //i+=comWrite(gFdPcServerCom, tx_buffer, tx_len);



			//printf("com4 ---\n");
			//PrintH(tx_len,tx_buffer);
 		}	
 		break;
 	case KB_USARTB1:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_PRN<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_PRN<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 			
			i+=comWrite(COM5, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTB2:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_IC<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_IC<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
			i+=comWrite(COM5, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTB3:
 		break;
 	case KB_USARTB4:
 		while(offset<nbytes)
 		{
			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
 			{
 				tx_buffer[tx_len]=(KB_PORT_CODE<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_CODE<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 			
 			i+=comWrite(COM5, tx_buffer, tx_len);
 		}
 		break;
 	case KB_USARTB5:
 		while(offset<nbytes)
 		{
 			for(tx_len=0; (tx_len<KB_UART_MAX_SIZE)&&(offset<nbytes);)
			{
 				tx_buffer[tx_len]=(KB_PORT_NET<<5)|(KB_DATA_HI<<4)|((buffer[offset]>>4)&0x0f);		tx_len++;
 				tx_buffer[tx_len]=(KB_PORT_NET<<5)|(KB_DATA_LOW<<4)|((buffer[offset]>>0)&0x0f);	tx_len++;
 				offset++;
 			}
 			
			i+=comWrite(COM5, tx_buffer, tx_len);

 		}
 		break;
 	default:
 		break;
 	}
 
 	return (i/2);
 }
 
 
// //*******************************************************************
// *Name				:kbYPUserSet
// *Description		:������Ʒѡ��ť��ǰ�û�ID
// *Input				:DEVx		�豸�� 
// *						:				DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3/
// *						:				DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3/
// *						:				DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB
// *Output			:��
// *Return				:0=�ɹ�������=ʧ�ܣ�
// *History			:2016-05-10,modified by syj*/
 
int kbYPUserSet(int DEVx, int user)
{
 	//�ж��豸ѡ��
 	if(!(DEV_SWITCH_SELA1 == DEVx || DEV_SWITCH_SELA2 == DEVx || DEV_SWITCH_SELA3 == DEVx ||\
 		DEV_SWITCH_SELB1 == DEVx || DEV_SWITCH_SELB2 == DEVx || DEV_SWITCH_SELB3 == DEVx ||\
 		DEV_SWITCH_LOCKA == DEVx || DEV_SWITCH_LOCKB == DEVx))
 	{
 		printf("%s:%d:�豸ѡ��Ƿ�!\n", __FUNCTION__, __LINE__);
 		return ERROR;
 	}
 
 	//�����û�ID
 	switch(DEVx)
 	{
 	case DEV_SWITCH_SELA1:
 		KbSwitch.UserYPA1 = user;
 		break;
 	case DEV_SWITCH_SELA2:
 		KbSwitch.UserYPA2 = user;
 		break;
 	case DEV_SWITCH_SELA3:
 		KbSwitch.UserYPA3 = user;
 		break;
 	case DEV_SWITCH_SELB1:
 		KbSwitch.UserYPB1 = user;
 		break;
 	case DEV_SWITCH_SELB2:
 		KbSwitch.UserYPB2 = user;
 		break;
 	case DEV_SWITCH_SELB3:
 		KbSwitch.UserYPB3 = user;
 		break;
 	case DEV_SWITCH_LOCKA:
 		KbSwitch.UserLockA = user;
 		break;
 	case DEV_SWITCH_LOCKB:
 		KbSwitch.UserLockB = user;
 		break;
 	default:
 		break;
 	}
 
 	return 0;
}
 
 
//*******************************************************************
// *Name				:kbYPUserGet
// *Description		:��ȡ��Ʒѡ��ť��ǰ�û�ID
// *Input				:DEVx		�豸�� 
// *						:				DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3/
// *						:				DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3/
// *						:				DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB
// *Output			:user			�û�ID��0��ʾ���û�
// *Return				:0=�ɹ�������=ʧ�ܣ�
// *History			:2016-05-10,modified by syj*/
 
int kbYPUserGet(int DEVx, int *user)
{
 	//�ж��豸ѡ��
 	if(!(DEV_SWITCH_SELA1 == DEVx || DEV_SWITCH_SELA2 == DEVx || DEV_SWITCH_SELA3 == DEVx ||\
 		DEV_SWITCH_SELB1 == DEVx || DEV_SWITCH_SELB2 == DEVx || DEV_SWITCH_SELB3 == DEVx ||\
 		DEV_SWITCH_LOCKA == DEVx || DEV_SWITCH_LOCKB == DEVx))
 	{
 		printf("%s:%d:�豸ѡ��Ƿ�!\n", __FUNCTION__, __LINE__);
 		return ERROR;
 	}
 
 	//�����û�ID
 	switch(DEVx)
 	{
 	case DEV_SWITCH_SELA1:
 		*user = KbSwitch.UserYPA1;
 		break;
 	case DEV_SWITCH_SELA2:
 		*user = KbSwitch.UserYPA2;
 		break;
 	case DEV_SWITCH_SELA3:
 		*user = KbSwitch.UserYPA3;
 		break;
 	case DEV_SWITCH_SELB1:
 		*user = KbSwitch.UserYPB1;
 		break;
 	case DEV_SWITCH_SELB2:
 		*user = KbSwitch.UserYPB2;
 		break;
 	case DEV_SWITCH_SELB3:
 		*user = KbSwitch.UserYPB3;
 		break;
 	case DEV_SWITCH_LOCKA:
 		*user = KbSwitch.UserLockA;
 		break;
 	case DEV_SWITCH_LOCKB:
 		*user = KbSwitch.UserLockB;
 		break;
 	default:
 		break;
 	}
 
 	return 0;
}
 
 
// //*******************************************************************
// *Name				:kbYPRead
// *Description		:��ȡ��Ʒ��ťֵ
// *Input				:DEVx		�豸�� 
// *						:				DEV_SWITCH_SELA1/DEV_SWITCH_SELA2/DEV_SWITCH_SELA3/
// *						:				DEV_SWITCH_SELB1/DEV_SWITCH_SELB2/DEV_SWITCH_SELB3/
// *						:				DEV_SWITCH_LOCKA/DEV_SWITCH_LOCKB
// *Output			:value		��ťֵ 0=�ް�ť������1=�а�ť����
// *Return				:0=�ɹ�������=ʧ�ܣ�
// *History			:2016-05-10,modified by syj*/
 
int kbYPRead(int DEVx, int *value)
{
 	//�ж��豸ѡ��
 	if(!(DEV_SWITCH_SELA1 == DEVx || DEV_SWITCH_SELA2 == DEVx || DEV_SWITCH_SELA3 == DEVx ||\
 		DEV_SWITCH_SELB1 == DEVx || DEV_SWITCH_SELB2 == DEVx || DEV_SWITCH_SELB3 == DEVx ||\
 		DEV_SWITCH_LOCKA == DEVx || DEV_SWITCH_LOCKB == DEVx))
 	{
 		printf("%s:%d:�豸ѡ��Ƿ�!\n", __FUNCTION__, __LINE__);
 		return ERROR;
 	}
 
 	//�����û�ID
 	switch(DEVx)
 	{
 	case DEV_SWITCH_SELA1:
 		*value = KbSwitch.ValueYPA1;
 		KbSwitch.ValueYPA1 = 0;
 		break;
 	case DEV_SWITCH_SELA2:
 		*value = KbSwitch.ValueYPA2;
 		KbSwitch.ValueYPA2 = 0;
 		break;
 	case DEV_SWITCH_SELA3:
 		*value = KbSwitch.ValueYPA3;
 		KbSwitch.ValueYPA3 = 0;
 		break;
 	case DEV_SWITCH_SELB1:
 		*value = KbSwitch.ValueYPB1;
 		KbSwitch.ValueYPB1 = 0;
 		break;
 	case DEV_SWITCH_SELB2:
 		*value = KbSwitch.ValueYPB2;
 		KbSwitch.ValueYPB2 = 0;
 		break;
 	case DEV_SWITCH_SELB3:
 		*value = KbSwitch.ValueYPB3;
 		KbSwitch.ValueYPB3 = 0;
 		break;
 	case DEV_SWITCH_LOCKA:
 		*value = KbSwitch.ValueLockA;
 		KbSwitch.ValueLockA = 0;
 		break;
 	case DEV_SWITCH_LOCKB:
 		*value = KbSwitch.ValueLockB;
 		KbSwitch.ValueLockB = 0;
 		break;
 	default:
 		break;
 	}
 
 	return 0;
 }
 
 
// //*******************************************************************
// *Name				:kbSetTransmit
// *Description		:����ͨѶ���ö˿����ݣ��˽ӿڷ�װ���ݲ����͵����̶˿ں�ȴ����ػ�ʱ2��
// *Input				:KEYx		����ѡ��0=A����	1=B����
// *						:inbuffer	��Ҫ���͵�����
// *						:nbytes		��Ҫ���͵����ݳ���
// *Output			:outbuffer	���յ�������
// *						:outbytes	���յ������ݳ���
// *Return				:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
// *History			:2015-07-01,modified by syj*/

int kbSetTransmit(int KEYx, unsigned char *inbuffer, int nbytes, unsigned char *outbuffer, int *outbytes)
{
 	KbParamStructType *kbparam=NULL;
 	char tx_buffer[1024]={0};
 	int tx_len=0, i=0, ilength=0, istate=0;
 	unsigned int timer=0;
 
 	//�жϼ����豸ѡ��
 	if(0==KEYx)			
		kbparam=&KbParamA1;
 	else if(1==KEYx)	
		kbparam=&KbParamB1;
 	else						
		return 3;
 
	//fj:
 	//if(NULL == kbparam->SemIdSet)
 	//{
 	//	return ERROR;
 	//}
 
    //semTake(kbparam->SemIdSet, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&kbparam->SemIdSet);
 
 	//�����������
 	memset(kbparam->SetBuffer, 0, sizeof(kbparam->SetBuffer));	
	kbparam->SetLen=0;	
	kbparam->SetPack=0;
 
 	//��֯����
 	for(i=0; i<nbytes && i<1024/2; i++)
 	{
 		tx_buffer[i*2+0]=(KB_PORT_SET<<5)|(KB_DATA_HI<<4)|((inbuffer[i]>>4)&0x0f);		tx_len++;
 		tx_buffer[i*2+1]=(KB_PORT_SET<<5)|(KB_DATA_LOW<<4)|((inbuffer[i]>>0)&0x0f);	tx_len++;
 	}
 
 	//��������
 	timer = 0;
 	if(0==KEYx)
	{
		ilength=comWrite(COM4, tx_buffer, tx_len);
	}
 	else
	{
		ilength=comWrite(COM5, tx_buffer, tx_len);
	}
 
 	//�ȴ���������
 	while(1)
 	{
 		if(1==kbparam->SetPack && kbparam->SetLen-4>0 && 0x04==kbparam->SetBuffer[2])
 		{
 			memcpy(outbuffer, kbparam->SetBuffer, kbparam->SetLen);	
			*outbytes=kbparam->SetLen;
 			goto DONE;
 		}
 		else if(timer>=5*ONE_SECOND)
 		{
 			istate=2;
 			goto DONE;
 		}
 
 		timer+=(10*ONE_MILLI_SECOND);
		usleep(10000);
 		//taskDelay(10*ONE_MILLI_SECOND); //fj:
 	}
 
 DONE:
 	//semGive(kbparam->SemIdSet);
	pthread_mutex_unlock(&kbparam->SemIdSet); //fj:20170925
 
 	return istate;
}
 
 
// //*******************************************************************
// *Name				:kbSetPsamTransmit
// *Description		:�����PSAM֮���ͨѶ
// *Input				:KEYx		����ѡ��0=A����	1=B����
// *						:inbuffer	��Ҫ���͵�����
// *						:nbytes		��Ҫ���͵����ݳ���
// *Output			:outbuffer	���յ�������
// *						:outbytes	���յ������ݳ���
// *Return				:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
// *History			:2015-07-01,modified by syj*/
 
int kbSetPsamTransmit(int KEYx, unsigned char *inbuffer, int nbytes, unsigned char *outbuffer, int *outbytes)
{
 	char tx_buffer[1024]={0};
 	int tx_len=0, istate=0;
 	char recv_buffer[1024]={0};
 	int recv_len=0;
	int  i = 0;
 
 	//����֡
 	tx_buffer[0]=0xF9;	tx_buffer[1]=2+nbytes;	tx_buffer[2]=0x04;	
 	memcpy(&tx_buffer[3], inbuffer, nbytes);
 	tx_buffer[3+nbytes]=xorGet(&tx_buffer[1], 2+nbytes);
 	tx_len=4+nbytes;
 
 	//���Ͳ���ȡ���أ��ж��Ƿ�Ϊ��������(������Ϊ0x04�����ص������а�����Ч����)
 	istate=kbSetTransmit(KEYx, tx_buffer, tx_len, recv_buffer, &recv_len);
 	if(0==istate && 0x04==recv_buffer[2] && recv_len-4 > 0)
 	{
 		memcpy(outbuffer, &recv_buffer[3], recv_len-4);	*outbytes=recv_len-4;
 	}
 	else if(1==istate || 2==istate || 3==istate || 4==istate)	
		istate = istate;
 	else istate=ERROR;
 
 	return istate;
}


/*******************************************************************
*Name				:kbInit
*Description		:����ģ���ʼ��
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
bool kbInit(void)
{
	int tIdDspA1 = 0, tIdDspB1 = 0;

	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamA1.KeyLock=ERROR;	
	KbParamA1.KeyLockChg=0;	   
	KbParamA1.OilLeft=ERROR;	
	KbParamA1.OilLeftChg=0;
	KbParamA1.OilRight=ERROR;	
	KbParamA1.OilRightChg=0;	
	KbParamA1.OilConfirm=ERROR;
	KbParamA1.OilConfirmChg=0;

	lstInit(&KbButtonListA1);	//���������������Ա�����յ��İ���

	//���ö˿����ݳ�ʼ��
	int nMutexInitRet = pthread_mutex_init(&KbParamA1.SemIdSet,NULL); //fj:
	if(nMutexInitRet != 0)
	{	
		jljUserLog("Error!Creat semaphore 'KbParamA1.SemIdSet' error!\n");
		return false;
	}

	RngRxUsartA1=rngCreate(KB_RNG_MAX_LEN);	//������չ����(��չA1)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartA1))	
	{
		rngFlush(RngRxUsartA1);
	}
	else
	{
		printf("Error! Create 'RngRxUsartA1' fail! \n");
		return false;
	}

	RngRxUsartA2=rngCreate(KB_RNG_MAX_LEN); 	//������չ����(��չA2)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartA2))	
	{
		rngFlush(RngRxUsartA2);
	}
	else
	{
		printf("Error! Create 'RngRxUsartA2' fail! \n");
		return false;
	}
		
	RngRxUsartA4=rngCreate(KB_RNG_MAX_LEN);	//������չ����(��չA4)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartA4))	
	{
		rngFlush(RngRxUsartA4);
	}
	else
	{
		printf("Error! Create 'RngRxUsartA4' fail! \n");
		return false;
	}
	
	RngRxUsartA5=rngCreate(KB_RNG_MAX_LEN);//������չ����(��չA5)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartA5))	
	{
		rngFlush(RngRxUsartA5);
	}
	else
	{
		printf("Error! Create 'RngRxUsartA5' fail! \n");
		return false;
	}

	msgIdDspA1 = msgget(IPC_PRIVATE,IPC_CREAT|IPC_EXCL|0666);
	if(msgIdDspA1 < 0)
	{
		printf("Error!	Creat messages  'msgIdDspA1' fail!\n");
		perror("get msgIdDspA1 is error");
		return false;
	}
	else
	{
		printf("------keyboard dsp:create mssage msgIdDspA1 is success\n");
	}

	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamB1.KeyLock=ERROR;	
	KbParamB1.KeyLockChg=0;	
	KbParamB1.OilLeft=ERROR;	
	KbParamB1.OilLeftChg=0;
	KbParamB1.OilRight=ERROR;	
	KbParamB1.OilRightChg=0;	
	KbParamB1.OilConfirm=ERROR;
	KbParamB1.OilConfirmChg=0;

	lstInit(&KbButtonListB1);//���������������Ա�����յ��İ���

	nMutexInitRet = pthread_mutex_init(&KbParamB1.SemIdSet,NULL);
	if(nMutexInitRet != 0)
	{
	    jljUserLog("Error!Creat semaphore 'KbParamB1.SemIdSet' error!\n");
		return false;
	}
	
	RngRxUsartB1=rngCreate(KB_RNG_MAX_LEN);//������չ����(��չB1)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartB1))
	{
		rngFlush(RngRxUsartB1);
	}
	else
	{
		printf("Error! Create 'RngRxUsartB1' fail! \n");
		return false;
	}
	
	RngRxUsartB2=rngCreate(KB_RNG_MAX_LEN);//������չ����(��չA2)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartB2))	
	{
		rngFlush(RngRxUsartB2);
	}
	else
	{
		printf("Error! Create 'RngRxUsartB2' fail! \n");
		return false;
	}

	RngRxUsartB4=rngCreate(KB_RNG_MAX_LEN);	//������չ����(��չA4)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartB4))	
	{
		rngFlush(RngRxUsartB4);
	}
	else	
	{
		printf("Error! Create 'RngRxUsartB4' fail! \n");
		return false;
	}
	
	RngRxUsartB5=rngCreate(KB_RNG_MAX_LEN);	//������չ����(��չA5)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartB5))
	{
		rngFlush(RngRxUsartB5);
	}
	else		
	{
		printf("Error! Create 'RngRxUsartB5' fail! \n");
	}

	msgIdDspB1 = msgget(IPC_PRIVATE,IPC_CREAT|IPC_EXCL|0666);
	if(msgIdDspB1 < 0)
	{
		printf("Error!	Creat messages  'msgIdDspB1' fail!\n");
		perror("get msgIdDspB1 is error");
		return false;
	}
	else
	{
		printf("------keyboard dsp:create mssage msgIdDspB1 is success\n");
	}

	return true;
}
/*
void kbInit(void)
{*/
	/*int tIdDspA1 = 0, tIdDspB1 = 0;

	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamA1.KeyLock=ERROR;	KbParamA1.KeyLockChg=0;	KbParamA1.OilLeft=ERROR;	KbParamA1.OilLeftChg=0;
	KbParamA1.OilRight=ERROR;	KbParamA1.OilRightChg=0;	KbParamA1.OilConfirm=ERROR;	KbParamA1.OilConfirmChg=0;

	//���������������Ա�����յ��İ���
	lstInit(&KbButtonListA1);

	//���ö˿����ݳ�ʼ��
	KbParamA1.SemIdSet=semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==KbParamA1.SemIdSet)	jljUserLog("Error!Creat semaphore 'KbParamA1.SemIdSet' error!\n");

	//������չ����(��չA1)���������λ������Ի��洮������
	RngRxUsartA1=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA1))	rngFlush(RngRxUsartA1);
	else											printf("Error! Create 'RngRxUsartA1' fail! \n");

	//������չ����(��չA2)���������λ������Ի��洮������
	RngRxUsartA2=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA2))	rngFlush(RngRxUsartA2);
	else											printf("Error! Create 'RngRxUsartA2' fail! \n");	
		
	//������չ����(��չA4)���������λ������Ի��洮������
	RngRxUsartA4=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA4))	rngFlush(RngRxUsartA4);
	else											printf("Error! Create 'RngRxUsartA4' fail! \n");

	//������չ����(��չA5)���������λ������Ի��洮������
	RngRxUsartA5=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA5))	rngFlush(RngRxUsartA5);
	else											printf("Error! Create 'RngRxUsartA5' fail! \n");

	//��ʾ�˿�ͨѶ�ź���
	semIdDspA1 = semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL == semIdDspA1)	printf("%s:%d: creat semaphore 'semIdDspA1' fail!\n");

	//���������ͨѶ����COM4���ݽ�������
	TaskIdRxCom4=taskSpawn("tRxKeyA1",	150, 0, 0x4000, (FUNCPTR)tRxCom4, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(TaskIdRxCom4))	printf("Error! Create task 'tRxKeyA1' fail!\n");	

	//����A1������ʾ��Ϣ����
	msgIdDspA1 = msgQCreate(10, 512, MSG_Q_FIFO);
	if(NULL == msgIdDspA1)	printf("Error!	Creat messages  'msgIdDspA1' fail!\n");
	//����A1������ʾ����
	tIdDspA1=taskSpawn("tKbDspA1",	156, 0, 0x4000, (FUNCPTR)tKbDsp, DEV_DSP_KEYA,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(tIdDspA1))	printf("Error! Create task 'tIdDspA1' fail!\n");








	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamB1.KeyLock=ERROR;	KbParamB1.KeyLockChg=0;	KbParamB1.OilLeft=ERROR;	KbParamB1.OilLeftChg=0;
	KbParamB1.OilRight=ERROR;	KbParamB1.OilRightChg=0;	KbParamB1.OilConfirm=ERROR;	KbParamB1.OilConfirmChg=0;

	//���������������Ա�����յ��İ���
	lstInit(&KbButtonListB1);

	//���ö˿����ݳ�ʼ��
	KbParamB1.SemIdSet=semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==KbParamB1.SemIdSet)	jljUserLog("Error!Creat semaphore 'KbParamB1.SemIdSet' error!\n");

	//������չ����(��չB1)���������λ������Ի��洮������
	RngRxUsartB1=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB1))	rngFlush(RngRxUsartB1);
	else											printf("Error! Create 'RngRxUsartB1' fail! \n");

	//������չ����(��չA2)���������λ������Ի��洮������
	RngRxUsartB2=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB2))	rngFlush(RngRxUsartB2);
	else											printf("Error! Create 'RngRxUsartB2' fail! \n");

	//������չ����(��չA4)���������λ������Ի��洮������
	RngRxUsartB4=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB4))	rngFlush(RngRxUsartB4);
	else											printf("Error! Create 'RngRxUsartB4' fail! \n");
	
	//������չ����(��չA5)���������λ������Ի��洮������
	RngRxUsartB5=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB5))	rngFlush(RngRxUsartB5);
	else												printf("Error! Create 'RngRxUsartB5' fail! \n");

	//��ʾ�˿�ͨѶ�ź���
	semIdDspB1 = semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL == semIdDspB1)	printf("%s:%d: creat semaphore 'semIdDspB1' fail!\n");

	//���������ͨѶ����COM5���ݽ�������
	TaskIdRxCom5=taskSpawn("tRxKeyB1",	150, 0, 0x4000, (FUNCPTR)tRxCom5, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(TaskIdRxCom5))	printf("Error! Create task 'tRxKeyB1' fail!\n");

	//����B1������ʾ��Ϣ����
	msgIdDspB1 = msgQCreate(10, 512, MSG_Q_FIFO);
	if(NULL == msgIdDspB1)	printf("Error!	Creat messages  'msgIdDspB1' fail!\n");
	//����B1������ʾ����
	tIdDspB1=taskSpawn("tKbDspB1",	156, 0, 0x4000, (FUNCPTR)tKbDsp, DEV_DSP_KEYB,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(tIdDspB1))	printf("Error! Create task 'tIdDspB1' fail!\n");


	//������ʱ��ȡIO״̬������
	TaskIdIORead=taskSpawn("tKeyIO",	150, 0, 0x1000, (FUNCPTR)kbTaskSwitchRead, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(TaskIdIORead))	printf("Error! Create task 'tKeyIO' failed!\n");*/

/*	int tIdDspA1 = 0, tIdDspB1 = 0;

	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamA1.KeyLock=ERROR;	KbParamA1.KeyLockChg=0;	    KbParamA1.OilLeft=ERROR;	KbParamA1.OilLeftChg=0;
	KbParamA1.OilRight=ERROR;	KbParamA1.OilRightChg=0;	KbParamA1.OilConfirm=ERROR;	KbParamA1.OilConfirmChg=0;

	//���������������Ա�����յ��İ���
	lstInit(&KbButtonListA1);

	//���ö˿����ݳ�ʼ��
	//KbParamA1.SemIdSet=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==KbParamA1.SemIdSet)	jljUserLog("Error!Creat semaphore 'KbParamA1.SemIdSet' error!\n");
	
	pthread_mutex_init(&KbParamA1.SemIdSet,NULL); //fj:

	//������չ����(��չA1)���������λ������Ի��洮������
	RngRxUsartA1=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA1))	
		rngFlush(RngRxUsartA1);
	else											
		printf("Error! Create 'RngRxUsartA1' fail! \n");

	RngRxUsartA2=rngCreate(KB_RNG_MAX_LEN); 	//������չ����(��չA2)���������λ������Ի��洮������
	if(!(NULL==RngRxUsartA2))	
		rngFlush(RngRxUsartA2);
	else											
		printf("Error! Create 'RngRxUsartA2' fail! \n");	
		
	//������չ����(��չA4)���������λ������Ի��洮������
	RngRxUsartA4=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA4))	
		rngFlush(RngRxUsartA4);
	else											
		printf("Error! Create 'RngRxUsartA4' fail! \n");

	//������չ����(��չA5)���������λ������Ի��洮������
	RngRxUsartA5=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartA5))	
		rngFlush(RngRxUsartA5);
	else											
		printf("Error! Create 'RngRxUsartA5' fail! \n");

	msgIdDspA1 = msgget(IPC_PRIVATE,IPC_CREAT|IPC_EXCL|0666);
	if(msgIdDspA1 < 0)
	{
		printf("Error!	Creat messages  'msgIdDspA1' fail!\n");
		perror("get msgIdDspA1 is error");
		return false;
	}
	else
	{
		printf("create mssage msgIdDspA1 is success\n");
	}

	//��ʾ�˿�ͨѶ�ź���,fj:�ѳ�ʼ��������
	//semIdDspA1 = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == semIdDspA1)	printf("%s:%d: creat semaphore 'semIdDspA1' fail!\n");

	//���������ͨѶ����COM4���ݽ�������,��AppPacket��ʵ��
	//TaskIdRxCom4=taskSpawn("tRxKeyA1",	150, 0, 0x4000, (FUNCPTR)tRxCom4, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(TaskIdRxCom4))	printf("Error! Create task 'tRxKeyA1' fail!\n");	

	//����A1������ʾ��Ϣ����
	//msgIdDspA1 = msgQCreate(10, 512, MSG_Q_FIFO);
	//if(NULL == msgIdDspA1)	printf("Error!	Creat messages  'msgIdDspA1' fail!\n");

	

	//����A1������ʾ����,fj:��AppPacket
	//tIdDspA1=taskSpawn("tKbDspA1",	156, 0, 0x4000, (FUNCPTR)tKbDsp, DEV_DSP_KEYA,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tIdDspA1))	printf("Error! Create task 'tIdDspA1' fail!\n");







	//������״̬����Ʒѡ�񿪹�ֵ�ȳ�ʼ��
	KbParamB1.KeyLock=ERROR;	KbParamB1.KeyLockChg=0;	KbParamB1.OilLeft=ERROR;	KbParamB1.OilLeftChg=0;
	KbParamB1.OilRight=ERROR;	KbParamB1.OilRightChg=0;	KbParamB1.OilConfirm=ERROR;	KbParamB1.OilConfirmChg=0;

	//���������������Ա�����յ��İ���
	lstInit(&KbButtonListB1);

	pthread_mutex_init(&KbParamB1.SemIdSet,NULL);

	//���ö˿����ݳ�ʼ��
	//KbParamB1.SemIdSet=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==KbParamB1.SemIdSet)	jljUserLog("Error!Creat semaphore 'KbParamB1.SemIdSet' error!\n");

	//������չ����(��չB1)���������λ������Ի��洮������
	RngRxUsartB1=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB1))	rngFlush(RngRxUsartB1);
	else											printf("Error! Create 'RngRxUsartB1' fail! \n");

	//������չ����(��չA2)���������λ������Ի��洮������
	RngRxUsartB2=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB2))	rngFlush(RngRxUsartB2);
	else											printf("Error! Create 'RngRxUsartB2' fail! \n");

	//������չ����(��չA4)���������λ������Ի��洮������
	RngRxUsartB4=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB4))	rngFlush(RngRxUsartB4);
	else											printf("Error! Create 'RngRxUsartB4' fail! \n");
	
	//������չ����(��չA5)���������λ������Ի��洮������
	RngRxUsartB5=rngCreate(KB_RNG_MAX_LEN);
	if(!(NULL==RngRxUsartB5))	rngFlush(RngRxUsartB5);
	else												printf("Error! Create 'RngRxUsartB5' fail! \n");

	msgIdDspB1 = msgget(IPC_PRIVATE,IPC_CREAT|IPC_EXCL|0666);
	if(msgIdDspB1 < 0)
	{
		printf("Error!	Creat messages  'msgIdDspB1' fail!\n");
		perror("get msgIdDspB1 is error");
		return false;
	}
	else
	{
		printf("create mssage msgIdDspB1 is success\n");
	}

	//��ʾ�˿�ͨѶ�ź���,fj��ʼ�ѳ�ʼ��
	//semIdDspB1 = semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL == semIdDspB1)	printf("%s:%d: creat semaphore 'semIdDspB1' fail!\n");


	//���������ͨѶ����COM5���ݽ�������
	//TaskIdRxCom5=taskSpawn("tRxKeyB1",	150, 0, 0x4000, (FUNCPTR)tRxCom5, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(TaskIdRxCom5))	printf("Error! Create task 'tRxKeyB1' fail!\n");

	//����B1������ʾ��Ϣ����
	//msgIdDspB1 = msgQCreate(10, 512, MSG_Q_FIFO);
	//if(NULL == msgIdDspB1)	printf("Error!	Creat messages  'msgIdDspB1' fail!\n");
	
	//����B1������ʾ����,fj:
	//tIdDspB1=taskSpawn("tKbDspB1",	156, 0, 0x4000, (FUNCPTR)tKbDsp, DEV_DSP_KEYB,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(tIdDspB1))	printf("Error! Create task 'tIdDspB1' fail!\n");


	//������ʱ��ȡIO״̬������,fj:
	//TaskIdIORead=taskSpawn("tKeyIO",	150, 0, 0x1000, (FUNCPTR)kbTaskSwitchRead, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(TaskIdIORead))	printf("Error! Create task 'tKeyIO' failed!\n");

	return;
}*/

