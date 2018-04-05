#include "../inc/main.h"

//�������ź���
static pthread_mutex_t semIdA1;
static pthread_mutex_t semIdB1;

//����������
static IcStructType IcStructA1;
static IcStructType IcStructB1;

/*******************************************************************
*Name				:IcPackSend
*Description		:��װ����IC��������
*Input				:nozzle				ǹѡ0=A1ǹ��1=B1ǹ
*					:cmd					������
*					:cmd_param		�������
*					:data				���ݰ�
*					:data_len			���ݰ�����
*Output				:None
*Return				:ʵ�ʷ��ͳ���
*History			:2013-07-01,modified by syj
*/

static int IcPackSend(unsigned char nozzle, unsigned char cmd, unsigned char cmd_param, const unsigned char *data, int data_len)
{
	unsigned char tx_buffer[256]={0};
	int tx_len=0, i=0;

	//del unsigned char dspbuffer[512]={0}; //fj:���ڴ�ӡ��

	tx_buffer[0]=0x02;
	tx_buffer[1]=(unsigned char)((data_len+2)>>8);
	tx_buffer[2]=(unsigned char)((data_len+2)>>0);
	tx_buffer[3]=cmd;
	tx_buffer[4]=cmd_param;
	memcpy(&tx_buffer[5], data, data_len);
	tx_buffer[5+data_len]=0x03;
	tx_buffer[6+data_len]=xorGet(tx_buffer, 6+data_len);
	tx_len=7+data_len;

	//szb_fj_20171120,update
	if(IC_NOZZLE_1==nozzle &&IptParamA.EtcOilFlg==0)
	{
		IcStructA1.RxLen=0;	IcStructA1.RxValid=0;
		i=comWrite(COM14, tx_buffer, tx_len);
	}
	else if(IC_NOZZLE_1==nozzle &&IptParamA.EtcOilFlg==1)
	{
		tx_len=0;
		tx_buffer[tx_len++]=ETC_CMD;
		tx_buffer[tx_len++]=ETC_05;
		tx_buffer[tx_len++]=IptParamA.LogicNozzle;
		memcpy(tx_buffer+tx_len,IptParamA.EtcSelCardInf+16,4);//2017.07.25����4�ֽڵ�MAC��
		tx_len+=4;
		tx_buffer[tx_len++]=1;/*1�ֽڵ�COS��������*/
		tx_buffer[tx_len++]=data_len;/*1�ֽڵ�APDU�����*/
		memcpy(tx_buffer+tx_len,data,data_len);
		tx_len=tx_len+data_len;
		pcd2PcSend(tx_buffer, tx_len);
		IptParamA.etc_rec_len=0;
		memset(IptParamA.etc_rec_buff,0,sizeof(IptParamA.etc_rec_buff));
	}
	else if(IC_NOZZLE_2==nozzle&&IptParamB.EtcOilFlg==0)
	{
		IcStructB1.RxLen=0;	IcStructB1.RxValid=0;
		i=comWrite(COM18, tx_buffer, tx_len);
	}
	else if(IC_NOZZLE_2==nozzle&&IptParamB.EtcOilFlg==1)
	{
		tx_len=0;
		tx_buffer[tx_len++]=ETC_CMD;
		tx_buffer[tx_len++]=ETC_05;
		tx_buffer[tx_len++]=IptParamB.LogicNozzle;
		memcpy(tx_buffer+tx_len,IptParamB.EtcSelCardInf+16,4);//2017.07.25����4�ֽڵ�MAC��
		tx_len+=4;
		tx_buffer[tx_len++]=1;/*1�ֽڵ�COS��������*/
		tx_buffer[tx_len++]=data_len;/*1�ֽڵ�APDU�����*/
		memcpy(tx_buffer+tx_len,data,data_len);
		tx_len=tx_len+data_len;
		pcd2PcSend(tx_buffer, tx_len);
		IptParamB.etc_rec_len=0;
		memset(IptParamB.etc_rec_buff,0,sizeof(IptParamB.etc_rec_buff));
	}

	return i;
}


/*******************************************************************
*Name			:ICLock
*Description	:��������
*Input			:nozzle	ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�������=ʧ��
*History		:2014-10-17,modified by syj
*/

static int ICLock(int nozzle)
{
	//del if(0==nozzle)	semTake(semIdA1, WAIT_FOREVER);
	//del else			semTake(semIdB1, WAIT_FOREVER);
	if(0==nozzle)	
		pthread_mutex_lock(&semIdA1);
	else			
		pthread_mutex_lock(&semIdB1);
	return 0;
}



/*******************************************************************
*Name			:ICUnlock
*Description	:�����������
*Input			:nozzle	ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�������=ʧ��
*History		:2014-10-17,modified by syj
*/

static int ICUnlock(int nozzle)
{
	//del if(0==nozzle)	semGive(semIdA1);
	//del else			semGive(semIdB1);
	if(0==nozzle)	
		pthread_mutex_unlock(&semIdA1);
	else			
		pthread_mutex_unlock(&semIdB1);
	return 0;
}


/*******************************************************************
*Name			:tICReceive
*Description	:IC�������ݶ�ȡ����������
*Input			:nozzle	����0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:None
*History		:2014-10-17,modified by syj
*/

//static void tICReceive(int nozzle)
void tICReceive(void* pNozzleType)
{
	int nozzle = (int*)pNozzleType;
	unsigned char data=0;
	int read_len=0, len=0;
	IcStructType *param=NULL;
	int com_fd=0;
	printf("tICReceive : nozzle = %d,tICReceive thread is success\n",nozzle);	
	if(IC_NOZZLE_1==nozzle)//�ж����
	{
		param=&IcStructA1;	
		com_fd=COM14;
		prctl(PR_SET_NAME,(unsigned long)"tICReceiveA");
	}
	else if(IC_NOZZLE_2==nozzle)
	{
		param=&IcStructB1;	
		com_fd=COM18;	
		prctl(PR_SET_NAME,(unsigned long)"tICReceiveB");
	}
	else 
		return;

	while ( 1 )
	{	
		if(0!=param->RxValid)//��ǰ״̬���������
		{
			//printf("RxValid = %d\n",param->RxValid);
			//usleep(1000);
			continue;
		}	
		read_len=comRead(com_fd, (char*)&data, 1);//��ȡ����
		if(1!=read_len)
		{
			//usleep(1000);
			continue;
		}

		//if(read_len > 0)  //fj:20170926
		//{
		//	printf("--------tICReceive: \n");
		//    PrintH(read_len,&data);
		//}
        //printf("tICReceive: read_len = %d\n",read_len);
				
		if(param->RxLen>=ICDATA_LEN)//��ֹ�������
		{
			param->RxLen=0;	
			param->RxValid=0;
		}
	
		param->RxBuffer[param->RxLen]=data;	//���沢��������
		if(0==param->RxLen)
		{
			if(0x06==param->RxBuffer[param->RxLen])	
			{
				comWrite(com_fd, "\x05", 1);
				//printf("IC WRITE :05\n");
			}
			else if(0x02==param->RxBuffer[param->RxLen])	
				param->RxLen++;
		}
		else
		{
			param->RxLen++;
		}

		//�ݴﵽָ������ʱ����У��
		len=((param->RxBuffer[1]<<8)|(param->RxBuffer[2]<<0))+5;
		if((param->RxLen>=3)&&(param->RxLen>=len))
		{
			if(param->RxBuffer[param->RxLen-1]==xorGet(param->RxBuffer, param->RxLen-1))
			{
				param->RxValid=1;
			}
		}
		//usleep(1000);
	}
	return;
}


/*******************************************************************
*Name			:tICPoll
*Description	:IC����״̬��ѯ
*Input			:nozzle	ǹѡ0=A1ǹ��1=B1ǹ
*Output		:None
*Return			:None
*History		:2014-10-17,modified by syj
*/

