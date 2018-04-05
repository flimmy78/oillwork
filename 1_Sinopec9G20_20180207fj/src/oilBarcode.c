//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilLog.h"
//#include "oilParam.h"
//#include "oilIpt.h"
//#include "oilBarcode.h"

#include "../inc/main.h"

//����ɨ��ģ�����
typedef struct
{
	unsigned char Brand;	//����ģ��Ʒ��YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125

	//����ģ���ʼ��ʱ��������
	unsigned char InitRxLen;					//�������ݳ���	
	unsigned char InitRxBuffer[BAR_RXLEN_MAX];	//���յ�����

	//�������ݽ��ռ�У�����
	unsigned char Data[6];							//��������ASCII����ȫ0ʱ��ʾ������
	unsigned char RxFlag;							//���ձ�ʶ0=���������ݣ�1=����������������
	unsigned char RxLen;							//�������ݳ���
	unsigned char RxBuffer[BAR_RXLEN_MAX];			//���յ�����
	
	int DEV;       //������������豸ID
	int UID;       //��ǰ�û���0=���û�������=�û�ID	
	int tIdReceive;//ɨ�����ݽ�������ID	
	int tIdScan;   //ɨ������ID	
	int Scan;      //ɨ�������0=ֹͣɨ�裬1=ɨ��

	
	sem_t semIdScan;
	pthread_mutex_t semIdData;

	//ɨ����������ź���
	//SEM_ID semIdScan;  //fj:
	//�������ݲ����ź���
	//SEM_ID semIdData; //fj:
}BarcodeStructType;

//����ɨ������
static BarcodeStructType barParamA, barParamB;


//����������Ȩ��̨����
typedef struct
{
	int ComFd;								//����������̨ͨѶ���ں�								
	int tIdRx;								//����������̨���ݽ�������											
	unsigned char RxStep;					//����������̨���ݽ��ղ���				
	unsigned char RxBuffer[CPOS_MAX_LEN];	//����������̨����
	int RxLen;								//����������̨���ݳ���
}CPOSStructType;

static CPOSStructType CPOS;

//�ڲ���������
static unsigned char barEAN8Parity(unsigned char *buffer, unsigned char nbytes);

/*******************************************************************
*Name				:barEAN8Parity
*Description		:EAN8��ʽ����У��
*Input				:buffer	У�����ݣ�ASCII
*						:nbytes	У�����ݳ��ȣ��̶�7�ֽ�
*Output			:None
*Return				:У���룬0xff=��������
*History			:2014-10-21,modified by syj
*/
static unsigned char barEAN8Parity(unsigned char *buffer, unsigned char nbytes)
{
	unsigned char pVal=0, i;
	unsigned char pData[7]={0};
	unsigned char pRet1=0;
	unsigned char pRet2=0;

	if(7!=nbytes)	
	{
		return 0xFF;
	}

	for(i=0; i<7; i++)
	{
		pData[i]=buffer[i]&0x0f;
	}
	
	pRet1 = pData[0]+pData[2]+pData[4]+pData[6];//1����ż��λ֮��
	pRet1 = pRet1*3;		                    //2��1���úͳ���3				
	pRet2 = pData[1]+pData[3]+pData[5];	        //3����ǰ����λǰ3��֮��	
	pRet1 = pRet1+pRet2;	                    //4��2��3�����		
	
	if(pRet1<=10)	
		pVal=10-pRet1;	 	                    //5���ô��ڻ����4�����10����С��������ȥ4���
	else if((pRet1%10)==0)	
		pVal=0;
	else if((pRet1%10)!=0)	
		pVal=(pRet1/10+1)*10-pRet1;

	return (pVal|0x30);
}


