//#include "../inc/main.h"
//#include "../inc/oilCfg.h"
//#include "../inc/oilDsp.h"
//#include "../inc/oilKb.h"
//#include <pthread.h>

//del #include "oilTerminalDevice.h"
//del #include "oilCom.h"
//del #include "oilPC.h"

#include "../inc/main.h"

//yym �������ݶ����ڡ�oilCom.h���ļ���
#define COM7		DEV_COM7						//COM7(ARM7_UART1)	7	����
#define COM8		DEV_COM8						//COM8(ARM7_UART2)	8	����
//yym �������ݶ����ڡ�oilPC.h���ļ���
#define PC_FUN_GRADE_OK				0x30			/*������Ʒȷ��*/
#define PC_FUN_GRADE_NO				0x31			/*������Ʒȷ��*/
//yym �������ݶ����ڡ�oilTerminalDevice.h���ļ���
//yym ��ע����ð�buffer���������ʹ�unsigned char*�ĳ�char*
//int tdDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, char *buffer, int nbytes) {return 0;};
//int tdDsp(int DEV_DSP_KEYx, int Contrast, int IsClr) {return 0;};



/*��ʾ��װ���ݽڵ㶨��*/
typedef struct
{
	NODE	Ndptrs;
	unsigned char buffer[DSP_LEN_MAX];
	int len;
}DspDataNodeType;

/*��ʾ�������ݽṹ*/
typedef struct
{
	/*��ʾ��ѡ��0=���̵�������1=androidƽ����ʾ��*/
	unsigned char DEV;
	/*���̵�������ʾ�豸��*/
	int DEVKeyx;
	/*ƽ����ʾͨѶ����*/
	int comFd;
	/*ǰһ����ʾ�Ľ���ID��*/
	int interfaceID;

	/*��ʾ�������ݻ���*/
	unsigned char txBuffer[DSP_LEN_MAX];
	/*��ʾ�������ݳ���*/
	int txLen;
	/*��ʾ���ͱ�ʶ*/
	unsigned char txMark;
	/*���ݷ���֡��*/
	unsigned char txFrame;
	/*���ݷ��Ͳ����ź���*/
	//del SEM_ID semIdTx;
	pthread_mutex_t semIdTx;
	
	/*�������ݻ���*/
	unsigned char rxBuffer[DSP_LEN_MAX];
	/*�������ݳ���*/
	int rxLen;
	/*����������ʶ*/
	unsigned char rxMark;

	/*��ʾ��������ID*/
	int tIdDspRx;
	/*��ʾ����ID*/
	int tIdDsp;
	
	/*��ʾ�Աȶ�*/
	int Contrast;
}DspParamStructType;


/*��ʾ���ܲ���*/
static DspParamStructType DspParamA, DspParamB;

/*��ʾ����ʱ�������*/
unsigned int dspTimerA=0, dspTimerB=0;