//static void tICPoll(int nozzle)
void tICPoll(void* pNozzleType)
{
	int nozzle = (int*)pNozzleType;
	IcStructType *param=NULL;
	unsigned int timer=0, overtimes=0;
	IptParamStructType *iptparam=NULL;

	printf("tICPoll is success!\n");
	
	if(IC_NOZZLE_1==nozzle)	//�ж�ǹѡ
	{
		param=&IcStructA1;
		prctl(PR_SET_NAME,(unsigned long)"tICPollA");
        iptparam=&IptParamA;
	}
	else
	{
		param=&IcStructB1;
		prctl(PR_SET_NAME,(unsigned long)"tICPollB");
        iptparam=&IptParamB;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	while ( 1 )
	{
		while(0!=ICLock(param->nozzle))//��������
		{
			usleep(1000);
		}

		//printf("param->shootAsk = %d\n",param->shootAsk);

		//�е�������ʱ����ִ�е�������
		if(1==param->shootAsk)
		{
			//���͵���ָ��
			IcPackSend(param->nozzle, 0x32, 0x40, (unsigned char*)"\x00", 0);

			//printf("222222222222\n");

			//�ж����ݽ���
			for(timer=0;;)
			{
				if( 1==param->RxValid && 0x32==param->RxBuffer[3] && 0x40==param->RxBuffer[4] && 0x30==param->RxBuffer[5]) //�����ɹ�
				{
					param->State.DeckStateS1=0;
					param->State.IcTypeS2=0;
					param->State.IcStateS3=0;
					param->State.PowerStateS4=0;

					param->shootAsk=0;
					break;
				}
				else if(1==param->RxValid && 0x32==param->RxBuffer[3] && 0x40==param->RxBuffer[4] && 0x31==param->RxBuffer[5]) //����ʧ��
				{
					param->shootAsk=0;
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes<3) //����ʱ���������Σ��ٴε���
				{
	                printf("IC Shoot -- ic timeout : < 3\n");
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes>=3) //������ʱ���������Σ���������
				{
	                printf("IC Shoot -- ic timeout : >= 3\n");
					overtimes=0;	
					param->shootAsk=0;
					break;
				}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	
				timer+=10;
			}
		}
		else if(0==param->pollLimit && iptparam->EtcOilFlg == 0)
		{
			//����Ѱ��ָ��
			IcPackSend(param->nozzle, 0x31, 0x41, (unsigned char*)"\x00", 0);

			//printf("1111111111111\n");

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if(1==param->RxValid && 0x31==param->RxBuffer[3] && 0x41==param->RxBuffer[4])
				{
					//printf("shootAsk = %d\n",param->shootAsk);
					//�е�������ʱ����״̬��ʱ��Ч
					if(0==param->shootAsk)
					{
						//printf("have card opt\n");
						param->State.DeckStateS1=param->RxBuffer[5];
						param->State.IcTypeS2=param->RxBuffer[6];
						param->State.IcStateS3=param->RxBuffer[7];
						param->State.PowerStateS4=param->RxBuffer[8];
					}

					overtimes=0;
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes<3)	//������ʱ
				{
					printf("ic timeout : < 3\n");
					overtimes++;
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes>=3)	//���γ�ʱ��Ϊ�޿�
				{
					printf("ic timeout : >= 3\n");
					param->State.DeckStateS1=0;
					param->State.IcTypeS2=0;
					param->State.IcStateS3=0;
					param->State.PowerStateS4=0;
					overtimes++;
					break;
				}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);
				//select(0,NULL,NULL,NULL,&tv);
				timer+=10;
			}
		}

		//�����������
		ICUnlock(param->nozzle);

		//��ʱԼ500ms�����´�ͨѶ
		usleep(500000);
	}

	return;
}


/*******************************************************************
*Name			:tICReceive
*Description	:IC�������ݶ�ȡ����������
*Input			:nozzle	ǹѡ0=A1ǹ��1=B1ǹ
*Output		:None
*Return			:None
*History		:2014-10-17,modified by syj
*/

