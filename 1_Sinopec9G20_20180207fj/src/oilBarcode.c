//#include "oilCfg.h"
//#include "oilCom.h"
//#include "oilLog.h"
//#include "oilParam.h"
//#include "oilIpt.h"
//#include "oilBarcode.h"

#include "../inc/main.h"

//条码扫描模块参数
typedef struct
{
	unsigned char Brand;	//条码模块品牌YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125

	//条码模块初始化时接收数据
	unsigned char InitRxLen;					//接收数据长度	
	unsigned char InitRxBuffer[BAR_RXLEN_MAX];	//接收的数据

	//条码数据接收及校验相关
	unsigned char Data[6];							//条码数据ASCII，非全0时表示有数据
	unsigned char RxFlag;							//接收标识0=无条码数据；1=有完整的条码数据
	unsigned char RxLen;							//接收数据长度
	unsigned char RxBuffer[BAR_RXLEN_MAX];			//接收的数据
	
	int DEV;       //条码操作串口设备ID
	int UID;       //当前用户，0=无用户；其它=用户ID	
	int tIdReceive;//扫描数据接收任务ID	
	int tIdScan;   //扫描任务ID	
	int Scan;      //扫描操作，0=停止扫描，1=扫描

	
	sem_t semIdScan;
	pthread_mutex_t semIdData;

	//扫描操作启动信号量
	//SEM_ID semIdScan;  //fj:
	//条码数据操作信号量
	//SEM_ID semIdData; //fj:
}BarcodeStructType;

//条码扫描数据
static BarcodeStructType barParamA, barParamB;


//条码自助授权后台数据
typedef struct
{
	int ComFd;								//条码自助后台通讯串口号								
	int tIdRx;								//条码自助后台数据接收任务											
	unsigned char RxStep;					//条码自助后台数据接收步骤				
	unsigned char RxBuffer[CPOS_MAX_LEN];	//条码自助后台数据
	int RxLen;								//条码自助后台数据长度
}CPOSStructType;

static CPOSStructType CPOS;

//内部函数声明
static unsigned char barEAN8Parity(unsigned char *buffer, unsigned char nbytes);