/********************************************************************
*Name				:tdspRecive
*Description		:��ʾ����ͨѶ����
*Input				:nozzle	����
*Output			:None
*Return				:None
*History			:2014-10-10,modified by syj
*/
static void tdspRecive(int nozzle)
{
	unsigned char rxbuffer[16]={0};
	int rxlen=0;
	int datalen=0, crc=0;
	DspParamStructType *param=NULL;

	/*�ж�ǹѡ*/
	if(0==nozzle)		param=&DspParamA;
	else if(1==nozzle)	param=&DspParamB;
	else				return;

	while ( 1 )
	{
		/*��ȡ������Э�黺������*/
		rxlen=comRead(param->comFd, (char*)rxbuffer, 1);
//printf("__%x\n", rxbuffer[0]);
		if(rxlen>0)
		{
			/*�������������*/
			if(param->rxLen>=DSP_LEN_MAX)	param->rxLen=0;

			/*���沢У������*/
			param->rxBuffer[param->rxLen]=rxbuffer[0];
			if((0==param->rxLen)&&(0xfa==rxbuffer[0]))	param->rxLen++;
			else	if(0!=param->rxLen)										param->rxLen++;

			datalen=(param->rxBuffer[1]<<8)|(param->rxBuffer[2]<<0);
			if((param->rxLen>=8)&&(param->rxLen>=(5+datalen)))
			{
		
				crc=(param->rxBuffer[3+datalen]<<8)|(param->rxBuffer[4+datalen]<<0);
				if(crc==crc16Get(&param->rxBuffer[1], 2+datalen))
				{
					param->rxMark=1;
				}
				else
				{
					param->rxLen=0;	param->rxMark=0;
				}
			}
		}
		
		usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:tdsp
*Description		:��ʾ����ͨѶ
*Input				:nozzle	ǹѡ0=1�ţ�1=2��
*Output			:None
*Return				:None
*History			:2014-10-10,modified by syj
*/
static void tdsp(int nozzle)
{
	DspParamStructType *dsp=NULL;
	unsigned int *timer=NULL;

	/*�ж�ǹѡ*/
	if(0==nozzle)			{dsp=&DspParamA;	timer=&dspTimerA;}
	else if(1==nozzle)	{dsp=&DspParamB;	timer=&dspTimerB;}
	else							return;

	while ( 1 )
	{
		/*�µķ��ͻ����ʱ�䣬ˢ����ʾӦ��һ�����*/
		if(*timer>=ONE_SECOND && dsp->txLen>0 && DSP_DEV_PCMONITOR==dsp->DEV)
		{
			//semTake(dsp->semIdTx, WAIT_FOREVER);
			
			dsp->rxLen=0;	dsp->rxMark=0;	*timer=0;	
			comWrite(dsp->comFd, (char*)dsp->txBuffer, dsp->txLen);
			
			//semGive(dsp->semIdTx);
		}
		
		usleep(5*1000);
	}
	
	return;
}

/********************************************************************
*Name				:dspVideo
*Description		:������Ƶ
*Input				:nozzle	ǹѡ0=1�ţ�1=2��
*Output			:None
*Return				:�ɹ�����0��������ʾʧ��
*History			:2014-10-10,modified by syj
*/
int dspVideo(int nozzle)
{
	DspParamStructType *dsp=NULL;
	unsigned int *timer=NULL;
	unsigned char tx_buffer[DSP_LEN_MAX]={0};
	unsigned int crc=0, tx_len=0;

	/*�ж�����ѡ��*/
	if(0==nozzle)		{dsp=&DspParamA;	timer=&dspTimerA;}
	else if(0==nozzle)	{dsp=&DspParamB;	timer=&dspTimerB;}
	else				return 0;//yym ���ӷ���0��ԭʼ�޷���ֵ

	/*�ж�֡�Ų�Ϊ0xfa*/
	dsp->txFrame++;	if(0xfa==dsp->txFrame)	dsp->txFrame++;	

	tx_buffer[0]=0xfa;
	tx_buffer[1]=(unsigned char)(4>>8);
	tx_buffer[2]=(unsigned char)(4>>0);
	tx_buffer[3]=dsp->txFrame;
	tx_buffer[4]=0x52;
	tx_buffer[5]=0;
	tx_buffer[6]=0;
	crc=crc16Get(&tx_buffer[1], 6);
	tx_buffer[7]=(unsigned char)(crc>>8);
	tx_buffer[8]=(unsigned char)(crc>>0);
	tx_len=9;
	comWrite(dsp->comFd, (char*)tx_buffer, tx_len);

	/*���ͼ����ʱ�����㲢�����ݱ���Ϊ���һ�ʷ�������*/
	//semTake(dsp->semIdTx, WAIT_FOREVER);
	
	*timer=0;
	memcpy(dsp->txBuffer, tx_buffer, tx_len);	dsp->txLen=tx_len;
	
	//semGive(dsp->semIdTx);

	return 0;
}


/********************************************************************
*Name				:dsp
*Description		:��ʾ���ú�����ִ�н������ȷ�ϣ��ɵײ㺯����������ִ�еĳɹ�
*Input				:nozzle	ǹѡ0=1�ţ�1=2��
*						:id			��ʾ����ID
*						:buffer	��ʾ����
*						:nbytes	��ʾ��������
*Output			:None
*Return				:�ɹ�����0��������ʾʧ��
*History			:2014-10-10,modified by syj
*/
int dsp(int nozzle, int id, unsigned char *buffer, int nbytes)
{
	DspParamStructType *dsp=NULL;
	unsigned char tmp_buffer[128]={0};
	unsigned int crc=0, *timer=NULL;
	int i=0, offset_x=0, offset_y=0, temp_len = 0;

	/*���̵�������ʾʱ��Ҫ�Ĳ���*/
	unsigned char dsp_buffer[128]={0};				/*������ʾ����*/
	unsigned char dsp_line_size[4]={0};				/*������ʾ���ݳ���*/
	unsigned int x=0, y=0, dsp_len=0;					/*��ʾ���λ��*/
	unsigned long long bcd_value=0, hex_value=0;

	/*�ж�ǹѡ*/
	if(DSP_NOZZLE_1==nozzle)			{dsp=&DspParamA;	timer=&dspTimerA;	}
	else if(DSP_NOZZLE_2==nozzle)		{dsp=&DspParamB;	timer=&dspTimerB;	}
	else								return 0;//yym ���ӷ���0��ԭʼ�޷���ֵ

//myDEBUG
//	printf("dsp_______");
//	for(i=0; i<nbytes; i++)	printf("_%x", buffer[i]);
//	printf("\n");

	/*
	*���̵�������ʾ
	*���ݽ������֯������ʾ���ݣ�һ���״���ʾʱ�������״�ʱ������
	*�������ڲ�������������Կո񸲸ǲ�����ʾ�ɱ�����
	*/
	if(DSP_DEV_KEYBOARD==dsp->DEV)
	{
		switch(id)
		{
		case DSP_TEXT_INFO:									
			/*
			*	�ı���ʾ����
			*	�ı���ʾ��16*16�ֿ���ʾ��ÿ�������ʾ8�����ֳ��ȣ�
			*	��4�������У�����Ļ��ʼ��ʾλ����������ʾ
			*	������1��ʱ���м�λ�þ�����ʾ��������2��ʱ�ӵڶ��п�ʼ��ʾ
			*/
			memset(dsp_line_size, 0, 4);	x=0;	y=0;
			if(nbytes<=16)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, (16-nbytes)/2*8, (char*)buffer, nbytes);
			}
			else
			{
				/*������2����ʾ�ӵ�2�п�ʼ��ʾ*/
				if(nbytes<=32)			x=1;

				/*����ʾ���ݰ��н��з�װ*/
				for(i=0; i<nbytes;)
				{
					/*�������ĸ,������ĸ��������y����һλ*/
					if(buffer[i]<128)
					{
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*����Ǻ���,�����λ����β������ʾ��ת����һ����ʾ*/
					else
					{
						if(y>=15)	{x++;	y=0;}
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*���y���곬��1����ת����һ��*/
					if(y>=16)	{x++;	y=0;}
					/*���x���곬��4��������ʾʣ�ಿ��*/
					if(x>=4)	break;
				}
				
				/*����ÿ�е���ʾ�����ж��Ƿ���ʾ*/
				for(i=0; i<4; i++)
				{
					if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
				}
			}
			
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_STANDBY:				
			/*
			*	���Ϳ����ʹ�������
			*	�������״���ʾ��ˢ������ʾȫ����Ϣ
			*	��������״���ʾ����ʾ�ɱ�����
			*/
			/*��0����ʾ:����״̬+ʱ�䣬�м���벿�����ո�*/
			if(1==buffer[7])	memcpy(&dsp_buffer[0], "��", 2);
			else						memcpy(&dsp_buffer[0], "  ", 2);
			memcpy(&dsp_buffer[2], "      ", 6);
			hex2Ascii(&buffer[0], 7, tmp_buffer, 14);
			dsp_buffer[8]=tmp_buffer[8];		dsp_buffer[9]=tmp_buffer[9];
			dsp_buffer[10]=':';
			dsp_buffer[11]=tmp_buffer[10];		dsp_buffer[12]=tmp_buffer[11];
			dsp_buffer[13]=':';
			dsp_buffer[14]=tmp_buffer[12];	dsp_buffer[15]=tmp_buffer[13];
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��3����ʾ:"���ͻ����°�"��"�����IC��"��"�����IC����ѡ����Ʒ��ɨ������"*/
#if _TYPE_BIG260_
			if(0==buffer[8])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   ���ͻ��°�   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else	if(0!=buffer[8] && 0==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "   �����IC��   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "��'��'����������", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��ȷ�ϼ���������", 16);
			}
#else
			if(0==buffer[8])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   ���ͻ��°�   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else	if(0!=buffer[8] && 0==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   �����IC��   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else if(0!=buffer[8] && 2==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "�����IC����ѡ��", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "��Ʒ��ɨ������  ", 16);
			}
#endif
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PRETREAT:							
			/*���Ϳ�Ԥ������ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���Ϳ�������", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, "���Ժ�....", 10);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_UNLOCK_FINISH:				
			/*���Ϳ����۴�����ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "�ҿ������۴�����", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_TACCLR_FINISH:
			/*���Ϳ����䴦����ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "���Ϳ����䴦����", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PASSIN:								
			/*���Ϳ������������*/
			/*��2��:��ʾ"�����뿨����:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����뿨����:", 13);
			/*��4��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_CARLIMIT:							
			/*�޳��ſ�Ա�������������*/
			/*��0��:��ʾ"�޳��ſ�"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�޳��ſ�:", 9);
			/*��2��:��ʾ����*/
			for(i=0; i<16; i++)
			{
				if(0xff==buffer[0+i])	break;
			}
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)&buffer[0], i);
			/*��4��:��ʾ"������Ա������:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "������Ա������:", 15);
			/*��6��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[16]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_BALANCE:							
			/*���Ϳ�������*/
			/*��0��:��ʾ�Żݼ۸�*/
			if(0 != memcmp(buffer + 16, "\x00\x00", 2))
			{
				memset(dsp_buffer, 0, sizeof(dsp_buffer));
				strcpy((char*)dsp_buffer, "�Żݼ۸�:");
				bcd_value=hex2Bcd((buffer[16]<<8)|(buffer[17]<<0));
				tmp_buffer[0]=(char)((bcd_value>>12)&0x0f)+0x30;	tmp_buffer[1]=(char)((bcd_value>>8)&0x0f)+0x30;	
				tmp_buffer[2]=(char)((bcd_value>>4)&0x0f)+0x30;	tmp_buffer[3]=(char)((bcd_value>>0)&0x0f)+0x30;
				if('0' == tmp_buffer[0])	*(dsp_buffer + strlen((char*)dsp_buffer)) = ' ';
				else						*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[0];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[1];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = '.';
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[2];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[3];
				strcpy((char*)dsp_buffer + strlen((char*)dsp_buffer), "Ԫ");
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
			}
			/*��2��:��ʾ��Ӧ���������*/
			if(0xff == buffer[18] && 1 == buffer[2])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û������:", strlen("�û������:"));
			else if(0xff == buffer[18] && 2 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������:", strlen("�������:"));
			else if(0xff == buffer[18] && 4 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա�������:", strlen("Ա�������:"));
			else if(0xff == buffer[18] && 5 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ÿ����:", strlen("��ÿ����:"));
			else if(0xff == buffer[18] && 6 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "ά�޿����:", strlen("ά�޿����:"));
			else if(0x01 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û������:", strlen("�û������:"));
			else if(0x04 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա�������:", strlen("Ա�������:"));
			else if(0x05 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ÿ����:", strlen("��ÿ����:"));
			else if(0x21 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û����������:", strlen("�û����������:"));
			else if(0x22 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������������:", strlen("�������������:"));
			else if(0x11 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û���˾�������", strlen("�û���˾�������"));
			else if(0x12 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���˿����:", strlen("���˿����:"));
			else if(0x13 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���������:", strlen("���������:"));
			else if(0x14 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���ÿ����:", strlen("���ÿ����:"));
			else if(0x15 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���������:", strlen("���������:"));
			else if(0x16 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������:", strlen("�������:"));
			else if(0x17 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������˾�������", strlen("������˾�������"));
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���Ϳ����:", strlen("���Ϳ����:"));
			/*��4��:��ʾ�����*/
			bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[14])				memcpy(&dsp_buffer[11], "Ԫ", 2);
			else if(1==buffer[14])		memcpy(&dsp_buffer[11], "��", 2);
			else									memcpy(&dsp_buffer[11], "��", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);
			/*��6��:��ʾ���㷽ʽ��ֻ��Ա�������д�������ʾ*/
			if(4==buffer[2] && 0==buffer[15])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "�ֽ�", 4);
			else if(4==buffer[2] && 1==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��Ʊ", 4);
			else if(4==buffer[2] && 2==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "����ƾ֤", 8);
			else if(4==buffer[2] && 3==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "���п�", 6);
			else if(4==buffer[2] && 4==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "����һ", 6);
			else if(4==buffer[2] && 5==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "������", 6);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_PRESET:								
			/*���Ϳ�����Ԥ�ý���*/
			/*��2��:��ʾ"������Ԥ����:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������Ԥ����:", 13);
			/*��4��:��ʾԤ��������λ*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])		memcpy(&dsp_buffer[11], "Ԫ", 2);
			else if(1==buffer[4])	memcpy(&dsp_buffer[11], "��", 2);
			else					memcpy(&dsp_buffer[11], "��", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_UNIT_SELECT	:
			/*���Ϳ����㵥λѡ�����*/
			/*��2��:��ʾ"1.������Ʊ"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "1.������Ʊ", 10);
			/*��4��:��ʾ"2.����Ӧ��"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, "2.����Ӧ��", 10);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_SETTLE_SELECT:				
			/*���Ϳ����㷽ʽѡ�����*/
			/*��0��:��ʾ"1.�ֽ�  5.����һ"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.�ֽ�  5.����һ", 16);
			/*��2��:��ʾ"2.��Ʊ  6.������"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.��Ʊ  6.������", 16);
			/*��4��:��ʾ"3.����ƾ֤"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.����ƾ֤", 10);
			/*��6��:��ʾ"4.���п�"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.���п�", 8);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_STAFF_PASSIN:					
			/*���Ϳ�����Ա�������������*/
			/*��4��:��ʾ"������Ա������:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������Ա������:", 15);
			/*��6��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_OIL_START:						
			/*���Ϳ�����������ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "����������...", 13);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 8*8, "����ο�", 8);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_OILLING:							
			/*���Ϳ������н���*/
			/*��0��:��ʾԤ����*/
			memcpy(&dsp_buffer[0], "Ԥ��:", 5);
			bcd_value=hex2Bcd((buffer[15]<<24)|(buffer[16]<<16)|(buffer[17]<<8)|(buffer[18]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[19])			memcpy(&dsp_buffer[14], "Ԫ", 2);
			else if(1==buffer[19])		memcpy(&dsp_buffer[14], "��", 2);
			else if(2==buffer[19])		memcpy(&dsp_buffer[14], "��", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, 16);
			/*��2��:��ʾ��Ӧ���������*/
			//if(1==buffer[2])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û������:", 11);
			//else if(2==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������:", 11);
			//else if(4==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա�������:", 11);
			//else if(5==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ÿ����:", 11);
			//else if(6==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "ά�޿����:", 11);
			//else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���Ϳ����:", 11);
			if(0xff == buffer[23] && 1 == buffer[2])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û������:", strlen("�û������:"));
			else if(0xff == buffer[23] && 2 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������:", strlen("�������:"));
			else if(0xff == buffer[23] && 4 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա�������:", strlen("Ա�������:"));
			else if(0xff == buffer[23] && 5 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ÿ����:", strlen("��ÿ����:"));
			else if(0xff == buffer[23] && 6 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "ά�޿����:", strlen("ά�޿����:"));
			else if(0x01 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û������:", strlen("�û������:"));
			else if(0x04 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա�������:", strlen("Ա�������:"));
			else if(0x05 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ÿ����:", strlen("��ÿ����:"));
			else if(0x21 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û����������:", strlen("�û����������:"));
			else if(0x22 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������������:", strlen("�������������:"));
			else if(0x11 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�û���˾�������", strlen("�û���˾�������"));
			else if(0x12 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���˿����:", strlen("���˿����:"));
			else if(0x13 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���������:", strlen("���������:"));
			else if(0x14 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���ÿ����:", strlen("���ÿ����:"));
			else if(0x15 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���������:", strlen("���������:"));
			else if(0x16 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������:", strlen("�������:"));
			else if(0x17 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������˾�������", strlen("������˾�������"));
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���Ϳ����:", strlen("���Ϳ����:"));
			/*��4��:��ʾ�����*/
			bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;		
			if(1==buffer[14])		memcpy(&dsp_buffer[11], "��", 2);
			else if(2==buffer[14])	memcpy(&dsp_buffer[11], "��", 2);
			else					memcpy(&dsp_buffer[11], "Ԫ", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);
			/*��6��:��ʾ�Ƿ����*/
#if _TYPE_BIG260_
			if(1==buffer[20])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "����            ", 16);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��ȷ�ϼ���������", 16);
#else
			if(1==buffer[20])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "����            ", 16);
			else if(0 != memcmp(buffer + 21, "\x00\x00", 2))
			{
				memset(dsp_buffer, 0, sizeof(dsp_buffer));
				strcpy((char*)dsp_buffer, "�Żݼ۸�:");
				bcd_value=hex2Bcd((buffer[21]<<8)|(buffer[22]<<0));
				tmp_buffer[0]=(char)((bcd_value>>12)&0x0f)+0x30;	tmp_buffer[1]=(char)((bcd_value>>8)&0x0f)+0x30;	
				tmp_buffer[2]=(char)((bcd_value>>4)&0x0f)+0x30;	tmp_buffer[3]=(char)((bcd_value>>0)&0x0f)+0x30;
				if('0' == tmp_buffer[0])	*(dsp_buffer + strlen((char*)dsp_buffer)) = ' ';
				else									*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[0];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[1];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = '.';
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[2];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[3];
				strcpy((char*)dsp_buffer + strlen((char*)dsp_buffer), "Ԫ");
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
			}
#endif

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);			
			break;
			
		case DSP_CARD_OIL_FINISH:						
			/*���Ϳ����ͽ�����ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���ͽ�����...", 13);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 8*8, "����ο�", 8);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_OILEND:
			/*���Ϳ�������ɽ���*/
			/*��0��:��ʾ"�������"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "�������", 8);
			/*��2��:��ʾ"���:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���:", 5);
			/*��4��:��ʾ�����*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(1==buffer[4])			memcpy(&dsp_buffer[11], "��", 2);
			else	if(2==buffer[4])	memcpy(&dsp_buffer[11], "��", 2);
			else								memcpy(&dsp_buffer[11], "Ԫ", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_ERR_INFO:
			/*���Ϳ����ͺ�����Ĵ�����ʾ����*/
			if(buffer[2]<=16)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, (16-buffer[2])/2*8, (char*)&buffer[3], buffer[2]);
			else
			{
				/*����ʾ���ݰ��н��з�װ*/
				for(i=0, x=0; i<buffer[2] && i<30;)
				{
					/*�������ĸ,������ĸ��������y����һλ*/
					if(buffer[3+i]<128)
					{
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*����Ǻ���,�����λ����β������ʾ��ת����һ����ʾ*/
					else
					{
						if(y>=15)	x++;
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*���y���곬��1����ת����һ��*/
					if(y>=16)	{x++;	y=0;}
				}
				
				/*����ÿ�е���ʾ�����ж��Ƿ���ʾ*/
				for(i=0; i<2; i++)
				{
					if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
				}
			}

			/*��4����ʾ*/
			hex2Ascii(&buffer[0], 2, dsp_buffer, 4);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_PASSWORD_INPUT:
			/*�����������*/
			/*��2��:��ʾ"����������:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "����������:", 11);
			/*��4��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OPERATE_SELECT:
			/*������ѡ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, "1.��ѯ", 6);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, "2.����", 6);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_SELECT:
			/*��ѯ��ѡ����棬��5ҳ*/
			if(0==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.����", 6);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.������ϸ", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.�ش�ӡ", 8);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.��ǹ��Ϣ", 10);
			}
			else if(1==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "5.�ͻ���Ϣ", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "6.ʱ��", 6);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "7.������Ϣ", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "8.��ӡ��Ϣ", 10);
			}
			else if(2==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "9.˰���ۼ�", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "10.�����ۼ�", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "11.�׼��ۼ�", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "12.˰�񵱴�����", 15);
			}
			else if(3==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "13.˰��ʱ��", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "14.˰������", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "15.˰������", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "16.������Ϣ", 11);
			}
			else if(4==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "17.��ǰ��", 9);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "18.��ʱͣ��ʱ��", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "19.����Ϣ", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "20.������Ϣ", 11);
			}
			else if(5==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "21.��ӡ����СƱ", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "22.��Ʒ����", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "23.�쳣��־", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "24.�ԱȶȲ�ѯ", 13);
			}
			else if(6==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "25.����ģ��Ʒ��", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "26.����������Ϣ", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "27.�ͻ���̨��Ϣ", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "28.ƽ�������Ϣ", 15);
			}

			/*��ҳ��ʾ�Ľ�����Ϊ����ǰһҳ��������������ʾ*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_JLSUM:
			/*��ѯ�������۽���*/
			/*��0��:��ʾ"���۽��:"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "���۽��:", 9);
			
			/*��2��:��ʾ���۽��*/
			hex_value=((long long)buffer[0]<<56)|((long long)buffer[1]<<48)|((long long)buffer[2]<<40)|\
								((long long)buffer[3]<<32)|((long long)buffer[4]<<24)|((long long)buffer[5]<<16)|\
								((long long)buffer[6]<<8)|((long long)buffer[7]<<0);
			/*����BCD�룬������13λ��Ϊ'.'��"Ԫ"Ԥ����ʾ�ռ�*/
			for(i=0; i<13; i++)
			{
				dsp_buffer[12-i]=(char)(hex_value%10)+0x30;
				hex_value=hex_value/10;
			}
			/*���С���㼰��λ��ʾ*/
			memcpy(&dsp_buffer[14], "Ԫ", 2);
			dsp_buffer[13]=dsp_buffer[12];	
			dsp_buffer[12]=dsp_buffer[11];
			dsp_buffer[11]='.';
			/*��λ������0����ʾ*/
			for(i=0; i<10; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ"��������:"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "��������:", 9);

			/*��6��:��ʾ��������*/
			hex_value=((long long)buffer[8]<<56)|((long long)buffer[9]<<48)|((long long)buffer[10]<<40)|\
								((long long)buffer[11]<<32)|((long long)buffer[12]<<24)|((long long)buffer[13]<<16)|\
								((long long)buffer[14]<<8)|((long long)buffer[15]<<0);
			/*����BCD�룬������13λ��Ϊ'.'��"��"Ԥ����ʾ�ռ�*/
			for(i=0; i<13; i++)
			{
				dsp_buffer[12-i]=(char)(hex_value%10)+0x30;
				hex_value=hex_value/10;
			}
			/*���С���㼰��λ��ʾ*/
			memcpy(&dsp_buffer[14], "��", 2);
			dsp_buffer[13]=dsp_buffer[12];	
			dsp_buffer[12]=dsp_buffer[11];
			dsp_buffer[11]='.';
			/*��λ������0����ʾ*/
			for(i=0; i<10; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_SELECT:
			/*��ѯ�˵���ϸ��������ѡ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 4*8, "1.���ͻ�", 8);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, "2.��ǹ", 6);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_TTCINPUT:
			/*��ѯ�˵���ϸTTC�������*/
			/*��2��:��ʾ"������TTC:"*/
			if(id!=dsp->interfaceID)tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������TTC:", 10);

			/*��4��:��ʾTTC��*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<=10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_INDEX:
			/*��ѯ�˵���ϸ��������*/
			/*��0��:��ʾTTC��*/
			memcpy(&dsp_buffer[0], "TTC:", 4);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			for(i=0; i<10; i++)
			{
				dsp_buffer[4+i]=(char)((bcd_value>>(9*4-i*4))&0x0f)+0x30;
			}
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[4+i])	dsp_buffer[4+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��2��:��ʾʱ��*/
			memcpy(&dsp_buffer[0], "T:", 2);	hex2Ascii(&buffer[4], 7, &dsp_buffer[2], 14);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ���ź�16λ*/
			hex2Ascii(&buffer[11], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ���ͽ��+����ǹ��*/
			bcd_value=hex2Bcd((buffer[19]<<16)|(buffer[20]<<8)|(buffer[21]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "Ԫ", 2);

			memset(&dsp_buffer[11], ' ', 3);

			hex2Ascii(&buffer[22], 1, &dsp_buffer[14], 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_DATA:
			/*��ѯ�˵���ϸԭʼ���ݽ���*/
			if(buffer[0]<4){
				/*ǰ��ҳÿҳ��ʾ������ϸ��ĩ����ʾ��ҳ��*/
				hex2Ascii(&buffer[1+8*0+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				hex2Ascii(&buffer[1+8*1+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				hex2Ascii(&buffer[1+8*2+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				dsp_buffer[0]=buffer[0]+1+0x30;	dsp_buffer[1]='/';		dsp_buffer[2]='5';
				dsp_len=3;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 13*8, (char*)dsp_buffer, dsp_len);
			}
			else if(4==buffer[0]){
				/*��5ҳ�����һҳ��ʾһ����ϸ��ĩ����ʾ��ҳ��*/
				hex2Ascii(&buffer[1+8*0+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				memset(dsp_buffer, ' ', 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				memset(dsp_buffer, ' ', 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				dsp_buffer[0]=buffer[0]+1+0x30;	dsp_buffer[1]='/';		dsp_buffer[2]='5';
				dsp_len=3;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 13*8, (char*)dsp_buffer, dsp_len);
			}
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_NOZZLE_INFO:
			/*��ѯ��ǹ��Ϣ����*/
			if(0==buffer[0]){
				
				/*��0��:��ʾ�߼�ǹ��*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "�߼�ǹ��:", 9);
				bcd_value=hex2Bcd(buffer[1]);
				dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
				for(i=0; i<2; i++)
				{
					if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
					else									break;
				}
				dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				/*��2��:��ʾ����ǹ��*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "����ǹ��:", 9);
				bcd_value=hex2Bcd(buffer[2]);
				dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
				for(i=0; i<2; i++)
				{
					if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
					else									break;
				}
				dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				/*��4��:��ʾ����*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "  ����:", 9);
				if(0==buffer[3])			memcpy(&dsp_buffer[9], "  1(A)", 6);
				else if(1==buffer[3])	memcpy(&dsp_buffer[9], "  2(B)", 6);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				/*��6��:��ʾ���¼�ͷ*/
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "��ҳ��", 6);
			}
			else{

				/*��2��:��ʾ��Ʒ����*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "��Ʒ����:", 9);
				hex2Ascii(&buffer[4], 2, &dsp_buffer[9], 4);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				/*��4��:��ʾ��ǰ�ܼ�TTC*/
				memcpy(&dsp_buffer[0], "TTC:  ", 6);
				bcd_value=hex2Bcd((buffer[6]<<24)|(buffer[7]<<16)|(buffer[8]<<8)|(buffer[9]<<0));
				for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
				for(i=0; i<9; i++)
				{
					if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
					else									break;
				}
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				/*��6��:��ʾδ�ϴ��˵�*/
				memcpy(&dsp_buffer[0], "����: ", 6);
				bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
				for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
				for(i=0; i<9; i++)
				{
					if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
					else									break;
				}
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				/*��6��:��ʾ���ϼ�ͷ*/
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "��ҳ��", 6);
			}

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BOARD_INFO:
			/*������Ϣ����*/
			/*��0��:��ʾ�����*/
			memcpy(&dsp_buffer[0], "�����:      ", 13);	
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[13]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[14]=(char)((bcd_value>>4)&0x0f)+0x30;
			if('0'==dsp_buffer[13] && '0'!=dsp_buffer[14])			{dsp_buffer[13]=' ';}
			else if('0'==dsp_buffer[13] && '0'==dsp_buffer[14])	{dsp_buffer[13]=' ';	dsp_buffer[14]=' ';}
			dsp_buffer[15]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��2��:��ʾ��ǰ�ܼ�TTC*/
			memcpy(&dsp_buffer[0], "TTC:  ", 6);
			bcd_value=hex2Bcd((buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]<<0));
			for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾδ�ϴ��˵�*/
			memcpy(&dsp_buffer[0], "����: ", 6);
			bcd_value=hex2Bcd((buffer[5]<<24)|(buffer[6]<<16)|(buffer[7]<<8)|(buffer[8]<<0));
			for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6����ʾ���¼���ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "������Ϣ�밴����", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_TIME:
			/*ʱ�����*/
			/*��2��:��ʾ����*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾʱ��*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_buffer[5]=':';
			dsp_buffer[6]=tmp_buffer[12];	dsp_buffer[7]=tmp_buffer[13];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VOICE:
			/*��ѯ������Ϣ����*/
			/*��0��:��ʾ����������*/
			memcpy(&dsp_buffer[0], "  ������:", 9);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[9], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[9], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[9], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[9], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[9], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[9], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[9], "      ", 6);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��2��:��ʾ��������*/
			memcpy(&dsp_buffer[0], "��������:", 9);
			if(0==buffer[1])			memcpy(&dsp_buffer[9], "Ů��", 4);
			else if(1==buffer[1])	memcpy(&dsp_buffer[9], "����", 4);
			else								memcpy(&dsp_buffer[9], "    ", 4);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��������*/
			memcpy(&dsp_buffer[0], "��������:", 9);
			bcd_value=hex2Bcd(buffer[2]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
				else									break;
			}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;	
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT:
			/*��ѯ��ӡ��Ϣ����*/
			/*��0��:��ʾ��ӡ��*/
			memcpy(&dsp_buffer[0], "  ��ӡ��:", 9);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[9], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[9], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[9], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[9], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[9], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[9], "3-2(B)", 6);
			else						memcpy(&dsp_buffer[9], "      ", 6);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��2��:��ʾ�Ƿ��Զ���ӡ*/
			memcpy(&dsp_buffer[0], "�Զ���ӡ:", 9);
			if(0==buffer[1])		memcpy(&dsp_buffer[9], "��", 2);
			else if(1==buffer[1])	memcpy(&dsp_buffer[9], "��", 2);
			else					memcpy(&dsp_buffer[9], "  ", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��ӡ����*/
			memcpy(&dsp_buffer[0], "��ӡ����:", 9);
			if(0==buffer[2])		memcpy(&dsp_buffer[9], "1��", 3);
			else if(1==buffer[2])	memcpy(&dsp_buffer[9], "2��", 3);
			else					memcpy(&dsp_buffer[9], "   ", 3);
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��ȷ�ϼ���ѯ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "ȷ�ϼ�", 6);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT_CARD:
			/*��ѯ��ӡ��Ϣ������ѡ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "��ѡ������", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "1.�û���4.��ÿ�", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "2.����5.ά�޿�", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "3.Ա����", 8);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT_BILLTYPE:
			/*��ѯ��ӡ��Ϣ������ѡ�����*/
			hex_value=(buffer[1]<<8)|(buffer[2]<<0);
			if(0==buffer[0])
			{
				/*��һҳǰ������ʾ�˵����ͣ�ĩβ��ʾ���·�ҳ*/
				if(0==((hex_value>>0)&1))			memcpy(&dsp_buffer[0], "����    ", 8);
				else if(1==((hex_value>>0)&1))		memcpy(&dsp_buffer[0], "������  ", 8);
				if(0==((hex_value>>1)&1))			memcpy(&dsp_buffer[8], "�ӿ�    ", 8);
				else if(1==((hex_value>>1)&1))		memcpy(&dsp_buffer[8], "�ӿ���  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
				
				if(0==((hex_value>>2)&1))			memcpy(&dsp_buffer[0], "��    ", 8);
				else if(1==((hex_value>>2)&1))		memcpy(&dsp_buffer[0], "����  ", 8);
				if(0==((hex_value>>3)&1))			memcpy(&dsp_buffer[8], "����    ", 8);
				else if(1==((hex_value>>3)&1))		memcpy(&dsp_buffer[8], "���ۡ�  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);
				
				if(0==((hex_value>>4)&1))			memcpy(&dsp_buffer[0], "����    ", 8);
				else if(1==((hex_value>>4)&1))		memcpy(&dsp_buffer[0], "�����  ", 8);
				if(0==((hex_value>>5)&1))			memcpy(&dsp_buffer[8], "�ϰ�    ", 8);
				else if(1==((hex_value>>5)&1))		memcpy(&dsp_buffer[8], "�ϰ��  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 14*8, "��", 2);
			}
			else
			{
				/*�ڶ�ҳǰ������ʾ�˵����ͣ�ĩβ��ʾ���Ϸ�ҳ*/
				if(0==((hex_value>>6)&1))			memcpy(&dsp_buffer[0], "�°�    ", 8);
				else if(1==((hex_value>>6)&1))		memcpy(&dsp_buffer[0], "�°��  ", 8);
				if(0==((hex_value>>7)&1))			memcpy(&dsp_buffer[8], "�ǿ�    ", 8);
				else if(1==((hex_value>>7)&1))		memcpy(&dsp_buffer[8], "�ǿ���  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				if(0==((hex_value>>8)&1))			memcpy(&dsp_buffer[0], "�ͼ۽��ա�", 10);
				else if(1==((hex_value>>8)&1))		memcpy(&dsp_buffer[0], "�ͼ۽��ա�", 10);
				dsp_len=10;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				if(0==((hex_value>>9)&1))			memcpy(&dsp_buffer[0], "����ܾ���", 10);
				else if(1==((hex_value>>9)&1))		memcpy(&dsp_buffer[0], "����ܾ���", 10);
				dsp_len=10;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 14*8, "��", 2);
			}

			/*��ҳ��ʾ�Ľ�����Ϊ����ǰһҳ��������������ʾ*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_MONTH_INPUT:
			/*�·��������*/
			/*��2��:��ʾ�������·�*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "�������·�", 10);

			/*��4��:��ʾ�·�*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_DATE_INPUT:
			/*�����������*/
			/*��0��:��ʾ����������*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "����������", 10);

			/*��2��:��ʾ����*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_LIMIT_INFO:
			/*��ѯ������Ϣ����*/
			/*��0��:��ʾԱ�����Ƿ��ֹ*/
			memcpy(&dsp_buffer[0], "Ա��������:", 11);
			if(0==buffer[0])			memcpy(&dsp_buffer[11], "����", 4);
			else	if(1==buffer[0])	memcpy(&dsp_buffer[11], "��ֹ", 4);
			else						memcpy(&dsp_buffer[11], "    ", 4);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ADVANCE_INFO:
			/*��ѯ��ǰ������*/
			/*��3��:��ʾ��ǰ��*/
			memcpy(&dsp_buffer[0], "��ǰ��:", 7);
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;	
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;	
			memcpy(&dsp_buffer[11], "��", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_UNPULSE_TIME:
			/*��ѯ������ͣ����ʱʱ�����*/
			/*��2��:��ʾ"��ʱͣ��ʱ��"�ı�*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "������ͣ��ʱ��", 14);

			/*��4��:��ʾ��ʱͣ��ʱ��*/
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;	
			memcpy(&dsp_buffer[3], "��", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BIND_INFO:
			/*��ѯ����Ϣ����*/
			/*��0��:��ʾ��ʱ��*/
			dsp_buffer[0]='T';	dsp_buffer[1]=':';
			hex2Ascii(&buffer[0], 7, &dsp_buffer[2], 14);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*��2��:��ʾоƬID*/
			hex2Ascii(&buffer[7], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*��4��:��ʾACT����*/
			hex2Ascii(&buffer[15], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*��4��:��ʾIRD����*/
			hex2Ascii(&buffer[23], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VERSION_INFO:
			/*��ѯ�汾��Ϣ����*/
			/*��0��:��ʾ�����汾��*/
			memcpy(&dsp_buffer[0], "JL: ", 4);		memcpy(&dsp_buffer[4], &buffer[0], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*��2��:��ʾIPT�汾��*/
			memcpy(&dsp_buffer[0], "IPT:", 4);		memcpy(&dsp_buffer[4], &buffer[12], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ�����汾��*/
			memcpy(&dsp_buffer[0], "PCD:", 4);	memcpy(&dsp_buffer[4], &buffer[24], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_SELECT:
			/*��ѯ��ѡ����棬��5ҳ*/
			if(0==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.��������", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.ʱ������", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.�״μ춨", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.�����춨", 10);
			}
			else if(1==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "5.��������", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "6.ҹ������", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "7.������������", 14);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "8.����ǹ������", 14);
			}
			else if(2==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "9.˰��ʱ��", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "10.������ѡ��", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "11.��������", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "12.��������", 11);
			}
			else if(3==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "13.��ӡ��ѡ��", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "14.�Զ���ӡ����", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "15.��ӡ������", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "16.�Զ���ӡ����", 15);
			}
			else if(4==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "17.��ǰ������", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "18.��ʱͣ��ʱ��", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "19.Ա��������", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "20.ģʽ����", 11);
			}
			else if(5==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "21.��Ʒ��������", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "22.�ͻ�ǹ������", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "23.����Ʒ������", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "24.��̨������ʽ", 15);
			}
			else if(6==buffer[0]){

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "25.������������", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "26.��̨��������", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "27.ƽ�������Ϣ", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "28.�Աȶ�����", 13);
			}
			
			/*��ҳ��ʾ�Ľ�����Ϊ����ǰһҳ��������������ʾ*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRICE:
			/*���õ��۽���*/
			/*��2��:��ʾ�����뵥��*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����뵥��:", 11);

			/*��4��:��ʾ����*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>12)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[3]='.';
			dsp_buffer[4]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[6], "Ԫ/��", 5);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_TIME:
			/*����ʱ�����*/
			/*��0��:��ʾ������ʱ��*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "������ʱ��", 10);

			/*��2��:��ʾ����*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾʱ��*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];	
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_buffer[5]=':';
			dsp_buffer[6]=tmp_buffer[12];	dsp_buffer[7]=tmp_buffer[13];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_BACKLIT:
			/*���ñ������*/
			/*��2��:��ʾ��������*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "��������:����", 13);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "��������:�ر�", 13);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "��������:ʡ��", 13);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "��������:    ", 13);

			/*��6��:��ʾ���¼���ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_NIGHTLOCK:
			/*����ҹ������*/
			/*��2��:��ʾ�Ƿ�ҹ������*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "ҹ������:��  ", 13);
			else if(1==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "ҹ������:����", 13);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "ҹ������:    ", 13);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_OLD:
			/*����ҹ������*/
			/*��2��:��ʾ�����������*/
			memcpy(dsp_buffer, "�����������:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ���������*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_NEW:
			/*����ҹ������*/
			/*��2��:��ʾ������������*/
			memcpy(dsp_buffer, "������������:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ���������*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_ACK:
			/*����ҹ������*/
			/*��2��:��ʾ��ȷ��������*/
			memcpy(dsp_buffer, "��ȷ��������:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ���������*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PHYSICAL_NOZZLE:
			/*��������ǹ��*/
			/*��3��:��ʾ�����ǹ��*/
			memcpy(&dsp_buffer[0], "����ǹ��:", 9);
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
				else									break;
			}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 2*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_TAX_TIME:
			/*����˰��ʱ��*/
			/*��0��:��ʾ������ʱ��*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "������ʱ��", 10);

			/*��2��:��ʾ����*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾʱ��*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];	
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_SPEAKER:
			/*����������*/
			/*��2��:��ʾ������ѡ��*/
			memcpy(&dsp_buffer[0], "������:", 7);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[7], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[7], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[7], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[7], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[7], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[7], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[7], "      ", 6);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_VOICE_VOLUME:
			/*������������*/
			/*��3��:��ʾ�����ǹ��*/
			memcpy(&dsp_buffer[0], "����������:", 11);
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[11+i])	dsp_buffer[11+i]=' ';
				else									break;
			}
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_VOICE_TYPE:
			/*������������*/
			/*��2��:��ʾ��������ѡ��*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "��������:Ů��", 13);
			else	if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "��������:����", 13);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "��������:    ", 13);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINTER:
			/*���ô�ӡ��*/
			/*��2��:��ʾ��ӡ��ѡ��*/
			memcpy(&dsp_buffer[0], "��ӡ��:", 7);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[7], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[7], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[7], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[7], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[7], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[7], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[7], "      ", 6);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_AUTO:
			/*�����Ƿ��Զ���ӡ*/
			/*��2��:��ʾ�Ƿ��Զ���ӡ*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "�Զ���ӡ:��", 11);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "�Զ���ӡ:��", 11);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "�Զ���ӡ:  ", 11);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_UNION:
			/*�����Զ���ӡ����*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "��ӡ����:1��", 12);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "��ӡ����:2��", 12);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "��ӡ����:   ", 12);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_CARD:
			/*�����Զ���ӡ������ѡ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "��ѡ������", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "1.�û���4.��ÿ�", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "2.����5.ά�޿�", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "3.Ա����", 8);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_BILLTYPE:
			/*�����Զ���ӡ�˵����ͽ���*/
			hex_value=(buffer[1]<<8)|(buffer[2]<<0);
			if(0==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ӿ��˵�:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ӿ��˵�:��", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "���˵�:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "���˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ��", 4);
			}
			else if(1==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�ӿ��˵�:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�ӿ��˵�:��", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "���˵�:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "���˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ��", 4);
			}
			else if(2==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ӿ��˵�:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ӿ��˵�:��", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "���˵�:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "���˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ��", 4);
			}
			else if(3==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ϰ��˵�:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ϰ��˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(4==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ϰ��˵�:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ϰ��˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(5==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����˵�:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����˵�:��", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "�ϰ��˵�:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "�ϰ��˵�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(6==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�°��˵�:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "�°��˵�:��", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ǿ��˵�:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ǿ��˵�:��", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ͼ۽���:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ͼ۽���:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(7==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�°��˵�:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�°��˵�:��", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�ǿ��˵�:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "�ǿ��˵�:��", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ͼ۽���:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "�ͼ۽���:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(8==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�°��˵�:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�°��˵�:��", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ǿ��˵�:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�ǿ��˵�:��", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "�ͼ۽���:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "�ͼ۽���:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else
			{
				if(0==((hex_value>>9)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "����ܾ�:  ", 11);
				else if(1==((hex_value>>9)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "����ܾ�:��", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "           ", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "           ", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ��", 4);
			}

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_ADVANCE:
			/*������ǰ������*/
			/*��2��:��ʾ��������ǰ��*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��������ǰ��:", 13);

			/*��4��:��ʾ��ǰ��*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[3]='.';
			dsp_buffer[4]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[6], "��", 2);
			dsp_len=9;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_UNPULSE_TIME:
			/*���������峬ʱͣ��ʱ�����*/
			/*��2��:��ʾ�����峬ʱʱ��*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����峬ʱʱ��:", 15);

			/*��4��:��ʾ�����峬ʱͣ��ʱ��*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "��", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 11*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_STAFF_LIMIT:
			/*����Ա�����������ƽ���*/
			/*��2��:��ʾԱ������������*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա��������:����", 15);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա��������:��ֹ", 15);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "Ա��������:    ", 15);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_MODE:
			/*����ģʽ����*/
			/*��2��:��ʾԱ������������*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, " ��������ģʽ ", 14);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "�ǿ�������ģʽ", 14);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, " ��������ģʽ ", 14);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "              ", 14);

			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OTHER_OPERATE_PASSSIN:
			/*�������������������*/
			/*��0��:��ʾ��������*/
			dsp_buffer[0]='C';	dsp_buffer[1]=':';
			dsp_buffer[2]=((buffer[0]>>4)&0x0f)+0x30;	dsp_buffer[3]=((buffer[0]>>0)&0x0f)+0x30;
			dsp_buffer[4]=((buffer[1]>>4)&0x0f)+0x30;	dsp_buffer[5]=((buffer[1]>>0)&0x0f)+0x30;
			dsp_buffer[6]=((buffer[2]>>4)&0x0f)+0x30;	dsp_buffer[7]=((buffer[2]>>0)&0x0f)+0x30;
			dsp_len=8;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��2��:��ʾԱ������������*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "����������:", 11);

			/*��4��:��ʾ����������*/
			memset(dsp_buffer, ' ', 6);	dsp_len=6;
			for(i=0; i<6 && i<buffer[3]; i++)	dsp_buffer[5-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 10*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OTHER_OPERATE:
			/*������������*/
			/*��2��:��ʾ�������������*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�������������:", 15);

			/*��4��:��ʾ����Ĵ���ASCII*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)buffer, 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_JLTYPE:
			/*����������ʾ����*/
			/*��3��:��ʾ��������*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����:��ͨ      ", 15);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����:��ǹ������", 15);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����:˫ǹ������", 15);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����:          ", 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_SHILED:
			/*������������������������ʾ����*/
			/*��0��:��ʾ����������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "������:", 7);

			/*��2��:��ʾ������*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "��", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��������������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "����������:", 11);

			/*��6��:��ʾ����������*/
			bcd_value=hex2Bcd((buffer[4]<<24)|(buffer[5]<<16)|(buffer[6]<<8)|(buffer[7]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "��", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 3*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_EQUIVALENT:
			/*��ʾ������������*/
			/*��3��:��ʾ����������*/
			memcpy(&dsp_buffer[0], "����:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>12)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[14]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VALVE_VOLUME:
			/*��ʾ�󷧴�ʱ�������*/
			/*��2��:��ʾ�󷧿�������������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�󷧿���������:", 15);
			
			/*��4��:��ʾ�󷧿���������*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "��", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_STANDBY:
			/*�ǿ���������������*/
			/*��0����ʾ:����״̬+ʱ�䣬�м���벿�����ո�*/
			if(1==buffer[7])	memcpy(&dsp_buffer[0], "��", 2);
			else						memcpy(&dsp_buffer[0], "  ", 2);
			memcpy(&dsp_buffer[2], "      ", 6);
			hex2Ascii(&buffer[0], 7, tmp_buffer, 14);
			dsp_buffer[8]=tmp_buffer[8];		dsp_buffer[9]=tmp_buffer[9];
			dsp_buffer[10]=':';
			dsp_buffer[11]=tmp_buffer[10];		dsp_buffer[12]=tmp_buffer[11];
			dsp_buffer[13]=':';
			dsp_buffer[14]=tmp_buffer[12];	dsp_buffer[15]=tmp_buffer[13];
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*��3����ʾ:"��ӭ����"*/
#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 4*8, "��ӭ����", 8);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "��'��'����������", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��ȷ�ϼ���������", 16);
#else
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 4*8, "��ӭ����", 8);
#endif

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_PRESET:
			/*�ǿ�������Ԥ�ý���*/
			/*��2��:��ʾ"������Ԥ����:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������Ԥ����:", 13);
			/*��4��:��ʾԤ��������λ*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])	memcpy(&dsp_buffer[11], "Ԫ", 2);
			else						memcpy(&dsp_buffer[11], "��", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_OILLING:
			/*�ǿ������������н���*/
			/*��0��:��ʾ"������"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "������", 6);
			/*��3��:��ʾ���*/
			memcpy(&dsp_buffer[0], "���:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[14], "Ԫ", 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��ȷ�ϼ���������", 16);
#endif

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_STATION_INFO1:
			/*��վͨ����Ϣ���棬��һ����*/
			/*��0��:��ʾ"ͨ����Ϣ"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "ͨ����Ϣ", 8);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "�汾��:    ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾʡ����*/
			hex2Ascii(&buffer[1], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "ʡ����:    ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ���д���*/
			hex2Ascii(&buffer[2], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "���д���:  ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_STATION_INFO2:
			/*��վͨ����Ϣ���棬�ڶ�����*/
			/*��0��:��ʾ"ͨ����Ϣ"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "ͨ����Ϣ", 8);

			/*��2��:��ʾ�ϼ���λ����*/
			hex2Ascii(&buffer[0], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "�ϼ�ID: ", 8);	memcpy(&dsp_buffer[8], tmp_buffer, 8);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��վID*/
			hex2Ascii(&buffer[4], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��վID: ", 8);	memcpy(&dsp_buffer[8], tmp_buffer, 8);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILINFO:
			/*��Ʒ�ͼ۱���Ϣ����*/
			/*��0��:��ʾ"��Ʒ��Ϣ"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "��Ʒ��Ϣ", 8);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "�汾��:  ", 9);	memcpy(&dsp_buffer[9], tmp_buffer, 2);	dsp_buffer[11]='H';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��Ʒ����*/
			memcpy(&dsp_buffer[0], "��Ʒ��Ŀ:", 9);
			bcd_value=hex2Bcd(buffer[1]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;
			if('0'==dsp_buffer[9] && '0'==dsp_buffer[10])			{dsp_buffer[9]=' ';	dsp_buffer[10]=' ';}
			else if('0'==dsp_buffer[9] && '0'!=dsp_buffer[10])	{dsp_buffer[9]=' ';}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��Чʱ��*/
			hex2Ascii(&buffer[2], 6, tmp_buffer, 12);
			memcpy(&dsp_buffer[0], "T:", 2);	memcpy(&dsp_buffer[2], tmp_buffer, 12);
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BASELIST:
			/*������������Ϣ����*/
			/*��0��:��ʾ"����������"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "����������", 10);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 2, tmp_buffer, 4);
			memcpy(&dsp_buffer[0], "�汾:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 4);	dsp_buffer[9]='H';
			dsp_buffer[10]=':';
			bcd_value=hex2Bcd((buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[11]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[14]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[15]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+11])	dsp_buffer[i+11]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��Чʱ��*/
			hex2Ascii(&buffer[4], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��Ч:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��ֹʱ��*/
			hex2Ascii(&buffer[8], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��ֹ:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ADDLIST:
			/*������������Ϣ����*/
			/*��0��:��ʾ"����������"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "����������", 10);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "�汾:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��Чʱ��*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��Ч:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��ֹʱ��*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��ֹ:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_DELLIST:
			/*��ɾ��������Ϣ����*/
			/*��0��:��ʾ"��ɾ������"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "��ɾ������", 10);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "�汾:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��Чʱ��*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��Ч:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��ֹʱ��*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��ֹ:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_WHITELIST:
			/*��������Ϣ����*/
			/*��0��:��ʾ"������"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "������", 6);

			/*��2��:��ʾ�汾��*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "�汾:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ��Чʱ��*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��Ч:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ��ֹʱ��*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "��ֹ:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OIL_OVER_INFO:
			/*���������Ϣ����*/
			/*��1��:��ʾ���͹���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 1, 0*8, "���������������", 16);

			/*��3��:��ʾ����������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "������:", 9);

			/*��5��:��ʾ������*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "Ԫ", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OIL_OVER_STAFFIN:
			/*�������Ա�������������*/
			/*��0��:��ʾ����������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "������:", 9);

			/*��2��:��ʾ������*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "Ԫ", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, (char*)dsp_buffer, dsp_len);

			/*��4��:��ʾ������Ա������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "������Ա������:", 15);

			/*��6��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[3]; i++)	dsp_buffer[16-1-i]='*';
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_TEST:
			/*����������,Э��Ȳ��Խ���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1-�絥  2-������", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "3-Э��  4-Э����", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "5-Э��������ʱ��", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "6-�˳�����", 10);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PROTOCOL_TIME:
			/*����Э������������ѯʱ�����ý���*/
			/*��2��:��ʾ"������Э��ʱ��:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������Э��ʱ��:", 15);

			/*��4��:��ʾ�����ʱ��*/
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++){
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "��", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 11*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_SCAN:
			/*��ɨ��������������ʾ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��ɨ�����룬��", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "ȷ�ϼ�����������", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_BARCODE_INPUT:
			/*�����������*/
			/*��2��:��ʾ"����������"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "����������:", 11);

			/*��4��:��ʾ���������*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZING:
			/*������Ȩ�����н���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��Ȩ�����У�", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "���Ժ�...", 9);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZE:
			/*��Ȩ��������ʾ����*/
			/*��2��:��ʾ"��Ȩ���:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��Ȩ���:", 9);

			/*��4��:��ʾ��Ȩ���*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "Ԫ", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZE_CANCEL:
			/*������������������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��Ȩȡ���У�", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "���Ժ�...", 9);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILSTART:
			/*������������������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���������У�", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "���Ժ�...", 9);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILLING:
			/*��������������*/
			/*��0��:��ʾ"������"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "������", 6);

			/*��2��:��ʾ"��Ȩ���:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��Ȩ���:", 9);

			/*��4��:��ʾ��Ȩ���*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "Ԫ", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "��ȷ�ϼ���������", 16);
#endif

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILFINISH:
			/*�����������ͽ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "���ͽ����У�", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "���Ժ�...", 9);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OIL_FINAL:
			/*�����������ͽ����ʾ����*/
			/*��0��:��ʾ"���ͽ���"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "���ͽ���", 8);

			/*��2��:��ʾ"��Ȩ���:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "��Ȩ���:", 9);

			/*��4��:��ʾ��Ȩ���*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "Ԫ", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_UNPULSE_OVERTIME:	/*�����峬ʱ�ش�ʱ�����*/
			/*��0��:��ʾ������ش�ʱ��*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������ش�ʱ��", 16);

			/*��4��:��ʾ������ش�ʱ�䣬��λΪ��*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	if('0'==dsp_buffer[0])	dsp_buffer[0]=' ';
			dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;	if(' '==dsp_buffer[0] && '0'==dsp_buffer[1])	dsp_buffer[1]=' ';
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "��", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ISAUTHEN:	/*�����Ƿ���ҪDES��֤�Ľ���*/
			/*��3��:��ʾ�Ƿ���Ҫ��֤*/
			memcpy(dsp_buffer, "�Ƿ�DES��֤:", 12);
			if(0==buffer[0])	memcpy(&dsp_buffer[12], "��", 2);
			else						memcpy(&dsp_buffer[12], "��", 2);
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILVOICE:		/*��Ʒ������ѯ����*/
			/*��0��:��ʾ��Ʒ��������ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "��Ʒ����", 8);

			/*��2~7��:��ʾ��Ʒ�����ļ���*/
			memset(dsp_buffer,  ' ', 16);
			for(i=0, offset_x=2, offset_y=0; ; )
			{
				/*�ַ�����һ�ֽڣ������򱣴����ֽ�*/
				if(buffer[1+i]<128){
					dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
				}
				else{
					if(offset_y<15){
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
					}
					else{
						offset_y++;
					}
				}

				/*����*/
				if(offset_y>=16){
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					memset(dsp_buffer,  ' ', 16);
					offset_x+=2;	offset_y=0;
				}

				/*���ݲ������*/
				if(!(i<buffer[0] && offset_x<8)){
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					break;
				}
			}

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_OILVOICE_SELECT:		/*��Ʒ��������ѡ�����*/
			/*��0��:��ʾ��Ʒ��������ѡ��*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, "1������", 7);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, "2������", 7);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_OILVOICE:		/*��Ʒ����ѡ�����*/
			/*��0��:��ʾ��Ʒ��������ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "��ѡ����Ʒ����", 14);

			/*��2~7��:��ʾ��Ʒ�����ļ���*/
			memset(dsp_buffer,  ' ', 48);
			for(i=0, offset_x=2, offset_y=0; ; )
			{
				/*�ַ�����һ�ֽڣ������򱣴����ֽ�*/
				if(buffer[1+i]<128)
				{
					dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
				}
				else
				{
					if(offset_y<15)
					{
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
					}
					else
					{
						offset_y++;
					}
				}

				/*����*/
				if(offset_y>=16)
				{
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					memset(dsp_buffer,  ' ', 16);
					offset_x+=2;	offset_y=0;
				}

				/*���ݲ������*/
				if(!(i<buffer[0] && offset_x<8))
				{
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					break;
				}
			}

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILERRLOG:	/*��ѯ������־����*/
			dsp_len=strlen((char*)buffer);
			memset(dsp_line_size, 0, 4);	x=0;	y=0;
			/*����ʾ���ݰ��н��з�װ*/
			for(i=0; i<dsp_len;)
			{
				/*�������ĸ,������ĸ��������y����һλ*/
				if(buffer[i]<128)
				{
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
				}
				/*����Ǻ���,�����λ����β������ʾ��ת����һ����ʾ*/
				else
				{
					if(y>=15){x++;	y=0;}
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
				}
				/*���y���곬��1����ת����һ��*/
				if(y>=16)	{x++;	y=0;}
				/*���x���곬��4��������ʾʣ�ಿ��*/
				if(x>=4)	break;
			}

			/*����ÿ�е���ʾ�����ж��Ƿ���ʾ*/
			for(i=0; i<4; i++)
			{
				if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
			}

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BARGUNNUMBER:	/*����ǹ����ѯ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "���뵥��ǹ��", 12);
			
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "��ǹ", 4);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "��ǹ", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BARBRAND:	/*����Ʒ�Ʋ�ѯ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "����ģ��Ʒ��", 12);
			
			if('1'==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "Զ�����άģ��  ", 16);
			else if('2'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "Զ����һάLV1000", 16);
			else if('3'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����Τ����άģ��", 16);
			else if('4'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����Τ��IS4125  ", 16);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_NOZZLE_NUMBER:	/*����ǹ�����ý���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "�ͻ�ǹ������", strlen("�ͻ�ǹ������"));
			
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "˫ǹ��", 6);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "��ǹ��", 6);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_BARBRAND:	/*����Ʒ�����ý���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "����ģ��Ʒ������", 16);
			
			if('1'==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "Զ�����άģ��  ", 16);
			else if('2'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "Զ����һάLV1000", 16);
			else if('3'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����Τ����άģ��", 16);
			else if('4'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "����Τ��IS4125  ", 16);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_LOCAL_NETINFO:				/*����������Ϣ��ѡ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.IP��ַ", strlen("1.IP��ַ"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.��������", strlen("2.��������"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.Ĭ������", strlen("3.Ĭ������"));

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_IP:						/*����IP��ַ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "����IP��ַ", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_MASK:					/*�����������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "������������", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_GATEWAY:			/*����Ĭ�����ؽ���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "����Ĭ������", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_INFO:			/*ʯ����̨������Ϣ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.��̨������ʽ", strlen("1.��̨������ʽ"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.��̨IP��ַ", strlen("2.��̨IP��ַ"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.��̨�˿ں�", strlen("3.��̨�˿ں�"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.���ض˿ں�", strlen("4.���ض˿ں�"));

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_IP:				/*ʯ����̨IP��ַ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "��̨IP��ַ", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_PORT:			/*ʯ����̨ͨѶ�˿ں���Ϣ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "��̨�˿ں�", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_LOCAL_PORT:			/*ʯ����̨ͨѶ���ط������˿ں���Ϣ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "���ض˿ں�", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_INFO:				/*ƽ�����������Ϣ��ѡ�����*/
			if(0 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.ƽ�����IP��ַ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.ƽ����������  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.ƽ��Ĭ������  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(1 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "4.ƽ����ѡDNS   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "5.ƽ�屸��DNS   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "6.ƽ��FTP��ַ   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(2 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "7.ƽ��FTP�˿ں� ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "8.ƽ���̨��ַ  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "9.ƽ������      ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}
			else if(3 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "10.�Խ���̨IP   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "11.�Ƿ����ô��� ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "12.��Ʒȷ�Ϲ��� ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);
			}

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_IP:					/*ƽ�����IP��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "ƽ��IP��ַ", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_MASK:				/*ƽ�����������Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ����������", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_GATEWAY:		/*ƽ�����������Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ��Ĭ������", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FIRSTDNS:		/*ƽ�������ѡDNS��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ����ѡDNS", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_SECONDDNS:	/*ƽ����Ա���DNS��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ�屸��DNS", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FTP_IP	:			/*ƽ�����FTP��ַ��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ��FTP��ַ", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FTP_PORT:		/*ƽ�����FTP�˿ں���Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "ƽ��FTP�˿ں�", 13);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_SERVERIP:		/*ƽ��������ӵĺ�̨IP��ַ��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "ƽ���̨��ַ", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_VOLUME:			/*ƽ�����������Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "  ƽ���������  ", 16);
			sprintf((char*)dsp_buffer, "%d", buffer[0]);
			if(strlen((char*)dsp_buffer) < 3)	memset(dsp_buffer + strlen((char*)dsp_buffer), ' ', 3 - strlen((char*)dsp_buffer));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 7*8, (char*)dsp_buffer, 3);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TABLETPC_TELE_IP:		/*ƽ����������Խ���̨IP��ַ��Ϣ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "ƽ��Խ���̨IP", 14);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		
		case DSP_CONNECT_TYPE_SET:		/*��̨���ӷ�ʽ���ý���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, " ��ѡ�����ӷ�ʽ ", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "  1.����������  ", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "  2.RJ45����    ", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CONNECT_TYPE_DSP:		/*��̨���ӷ�ʽ��ѯ����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "ʯ����̨���ӷ�ʽ", 16);
			if('0' == buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   ����������   ", 16);
			else if('1' == buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "    RJ45����    ", 16);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CONTRAST:						/*��ʾ�Աȶ�*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "��ʾ�Աȶ�", 10);
			sprintf((char*)dsp_buffer, "%d", buffer[0]);	memset(dsp_buffer+strlen((char*)dsp_buffer), ' ', 3);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 7*8, (char*)dsp_buffer, 3);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_DICOUNT_FAIL_KEEP:		/*�����ۿ�ʧ�ܺ��˹�ȷ�Ͻ���*/
			if(id!=dsp->interfaceID)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�ۿۼ۸�����ʧ��", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������ԭ�۸����", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "������Ա������: ", 16);
			}
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_MODEL:							/*���Ͳ�������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "����", 16);
			
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_AUTH_BALANCE:				/*��Ȩ����������*/
			/*��0��*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "��Ȩ���", strlen("��Ȩ���"));
			/*��3��*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "Ԫ", 2);
			dsp_len = 13-i;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, (16-dsp_len)/2*8, (char*)dsp_buffer+i, dsp_len);

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CARD_DEBIT:				/*�������ѿۿ����*/
			/*��0��*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "֧�����", strlen("֧�����"));
			/*��3��*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "Ԫ", 2);
			dsp_len = 13-i;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, (16-dsp_len)/2*8, (char*)dsp_buffer+i, dsp_len);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "ȷ��--���п�֧��", strlen("ȷ��--���п�֧��"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "����--ȡ����֧��", strlen("����--ȡ����֧��"));

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_PROMOTION:					/*�Ƿ����ô�������*/
			/*��0��:��ʾ��ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "�Ƿ����ô�������", 16);
			/*��2��:��ʾ�Ƿ����ô�������*/
			if(1 == buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, " ���� ", 6);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "������", 6);
			
			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID && 1 == buffer[1])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_YuLe_Grade_Fun:					/*�Ƿ�������Ʒȷ�Ϲ���*/
			/*��0��:��ʾ��ʾ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "��Ʒȷ�Ϲ���", 16);
			/*��2��:��ʾ�Ƿ����ô�������*/
			if(PC_FUN_GRADE_OK == buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, " ���� ", 6);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "������", 6);
			
			/*��6��:��ʾ���¼���ʾ*/
			if(id!=dsp->interfaceID && 1 == buffer[1])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "����", 4);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TM_SCAN_AND_INPUT:	/*����ɨ�輰�������*/
			/*��0,2��:��ʾ"��ɨ�����룬���ڼ�����������֤��"*/
			if(id!=dsp->interfaceID)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "��ɨ�����룬����", strlen("��ɨ�����룬����"));
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������������֤��", strlen("������������֤��"));
			}
			
			/*��4��:��ʾ���������*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			/*��6��:��ʾ�»���*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 3*8, "----------", strlen("----------"));

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_PRESET:				/*���п�Ԥ�ý���*/
			/*��2��:��ʾ"������Ԥ����:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "������Ԥ����:", 13);

			/*��4��:��ʾԤ��������λ*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])	memcpy(&dsp_buffer[11], "Ԫ", 2);
			else						memcpy(&dsp_buffer[11], "��", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_PIN_INPUT:			/*���п������������*/
			/*��2��:��ʾ"�����뿨����:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "�����뿨����:", 13);
			/*��4��:��������λ����ʾ*�ţ�û��*����Ҫ��ʾ�ĵط��Կո�*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_AUTH_REGUEST:	/*���п�Ԥ��Ȩ*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "��Ȩ���������Ժ�", 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_BANK_AUTH_RESULT:	/*���п�Ԥ��Ȩ�ɹ�����*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "��Ȩ�ɹ�,����ǹ.", 16);
			
			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_BANK_OILLING:				/*���п������н���*/
			/*��0��:������*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "������", 6);

			/*��4��:��ǰ���ͽ��*/
			memcpy(&dsp_buffer[0], "���:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[14], "Ԫ", 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*�״���ʾ�Ľ�����Ҫ���������״���ʾ���治����*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);			
			break;

		default:
			break;
		}

		/*�洢��ǰ��ʾ�����*/
		dsp->interfaceID=id;

		return 0;
	}


	/*ƽ�������ʾ����ʾ*/
	if(DSP_DEV_PCMONITOR==dsp->DEV)
	{
		/*�жϳ���*/
		if(8+nbytes > DSP_LEN_MAX)	return ERROR;

		/*�����ʾˢ�¶�ʱ��*/
		*timer=0;
			
		/*�ж�֡�Ų�Ϊ0xfa*/
		dsp->txFrame++;	if(0xfa==dsp->txFrame)	dsp->txFrame++;	

		dsp->txBuffer[0]=0xfa;
		dsp->txBuffer[1]=(char)((3+nbytes)>>8);
		dsp->txBuffer[2]=(char)((3+nbytes)>>0);
		dsp->txBuffer[3]=dsp->txFrame;
		dsp->txBuffer[4]=0x55;
		dsp->txBuffer[5]=id;
		memcpy(&dsp->txBuffer[6], buffer, nbytes);
		crc=crc16Get(&dsp->txBuffer[1], 5+nbytes);
		dsp->txBuffer[6+nbytes]=(char)(crc>>8);
		dsp->txBuffer[7+nbytes]=(char)(crc>>0);
		dsp->txLen=8+nbytes;
		comWrite(dsp->comFd, (char*)dsp->txBuffer, dsp->txLen);
	
		return 0;
	}

	return 0;
}


/********************************************************************
*Name				:dspInit
*Description		:��ʾģ�鹦�ܳ�ʼ��
*Input				:None
*Output				:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-10,modified by syj
*/
int dspInit(void)
{
	DspParamStructType *dsp=NULL;

	/*A����ʾ����*/
	dsp=&DspParamA;

	/*��ʾ�Աȶ�*/
	dsp->Contrast=32;

/*���̵�������ʾ��ز���*/
	/*���̵�����ʾ��*/
	dsp->DEV=DSP_DEV_KEYBOARD;
	/*�������豸��*/
	dsp->DEVKeyx=DEV_DSP_KEYA;

/*andriodƽ����ʾ����ز���*/	
	/*ƽ��ͨѶ����*/
	dsp->comFd=COM7;
	/*�������ͻ��汣���ź���*/
	//del dsp->semIdTx=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==dsp->semIdTx)	printf("Error! Create semaphore 'dspA.semIdTx' failed!\n");
	pthread_mutex_init(&dsp->semIdTx, NULL);

	/*��ʾ���ؽ��������ʼ��*/
	//del dsp->tIdDspRx=taskSpawn("tDspRxA", 156, 0, 0x2000, (FUNCPTR)tdspRecive, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDspRx))	printf("Error!	Creat task 'tDspRxA' failed!\n");
	pthread_t tDspRxA;
	dsp->tIdDspRx=pthread_create(&tDspRxA, NULL, (void*)tdspRecive, NULL);
	if(0!=dsp->tIdDspRx) printf("Error!	Creat task 'tDspRxA' failed!\n");
	pthread_detach(tDspRxA);

	/*��ʾˢ�������ʼ��*/
	//del dsp->tIdDsp=taskSpawn("tDspA", 156, 0, 0x2000, (FUNCPTR)tdsp, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDsp))	printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_t tDspA;
	dsp->tIdDsp=pthread_create(&tDspA, NULL, (void*)tdsp, NULL);
	if(0!=dsp->tIdDsp) printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_detach(tDspA);




	/*B����ʾ����*/
	dsp=&DspParamB;

	/*��ʾ�Աȶ�*/
	dsp->Contrast=32;

/*���̵�������ʾ��ز���*/
	/*���̵�����ʾ��*/
	dsp->DEV=DSP_DEV_KEYBOARD;
	/*�������豸��*/
	dsp->DEVKeyx=DEV_DSP_KEYB;	

	
/*andriodƽ����ʾ����ز���*/
	/*ƽ��ͨѶ����*/
	dsp->comFd=COM8;					
	/*�������ͻ��汣���ź���*/
	//del dsp->semIdTx=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==dsp->semIdTx)	printf("Error! Create semaphore 'dspB.semIdTx' failed!\n");
	pthread_mutex_init(&dsp->semIdTx, NULL);

	/*��ʾ���ؽ��������ʼ��*/
	//del dsp->tIdDspRx=taskSpawn("tDspRxB", 156, 0, 0x2000, (FUNCPTR)tdspRecive, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDspRx))	printf("Error!	Creat task 'tDspRxB' failed!\n");
	pthread_t tDspRxB;
	dsp->tIdDspRx=pthread_create(&tDspRxB, NULL, (void*)tdspRecive, NULL);
	if(0!=dsp->tIdDspRx) printf("Error!	Creat task 'tDspRxB' failed!\n");
	pthread_detach(tDspRxB);

	/*��ʾˢ�������ʼ��*/
	//del dsp->tIdDsp=taskSpawn("tDspB", 156, 0, 0x2000, (FUNCPTR)tdsp, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDsp))	printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_t tDspB;
	dsp->tIdDsp=pthread_create(&tDspB, NULL, (void*)tdsp, NULL);
	if(0!=dsp->tIdDsp) printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_detach(tDspB);

	return 0;
}