static void tICShoot(int nozzle)
{
	IcStructType *param=NULL;
	int timer=0, overtimes=0;

	//�ж�ǹѡ
	if(0==nozzle)	
		param=&IcStructA1;
	else			
		param=&IcStructB1;

	while ( 1 )
	{	
		while(0!=ICLock(param->nozzle))//��������
		{
			//usleep(1000);
			continue;  //fj:20171117_add
		}	
		overtimes=0;	//��ʱ��������	
		IcPackSend(param->nozzle, 0x32, 0x40, (unsigned char*)"\x00", 0);//���͵���ָ��	
		for(timer=0;;)//�ж����ݽ���
		{
			//�����ɹ�
			if((1==param->RxValid)&&(0x32==param->RxBuffer[3])&&(0x40==param->RxBuffer[4])&&(0x30==param->RxBuffer[5]))
			{
				param->State.DeckStateS1=0;
				param->State.IcTypeS2=0;
				param->State.IcStateS3=0;
				param->State.PowerStateS4=0;
				break;
			}	
			else if((1==param->RxValid)&&(0x32==param->RxBuffer[3])&&(0x40==param->RxBuffer[4])&&(0x31==param->RxBuffer[5]))//����ʧ��
			{
				break;
			}
            else if((timer>=IC_OVERTIME)&&(overtimes<3)) 	//������ʱ���������Σ��ٴε���
			{	
				break;
			}
			else if((timer>=IC_OVERTIME)&&(overtimes>=3))//������ʱ���������Σ���������
			{
				break;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	
			timer+=10;
		}
		//�����������
		ICUnlock(param->nozzle);
		//usleep(1000);
	}

	return;
}


/*******************************************************************
*Name			:ICShoot
*Description	:IC������
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�;����=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICShoot(int nozzle)
{
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))	//����POS
	{
		return ldESEjectCard(nozzle);
	}

	if(0==nozzle && IptParamA.EtcOilFlg == 0)//szb_fj_20171120,update	
	{	
		IcStructA1.State.DeckStateS1=0;	IcStructA1.State.IcTypeS2=0;
		IcStructA1.State.IcStateS3=0;	IcStructA1.State.PowerStateS4=0;
		IcStructA1.shootAsk=1;	
	}
	else if(IptParamB.EtcOilFlg == 0)//szb_fj_20171120,update					
	{
		IcStructB1.State.DeckStateS1=0;	IcStructB1.State.IcTypeS2=0;
		IcStructB1.State.IcStateS3=0;	IcStructB1.State.PowerStateS4=0;
		IcStructB1.shootAsk=1;
	}

	return 0;
}


/*******************************************************************
*Name			:ICReset
*Description	:IC����λ
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*					:sam			0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*Output		:None
*Return		:0=�����ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICReset(int nozzle, int sam)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	IcStructType *param=NULL;
	unsigned int timer=0;

	//szb_fj_20171120,add
	if(0==nozzle && IptParamA.EtcOilFlg==1)
		return 0;
	else if(0==nozzle && IptParamB.EtcOilFlg==1)
		return 0;

	//�жϲ�������
	if(0==nozzle)			
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else							
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		istate = ldICPowerUp(nozzle, LD_ICDEV_USERCARD);
		if(0 == istate)	
			return 0;
		else					
			return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//����selectѡ��������뿨��SIM�������ÿ�
	if(0==sam)
	{
		//���Ϳ���λ����
		if(IC_CARD_CPU==param->State.IcTypeS2)				
			IcPackSend(param->nozzle, 0x37, 0x40, (unsigned char*)"\x00", 0);
		else if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	
			IcPackSend(param->nozzle, 0x35, 0x40, (unsigned char*)"\x00", 0);
		else if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	
			IcPackSend(param->nozzle, 0x34, 0x40, (unsigned char*)"\x00", 0);

		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if(1==param->RxValid &&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x40==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5] || 'Z'==param->RxBuffer[5]))
			{
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if(1==param->RxValid &&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x40==param->RxBuffer[4])))
			{	 
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	
			timer+=10;
		}

	}
	else
	{
		//���͸�λ����
		tx_buffer[0]=sam;
		tx_len=1;
		IcPackSend(param->nozzle, 0x3d, 0x41, tx_buffer, tx_len);
		
		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
				(('Y'==param->RxBuffer[6])||('Z'==param->RxBuffer[6])))
			{
				ICUnlock(param->nozzle);
				return 0;
			}
            else if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x33==param->RxBuffer[5]))
			{
				ICUnlock(param->nozzle);//����ʧ��
				return 1;
			}	
			else if(timer>=IC_OVERTIME)//������ʱ
			{	
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	
			timer+=10;
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICMFSelect
*Description	:IC��MFѡ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*				:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*				:maxbytes	���������󳤶�
*Output			:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICMFSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else				
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xa4\x04\x00", 4);	tx_len += 4;
		tx_buffer[tx_len] = 14;													tx_len += 1;
		memcpy(tx_buffer + tx_len, "1PAY.SYS.DDF01", 14);	tx_len += 14;
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	
			return 0;
		else		
			return ERROR;
	}

	if(0!=ICLock(param->nozzle))//��������
		return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���Ϳ�MFѡ������*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x13;
		memcpy(&tx_buffer[2], "\x00\xa4\x04\x00", 4);
		tx_buffer[6]=14;
		memcpy(&tx_buffer[7], "1PAY.SYS.DDF01", 14);
		tx_len=21;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)	//����selectѡ��������뿨��SIM�������ÿ�
		{
			//���Ϳ�MFѡ������
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x13;
			memcpy(&tx_buffer[2], "\x00\xa4\x04\x00", 4);
			tx_buffer[6]=14;
			memcpy(&tx_buffer[7], "1PAY.SYS.DDF01", 14);
			tx_len=21;
			if(IC_CARD_CPU==param->State.IcTypeS2)
				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)
				IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)
				IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			for(timer=0;;)//�ж����ݽ���
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5]))
				{	
					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}	
				else if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4])))
				{	 
					ICUnlock(param->nozzle);	//����ʧ��
					return 1;
				}
				else if(timer>=IC_OVERTIME)//������ʱ
				{
					ICUnlock(param->nozzle);
					return 2;
				}
				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	
				timer+=10;
			}
		}
		else
		{
			//����MFѡ������
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x13;
			memcpy(&tx_buffer[3], "\x00\xa4\x04\x00", 4);
			tx_buffer[7]=14;
			memcpy(&tx_buffer[8], "1PAY.SYS.DDF01", 14);
			tx_len=22;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);	
			for(timer=0;;)//�ж����ݽ���
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}	
				else if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
						('N'==param->RxBuffer[6]))//����ʧ��
				{
					ICUnlock(param->nozzle);
					return 1;
				}	
				else if(timer>=IC_OVERTIME)//������ʱ
				{
					ICUnlock(param->nozzle);
					return 2;
				}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	
				timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICAppSelect
*Description	:IC��Ӧ��ѡ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:app				0=������Ʊ��1=����Ӧ��
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICAppSelect(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char app)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)
	{
		param=&IcStructA1;
        iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
        iptparam=&IptParamB;
	}
	else	
		return 3;
	
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))//����POS
	{
		memcpy(tx_buffer + tx_len, "\x00\xa4\x04\x00", 4);	tx_len += 4;
		tx_buffer[tx_len] = 12;													tx_len += 1;
		if(0==app)	
			memcpy(tx_buffer + tx_len, "\xa0\x00\x00\x00\x03SINOPEC", 12);
		else				
			memcpy(tx_buffer + tx_len, "\xa0\x00\x00\x00\x03LOYALTY", 12);
		tx_len += 12;

		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)
			return 0;
		else				
			return ERROR;
	}

	if(0!=ICLock(param->nozzle)) //��������
		return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x11;
		memcpy(&tx_buffer[2], "\x00\xa4\x04\x00", 4);
		tx_buffer[6]=12;
		if(0==app)	memcpy(&tx_buffer[7], "\xa0\x00\x00\x00\x03SINOPEC", 12);
		else				memcpy(&tx_buffer[7], "\xa0\x00\x00\x00\x03LOYALTY", 12);
		tx_len=19;				
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)	//����selectѡ��������뿨��SIM�������ÿ�
		{
			//���Ϳ�Ӧ��ѡ������
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x11;
			memcpy(&tx_buffer[2], "\x00\xa4\x04\x00", 4);
			tx_buffer[6]=12;
			if(0==app)	
				memcpy(&tx_buffer[7], "\xa0\x00\x00\x00\x03SINOPEC", 12);
			else				
				memcpy(&tx_buffer[7], "\xa0\x00\x00\x00\x03LOYALTY", 12);
			tx_len=19;				
			if(IC_CARD_CPU==param->State.IcTypeS2)				
				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	
				IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	
				IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}
				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//���Ϳ�Ӧ��ѡ������
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x11;
			memcpy(&tx_buffer[3], "\x00\xa4\x04\x00", 4);
			tx_buffer[7]=12;
			if(0==app)	memcpy(&tx_buffer[8], "\xa0\x00\x00\x00\x03SINOPEC", 12);
			else				memcpy(&tx_buffer[8], "\xa0\x00\x00\x00\x03LOYALTY", 12);
			tx_len=20;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile21Read
*Description	:IC��21�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICFile21Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
        iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x95\x00\x1E", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*����21�ļ���ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x95\x00\x1E", 5);
		tx_len=7;				
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);			
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//����21�ļ���ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x95\x00\x1E", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//����21�ļ���ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x95\x00\x1E", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile22Read
*Description	:IC��22�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICFile22Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)	
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x96\x00\x29", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*����22�ļ���ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x96\x00\x29", 5);
		tx_len=7;			
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//����22�ļ���ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x96\x00\x29", 5);
			tx_len=7;			
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//����22�ļ���ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x96\x00\x29", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile27Read
*Description	:IC��27�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICFile27Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)	
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9B\x00\x20", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(iptparam->EtcOilFlg==1)
	{
		/*����27�ļ���ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9B\x00\x20", 5);
		tx_len=7;			
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//����27�ļ���ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x9B\x00\x20", 5);
			tx_len=7;					
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//����27�ļ���ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x9B\x00\x04", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile28Read
*Description	:IC��28�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICFile28Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9C\x00\x42", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*����28�ļ���ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9C\x00\x42", 5);
		tx_len=7;		
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//����28�ļ���ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x9C\x00\x42", 5);
			tx_len=7;			
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;

			}
		}
		else
		{
			//����28�ļ���ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x9C\x00\x42", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICNotesRead
*Description	:IC��������ϸ��ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:notes_id		������ϸ��Ŀ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICNotesRead(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char notes_id)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)		
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else	
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		tx_buffer[tx_len++] = 0x00;	tx_buffer[tx_len++] = 0xb2;
		tx_buffer[tx_len++] = notes_id;	
		tx_buffer[tx_len++] = 0xc4;	tx_buffer[tx_len++] = 0x17;

		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���ͽ�����ϸ��ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		tx_buffer[2]=0x00;	tx_buffer[3]=0xb2;	tx_buffer[4]=notes_id;
		tx_buffer[5]=0xc4;	tx_buffer[6]=0x17;
		tx_len=7;	
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//���ͽ�����ϸ��ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			tx_buffer[2]=0x00;	tx_buffer[3]=0xb2;	tx_buffer[4]=notes_id;
			tx_buffer[5]=0xc4;	tx_buffer[6]=0x17;
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;

			}
		}
		else
		{
			//���ͽ�����ϸ��ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			tx_buffer[3]=0x00;	tx_buffer[4]=0xb2;	tx_buffer[5]=notes_id;
			tx_buffer[6]=0xc4;	tx_buffer[7]=0x17;
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICPinCheck
*Description	:IC������У��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:pin				����ASCII
*					:pin_len			����λ��0~12
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICPinCheck(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *pin, int pin_len)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	unsigned char  i=0, pass[12]={0}, pass_len=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	//�жϲ�������
	if(0==nozzle)	
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
        iptparam=&IptParamB;
	}
	else	
		return 3;

	//�ж����볤��
	if(0==pin_len || pin_len>12)	return ERROR;

	//�������Ϊѹ��BCD��ʽ�������F
	if(0==(pin_len%2))	pass_len=pin_len/2;
	else							pass_len=pin_len/2+1;
	for(i=0; i<pin_len; i++)
	{
		if(0==(i%2))	pass[i/2]=pass[i/2]|((pin[i]&0x0f)<<4);
		else					pass[i/2]=pass[i/2]|((pin[i]&0x0f)<<0);

		if((i+1)>=pin_len && 0==(i%2))	pass[i/2]=pass[i/2]|0x0f;
	}

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\x20\x00\x00", 4);	tx_len += 4;
		tx_buffer[tx_len++] = pass_len;
		memcpy(tx_buffer + tx_len, pass, pass_len);				tx_len += pass_len;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���Ϳ�������֤����*/
		tx_buffer[0]=(unsigned char)((5+pass_len)>>8);
		tx_buffer[1]=(unsigned char)((5+pass_len)>>0);
		memcpy(&tx_buffer[2], "\x00\x20\x00\x00", 4);
		tx_buffer[6]=pass_len;
		memcpy(&tx_buffer[7], pass, pass_len);
		tx_len=7+pass_len;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//���Ϳ�������֤����
			tx_buffer[0]=(unsigned char)((5+pass_len)>>8);
			tx_buffer[1]=(unsigned char)((5+pass_len)>>0);
			memcpy(&tx_buffer[2], "\x00\x20\x00\x00", 4);
			tx_buffer[6]=pass_len;
			memcpy(&tx_buffer[7], pass, pass_len);
			tx_len=7+pass_len;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//���Ϳ�������֤����
			tx_buffer[0]=sam;
			tx_buffer[1]=(unsigned char)((5+pass_len)>>8);
			tx_buffer[2]=(unsigned char)((5+pass_len)>>0);
			memcpy(&tx_buffer[3], "\x00\x20\x00\x00", 4);
			tx_buffer[7]=pass_len;
			memcpy(&tx_buffer[8], pass, pass_len);
			tx_len=8+pass_len;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyInfoRead
*Description	:IC��������Ϣ��ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICGreyInfoRead(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)		
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else	
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\xCA\x00\x00\x1E", 5);	
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���ͻ�����Ϣ��ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\xE0\xCA\x00\x00\x1E", 5);
		tx_len=0x07;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//���ͻ�����Ϣ��ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\xE0\xCA\x00\x00\x1E", 5);
			tx_len=0x07;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//���ͻ�����Ϣ��ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\xE0\xCA\x00\x00\x1E", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICBalanceRead
*Description	:IC������ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICBalanceRead(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)		
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x80\x5C\x00\x01\x04", 5);	
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*��������ȡ����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x80\x5C\x00\x01\x04", 5);
		tx_len=7;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//��������ȡ����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x80\x5C\x00\x01\x04", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//��������ȡ����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x80\x5C\x00\x01\x04", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICLockInit
*Description	:IC��������ʼ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:keyIndex		PSAM������Կ������
*					:termId			PSAM�ն˻���ţ�6bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICLockInit(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char keyIndex, unsigned char *termId)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\x7A\x08\x01\x07", 5);		tx_len += 5;
		tx_buffer[tx_len++] = keyIndex;
		memcpy(tx_buffer + tx_len, termId, 6);									tx_len += 6;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���ͻ�����ʼ������*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x0c;
		memcpy(&tx_buffer[2], "\xE0\x7A\x08\x01\x07", 5);
		tx_buffer[7]=keyIndex;
		memcpy(&tx_buffer[8], termId, 6);
		tx_len=14;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//���ͻ�����ʼ������
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x0c;
			memcpy(&tx_buffer[2], "\xE0\x7A\x08\x01\x07", 5);
			tx_buffer[7]=keyIndex;
			memcpy(&tx_buffer[8], termId, 6);
			tx_len=14;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//���ͻ�����ʼ������
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x0c;
			memcpy(&tx_buffer[3], "\xE0\x7A\x08\x01\x07", 5);
			tx_buffer[8]=keyIndex;
			memcpy(&tx_buffer[9], termId, 6);
			tx_len=15;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyLock
*Description	:IC������
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:psamTTC				PSAM�ն˽�����ţ�4bytes
*					:psamRandom		PSAM�ն��������4bytes
*					:time					����ʱ�䣬7bytes
*					:psamMAC1			PSAM�����MAC1��4bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICGreyLock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *psamTTC, unsigned char *psamRandom, unsigned char *time, unsigned char *psamMAC1)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)		
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\x7C\x08\x00\x13", 5);	tx_len += 5;
		memcpy(tx_buffer + tx_len, psamTTC, 4);							tx_len += 4;
		memcpy(tx_buffer + tx_len, psamRandom, 4);					tx_len += 4;
		memcpy(tx_buffer + tx_len, time, 7);									tx_len += 7;
		memcpy(tx_buffer + tx_len, psamMAC1, 4);						tx_len += 4;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���ͻ�������*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x18;
		memcpy(&tx_buffer[2], "\xE0\x7C\x08\x00\x13", 5);
		memcpy(&tx_buffer[7], psamTTC, 4);
		memcpy(&tx_buffer[11], psamRandom, 4);
		memcpy(&tx_buffer[15], time, 7);
		memcpy(&tx_buffer[22], psamMAC1, 4);
		tx_len=26;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);
            usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//���ͻ�������
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x18;
			memcpy(&tx_buffer[2], "\xE0\x7C\x08\x00\x13", 5);
			memcpy(&tx_buffer[7], psamTTC, 4);
			memcpy(&tx_buffer[11], psamRandom, 4);
			memcpy(&tx_buffer[15], time, 7);
			memcpy(&tx_buffer[22], psamMAC1, 4);
			tx_len=26;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//���ͻ�������
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x18;
			memcpy(&tx_buffer[3], "\xE0\x7C\x08\x00\x13", 5);
			memcpy(&tx_buffer[8], psamTTC, 4);
			memcpy(&tx_buffer[12], psamRandom, 4);
			memcpy(&tx_buffer[16], time, 7);
			memcpy(&tx_buffer[23], psamMAC1, 4);
			tx_len=27;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyUnlock
*Description	:IC������
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:money				���ѽ�HEX
*					:ICLockInitCTC	�ѻ�������ţ�2bytes
*					:PsamTermId	PSAM�ն˻���ţ�6bytes
*					:ICPsamTTC		PSAM�ն˽�����ţ�4bytes
*					:time				����ʱ�䣬7bytes
*					:ICPsamGMAC	GMAC��4bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICGreyUnlock(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned int money,unsigned char *ICLockInitCTC, unsigned char *PsamTermId, unsigned char *ICPsamTTC,unsigned char *time, unsigned char *ICPsamGMAC)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)		
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\x7E\x08\x01\x1B", 5);	tx_len += 5;
		tx_buffer[tx_len++] = money>>24;	tx_buffer[tx_len++] = money>>16;
		tx_buffer[tx_len++] = money>>8;	tx_buffer[tx_len++] = money>>0;
		memcpy(tx_buffer + tx_len, ICLockInitCTC, 2);					tx_len += 2;
		memcpy(tx_buffer + tx_len, PsamTermId, 6);						tx_len += 6;
		memcpy(tx_buffer + tx_len, ICPsamTTC, 4);						tx_len += 4;
		memcpy(tx_buffer + tx_len, time, 7);									tx_len += 7;
		memcpy(tx_buffer + tx_len, ICPsamGMAC, 4);					tx_len += 4;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*���ͽ�������*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x20;
		memcpy(&tx_buffer[2], "\xE0\x7E\x08\x01\x1B", 5);
		tx_buffer[7]=money>>24;	tx_buffer[8]=money>>16;
		tx_buffer[9]=money>>8;		tx_buffer[10]=money>>0;
		memcpy(&tx_buffer[11], ICLockInitCTC, 2);
		memcpy(&tx_buffer[13], PsamTermId, 6);
		memcpy(&tx_buffer[19], ICPsamTTC, 4);
		memcpy(&tx_buffer[23], time, 7);
		memcpy(&tx_buffer[30], ICPsamGMAC, 4);
		tx_len=34;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//���ͽ�������
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x20;
			memcpy(&tx_buffer[2], "\xE0\x7E\x08\x01\x1B", 5);
			tx_buffer[7]=money>>24;	tx_buffer[8]=money>>16;
			tx_buffer[9]=money>>8;		tx_buffer[10]=money>>0;
			memcpy(&tx_buffer[11], ICLockInitCTC, 2);
			memcpy(&tx_buffer[13], PsamTermId, 6);
			memcpy(&tx_buffer[19], ICPsamTTC, 4);
			memcpy(&tx_buffer[23], time, 7);
			memcpy(&tx_buffer[30], ICPsamGMAC, 4);
			tx_len=34;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){				
					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					//printf("sam aaaaaaaa\n");
					//PrintH(7,param->RxBuffer);

					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);
				timer+=10;
			}
		}
		else
		{
			printf("money = %d",money);
			printf("CTC = %02x%02x\n",ICLockInitCTC[0],ICLockInitCTC[1]);
			printf("PsamTermID = "); PrintH(6,PsamTermId);
			printf("PsamTTC = ");    PrintH(4,ICPsamTTC);
			printf("OilTime = ");    PrintH(7,time);
			printf("PsamGMAC = ");   PrintH(4,ICPsamGMAC);
			//���ͽ�������
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x20;
			memcpy(&tx_buffer[3], "\xE0\x7E\x08\x01\x1B", 5);
			tx_buffer[8]=money>>24;	tx_buffer[9]=money>>16;
			tx_buffer[10]=money>>8;	tx_buffer[11]=money>>0;
			memcpy(&tx_buffer[12], ICLockInitCTC, 2);
			memcpy(&tx_buffer[14], PsamTermId, 6);
			memcpy(&tx_buffer[20], ICPsamTTC, 4);
			memcpy(&tx_buffer[24], time, 7);
			memcpy(&tx_buffer[31], ICPsamGMAC, 4);
			tx_len=35;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);

					//printf("ICC aaaa\n");
					//PrintH(7+2+apdu_len,param->RxBuffer);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}	
					else if(timer>=IC_OVERTIME)//������ʱ
					{
						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ	
				usleep(10*1000);	
				timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICTacClr
*Description	:IC�����TAC
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICTacClr(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;
	IptParamStructType *iptparam=NULL;

	/*�жϲ�������*/
	if(0==nozzle)
	{
		param=&IcStructA1;
		iptparam=&IptParamA;
	}
	else if(1==nozzle)	
	{
		param=&IcStructB1;
		iptparam=&IptParamB;
	}
	else
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\xCA\x01\x00\x00", 5);	tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*�������TAC����*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\xE0\xCA\x01\x00\x00", 5);
		tx_len=7;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*�����ɹ�*/
			if(iptparam->etc_rec_buff[0]==ETC_CMD && iptparam->etc_rec_buff[1]==ETC_05){

				if(iptparam->etc_rec_buff[2]==0 && iptparam->etc_rec_buff[3]==iptparam->LogicNozzle)
				{
					apdu_len=iptparam->etc_rec_len-ETC_05_H_LEN;
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}
					buffer[0]=(apdu_len>>8);
					buffer[1]=(apdu_len>>0);
					memcpy(buffer+2, iptparam->etc_rec_buff+ETC_05_H_LEN, apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}
				else
				{
					ICUnlock(param->nozzle);
					return 1;
				}
			}

			/*������ʱ*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*�ȴ���ʱ���ۼƳ�ʱֵ*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//�������TAC����
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\xE0\xCA\x01\x00\x00", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//�ж�APDU����
					apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//�������TAC����
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\xE0\xCA\x01\x00\x00", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//�ж����ݽ���
			for(timer=0;;)
			{
				//�����ɹ�
				if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
						('Y'==param->RxBuffer[6]))
				{
					apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
					if(maxbytes<(2+apdu_len))
					{
						ICUnlock(param->nozzle);
						return 4;
					}

					memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
					ICUnlock(param->nozzle);
					return 0;
				}

				//����ʧ��
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//������ʱ
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//�ȴ���ʱ���ۼƳ�ʱֵ
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile26Read
*Description	:IC��26�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICFile26Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9a\x00\x02", 5);	tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//����26�ļ���ȡ����
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9a\x00\x02", 5);
		tx_len=7;
		if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//�ж�APDU����
				apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//����26�ļ���ȡ����
		tx_buffer[0]=sam;
		tx_buffer[1]=0x00;
		tx_buffer[2]=0x05;
		memcpy(&tx_buffer[3], "\x00\xB0\x9a\x00\x02", 5);
		tx_len=8;
		IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
				('Y'==param->RxBuffer[6]))
			{
				apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICDESCrypt
*Description	:IC��ר��DES����(DES CRYPT)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:KeyIndex			��֤��Կ������(ACT������)
*					:PSAMRandom	PSAM�������4bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICDESCrypt(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *PSAMRandom)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		tx_buffer[tx_len++] = 0x80;	tx_buffer[tx_len++] = 0xa8;	tx_buffer[tx_len++] = 0x00;
		tx_buffer[tx_len++] = KeyIndex;	
		tx_buffer[tx_len++] = 0x08;
		memcpy(tx_buffer + tx_len, PSAMRandom, 4);				tx_len += 4;
		memcpy(tx_buffer + tx_len, "\x00\x00\x00\x00", 4);		tx_len += 4;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//����ר��DES��������
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x0d;
		tx_buffer[2]=0x80;	tx_buffer[3]=0xa8;	tx_buffer[4]=0x00;
		tx_buffer[5]=KeyIndex;	tx_buffer[6]=0x08;
		memcpy(&tx_buffer[7], PSAMRandom, 4);	
		memcpy(&tx_buffer[11], "\x00\x00\x00\x00", 4);
		tx_len=15;
		if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//�ж�APDU����
				apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//����ר��DES��������
		tx_buffer[0]=sam;
		tx_buffer[1]=0x00;
		tx_buffer[2]=0x0d;
		tx_buffer[3]=0x80;	tx_buffer[4]=0xa8;	tx_buffer[5]=0x00;
		tx_buffer[6]=KeyIndex;	tx_buffer[7]=0x08;
		memcpy(&tx_buffer[8], PSAMRandom, 4);	
		memcpy(&tx_buffer[12], "\x00\x00\x00\x00", 4);
		tx_len=16;
		IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
				('Y'==param->RxBuffer[6]))
			{
				apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcAppendLog
*Description	:PSAM�������־��¼(APPEND LOG RECORD)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:sam				0=����뿨��0x30~0x33'��ʾ���ÿ�����'0'~'3'
*					:maxbytes	���������󳤶�
*					:time					����������ʱ�䣬7bytes
*					:ACTAppId				ACT�����ţ�10bytes
*					:ACTKeyIndex		ACT��֤��Կ������
*					:RIDAppId				RID�����ţ�10bytes
*					:RIDKeyIndex		RID��֤��Կ������
*					:RIDCalKeyIndex	��־MAC������Կ������
*					:PsamId				PSAM�����ţ�10bytes
*					:mboardID			�ͻ�оƬID��8bytes
*					:RIDMAC				RID��֤MAC��4bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICAppendLog(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardID, unsigned char *RIDMAC)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x80\xe6\x00\x00\x34", 5);		tx_len += 5;
		memcpy(tx_buffer + tx_len, time, 7);									tx_len += 7;
		memcpy(tx_buffer+ tx_len, ACTAppId, 10);							tx_len += 10;
		tx_buffer[tx_len++] = ACTKeyIndex;							
		memcpy(tx_buffer + tx_len, RIDAppId, 10);						tx_len += 10;
		tx_buffer[tx_len++]=RIDKeyIndex;								
		tx_buffer[tx_len++]=RIDCalKeyIndex;		
		memcpy(tx_buffer + tx_len, PsamId, 10);							tx_len += 10;
		memcpy(tx_buffer + tx_len, mboardID, 8);							tx_len += 8;
		memcpy(tx_buffer + tx_len, RIDMAC, 4);								tx_len += 4;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//���Ϳ������־��¼����
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x39;
		memcpy(&tx_buffer[2], "\x80\xe6\x00\x00\x34", 5);
		memcpy(&tx_buffer[7], time, 7);	
		memcpy(&tx_buffer[14], ACTAppId, 10);	
		tx_buffer[24]=ACTKeyIndex;							
		memcpy(&tx_buffer[25], RIDAppId, 10);			
		tx_buffer[35]=RIDKeyIndex;								
		tx_buffer[36]=RIDCalKeyIndex;		
		memcpy(&tx_buffer[37], PsamId, 10);	
		memcpy(&tx_buffer[47], mboardID, 8);	
		memcpy(&tx_buffer[55], RIDMAC, 4);	
		tx_len=59;
		if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//�ж�APDU����
				apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//���Ϳ������־��¼����
		tx_buffer[0]=sam;
		tx_buffer[1]=0x00;
		tx_buffer[2]=0x39;
		memcpy(&tx_buffer[3], "\x80\xe6\x00\x00\x34", 5);
		memcpy(&tx_buffer[8], time, 7);	
		memcpy(&tx_buffer[15], ACTAppId, 10);	
		tx_buffer[25]=ACTKeyIndex;							
		memcpy(&tx_buffer[26], RIDAppId, 10);			
		tx_buffer[36]=RIDKeyIndex;								
		tx_buffer[37]=RIDCalKeyIndex;		
		memcpy(&tx_buffer[38], PsamId, 10);	
		memcpy(&tx_buffer[48], mboardID, 8);	
		memcpy(&tx_buffer[56], RIDMAC, 4);	
		tx_len=60;
		IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

		//�ж����ݽ���
		for(timer=0;;)
		{
			//�����ɹ�
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
				('Y'==param->RxBuffer[6]))
			{
				apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
				if(maxbytes<(2+apdu_len))
				{
					ICUnlock(param->nozzle);
					return 4;
				}

				memcpy(buffer, &param->RxBuffer[7], 2+apdu_len);
				ICUnlock(param->nozzle);
				return 0;
			}

			//����ʧ��
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//������ʱ
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//�ȴ���ʱ���ۼƳ�ʱֵ
			usleep(10*1000);	timer+=10;
		}
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:ICKeyADFSelect
*Description	:IC��Կ��Ӧ��ѡ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyADFSelect(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)	
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else	
		return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
	    printf("ICKeyADFSelect  pos bbbbbbbb\n");

		memcpy(tx_buffer + tx_len, "\x00\xa4\x00\x00\x02\x10\x01", 7);		
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//���Ϳ�Ӧ��ѡ������
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	memcpy(&tx_buffer[2], "\x00\xa4\x00\x00\x02\x10\x01", 7);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)
	{
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	}
	else if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)
	{
		IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	}
	else if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)
	{
		IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);
	}

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5]))
		{		
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4])))
		{
		    PrintH(6,param->RxBuffer);		 	
			printf("ICKeyADFSelect eeeeeee,RxBuffer[5] = %c\n",param->RxBuffer[5]);
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME)
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}
		
	//�����������
	ICUnlock(param->nozzle);


	return 0;
}


/*******************************************************************
*Name			:ICKeyEF01Select
*Description	:IC��Կ��ѡ��EF 01�ļ�
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Select(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xA4\x00\x00\x02\xEF\x01", 7);		
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����ѡ��EF 01�ļ�����
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	memcpy(&tx_buffer[2], "\x00\xA4\x00\x00\x02\xEF\x01", 7);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}
	
	//�����������
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:ICKeyEF01Read
*Description	:IC��Կ����ȡEF 01�ļ�
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Read(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x00\x00\x20", 5);		
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//���Ͷ�ȡEF 01�ļ�����
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x05;
	memcpy(&tx_buffer[2], "\x00\xB0\x00\x00\x20", 5);
	tx_len=7;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}
	
	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF01Write
*Description	:IC��Կ��δ���������޸�
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:number		δ��������
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Write(int nozzle, unsigned char *buffer, int maxbytes, unsigned int number)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xd6\x00\x02\x02", 5);	tx_len += 5;
		tx_buffer[tx_len++] = (char)(number >> 8);
		tx_buffer[tx_len++] = (char)(number >> 0);
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����дEF 01�ļ�����
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	tx_buffer[2]=0x00;	tx_buffer[3]=0xd6;tx_buffer[4]=0x00;tx_buffer[5]=0x02;tx_buffer[6]=0x02;
	tx_buffer[7]=(char)(number>>8);
	tx_buffer[8]=(char)(number>>0);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF02Select
*Description	:IC��Կ��ѡ��EF 02�ļ�
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyEF02Select(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xA4\x00\x00\x02\xEF\x02", 7);	
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����ѡ��EF 02�ļ�����
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	memcpy(&tx_buffer[2], "\x00\xA4\x00\x00\x02\xEF\x02", 7);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}
	
	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF02Read
*Description	:IC��Կ��дEF 01�ļ�
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:offset			��ȡ��ʼλ��ƫ��
*					:readbytes	Ҫ��ȡ�����ݳ���
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICKeyEF02Read(int nozzle, unsigned char *buffer, int maxbytes, unsigned int offset, int readbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		tx_buffer[tx_len++] = 0x00;	tx_buffer[tx_len++] = 0xb0;
		tx_buffer[tx_len++] = (char)(offset >> 8);
		tx_buffer[tx_len++] = (char)(offset >> 0);
		tx_buffer[tx_len++] = readbytes;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//���Ͷ�EF 02�ļ�����
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x05;
	tx_buffer[2]=0x00;	tx_buffer[3]=0xb0;
	tx_buffer[4]=(char)(offset>>8);tx_buffer[5]=(char)(offset>>0);
	tx_buffer[6]=readbytes;
	tx_len=7;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//�ж�APDU����
			apdu_len=(param->RxBuffer[6]<<8)|(param->RxBuffer[7]<<0);
			if(maxbytes<(2+apdu_len))
			{
				ICUnlock(param->nozzle);
				return 4;
			}

			memcpy(buffer, &param->RxBuffer[6], 2+apdu_len);
			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}






/*******************************************************************
*Name				:IcPsamApduSend
*Description		:��װPSAM��APDU����
*Input				:nozzle				IC_NOZZLE_1=1�ſ��豸��IC_NOZZLE_2=2�ſ��豸��
*					:sam			    �ж������Ƿ����������Ǽ��� (>=0x30������<0x30����)
*					:inbuffer			apdu����
*					:inbytes			apdu���ݳ���
*					:maxbytes			������ݻ��泤��
*Output				:outbuffer			������ݻ���
*Return				:0=�ɹ���1=ʧ�ܣ�2=��ʱ������=����
*History			:
*/

int IcPsamApduSend(int nozzle, int sam, const unsigned char *inbuffer, int inbytes, char *outbuffer, int maxbytes)
{
	int apdu_len=0;
	int tx_len=0;
	int i_outbytes=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	unsigned char tx_buffer[128]={0};
	unsigned char i_outbuffer[128]={0};
	int istate = 0;
 	 //�жϲ�������
	if(0==nozzle)			
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else							
		return 3;


  	if(sam>='0')
	{
    //��������
  	if(0!=ICLock(param->nozzle))	
		return ERROR;

    IcPackSend(param->nozzle, 0x3d, 0x43, inbuffer, inbytes);

  	//�ж����ݽ���
  	for(timer=0;;)
  	{
  		//�����ɹ�
  		if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])&&\
  			('Y'==param->RxBuffer[6]))
  		{
  			apdu_len=(param->RxBuffer[7]<<8)|(param->RxBuffer[8]<<0);
  			if(maxbytes<(2+apdu_len))
  			{
  				ICUnlock(param->nozzle);
  				return 4;
  			}

  			memcpy(outbuffer, &param->RxBuffer[7], 2+apdu_len);
  			ICUnlock(param->nozzle);
  			return 0;
  		}

  		//����ʧ��
  		else
  		if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])&&\
  			('N'==param->RxBuffer[6]))
  		{
  			ICUnlock(param->nozzle);
  			return 1;
  		}

  		//������ʱ
  		else if(timer>=IC_OVERTIME)
  		{
  			ICUnlock(param->nozzle);
  			return 2;
  		}

  		//�ȴ���ʱ���ۼƳ�ʱֵ
  		usleep(10*1000);	timer+=10;
  	}

  	//�����������
  	ICUnlock(param->nozzle);

  }
  else
  {
     //��������
  	if(0!=ICLock(param->nozzle))	return ERROR;
    
    tx_buffer[0]=0x02;
  	tx_buffer[1]=(unsigned char)((inbytes+2)>>8);
  	tx_buffer[2]=(unsigned char)((inbytes+2)>>0);
  	tx_buffer[3]=0x3d;
  	tx_buffer[4]=0x43;
  	memcpy(&tx_buffer[5], inbuffer, inbytes);
  	tx_buffer[5+inbytes]=0x03;
  	tx_buffer[6+inbytes]=xorGet(tx_buffer, 6+inbytes);
  	tx_len=7+inbytes;
	istate = kbSetPsamTransmit(nozzle,tx_buffer,tx_len,i_outbuffer,&i_outbytes);
	if(0 == istate && i_outbytes>9)
	{
		memcpy(outbuffer, &i_outbuffer[7], i_outbytes-9);
	}

	
    //if(0!=)
	//{
	//	ICUnlock(param->nozzle);
	//	return 1;
	//}
	//�������
    //if(i_outbytes>9)	memcpy(outbuffer, &i_outbuffer[7], i_outbytes-9);

    //�����������
  	ICUnlock(param->nozzle);

	return istate;
  }

	return 0;
}