/*******************************************************************
*Name				:tCPOSRx
*Description		:���ղ���������������̨������
*Input				:None
*Output			:None
*Return				:None
*History			:2014-11-13,modified by syj
*/
void tCPOSRx(void)
{
    prctl(PR_SET_NAME,(unsigned long)"tCPOSRx");
	unsigned char tx_buffer[CPOS_MAX_LEN]={0};
	int tx_len=0;
	unsigned char rx_buffer[8]={0};
	int rx_len=0, i=0;
	unsigned short crc=0, data_len=0;
	struct msg_struct msg_st;
	msg_st.msgType = 1;

	//��ʼ����������
	CPOS.RxStep=0;	
	CPOS.RxLen=0;

	//��������������̨�����ݣ��㲥���͸�������ǹ
	FOREVER
	{
		rx_len=comRead(CPOS.ComFd, rx_buffer, 1);

		if(rx_len>0)
		{
			CPOS.RxBuffer[CPOS.RxLen]=rx_buffer[0];
			switch(CPOS.RxStep)
			{
			case 0:	//�������ݿ�ʼ�ֽ�
				if(0xfa==rx_buffer[0])	
				{	
					CPOS.RxStep=1;	
					CPOS.RxLen=1;	
				}
				else	
				{	
					CPOS.RxStep=0;	
					CPOS.RxLen=0;	
				}
				break;
			case 1:	//��������Ŀ���ַ��Դ��ַ��֡��/���ơ���Ч���ݳ��ȣ�������0xFA��			
				if(0xfa==rx_buffer[0])
				{	
					CPOS.RxStep=0;
					CPOS.RxLen=0;	
				}
				else
				{	
					CPOS.RxLen++;
				}
				if(CPOS.RxLen>=6)
				{	
					CPOS.RxStep=2;
				}
				break;
			case 2://����λ0xfaʱ������Ϊ���ֽ�Ϊת���ַ�ִ����һ���������򱣴���ֽ�����	
				if(0xfa==rx_buffer[0])	
				{	
					CPOS.RxStep=3;	
				}
				else
				{	
					CPOS.RxLen++;	
				}

				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
				data_len=((((int)CPOS.RxBuffer[4])>>4)&0x0f)*1000+((((int)CPOS.RxBuffer[4])>>0)&0x0f)*100+\
							((((int)CPOS.RxBuffer[5])>>4)&0x0f)*10+((((int)CPOS.RxBuffer[5])>>0)&0x0f)*1;
				if((CPOS.RxLen>=CPOS_MAX_LEN)||((data_len+8)>=CPOS_MAX_LEN))
				{
					CPOS.RxStep=0;	
					CPOS.RxLen=0;
				}
				else if((CPOS.RxLen>=8)&&(CPOS.RxLen>=(data_len+8)))
				{
					crc=crc16Get(&CPOS.RxBuffer[1], 5+data_len);
					if(crc==(CPOS.RxBuffer[6+data_len]<<8)|(CPOS.RxBuffer[7+data_len]<<0))
					{
                        memcpy(msg_st.msgBuffer,CPOS.RxBuffer,CPOS.RxLen);
						msgsnd(iptMsgIdRead(0),&msg_st,CPOS.RxLen,IPC_NOWAIT);
						msgsnd(iptMsgIdRead(1),&msg_st,CPOS.RxLen,IPC_NOWAIT);
						//fj:
						//msgQSend((MSG_Q_ID)iptMsgIdRead(0), CPOS.RxBuffer, CPOS.RxLen, NO_WAIT, MSG_PRI_NORMAL);
						//msgQSend((MSG_Q_ID)iptMsgIdRead(1), CPOS.RxBuffer, CPOS.RxLen, NO_WAIT, MSG_PRI_NORMAL);
					}
					CPOS.RxStep=0;
					CPOS.RxLen=0;
				}
				break;
			case 3:	//�����0xfa����Ϊת�屣�浱ǰ���ݣ��������0xfa����Ϊǰһ��0xfaΪ��ͷ	
				if(0xfa==rx_buffer[0])	
				{	
					CPOS.RxStep=2;	
					CPOS.RxLen++;	
				}
				else								
				{
					CPOS.RxBuffer[0]=0xfa;	CPOS.RxBuffer[1]=rx_buffer[0];
					CPOS.RxStep=1;	CPOS.RxLen=2;
				}

				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
				data_len=((((int)CPOS.RxBuffer[4])>>4)&0x0f)*1000+((((int)CPOS.RxBuffer[4])>>0)&0x0f)*100+\
							((((int)CPOS.RxBuffer[5])>>4)&0x0f)*10+((((int)CPOS.RxBuffer[5])>>0)&0x0f)*1;
				if((CPOS.RxLen>=CPOS_MAX_LEN)||((data_len+8)>=CPOS_MAX_LEN))
				{
					CPOS.RxStep=0;	CPOS.RxLen=0;
				}
				else if((CPOS.RxLen>=8)&&(CPOS.RxLen>=(data_len+8)))
				{
					crc=(CPOS.RxBuffer[6+data_len]<<8)|(CPOS.RxBuffer[7+data_len]<<0);
					if(crc==crc16Get(&CPOS.RxBuffer[1], 5+data_len))
					{
                        memcpy(msg_st.msgBuffer,CPOS.RxBuffer,CPOS.RxLen);
						msgsnd(iptMsgIdRead(0),&msg_st,CPOS.RxLen,IPC_NOWAIT);
						msgsnd(iptMsgIdRead(1),&msg_st,CPOS.RxLen,IPC_NOWAIT);
						//msgQSend((MSG_Q_ID)iptMsgIdRead(0), CPOS.RxBuffer, CPOS.RxLen, NO_WAIT, MSG_PRI_NORMAL);
						//msgQSend((MSG_Q_ID)iptMsgIdRead(1), CPOS.RxBuffer, CPOS.RxLen, NO_WAIT, MSG_PRI_NORMAL);
					}
					CPOS.RxStep=0;	
					CPOS.RxLen=0;
				}
				break;
			default:
				CPOS.RxStep=0;
				CPOS.RxLen=0;
				break;
			}
		}

		//taskDelay(1);
		usleep(1000);
	}

	return;
}