/*******************************************************************
*Name				:barEAN8Parity
*Description		:EAN8格式条码校验
*Input				:buffer	校验数据，ASCII
*						:nbytes	校验数据长度，固定7字节
*Output			:None
*Return				:校验码，0xff=参数错误
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
	
	pRet1 = pData[0]+pData[2]+pData[4]+pData[6];//1，求偶数位之和
	pRet1 = pRet1*3;		                    //2，1所得和乘以3				
	pRet2 = pData[1]+pData[3]+pData[5];	        //3，求前奇数位前3个之和	
	pRet1 = pRet1+pRet2;	                    //4，2与3步求和		
	
	if(pRet1<=10)	
		pVal=10-pRet1;	 	                    //5，用大于或等于4结果的10的最小整数倍减去4结果
	else if((pRet1%10)==0)	
		pVal=0;
	else if((pRet1%10)!=0)	
		pVal=(pRet1/10+1)*10-pRet1;

	return (pVal|0x30);
}


/*******************************************************************
*Name				:tCPOSRx
*Description		:接收并解析条码自助后台的数据
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

	//初始化接收数据
	CPOS.RxStep=0;	
	CPOS.RxLen=0;

	//接收条码自助后台的数据，广播发送给所有油枪
	FOREVER
	{
		rx_len=comRead(CPOS.ComFd, rx_buffer, 1);

		if(rx_len>0)
		{
			CPOS.RxBuffer[CPOS.RxLen]=rx_buffer[0];
			switch(CPOS.RxStep)
			{
			case 0:	//接收数据开始字节
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
			case 1:	//接收数据目标地址、源地址、帧号/控制、有效数据长度（不能有0xFA）			
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
			case 2://数据位0xfa时初步认为该字节为转意字符执行下一步处理，否则保存该字节数据	
				if(0xfa==rx_buffer[0])	
				{	
					CPOS.RxStep=3;	
				}
				else
				{	
					CPOS.RxLen++;	
				}

				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
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
			case 3:	//如果是0xfa则作为转义保存当前数据，如果不是0xfa则认为前一个0xfa为包头	
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

				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
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
*Description		:条码自助授权后台数据发送
*Input				:nozzle		0=1号端口；1=2号端口
*						:pos_p		逻辑编号POS_P
*						:buffer		有效数据
*						:nbytes		有效数据长度
*Output			:None
*Return				:0=成功；其他=失败
*History			:2014-10-21,modified by syj
*/
int CPOSWrite(int nozzle, unsigned char pos_p, unsigned char *buffer, int nbytes)
{
	int istate = 0;
	static unsigned char frame=0;
	unsigned int len=0, crc_data=0, i=0;
	unsigned long long data=0;
	unsigned char data_buffer[CPOS_MAX_LEN]={0}, tx_buffer[2*CPOS_MAX_LEN]={0}, tx_len=0;
	
	if(8+nbytes > CPOS_MAX_LEN)//判断有效数据长度
		return ERROR;

	if(frame>=0x3f)	//帧号最大为0x3f
		frame=0;

	//组织数据
	data_buffer[0]=0xfa;					//数据包头
	data_buffer[1]=0;						//目标地址
	data_buffer[2]=pos_p;					//源地址
	data_buffer[3]=(0<<7)|(0<<6)|(frame++);	//帧号			
	data=hex2Bcd(nbytes);					//有效数据长度
	data_buffer[4]=(char)(data>>8);	
	data_buffer[5]=(char)(data>>0);
	memcpy(&data_buffer[6], buffer, nbytes);//有效数据	

	//CRC校验
	crc_data=crc16Get(&data_buffer[1], 5+nbytes);							
	data_buffer[6+nbytes]=(char)(crc_data>>8);
	data_buffer[7+nbytes]=(char)(crc_data>>0);

	//添加0xfa
	memcpy(&tx_buffer[0], &data_buffer[0], 6);
	tx_len=6;
	for(i=6; i<8+nbytes; i++)
	{
		//发送数据赋值
		tx_buffer[tx_len]=data_buffer[i];	
		tx_len++;	
		if(tx_len>=2*CPOS_MAX_LEN)	//防止发送长度溢出
			break;
	
		if(0xfa==data_buffer[i])//发送的数据为0xfa时添加一位0xfa
		{
			tx_buffer[tx_len]=data_buffer[i];
			tx_len++;
		}
	
		if(tx_len>=2*CPOS_MAX_LEN)//防止发送长度溢出
			break;
	}
	
	if(MODEL_LIANDA == paramModelGet())//串口发送并判断结果
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
*Description		:条码扫描
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:None
*Return				:None
*History			:2014-10-21,modified by syj
*/
void tBarcodeScan(int nozzle)
{
	unsigned int i=0;
	BarcodeStructType *param=NULL;

	//判断模块选择
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

	//初始化扫描模块
	barScanModuleInit(nozzle, param->Brand);

	FOREVER
	{
		//等待扫描任务启动
		//semTake(param->semIdScan, WAIT_FOREVER);  //fj:
		sem_wait(&param->semIdScan);

	
		//启动扫描:
		//启动扫描时即获取到启动扫描信号量则本任务执行
		//发送扫描命令后等待条码扫描，达到单次扫描时间则继续下次扫描，
		//等待过程中判断如果扫描到条码或不在扫描则退出等待过程，
		//判断是否有条码数据，有则保存并退出扫描；
		//判断是否继续扫描，不继续扫描则退出扫描
		//上述判断均不符合则再次发送扫描命令。
		
		while(BARCODE_SCAN==param->Scan)
		{
			//扫描命令
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
	
			for(i=0; i<HONEYWELL_TIME; i++)//等待扫描数据
			{
				if(1==param->RxFlag || BARCODE_IDLE==param->Scan)
					break;
				//taskDelay(ONE_MILLI_SECOND);
				usleep(1000);
			}
			if(1==param->RxFlag)//判断是否有条码数据
			{
				//semTake(param->semIdData, WAIT_FOREVER); //fj:
				pthread_mutex_lock(&param->semIdData);
				memcpy(param->Data, &param->RxBuffer[1], 6);
				pthread_mutex_unlock(&param->semIdData);
				//semGive(param->semIdData); //fj:
				break;
			}	
			if(BARCODE_IDLE==param->Scan)//判断是否继续扫描
			{
				break;
			}
		}

		//停止扫描
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
*Description		:条码数据读取
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:None
*Return				:None
*History			:2014-10-21,modified by syj
*/
void tBarcodeReceive(int nozzle)
{
	unsigned char buffer[16]={0};
	int len=0;
	BarcodeStructType *param=NULL;

	//判断模块选择
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
		
		len=comRead(param->DEV, buffer, 1);//接收到数据解析	
		if(!(len>0))	                   //判断是否接收到数据
		{
			//taskDelay(1);
			//usleep(1000);
			continue;
		}
		
		if(param->InitRxLen>=BAR_RXLEN_MAX)	//接收为初始化操作使用的数据
		{
			param->InitRxLen=0;
		}
		param->InitRxBuffer[param->InitRxLen++]=buffer[0];
	
		if(0!=param->RxFlag)//判断是否允许解析
		{
			//taskDelay(1);
			//usleep(1000);
			continue;
		}
	
		if(param->RxLen>=BAR_RXLEN_MAX)	//防止接收长度溢出
		{
			param->RxLen=0;
		}
	
		param->RxBuffer[param->RxLen]=buffer[0];	//缓存数据，首位数据应为0x36
		if(0==param->RxLen && 0x36!=buffer[0])
		{
			param->RxLen=0;
		}
		else
		{
			param->RxLen++;
		}
	
		if(param->RxLen>=8)//校验数据
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
*Description		:条码扫描模块初始化，主要是根据具体品牌对扫描模块上电后的一些命令配置
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*						:brand		模块品牌YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/
int barScanModuleInit(int nozzle, unsigned char brand)
{
	int i=0, itimes=0, istate=0;
	BarcodeStructType *param=NULL;

	//判断模块选择
	if(BARCODE_NOZZLE_1==nozzle)		
		param=&barParamA;
	else if(BARCODE_NOZZLE_2==nozzle)	
		param=&barParamB;
	else	
		return ERROR;

	//判断品牌
	if(!(YUANJD_BRAND==brand||YUANJD_LV1000==brand||HONEYWELL_BRAND==brand||HONEYWELL_IS4125==brand))
		return ERROR;

	//设置品牌
	param->Brand=brand;


	//远景达一维模块LV1000需要进行命令设置
	if(YUANJD_LV1000==param->Brand)
	{
    itimes=0;
LV1000_WRITE_SEND0:

		//发送进入命令模式的命令
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "$$$$#99900116;%%%%", 18);

		//等待超时或返回
		for(i=0;;)
		{	
			if(0==memcmp(param->InitRxBuffer, "@@@@!99900116;^^^^", 18))//正确返回则退出
			{
				break;
			}

			//延时10毫秒
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);  //fj:20170930	
			i+=(10*ONE_MILLI_SECOND);//超时累加

			//判断超时，超时小于三次继续发送，超时超过三次退出
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

	//设置为霍尼韦尔一维模块IS4125
	if(HONEYWELL_IS4125==param->Brand)
	{
itimes=0;
IS4125_WRITE_SEND0:
		//发送进入编程的命令
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);	
		for(i=0;;)//等待超时或返回
		{
			//正确返回则退出
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//延时10毫秒
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000); //fj:	
			i+=(10*ONE_MILLI_SECOND);//超时累加

			//判断超时，超时小于三次继续发送，超时超过三次退出
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
		//发送设置命令模式的命令，与上一条命令间间隔200毫秒
		//taskDelay(200*ONE_MILLI_SECOND);
	    usleep(200000);	
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);
	
		for(i=0;;)//等待超时或返回
		{
			//正确返回则退出
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//延时10毫秒
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000); //fj:	
			i+=(10*ONE_MILLI_SECOND);//超时累加

			//判断超时，超时小于三次继续发送，超时超过三次退出
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
		//发送退出编程的命令，与上一条命令间间隔200毫秒
		//taskDelay(200*ONE_MILLI_SECOND);
		usleep(200);
		memset(param->InitRxBuffer, 0, BAR_RXLEN_MAX);	param->InitRxLen=0;
		comWrite(param->DEV, "\x02\x39\x39\x39\x39\x39\x39\x03", 8);

		for(i=0;;)//等待超时或返回
		{
			//正确返回则退出
			if(0x06==param->InitRxBuffer[0])
			{
				break;
			}

			//延时10毫秒
			//taskDelay(10*ONE_MILLI_SECOND);
			usleep(10000);	
			i+=(10*ONE_MILLI_SECOND);	//超时累加

			//判断超时，超时小于三次继续发送，超时超过三次退出
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
*Description		:条码扫描模块品牌设置
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*						:brand		模块品牌YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barBrandWrite(int nozzle, unsigned char brand)
{
	int i=0, itimes=0, istate=0;
	unsigned int ioffset = 0;
	BarcodeStructType *param=NULL;
	char ibuffer[6] = {0};

	//判断模块选择
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

	//判断品牌
	if(!(YUANJD_BRAND==brand||YUANJD_LV1000==brand||HONEYWELL_BRAND==brand||HONEYWELL_IS4125==brand))
		return ERROR;

	//写入配置文件
	*ibuffer = brand;
	if(0!=paramSetupWrite(ioffset, ibuffer, 1))
	{
		return ERROR;
	}

	param->Brand=brand;	                //设置品牌
	barScanModuleInit(nozzle, brand);	//配置扫描模块命令

	return 0;
}


/*******************************************************************
*Name				:barBrandRead
*Description		:条码扫描模块品牌获取
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:brand		模块品牌YUANJD_BRAND/YUANJD_LV1000/HONEYWELL_BRAND/HONEYWELL_IS4125
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barBrandRead(int nozzle, unsigned char *brand)
{
	BarcodeStructType *param=NULL;
	
	if(BARCODE_NOZZLE_1==nozzle)//判断模块选择		
		param=&barParamA;
	else if(BARCODE_NOZZLE_2==nozzle)	
		param=&barParamB;
	else
		return ERROR;
	
	*brand=param->Brand;//返回扫描模块品牌

	return 0;
}


/*******************************************************************
*Name				:barScan
*Description		:条码扫描，直到停止或扫到条码，清除上次的条码
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barScan(int nozzle, int UID)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//判断模块选择
		param=&barParamA;
	else if(1==nozzle)	
		param=&barParamB;
	else	
		return ERROR;

	//清除上次的条码
	//semTake(param->semIdData, WAIT_FOREVER); //fj:
	pthread_mutex_lock(&param->semIdData);
	memset(param->Data, 0, 6);
	pthread_mutex_unlock(&param->semIdData);
	//semGive(param->semIdData);  //fj:

	//记录用户ID
	param->UID = UID;

	//如果已经处于扫描状态则直接返回成功
	if(BARCODE_SCAN == param->Scan)
	{
		return 0;
	}

	//启动扫描
	param->Scan=BARCODE_SCAN;
	sem_post(&param->semIdScan);
	//semGive(param->semIdScan); //fj:

	return 0;
}


/*******************************************************************
*Name				:barStop
*Description		:条码停止扫描
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*						:UID			用户ID
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barStop(int nozzle, int UID)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//判断模块选择		
		param=&barParamA;
	else if(1==nozzle)	
		param=&barParamB;
	else
		return ERROR;

	param->UID = UID;	//用户ID

	//启动停止
	param->Scan=BARCODE_IDLE;
	//semGive(param->semIdScan);  //fj:
	sem_post(&param->semIdScan);
	return 0;
}


/*******************************************************************
*Name				:barUserIDSet
*Description		:设置当前用户ID
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*						:id				用户ID，BARCODE_USER_NO表示无用户，即当前用户放弃对其使用
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barUserIDSet(int nozzle, int id)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)	//判断模块选择	
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
*Description		:获取当前用户ID
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:None
*Return				:用户ID，BARCODE_USER_NO表示无用户，即当前用户放弃对其使用
*History			:2014-10-21,modified by syj
*/

int barUserIDGet(int nozzle)
{
	BarcodeStructType *param=NULL;


	if(0==nozzle)	//判断模块选择
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else
		return ERROR;

	return 	(param->UID);
}


/*******************************************************************
*Name				:barStateRead
*Description		:条码扫描状态读取
*Input				:nozzle		模块选择0=1号模块；1=2号模块
*Output			:None
*Return				:0=空闲；1=正在扫描；其它=错误
*History			:2014-10-21,modified by syj
*/

int barStateRead(int nozzle)
{
	BarcodeStructType *param=NULL;

	if(0==nozzle)	//判断模块选择
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else	
		return ERROR;

	return param->Scan;
}


/*******************************************************************
*Name				:barRead
*Description		:条码读取，读取之后自动清除
*Input				:nozzle			模块选择0=1号模块；1=2号模块
*						:maxbytes	条码输出缓存最大长度，不小于6bytes
*Output			:buffer			条码ASCII，全0表示无条码，6bytes
*Return				:0=成功；其它=失败
*History			:2014-10-21,modified by syj
*/

int barRead(int nozzle, unsigned char *buffer, int maxbytes)
{
	BarcodeStructType *param=NULL;
	
	if(0==nozzle)		//判断模块选择
		param=&barParamA;
	else if(1==nozzle)
		param=&barParamB;
	else	
		return ERROR;
	
	if(maxbytes<6)//判断缓存长度
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
*Description		:条码功能模块初始化
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
	
	param=&barParamA;//1号条码模块状态初始化	
	param->DEV=COM11;//条码模块连接串口设备ID

	//创建条码数据操作信号量
	//param->semIdData=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==param->semIdData)	printf("Error! Create List 'barParamA.semIdData' failed!\n");
	nMutexInit = pthread_mutex_init(&param->semIdData,NULL);
	if(nMutexInit != 0)
	{
		printf("Error! Create List 'barParamA.semIdData' failed!\n");
		return false;
	}

	//创建扫描任务启动信号量
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


	//创建扫描数据接收任务
	//param->tIdReceive=taskSpawn("tBarRxA", 155, 0, 0x2000, (FUNCPTR)tBarcodeReceive, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tBarRxA' failed!\n");

	//获取当前配置的品牌，无有效配置则默认为远景达LV1000
	//	注:因设置中取条码模块返回数据，故此处应添加在条码模块数据接收任务启动之后
	
	istate = paramSetupRead(PRM_BARCODE_BRAND_A, rdbuffer, 1);
	if(0==istate && (YUANJD_BRAND==*rdbuffer || YUANJD_LV1000==*rdbuffer || HONEYWELL_BRAND==*rdbuffer || HONEYWELL_IS4125==*rdbuffer))
	{	
		param->Brand=*rdbuffer;
	}
	else
	{
		param->Brand = YUANJD_LV1000;
	}
	

	//创建条码扫描任务
	//param->tIdScan=taskSpawn("tBarSacnA", 155, 0, 0x2000, (FUNCPTR)tBarcodeScan, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdScan))	printf("Error!	Creat task 'tBarSacnA' failed!\n");


	
	param=&barParamB; //2号条码模块状态初始化
	param->DEV=COM12; //条码模块连接串口设备ID

	//创建条码数据操作信号量
	//param->semIdData=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//if(NULL==param->semIdData)	printf("Error! Create List 'barParamB.semIdData' failed!\n");
	nMutexInit = pthread_mutex_init(&param->semIdData,NULL);
	if(nMutexInit != 0)
	{
		printf("Error! Create List 'barParamB.semIdData' failed!\n");
		return false;
	}

	//创建扫描任务启动信号量
	//param->semIdScan=semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	//if(NULL==param->semIdScan)	printf("Error! Create List 'barParamB.semIdScan' failed!\n");
	nSemInit = sem_init(&param->semIdScan,0,0);
	if(nSemInit != 0)
	{
		printf("Error! Create List 'barParamB.semIdScan' failed!\n");
		return false;
	}

	//创建扫描数据接收任务
	//param->tIdReceive=taskSpawn("tBarRxB", 155, 0, 0x2000, (FUNCPTR)tBarcodeReceive, 1,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdReceive))	printf("Error!	Creat task 'tBarRxB' failed!\n");

	//获取当前配置的品牌，无有效配置则默认为远景达LV1000
	//注:因设置中取条码模块返回数据，故此处应添加在条码模块数据接收任务启动之后
	
	istate = paramSetupRead(PRM_BARCODE_BRAND_B, rdbuffer, 1);
	if(0==istate && (YUANJD_BRAND==*rdbuffer || YUANJD_LV1000==*rdbuffer || HONEYWELL_BRAND==*rdbuffer || HONEYWELL_IS4125==*rdbuffer))
	{	
		param->Brand=*rdbuffer;
	}
	else
	{
		param->Brand = YUANJD_LV1000;
	}

	//创建条码扫描任务
	//param->tIdScan=taskSpawn("tBarSacnB", 155, 0, 0x2000, (FUNCPTR)tBarcodeScan, 1,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(param->tIdScan))	printf("Error!	Creat task 'tBarSacnB' failed!\n");


	//条码授权后台数据初始化
	CPOS.ComFd=COM15;	
	CPOS.RxStep=0;
	CPOS.RxLen=0;

	//接收条码自助后台数据任务初始化
	//CPOS.tIdRx=taskSpawn("tCPOSRx", 153, 0, 0x4000, (FUNCPTR)tCPOSRx, 0,1,2,3,4,5,6,7,8,9);
	//if(OK!=taskIdVerify(CPOS.tIdRx))		printf("Error!	Creat task 'tCPOSRx' failed!\n");

	return true;
}