/*******************************************************************
*Name			:PsamReset
*Description	:PSAM����λ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output		:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

 int PsamReset(int nozzle,int sam)
{
	unsigned char tx_buffer[128]={0};
	unsigned char i_outbuffer[128]={0};
	int tx_len=0, istate=0;
	int i_outbytes=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
 	unsigned char sambuffer=(char)sam;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//����POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		//istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);;
	}

	if(sam>='0')
	{
	  	//��������
	  	if(0!=ICLock(param->nozzle))	return ERROR;

	  	//����PSAM��λ����    
	  	IcPackSend(param->nozzle, 0x3d, 0x41, &sambuffer, 1);
	  	
	  	//�ж����ݽ���
	  	for(timer=0;;)
	  	{
	  		if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])&&\
	  			(('Y'==param->RxBuffer[6])||('Z'==param->RxBuffer[6]))) //�����ɹ�
	  		{
	  			ICUnlock(param->nozzle);
	  			return 0;
	  		}
	  		else if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])) //����ʧ��
	  		{
	  			ICUnlock(param->nozzle);
	  			return 1;
	  		}
	  		else if(timer>=IC_OVERTIME)//������ʱ
	  		{
	  			ICUnlock(param->nozzle);
	  			return 2;
	  		}

	  		//�ȴ���ʱ���ۼƳ�ʱֵ
	  		usleep(10*1000);	
			timer+=10;
	  	}

		//�����������
	  	ICUnlock(param->nozzle);
	}
	else
  	{
		//��������
  		if(0!=ICLock(param->nozzle))	return ERROR;
    
	    tx_buffer[0]=0x02;
	  	tx_buffer[1]=(unsigned char)(3>>8);
	  	tx_buffer[2]=(unsigned char)(3>>0);
	  	tx_buffer[3]=0x3d;
	  	tx_buffer[4]=0x41;
	  	tx_buffer[5]=(char)sam;
	  	tx_buffer[6]=0x03;
	  	tx_buffer[7]=xorGet(tx_buffer, 7);
	  	tx_len=8;
		istate = kbSetPsamTransmit(nozzle,tx_buffer,tx_len,i_outbuffer,&i_outbytes);
		if(0==istate && ('Y'==i_outbuffer[6] || 'Z'==i_outbuffer[6]))	istate = 0;
		else if(1==istate || 2==istate || 3==istate || 4==istate)		istate = istate;
		else																							istate = ERROR;

	    //if(0!=)
		//{
		//	ICUnlock(param->nozzle);
		//	return 1;
		//}

	    //�����������
	  	ICUnlock(param->nozzle);

		return istate;
	}

	return 0;
}



/*******************************************************************
*Name			:PsamMFSelect
*Description	:PSAM��MFѡ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamMFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
		
	//����PSAM��MFѡ������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x13;
	memcpy(&tx_buffer[3], "\x00\xa4\x04\x00", 4);
	tx_buffer[7]=0x0e;
	memcpy(&tx_buffer[8], "1PAY.SYS.DDF01", 0x0e);
	tx_len=0x16;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);
	
	return istate;
}


/*******************************************************************
*Name			:PsamFile21Read
*Description	:PSAM��21�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamFile21Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM��21�ļ���ȡ����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	memcpy(&tx_buffer[3], "\x00\xb0\x95\x00\x0E", 5);
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamFile22Read
*Description	:PSAM��22�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamFile22Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM��22�ļ���ȡ����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	memcpy(&tx_buffer[3], "\x00\xb0\x96\x00\x06", 5);
	tx_len=8;
	istate  = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamDF1Select
*Description	:PSAM��DFѡ��ʯ��Ӧ��1
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:DF				Ӧ��ѡ��1=Ӧ��1��2=Ӧ��2
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamDFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes, int DF)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM��DFѡ��ʯ��Ӧ��1����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x12;
	if(1==DF)	memcpy(&tx_buffer[3], "\x00\xa4\x04\x00\x0d\xa0\x00\x00\x00\x03SINOPEC1", 18);
	else			memcpy(&tx_buffer[3], "\x00\xa4\x04\x00\x0d\xa0\x00\x00\x00\x03SINOPEC2", 18);
	tx_len=21;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamFile23Read
*Description	:PSAM��23�ļ���ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamFile23Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM��23�ļ���ȡ����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	memcpy(&tx_buffer[3], "\x00\xb0\x97\x00\x19", 5);
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamGetAPProof
*Description	:PSAM����ȡ��ȫ����״̬(GET ANTI-PLAGIAREZE PROOF)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamGetAPProof(int nozzle, int sam,unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM����ȫ����״̬��ȡ����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	memcpy(&tx_buffer[3], "\x80\xa2\x00\x00\x01", 5);
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamGetRandom
*Description	:PSAM����ȡ�����
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamGetRandom(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM���������ȡ����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	memcpy(&tx_buffer[3], "\x00\x84\x00\x00\x04", 5);
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);
  
	return istate;
}


/*******************************************************************
*Name			:PsamAPAuthen
*Description	:PSAM����ȫ������֤(ANTI-PLAGIAREZE AUTHENTICATION)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:Ciphertext	��ȫ����DES�������ģ��̶��ֳ�8bytes�������0
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamAPAuthen(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM����ȫ������֤����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x0d;
	memcpy(&tx_buffer[3], "\x80\xae\x00\x00\x08", 5);
	memcpy(&tx_buffer[8], Ciphertext, 8);
	tx_len=16;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamLockInit
*Description	:PSAM������ʼ��������MAC1	(INIT_SAM_GREY_LOCK)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:ICLockInitRandom		IC��������ʼ����õ�α�������4bytes
*					:ICLockInitCTC				IC��������ʼ���ѻ�������ţ�2bytes
*					:ICLockInitBalance			IC��������ʼ�������Ŀ���4bytes
*					:time							����ʱ�䣬7bytes
*					:ICLockInitKeysVersion	IC��������ʼ����Կ�汾��
*					:IcLockInitArithmetic		IC��������ʼ���㷨��ʶ
*					:IcAppId						IC��Ӧ�����кź�16λ��8bytes
*					:IcIssuerMark				IC����������ʶ, 8bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamLockInit(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICLockInitRandom, unsigned char *ICLockInitCTC, unsigned char *ICLockInitBalance, unsigned char *time, unsigned char ICLockInitKeysVersion,unsigned char ICLockInitArithmetic, unsigned char *ICAppId, unsigned char *ICIssuerMark)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM��������ʼ��������MAC1����
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x29;
	memcpy(&tx_buffer[3], "\xE0\x40\x00\x00\x24", 5);
	memcpy(&tx_buffer[8], ICLockInitRandom, 4);
	memcpy(&tx_buffer[12], ICLockInitCTC, 2);
	memcpy(&tx_buffer[14], ICLockInitBalance, 4);
	tx_buffer[18]=0x91;
	memcpy(&tx_buffer[19], time, 7);
	tx_buffer[26]=ICLockInitKeysVersion;
	tx_buffer[27]=ICLockInitArithmetic;
	memcpy(&tx_buffer[28], ICAppId, 8);
	memcpy(&tx_buffer[36], ICIssuerMark, 8);
	tx_len=0x2c;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamMAC2Check
*Description	:PSAM��֤MAC2
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:MAC2			MAC2��4bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamMAC2Check(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *MAC2)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����MAC2��֤����
	tx_buffer[0]=sam;			
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x09;
	memcpy(&tx_buffer[3], "\xE0\x42\x00\x00\x04", 5);
	memcpy(&tx_buffer[8], MAC2, 4);
	tx_len=0x0c;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamGMAC
*Description	:PSAM������GMAC
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:ICAppId			IC��Ӧ�����кź�16λ
*					:ICLockInitCTC	�ѻ��������,2bytes
*					:money				���ѽ�HEX
*Output		:buffer				APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamGMAC(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICAppId, unsigned char *ICLockInitCTC, unsigned int money)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM������GMAC����
	tx_buffer[0]=sam;
	tx_buffer[1]=(unsigned char)(20>>8);
	tx_buffer[2]=(unsigned char)(20>>0);
	memcpy(&tx_buffer[3], "\xE0\x44\x00\x00\x0F", 5);
	tx_buffer[8]=0x93;
	memcpy(&tx_buffer[9], ICAppId, 8);
	memcpy(&tx_buffer[17], ICLockInitCTC, 2);
	tx_buffer[19]=money>>24;	tx_buffer[20]=money>>16;	
	tx_buffer[21]=money>>8;	tx_buffer[22]=money>>0;
	tx_len=23;
  	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamGMACRead
*Description	:PSAM����ȡGMAC
*Input			:nozzle				ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes		���������󳤶�
*					:ICPsamTTC		PSAM�ն˽�����ţ�4bytes
*Output		:buffer				APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamGMACRead(int nozzle,int sam, unsigned char *buffer, int maxbytes, const unsigned char *ICPsamTTC)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM������GMAC����
	tx_buffer[0]=sam;
	tx_buffer[1]=(unsigned char)(10>>8);
	tx_buffer[2]=(unsigned char)(10>>0);
	memcpy(&tx_buffer[3], "\xE0\x46\x00\x00\x04", 5);
	memcpy(&tx_buffer[8], ICPsamTTC, 4);
	tx_buffer[12]=0x08;
	tx_len=13;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamTMACInit
*Description	:PSAM��TMAC�����ʼ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*Output		:buffer				APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamTMACInit(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM��TMAC�����ʼ������
	tx_buffer[0]=sam;
	tx_buffer[1]=(unsigned char)(5>>8);
	tx_buffer[2]=(unsigned char)(5>>0);
	memcpy(&tx_buffer[3], "\x80\x1A\x08\x01\x00", 5);
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamTMACInit
*Description	:PSAM��TMAC�����ʼ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:inbuffer		Ҫ���������
*					:len				Ҫ��������ݳ���
*					:follow			�Ƿ��к����ֽڣ�0=�޺����飻1=�к�����
*					:initvalue		���޳�ʼֵ��0=�ޣ�1=��
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamTMACOperat(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *inbuffer, int len, int follow, int initvalue)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//�жϳ���
	if(len>100)	return ERROR;

	//�ж����޺����ֽڵĲ���
	if((0!=follow)&&(1!=follow))	return ERROR;

	//�ж����޳�ʼֵ�Ĳ���
	if((0!=initvalue)&&(1!=initvalue))	return ERROR;

	//����PSAM��TMAC����(DES CRYPT)����
	tx_buffer[0]=sam;
	tx_buffer[1]=(unsigned char)((len+5)>>8);
	tx_buffer[2]=(unsigned char)((len+5)>>0);
	tx_buffer[3]=0x80;
	tx_buffer[4]=0xfa;
	tx_buffer[5]=(initvalue<<2)|(follow<<1)|(1<<0);
	tx_buffer[6]=0;
	tx_buffer[7]=len;											//���ȣ�8�ı���
	memcpy(&tx_buffer[8], inbuffer, len);
	tx_len=len+8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}



/*******************************************************************
*Name			:PsamStartBind
*Description	:PSAM����������ע�Ṧ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:KeyIndex	��֤��Կ������(ACT������)
*					:ICAppId			IC��Ӧ�����кź�16λ��8bytes
*					:Ciphertext	�������ݣ�8bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamStartBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM����������ע�Ṧ������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x15;
	tx_buffer[3]=0x80;
	tx_buffer[4]=0xa6;
	tx_buffer[5]=KeyIndex;
	tx_buffer[6]=0x00;
	tx_buffer[7]=0x10;
	memcpy(&tx_buffer[8], ICAppId, 8);
	memcpy(&tx_buffer[16], Ciphertext, 8);
	tx_len=0x18;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamInitBind
*Description	:PSAM����ʼ������ע�Ṧ��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:KeyIndex		��֤��Կ������(ACT������)
*					:ICAppId		IC��Ӧ�����кź�16λ��8bytes
*					:CoID			���̱��룬8bytes
*					:Ciphertext	�������ݣ�8bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamInitBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *CoID, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM����ʼ������ע�Ṧ������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x1d;
	tx_buffer[3]=0x80;
	tx_buffer[4]=0xac;
	tx_buffer[5]=KeyIndex;
	tx_buffer[6]=0x00;
	tx_buffer[7]=0x18;
	memcpy(&tx_buffer[8], ICAppId, 8);
	memcpy(&tx_buffer[16], CoID, 8);
	memcpy(&tx_buffer[24], Ciphertext, 8);
	tx_len=32;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);
	
	return istate;
}


/*******************************************************************
*Name			:PsamBind
*Description	:PSAM������ע��
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:KeyIndex		��֤��Կ������(ACT������)
*					:mboardID		����ID��8bytes
*					:CoID			���̱��룬8bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char const *mboardID, unsigned char *CoID)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//����PSAM������ע������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x15;
	memcpy(&tx_buffer[3], "\x84\xaa\x00\x00\x10", 5);
	memcpy(&tx_buffer[8], mboardID, 8);
	memcpy(&tx_buffer[16], CoID, 8);
	tx_len=24;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamInitDESCrypt
*Description	:PSAM��ר��DES�����ʼ��(INIT_FOR_DESCRYPT)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:CalKeyIndex	������Կ������( ��־MAC����)
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamInitDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char CalKeyIndex)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM��ר��DES�����ʼ������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x05;
	tx_buffer[3]=0x80;
	tx_buffer[4]=0x1a;
	tx_buffer[5]=0x08;
	tx_buffer[6]=CalKeyIndex;
	tx_buffer[7]=0x00;
	tx_len=8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}


/*******************************************************************
*Name			:PsamDESCrypt
*Description	:PSAM��ר��DES����(DES CRYPT)
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*					:maxbytes	���������󳤶�
*					:time					����������ʱ�䣬7bytes
*					:ACTAppId				ACT�����ţ�10bytes
*					:ACTKeyIndex		ACT��֤��Կ������
*					:RIDAppId				RID�����ţ�10bytes
*					:RIDKeyIndex		RID��֤��Կ������
*					:RIDCalKeyIndex	��־MAC������Կ������
*					:PsamId				PSAM�����ţ�10bytes
*					:mboardID			�ͻ�оƬID��8bytes
*Output		:buffer			APDU����
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int PsamDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardID)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//����PSAM��ר��DES��������
	tx_buffer[0]=sam;
	tx_buffer[1]=0x00;
	tx_buffer[2]=0x35;
	tx_buffer[3]=0x80;tx_buffer[4]=0xfa;tx_buffer[5]=0x05;tx_buffer[6]=0x00;tx_buffer[7]=0x30;
	memcpy(&tx_buffer[8], time, 7);
	memcpy(&tx_buffer[15], ACTAppId, 10);	
	tx_buffer[25]=ACTKeyIndex;
	memcpy(&tx_buffer[26], RIDAppId, 10);	
	tx_buffer[36]=RIDKeyIndex;
	tx_buffer[37]=RIDCalKeyIndex;
	memcpy(&tx_buffer[38], PsamId, 10);
	memcpy(&tx_buffer[48], mboardID, 8);
	tx_len=56;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}

/*******************************************************************
*Name			:IcReaderTestElectrical
*Description	:���뿨�������Բ���
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*					:type		����ģʽ0x30=����ģʽ��0x31=����ģʽ
*Output		:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderTestElectrical(int nozzle, int type)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)	
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����PSAM��λ����
	IcPackSend(param->nozzle, 0x80, type, (unsigned char*)"\x00", 0);
	
	//�ж����ݽ���
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //�����ɹ�
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //����ʧ��
		{

			ICUnlock(param->nozzle);
			return 1;
		}
		else if(timer>=IC_OVERTIME) //������ʱ
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	
		timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcReaderTestProtocol
*Description	:���뿨��Э�����
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*					:type		����ģʽ0x34=����ģʽ��0x35=����ģʽ
*Output		:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderTestProtocol(int nozzle, int type)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����PSAM��λ����
	IcPackSend(param->nozzle, 0x80, type, (unsigned char*)"\x00", 0);

	//�ж����ݽ���
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //�����ɹ�
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //����ʧ��
		{
			ICUnlock(param->nozzle);
			return 1;
		}	
		else if(timer>=IC_OVERTIME) //������ʱ
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	
		timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}

#if 0
/*******************************************************************
*Name			:IcReaderTestRadiofrequency
*Description	:���뿨����Ƶ����
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*Output		:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderTestRadiofrequency(int nozzle)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//��������
	if(0!=ICLock(param->nozzle))
		return ERROR;

	//����PSAM��λ����
	IcPackSend(param->nozzle, 0x42, 0x30, (unsigned char*)"\x00", 0);

	//�ж����ݽ���
	for(timer=0;;)
	{	
		if(1==param->RxValid && 0x42==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //�����ɹ�
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x42==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //����ʧ��
		{
			ICUnlock(param->nozzle);
			return 1;
		}
		else if(timer>=IC_OVERTIME) //������ʱ
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	
		timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}
#endif

/*******************************************************************
*Name			:IcReaderTestProtocolTime
*Description	:����Э������������ѯʱ��
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*					:time		ʱ�䣬��λ0.1��
*Output		:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderTestProtocolTime(int nozzle, int time)
{
	IcStructType *param=NULL;
	unsigned char tx_buffer[16]={0};
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//��������
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//����PSAM��λ����
	tx_buffer[0]=(char)time>>8;	tx_buffer[1]=(char)(time>>0);
	IcPackSend(param->nozzle, 0x80, 0x36, tx_buffer, 2);

	//�ж����ݽ���
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //�����ɹ�
		{
			ICUnlock(param->nozzle);
			return 0;
		}
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //����ʧ��
		{
			ICUnlock(param->nozzle);
			return 1;
		}	
		else if(timer>=IC_OVERTIME) //������ʱ
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	
		timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcReaderTestExit
*Description	:�˳���������ģʽ
*Input			:nozzle		ǹѡ0=A1ǹ��1=B1ǹ
*				:type		����ģʽ0x30=����ģʽ��0x31=����ģʽ
*Output			:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderTestExit(int nozzle)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//����PSAM��λ����
	IcPackSend(param->nozzle, 0x80, 0x81, (unsigned char*)"\x00", 0);
	
	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]){

			ICUnlock(param->nozzle);
			return 0;
		}

		//����ʧ��
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4])
		{
			ICUnlock(param->nozzle);
			return 1;
		}

		//������ʱ
		else if(timer>=IC_OVERTIME){

			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:IcPollLimit
*Description	:��ֹ��ѯ����
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�;����=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcPollLimit(int nozzle)
{
	if(IC_NOZZLE_1==nozzle)			IcStructA1.pollLimit=1;
	else if(IC_NOZZLE_2==nozzle)	IcStructB1.pollLimit=1;
	else												return ERROR;

	return 0;
}


/*******************************************************************
*Name			:IcPollStart
*Description	:�ָ���ѯ����
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�;1=����ʧ��;2=��ʱ;3=nozzle�Ƿ�;4=�������̫С������=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcPollStart(int nozzle)
{
	if(IC_NOZZLE_1==nozzle)				IcStructA1.pollLimit=0;
	else if(IC_NOZZLE_2==nozzle)	IcStructB1.pollLimit=0;
	else													return ERROR;

	return 0;
}



/*******************************************************************
*Name			:IcReaderReset
*Description	:��������λ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output			:None
*Return			:0=�ɹ�;2=��ʱ;����=ʧ��
*History		:2014-10-17,modified by syj
*/