/*******************************************************************
*Name				:CPOSWrite
*Description		:����������Ȩ��̨���ݷ���
*Input				:nozzle		0=1�Ŷ˿ڣ�1=2�Ŷ˿�
*						:pos_p		�߼����POS_P
*						:buffer		��Ч����
*						:nbytes		��Ч���ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/
int CPOSWrite(int nozzle, unsigned char pos_p, unsigned char *buffer, int nbytes)
{
	int istate = 0;
	static unsigned char frame=0;
	unsigned int len=0, crc_data=0, i=0;
	unsigned long long data=0;
	unsigned char data_buffer[CPOS_MAX_LEN]={0}, tx_buffer[2*CPOS_MAX_LEN]={0}, tx_len=0;
	
	if(8+nbytes > CPOS_MAX_LEN)//�ж���Ч���ݳ���
		return ERROR;

	if(frame>=0x3f)	//֡�����Ϊ0x3f
		frame=0;

	//��֯����
	data_buffer[0]=0xfa;					//���ݰ�ͷ
	data_buffer[1]=0;						//Ŀ���ַ
	data_buffer[2]=pos_p;					//Դ��ַ
	data_buffer[3]=(0<<7)|(0<<6)|(frame++);	//֡��			
	data=hex2Bcd(nbytes);					//��Ч���ݳ���
	data_buffer[4]=(char)(data>>8);	
	data_buffer[5]=(char)(data>>0);
	memcpy(&data_buffer[6], buffer, nbytes);//��Ч����	

	//CRCУ��
	crc_data=crc16Get(&data_buffer[1], 5+nbytes);							
	data_buffer[6+nbytes]=(char)(crc_data>>8);
	data_buffer[7+nbytes]=(char)(crc_data>>0);

	//���0xfa
	memcpy(&tx_buffer[0], &data_buffer[0], 6);
	tx_len=6;
	for(i=6; i<8+nbytes; i++)
	{
		//�������ݸ�ֵ
		tx_buffer[tx_len]=data_buffer[i];	
		tx_len++;	
		if(tx_len>=2*CPOS_MAX_LEN)	//��ֹ���ͳ������
			break;
	
		if(0xfa==data_buffer[i])//���͵�����Ϊ0xfaʱ���һλ0xfa
		{
			tx_buffer[tx_len]=data_buffer[i];
			tx_len++;
		}
	
		if(tx_len>=2*CPOS_MAX_LEN)//��ֹ���ͳ������
			break;
	}
	
	if(MODEL_LIANDA == paramModelGet())//���ڷ��Ͳ��жϽ��
	{
		istate = kjldWrite(tx_buffer, tx_len);
		printf("MODEL_LIANDA\n");
	}
	else
	{
        PrintH(tx_len,tx_buffer);	
		istate = comWriteInTime(CPOS.ComFd, tx_buffer, tx_len, 5);
	    printf("comWrite,CPOS.ComFd = %d,istate = %d\n",CPOS.ComFd,istate);
	}

	return istate;
}


/*******************************************************************
*Name				:tBarcodeScan
*Description		:����ɨ��
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:None
*Return				:None
*History			:2014-10-21,modified by syj
*/
void tBarcodeScan(int nozzle)
{
	unsigned int i=0;
	BarcodeStructType *param=NULL;

	//�ж�ģ��ѡ��
	if(BARCODE_NOZZLE_1==nozzle)
	{
		param=&barParamA;
		prctl(PR_SET_NAME,(unsigned long)"tBarcodeScanA");
	}
	else if(BARCODE_NOZZLE_2==nozzle)	
	{
		param=&barParamB;
	    prctl(PR_SET_NAME,(unsigned long)"tBarcodeScanB");
	}
	else				
		return;

	//��ʼ��ɨ��ģ��
	barScanModuleInit(nozzle, param->Brand);

	FOREVER
	{
		//�ȴ�ɨ����������
		//semTake(param->semIdScan, WAIT_FOREVER);  //fj:
		sem_wait(&param->semIdScan);

	
		//����ɨ��:
		//����ɨ��ʱ����ȡ������ɨ���ź���������ִ��
		//����ɨ�������ȴ�����ɨ�裬�ﵽ����ɨ��ʱ��������´�ɨ�裬
		//�ȴ��������ж����ɨ�赽�������ɨ�����˳��ȴ����̣�
		//�ж��Ƿ����������ݣ����򱣴沢�˳�ɨ�裻
		//�ж��Ƿ����ɨ�裬������ɨ�����˳�ɨ��
		//�����жϾ����������ٴη���ɨ�����
		
		while(BARCODE_SCAN==param->Scan)
		{
			//ɨ������
			if(YUANJD_BRAND==param->Brand)
			{
				comWrite(param->DEV, "\x1B\x31", 2);			param->RxFlag=0;	param->RxLen=0;
			}
			else if(YUANJD_LV1000==param->Brand)
			{
				comWrite(param->DEV, "$$$$#99900035;%%%%", 18);			param->RxFlag=0;	param->RxLen=0;
			}
			else if(HONEYWELL_BRAND==param->Brand)
			{
				comWrite(param->DEV, "\x16\x54\x0D", 3);	param->RxFlag=0;	param->RxLen=0;
			}
			else if(HONEYWELL_IS4125==param->Brand)
			{
				comWrite(param->DEV, "\x12", 1);	param->RxFlag=0;	param->RxLen=0;
			}
	
			for(i=0; i<HONEYWELL_TIME; i++)//�ȴ�ɨ������
			{
				if(1==param->RxFlag || BARCODE_IDLE==param->Scan)
					break;
				//taskDelay(ONE_MILLI_SECOND);
				usleep(1000);
			}
			if(1==param->RxFlag)//�ж��Ƿ�����������
			{
				//semTake(param->semIdData, WAIT_FOREVER); //fj:
				pthread_mutex_lock(&param->semIdData);
				memcpy(param->Data, &param->RxBuffer[1], 6);
				pthread_mutex_unlock(&param->semIdData);
				//semGive(param->semIdData); //fj:
				break;
			}	
			if(BARCODE_IDLE==param->Scan)//�ж��Ƿ����ɨ��
			{
				break;
			}
		}

		//ֹͣɨ��
		if(YUANJD_BRAND==param->Brand)	
			comWrite(param->DEV, "\x1B\x30", 2);
		else if(YUANJD_LV1000==param->Brand)		
			comWrite(param->DEV, "$$$$#99900036;%%%%", 18);
		else if(HONEYWELL_BRAND==param->Brand)	    
			comWrite(param->DEV, "\x16\x55\x0D", 3);
		else if(HONEYWELL_IS4125==param->Brand)	;

		//taskDelay(1);
		usleep(1000);
	}

	return;
}


