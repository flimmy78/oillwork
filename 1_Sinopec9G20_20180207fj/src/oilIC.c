#include "../inc/main.h"

//卡操作信号量
static pthread_mutex_t semIdA1;
static pthread_mutex_t semIdB1;

//卡操作数据
static IcStructType IcStructA1;
static IcStructType IcStructB1;

/*******************************************************************
*Name				:IcPackSend
*Description		:封装发送IC卡座数据
*Input				:nozzle				枪选0=A1枪；1=B1枪
*					:cmd					命令字
*					:cmd_param		命令参数
*					:data				数据包
*					:data_len			数据包长度
*Output				:None
*Return				:实际发送长度
*History			:2013-07-01,modified by syj
*/

static int IcPackSend(unsigned char nozzle, unsigned char cmd, unsigned char cmd_param, const unsigned char *data, int data_len)
{
	unsigned char tx_buffer[256]={0};
	int tx_len=0, i=0;

	//del unsigned char dspbuffer[512]={0}; //fj:用于打印的

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
		memcpy(tx_buffer+tx_len,IptParamA.EtcSelCardInf+16,4);//2017.07.25增加4字节的MAC号
		tx_len+=4;
		tx_buffer[tx_len++]=1;/*1字节的COS命令数量*/
		tx_buffer[tx_len++]=data_len;/*1字节的APDU命令长度*/
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
		memcpy(tx_buffer+tx_len,IptParamB.EtcSelCardInf+16,4);//2017.07.25增加4字节的MAC号
		tx_len+=4;
		tx_buffer[tx_len++]=1;/*1字节的COS命令数量*/
		tx_buffer[tx_len++]=data_len;/*1字节的APDU命令长度*/
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
*Description	:卡操作锁
*Input			:nozzle	枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功；其它=失败
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
*Description	:卡操作解除锁
*Input			:nozzle	枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功；其它=失败
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
*Description	:IC卡座数据读取及分析任务
*Input			:nozzle	面板号0=A1枪，1=B1枪
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
	if(IC_NOZZLE_1==nozzle)//判断面板
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
		if(0!=param->RxValid)//当前状态不允许接收
		{
			//printf("RxValid = %d\n",param->RxValid);
			//usleep(1000);
			continue;
		}	
		read_len=comRead(com_fd, (char*)&data, 1);//读取数据
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
				
		if(param->RxLen>=ICDATA_LEN)//防止长度溢出
		{
			param->RxLen=0;	
			param->RxValid=0;
		}
	
		param->RxBuffer[param->RxLen]=data;	//缓存并解析数据
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

		//据达到指定长度时进行校验
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
*Description	:IC卡座状态轮询
*Input			:nozzle	枪选0=A1枪，1=B1枪
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
	
	if(IC_NOZZLE_1==nozzle)	//判断枪选
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
		while(0!=ICLock(param->nozzle))//卡操作锁
		{
			usleep(1000);
		}

		//printf("param->shootAsk = %d\n",param->shootAsk);

		//有弹卡操作时优先执行弹卡操作
		if(1==param->shootAsk)
		{
			//发送弹卡指令
			IcPackSend(param->nozzle, 0x32, 0x40, (unsigned char*)"\x00", 0);

			//printf("222222222222\n");

			//判断数据接收
			for(timer=0;;)
			{
				if( 1==param->RxValid && 0x32==param->RxBuffer[3] && 0x40==param->RxBuffer[4] && 0x30==param->RxBuffer[5]) //操作成功
				{
					param->State.DeckStateS1=0;
					param->State.IcTypeS2=0;
					param->State.IcStateS3=0;
					param->State.PowerStateS4=0;

					param->shootAsk=0;
					break;
				}
				else if(1==param->RxValid && 0x32==param->RxBuffer[3] && 0x40==param->RxBuffer[4] && 0x31==param->RxBuffer[5]) //操作失败
				{
					param->shootAsk=0;
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes<3) //作超时，不足三次，再次弹卡
				{
	                printf("IC Shoot -- ic timeout : < 3\n");
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes>=3) //操作超时，超过三次，放弃操作
				{
	                printf("IC Shoot -- ic timeout : >= 3\n");
					overtimes=0;	
					param->shootAsk=0;
					break;
				}

				//等待延时并累计超时值
				usleep(10*1000);	
				timer+=10;
			}
		}
		else if(0==param->pollLimit && iptparam->EtcOilFlg == 0)
		{
			//发送寻卡指令
			IcPackSend(param->nozzle, 0x31, 0x41, (unsigned char*)"\x00", 0);

			//printf("1111111111111\n");

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if(1==param->RxValid && 0x31==param->RxBuffer[3] && 0x41==param->RxBuffer[4])
				{
					//printf("shootAsk = %d\n",param->shootAsk);
					//有弹卡操作时卡的状态暂时无效
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
				else if(timer>=IC_OVERTIME && overtimes<3)	//操作超时
				{
					printf("ic timeout : < 3\n");
					overtimes++;
					break;
				}
				else if(timer>=IC_OVERTIME && overtimes>=3)	//三次超时认为无卡
				{
					printf("ic timeout : >= 3\n");
					param->State.DeckStateS1=0;
					param->State.IcTypeS2=0;
					param->State.IcStateS3=0;
					param->State.PowerStateS4=0;
					overtimes++;
					break;
				}

				//等待延时并累计超时值
				usleep(10*1000);
				//select(0,NULL,NULL,NULL,&tv);
				timer+=10;
			}
		}

		//解除卡操作锁
		ICUnlock(param->nozzle);

		//延时约500ms进行下次通讯
		usleep(500000);
	}

	return;
}


/*******************************************************************
*Name			:tICReceive
*Description	:IC卡座数据读取及分析任务
*Input			:nozzle	枪选0=A1枪，1=B1枪
*Output		:None
*Return			:None
*History		:2014-10-17,modified by syj
*/