int IcReaderReset(int nozzle)
{
	//del unsigned char tx_buffer[128]={0};
	//del int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;

	//�жϲ�������
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//��������
	if(0!=ICLock(param->nozzle))	return ERROR;

	//���Ͷ�������λ����
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);

	//�ж����ݽ���
	for(timer=0;;)
	{
		//�����ɹ�
		if((1==param->RxValid)&&(0x30==param->RxBuffer[3])&&(0x40==param->RxBuffer[4]))
		{
			ICUnlock(param->nozzle);
			return 0;
		}

		//������ʱ
		else
		if(timer>=IC_OVERTIME)
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//�ȴ���ʱ���ۼƳ�ʱֵ
		usleep(10*1000);	timer+=10;
	}

	//�����������
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICStateRead
*Description	:��״̬��ȡ
*Input			:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
*Output			:state			��״̬����
*Return			:0=�ɹ�;2=��ʱ;����=ʧ��
*History		:2014-10-17,modified by syj
*/

int ICStateRead(int nozzle, IcStateType *state)
{
	if(0==nozzle)	memcpy((char*)state, &IcStructA1.State, sizeof(IcStateType));
	else			memcpy((char*)state, &IcStructB1.State, sizeof(IcStateType));

	return 0;
}


/*******************************************************************
*Name			:IcModuleInit
*Description	:������ģ�鹦�ܳ�ʼ��
*Input			:None
*Output			:None
*Return			:0=�ɹ�;����=ʧ��
*History		:2014-10-17,modified by syj
*/
bool IcModuleInit(void)
{
	IcStructType *param=NULL;
	//����A1��ز����ĳ�ʼ��
	int nInitRet = pthread_mutex_init(&semIdA1,NULL);	//�����������ź�������ʼ��Ϊ��Ч
    if(nInitRet != 0)
	{
       printf("Error! Create List 'semIdA1' failed!\n");
	   return false;
	}

	//��ʼ������
	param=&IcStructA1;
	param->nozzle=IC_NOZZLE_1;	
	param->pollLimit=0;

	//��λ����
   IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);
   ICShoot(0); //fj:20171121

 //  printf("A wait 3 second \n");
 //  while(1)
 //  {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