/*******************************************************************
*Name				:tBarcodeReceive
*Description		:�������ݶ�ȡ
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:None
*Return				:None
*History			:2014-10-21,modified by syj
*/
void tBarcodeReceive(int nozzle)
{
	unsigned char buffer[16]={0};
	int len=0;
	BarcodeStructType *param=NULL;

	//�ж�ģ��ѡ��
	if(BARCODE_NOZZLE_1==nozzle)
	{
		param=&barParamA;
	    prctl(PR_SET_NAME,(unsigned long)"tBarcodeRecvA");
	}
	else if(BARCODE_NOZZLE_2==nozzle)	
	{
		param=&barParamB;
        prctl(PR_SET_NAME,(unsigned long)"tBarcodeRecvB");
	}
	else	
		return;

	FOREVER
	{
		
		len=comRead(param->DEV, buffer, 1);//���յ����ݽ���	
		if(!(len>0))	                   //�ж��Ƿ���յ�����
		{
			//taskDelay(1);
			//usleep(1000);
			continue;
		}
		
		if(param->InitRxLen>=BAR_RXLEN_MAX)	//����Ϊ��ʼ������ʹ�õ�����
		{
			param->InitRxLen=0;
		}
		param->InitRxBuffer[param->InitRxLen++]=buffer[0];
	
		if(0!=param->RxFlag)//�ж��Ƿ��������
		{
			//taskDelay(1);
			//usleep(1000);
			continue;
		}
	
		if(param->RxLen>=BAR_RXLEN_MAX)	//��ֹ���ճ������
		{
			param->RxLen=0;
		}
	
		param->RxBuffer[param->RxLen]=buffer[0];	//�������ݣ���λ����ӦΪ0x36
		if(0==param->RxLen && 0x36!=buffer[0])
		{
			param->RxLen=0;
		}
		else
		{
			param->RxLen++;
		}
	
		if(param->RxLen>=8)//У������
		{
			printf("param->RxLen = %d\n",param->RxLen);
			PrintH(param->RxLen,param->RxBuffer);
			if(param->RxBuffer[7]==barEAN8Parity(param->RxBuffer, 7))
			{
				param->RxFlag=1;
				printf("param->RxBuffer[7] = %d\n",param->RxBuffer[7]);
			}
			else
			{
				param->RxFlag=0;	
				param->RxLen=0;
			}
		}

		//taskDelay(1);
		//usleep(1000);
	}

	return;
}