static void tICShoot(int nozzle)
{
	IcStructType *param=NULL;
	int timer=0, overtimes=0;

	//判断枪选
	if(0==nozzle)	
		param=&IcStructA1;
	else			
		param=&IcStructB1;

	while ( 1 )
	{	
		while(0!=ICLock(param->nozzle))//卡操作锁
		{
			//usleep(1000);
			continue;  //fj:20171117_add
		}	
		overtimes=0;	//超时次数清零	
		IcPackSend(param->nozzle, 0x32, 0x40, (unsigned char*)"\x00", 0);//发送弹卡指令	
		for(timer=0;;)//判断数据接收
		{
			//操作成功
			if((1==param->RxValid)&&(0x32==param->RxBuffer[3])&&(0x40==param->RxBuffer[4])&&(0x30==param->RxBuffer[5]))
			{
				param->State.DeckStateS1=0;
				param->State.IcTypeS2=0;
				param->State.IcStateS3=0;
				param->State.PowerStateS4=0;
				break;
			}	
			else if((1==param->RxValid)&&(0x32==param->RxBuffer[3])&&(0x40==param->RxBuffer[4])&&(0x31==param->RxBuffer[5]))//操作失败
			{
				break;
			}
            else if((timer>=IC_OVERTIME)&&(overtimes<3)) 	//操作超时，不足三次，再次弹卡
			{	
				break;
			}
			else if((timer>=IC_OVERTIME)&&(overtimes>=3))//操作超时，超过三次，放弃操作
			{
				break;
			}

			//等待延时并累计超时值
			usleep(10*1000);	
			timer+=10;
		}
		//解除卡操作锁
		ICUnlock(param->nozzle);
		//usleep(1000);
	}

	return;
}


/*******************************************************************
*Name			:ICShoot
*Description	:IC卡弹卡
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功;其他=失败
*History		:2014-10-17,modified by syj
*/

int ICShoot(int nozzle)
{
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))	//联迪POS
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
*Description	:IC卡复位
*Input			:nozzle		枪选0=A1枪，1=B1枪
*					:sam			0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*Output		:None
*Return		:0=操作成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
	if(0==nozzle)			
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else							
		return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		istate = ldICPowerUp(nozzle, LD_ICDEV_USERCARD);
		if(0 == istate)	
			return 0;
		else					
			return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//根据select选择处理外插入卡或SIM卡槽内置卡
	if(0==sam)
	{
		//发送卡复位命令
		if(IC_CARD_CPU==param->State.IcTypeS2)				
			IcPackSend(param->nozzle, 0x37, 0x40, (unsigned char*)"\x00", 0);
		else if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	
			IcPackSend(param->nozzle, 0x35, 0x40, (unsigned char*)"\x00", 0);
		else if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	
			IcPackSend(param->nozzle, 0x34, 0x40, (unsigned char*)"\x00", 0);

		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
			if(1==param->RxValid &&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x40==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5] || 'Z'==param->RxBuffer[5]))
			{
				ICUnlock(param->nozzle);
				return 0;
			}

			//操作失败
			else
			if(1==param->RxValid &&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x40==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x40==param->RxBuffer[4])))
			{	 
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	
			timer+=10;
		}

	}
	else
	{
		//发送复位命令
		tx_buffer[0]=sam;
		tx_len=1;
		IcPackSend(param->nozzle, 0x3d, 0x41, tx_buffer, tx_len);
		
		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(sam==param->RxBuffer[5])&&\
				(('Y'==param->RxBuffer[6])||('Z'==param->RxBuffer[6])))
			{
				ICUnlock(param->nozzle);
				return 0;
			}
            else if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x33==param->RxBuffer[5]))
			{
				ICUnlock(param->nozzle);//操作失败
				return 1;
			}	
			else if(timer>=IC_OVERTIME)//操作超时
			{	
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	
			timer+=10;
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICMFSelect
*Description	:IC卡MF选择
*Input			:nozzle			枪选0=A1枪，1=B1枪
*				:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*				:maxbytes	输出缓存最大长度
*Output			:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
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

	if(0!=ICLock(param->nozzle))//卡操作锁
		return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送卡MF选择命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x13;
		memcpy(&tx_buffer[2], "\x00\xa4\x04\x00", 4);
		tx_buffer[6]=14;
		memcpy(&tx_buffer[7], "1PAY.SYS.DDF01", 14);
		tx_len=21;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)	//根据select选择处理外插入卡或SIM卡槽内置卡
		{
			//发送卡MF选择命令
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

			for(timer=0;;)//判断数据接收
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5]))
				{	
					//判断APDU长度
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
					ICUnlock(param->nozzle);	//操作失败
					return 1;
				}
				else if(timer>=IC_OVERTIME)//操作超时
				{
					ICUnlock(param->nozzle);
					return 2;
				}
				//等待延时并累计超时值
				usleep(10*1000);	
				timer+=10;
			}
		}
		else
		{
			//发送MF选择命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x13;
			memcpy(&tx_buffer[3], "\x00\xa4\x04\x00", 4);
			tx_buffer[7]=14;
			memcpy(&tx_buffer[8], "1PAY.SYS.DDF01", 14);
			tx_len=22;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);	
			for(timer=0;;)//判断数据接收
			{
				//操作成功
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
						('N'==param->RxBuffer[6]))//操作失败
				{
					ICUnlock(param->nozzle);
					return 1;
				}	
				else if(timer>=IC_OVERTIME)//操作超时
				{
					ICUnlock(param->nozzle);
					return 2;
				}

				//等待延时并累计超时值
				usleep(10*1000);	
				timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICAppSelect
*Description	:IC卡应用选择
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:app				0=电子油票；1=积分应用
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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
	
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))//联迪POS
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

	if(0!=ICLock(param->nozzle)) //卡操作锁
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
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)	//根据select选择处理外插入卡或SIM卡槽内置卡
		{
			//发送卡应用选择命令
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

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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
				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}
				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//发送卡应用选择命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x11;
			memcpy(&tx_buffer[3], "\x00\xa4\x04\x00", 4);
			tx_buffer[7]=12;
			if(0==app)	memcpy(&tx_buffer[8], "\xa0\x00\x00\x00\x03SINOPEC", 12);
			else				memcpy(&tx_buffer[8], "\xa0\x00\x00\x00\x03LOYALTY", 12);
			tx_len=20;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile21Read