//	   struct timeval tv;
//	   tv.tv_sec = 3;
//	   tv.tv_usec = 0;
//	   select(0,NULL,NULL,NULL,&tv);
//	   break;
//   }



	//����B1��ز�����ʼ��
	nInitRet = pthread_mutex_init(&semIdB1,NULL);
	if(nInitRet != 0)
	{
		printf("Error! Create List 'semIdB1' failed!\n");
		return false;
	}

	//��ʼ������
	param=&IcStructB1;
	param->nozzle=IC_NOZZLE_2;	
	param->pollLimit=0;

	//��λ����
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);
	ICShoot(1); //20171121
    
//   printf("B  wait 3 second \n");
//   while(1)
//   {
	   //PrintTime("main start --sec=%d,  ","millsec=%d\n");
//	   struct timeval tv;
//	   tv.tv_sec = 3;
//	   tv.tv_usec = 0;
//	   select(0,NULL,NULL,NULL,&tv);
//	   break;
//   }


	return true;
}

void ResetIcPack(int nozzle)
{
    IcPackSend(nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);
}

/*
int IcModuleInit(void)
{
	//del unsigned char bffer[128]={0};
	IcStructType *param=NULL;


	//***************************************************************************
	//����A1����������ȵĳ�ʼ��
	

	//�����������ź�������ʼ��Ϊ��Ч
	//del semIdA1=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==semIdA1)	printf("Error! Create List 'semIdA1' failed!\n");
	pthread_mutex_init(&semIdA1, NULL);

	//��ʼ������
	param=&IcStructA1;
	param->nozzle=IC_NOZZLE_1;	param->pollLimit=0;

	//�����������ݽ�������
	//del param->tIdReceive=taskSpawn("tIcRxA1", 151, 0, 0x2000, (FUNCPTR)tICReceive, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tIcRxA1' failed!\n");
	pthread_t pIcRxA1;
	param->tIdReceive=pthread_create(&pIcRxA1, NULL, (void*)tICReceive, NULL);
	if(0!=param->tIdReceive) printf("Error!	Creat task 'tIcRxA1' failed!\n");
	pthread_detach(pIcRxA1);

	//���������ź���
	//param->semIdShoot=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdShoot)	printf("Error! Create List 'IcStructA1.ssemIdShoot' failed!\n");

	//������������
	//param->tIdShoot=taskSpawn("tIcShootA1", 156, 0, 0x2000, (FUNCPTR)tICShoot, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdShoot))	printf("Error!	Creat task 'tIcShootA1' failed!\n");

	//������״̬��ѯ����
	//del param->tIdPoll=taskSpawn("tIcPollA1", 156, 0, 0x2000, (FUNCPTR)tICPoll, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdPoll))	printf("Error!	Creat task 'tIcPollA1' failed!\n");
	pthread_t pIcPollA1;
	param->tIdPoll=pthread_create(&pIcPollA1, NULL, (void*)tICPoll, NULL);
	if(0!=param->tIdPoll) printf("Error!	Creat task 'tIcPollA1' failed!\n");
	pthread_detach(pIcPollA1);


	//��λ����
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);




	//***************************************************************************
	//����B1����������ȵĳ�ʼ��
	

	//�����������ź�������ʼ��Ϊ��Ч
	//del semIdB1=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==semIdB1)	printf("Error! Create List 'semIdB1' failed!\n");
	pthread_mutex_init(&semIdB1, NULL);

	//��ʼ������
	param=&IcStructB1;
	param->nozzle=IC_NOZZLE_2;	param->pollLimit=0;

	//�����������ݽ�������
	//del param->tIdReceive=taskSpawn("tIcRxB1", 151, 0, 0x2000, (FUNCPTR)tICReceive, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tIcRxB1' failed!\n");
	pthread_t pIdReceive;
	param->tIdReceive=pthread_create(&pIdReceive, NULL, (void*)tICReceive, NULL);
	if(0!=param->tIdReceive) printf("Error!	Creat task 'tIcRxB1' failed!\n");
	pthread_detach(pIdReceive);

	//���������ź���
	//param->semIdShoot=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdShoot)	printf("Error! Create List 'IcStructA1.ssemIdShoot' failed!\n");

	//������������
	//param->tIdShoot=taskSpawn("tIcShootA1", 156, 0, 0x2000, (FUNCPTR)tICShoot, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdShoot))	printf("Error!	Creat task 'tIcShootA1' failed!\n");

	//������״̬��ѯ����
	//del param->tIdPoll=taskSpawn("tIcPollB1", 156, 0, 0x2000, (FUNCPTR)tICPoll, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdPoll))	printf("Error!	Creat task 'tIcPollB1' failed!\n");
	pthread_t pIcPollB1;
	param->tIdPoll=pthread_create(&pIcPollB1, NULL, (void*)tICPoll, NULL);
	if(0!=param->tIdPoll) printf("Error!	Creat task 'tIcPollB1' failed!\n");
	pthread_detach(pIcPollB1);

	//��λ����
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);

	return 0;
}*/