/*******************************************************************
*Name				:barScanModuleInit
*Description		:����ɨ��ģ���ʼ������Ҫ�Ǹ��ݾ���Ʒ�ƶ�ɨ��ģ���ϵ���һЩ��������
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*						:brand		ģ��Ʒ��YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/
int barScanModuleInit(int nozzle, unsigned char brand)
{
	int i=0, itimes=0, istate=0;
	BarcodeStructType *param=NULL;

	//�ж�ģ��ѡ��
	if(BARCODE_NOZZLE_1==nozzle)		
		param=&barParamA;
	else if(BARCODE_NOZZLE_2==nozzle)	
		param=&barParamB;
	else	
		return ERROR;

	//�ж�Ʒ��
	if(!(YUANJD_BRAND==brand||YUANJD_LV1000==brand||HONEYWELL_BRAND==brand||HONEYWELL_IS4125==brand))
		return ERROR;

	//����Ʒ��
	param->Brand=brand;


	//Զ����һάģ��LV1000��Ҫ������������
	if(YUANJD_LV1000==param->Brand)
	{
    itimes=0;
LV1000_WRITE_SEND0:

		//���ͽ�������ģʽ������
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "$$$$#99900116;%%%%", 18);

		//�ȴ���ʱ�򷵻�
		for(i=0;;)
		{	
			if(0==memcmp(param->InitRxBuffer, "@@@@!99900116;^^^^", 18))//��ȷ�������˳�
			{
				break;
			}

			//��ʱ10����
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);  //fj:20170930	
			i+=(10*ONE_MILLI_SECOND);//��ʱ�ۼ�

			//�жϳ�ʱ����ʱС�����μ������ͣ���ʱ���������˳�
			if(i>=ONE_SECOND && itimes<3)
			{
				itimes++;
				goto LV1000_WRITE_SEND0;	
			}
			if(i>=ONE_SECOND && itimes>=3)
			{
				istate=ERROR;
				break;
			}
		}
	}

	//����Ϊ����Τ��һάģ��IS4125
	if(HONEYWELL_IS4125==param->Brand)
	{
itimes=0;
IS4125_WRITE_SEND0:
		//���ͽ����̵�����
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);	
		for(i=0;;)//�ȴ���ʱ�򷵻�
		{
			//��ȷ�������˳�
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//��ʱ10����
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000); //fj:	
			i+=(10*ONE_MILLI_SECOND);//��ʱ�ۼ�

			//�жϳ�ʱ����ʱС�����μ������ͣ���ʱ���������˳�
			if(i>=ONE_SECOND && itimes<3)
			{
				itimes++;
				goto IS4125_WRITE_SEND0;
			}
			if(i>=ONE_SECOND && itimes>=3){
				istate=ERROR;
				goto IS4125_WRITE_END;
			}
		}

itimes=0;
IS4125_WRITE_SEND1:
		//������������ģʽ���������һ���������200����
		//taskDelay(200*ONE_MILLI_SECOND);
	    usleep(200000);	
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);
	
		for(i=0;;)//�ȴ���ʱ�򷵻�
		{
			//��ȷ�������˳�
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//��ʱ10����
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000); //fj:	
			i+=(10*ONE_MILLI_SECOND);//��ʱ�ۼ�

			//�жϳ�ʱ����ʱС�����μ������ͣ���ʱ���������˳�
			if(i>=ONE_SECOND && itimes<3)
			{
				itimes++;
				goto IS4125_WRITE_SEND1;	
			}
			if(i>=ONE_SECOND && itimes>=3)
			{
				istate=ERROR;
				goto IS4125_WRITE_END;
			}
		}

itimes=0;
IS4125_WRITE_SEND2:
		//�����˳���̵��������һ���������200����
		//taskDelay(200*ONE_MILLI_SECOND);
		usleep(200);
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);

		for(i=0;;)//�ȴ���ʱ�򷵻�
		{
			//��ȷ�������˳�
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//��ʱ10����
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);	
			i+=(10*ONE_MILLI_SECOND);	//��ʱ�ۼ�

			//�жϳ�ʱ����ʱС�����μ������ͣ���ʱ���������˳�
			if(i>=ONE_SECOND && itimes<3)
			{
				itimes++;
				goto IS4125_WRITE_SEND2;	
			}
			if(i>=ONE_SECOND && itimes>=3)
			{
				istate=ERROR;
				goto IS4125_WRITE_END;
			}
		}

IS4125_WRITE_END:
		return istate;
	}


	return istate;
}


/*******************************************************************
*Name				:barBrandWrite
*Description		:����ɨ��ģ��Ʒ������
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*						:brand		ģ��Ʒ��YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barBrandWrite(int nozzle, unsigned char brand)
{
	int i=0, itimes=0, istate=0;
	unsigned int ioffset = 0;
	BarcodeStructType *param=NULL;
	char ibuffer[6] = {0};

	//�ж�ģ��ѡ��
	if(BARCODE_NOZZLE_1==nozzle)		
	{
		param=&barParamA;	
		ioffset=PRM_BARCODE_BRAND_A;
	}
	else if(BARCODE_NOZZLE_2==nozzle)
	{
		param=&barParamB;		
		ioffset=PRM_BARCODE_BRAND_B;
	}
	else
		return ERROR;

	//�ж�Ʒ��
	if(!(YUANJD_BRAND==brand||YUANJD_LV1000==brand||HONEYWELL_BRAND==brand||HONEYWELL_IS4125==brand))
		return ERROR;

	//д�������ļ�
	*ibuffer = brand;
	if(0!=paramSetupWrite(ioffset, ibuffer, 1))
	{
		return ERROR;
	}

	param->Brand=brand;	                //����Ʒ��
	barScanModuleInit(nozzle, brand);	//����ɨ��ģ������

	return 0;
}


/*******************************************************************
*Name				:barBrandRead
*Description		:����ɨ��ģ��Ʒ�ƻ�ȡ
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:brand		ģ��Ʒ��YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barBrandRead(int nozzle, unsigned char *brand)
{
	BarcodeStructType *param=NULL;
	
	if(BARCODE_NOZZLE_1==nozzle)//�ж�ģ��ѡ��		
		param=&barParamA;
	else if(BARCODE_NOZZLE_2==nozzle)	
		param=&barParamB;
	else
		return ERROR;
	
	*brand=param->Brand;//����ɨ��ģ��Ʒ��

	return 0;
}


/*******************************************************************
*Name				:barScan
*Description		:����ɨ�裬ֱ��ֹͣ��ɨ�����룬����ϴε�����
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barScan(int nozzle, int UID)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//�ж�ģ��ѡ��
		param=&barParamA;
	else if(1==nozzle)	
		param=&barParamB;
	else	
		return ERROR;

	//����ϴε�����
	//semTake(param->semIdData, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&param->semIdData);
	memset(param->Data, 0, 6);
	pthread_mutex_unlock(&param->semIdData);
	//semGive(param->semIdData);  //fj:

	//��¼�û�ID
	param->UID = UID;

	//����Ѿ�����ɨ��״̬��ֱ�ӷ��سɹ�
	if(BARCODE_SCAN == param->Scan)
	{
		return 0;
	}

	//����ɨ��
	param->Scan=BARCODE_SCAN;
	sem_post(&param->semIdScan);
	//semGive(param->semIdScan); //fj:

	return 0;
}


/*******************************************************************
*Name				:barStop
*Description		:����ֹͣɨ��
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*						:UID			�û�ID
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barStop(int nozzle, int UID)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//�ж�ģ��ѡ��		
		param=&barParamA;
	else if(1==nozzle)	
		param=&barParamB;
	else
		return ERROR;

	param->UID = UID;	//�û�ID

	//����ֹͣ
	param->Scan=BARCODE_IDLE;
	//semGive(param->semIdScan);  //fj:
	sem_post(&param->semIdScan);
	return 0;
}


/*******************************************************************
*Name				:barUserIDSet
*Description		:���õ�ǰ�û�ID
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*						:id				�û�ID��BARCODE_USER_NO��ʾ���û�������ǰ�û���������ʹ��
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barUserIDSet(int nozzle, int id)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//�ж�ģ��ѡ��	
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else	
		return ERROR;

	param->UID = id;

	return 0;
}

/*******************************************************************
*Name				:barUserIDGet
*Description		:��ȡ��ǰ�û�ID
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:None
*Return				:�û�ID��BARCODE_USER_NO��ʾ���û�������ǰ�û���������ʹ��
*History			:2014-10-21,modified by syj
*/