*Description	:IC卡21文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x95\x00\x1E", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送21文件读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x95\x00\x1E", 5);
		tx_len=7;				
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);			
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送21文件读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x95\x00\x1E", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//发送21文件读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x95\x00\x1E", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile22Read
*Description	:IC卡22文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x96\x00\x29", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送22文件读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x96\x00\x29", 5);
		tx_len=7;			
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//发送22文件读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x96\x00\x29", 5);
			tx_len=7;			
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//发送22文件读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x96\x00\x29", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile27Read
*Description	:IC卡27文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9B\x00\x20", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(iptparam->EtcOilFlg==1)
	{
		/*发送27文件读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9B\x00\x20", 5);
		tx_len=7;			
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//发送27文件读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x9B\x00\x20", 5);
			tx_len=7;					
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}

		}
		else
		{
			//发送27文件读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x9B\x00\x04", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile28Read
*Description	:IC卡28文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9C\x00\x42", 5);
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送28文件读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9C\x00\x42", 5);
		tx_len=7;		
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送28文件读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x00\xB0\x9C\x00\x42", 5);
			tx_len=7;			
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;

			}
		}
		else
		{
			//发送28文件读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x00\xB0\x9C\x00\x42", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICNotesRead
*Description	:IC卡交易明细读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:notes_id		交易明细条目
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		tx_buffer[tx_len++] = 0x00;	tx_buffer[tx_len++] = 0xb2;
		tx_buffer[tx_len++] = notes_id;	
		tx_buffer[tx_len++] = 0xc4;	tx_buffer[tx_len++] = 0x17;

		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送交易明细读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		tx_buffer[2]=0x00;	tx_buffer[3]=0xb2;	tx_buffer[4]=notes_id;
		tx_buffer[5]=0xc4;	tx_buffer[6]=0x17;
		tx_len=7;	
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送交易明细读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			tx_buffer[2]=0x00;	tx_buffer[3]=0xb2;	tx_buffer[4]=notes_id;
			tx_buffer[5]=0xc4;	tx_buffer[6]=0x17;
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;

			}
		}
		else
		{
			//发送交易明细读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			tx_buffer[3]=0x00;	tx_buffer[4]=0xb2;	tx_buffer[5]=notes_id;
			tx_buffer[6]=0xc4;	tx_buffer[7]=0x17;
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICPinCheck
*Description	:IC卡密码校验
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:pin				密码ASCII
*					:pin_len			密码位数0~12
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
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

	//判断密码长度
	if(0==pin_len || pin_len>12)	return ERROR;

	//密码解析为压缩BCD格式，不足后补F
	if(0==(pin_len%2))	pass_len=pin_len/2;
	else							pass_len=pin_len/2+1;
	for(i=0; i<pin_len; i++)
	{
		if(0==(i%2))	pass[i/2]=pass[i/2]|((pin[i]&0x0f)<<4);
		else					pass[i/2]=pass[i/2]|((pin[i]&0x0f)<<0);

		if((i+1)>=pin_len && 0==(i%2))	pass[i/2]=pass[i/2]|0x0f;
	}

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\x20\x00\x00", 4);	tx_len += 4;
		tx_buffer[tx_len++] = pass_len;
		memcpy(tx_buffer + tx_len, pass, pass_len);				tx_len += pass_len;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送卡密码验证命令*/
		tx_buffer[0]=(unsigned char)((5+pass_len)>>8);
		tx_buffer[1]=(unsigned char)((5+pass_len)>>0);
		memcpy(&tx_buffer[2], "\x00\x20\x00\x00", 4);
		tx_buffer[6]=pass_len;
		memcpy(&tx_buffer[7], pass, pass_len);
		tx_len=7+pass_len;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//发送卡密码验证命令
			tx_buffer[0]=(unsigned char)((5+pass_len)>>8);
			tx_buffer[1]=(unsigned char)((5+pass_len)>>0);
			memcpy(&tx_buffer[2], "\x00\x20\x00\x00", 4);
			tx_buffer[6]=pass_len;
			memcpy(&tx_buffer[7], pass, pass_len);
			tx_len=7+pass_len;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送卡密码验证命令
			tx_buffer[0]=sam;
			tx_buffer[1]=(unsigned char)((5+pass_len)>>8);
			tx_buffer[2]=(unsigned char)((5+pass_len)>>0);
			memcpy(&tx_buffer[3], "\x00\x20\x00\x00", 4);
			tx_buffer[7]=pass_len;
			memcpy(&tx_buffer[8], pass, pass_len);
			tx_len=8+pass_len;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyInfoRead
*Description	:IC卡灰锁信息读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\xCA\x00\x00\x1E", 5);	
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送灰锁信息读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\xE0\xCA\x00\x00\x1E", 5);
		tx_len=0x07;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送灰锁信息读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\xE0\xCA\x00\x00\x1E", 5);
			tx_len=0x07;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送灰锁信息读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\xE0\xCA\x00\x00\x1E", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICBalanceRead
*Description	:IC卡余额读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x80\x5C\x00\x01\x04", 5);	
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送余额读取命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x80\x5C\x00\x01\x04", 5);
		tx_len=7;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
	        usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//发送余额读取命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\x80\x5C\x00\x01\x04", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送余额读取命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\x80\x5C\x00\x01\x04", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICLockInit
*Description	:IC卡灰锁初始化
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:keyIndex		PSAM消费密钥索引号
*					:termId			PSAM终端机编号，6bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\x7A\x08\x01\x07", 5);		tx_len += 5;
		tx_buffer[tx_len++] = keyIndex;
		memcpy(tx_buffer + tx_len, termId, 6);									tx_len += 6;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送灰锁初始化命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x0c;
		memcpy(&tx_buffer[2], "\xE0\x7A\x08\x01\x07", 5);
		tx_buffer[7]=keyIndex;
		memcpy(&tx_buffer[8], termId, 6);
		tx_len=14;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送灰锁初始化命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x0c;
			memcpy(&tx_buffer[2], "\xE0\x7A\x08\x01\x07", 5);
			tx_buffer[7]=keyIndex;
			memcpy(&tx_buffer[8], termId, 6);
			tx_len=14;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送灰锁初始化命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x0c;
			memcpy(&tx_buffer[3], "\xE0\x7A\x08\x01\x07", 5);
			tx_buffer[8]=keyIndex;
			memcpy(&tx_buffer[9], termId, 6);
			tx_len=15;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyLock
*Description	:IC卡灰锁
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:psamTTC				PSAM终端交易序号，4bytes
*					:psamRandom		PSAM终端随机数，4bytes
*					:time					消费时间，7bytes
*					:psamMAC1			PSAM计算的MAC1，4bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
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

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送灰锁命令*/
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
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);
            usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送灰锁命令
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

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送灰锁命令
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

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}


	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICGreyUnlock
*Description	:IC卡解锁
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:money				消费金额，HEX
*					:ICLockInitCTC	脱机交易序号，2bytes
*					:PsamTermId	PSAM终端机编号，6bytes
*					:ICPsamTTC		PSAM终端交易序号，4bytes
*					:time				消费时间，7bytes
*					:ICPsamGMAC	GMAC，4bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
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

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

    //szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送解锁命令*/
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
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);
			timer+=10;
		}
	}
	else
	{
		if(0==sam)
		{
			//发送解锁命令
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

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){				
					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
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
			//发送解锁命令
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

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}	
					else if(timer>=IC_OVERTIME)//操作超时
					{
						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值	
				usleep(10*1000);	
				timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICTacClr
*Description	:IC卡清除TAC
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	/*判断操作对象*/
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

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\xE0\xCA\x01\x00\x00", 5);	tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//szb_fj_20171120,add
	if(iptparam->EtcOilFlg==1)
	{
		/*发送清除TAC命令*/
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\xE0\xCA\x01\x00\x00", 5);
		tx_len=7;
		IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer+2, tx_len-2);
		for(timer=0;;)
		{
			/*操作成功*/
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

			/*操作超时*/
			else if(timer>=IC_OVERTIME){

				ICUnlock(param->nozzle);
				return 2;
			}

			/*等待延时并累计超时值*/
			//taskDelay(10*ONE_MILLI_SECOND);	
			usleep(10000);
			timer+=10;
		}
	}
	else
	{

		if(0==sam)
		{
			//发送清除TAC命令
			tx_buffer[0]=0x00;
			tx_buffer[1]=0x05;
			memcpy(&tx_buffer[2], "\xE0\xCA\x01\x00\x00", 5);
			tx_len=7;
			if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
			else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
				if((1==param->RxValid)&&\
						((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
						 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
						('Y'==param->RxBuffer[5])){

					//判断APDU长度
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

				//操作失败
				else
					if((1==param->RxValid)&&\
							((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
							 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){

						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else if(timer>=IC_OVERTIME){

						ICUnlock(param->nozzle);
						return 2;
					}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
		else
		{
			//发送清除TAC命令
			tx_buffer[0]=sam;
			tx_buffer[1]=0x00;
			tx_buffer[2]=0x05;
			memcpy(&tx_buffer[3], "\xE0\xCA\x01\x00\x00", 5);
			tx_len=8;
			IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

			//判断数据接收
			for(timer=0;;)
			{
				//操作成功
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

				//操作失败
				else
					if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
							('N'==param->RxBuffer[6]))
					{
						ICUnlock(param->nozzle);
						return 1;
					}

				//操作超时
					else
						if(timer>=IC_OVERTIME)
						{
							ICUnlock(param->nozzle);
							return 2;
						}

				//等待延时并累计超时值
				usleep(10*1000);	timer+=10;
			}
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICFile26Read
*Description	:IC卡26文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICFile26Read(int nozzle, int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x9a\x00\x02", 5);	tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//发送26文件读取命令
		tx_buffer[0]=0x00;
		tx_buffer[1]=0x05;
		memcpy(&tx_buffer[2], "\x00\xB0\x9a\x00\x02", 5);
		tx_len=7;
		if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
		else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);


		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//判断APDU长度
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

			//操作失败
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//发送26文件读取命令
		tx_buffer[0]=sam;
		tx_buffer[1]=0x00;
		tx_buffer[2]=0x05;
		memcpy(&tx_buffer[3], "\x00\xB0\x9a\x00\x02", 5);
		tx_len=8;
		IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
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

			//操作失败
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICDESCrypt
*Description	:IC卡专用DES计算(DES CRYPT)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:KeyIndex			认证密钥索引号(ACT索引号)
*					:PSAMRandom	PSAM随机数，4bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICDESCrypt(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *PSAMRandom)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
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

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//发送专用DES计算命令
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


		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//判断APDU长度
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

			//操作失败
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//发送专用DES计算命令
		tx_buffer[0]=sam;
		tx_buffer[1]=0x00;
		tx_buffer[2]=0x0d;
		tx_buffer[3]=0x80;	tx_buffer[4]=0xa8;	tx_buffer[5]=0x00;
		tx_buffer[6]=KeyIndex;	tx_buffer[7]=0x08;
		memcpy(&tx_buffer[8], PSAMRandom, 4);	
		memcpy(&tx_buffer[12], "\x00\x00\x00\x00", 4);
		tx_len=16;
		IcPackSend(param->nozzle, 0x3d, 0x43, tx_buffer, tx_len);

		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
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

			//操作失败
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcAppendLog
*Description	:PSAM卡添加日志记录(APPEND LOG RECORD)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:sam				0=外插入卡，0x30~0x33'表示内置卡座号'0'~'3'
*					:maxbytes	输出缓存最大长度
*					:time					启动绑定日期时间，7bytes
*					:ACTAppId				ACT卡卡号，10bytes
*					:ACTKeyIndex		ACT认证密钥索引号
*					:RIDAppId				RID卡卡号，10bytes
*					:RIDKeyIndex		RID认证密钥索引号
*					:RIDCalKeyIndex	日志MAC计算密钥索引号
*					:PsamId				PSAM卡卡号，10bytes
*					:mboardID			油机芯片ID，8bytes
*					:RIDMAC				RID认证MAC，4bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICAppendLog(int nozzle, int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardID, unsigned char *RIDMAC)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
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

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	if(0==sam)
	{
		//发送卡添加日志记录命令
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


		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
				('Y'==param->RxBuffer[5])){
				
				//判断APDU长度
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

			//操作失败
			else
			if((1==param->RxValid)&&\
				((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
				 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else if(timer>=IC_OVERTIME){
				
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}
	else
	{
		//发送卡添加日志记录命令
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

		//判断数据接收
		for(timer=0;;)
		{
			//操作成功
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

			//操作失败
			else
			if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x33==param->RxBuffer[5])&&\
				('N'==param->RxBuffer[6]))
			{
				ICUnlock(param->nozzle);
				return 1;
			}

			//操作超时
			else
			if(timer>=IC_OVERTIME)
			{
				ICUnlock(param->nozzle);
				return 2;
			}

			//等待延时并累计超时值
			usleep(10*1000);	timer+=10;
		}
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:ICKeyADFSelect
*Description	:IC密钥卡应用选择
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyADFSelect(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)	
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else	
		return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
	    printf("ICKeyADFSelect  pos bbbbbbbb\n");

		memcpy(tx_buffer + tx_len, "\x00\xa4\x00\x00\x02\x10\x01", 7);		
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//发送卡应用选择命令
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

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5]))
		{		
			//判断APDU长度
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

		//操作失败
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

		//操作超时
		else if(timer>=IC_OVERTIME)
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}
		
	//解除卡操作锁
	ICUnlock(param->nozzle);


	return 0;
}


/*******************************************************************
*Name			:ICKeyEF01Select
*Description	:IC密钥卡选择EF 01文件
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Select(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xA4\x00\x00\x02\xEF\x01", 7);		
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送选择EF 01文件命令
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	memcpy(&tx_buffer[2], "\x00\xA4\x00\x00\x02\xEF\x01", 7);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//判断APDU长度
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

		//操作失败
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}
	
	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:ICKeyEF01Read
*Description	:IC密钥卡读取EF 01文件
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Read(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xB0\x00\x00\x20", 5);		
		tx_len += 5;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送读取EF 01文件命令
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x05;
	memcpy(&tx_buffer[2], "\x00\xB0\x00\x00\x20", 5);
	tx_len=7;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//判断APDU长度
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

		//操作失败
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}
	
	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF01Write
*Description	:IC密钥卡未下载条数修改
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:number		未下载条数
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyEF01Write(int nozzle, unsigned char *buffer, int maxbytes, unsigned int number)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xd6\x00\x02\x02", 5);	tx_len += 5;
		tx_buffer[tx_len++] = (char)(number >> 8);
		tx_buffer[tx_len++] = (char)(number >> 0);
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送写EF 01文件命令
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	tx_buffer[2]=0x00;	tx_buffer[3]=0xd6;tx_buffer[4]=0x00;tx_buffer[5]=0x02;tx_buffer[6]=0x02;
	tx_buffer[7]=(char)(number>>8);
	tx_buffer[8]=(char)(number>>0);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//判断APDU长度
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

		//操作失败
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF02Select
*Description	:IC密钥卡选择EF 02文件
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyEF02Select(int nozzle, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		memcpy(tx_buffer + tx_len, "\x00\xA4\x00\x00\x02\xEF\x02", 7);	
		tx_len += 7;
		
		istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);
		if(0 == istate)	return 0;
		else					return ERROR;
	}

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送选择EF 02文件命令
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x07;
	memcpy(&tx_buffer[2], "\x00\xA4\x00\x00\x02\xEF\x02", 7);
	tx_len=0x09;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//判断APDU长度
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

		//操作失败
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}
	
	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICKeyEF02Read
*Description	:IC密钥卡写EF 01文件
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:offset			读取起始位置偏移
*					:readbytes	要读取的数据长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int ICKeyEF02Read(int nozzle, unsigned char *buffer, int maxbytes, unsigned int offset, int readbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;
	int istate = 0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
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

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送读EF 02文件命令
	tx_buffer[0]=0x00;
	tx_buffer[1]=0x05;
	tx_buffer[2]=0x00;	tx_buffer[3]=0xb0;
	tx_buffer[4]=(char)(offset>>8);tx_buffer[5]=(char)(offset>>0);
	tx_buffer[6]=readbytes;
	tx_len=7;
	if(IC_CARD_CPU==param->State.IcTypeS2)				IcPackSend(param->nozzle, 0x37, 0x43, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEB==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x35, 0x41, tx_buffer, tx_len);
	else	if(IC_CARD_RF_TYPEA==param->State.IcTypeS2)	IcPackSend(param->nozzle, 0x34, 0x41, tx_buffer, tx_len);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))&&\
			('Y'==param->RxBuffer[5])){
				
			//判断APDU长度
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

		//操作失败
		else
		if((1==param->RxValid)&&\
			((IC_CARD_CPU==param->State.IcTypeS2 && 0x37==param->RxBuffer[3] && 0x43==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEB==param->State.IcTypeS2 && 0x35==param->RxBuffer[3] && 0x41==param->RxBuffer[4])||\
			 (IC_CARD_RF_TYPEA==param->State.IcTypeS2 && 0x34==param->RxBuffer[3] && 0x41==param->RxBuffer[4]))){
				 
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){
				
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}






/*******************************************************************
*Name				:IcPsamApduSend
*Description		:封装PSAM卡APDU数据
*Input				:nozzle				IC_NOZZLE_1=1号卡设备；IC_NOZZLE_2=2号卡设备；
*					:sam			    判断数据是发到卡座还是键盘 (>=0x30卡座，<0x30键盘)
*					:inbuffer			apdu数据
*					:inbytes			apdu数据长度
*					:maxbytes			输出数据缓存长度
*Output				:outbuffer			输出数据缓存
*Return				:0=成功；1=失败；2=超时；其它=错误
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
 	 //判断操作对象
	if(0==nozzle)			
		param=&IcStructA1;
	else if(1==nozzle)	
		param=&IcStructB1;
	else							
		return 3;


  	if(sam>='0')
	{
    //卡操作锁
  	if(0!=ICLock(param->nozzle))	
		return ERROR;

    IcPackSend(param->nozzle, 0x3d, 0x43, inbuffer, inbytes);

  	//判断数据接收
  	for(timer=0;;)
  	{
  		//操作成功
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

  		//操作失败
  		else
  		if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x43==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])&&\
  			('N'==param->RxBuffer[6]))
  		{
  			ICUnlock(param->nozzle);
  			return 1;
  		}

  		//操作超时
  		else if(timer>=IC_OVERTIME)
  		{
  			ICUnlock(param->nozzle);
  			return 2;
  		}

  		//等待延时并累计超时值
  		usleep(10*1000);	timer+=10;
  	}

  	//解除卡操作锁
  	ICUnlock(param->nozzle);

  }
  else
  {
     //卡操作锁
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
	//数据输出
    //if(i_outbytes>9)	memcpy(outbuffer, &i_outbuffer[7], i_outbytes-9);

    //解除卡操作锁
  	ICUnlock(param->nozzle);

	return istate;
  }

	return 0;
}



/*******************************************************************
*Name			:PsamReset
*Description	:PSAM卡复位
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output		:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//联迪POS
	if(TD_DEVICE_LIANDI == tdDeviceGet(nozzle))
	{
		//istate = ldICExchangAPDU(nozzle, LD_ICDEV_USERCARD, (char*)tx_buffer, tx_len, (char*)buffer, maxbytes);;
	}

	if(sam>='0')
	{
	  	//卡操作锁
	  	if(0!=ICLock(param->nozzle))	return ERROR;

	  	//发送PSAM复位命令    
	  	IcPackSend(param->nozzle, 0x3d, 0x41, &sambuffer, 1);
	  	
	  	//判断数据接收
	  	for(timer=0;;)
	  	{
	  		if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])&&\
	  			(('Y'==param->RxBuffer[6])||('Z'==param->RxBuffer[6]))) //操作成功
	  		{
	  			ICUnlock(param->nozzle);
	  			return 0;
	  		}
	  		else if((1==param->RxValid)&&(0x3d==param->RxBuffer[3])&&(0x41==param->RxBuffer[4])&&(0x30==param->RxBuffer[5])) //操作失败
	  		{
	  			ICUnlock(param->nozzle);
	  			return 1;
	  		}
	  		else if(timer>=IC_OVERTIME)//操作超时
	  		{
	  			ICUnlock(param->nozzle);
	  			return 2;
	  		}

	  		//等待延时并累计超时值
	  		usleep(10*1000);	
			timer+=10;
	  	}

		//解除卡操作锁
	  	ICUnlock(param->nozzle);
	}
	else
  	{
		//卡操作锁
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

	    //解除卡操作锁
	  	ICUnlock(param->nozzle);

		return istate;
	}

	return 0;
}



/*******************************************************************
*Name			:PsamMFSelect
*Description	:PSAM卡MF选择
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamMFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
		
	//发送PSAM卡MF选择命令
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
*Description	:PSAM卡21文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamFile21Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡21文件读取命令
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
*Description	:PSAM卡22文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamFile22Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡22文件读取命令
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
*Description	:PSAM卡DF选择石化应用1
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:DF				应用选择；1=应用1；2=应用2
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamDFSelect(int nozzle,int sam, unsigned char *buffer, int maxbytes, int DF)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡DF选择石化应用1命令
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
*Description	:PSAM卡23文件读取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamFile23Read(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡23文件读取命令
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
*Description	:PSAM卡获取安全提升状态(GET ANTI-PLAGIAREZE PROOF)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamGetAPProof(int nozzle, int sam,unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡安全提升状态读取命令
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
*Description	:PSAM卡获取随机数
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamGetRandom(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡随机数读取命令
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
*Description	:PSAM卡安全提升认证(ANTI-PLAGIAREZE AUTHENTICATION)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:Ciphertext	安全提升DES计算密文，固定字长8bytes，不足后补0
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamAPAuthen(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡安全提升认证命令
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
*Description	:PSAM灰锁初始化，计算MAC1	(INIT_SAM_GREY_LOCK)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:ICLockInitRandom		IC卡灰锁初始化获得的伪随机数，4bytes
*					:ICLockInitCTC				IC卡灰锁初始化脱机交易序号，2bytes
*					:ICLockInitBalance			IC卡灰锁初始化锁定的卡余额，4bytes
*					:time							消费时间，7bytes
*					:ICLockInitKeysVersion	IC卡灰锁初始化密钥版本号
*					:IcLockInitArithmetic		IC卡灰锁初始化算法标识
*					:IcAppId						IC卡应用序列号后16位，8bytes
*					:IcIssuerMark				IC卡发卡方标识, 8bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamLockInit(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICLockInitRandom, unsigned char *ICLockInitCTC, unsigned char *ICLockInitBalance, unsigned char *time, unsigned char ICLockInitKeysVersion,unsigned char ICLockInitArithmetic, unsigned char *ICAppId, unsigned char *ICIssuerMark)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡灰锁初始化，计算MAC1命令
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
*Description	:PSAM验证MAC2
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:MAC2			MAC2，4bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamMAC2Check(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *MAC2)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送MAC2验证命令
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
*Description	:PSAM卡计算GMAC
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:ICAppId			IC卡应用序列号后16位
*					:ICLockInitCTC	脱机交易序号,2bytes
*					:money				消费金额，HEX
*Output		:buffer				APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamGMAC(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *ICAppId, unsigned char *ICLockInitCTC, unsigned int money)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡计算GMAC命令
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
*Description	:PSAM卡读取GMAC
*Input			:nozzle				枪选0=A1枪，1=B1枪
*					:maxbytes		输出缓存最大长度
*					:ICPsamTTC		PSAM终端交易序号，4bytes
*Output		:buffer				APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamGMACRead(int nozzle,int sam, unsigned char *buffer, int maxbytes, const unsigned char *ICPsamTTC)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡计算GMAC命令
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
*Description	:PSAM卡TMAC计算初始化
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*Output		:buffer				APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamTMACInit(int nozzle,int sam, unsigned char *buffer, int maxbytes)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡TMAC计算初始化命令
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
*Description	:PSAM卡TMAC计算初始化
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:inbuffer		要计算的数据
*					:len				要计算的数据长度
*					:follow			是否有后续字节，0=无后续块；1=有后续块
*					:initvalue		有无初始值，0=无；1=有
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamTMACOperat(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *inbuffer, int len, int follow, int initvalue)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//判断长度
	if(len>100)	return ERROR;

	//判断有无后续字节的参数
	if((0!=follow)&&(1!=follow))	return ERROR;

	//判断有无初始值的参数
	if((0!=initvalue)&&(1!=initvalue))	return ERROR;

	//发送PSAM卡TMAC计算(DES CRYPT)命令
	tx_buffer[0]=sam;
	tx_buffer[1]=(unsigned char)((len+5)>>8);
	tx_buffer[2]=(unsigned char)((len+5)>>0);
	tx_buffer[3]=0x80;
	tx_buffer[4]=0xfa;
	tx_buffer[5]=(initvalue<<2)|(follow<<1)|(1<<0);
	tx_buffer[6]=0;
	tx_buffer[7]=len;											//长度，8的倍数
	memcpy(&tx_buffer[8], inbuffer, len);
	tx_len=len+8;
	istate = IcPsamApduSend(nozzle, sam, tx_buffer, tx_len, (char*)buffer, maxbytes);

	return istate;
}



/*******************************************************************
*Name			:PsamStartBind
*Description	:PSAM卡启动计量注册功能
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:KeyIndex	认证密钥索引号(ACT索引号)
*					:ICAppId			IC卡应用序列号后16位，8bytes
*					:Ciphertext	密文数据，8bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamStartBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡启动计量注册功能命令
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
*Description	:PSAM卡初始化计量注册功能
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:KeyIndex		认证密钥索引号(ACT索引号)
*					:ICAppId		IC卡应用序列号后16位，8bytes
*					:CoID			厂商编码，8bytes
*					:Ciphertext	密文数据，8bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamInitBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char KeyIndex, unsigned char *ICAppId, unsigned char *CoID, unsigned char *Ciphertext)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡初始化计量注册功能命令
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
*Description	:PSAM卡计量注册
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:KeyIndex		认证密钥索引号(ACT索引号)
*					:mboardID		主板ID，8bytes
*					:CoID			厂商编码，8bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamBind(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char const *mboardID, unsigned char *CoID)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;
	
	//发送PSAM卡计量注册命令
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
*Description	:PSAM卡专用DES计算初始化(INIT_FOR_DESCRYPT)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:CalKeyIndex	计算密钥索引号( 日志MAC计算)
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamInitDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char CalKeyIndex)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡专用DES计算初始化命令
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
*Description	:PSAM卡专用DES计算(DES CRYPT)
*Input			:nozzle			枪选0=A1枪，1=B1枪
*					:maxbytes	输出缓存最大长度
*					:time					启动绑定日期时间，7bytes
*					:ACTAppId				ACT卡卡号，10bytes
*					:ACTKeyIndex		ACT认证密钥索引号
*					:RIDAppId				RID卡卡号，10bytes
*					:RIDKeyIndex		RID认证密钥索引号
*					:RIDCalKeyIndex	日志MAC计算密钥索引号
*					:PsamId				PSAM卡卡号，10bytes
*					:mboardID			油机芯片ID，8bytes
*Output		:buffer			APDU数据
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int PsamDESCrypt(int nozzle,int sam, unsigned char *buffer, int maxbytes, unsigned char *time, unsigned char *ACTAppId, unsigned char ACTKeyIndex, unsigned char *RIDAppId, unsigned char RIDKeyIndex, unsigned char RIDCalKeyIndex,unsigned char *PsamId, unsigned char const *mboardID)
{
	unsigned char tx_buffer[128]={0};
	int tx_len=0;
	int istate = 0;

	//发送PSAM卡专用DES计算命令
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
*Description	:进入卡座电特性测试
*Input			:nozzle		枪选0=A1枪，1=B1枪
*					:type		测试模式0x30=单次模式；0x31=连续模式
*Output		:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderTestElectrical(int nozzle, int type)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)	
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送PSAM复位命令
	IcPackSend(param->nozzle, 0x80, type, (unsigned char*)"\x00", 0);
	
	//判断数据接收
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //操作成功
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //操作失败
		{

			ICUnlock(param->nozzle);
			return 1;
		}
		else if(timer>=IC_OVERTIME) //操作超时
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	
		timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcReaderTestProtocol
*Description	:进入卡座协议测试
*Input			:nozzle		枪选0=A1枪，1=B1枪
*					:type		测试模式0x34=单次模式；0x35=连续模式
*Output		:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderTestProtocol(int nozzle, int type)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送PSAM复位命令
	IcPackSend(param->nozzle, 0x80, type, (unsigned char*)"\x00", 0);

	//判断数据接收
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //操作成功
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //操作失败
		{
			ICUnlock(param->nozzle);
			return 1;
		}	
		else if(timer>=IC_OVERTIME) //操作超时
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	
		timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}

#if 0
/*******************************************************************
*Name			:IcReaderTestRadiofrequency
*Description	:进入卡座射频测试
*Input			:nozzle		枪选0=A1枪，1=B1枪
*Output		:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderTestRadiofrequency(int nozzle)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))
		return ERROR;

	//发送PSAM复位命令
	IcPackSend(param->nozzle, 0x42, 0x30, (unsigned char*)"\x00", 0);

	//判断数据接收
	for(timer=0;;)
	{	
		if(1==param->RxValid && 0x42==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //操作成功
		{
			ICUnlock(param->nozzle);
			return 0;
		}	
		else if(1==param->RxValid && 0x42==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //操作失败
		{
			ICUnlock(param->nozzle);
			return 1;
		}
		else if(timer>=IC_OVERTIME) //操作超时
		{

			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	
		timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}
#endif

/*******************************************************************
*Name			:IcReaderTestProtocolTime
*Description	:设置协议连续测试轮询时间
*Input			:nozzle		枪选0=A1枪，1=B1枪
*					:time		时间，单位0.1秒
*Output		:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderTestProtocolTime(int nozzle, int time)
{
	IcStructType *param=NULL;
	unsigned char tx_buffer[16]={0};
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)
		param=&IcStructA1;
	else if(1==nozzle)
		param=&IcStructB1;
	else							
		return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))	
		return ERROR;

	//发送PSAM复位命令
	tx_buffer[0]=(char)time>>8;	tx_buffer[1]=(char)(time>>0);
	IcPackSend(param->nozzle, 0x80, 0x36, tx_buffer, 2);

	//判断数据接收
	for(timer=0;;)
	{
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]) //操作成功
		{
			ICUnlock(param->nozzle);
			return 0;
		}
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4]) //操作失败
		{
			ICUnlock(param->nozzle);
			return 1;
		}	
		else if(timer>=IC_OVERTIME) //操作超时
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	
		timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:IcReaderTestExit
*Description	:退出卡座测试模式
*Input			:nozzle		枪选0=A1枪，1=B1枪
*				:type		测试模式0x30=单次模式；0x31=连续模式
*Output			:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderTestExit(int nozzle)
{
	IcStructType *param=NULL;
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送PSAM复位命令
	IcPackSend(param->nozzle, 0x80, 0x81, (unsigned char*)"\x00", 0);
	
	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x30==param->RxBuffer[4]){

			ICUnlock(param->nozzle);
			return 0;
		}

		//操作失败
		else if(1==param->RxValid && 0x80==param->RxBuffer[3] && 0x31==param->RxBuffer[4])
		{
			ICUnlock(param->nozzle);
			return 1;
		}

		//操作超时
		else if(timer>=IC_OVERTIME){

			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}



/*******************************************************************
*Name			:IcPollLimit
*Description	:禁止轮询命令
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功;其它=失败
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
*Description	:恢复轮询命令
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功;1=操作失败;2=超时;3=nozzle非法;4=输出缓存太小；其他=失败
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
*Description	:读卡器复位
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output			:None
*Return			:0=成功;2=超时;其他=失败
*History		:2014-10-17,modified by syj
*/

int IcReaderReset(int nozzle)
{
	//del unsigned char tx_buffer[128]={0};
	//del int tx_len=0, apdu_len=0;
	IcStructType *param=NULL;
	unsigned int timer=0;

	//判断操作对象
	if(0==nozzle)			param=&IcStructA1;
	else if(1==nozzle)	param=&IcStructB1;
	else							return 3;

	//卡操作锁
	if(0!=ICLock(param->nozzle))	return ERROR;

	//发送读卡器复位命令
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);

	//判断数据接收
	for(timer=0;;)
	{
		//操作成功
		if((1==param->RxValid)&&(0x30==param->RxBuffer[3])&&(0x40==param->RxBuffer[4]))
		{
			ICUnlock(param->nozzle);
			return 0;
		}

		//操作超时
		else
		if(timer>=IC_OVERTIME)
		{
			ICUnlock(param->nozzle);
			return 2;
		}

		//等待延时并累计超时值
		usleep(10*1000);	timer+=10;
	}

	//解除卡操作锁
	ICUnlock(param->nozzle);

	return 0;
}


/*******************************************************************
*Name			:ICStateRead
*Description	:卡状态获取
*Input			:nozzle			枪选0=A1枪，1=B1枪
*Output			:state			卡状态数据
*Return			:0=成功;2=超时;其他=失败
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
*Description	:卡操作模块功能初始化
*Input			:None
*Output			:None
*Return			:0=成功;其他=失败
*History		:2014-10-17,modified by syj
*/
bool IcModuleInit(void)
{
	IcStructType *param=NULL;
	//卡座A1相关参数的初始化
	int nInitRet = pthread_mutex_init(&semIdA1,NULL);	//创建卡操作信号量，初始化为有效
    if(nInitRet != 0)
	{
       printf("Error! Create List 'semIdA1' failed!\n");
	   return false;
	}

	//初始化变量
	param=&IcStructA1;
	param->nozzle=IC_NOZZLE_1;	
	param->pollLimit=0;

	//复位卡座
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



	//卡座B1相关参数初始化
	nInitRet = pthread_mutex_init(&semIdB1,NULL);
	if(nInitRet != 0)
	{
		printf("Error! Create List 'semIdB1' failed!\n");
		return false;
	}

	//初始化变量
	param=&IcStructB1;
	param->nozzle=IC_NOZZLE_2;	
	param->pollLimit=0;

	//复位卡座
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
	//卡座A1参数及任务等的初始化
	

	//创建卡操作信号量，初始化为有效
	//del semIdA1=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==semIdA1)	printf("Error! Create List 'semIdA1' failed!\n");
	pthread_mutex_init(&semIdA1, NULL);

	//初始化变量
	param=&IcStructA1;
	param->nozzle=IC_NOZZLE_1;	param->pollLimit=0;

	//创建卡座数据接收任务
	//del param->tIdReceive=taskSpawn("tIcRxA1", 151, 0, 0x2000, (FUNCPTR)tICReceive, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tIcRxA1' failed!\n");
	pthread_t pIcRxA1;
	param->tIdReceive=pthread_create(&pIcRxA1, NULL, (void*)tICReceive, NULL);
	if(0!=param->tIdReceive) printf("Error!	Creat task 'tIcRxA1' failed!\n");
	pthread_detach(pIcRxA1);

	//创建弹卡信号量
	//param->semIdShoot=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdShoot)	printf("Error! Create List 'IcStructA1.ssemIdShoot' failed!\n");

	//创建弹卡任务
	//param->tIdShoot=taskSpawn("tIcShootA1", 156, 0, 0x2000, (FUNCPTR)tICShoot, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdShoot))	printf("Error!	Creat task 'tIcShootA1' failed!\n");

	//创建卡状态轮询任务
	//del param->tIdPoll=taskSpawn("tIcPollA1", 156, 0, 0x2000, (FUNCPTR)tICPoll, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdPoll))	printf("Error!	Creat task 'tIcPollA1' failed!\n");
	pthread_t pIcPollA1;
	param->tIdPoll=pthread_create(&pIcPollA1, NULL, (void*)tICPoll, NULL);
	if(0!=param->tIdPoll) printf("Error!	Creat task 'tIcPollA1' failed!\n");
	pthread_detach(pIcPollA1);


	//复位卡座
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);




	//***************************************************************************
	//卡座B1参数及任务等的初始化
	

	//创建卡操作信号量，初始化为有效
	//del semIdB1=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==semIdB1)	printf("Error! Create List 'semIdB1' failed!\n");
	pthread_mutex_init(&semIdB1, NULL);

	//初始化变量
	param=&IcStructB1;
	param->nozzle=IC_NOZZLE_2;	param->pollLimit=0;

	//创建卡座数据接收任务
	//del param->tIdReceive=taskSpawn("tIcRxB1", 151, 0, 0x2000, (FUNCPTR)tICReceive, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tIcRxB1' failed!\n");
	pthread_t pIdReceive;
	param->tIdReceive=pthread_create(&pIdReceive, NULL, (void*)tICReceive, NULL);
	if(0!=param->tIdReceive) printf("Error!	Creat task 'tIcRxB1' failed!\n");
	pthread_detach(pIdReceive);

	//创建弹卡信号量
	//param->semIdShoot=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdShoot)	printf("Error! Create List 'IcStructA1.ssemIdShoot' failed!\n");

	//创建弹卡任务
	//param->tIdShoot=taskSpawn("tIcShootA1", 156, 0, 0x2000, (FUNCPTR)tICShoot, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdShoot))	printf("Error!	Creat task 'tIcShootA1' failed!\n");

	//创建卡状态轮询任务
	//del param->tIdPoll=taskSpawn("tIcPollB1", 156, 0, 0x2000, (FUNCPTR)tICPoll, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(param->tIdPoll))	printf("Error!	Creat task 'tIcPollB1' failed!\n");
	pthread_t pIcPollB1;
	param->tIdPoll=pthread_create(&pIcPollB1, NULL, (void*)tICPoll, NULL);
	if(0!=param->tIdPoll) printf("Error!	Creat task 'tIcPollB1' failed!\n");
	pthread_detach(pIcPollB1);

	//复位卡座
	IcPackSend(param->nozzle, 0x30, 0x40, (unsigned char*)"\x00", 0);

	return 0;
}*/