int barUserIDGet(int nozzle)
{
	BarcodeStructType *param=NULL;


	if(0==nozzle)	//�ж�ģ��ѡ��
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else
		return ERROR;

	return 	(param->UID);
}


/*******************************************************************
*Name				:barStateRead
*Description		:����ɨ��״̬��ȡ
*Input				:nozzle		ģ��ѡ��0=1��ģ�飻1=2��ģ��
*Output			:None
*Return				:0=���У�1=����ɨ�裻����=����
*History			:2014-10-21,modified by syj
*/

int barStateRead(int nozzle)
{
	BarcodeStructType *param=NULL;

	if(0==nozzle)	//�ж�ģ��ѡ��
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else	
		return ERROR;

	return param->Scan;
}


/*******************************************************************
*Name				:barRead
*Description		:�����ȡ����ȡ֮���Զ����
*Input				:nozzle			ģ��ѡ��0=1��ģ�飻1=2��ģ��
*						:maxbytes	�������������󳤶ȣ���С��6bytes
*Output			:buffer			����ASCII��ȫ0��ʾ�����룬6bytes
*Return				:0=�ɹ�������=ʧ��
*History			:2014-10-21,modified by syj
*/

int barRead(int nozzle, unsigned char *buffer, int maxbytes)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)		//�ж�ģ��ѡ��
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else	
		return ERROR;
	
	if(maxbytes<6)//�жϻ��泤��
	{
		return ERROR;
	}

	//semTake(param->semIdData, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&param->semIdData);
	memcpy(buffer, param->Data, 6);
	memset(param->Data, 0, 6);
	pthread_mutex_unlock(&param->semIdData);
	//semGive(param->semIdData); //fj:

	return 0;
}


/*******************************************************************
*Name				:barcodeInit
*Description		:���빦��ģ���ʼ��
*Input				:None
*Output			:None
*Return				:None
*History			:2014-10-21,modified by syj
*/

bool barcodeInit(void)
{
	BarcodeStructType *param=NULL;
	char rdbuffer[64] = {0};
	int istate = 0;
	int nMutexInit = -1;
	int nSemInit = -1;
	
	param=&barParamA;//1������ģ��״̬��ʼ��	
	param->DEV=COM11;//����ģ�����Ӵ����豸ID

	//�����������ݲ����ź���
	//param->semIdData=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==param->semIdData)	printf("Error! Create List 'barParamA.semIdData' failed!\n");
	nMutexInit = pthread_mutex_init(&param->semIdData,NULL);
	if(nMutexInit != 0)
	{
		printf("Error! Create List 'barParamA.semIdData' failed!\n");
		return false;
	}

	//����ɨ�����������ź���
	//param->semIdScan=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdScan)	printf("Error! Create List 'barParamA.semIdScan' failed!\n");
	nSemInit = sem_init(&param->semIdScan,0,0);
	if(nSemInit != 0)
	{
		printf("Error! Create List 'barParamA.semIdScan' failed!\n");
		return false;
	}
	else
	{
		printf("create semIdScan success!\n");
	}


	//����ɨ�����ݽ�������
	//param->tIdReceive=taskSpawn("tBarRxA", 155, 0, 0x2000, (FUNCPTR)tBarcodeReceive, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tBarRxA' failed!\n");

	//��ȡ��ǰ���õ�Ʒ�ƣ�����Ч������Ĭ��ΪԶ����LV1000
	//	ע:��������ȡ����ģ�鷵�����ݣ��ʴ˴�Ӧ���������ģ�����ݽ�����������֮��
	
	istate = paramSetupRead(PRM_BARCODE_BRAND_A, rdbuffer, 1);
	if(0==istate && (YUANJD_BRAND==*rdbuffer || YUANJD_LV1000==*rdbuffer || HONEYWELL_BRAND==*rdbuffer || HONEYWELL_IS4125==*rdbuffer))
	{	
		param->Brand=*rdbuffer;
	}
	else
	{
		param->Brand = YUANJD_LV1000;
	}
	

	//��������ɨ������
	//param->tIdScan=taskSpawn("tBarSacnA", 155, 0, 0x2000, (FUNCPTR)tBarcodeScan, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdScan))	printf("Error!	Creat task 'tBarSacnA' failed!\n");


	
	param=&barParamB; //2������ģ��״̬��ʼ��
	param->DEV=COM12; //����ģ�����Ӵ����豸ID

	//�����������ݲ����ź���
	//param->semIdData=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==param->semIdData)	printf("Error! Create List 'barParamB.semIdData' failed!\n");
	nMutexInit = pthread_mutex_init(&param->semIdData,NULL);
	if(nMutexInit != 0)
	{
		printf("Error! Create List 'barParamB.semIdData' failed!\n");
		return false;
	}

	//����ɨ�����������ź���
	//param->semIdScan=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdScan)	printf("Error! Create List 'barParamB.semIdScan' failed!\n");
	nSemInit = sem_init(&param->semIdScan,0,0);
	if(nSemInit != 0)
	{
		printf("Error! Create List 'barParamB.semIdScan' failed!\n");
		return false;
	}

	//����ɨ�����ݽ�������
	//param->tIdReceive=taskSpawn("tBarRxB", 155, 0, 0x2000, (FUNCPTR)tBarcodeReceive, 1,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tBarRxB' failed!\n");

	//��ȡ��ǰ���õ�Ʒ�ƣ�����Ч������Ĭ��ΪԶ����LV1000
	//ע:��������ȡ����ģ�鷵�����ݣ��ʴ˴�Ӧ���������ģ�����ݽ�����������֮��
	
	istate = paramSetupRead(PRM_BARCODE_BRAND_B, rdbuffer, 1);
	if(0==istate && (YUANJD_BRAND==*rdbuffer || YUANJD_LV1000==*rdbuffer || HONEYWELL_BRAND==*rdbuffer || HONEYWELL_IS4125==*rdbuffer))
	{	
		param->Brand=*rdbuffer;
	}
	else
	{
		param->Brand = YUANJD_LV1000;
	}

	//��������ɨ������
	//param->tIdScan=taskSpawn("tBarSacnB", 155, 0, 0x2000, (FUNCPTR)tBarcodeScan, 1,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdScan))	printf("Error!	Creat task 'tBarSacnB' failed!\n");


	//������Ȩ��̨���ݳ�ʼ��
	CPOS.ComFd=COM15;	
	CPOS.RxStep=0;
	CPOS.RxLen=0;

	//��������������̨���������ʼ��
	//CPOS.tIdRx=taskSpawn("tCPOSRx", 153, 0, 0x4000, (FUNCPTR)tCPOSRx, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(CPOS.tIdRx))		printf("Error!	Creat task 'tCPOSRx' failed!\n");

	return true;
}


