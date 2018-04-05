
#include "oilCfg.h"
#include "oilFram.h"
#include "oilParam.h"
#include "oilSensor.h"
#include "oilTax.h"
#include "oilLog.h"
#include "oilCom.h"
#include "oilJl.h"


/*本结构用于接收税控报税口数据*/
typedef struct {

	unsigned char RxFlag;					/*接受标识，0=允许接收；1=数据正在使用等原因禁止覆盖接收*/
	unsigned char Buffer[128];			/*接收的数据*/
	unsigned char Length;					/*接收的数据长度*/
	unsigned char IsAvilable;				/*是否接收到完整且正确的数据帧，0=无；1=有*/
	unsigned char TxCmd;					/*发送的命令字，接收时判断接收对应命令字的数据*/
}jlTaxDataStructType;
jlTaxDataStructType jlTaxData;
/*与税控报税口通讯的信号量，保护同一时间段只有一个任务在使用报税口数据*/
LOCAL SEM_ID jlSemIdTax=NULL;
/*接收税控报税口数据的函数*/
LOCAL void jlTaxNoteRecive(void);
/*读取税控当次加油数据*/
LOCAL int jlTaxNoteRead(JlParamStruct *jlparam, unsigned int *money, unsigned int *volume, unsigned int *price);


/*计量加油启动失败原因*/
const char *jlStartFiledReson[]=
{
	"成功",
	"枪选非法",
	"单价不一致",
	"计量单价非法",
	"当量非法",
	"无脉冲超时时间非法",
	"提前量非法",
	"屏蔽量非法",
	"过冲屏蔽量非法",
	"大阀开启延迟出油量非法",
	"计量机型非法",
	"预置方式非法",
	"预置量非法",
	"电源状态非法",
	"税控启动失败",
	"缺一路脉冲超次",
	"零加油超次",
	"无脉冲停机超次",
	"缺一路编码器超次",
};


/*计量加油停机代码*/
const char *jlStopReason[]=
{
	"正常",
	"缺一路脉冲，停机",
	"达到预置量，停机",
	"税控禁止，停机",
	"无脉冲超时，停机",
	"主机掉电，停机",
	"A1缺一路脉冲",
	"A2缺一路脉冲",
	"B1缺一路脉冲",
	"B2缺一路脉冲",
	"A1缺一组脉冲",
	"A2缺一组脉冲",
	"B1缺一组脉冲",
	"B2缺一组脉冲",
};


/*计量参数声明*/
static JlParamStruct JlParamA1, JlParamB1;

/*看门狗定时器*/
static WDOG_ID JlWdgId=NULL;

/*内部函数声明*/
LOCAL void jlTaxNoteRecive(void);
LOCAL int jlTaxNoteRead(JlParamStruct *jlparam, unsigned int *volume, unsigned int *money, unsigned int *price);
static int jlLock(JlParamStruct *jlparam);
static int jlUnlock(JlParamStruct *jlparam);
static void jlWdgIsr(void);
static int jlOilDataWrite(JlParamStruct *jlparam);
static void jlOilling(JlParamStruct *jlparam);



/********************************************************************
*Name				:jlTaxNoteRecive
*Description		:接受并解析税控报税口数据
*Input				:None
*Output			:None
*Return				:0=成功；其它=失败
*History			:2015-08-05,modified by syj
*/
LOCAL void jlTaxNoteRecive(void)
{
	unsigned char read_buffer[32+1] = {0};
	int read_len = 0, i = 0, ixor = 0,j=0;

	FOREVER
	{
		read_len=comRead(COM6, read_buffer, 32);
		for(i = 0; i < read_len; i++)
		{
			/*标识未清，不允许存储数据*/
			if(0!=jlTaxData.RxFlag || 0!=jlTaxData.IsAvilable)
			{
				break;
			}

			/*防止缓存溢出*/
			if(jlTaxData.Length >= 128)	jlTaxData.Length = 0;
			
			/*缓存数据*/
			jlTaxData.Buffer[jlTaxData.Length]=read_buffer[i];
			if(0 == jlTaxData.Length && 0xbb == read_buffer[i])			jlTaxData.Length++;
			else if(0 == jlTaxData.Length && 0xbb != read_buffer[i])	jlTaxData.Length = 0;
			else																						jlTaxData.Length++;

			/*判断是否是符合协议的数据*/
			if(jlTaxData.Length >= 4 && 0xff!=jlTaxData.Buffer[2])
			{
				jlTaxData.RxFlag=0;	jlTaxData.IsAvilable=0;	jlTaxData.Length=0;
				continue;
			}

			/*校验数据*/
			if(jlTaxData.Length >= 5 && jlTaxData.Length >= jlTaxData.Buffer[1]+2)
			{
				ixor = xorGet(&jlTaxData.Buffer[2], jlTaxData.Length-3);
				if(ixor == jlTaxData.Buffer[jlTaxData.Length - 1] && jlTaxData.TxCmd == jlTaxData.Buffer[3])
				{
/*
printf("rx111=====");
for(j=0; j<jlTaxData.Length ; j++)	printf(":%x", jlTaxData.Buffer[j]);
printf("\n");
//*/
					jlTaxData.RxFlag=1;	jlTaxData.IsAvilable=1;
				}
				else
				{
/*
printf("rx222=====");
for(j=0; j<jlTaxData.Length ; j++)	printf(":%x", jlTaxData.Buffer[j]);
printf("\n");
*/
					jlTaxData.RxFlag=0;	jlTaxData.IsAvilable=0;	jlTaxData.Length=0;
				}
			}
		}

		taskDelay(1);
	}

	return;
}


/********************************************************************
*Name				:jlTaxNoteRead
*Description		:通过税控报税口，获取当次加油数据
*Input				:jlparam		计量操作数据结构
*Output			:None
*Return				:0=成功；其它=失败
*History			:2015-08-05,modified by syj
*/
LOCAL int jlTaxNoteRead(JlParamStruct *jlparam, unsigned int *volume, unsigned int *money, unsigned int *price)
{
	int istate=0, tid=0,i=0;
	unsigned int overtimer=0;

	/*判断信号量是否有效*/
	if(NULL==jlSemIdTax)
	{
		istate=ERROR;
		goto DONE;
	}

	/*获取信号量*/
	semTake(jlSemIdTax, WAIT_FOREVER);

	/*初始化接收缓存*/
	memset(jlTaxData.Buffer, 0, sizeof(jlTaxData.Buffer));	
	jlTaxData.Length=0;	jlTaxData.RxFlag=0;	jlTaxData.IsAvilable=0;	jlTaxData.TxCmd = 0;

	/*创建税控数据接收任务*/
	tid=taskSpawn("tTaxRx", 155, 0, 0x2000, (FUNCPTR)jlTaxNoteRecive, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(tid))
	{
		istate=ERROR;
		goto DONE;
	}
	taskDelay(sysClkRateGet()/10);

	/*申请读取当次数据*/
	jlTaxData.TxCmd = 0xf1;
	if(0!=taxNoteRead(jlparam->Nozzle))
	{
		istate=ERROR;
		goto DONE;
	}

	/*判断并接收数据*/
	FOREVER
	{

		if(1==jlTaxData.IsAvilable && 0xBB==jlTaxData.Buffer[0] && 0xF1==jlTaxData.Buffer[3])
		{
#if _TYPE_BIG260_
			*volume=(jlTaxData.Buffer[11]&0x0f)*10000000+(jlTaxData.Buffer[12]&0x0f)*1000000+\
				(jlTaxData.Buffer[13]&0x0f)*100000+(jlTaxData.Buffer[14]&0x0f)*10000+\
				(jlTaxData.Buffer[15]&0x0f)*1000+(jlTaxData.Buffer[16]&0x0f)*100+\
				(jlTaxData.Buffer[17]&0x0f)*10+(jlTaxData.Buffer[18]&0x0f)*1;
			*money=(jlTaxData.Buffer[19]&0x0f)*10000000+(jlTaxData.Buffer[20]&0x0f)*1000000+\
				(jlTaxData.Buffer[21]&0x0f)*100000+(jlTaxData.Buffer[22]&0x0f)*10000+\
				(jlTaxData.Buffer[23]&0x0f)*1000+(jlTaxData.Buffer[24]&0x0f)*100+\
				(jlTaxData.Buffer[25]&0x0f)*10+(jlTaxData.Buffer[26]&0x0f)*1;
			*price=(jlTaxData.Buffer[27]&0x0f)*100000+(jlTaxData.Buffer[28]&0x0f)*10000+\
				(jlTaxData.Buffer[29]&0x0f)*1000+(jlTaxData.Buffer[30]&0x0f)*100+\
				(jlTaxData.Buffer[31]&0x0f)*10+(jlTaxData.Buffer[32]&0x0f)*1;
#else
			*volume=(jlTaxData.Buffer[11]&0x0f)*100000+(jlTaxData.Buffer[12]&0x0f)*10000+\
				(jlTaxData.Buffer[13]&0x0f)*1000+(jlTaxData.Buffer[14]&0x0f)*100+\
				(jlTaxData.Buffer[15]&0x0f)*10+(jlTaxData.Buffer[16]&0x0f)*1;
			*money=(jlTaxData.Buffer[17]&0x0f)*100000+(jlTaxData.Buffer[18]&0x0f)*10000+\
				(jlTaxData.Buffer[19]&0x0f)*1000+(jlTaxData.Buffer[20]&0x0f)*100+\
				(jlTaxData.Buffer[21]&0x0f)*10+(jlTaxData.Buffer[22]&0x0f)*1;
			*price=(jlTaxData.Buffer[23]&0x0f)*1000+(jlTaxData.Buffer[24]&0x0f)*100+\
				(jlTaxData.Buffer[25]&0x0f)*10+(jlTaxData.Buffer[26]&0x0f)*1;
#endif

			goto DONE;
		}
		else if(overtimer>=(5*ONE_SECOND))
		{
			istate=ERROR;
			goto DONE;
		}

		overtimer+=100*ONE_MILLI_SECOND;
		taskDelay(100*ONE_MILLI_SECOND);
	}

DONE:
	if(OK==taskIdVerify(tid))	taskDelete(tid);

	semGive(jlSemIdTax);

	return istate;
}


/********************************************************************
*Name				:jlTaxEquivalentRead
*Description		:通过税控报税口，获取税控K值
*Input				:nozzle			0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:equivalent	获取到的当量(压缩BCD格式，如K值5000，则输出值为0x5000)
*Return				:0=成功；其它=失败
*History			:2016-03-01,modified by syj
*/
int jlTaxEquivalentRead(int nozzle, int *equivalent)
{
	int istate=0, tid=0,i=0;
	unsigned int overtimer=0;
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*判断信号量是否有效*/
	if(NULL==jlSemIdTax)
	{
		istate=ERROR;
		goto DONE;
	}

	/*获取信号量*/
	semTake(jlSemIdTax, WAIT_FOREVER);

	/*初始化接收缓存*/
	memset(jlTaxData.Buffer, 0, sizeof(jlTaxData.Buffer));	
	jlTaxData.Length=0;	jlTaxData.RxFlag=0;	jlTaxData.IsAvilable=0;	jlTaxData.TxCmd = 0;

	/*创建税控数据接收任务*/
	tid=taskSpawn("tTaxRx", 155, 0, 0x2000, (FUNCPTR)jlTaxNoteRecive, 0,1,2,3,4,5,6,7,8,9);
	if(OK!=taskIdVerify(tid))
	{
		istate=ERROR;
		goto DONE;
	}
	taskDelay(sysClkRateGet()/10);

	/*申请读取当次数据*/
	jlTaxData.TxCmd = 0xf3;
	if(0!=taxEquivalentRead(nozzle))
	{
		istate=ERROR;
		goto DONE;
	}

	/*判断并接收数据*/
	FOREVER
	{
		if(1==jlTaxData.IsAvilable && 0xBB==jlTaxData.Buffer[0] && 0xF3==jlTaxData.Buffer[3])
		{
			*equivalent=(jlTaxData.Buffer[5]<<16)|(jlTaxData.Buffer[6]<<8)|(jlTaxData.Buffer[7]<<0);
			goto DONE;
		}
		else if(overtimer>=(5*ONE_SECOND))
		{
			istate=ERROR;
			goto DONE;
		}

		overtimer+=100*ONE_MILLI_SECOND;
		taskDelay(100*ONE_MILLI_SECOND);
	}

DONE:
	if(OK==taskIdVerify(tid))	taskDelete(tid);

	semGive(jlSemIdTax);

	return istate;
}

int jlBigVolTimeRead(int nozzle,unsigned int *time)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*time=jlparam->bigVolTime;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}
int jlBigVolSpeedRead(int nozzle,unsigned int *Speed)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*Speed=jlparam->bigVolSpeed;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlLock
*Description		:计量操作锁，请勿嵌套使用本锁进行操作，一般只在变量操作时使用
*Input				:jlparam		计量操作数据结构
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
static int jlLock(JlParamStruct *jlparam)
{
	semTake(jlparam->SemId, WAIT_FOREVER);
	return 0;
}


/********************************************************************
*Name				:jlUnlock
*Description		:计量操作锁解锁，请勿嵌套使用本锁进行操作
*Input				:jlparam		计量操作数据结构
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
static int jlUnlock(JlParamStruct *jlparam)
{
	semGive(jlparam->SemId);
	return 0;
}


/********************************************************************
*Name				:jlWdgIsr
*Description		:计量定时看门狗中断处理函数
*Input				:None
*Output			:None
*Return				:None
*History			:2013-08-05,modified by syj
*/
static void jlWdgIsr()
{
	/*无脉冲超时时间*/
	JlParamA1.NoSensorTimer+=JL_WDG_TICKS;
	JlParamB1.NoSensorTimer+=JL_WDG_TICKS;

	JlParamA1.bigTimeA1+=JL_WDG_TICKS;
	JlParamA1.bigTimeA2+=JL_WDG_TICKS;
	JlParamB1.bigTimeB1+=JL_WDG_TICKS;
	JlParamB1.bigTimeB2+=JL_WDG_TICKS;		
	
	wdStart(JlWdgId, JL_WDG_TICKS, (FUNCPTR)jlWdgIsr, 0);

	return;
}


/********************************************************************
*Name				:jlOilDataWrite
*Description		:计量加油数据(包括油量，金额，单价)保存
*Input				:jlparam	油枪参数数据结构指针
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
static int jlOilDataWrite(JlParamStruct *jlparam)
{
	unsigned int crc=0, offset=0, offset_backup=0;
	unsigned char wrbuffer[64]={0};

	/*计算本枪加油信息起始位置偏移*/
	if(JL_NOZZLE_1==jlparam->Nozzle)	{offset=JL_FM_DATA;						offset_backup=JL_FM_DATA2;}
	else														{offset=JL_FM_DATA+JL_FM_LEN;	offset_backup=JL_FM_DATA2+JL_FM_LEN;}

	/*锁*/
	jlLock(jlparam);

	/*获得当前加油数据*/
	wrbuffer[0]=(char)(jlparam->OilVolume>>16);	wrbuffer[1]=(char)(jlparam->OilVolume>>8);		wrbuffer[2]=(char)(jlparam->OilVolume>>0);
	wrbuffer[3]=(char)(jlparam->OilMoney>>16);		wrbuffer[4]=(char)(jlparam->OilMoney>>8);		wrbuffer[5]=(char)(jlparam->OilMoney>>0);
	wrbuffer[6]=(char)(jlparam->OilPrice>>8);			wrbuffer[7]=(char)(jlparam->OilPrice>>0);
	wrbuffer[8]=(char)(jlparam->VolumeSum>>40);	wrbuffer[9]=(char)(jlparam->VolumeSum>>32);
	wrbuffer[10]=(char)(jlparam->VolumeSum>>24);	wrbuffer[11]=(char)(jlparam->VolumeSum>>16);
	wrbuffer[12]=(char)(jlparam->VolumeSum>>8);	wrbuffer[13]=(char)(jlparam->VolumeSum>>0);
	wrbuffer[14]=(char)(jlparam->MoneySum>>40);	wrbuffer[15]=(char)(jlparam->MoneySum>>32);
	wrbuffer[16]=(char)(jlparam->MoneySum>>24);	wrbuffer[17]=(char)(jlparam->MoneySum>>16);
	wrbuffer[18]=(char)(jlparam->MoneySum>>8);		wrbuffer[19]=(char)(jlparam->MoneySum>>0);
	wrbuffer[20]=jlparam->StopNo;
	wrbuffer[21]=jlparam->JlState;

	/*解锁*/
	jlUnlock(jlparam);

	/*计算CRC*/
	crc=crc16Get(wrbuffer, 62);
	wrbuffer[62]=(unsigned char)(crc>>8);	wrbuffer[63]=(unsigned char)(crc>>0);

	/*存储加油信息及备份*/
	framWrite(FM_ADDR_JL, offset, wrbuffer, 64);
	framWrite(FM_ADDR_JL, offset_backup, wrbuffer, 64);

	return 0;
}


/********************************************************************
*Name				:jlOilling
*Description		:计量操作，包括加油中计数及异常处理
*Input				:jlparam	计量操作数据结构
*Output			:None
*Return				:None
*History			:2014-03-03,modified by syj
*/
static void jlOilling(JlParamStruct *jlparam)
{
	unsigned int sensor_buffer[8]={0};								/*读取脉冲时的缓存*/
	unsigned long long sensor=0, volume=0, money=0;	/*实时脉冲数，金额，油量*/
	unsigned int speed_buffer[8]={0};								/*读取脉冲速度缓存*/
	unsigned long long speed=0;											/*实时流速，单位:升/分钟*/
	int istate=0, ischg=0;													/*税控禁止位状态*/
	unsigned long long speed0=0,speed1=0,speed2=0;
	
	/*判断是否在加油状态，不在加油状态时禁止计量*/
	if(JL_STATE_OILLING!=jlparam->JlState)
	{
		taskDelay(1);
		return;
	}

	/**************读取编码器脉冲,计算加油量及加油金额，计算实时速度**********************/
	if(1==((jlparam->Pulse>>0)&1))
	{	/*A1路*/
		sensor_buffer[0]=sensorRead(SENSOR_A11);	sensor_buffer[1]=sensorRead(SENSOR_A12);
		if(((sensor_buffer[0]>sensor_buffer[1])&&((sensor_buffer[0]-sensor_buffer[1])>=JL_PULSE_DIFFERENCE))||\
			((sensor_buffer[1]>sensor_buffer[0])&&((sensor_buffer[1]-sensor_buffer[0])>=JL_PULSE_DIFFERENCE)))
		{
//			jlparam->StopNo=0x01;
			/*普通机型*/
			if(jlparam->Type==0)
				jlparam->StopNo=0x01;
			/*大流量单双枪A1*/
			else if(jlparam->Type==1 || jlparam->Type==2)
				jlparam->StopNo=0x06;
		}

		speed_buffer[0]=sensorSpeedRead(SENSOR_A11);	speed_buffer[1]=sensorSpeedRead(SENSOR_A12);
	}
	if(1==((jlparam->Pulse>>1)&1))
	{	/*A2路*/
		sensor_buffer[2]=sensorRead(SENSOR_A21);	sensor_buffer[3]=sensorRead(SENSOR_A22);
		if(((sensor_buffer[2]>sensor_buffer[3])&&((sensor_buffer[2]-sensor_buffer[3])>=JL_PULSE_DIFFERENCE))||\
			((sensor_buffer[3]>sensor_buffer[2])&&((sensor_buffer[3]-sensor_buffer[2])>=JL_PULSE_DIFFERENCE)))
		{
//			jlparam->StopNo=0x01;
			/*普通机型*/
			if(jlparam->Type==0)
				jlparam->StopNo=0x01;
			/*大流量单双枪A2*/
			else if(jlparam->Type==1 || jlparam->Type==2)
				jlparam->StopNo=0x07;
		}

		speed_buffer[2]=sensorSpeedRead(SENSOR_A21);	speed_buffer[3]=sensorSpeedRead(SENSOR_A22);
	}
	if(1==((jlparam->Pulse>>2)&1))
	{	/*B1路*/
		sensor_buffer[4]=sensorRead(SENSOR_B11);	sensor_buffer[5]=sensorRead(SENSOR_B12);
		if(((sensor_buffer[4]>sensor_buffer[5])&&((sensor_buffer[4]-sensor_buffer[5])>=JL_PULSE_DIFFERENCE))||\
			((sensor_buffer[5]>sensor_buffer[4])&&((sensor_buffer[5]-sensor_buffer[4])>=JL_PULSE_DIFFERENCE)))
		{	
//			jlparam->StopNo=0x01;
			/*普通机型*/
			if(jlparam->Type==0)
				jlparam->StopNo=0x01;
			/*大流量单双枪B1*/
			else if(jlparam->Type==1 || jlparam->Type==2)
				jlparam->StopNo=0x08;
		}

		speed_buffer[4]=sensorSpeedRead(SENSOR_B11);	speed_buffer[5]=sensorSpeedRead(SENSOR_B12);
	}
	if(1==((jlparam->Pulse>>3)&1))	
	{	/*B2路*/
		sensor_buffer[6]=sensorRead(SENSOR_B21);	sensor_buffer[7]=sensorRead(SENSOR_B22);
		if(((sensor_buffer[6]>sensor_buffer[7])&&((sensor_buffer[6]-sensor_buffer[7])>=JL_PULSE_DIFFERENCE))||\
			((sensor_buffer[7]>sensor_buffer[6])&&((sensor_buffer[7]-sensor_buffer[6])>=JL_PULSE_DIFFERENCE)))
		{
//			jlparam->StopNo=0x01;
			/*普通机型*/
			if(jlparam->Type==0)
				jlparam->StopNo=0x01;
			/*大流量单双枪B1*/
			else if(jlparam->Type==1 || jlparam->Type==2)
				jlparam->StopNo=0x09;
		}

		speed_buffer[6]=sensorSpeedRead(SENSOR_B21);	speed_buffer[7]=sensorSpeedRead(SENSOR_B22);
	}

	/*************************计算有效加油数据***********************************/
	sensor=sensor_buffer[0]+sensor_buffer[1]+sensor_buffer[2]+sensor_buffer[3]+sensor_buffer[4]+sensor_buffer[5]+sensor_buffer[6]+sensor_buffer[7];
	volume=(unsigned int)((unsigned long long)sensor*(unsigned long long)jlparam->Equivalent/10000);
	if(JL_ALGORITHM_UP == jlparam->Algorithm)
	{
		money=volume*jlparam->OilPrice/100;
	}
	else if(JL_ALGORITHM_UP!=jlparam->Algorithm && (volume*jlparam->OilPrice/10%10)<JL_ROUNDING)
	{
		money=volume*jlparam->OilPrice/100;
	}
	else	if(JL_ALGORITHM_UP!=jlparam->Algorithm && (volume*jlparam->OilPrice/10%10)>=JL_ROUNDING)
	{
		money=volume*jlparam->OilPrice/100+1;
	}
	
	if((jlparam->OilVolume!=volume)&&(volume<jlparam->Shield))
	{
		/*通知税控加油数据*/
		taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);
	}
	else
	if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume<jlparam->PresetVolume))
	{
		/*锁*/
		jlLock(jlparam);
	
		/*超过屏蔽量，不超过最大允许量，未达到预置量时以实际油量为准*/
		jlparam->OilVolume=volume;	jlparam->OilMoney=money;

		/*解锁*/
		jlUnlock(jlparam);
		/*通知税控加油数据*/
		taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

		/*存储加油数据*/
		jlOilDataWrite(jlparam);
	}
	else
	if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume>=jlparam->PresetVolume)&&(volume<(jlparam->OilVolume+jlparam->OverShield)))
	{
		/*锁*/
		jlLock(jlparam);

		/*超过屏蔽量，不超过最大允许量， 到达预置量而未达到过冲屏蔽量时以预置量为实际加油量*/
		jlparam->OilVolume=jlparam->PresetVolume;	jlparam->OilMoney=jlparam->PresetMoney;

		/*解锁*/
		jlUnlock(jlparam);
		
		taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

		/*存储加油数据*/
		jlOilDataWrite(jlparam);

		jlparam->StopNo=0x02;
	}
	else
	if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume>=jlparam->PresetVolume)&&(volume>=(jlparam->OilVolume+jlparam->OverShield)))
	{
		/*锁*/
		jlLock(jlparam);
	
		/*超过屏蔽量，不超过最大允许量， 到达预置量且达到过冲屏蔽量时以实际油量为准*/
		jlparam->OilVolume=volume;	jlparam->OilMoney=money;

		/*解锁*/
		jlUnlock(jlparam);

		/*通知税控加油数据*/
		taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

		/*存储加油数据*/
		jlOilDataWrite(jlparam);

		jlparam->StopNo=0x02;
	}
	else 
	if(jlparam->OilVolume!=volume)
	{
		/*超出判断范围的值为非法值，立即结束加油*/
		jlparam->StopNo=0x02;
	}

	/************************计算实时流速，单位:0.01升/分钟************************/
	speed=speed_buffer[0]+speed_buffer[1]+speed_buffer[2]+speed_buffer[3]+speed_buffer[4]+speed_buffer[5]+speed_buffer[6]+speed_buffer[7];
	jlparam->Speed=(unsigned int)(speed*60*jlparam->Equivalent/10000);
	if(0!=jlparam->Speed)	jlparam->NoSensorTimer=0;

	/*大流量单枪A1 B1*/
	if(jlparam->Type==1)
	{
		speed0=speed_buffer[0]+speed_buffer[1];
		speed1=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed1!=0)	jlparam->bigTimeA1=0;
		
		speed0=speed_buffer[4]+speed_buffer[5];
		speed2=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed2!=0)	jlparam->bigTimeA2=0;/*bigTimeA2代替bigTimeB1的时间控制*/

		if((speed1>=jlparam->bigVolSpeed*100) && speed2==0 && (jlparam->bigTimeA2>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x12;
		else if((speed2>=jlparam->bigVolSpeed*100) && speed1==0 && (jlparam->bigTimeA1>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x10;
	}
	/*大流量双枪A1 A2*/
	if(jlparam->Type==2 && jlparam->Nozzle==JL_NOZZLE_1)
	{
		speed0=speed_buffer[0]+speed_buffer[1];
		speed1=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed1!=0)	jlparam->bigTimeA1=0;
		
		speed0=speed_buffer[2]+speed_buffer[3];
		speed2=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed2!=0)	jlparam->bigTimeA2=0;

		if((speed1>=jlparam->bigVolSpeed*100) && speed2==0 && (jlparam->bigTimeA2>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x11;
		else if((speed2>=jlparam->bigVolSpeed*100) && speed1==0 && (jlparam->bigTimeA1>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x10;
	}
	/*大流量双枪B1 B2*/
	if(jlparam->Type==2 && jlparam->Nozzle==JL_NOZZLE_2)
	{
		speed0=speed_buffer[4]+speed_buffer[5];
		speed1=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed1!=0)	jlparam->bigTimeB1=0;
		
		speed0=speed_buffer[6]+speed_buffer[7];
		speed2=(unsigned int)(speed0*60*jlparam->Equivalent/10000);
		if(speed2!=0)	jlparam->bigTimeB2=0;

		if((speed1>=jlparam->bigVolSpeed*100) && speed2==0 && (jlparam->bigTimeB2>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x13;
		else if((speed2>=jlparam->bigVolSpeed*100) && speed1==0 && (jlparam->bigTimeB1>=jlparam->bigVolTime*ONE_SECOND))
			jlparam->StopNo=0x12;
	}
	

	/****************正常状态，加油量未达到提前量时操作大阀***********************/
	if((0==jlparam->StopNo)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&((volume+jlparam->Advance)<jlparam->PresetVolume))
	{
		/*无脉冲时间未超时且大阀处于关闭状态时开启大阀*/
		if((jlparam->NoSensorTimer<jlparam->UnPulseStopTime*ONE_SECOND)&&(volume>=jlparam->ValveVolume))
		{
			/*打开A面大阀*/
			if(((1==((jlparam->Valve>>0)&1))||(1==((jlparam->Valve>>1)&1)))&&(JL_IOSTATE_OFF==JL_VALVEAA_READ))	
			{
				JL_VALVEAA_ON;
			}
			/*打开B面大阀*/
			if(((1==((jlparam->Valve>>2)&1))||(1==((jlparam->Valve>>3)&1)))&&(JL_IOSTATE_OFF==JL_VALVEBA_READ)	)
			{
				JL_VALVEBA_ON;
			}
		}
	
		/*无脉冲时间超时关闭大阀*/
		if(jlparam->NoSensorTimer>=jlparam->UnPulseStopTime*ONE_SECOND)
		{
			/*关闭A面大阀*/
			if(((1==((jlparam->Valve>>0)&1))||(1==((jlparam->Valve>>1)&1)))&&(JL_IOSTATE_ON==JL_VALVEAA_READ))	
			{
				JL_VALVEAA_OFF;	
			}
			/*关闭B面大阀*/
			if(((1==((jlparam->Valve>>2)&1))||(1==((jlparam->Valve>>3)&1)))&&(JL_IOSTATE_ON==JL_VALVEBA_READ)	)
			{
				JL_VALVEBA_OFF;
			}
		}
	}
	/*达到预置量时关闭大阀*/
	else if((volume+jlparam->Advance)>=jlparam->PresetVolume)
	{
		/*关闭A面大阀*/
		if(((1==((jlparam->Valve>>0)&1))||(1==((jlparam->Valve>>1)&1)))&&(JL_IOSTATE_ON==JL_VALVEAA_READ))	
		{
			JL_VALVEAA_OFF;	
		}
		/*关闭B面大阀*/
		if(((1==((jlparam->Valve>>2)&1))||(1==((jlparam->Valve>>3)&1)))&&(JL_IOSTATE_ON==JL_VALVEBA_READ)	)
		{
			JL_VALVEBA_OFF;
		}
	}

	/***************************税控禁止时停机************************************/
	istate=spi1PumpPermitRead(jlparam->DevPumpPermit, (char*)&ischg);
	if(0!=istate){
		jlparam->StopNo=0x03;
	}

	/***************************无脉冲时间超时停机************************************/
	if(jlparam->NoSensorTimer>=jlparam->UnPulseTime*ONE_SECOND){
		jlparam->StopNo=0x04;
	}

	/***************************主机掉电时停机***********************************/
	if(0!=powerStateRead()){
		jlparam->StopNo=0x05;
	}

	/*************************加油完成或异常状态时停机操作*****************/
	if(0!=jlparam->StopNo)
	{
		/*关闭A面大阀及小阀*/
		if((1==((jlparam->Valve>>0)&1))||(1==((jlparam->Valve>>1)&1)))
		{
			if(JL_IOSTATE_ON==JL_VALVEAA_READ)		JL_VALVEAA_OFF;
			if(JL_IOSTATE_ON==JL_VALVEAB_READ)		JL_VALVEAB_OFF;
		}
		/*关闭B面大阀及小阀*/
		if((1==((jlparam->Valve>>2)&1))||(1==((jlparam->Valve>>3)&1)))
		{
			if(JL_IOSTATE_ON==JL_VALVEBA_READ)		JL_VALVEBA_OFF;
			if(JL_IOSTATE_ON==JL_VALVEBB_READ)		JL_VALVEBB_OFF;
		}
		/*关闭A1电机*/
		if((1==((jlparam->Motor>>0)&1))&&(JL_IOSTATE_ON==JL_MOTORA1_READ)	)	JL_MOTORA1_OFF;
		/*关闭A2电机*/
		if((1==((jlparam->Motor>>1)&1))&&(JL_IOSTATE_ON==JL_MOTORA2_READ)	)	JL_MOTORA2_OFF;
		/*关闭B1电机*/
		if((1==((jlparam->Motor>>2)&1))&&(JL_IOSTATE_ON==JL_MOTORB1_READ)	)	JL_MOTORB1_OFF;
		/*关闭B2电机*/
		if((1==((jlparam->Motor>>3)&1))&&(JL_IOSTATE_ON==JL_MOTORB2_READ)	)	JL_MOTORB2_OFF;
	}

	return;
}


/********************************************************************
*Name				:jlOilTask
*Description		:计量任务
*Input				:nozzelNum			枪选0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:None
*Return				:None
*History			:2014-03-03,modified by syj
*/
static void jlOilTask(int nozzle)
{
	JlParamStruct *jlparam=NULL;

	if(JL_NOZZLE_1==nozzle)			jlparam=&JlParamA1;
	else if(JL_NOZZLE_2==nozzle)	jlparam=&JlParamB1;
	else												return;
	

	FOREVER
	{
		jlOilling(jlparam);

		taskDelay(1);
	}

	return;
}


/********************************************************************
*Name				:jlOilStart
*Description		:计量加油启动操作
*Input				:nozzle					枪选0=1号枪(A1枪)；1=2号枪(B1枪)
*						:preset_value		预置量，任意加油时为最大金额
*						:preset_price			预支单价
*						:preset_mode		预置方式0=任意；1=定升数；2=定金额
*Output			:None
*Return				:0x00=成功；其它=失败
*						:0x01=枪选非法；
*						:0x02=单价不一致；
*						:0x03=计量单价非法；
*						:0x04=当量非法；
*						:0x05=无脉冲超时时间非法；
*						:0x06=提前量非法；
*						:0x07=屏蔽量非法；
*						:0x08=过冲屏蔽量非法；
*						:0x09=大阀开启延迟出油量非法；
*						:0x10=计量机型非法；
*						:0x11=预置方式非法；
*						:0x12=预置量非法；
*						:0x13=电源状态非法；
*						:0x14=税控启动失败；
*						:0x15=缺一路脉冲超次；
*						:0x16=零加油超次；
*						:0x17=无脉冲停机超次；
*						:0x18=缺一路编码器超次；
*						:该值会对应 jlStartFiledReson 中停机原因，故添加时按照BCD码顺序添加，并在jlStartFiledReson中添加对应信息
*History			:2014-03-03,modified by syj
*/
int jlOilStart(int nozzle, unsigned int preset, unsigned int preset_price, unsigned int preset_mode)
{
	JlParamStruct *jlparam=NULL;
	char ischg=0;
	int i=0, istate=0, istate2=0;
	int tmp = 0, maxmoney = JL_MONEY_MAX, maxvolume = JL_VOLUME_MAX;
	unsigned long long money=0, volume=0, price=0, mode=0;
	unsigned char ibuffer[64] = {0};
	unsigned char ClearZero=0;

	/*@1	判断枪选*/
	if(0==nozzle)			jlparam=&JlParamA1;
	else if(1==nozzle)	jlparam=&JlParamB1;
	else							{return 0x01;}

	if(0==nozzle)
	{
		jlparam->bigTimeA1=0;
		jlparam->bigTimeA2=0;
	}
	else
	{
		jlparam->bigTimeB1=0;
		jlparam->bigTimeB2=0;
	}	
	
	/*判断计量状态:空闲继续执行；加油中则直接返回申请成功*/
	if(JL_STATE_OILLING==jlparam->JlState)
	{
		return 0;
	}

	/*有加油启动则大屏显示全8*/
	taxDsp(jlparam->Nozzle, "\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38\x38", TAX_POINT_NO);

	/*@2	判断单价是否一致*/
	if(jlparam->Price!=preset_price)
	{
		return 0x02;
	}

	/*@3	判断单价合法性*/
	if(preset_price<JL_PRICE_MIN || preset_price>JL_PRICE_MAX)
	{
		return 0x03;
	}

	/*@4	判断当量合法性，有效范围:1000~19999*/
	if(jlparam->Equivalent<JL_EQUIVALENT_MIN || jlparam->Equivalent>JL_EQUIVALENT_MAX)
	{
		return 0x04;
	}

	/*@5	判断无脉冲超时时间合法性，有效范围:30~180秒*/
	if(jlparam->UnPulseTime<JL_UNPULSE_TIME_MIN || jlparam->UnPulseTime>JL_UNPULSE_TIME_MAX)
	{
		return 0x05;
	}

	/*@6	判断提前量合法性，有效范围:0.05~9.99升*/
	if(jlparam->Advance<JL_ADVANCE_MIN || jlparam->Advance>JL_ADVANCE_MAX)
	{
		return 0x06;
	}

	/*@7	判断屏蔽量合法性，有效范围:0.01~0.30升*/
	if(jlparam->Shield<1 || jlparam->Shield>30)
	{
		return 0x07;
	}

	/*@8	判断过冲屏蔽量合法性，有效范围:0.05~0.99升*/
	if( jlparam->OverShield<5 || jlparam->OverShield>99)
	{
		return 0x08;
	}

	/*@9	判断大阀开启延迟出油量合法性，有效范围:0~0.30升*/
	if(jlparam->ValveVolume<0 || jlparam->ValveVolume>30)
	{
		return 0x09;
	}

	/*@10	判断计量机型合法性，暂时只支持普通机型Type=0*/
//	if(0!=jlparam->Type)
	if(jlparam->Type>2)
	{
		return 0x10;
	}

	/*@11	判断预置方式合法性*/
	if(0!=preset_mode && 1!=preset_mode && 2!=preset_mode)
	{
		return 0x11;
	}

	/*计算预置量*/
	if(1 != preset_mode && 2 != preset_mode)
	{
		if(preset < JL_MONEY_MAX)	maxmoney=preset;
		else											maxmoney=JL_MONEY_MAX;
	}
	ibuffer[0] = (char)(maxmoney>>24);			ibuffer[1] = (char)(maxmoney>>16);
	ibuffer[2] = (char)(maxmoney>>8);				ibuffer[3] = (char)(maxmoney>>0);
	ibuffer[4] = (char)(maxvolume>>24);			ibuffer[5] = (char)(maxvolume>>16);
	ibuffer[6] = (char)(maxvolume>>8);				ibuffer[7] = (char)(maxvolume>>0);
	if(1 == preset_mode)		/*定升数*/
	{
		ibuffer[8] = (char)(0>>24);				ibuffer[9] = (char)(0>>16);
		ibuffer[10] = (char)(0>>8);				ibuffer[11] = (char)(0>>0);
		ibuffer[12] = (char)(preset>>24);	ibuffer[13] = (char)(preset>>16);
		ibuffer[14] = (char)(preset>>8);		ibuffer[15] = (char)(preset>>0);
	}
	else if(2 == preset_mode)		/*定金额*/
	{
		ibuffer[8] = (char)(preset>>24);		ibuffer[9] = (char)(preset>>16);
		ibuffer[10] = (char)(preset>>8);		ibuffer[11] = (char)(preset>>0);
		ibuffer[12] = (char)(0>>24);				ibuffer[13] = (char)(0>>16);
		ibuffer[14] = (char)(0>>8);				ibuffer[15] = (char)(0>>0);
	}
	else											/*升数*/
	{
		ibuffer[8] = (char)(0>>24);		ibuffer[9] = (char)(0>>16);
		ibuffer[10] = (char)(0>>8);		ibuffer[11] = (char)(0>>0);
		ibuffer[12] = (char)(0>>24);		ibuffer[13] = (char)(0>>16);
		ibuffer[14] = (char)(0>>8);		ibuffer[15] = (char)(0>>0);
	}
	ibuffer[16] = (char)(preset_price>>24);				ibuffer[17] = (char)(preset_price>>16);
	ibuffer[18] = (char)(preset_price>>8);				ibuffer[19] = (char)(preset_price>>0);
	ibuffer[20] = preset_mode;
	jlPresetAmountCalculate(nozzle, ibuffer);
	money = (ibuffer[8]<<24)|(ibuffer[9]<<16)|(ibuffer[10]<<8)|(ibuffer[11]<<0);
	volume = (ibuffer[12]<<24)|(ibuffer[13]<<16)|(ibuffer[14]<<8)|(ibuffer[15]<<0);
	price = preset_price;
	mode = ibuffer[20];

#if 0
	/*任意加油*/
	if(1 != preset_mode && 2 != preset_mode)
	{
		price=preset_price;	mode=preset_mode;	tmp = 2;
		if(preset<JL_MONEY_MAX)	maxmoney=preset;
		else										maxmoney=JL_MONEY_MAX;
	}
	/*预置升数*/
	if(1 == preset_mode || 1 == tmp)			
	{
		if(1 == tmp)		volume = maxvolume;
		else					volume = preset;	
		price=preset_price;	mode=preset_mode;
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			money=volume*price/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (volume*price/10%10)<JL_ROUNDING)
		{
			money = volume*price/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (volume*price/10%10)>=JL_ROUNDING)
		{
			money = volume*price/100 + 1;
		}
	}
	/*预置金额*/
	if(2 == preset_mode || 2 == tmp)			
	{
		if(2 == tmp)		money = maxmoney;
		else					money = preset;
		price=preset_price;	mode=preset_mode;
		if(money >= JL_MONEY_MAX)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100) > 0)
		{
			volume=money*100/price+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100) == 0)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10) >= JL_ROUNDING)
		{
			volume=money*100/price + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10) < JL_ROUNDING)
		{
			volume=money*100/price;
		}
	}
#endif

	//printf("[%s][%d][volume=%d][money=%d][price=%d][mode=%d].\n", __FUNCTION__, __LINE__, (unsigned int)volume, (unsigned int)money, (unsigned int)price, (unsigned int)mode);

	/*@12	判断预置量合法性*/
	if(volume<JL_VOLUME_MIN || volume>JL_VOLUME_MAX || money<JL_MONEY_MIN || money>JL_MONEY_MAX)
	{
		return 0x12;
	}

	/*@13	判断电源状态是否正常*/
	if(0!=powerStateRead())
	{
		return 0x13;
	}
			

	/*@14	税控启动加油*/
	for(i=0; i<3;i++)
	{
		istate=taxOilStart(jlparam->Nozzle);
		if(istate==0&&ClearZero==0)
			{
				ClearZero=1;
				/*脉冲累计清零*/
				if(1==((jlparam->Pulse>>0)&1))	{sensorFlush(SENSOR_A11);	sensorFlush(SENSOR_A12);}
				if(1==((jlparam->Pulse>>1)&1))	{sensorFlush(SENSOR_A21);	sensorFlush(SENSOR_A22);}
				if(1==((jlparam->Pulse>>2)&1))	{sensorFlush(SENSOR_B11);	sensorFlush(SENSOR_B12);}
				if(1==((jlparam->Pulse>>3)&1))	{sensorFlush(SENSOR_B21);	sensorFlush(SENSOR_B22);}
			}
		istate2=spi1PumpPermitRead(jlparam->DevPumpPermit, &ischg);
		if(0==istate && 0==istate2)
		{
			/*脉冲累计清零
			if(1==((jlparam->Pulse>>0)&1))	{sensorFlush(SENSOR_A11);	sensorFlush(SENSOR_A12);}
			if(1==((jlparam->Pulse>>1)&1))	{sensorFlush(SENSOR_A21);	sensorFlush(SENSOR_A22);}
			if(1==((jlparam->Pulse>>2)&1))	{sensorFlush(SENSOR_B11);	sensorFlush(SENSOR_B12);}
			if(1==((jlparam->Pulse>>3)&1))	{sensorFlush(SENSOR_B21);	sensorFlush(SENSOR_B22);}
			*/

			/*电机及电磁阀操作，同一面的电机不可同时开启*/
			/*选择A面A1阀，打开A面小阀*/
			if(1==((jlparam->Valve>>0)&1))
			{
				JL_VALVEA1_SEL;	JL_VALVEAB_ON;
			}
			/*选择A面A2阀，打开A面小阀*/
			if(1==((jlparam->Valve>>1)&1))
			{
				JL_VALVEA2_SEL;	JL_VALVEAB_ON;
			}
			/*选择B面B1阀，打开B面小阀*/
			if(1==((jlparam->Valve>>2)&1))
			{
				JL_VALVEB1_SEL;	JL_VALVEBB_ON;									
			}
			/*选择B面B2阀，打开B面小阀*/
			if(1==((jlparam->Valve>>3)&1))
			{
				JL_VALVEB2_SEL;	JL_VALVEBB_ON;											
			}
			/*关闭A2电机，打开A1电机*/
			if(1==((jlparam->Motor>>0)&1))	
			{
				JL_MOTORA2_OFF;	JL_MOTORA1_ON;
#if 0
				/*普通机型:关闭A2电机，打开A1电机*/
				if(jlparam->Type==0)
					{JL_MOTORA2_OFF;	JL_MOTORA1_ON;}
				/*单枪大流量:关闭A2电机，打开A1电机*/
				else if(jlparam->Type==1)
					{JL_MOTORA2_OFF;	JL_MOTORA1_ON;}
				/*双枪大流量:打开A2电机，打开A1电机*/
				else if(jlparam->Type==1)
					{JL_MOTORA2_ON;	JL_MOTORA1_ON;}
#endif
			}
			/*关闭A1电机，打开A2电机*/
			if(1==((jlparam->Motor>>1)&1))
			{
				JL_MOTORA1_OFF;	JL_MOTORA2_ON;
#if 0
				/*普通机型:关闭A1电机，打开A2电机*/
				if(jlparam->Type==0)
					{JL_MOTORA1_OFF;	JL_MOTORA2_ON;}
				/*单枪大流量:关闭A1电机，打开A2电机*/
				else if(jlparam->Type==1)
					{JL_MOTORA1_OFF;	JL_MOTORA2_ON;}
				/*双枪大流量:打开A2电机，打开A1电机*/
				else if(jlparam->Type==1)
					{JL_MOTORA2_ON;	JL_MOTORA1_ON;}
#endif
			}
			/*关闭B2电机，打开B1电机*/
			if(1==((jlparam->Motor>>2)&1))	
			{
				JL_MOTORB2_OFF;	JL_MOTORB1_ON;
#if 0
				/*普通机型:关闭B2电机，打开B1电机*/
				if(jlparam->Type==0)
					{JL_MOTORB2_OFF;	JL_MOTORB1_ON;}
				/*单枪大流量:关闭B2电机，打开B1电机*/
				else if(jlparam->Type==1)
					{JL_MOTORB2_OFF;	JL_MOTORB1_ON;}
				/*双枪大流量:打开B2电机，打开B1电机*/
				else if(jlparam->Type==1)
					{JL_MOTORB2_ON;	JL_MOTORB1_ON;}
#endif
			}
			/*关闭B1电机，打开B2电机*/
			if(1==((jlparam->Motor>>3)&1))	
			{
				JL_MOTORB1_OFF;	JL_MOTORB2_ON;
#if 0
				/*普通机型:关闭B1电机，打开B2电机*/
				if(jlparam->Type==0)
					{JL_MOTORB1_OFF;	JL_MOTORB2_ON;}
				/*单枪大流量:关闭B1电机，打开B2电机*/
				else if(jlparam->Type==1)
					{JL_MOTORB1_OFF;	JL_MOTORB2_ON;}
				/*双枪大流量:打开B1电机，打开B2电机*/
				else if(jlparam->Type==1)
					{JL_MOTORB1_ON;	JL_MOTORB2_ON;}
#endif
			}


			/*锁*/
			jlLock(jlparam);
			
			/*本次加油数据初始化*/
			jlparam->PresetMode=mode;			
			jlparam->PresetMoney=money;	jlparam->PresetVolume=volume;		
			jlparam->OilVolume=0;	jlparam->OilMoney=0;	jlparam->OilPrice=price;						
			jlparam->StopNo=0;						
			jlparam->NoSensorTimer=0;		
			jlparam->JlState=JL_STATE_OILLING;	

			/*解锁*/
			jlUnlock(jlparam);

			/*存储加油信息*/
			jlOilDataWrite(jlparam);

			/*通知税控0加油数据，使大屏显示归零*/
			taxOillingData(jlparam->Nozzle, 0, 0, jlparam->OilPrice);

			break;
		}
		else 
		{
			/*失败延时500ms*/
			taskDelay(500*ONE_MILLI_SECOND);
		}
	}

	/*@14	判断加油启动是否成功*/
	if(JL_STATE_OILLING!=jlparam->JlState)
	{
		return 0x14;
	}

	return 0;
}


/********************************************************************
*Name				:jlOilRead
*Description		:计量加油数据读取
*Input				:nozzle			0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:money		当次金额
*						:volume		当次油量
*						:price			当次单价
*						:stop_no		当次停机代码	0x00=正常
																					0x01=缺一路脉冲，停机
																					0x02=达到预置量，停机
																					0x03=税控禁止，停机
																					0x04=无脉冲超时，停机
																					0x05=主机掉电，停机
*Return			:0=成功；其它=失败；
*History			:2014-03-03,modified by syj
*/
int jlOilRead(int nozzle, unsigned int *money, unsigned int *volume, unsigned int *price, unsigned char *stop_no)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			jlparam=&JlParamA1;
	else if(1==nozzle)	jlparam=&JlParamB1;
	else							{return ERROR;}

	/*锁*/
	jlLock(jlparam);
			
	/*加油数据赋值*/
	*money=jlparam->OilMoney;	*volume=jlparam->OilVolume;	*price=jlparam->OilPrice;		*stop_no=jlparam->StopNo;	

	/*解锁*/
	jlUnlock(jlparam);

	return 0;
}


/********************************************************************
*Name				:jlOilFinish
*Description		:计量加油结束
*Input				:nozzle					0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:money_sum		总累金额
*						:volume_sum		总累油量
*						:pmoney				当次加油金额
*						:pvolume				当次加油油量
*						:pprice					当次加油单价
*						:stop_no				当次加油停机代码
*Return				:0=成功；其它=失败
*History			:2014-03-03,modified by syj
*/
int jlOilFinish(int nozzle, unsigned long long *money_sum, unsigned long long *volume_sum, unsigned int *pmoney, unsigned int *pvolume, unsigned int *pprice, unsigned char *stop_no)
{
	JlParamStruct *jlparam=NULL;
	unsigned int sensor_buffer[8]={0};						//读取脉冲时的缓存
	unsigned long long sensor=0, volume=0, money=0;		//当前脉冲数，金额，油量
	unsigned char data_buffer[4]={0}, sum_buffer[6]={0};	
	long long bcd_data=0;
	int i=0, istate=0, istate2=0;
	unsigned char speed_buffer[8]={0};
	long long speed=0;												/*当前流速*/
	unsigned int jltimer=0;											/*延时读取加油量定时器*/
	unsigned int sensor_speed_buffer[8]={0};			/*读取脉冲速度的缓存*/
	long long sum=0;

	/*判断枪选*/
	if(0==nozzle)			jlparam=&JlParamA1;
	else if(1==nozzle)	jlparam=&JlParamB1;
	else							return ERROR;

	/*计量空闲状态时直接返回成功和当次加油数据*/
	if(JL_STATE_IDLE==jlparam->JlState)
	{
		/*锁*/
		jlLock(jlparam);

		*money_sum=jlparam->MoneySum;	*volume_sum=jlparam->VolumeSum;
		*pmoney=jlparam->OilMoney;	*pvolume=jlparam->OilVolume;		*pprice=jlparam->OilPrice;	*stop_no=jlparam->StopNo;	

		/*解锁*/
		jlUnlock(jlparam);

		return 0;
	}

	/*计量状态更改为加油结束中*/
	jlparam->JlState=JL_STATE_FINISHING;

	/*关闭A面大阀及小阀*/
	if((1==((jlparam->Valve>>0)&1))||(1==((jlparam->Valve>>1)&1)))
	{	
		if(JL_IOSTATE_ON==JL_VALVEAA_READ)		JL_VALVEAA_OFF;
		if(JL_IOSTATE_ON==JL_VALVEAB_READ)		JL_VALVEAB_OFF;
	}
	/*关闭B面大阀及小阀*/
	if((1==((jlparam->Valve>>2)&1))||(1==((jlparam->Valve>>3)&1)))
	{
		if(JL_IOSTATE_ON==JL_VALVEBA_READ)		JL_VALVEBA_OFF;
		if(JL_IOSTATE_ON==JL_VALVEBB_READ)		JL_VALVEBB_OFF;
	}
	/*关闭A1电机*/
	if((1==((jlparam->Motor>>0)&1))&&(JL_IOSTATE_ON==JL_MOTORA1_READ)	)	JL_MOTORA1_OFF;
	/*关闭A2电机*/
	if((1==((jlparam->Motor>>1)&1))&&(JL_IOSTATE_ON==JL_MOTORA2_READ)	)	JL_MOTORA2_OFF;
	/*关闭B1电机*/
	if((1==((jlparam->Motor>>2)&1))&&(JL_IOSTATE_ON==JL_MOTORB1_READ)	)	JL_MOTORB1_OFF;
	/*关闭B2电机*/
	if((1==((jlparam->Motor>>3)&1))&&(JL_IOSTATE_ON==JL_MOTORB2_READ)	)	JL_MOTORB2_OFF;
	
	while(1)
	{
		/**************读取编码器脉冲,计算加油量及加油金额，计算实时速度**********************/
		if(1==((jlparam->Pulse>>0)&1))
		{	/*A1路*/
			sensor_buffer[0]=sensorRead(SENSOR_A11);	sensor_buffer[1]=sensorRead(SENSOR_A12);
			if(((sensor_buffer[0]>sensor_buffer[1])&&((sensor_buffer[0]-sensor_buffer[1])>=JL_PULSE_DIFFERENCE))||\
				((sensor_buffer[1]>sensor_buffer[0])&&((sensor_buffer[1]-sensor_buffer[0])>=JL_PULSE_DIFFERENCE)))
			{
//				if(0==jlparam->StopNo)	jlparam->StopNo=0x01;
				if(0==jlparam->StopNo)
				{
					/*普通机型*/
					if(jlparam->Type==0)
						jlparam->StopNo=0x01;
					/*大流量单双枪A1*/
					else if(jlparam->Type==1 || jlparam->Type==2)
						jlparam->StopNo=0x06;
				}
			}

			speed_buffer[0]=sensorSpeedRead(SENSOR_A11);	speed_buffer[1]=sensorSpeedRead(SENSOR_A12);
		}
		if(1==((jlparam->Pulse>>1)&1))	
		{	/*A2路*/
			sensor_buffer[2]=sensorRead(SENSOR_A21);	sensor_buffer[3]=sensorRead(SENSOR_A22);
			if(((sensor_buffer[2]>sensor_buffer[3])&&((sensor_buffer[2]-sensor_buffer[3])>=JL_PULSE_DIFFERENCE))||\
				((sensor_buffer[3]>sensor_buffer[2])&&((sensor_buffer[3]-sensor_buffer[2])>=JL_PULSE_DIFFERENCE)))
			{
//				if(0==jlparam->StopNo)	jlparam->StopNo=0x01;
				if(0==jlparam->StopNo)
				{
					/*普通机型*/
					if(jlparam->Type==0)
						jlparam->StopNo=0x01;
					/*大流量单双枪A2*/
					else if(jlparam->Type==1 || jlparam->Type==2)
						jlparam->StopNo=0x07;
				}
			}

			speed_buffer[2]=sensorSpeedRead(SENSOR_A21);	speed_buffer[3]=sensorSpeedRead(SENSOR_A22);
		}
		if(1==((jlparam->Pulse>>2)&1))	
		{	/*B1路*/
			sensor_buffer[4]=sensorRead(SENSOR_B11);	sensor_buffer[5]=sensorRead(SENSOR_B12);
			if(((sensor_buffer[4]>sensor_buffer[5])&&((sensor_buffer[4]-sensor_buffer[5])>=JL_PULSE_DIFFERENCE))||\
				((sensor_buffer[5]>sensor_buffer[4])&&((sensor_buffer[5]-sensor_buffer[4])>=JL_PULSE_DIFFERENCE)))
			{
//				if(0==jlparam->StopNo)	jlparam->StopNo=0x01;
				if(0==jlparam->StopNo)
				{
					/*普通机型*/
					if(jlparam->Type==0)
						jlparam->StopNo=0x01;
					/*大流量单双枪B1*/
					else if(jlparam->Type==1 || jlparam->Type==2)
						jlparam->StopNo=0x08;
				}
			}

			speed_buffer[4]=sensorSpeedRead(SENSOR_B11);	speed_buffer[5]=sensorSpeedRead(SENSOR_B12);
		}
		if(1==((jlparam->Pulse>>3)&1))	
		{	/*B2路*/
			sensor_buffer[6]=sensorRead(SENSOR_B21);	sensor_buffer[7]=sensorRead(SENSOR_B22);
			if(((sensor_buffer[6]>sensor_buffer[7])&&((sensor_buffer[6]-sensor_buffer[7])>=JL_PULSE_DIFFERENCE))||\
				((sensor_buffer[7]>sensor_buffer[6])&&((sensor_buffer[7]-sensor_buffer[6])>=JL_PULSE_DIFFERENCE)))
			{
//				if(0==jlparam->StopNo)	jlparam->StopNo=0x01;
				if(0==jlparam->StopNo)
				{
					/*普通机型*/
					if(jlparam->Type==0)
						jlparam->StopNo=0x01;
					/*大流量单双枪B1*/
					else if(jlparam->Type==1 || jlparam->Type==2)
						jlparam->StopNo=0x09;
				}
			}

			speed_buffer[6]=sensorSpeedRead(SENSOR_B21);	speed_buffer[7]=sensorSpeedRead(SENSOR_B22);
		}

		/*************************计算有效加油数据***********************************/
		sensor=sensor_buffer[0]+sensor_buffer[1]+sensor_buffer[2]+sensor_buffer[3]+sensor_buffer[4]+sensor_buffer[5]+sensor_buffer[6]+sensor_buffer[7];
		volume=(unsigned int)((unsigned long long)sensor*(unsigned long long)jlparam->Equivalent/10000);
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			money=volume*jlparam->OilPrice/100;
		}
		else if(JL_ALGORITHM_UP!=jlparam->Algorithm && (volume*jlparam->OilPrice/10%10)<JL_ROUNDING)
		{
			money=volume*jlparam->OilPrice/100;
		}
		else if(JL_ALGORITHM_UP!=jlparam->Algorithm && (volume*jlparam->OilPrice/10%10)>=JL_ROUNDING)
		{
			money=volume*jlparam->OilPrice/100+1;
		}
		
		if((jlparam->OilVolume!=volume)&&(volume<jlparam->Shield))
		{
			/*未达到屏蔽量时不存储加油数据*/
		}
		else
		if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume<jlparam->PresetVolume))
		{
			/*锁*/
			jlLock(jlparam);
		
			/*超过屏蔽量，不超过最大允许量，未达到预置量时以实际油量为准*/
			jlparam->OilVolume=volume;	jlparam->OilMoney=money;

			/*解锁*/
			jlUnlock(jlparam);	

			taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

			jlOilDataWrite(jlparam);
		}
		else
		if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume>=jlparam->PresetVolume)&&(volume<(jlparam->OilVolume+jlparam->OverShield)))
		{
			/*锁*/
			jlLock(jlparam);
		
			/*超过屏蔽量，不超过最大允许量， 到达预置量而未达到过冲屏蔽量时以预置量为实际加油量*/
			jlparam->OilVolume=jlparam->PresetVolume;	jlparam->OilMoney=jlparam->PresetMoney;

			/*解锁*/
			jlUnlock(jlparam);		
			
			taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

			jlOilDataWrite(jlparam);
		}
		else
		if((jlparam->OilVolume!=volume)&&(volume>=jlparam->Shield)&&(volume<=JL_VOLUME_MAX)&&(money<=JL_MONEY_MAX)&&(volume>=jlparam->PresetVolume)&&(volume>=(jlparam->OilVolume+jlparam->OverShield)))
		{
			/*锁*/
			jlLock(jlparam);
		
			/*超过屏蔽量，不超过最大允许量， 到达预置量且达到过冲屏蔽量时以实际油量为准*/
			jlparam->OilVolume=volume;	jlparam->OilMoney=money;

			/*解锁*/
			jlUnlock(jlparam);
			
			taxOillingData(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney,	jlparam->OilPrice);

			jlOilDataWrite(jlparam);
		}

		/************************计算实时流速，单位:0.01升/分钟************************/
		speed=speed_buffer[0]+speed_buffer[1]+speed_buffer[2]+speed_buffer[3]+speed_buffer[4]+speed_buffer[5]+speed_buffer[6]+speed_buffer[7];
		jlparam->Speed=(unsigned int)(speed*60*jlparam->Equivalent/10000);

		/************************读延时1秒或流速为零时结束延时*********************/
		if((0==jlparam->Speed)||(jltimer>=ONE_SECOND))	break;
		else
		{
			jltimer++;	taskDelay(ONE_MILLI_SECOND);
		}
	}
	
	/*结束税控加油状态*/
	for(i=0; i<3; i++)
	{
		if(0==taxOilfinish(jlparam->Nozzle, jlparam->OilVolume, jlparam->OilMoney, jlparam->OilPrice))
		{
			/*锁*/
			jlLock(jlparam);

/*
printf("%s:%d=", __FUNCTION__, __LINE__);
printf("jlparam->OilMoney=%x;	jlparam->OilVolume=%x;		jlparam->OilPrice=%x;		jlparam->OilMoney=%x;		jlparam->VolumeSum=%x;\n", jlparam->OilMoney, jlparam->OilVolume, jlparam->OilPrice, jlparam->OilMoney, jlparam->VolumeSum);
printf("*pmoney=%x;	*pvolume=%x;		*pprice=%x;		*money_sum=%x;		*volume_sum=%x;\n", *pmoney, *pvolume, *pprice, *money_sum, *volume_sum);
//*/
		
			/*计算,保存总累*/
			jlparam->VolumeSum=jlparam->VolumeSum+jlparam->OilVolume;
			jlparam->MoneySum=jlparam->MoneySum+jlparam->OilMoney;

			/*计量状态*/
			jlparam->JlState=JL_STATE_IDLE;

			*money_sum=jlparam->MoneySum;	*volume_sum=jlparam->VolumeSum;
			*pmoney=jlparam->OilMoney;	*pvolume=jlparam->OilVolume;		*pprice=jlparam->OilPrice;	*stop_no=jlparam->StopNo;	

/*
printf("%s:%d=", __FUNCTION__, __LINE__);
printf("jlparam->OilMoney=%x;	jlparam->OilVolume=%x;		jlparam->OilPrice=%x;		jlparam->OilMoney=%x;		jlparam->VolumeSum=%x;\n", jlparam->OilMoney, jlparam->OilVolume, jlparam->OilPrice, jlparam->OilMoney, jlparam->VolumeSum);
printf("*pmoney=%x;	*pvolume=%x;		*pprice=%x;		*money_sum=%x;		*volume_sum=%x;\n", *pmoney, *pvolume, *pprice, *money_sum, *volume_sum);
//*/
			/*解锁*/
			jlUnlock(jlparam);

			jlOilDataWrite(jlparam);

			break;
		}
		else
		{
			taskDelay(500*sysClkRateGet()/1000);
		}
	}

	return ((JL_STATE_IDLE==jlparam->JlState)?0:ERROR);
}


/********************************************************************
*Name				:jlOilCZ
*Description		:获取计量软件版本
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:handle		0=金额凑整；1=油量凑整
*Output			:buffer		输出缓存
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlOilCZ(int nozzle, int handle)
{
	JlParamStruct *jlparam=NULL;
	unsigned long long money=0, volume=0, price=0, mode=0, premoney=0, prevolume=0;
	unsigned int sensor[8]={0}, sensor_speed=0, speed=0;

	/*判断枪选*/
	if(0==nozzle)			jlparam=&JlParamA1;
	else if(1==nozzle)	jlparam=&JlParamB1;
	else							return ERROR;

	/*计算所操作的编码器流速，单位:0.01升/分钟*/
	if(1==((jlparam->Pulse>>0)&1))
	{	/*A1路*/
		sensor[0]=sensorSpeedRead(SENSOR_A11);	sensor[1]=sensorSpeedRead(SENSOR_A12);
	}
	if(1==((jlparam->Pulse>>1)&1))	
	{	/*A2路*/
		sensor[2]=sensorSpeedRead(SENSOR_A21);	sensor[3]=sensorSpeedRead(SENSOR_A22);
	}
	if(1==((jlparam->Pulse>>2)&1))	
	{	/*B1路*/
		sensor[4]=sensorSpeedRead(SENSOR_B11);	sensor[5]=sensorSpeedRead(SENSOR_B12);
	}
	if(1==((jlparam->Pulse>>3)&1))	
	{	/*B2路*/
		sensor[6]=sensorSpeedRead(SENSOR_B21);	sensor[7]=sensorSpeedRead(SENSOR_B22);
	}
	sensor_speed=sensor[0]+sensor[1]+sensor[2]+sensor[3]+sensor[4]+sensor[5]+sensor[6]+sensor[7];
	speed=(unsigned int)((unsigned long long)sensor_speed*60*jlparam->Equivalent/10000);

	/*判断流速高时返回错误*/
	if(speed>=JL_SPEED_HI)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*重新计算预置量，当前加油量累加一元或一升
	*金额凑整，如果凑整后的金额是最大允许金额，则无论以何种算法应舍弃第3位小数及之后的数据，这是因为税控的限制；
	*金额凑整，新算法(油量舍金额入的算法)以金额为预置基础，升数如果第3,4位小数非0则第2位小数累加；
	*金额凑整，旧算法(四舍五入算法)以金额为预置基础，升数第3位小数四舍五入；
	*升数凑整，新算法(油量舍金额入的算法)以升数为预置基础，金额舍弃第3位小数及之后的数据；
	*升数凑整，旧算法(四舍五入算法)以升数为预置基础，
	*/
	premoney=jlparam->PresetMoney;	prevolume=jlparam->PresetVolume;
	money=jlparam->OilMoney;	volume=jlparam->OilVolume;	price=jlparam->OilPrice;
	if(0==handle)	/*金额凑整*/
	{
		mode=2;
		money=(money+100)/100*100;
		if(money >= JL_MONEY_MAX)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100)>0)
		{
			volume=money*100/price+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100)==0)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10)<JL_ROUNDING)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10)>=JL_ROUNDING)
		{
			volume=money*100/price+1;
		}
	}
	else					/*升数凑整*/
	{
		mode=1;
		volume=(money+100)/100*100;
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			money=volume*price/100;
		}
		else
		{
			if((volume*price/10%10)<JL_ROUNDING)	money=volume*price/100;
			else																money=volume*price/100+1;
		}
	}

	/*判断凑整后预置量是否合法*/
	if(	money>premoney || volume>prevolume ||\
		volume<JL_VOLUME_MIN || volume>JL_VOLUME_MAX ||\
		money<JL_MONEY_MIN || money>JL_MONEY_MAX)
	{
		/*解锁*/
		jlUnlock(jlparam);
		return ERROR;
	}

	/*更新预置数据*/
	jlparam->PresetMoney=money;	jlparam->PresetVolume=volume;	jlparam->PresetMode=mode;

	/*解锁*/
	jlUnlock(jlparam);

	/*操作对象为A面，关闭A面大阀*/
	if(0!=(jlparam->Valve&0x03) && JL_IOSTATE_ON==JL_VALVEAA_READ)
	{
		JL_VALVEAA_OFF;	
	}
	/*操作对象为B面，关闭B面大阀*/
	if(0!=(jlparam->Valve&0x0c) && JL_IOSTATE_ON==JL_VALVEBA_READ)	
	{
		JL_VALVEBA_OFF;
	}

	return 0;
}

/********************************************************************
*Name				:jlSpeedRead
*Description		:获取计量流速
*Input				:maxbytes	输出缓存最大长度
*Output			:buffer			输出缓存
*Return			:流速，单位0.01升/秒；错误返回ERROR
*History			:2013-08-05,modified by syj
*/
int jlSpeedRead(int nozzle)
{
	int pspeed_1 = 0, pspeed_2 = 0, pspeed_3 = 0, pspeed_4 = 0, pspeed = 0;
	int oilspeed = 0;
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*脉冲速度*/
	if(1 == ((jlparam->Pulse>>0)&1))	/*A1路*/
	{	
		pspeed_1 = sensorSpeedRead(SENSOR_A11) + sensorSpeedRead(SENSOR_A12);
	}
	if(1 == ((jlparam->Pulse>>1)&1))	/*A2路*/
	{	
		pspeed_2 = sensorSpeedRead(SENSOR_A21) + sensorSpeedRead(SENSOR_A22);
	}
	if(1 == ((jlparam->Pulse>>2)&1))	/*B1路*/
	{	
		pspeed_3 = sensorSpeedRead(SENSOR_B11) + sensorSpeedRead(SENSOR_B12);
	}
	if(1 == ((jlparam->Pulse>>3)&1))	/*B2路*/
	{	
		pspeed_4 = sensorSpeedRead(SENSOR_B21) + sensorSpeedRead(SENSOR_B22);
	}
	pspeed = pspeed_1 + pspeed_2 + pspeed_3 + pspeed_4;

	/*升数流速*/
	oilspeed = pspeed * jlparam->Equivalent / 10000;
	
	return oilspeed;
}


/********************************************************************
*Name				:jlVersionRead
*Description		:获取计量软件版本
*Input				:maxbytes	输出缓存最大长度
*Output			:buffer			输出缓存
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlVersionRead(char *buffer, int maxbytes)
{
	if(maxbytes<strlen(JL_VERSION))	memcpy(buffer, JL_VERSION, maxbytes);
	else													memcpy(buffer, JL_VERSION, strlen(JL_VERSION));

	return 0;
}


/********************************************************************
*Name				:jlSumRead
*Description		:获取计量总累数据
*Input				:nozzle			0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:volume		总累加油升数
*						:money			总累加油金额
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlSumRead(int nozzle, unsigned long long *volume, unsigned long long *money)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*volume=jlparam->VolumeSum;	*money=jlparam->MoneySum;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlPriceRead
*Description		:获取计量单价
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:price		计量单价
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlPriceRead(int nozzle, unsigned int *price)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(JL_NOZZLE_1==nozzle)			{jlparam=&JlParamA1;}
	else if(JL_NOZZLE_2==nozzle)	{jlparam=&JlParamB1;}
	else												return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*price=jlparam->Price;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlEquivalentRead
*Description		:获取计量当量
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:equivalent		计量当量
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlEquivalentRead(int nozzle, unsigned int *equivalent)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*equivalent=jlparam->Equivalent;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlUnPulseTimeRead
*Description		:获取计量无脉冲超时停机时间
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:param		计量无脉冲超时停机时间
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlUnPulseTimeRead(int nozzle, unsigned int *param)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*param=jlparam->UnPulseTime;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlAdvanceRead
*Description		:获取计量提前量
*Input				:nozzle			0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:advance		提前量
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlAdvanceRead(int nozzle, unsigned int *advance)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*advance=jlparam->Advance;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlShieldRead
*Description		:获取屏蔽量
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:shield		屏蔽量
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlShieldRead(int nozzle, unsigned int *shield)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*shield=jlparam->Shield;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlOverShieldRead
*Description		:获取过冲屏蔽量
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:overShield		过冲屏蔽量
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlOverShieldRead(int nozzle, unsigned int *overShield)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*overShield=jlparam->OverShield;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlValveVolumeRead
*Description		:获取计量大阀启动油量
*Input				:nozzle					0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:valveVolume		计量大阀启动油量
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlValveVolumeRead(int nozzle, unsigned int *valveVolume)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*valveVolume=jlparam->ValveVolume;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlValveStopTimeRead
*Description		:获取计量无脉冲超时关闭大阀时间
*Input				:nozzle					0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:valveStopTime		计量无脉冲超时关闭大阀时间，单位:秒
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlValveStopTimeRead(int nozzle, unsigned int *valveStopTime)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*valveStopTime=jlparam->UnPulseStopTime;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlAlgorithmRead
*Description		:获取计量加油量算法类型
*Input				:nozzle			0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:algorithm		计量加油量算法类型 0x00=四舍五入；0x01=油量舍金额入；
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlAlgorithmRead(int nozzle, unsigned int *algorithm)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

	/*输出赋值*/
	*algorithm = jlparam->Algorithm;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlNoteDsp
*Description		:计量当次数据显示
*Input				:nozzle					0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlNoteDsp(int nozzle)
{
	JlParamStruct *jlparam=NULL;
	unsigned char buffer[32]={0}, i=0;
	unsigned long long bcd_data=0;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*锁*/
	jlLock(jlparam);

#if _TYPE_BIG260_
	/*金额*/
	bcd_data=hex2Bcd(jlparam->OilMoney);
	buffer[0]=(char)((bcd_data>>28)&0x0f)+0x30;	buffer[1]=(char)((bcd_data>>24)&0x0f)+0x30;
	buffer[2]=(char)((bcd_data>>20)&0x0f)+0x30;	buffer[3]=(char)((bcd_data>>16)&0x0f)+0x30;	
	buffer[4]=(char)((bcd_data>>12)&0x0f)+0x30;	buffer[5]=(char)((bcd_data>>8)&0x0f)+0x30;	
	buffer[6]=(char)((bcd_data>>4)&0x0f)+0x30;		buffer[7]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<5; i++)
	{
		if(0x30==buffer[0+i])	buffer[0+i]=' ';
		else								break;
	}
	/*油量*/
	bcd_data=hex2Bcd(jlparam->OilVolume);
	buffer[8]=(char)((bcd_data>>28)&0x0f)+0x30;	buffer[9]=(char)((bcd_data>>24)&0x0f)+0x30;
	buffer[10]=(char)((bcd_data>>20)&0x0f)+0x30;	buffer[11]=(char)((bcd_data>>16)&0x0f)+0x30;	
	buffer[12]=(char)((bcd_data>>12)&0x0f)+0x30;	buffer[13]=(char)((bcd_data>>8)&0x0f)+0x30;	
	buffer[14]=(char)((bcd_data>>4)&0x0f)+0x30;	buffer[15]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<5; i++)
	{
		if(0x30==buffer[8+i])	buffer[8+i]=' ';
		else								break;
	}
	/*单价*/
	bcd_data=hex2Bcd(jlparam->OilPrice);
	buffer[16]=(char)((bcd_data>>20)&0x0f)+0x30;	buffer[17]=(char)((bcd_data>>16)&0x0f)+0x30;
	buffer[18]=(char)((bcd_data>>12)&0x0f)+0x30;	buffer[19]=(char)((bcd_data>>8)&0x0f)+0x30;	
	buffer[20]=(char)((bcd_data>>4)&0x0f)+0x30;	buffer[21]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<3; i++)
	{
		if(0x30==buffer[16+i])	buffer[16+i]=' ';
		else								break;
	}
#else
	/*金额*/
	bcd_data=hex2Bcd(jlparam->OilMoney);
	buffer[0]=(char)((bcd_data>>20)&0x0f)+0x30;	buffer[1]=(char)((bcd_data>>16)&0x0f)+0x30;	buffer[2]=(char)((bcd_data>>12)&0x0f)+0x30;
	buffer[3]=(char)((bcd_data>>8)&0x0f)+0x30;	buffer[4]=(char)((bcd_data>>4)&0x0f)+0x30;	buffer[5]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<3; i++)
	{
		if(0x30==buffer[0+i])	buffer[0+i]=' ';
		else								break;
	}
	/*油量*/
	bcd_data=hex2Bcd(jlparam->OilVolume);
	buffer[6]=(char)((bcd_data>>20)&0x0f)+0x30;	buffer[7]=(char)((bcd_data>>16)&0x0f)+0x30;	buffer[8]=(char)((bcd_data>>12)&0x0f)+0x30;
	buffer[9]=(char)((bcd_data>>8)&0x0f)+0x30;	buffer[10]=(char)((bcd_data>>4)&0x0f)+0x30;	buffer[11]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<3; i++)
	{
		if(0x30==buffer[6+i])	buffer[6+i]=' ';
		else								break;
	}
	/*单价*/
	bcd_data=hex2Bcd(jlparam->OilPrice);
	buffer[12]=(char)((bcd_data>>12)&0x0f)+0x30;	buffer[13]=(char)((bcd_data>>8)&0x0f)+0x30;	
	buffer[14]=(char)((bcd_data>>4)&0x0f)+0x30;		buffer[15]=(char)((bcd_data>>0)&0x0f)+0x30;
	for(i=0; i<1; i++)
	{
		if(0x30==buffer[12+i])	buffer[12+i]=' ';
		else									break;
	}
#endif
	/*解锁*/
	jlUnlock(jlparam);

	/*显示当次明细*/
	taxDsp(jlparam->Nozzle, buffer, TAX_POINT_DSP);
	
	return	0;
}


/********************************************************************
*Name				:jlTypeRead
*Description		:获取计量机型
*Input				:None
*Output			:None
*Return				:计量机型
*History			:2013-08-05,modified by syj
*/
int jlTypeRead(int nozzle)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else									return ERROR;

	return	jlparam->Type;
}


/********************************************************************
*Name				:jlTaxDsp
*Description		:税控大屏显示
*Input				:nozzle		枪号0=1号枪(即A1枪)；1=2号枪(即B1枪)
*						:buffer		显示缓存，固定长度为16字节ASCII字符，可显示字符包括\
*						:				数字0~9；字母'A','L','P','H','E'；无显示则为空格符号' '；\
*						:				显示顺序自油量屏高位到低位->金额屏高位到低位->单价屏高位到低位
*						:point		0=不点亮显示小数点；1=点亮显示小数点
*Output			:None
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxDsp(unsigned char nozzle, unsigned char *buffer, unsigned char point)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else									return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*显示*/
	if(0!=taxDsp(jlparam->Nozzle, buffer, point))
	{
		return ERROR;
	}
	
	return	0;
}


/********************************************************************
*Name				:jlTaxNoteDsp
*Description		:税控当次加油数据显示
*Input				:nozzle		0=1号枪(即A1枪)；1=2号枪(即B1枪)
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxNoteDsp(int nozzle)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*显示税控当次加油数据*/
	if(0!=taxNoteDsp(jlparam->Nozzle))
	{
		return ERROR;
	}

	return	0;
}


/********************************************************************
*Name				:jlTaxDayVolDsp
*Description		:税控日累显示
*Input				:nozzle		0=1号枪(即A1枪)；1=2号枪(即B1枪)
*						:time		日期，4字节压缩BCD码，格式为CCYYMMDD
*Output			:None
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxDayVolDsp(int nozzle, char *time, int nbytes)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*判断时间合法性*/
	if(0!=timeVerification(time, 4))
	{
		return ERROR;
	}

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*显示税控日累*/
	if(0!=taxDayVolDsp(jlparam->Nozzle, time))
	{
		return ERROR;
	}	
	
	return	0;
}


/********************************************************************
*Name				:jlTaxMonthVolDsp
*Description		:税控月累显示
*Input				:nozzle		0=1号枪(即A1枪)；1=2号枪(即B1枪)
*						:time		月份，3字节压缩BCD码，格式为YYYYMM
*Output			:None
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxMonthVolDsp(int nozzle, char *time, int nbytes)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*判断时间合法性*/
	if(0!=timeVerification(time, 3))
	{
		return ERROR;
	}

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*显示税控月累*/
	if(0!=taxMonthVolDsp(jlparam->Nozzle, time))
	{
		return ERROR;
	}
	
	return	0;
}


/********************************************************************
*Name				:jlTaxTimeDsp
*Description		:税控时间显示
*Input				:None
*Output			:None
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxTimeDsp(void)
{
	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}

	/*设置税控时间*/
	if(0!=taxTimeDsp())
	{
		return ERROR;
	}

	return 0;
}


/********************************************************************
*Name				:jlTaxSumDsp
*Description		:税控总累数据显示
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:handle		0=显示总累数据；1=显示出厂检定数据；2=显示税务检定数据?
*Output			:None
*Return				:0=成功，其它=失败
*History			:2013-08-05,modified by syj
*/
int jlTaxSumDsp(int nozzle, int handle)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*显示税控累计*/
	if(0!=taxSumDsp(jlparam->Nozzle, (unsigned char)handle))
	{
		return ERROR;
	}

	return	0;
}


/********************************************************************
*Name				:jlTaxTimeWrite
*Description		:税控时间设置，加油状态不允许进行操作
*Input				:time		时间(BCD，6bytes，YYYYMMDDHHMM)
*						:nbytes		时间数据长度，固定6字节
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlTaxTimeWrite(char *time, int nbytes)
{
	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}

	/*判断时间长度*/
	if(6!=nbytes)
	{
		return ERROR;
	}

	/*设置税控时间*/
	if(0!=taxTimeChg(time))
	{
		return ERROR;
	}

	return 0;
}


/********************************************************************
*Name				:jlTaxVerifyWrite
*Description		:税控检定，两条枪都处于空闲状态时允许操作
*Input				:param		内容:0=出厂检定；1=税务首次检定
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlTaxVerifyWrite(int handle)
{
	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}

	/*税控检定设置*/
	if(0!=taxVerification((unsigned char)handle))
	{
		return ERROR;
	}
	
	return 0;
}


/********************************************************************
*Name				:jlPriceWrite
*Description		:单价设置
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:price		单价
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlPriceWrite(int nozzle, unsigned int price)
{
	unsigned char wrbuffer[8]={0};
	off_t offset=0;
	JlParamStruct *jlparam=NULL;
	int istate=0;

	/*判断枪选*/
	if(JL_NOZZLE_1==nozzle)			{jlparam=&JlParamA1;	offset=JL0_PRICE;}
	else if(JL_NOZZLE_2==nozzle){jlparam=&JlParamB1;	offset=JL1_PRICE;}
	else												return ERROR;

	/*计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE != JlParamA1.JlState || JL_STATE_IDLE != JlParamB1.JlState)
	{
		printf("%s:%d:计量任务处于非空闲状态，禁止修改价格!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	/*设置税控单价*/
	istate=taxPriceChg(jlparam->Nozzle, price);
	if(0!=istate)
	{
		return ERROR;
	}

	/*保存计量单价配置*/
	wrbuffer[0]=(unsigned char)(price>>24);	wrbuffer[1]=(unsigned char)(price>>16);
	wrbuffer[2]=(unsigned char)(price>>8);	wrbuffer[3]=(unsigned char)(price>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新计量单价并产生并以新单价产生零加油记录*/
	jlparam->Price=price;
	jlparam->OilVolume=0;	jlparam->OilMoney=0;		jlparam->OilPrice=jlparam->Price;	

	/*解锁*/
	jlUnlock(jlparam);

	jlOilDataWrite(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlEquivalentWrite
*Description		:计量当量设置
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:equivalent		当量
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlEquivalentWrite(int nozzle, unsigned int equivalent)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_EQUIVALENT;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_EQUIVALENT;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(equivalent>>24);	wrbuffer[1]=(unsigned char)(equivalent>>16);
	wrbuffer[2]=(unsigned char)(equivalent>>8);		wrbuffer[3]=(unsigned char)(equivalent>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->Equivalent=equivalent;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlUnPulseTimeWrite
*Description		:计量无脉冲停机时间设置
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:unPulseTime		无脉冲停机时间设置
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlUnPulseTimeWrite(int nozzle_num, unsigned int unPulseTime)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;
	

	/*判断枪选*/
	if(0==nozzle_num)			{jlparam=&JlParamA1;	offset=JL0_UNPULSE_TIME;}
	else if(1==nozzle_num)	{jlparam=&JlParamB1;	offset=JL1_UNPULSE_TIME;}
	else									return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*判断有效范围*/
	if(unPulseTime<JL_UNPULSE_TIME_MIN|| unPulseTime>JL_UNPULSE_TIME_MAX)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(unPulseTime>>24);	wrbuffer[1]=(unsigned char)(unPulseTime>>16);
	wrbuffer[2]=(unsigned char)(unPulseTime>>8);		wrbuffer[3]=(unsigned char)(unPulseTime>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->UnPulseTime=unPulseTime;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlAdvanceWrite
*Description		:计量提前量设置，范围0.05升~9.99升
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:advance	提前量，单位:0.01升
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlAdvanceWrite(int nozzle, unsigned int advance)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_ADVANCE;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_ADVANCE;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*判断范围*/
	if(advance<JL_ADVANCE_MIN || advance>JL_ADVANCE_MAX)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(advance>>24);	wrbuffer[1]=(unsigned char)(advance>>16);
	wrbuffer[2]=(unsigned char)(advance>>8);		wrbuffer[3]=(unsigned char)(advance>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->Advance=advance;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlShieldWrite
*Description		:计量屏蔽量设置
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:shiled		屏蔽量
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlShieldWrite(int nozzle, unsigned int shiled)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_SHIELD;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_SHIELD;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(shiled>>24);	wrbuffer[1]=(unsigned char)(shiled>>16);
	wrbuffer[2]=(unsigned char)(shiled>>8);		wrbuffer[3]=(unsigned char)(shiled>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->Shield=shiled;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlOverShieldWrite
*Description		:计量过冲屏蔽量设置
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:overShield		过冲屏蔽量
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlOverShieldWrite(int nozzle, unsigned int overShield)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_OVER_SHIELD;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_OVER_SHIELD;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(overShield>>24);	wrbuffer[1]=(unsigned char)(overShield>>16);
	wrbuffer[2]=(unsigned char)(overShield>>8);		wrbuffer[3]=(unsigned char)(overShield>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->OverShield=overShield;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlValveVolumeWrite
*Description		:油量达到一定值时开启大阀
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:valveVolume	大阀开启油量
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlValveVolumeWrite(int nozzle, unsigned int valveVolume)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_VAVLE_VOLUME;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_VAVLE_VOLUME;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(valveVolume>>24);	wrbuffer[1]=(unsigned char)(valveVolume>>16);
	wrbuffer[2]=(unsigned char)(valveVolume>>8);		wrbuffer[3]=(unsigned char)(valveVolume>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->ValveVolume=valveVolume;

	/*解锁*/
	jlUnlock(jlparam);
	
	return	0;
}


/********************************************************************
*Name				:jlValveStopTimeWrite
*Description		:无脉冲时间超过此值时关闭大阀
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:valveStop		大阀关闭时间，单位:秒
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlValveStopTimeWrite(int nozzle, unsigned int valveStop)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_VAVLE_STOP;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_VAVLE_STOP;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(char)(valveStop>>24);	wrbuffer[1]=(char)(valveStop>>16);
	wrbuffer[2]=(char)(valveStop>>8);		wrbuffer[3]=(char)(valveStop>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->UnPulseStopTime=valveStop;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlAlgorithmWrite
*Description		:加油量算法类型设置
*Input				:nozzle				0=1号枪(A1枪)；1=2号枪(B1枪)
*						:algorithm			加油量算法类型 0x00=四舍五入(JL_ALGORITHM_45)；0x01=油量舍金额入(JL_ALGORITHM_UP)；
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2016-01-04,modified by syj
*/
int jlAlgorithmWrite(int nozzle, unsigned int algorithm)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_ALGORITHM;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_ALGORITHM;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(char)(algorithm>>24);	wrbuffer[1]=(char)(algorithm>>16);
	wrbuffer[2]=(char)(algorithm>>8);		wrbuffer[3]=(char)(algorithm>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->Algorithm = algorithm;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlTypeWrite
*Description		:计量机型设置
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:type		机型
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2013-08-05,modified by syj
*/
int jlTypeWrite(int nozzle, unsigned int type)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_TYPE;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_TYPE;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(type>>24);	wrbuffer[1]=(unsigned char)(type>>16);
	wrbuffer[2]=(unsigned char)(type>>8);	wrbuffer[3]=(unsigned char)(type>>0);
	istate=paramSetupWrite(offset, wrbuffer, 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->Type=type;

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}

int jlBigVolTimeWrite(int nozzle,unsigned int time)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_BIG_VOL_TIME;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_BIG_VOL_TIME;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(time>>0);
	istate=paramSetupWrite(offset, wrbuffer, 1);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->bigVolTime=(time>>4)*10+(time&0x0f);

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}
int jlBigVolSpeedWrite(int nozzle,unsigned int Speed)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=JL0_BIG_VOL_SPEED;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=JL1_BIG_VOL_SPEED;}
	else							return ERROR;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	/*保存配置*/
	wrbuffer[0]=(unsigned char)(Speed>>0);
	istate=paramSetupWrite(offset, wrbuffer, 1);
	if(0!=istate)
	{
		return ERROR;
	}

	/*锁*/
	jlLock(jlparam);

	/*更新缓存内配置*/
	jlparam->bigVolSpeed=(Speed>>4)*10+(Speed&0x0f);

	/*解锁*/
	jlUnlock(jlparam);

	return	0;
}

/********************************************************************
*Name				:jlOilDataClr
*Description		:清除计量总累等数据 
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:type		机型
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2016-03-11,modified by syj
*/
int jlOilDataClr(void)
{
	JlParamStruct *jlparam = NULL;

	/*判断状态，计量非空闲状态不允许设置*/
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}

	jlparam = &JlParamA1;
	jlparam->OilMoney = 0;	jlparam->OilVolume = 0;	jlparam->OilPrice = 0;
	jlparam->VolumeSum = 0;	jlparam->MoneySum = 0;
	jlparam->StopNo = 0;	jlparam->JlState = 0;
	jlOilDataWrite(jlparam);

	jlparam = &JlParamB1;
	jlparam->OilMoney = 0;	jlparam->OilVolume = 0;	jlparam->OilPrice = 0;
	jlparam->VolumeSum = 0;	jlparam->MoneySum = 0;
	jlparam->StopNo = 0;	jlparam->JlState = 0;
	jlOilDataWrite(jlparam);

	return	0;
}


/********************************************************************
*Name				:jlOilDataClr
*Description		:清除计量总累等数据 
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:element	操作对象 0 = 累计升数；1 = 累计金额；
*Output			:None
*Return				:0=成功；其它=失败 
*History			:2016-06-11,modified by syj
*/
int jlSumWrite(int nozzle, int element, unsigned long long value)
{
	JlParamStruct *jlparam=NULL;

	/*判断枪选*/
	if(0==nozzle)			{jlparam = &JlParamA1;}
	else if(1==nozzle)	{jlparam = &JlParamB1;}
	else							{printf("%s:%d:枪选错误 [nozzle = %d] !\n", __FUNCTION__, __LINE__, nozzle);return ERROR;}

	/*判断计量状态，只有空闲状态允许更改*/
	if(JL_STATE_IDLE != jlparam->JlState)
	{
		return ERROR;
	}

	/*更改数据*/
	jlLock(jlparam);
	if(0 == element)	jlparam->VolumeSum = value;	
	else						jlparam->MoneySum = value;
	jlUnlock(jlparam);

	/*存储数据*/
	jlOilDataWrite(jlparam);

	return 0;
}


/********************************************************************
*Name				:jlPresetAmountCalculate
*Description		:计算预置量，根据预置金额(油量)计算预置油量(金额)，如果是任意加油则以最大金额和最大升数的较小值为基础计算
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*						:data		预置数据:
*						:				最大金额 4Hex +
*						:				最大升数 4Hex +
*						:				预置金额 4Hex +
*						:				预置升数 4Hex +
*						:				价格		  4Hex +
*						:				预置方式 1Bin(0=任意；1=定升数；2=定金额)
*Output			:data		预置数据:
*						:				最大金额 4Hex +
*						:				最大升数 4Hex +
*						:				预置金额 4Hex +
*						:				预置升数 4Hex +
*						:				价格		  4Hex +
*						:				预置方式 1Bin(0=任意；1=定升数；2=定金额)
*Return				:成功返回0；其它返回错误；
*History			:2015-08-05,modified by syj
*/
int jlPresetAmountCalculate(int nozzle, unsigned char *data)
{
	unsigned long long temp_volume = 0;
	unsigned int pmoney_max = 0, pvolume_max = 0;
	unsigned int pmoney = 0, pvolume = 0, pprice = 0;
	unsigned char pmode = 0;
	unsigned char tmp = 0;
	JlParamStruct *jlparam=NULL;

	/*解析数据*/
	pmoney_max = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|(data[3]<<0);
	pvolume_max = (data[4]<<24)|(data[5]<<16)|(data[6]<<8)|(data[7]<<0);
	pmoney = (data[8]<<24)|(data[9]<<16)|(data[10]<<8)|(data[11]<<0);
	pvolume = (data[12]<<24)|(data[13]<<16)|(data[14]<<8)|(data[15]<<0);
	pprice = (data[16]<<24)|(data[17]<<16)|(data[18]<<8)|(data[19]<<0);
	pmode = data[20];

	/*判断单价*/
	if(0 == pprice)
	{
		printf("%s:%d:单价非法!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							{printf("%s:%d:枪选错误 [nozzle = %d] !\n", __FUNCTION__, __LINE__, nozzle);return ERROR;}

	/*任意加油，以允许的最大值和计量允许的最大值较小值为最大允许值；
	*	以最大金额计算最大升数，如果以此计算得出的最大升数不大于当前允许最大升数则以此金额为预置
	*/
	if(1 != pmode && 2 != pmode)
	{
		if(pmoney_max <= JL_MONEY_MAX)	pmoney_max = pmoney_max;
		else														pmoney_max = JL_MONEY_MAX;
		if(pvolume_max <= JL_MONEY_MAX)pvolume_max = pvolume_max;
		else														pvolume_max = JL_MONEY_MAX;
		//if(pprice < 100)	{tmp = 1;	pvolume = pvolume_max;}
		//else					{tmp = 2;	pmoney = pmoney_max;}

		if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney_max*10000/pprice%100) > 0)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney_max*10000/pprice%100) == 0)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney_max*1000/pprice%10) >= JL_ROUNDING)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney_max*1000/pprice%10) < JL_ROUNDING)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice;
		}
		if(temp_volume <= pvolume_max)	{tmp = 2;	pmoney = pmoney_max;}
		else														{tmp = 1;	pvolume = pvolume_max;}
	}
	/*预置升数，计算得出的金额: 新算法直接舍去；四舍五入算法*/
	if(1 == pmode || 1 == tmp)
	{
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			pmoney= (unsigned long long)pvolume*pprice/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pvolume*pprice/10%10)<JL_ROUNDING)
		{
			pmoney = (unsigned long long)pvolume*pprice/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pvolume*pprice/10%10)>=JL_ROUNDING)
		{
			pmoney = (unsigned long long)pvolume*pprice/100 + 1;
		}

		pmode = 1;
	}
	/*预置金额，计算得出的升数: 新算法第三、四位小数为0时舍去、非0时进位；四舍五入算法第三位小数小于5时舍去、不小于5时进位*/
	if(2 == pmode || 2 == tmp)
	{
		if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney*10000/pprice%100) > 0)
		{
			pvolume = (unsigned long long)pmoney*100/pprice+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney*10000/pprice%100) == 0)
		{
			pvolume = (unsigned long long)pmoney*100/pprice;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney*1000/pprice%10) >= JL_ROUNDING)
		{
			pvolume = (unsigned long long)pmoney*100/pprice + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney*1000/pprice%10) < JL_ROUNDING)
		{
			pvolume = (unsigned long long)pmoney*100/pprice;
		}

		pmode = 2;
	}

	/*输出数据*/
	data[0] = (char)(pmoney_max>>24);	data[1] = (char)(pmoney_max>>16);
	data[2] = (char)(pmoney_max>>8);	data[3] = (char)(pmoney_max>>0);
	data[4] = (char)(pvolume_max>>24);	data[5] = (char)(pvolume_max>>16);
	data[6] = (char)(pvolume_max>>8);	data[7] = (char)(pvolume_max>>0);
	data[8] = (char)(pmoney>>24);			data[9] = (char)(pmoney>>16);
	data[10] = (char)(pmoney>>8);			data[11] = (char)(pmoney>>0);
	data[12] = (char)(pvolume>>24);		data[13] = (char)(pvolume>>16);
	data[14] = (char)(pvolume>>8);			data[15] = (char)(pvolume>>0);
	data[16] = (char)(pprice>>24);			data[17] = (char)(pprice>>16);
	data[18] = (char)(pprice>>8);				data[19] = (char)(pprice>>0);
	data[20] = pmode;

#if 0
	pmoney = *money;	pvolume = *volume;
	if(0==mode && price<100)
	{
	}
	else if(0==mode && price>=100)
	{
		if(pmoney<JL_MONEY_MAX)	pmoney=pmoney;
		else										pmoney=JL_MONEY_MAX;

		if((pmoney*1000/price%10)<JL_ROUNDING)	pvolume=pmoney*100/price;
		else																	pvolume=pmoney*100/price+1;
	}
	else if(1==mode)
	{
		if((pvolume*price/10%10)<JL_ROUNDING)		pmoney=pvolume*price/100;
		else																	pmoney=pvolume*price/100+1;
	}
	else if(2==mode)
	{
		if((pmoney*1000/price%10)<JL_ROUNDING)	pvolume=pmoney*100/price;
		else																	pvolume=pmoney*100/price+1;
	}

	*money = pmoney;	*volume = pvolume;
#endif

#if 0
	/*任意加油*/
	if(1 != preset_mode && 2 != preset_mode)
	{
		price=preset_price;	mode=preset_mode;	tmp = 2;
		if(preset<JL_MONEY_MAX)	maxmoney=preset;
		else										maxmoney=JL_MONEY_MAX;
	}
	/*预置升数*/
	if(1 == preset_mode || 1 == tmp)			
	{
		if(1 == tmp)		volume = maxvolume;
		else					volume = preset;	
		price=preset_price;	mode=preset_mode;
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			money=volume*price/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (volume*price/10%10)<JL_ROUNDING)
		{
			money = volume*price/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (volume*price/10%10)>=JL_ROUNDING)
		{
			money = volume*price/100 + 1;
		}
	}
	/*预置金额*/
	if(2 == preset_mode || 2 == tmp)			
	{
		if(2 == tmp)		money = maxmoney;
		else					money = preset;
		price=preset_price;	mode=preset_mode;
		if(money >= JL_MONEY_MAX)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100) > 0)
		{
			volume=money*100/price+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && (money*10000/price%100) == 0)
		{
			volume=money*100/price;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10) >= JL_ROUNDING)
		{
			volume=money*100/price + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && (money*1000/price%10) < JL_ROUNDING)
		{
			volume=money*100/price;
		}
	}
#endif

	return 0;
}


/**********************************************************************
*Name				:jlParamInit
*Description		:计量模块参数初始化，包括配置信息及加油数据
*Input				:nozzle		0=1号枪(A1枪)；1=2号枪(B1枪)
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-06-09,modified by syj
*/
int jlParamInit(int nozzle)
{
	JlParamStruct *jlparam=NULL;
	int istate=0, i=0;
	char buffer[32]={0};
	off_t offset=0;

	/*判断枪选*/
	if(0==nozzle)			{jlparam=&JlParamA1;	offset=0*JL_FM_LEN;}
	else if(1==nozzle)	{jlparam=&JlParamB1;	offset=1*JL_FM_LEN;}
	else							return ERROR;

	/*清除铁电内容*/
	for(i=0; i<(JL_FM_LEN/32); i++)
	{
		istate=framWrite(FM_ADDR_JL, i*32+offset, buffer, 32);
		if(0!=istate)
		{
			return ERROR;
		}
	}
	/*清除铁电最后不足32字节的位置*/
	if(0!=JL_FM_LEN%32)
	{
		istate=framWrite(FM_ADDR_JL, (JL_FM_LEN/32)*32+offset, buffer, JL_FM_LEN%32);
		if(0!=istate)
		{
			return ERROR;
		}
	}

	/*当量5000*/
	istate=jlEquivalentWrite(nozzle, 5000);
	if(0!=istate)
	{
		return ERROR;
	}

	/*无脉冲停机时间60秒*/
	istate=jlUnPulseTimeWrite(nozzle, 60);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量提前量0.50升*/
	istate=jlAdvanceWrite(nozzle, 50);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量屏蔽量0.10升*/
	istate=jlShieldWrite(nozzle, 10);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量过冲屏蔽量0.30升*/
	istate=jlOverShieldWrite(nozzle, 30);
	if(0!=istate)
	{
		return ERROR;
	}

	/*油量达到一定值时开启大阀0.01升*/
	istate=jlValveVolumeWrite(nozzle, 1);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量无脉冲时间超过此值时关闭大阀10秒*/
	istate=jlValveStopTimeWrite(nozzle, 10);
	if(0!=istate)
	{
		return ERROR;
	}

	/*加油量算法类型，默认为四舍五入算法*/
	istate=jlAlgorithmWrite(nozzle, JL_ALGORITHM_UP);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量机型，默认为0(即普通计量)*/
	istate=jlTypeWrite(nozzle, 0);
	if(0!=istate)
	{
		return ERROR;
	}

	/*计量单价1.00元*/
	if(0==nozzle)	istate=paramSetupWrite(JL0_PRICE, "\x00\x00\x00\x64", 4);
	else					istate=paramSetupWrite(JL1_PRICE, "\x00\x00\x00\x64", 4);
	if(0!=istate)
	{
		return ERROR;
	}

	/*双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8*/
	if(0==nozzle)	istate=paramSetupWrite(JL0_BIG_VOL_TIME, "\x08", 1);
	else					istate=paramSetupWrite(JL1_BIG_VOL_TIME, "\x08", 1);
	if(0!=istate)
	{
		return ERROR;
	}

	/*双编码器大流量流速控制1BCD L/分10-80默认15*/
	if(0==nozzle)	istate=paramSetupWrite(JL0_BIG_VOL_SPEED, "\x15", 1);
	else					istate=paramSetupWrite(JL1_BIG_VOL_SPEED, "\x15", 1);
	if(0!=istate)
	{
		return ERROR;
	}
	
	/*锁*/
	jlLock(jlparam);
	/*更新计量单价并产生并以新单价产生零加油记录*/
	jlparam->Price=100;
	jlparam->OilVolume=0;	jlparam->OilMoney=0;		jlparam->OilPrice=jlparam->Price;
	jlparam->VolumeSum=0;	jlparam->MoneySum=0;
	jlparam->StopNo=0;	jlparam->JlState=0;
	/*解锁*/
	jlUnlock(jlparam);

	/*保存当笔数据*/
	jlOilDataWrite(jlparam);

	return 0;
}


/********************************************************************
*Name				:jlInit
*Description	:计量模块初始化
*Input				:None
*Output			:None
*Return			:0=成功；
*History			:2013-08-05,modified by syj
*/
int jlInit()
{
	JlParamStruct *jlparam=NULL;
	unsigned char buffer[64]={0}, tmp_buffer[256]={0};
	long long bcd_data=0;
	int i=0, istate=0;
	unsigned int crc=0, money=0, volume=0, price=0;
	int read_flag=0;
	int tid_tax_receive=0;


	JL_MOTORA1_OFF;
	JL_MOTORA2_OFF;
	JL_MOTORB1_OFF;
	JL_MOTORB2_OFF;
	JL_VALVEAA_OFF;
	JL_VALVEAB_OFF;
	JL_VALVEBA_OFF;
	JL_VALVEBB_OFF;


	//*看门狗定时器，时间间隔1tick
	if(NULL==JlWdgId)	JlWdgId=wdCreate();
	if(NULL==JlWdgId)	printf("Error! Create watch dog timer 'JlWdgId' failed!\n");
	else								wdStart(JlWdgId, JL_WDG_TICKS, (FUNCPTR)jlWdgIsr, 0);
	//*/

	/*创建接收税控报税口数据的信号量*/
	jlSemIdTax=semBCreate(SEM_FULL, SEM_Q_PRIORITY|SEM_Q_FIFO);
	if(NULL==jlSemIdTax)	printf("Error!Create semaphore 'jlSemIdTax failed!\n");

	/*税控初始化*/
	taxInit();

	/*A1枪参数初始化*/
	jlparam=&JlParamA1;
	jlparam->Nozzle=JL_NOZZLE_1;	jlparam->DevPumpPermit=DEV_PUMP_PERMITA;	jlparam->JlState=JL_STATE_IDLE;
	/*当前有效价格*/
	if(0==paramSetupRead(JL0_PRICE, buffer, 4))
	{jlparam->Price=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*当量，默认为5000*/
	if(0==paramSetupRead(JL0_EQUIVALENT, buffer, 4))	
	{jlparam->Equivalent=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	if(jlparam->Equivalent < JL_EQUIVALENT_MIN || jlparam->Equivalent > JL_EQUIVALENT_MAX)
	{
		jlparam->Equivalent = JL_EQUIVALENT_DEFAULT;
	}

	/*无脉冲超时时间*/
	if(0==paramSetupRead(JL0_UNPULSE_TIME, buffer, 4))	
	{jlparam->UnPulseTime=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*提前量*/
	if(0==paramSetupRead(JL0_ADVANCE, buffer, 4))
	{jlparam->Advance=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*屏蔽量*/
	if(0==paramSetupRead(JL0_SHIELD, buffer, 4))	
	{jlparam->Shield=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*过冲屏蔽量*/
	if(0==paramSetupRead(JL0_OVER_SHIELD, buffer, 4))
	{jlparam->OverShield=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*大阀开启屏蔽量*/
	if(0==paramSetupRead(JL0_VAVLE_VOLUME, buffer, 4))	
	{jlparam->ValveVolume=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*无脉冲超时关大阀时间*/
	if(0==paramSetupRead(JL0_VAVLE_STOP, buffer, 4))
	{jlparam->UnPulseStopTime=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}

	/*计量加油量算法标识，默认为新的算法*/
	if(0==paramSetupRead(JL0_ALGORITHM, buffer, 4))
	{jlparam->Algorithm = (buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	if(JL_ALGORITHM_45!=jlparam->Algorithm && JL_ALGORITHM_UP!=jlparam->Algorithm)
	{
		jlparam->Algorithm = JL_ALGORITHM_UP;
	}

	/*计量机型*/
	if(0==paramSetupRead(JL0_TYPE, buffer, 4))	
	{
		jlparam->Type=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);
	}

	/*机型非法，默认普通机型*/
	if(jlparam->Type>2)
		jlparam->Type=0;
	
	/*普通机型*/
	if(0==jlparam->Type)
	{
		jlparam->Motor=1<<0;		jlparam->Valve=1<<0;	jlparam->Pulse=1<<0;
	}
	/*单枪大流量*/
	else if(1==jlparam->Type)
	{
		/*A1与B1双流量*/
		jlparam->Motor=0x01;		jlparam->Valve=0x01;	jlparam->Pulse=0x05;
	}
	/*双枪大流量*/
	else if(2==jlparam->Type)
	{
		/*A1与A2双流量,B1与B2双流量*/
		jlparam->Motor=0x01;		jlparam->Valve=0x01;	jlparam->Pulse=0x03;
	}

	/*(A1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8*/
	if(0==paramSetupRead(JL0_BIG_VOL_TIME, buffer,1))	
	{
		if((buffer[0]>=0x05) &&(buffer[0]<=0x20))
			jlparam->bigVolTime=(buffer[0]>>4)*10+(buffer[0]&0x0f);
		else
			jlparam->bigVolTime=8;
	}

	/*(A1枪)双编码器大流量流速控制1BCD L/分10-80默认15*/
	if(0==paramSetupRead(JL0_BIG_VOL_SPEED, buffer,1))	
	{
		if((buffer[0]>=0x10) &&(buffer[0]<=0x80))
			jlparam->bigVolSpeed=(buffer[0]>>4)*10+(buffer[0]&0x0f);
		else
			jlparam->bigVolSpeed=15;
	}
	
	/*A1枪判断加油数据合法性，不合法且备份合法时以备份数据存储*/
	framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA, buffer, 64);
	crc=crc16Get(buffer, 62);	tmp_buffer[0]=(char)(crc>>8);	tmp_buffer[1]=(char)(crc>>0);
	if(0!=memcmp(tmp_buffer, &buffer[62], 2))
	{
		framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA2, buffer, 64);
		crc=crc16Get(buffer, 62);	tmp_buffer[0]=(char)(crc>>8);	tmp_buffer[1]=(char)(crc>>0);
		if(0==memcmp(tmp_buffer, &buffer[62], 2))
		{
			framWrite(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*0, buffer, 64);
		}
	}

	/*************************************************************************************
	*读取上次加油数据，总累等，如果上次为加油中状态异常断电，则根据税控当次数据进行恢复，显示当次加油数据
	*/
	/*A1枪计量状态*/
	if(0==framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA+21, buffer, 1))
	{
		jlparam->JlState=buffer[0];
	}
	/*A1枪显示上次加油数据*/
	framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA, buffer, 8);
	jlparam->OilVolume=(buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0);
	jlparam->OilMoney=(buffer[3]<<16)|(buffer[4]<<8)|(buffer[5]<<0);
	jlparam->OilPrice=(buffer[6]<<8)|(buffer[7]<<0);
	/*A1枪总累数据*/
	framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA+8, buffer, 6);
	jlparam->VolumeSum=((unsigned long long)buffer[0]<<40)|((unsigned long long)buffer[1]<<32)|((unsigned long long)buffer[2]<<24)|\
		((unsigned long long)buffer[3]<<16)|((unsigned long long)buffer[4]<<8)|((unsigned long long)buffer[5]<<0);
	framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA+14, buffer, 6);
	jlparam->MoneySum=((unsigned long long)buffer[0]<<40)|((unsigned long long)buffer[1]<<32)|((unsigned long long)buffer[2]<<24)|\
		((unsigned long long)buffer[3]<<16)|((unsigned long long)buffer[4]<<8)|((unsigned long long)buffer[5]<<0);
	/*A1枪上次加油未完毕的处理*/
	if(JL_STATE_OILLING==jlparam->JlState || JL_STATE_FINISHING==jlparam->JlState)
	{
		for(i=0; i<3; i++)
		{
			istate=jlTaxNoteRead(jlparam, &volume, &money, &price);
			if(0==istate && 0!=volume && 0!=money && 0!=price)
			{
				memset(tmp_buffer, 0, sizeof(tmp_buffer));
				sprintf(tmp_buffer, "计量断电恢复:[JLNOZZLE=%d]", jlparam->Nozzle);
				sprintf(tmp_buffer+strlen(tmp_buffer), "[计量数据:升数=%d;金额=%d;单价=%d;升累=%d;金额累计=%d]", jlparam->OilVolume,jlparam->OilMoney, jlparam->OilPrice, jlparam->VolumeSum, jlparam->MoneySum);
				sprintf(tmp_buffer+strlen(tmp_buffer), "[税控数据:升数=%d;金额=%d;单价=%d]", volume,money,price);
				jljUserLog("%s\n", tmp_buffer);
				/*保存加油数据*/
				jlparam->OilVolume=volume;	jlparam->OilMoney=money;	jlparam->OilPrice=price;
				/*计算,保存总累*/
				jlparam->VolumeSum=jlparam->VolumeSum+jlparam->OilVolume;
				jlparam->MoneySum=jlparam->MoneySum+jlparam->OilMoney;
				/*计量状态恢复为空闲*/
				jlparam->JlState=JL_STATE_IDLE;
				/*存储当次数据*/
				jlOilDataWrite(jlparam);
				break;
			}
		}
	}
	/*显示当次加油数据*/
	for(i=0; i<5; i++)
	{
		if(0==jlNoteDsp(jlparam->Nozzle))	break;
		else														taskDelay(ONE_SECOND);
	}


	/*A1枪互斥信号量*/
	jlparam->SemId=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==jlparam->SemId)	printf("Error!Create semaphore 'JlParamA1.SemId' failed!\n");

	/*A1枪初始化计量任务*/
	jlparam->TaskId=taskSpawn("tJlA1", 150, 0, 0x4000, (FUNCPTR)jlOilTask, 0,1,2,3,4,5,6,7,8,9);
	if(ERROR==jlparam->TaskId)
	{
		printf("Error! Creat task 'tJlA1' failed!\n");
	}

//myDEBUG
//jlparam->OilVolume= 0;
//jlparam->OilMoney= 0;
//	wrbuffer[6]=(char)(jlparam->OilPrice>>8);			wrbuffer[7]=(char)(jlparam->OilPrice>>0);
//jlparam->VolumeSum = 0;
//jlparam->MoneySum = 0;
//jlparam->StopNo=0;
//jlparam->JlState=0;
//jlOilDataWrite(jlparam);





	/*B1枪参数*/
	jlparam=&JlParamB1;
	jlparam->Nozzle=JL_NOZZLE_2;	jlparam->DevPumpPermit=DEV_PUMP_PERMITB;	jlparam->JlState=JL_STATE_IDLE;	

	/*当前有效价格*/
	if(0==paramSetupRead(JL1_PRICE, buffer, 4))
	{jlparam->Price=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*当量，默认为5000*/
	if(0==paramSetupRead(JL1_EQUIVALENT, buffer, 4))
	{jlparam->Equivalent=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	if(jlparam->Equivalent < JL_EQUIVALENT_MIN || jlparam->Equivalent > JL_EQUIVALENT_MAX)
	{
		jlparam->Equivalent = JL_EQUIVALENT_DEFAULT;
	}
	/*无脉冲超时时间*/
	if(0==paramSetupRead(JL1_UNPULSE_TIME, buffer, 4))	
	{jlparam->UnPulseTime=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*提前关阀量*/
	if(0==paramSetupRead(JL1_ADVANCE, buffer, 4))
	{jlparam->Advance=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*计数屏蔽量*/
	if(0==paramSetupRead(JL1_SHIELD, buffer, 4))	
	{jlparam->Shield=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*计量过冲屏蔽量*/
	if(0==paramSetupRead(JL1_OVER_SHIELD, buffer, 4))
	{jlparam->OverShield=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*需要有多少油量时才开启大阀，0表示立即开启，单位:0.01升*/
	if(0==paramSetupRead(JL1_VAVLE_VOLUME, buffer, 4))
	{jlparam->ValveVolume=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*无脉冲时间超过此值时关闭大阀*/
	if(0==paramSetupRead(JL1_VAVLE_STOP, buffer, 4))
	{jlparam->UnPulseStopTime=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	/*计量加油量算法标识，默认为新的算法*/
	if(0==paramSetupRead(JL1_ALGORITHM, buffer, 4))
	{jlparam->Algorithm = (buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);}
	if(JL_ALGORITHM_45!=jlparam->Algorithm && JL_ALGORITHM_UP!=jlparam->Algorithm)
	{
		jlparam->Algorithm = JL_ALGORITHM_UP;
	}
	/*计量机型*/
	if(0==paramSetupRead(JL1_TYPE, buffer, 4))	
	{
		jlparam->Type=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);
	}

	/*机型非法，默认普通机型*/
	if(jlparam->Type>2)
		jlparam->Type=0;
	
	/*普通机型*/
	if(0==jlparam->Type)
	{
		jlparam->Motor=1<<2;		jlparam->Valve=1<<2;	jlparam->Pulse=1<<2;
	}
	/*单枪大流量*/
	else if(1==jlparam->Type)
	{
		/*A1与B1双流量*/
		jlparam->Motor=0x04;		jlparam->Valve=0x04;	jlparam->Pulse=0x05;
	}
	/*双枪大流量*/
	else if(2==jlparam->Type)
	{
		/*A1与A2双流量,B1与B2双流量*/
		jlparam->Motor=0x04;		jlparam->Valve=0x04;	jlparam->Pulse=0x0C;
	}

	/*(B1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8*/
	if(0==paramSetupRead(JL1_BIG_VOL_TIME, buffer,1))	
	{
		if((buffer[0]>=0x05) &&(buffer[0]<=0x20))
			jlparam->bigVolTime=(buffer[0]>>4)*10+(buffer[0]&0x0f);
		else
			jlparam->bigVolTime=8;
	}

	/*(B1枪)双编码器大流量流速控制1BCD L/分10-80默认15*/
	if(0==paramSetupRead(JL1_BIG_VOL_SPEED, buffer,1))	
	{
		if((buffer[0]>=0x10) &&(buffer[0]<=0x80))
			jlparam->bigVolSpeed=(buffer[0]>>4)*10+(buffer[0]&0x0f);
		else
			jlparam->bigVolSpeed=15;
	}
	/*B1枪判断加油数据合法性，不合法且备份合法时以备份数据存储*/
	framRead(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*jlparam->Nozzle, buffer, 64);
	crc=crc16Get(buffer, 62);	tmp_buffer[0]=(char)(crc>>8);	tmp_buffer[1]=(char)(crc>>0);
	if(0!=memcmp(tmp_buffer, &buffer[62], 2))
	{
		framRead(FM_ADDR_JL, JL_FM_DATA2+JL_FM_LEN*jlparam->Nozzle, buffer, 64);
		crc=crc16Get(buffer, 62);	tmp_buffer[0]=(char)(crc>>8);	tmp_buffer[1]=(char)(crc>>0);
		if(0==memcmp(tmp_buffer, &buffer[62], 2))
		{
			framWrite(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*1, buffer, 64);
		}
	}

	/*************************************************************************************
	*读取上次加油数据，总累等，如果上次为加油中状态异常断电，则根据税控当次数据进行恢复，显示当次加油数据
	*/
	/*B1枪计量状态*/
	if(0==framRead(FM_ADDR_JL, jlparam->Nozzle*JL_FM_LEN+JL_FM_DATA+21, buffer, 1))
	{
		jlparam->JlState=buffer[0];
	}
	/*B1枪上次加油数据*/
	framRead(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*1, buffer, 8);
	jlparam->OilVolume=(buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0);
	jlparam->OilMoney=(buffer[3]<<16)|(buffer[4]<<8)|(buffer[5]<<0);
	jlparam->OilPrice=(buffer[6]<<8)|(buffer[7]<<0);
	/*B1枪总累数据*/
	framRead(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*1+8, buffer, 6);
	jlparam->VolumeSum=((unsigned long long)buffer[0]<<40)|((unsigned long long)buffer[1]<<32)|((unsigned long long)buffer[2]<<24)|\
		((unsigned long long)buffer[3]<<16)|((unsigned long long)buffer[4]<<8)|((unsigned long long)buffer[5]<<0);
	framRead(FM_ADDR_JL, JL_FM_DATA+JL_FM_LEN*1+14, buffer, 6);
	jlparam->MoneySum=((unsigned long long)buffer[0]<<40)|((unsigned long long)buffer[1]<<32)|((unsigned long long)buffer[2]<<24)|\
		((unsigned long long)buffer[3]<<16)|((unsigned long long)buffer[4]<<8)|((unsigned long long)buffer[5]<<0);
	/*B1枪上次加油未完毕的处理*/
	if(JL_STATE_OILLING==jlparam->JlState || JL_STATE_FINISHING==jlparam->JlState)
	{
		for(i=0; i<3; i++)
		{
			istate=jlTaxNoteRead(jlparam, &volume, &money, &price);
			if(0==istate && 0!=volume && 0!=money && 0!=price)
			{
				memset(tmp_buffer, 0, sizeof(tmp_buffer));
				sprintf(tmp_buffer, "计量断电恢复:[JLNOZZLE=%d]", jlparam->Nozzle);
				sprintf(tmp_buffer+strlen(tmp_buffer), "[计量数据:升数=%d;金额=%d;单价=%d;升累=%d;金额累计=%d]", jlparam->OilVolume,jlparam->OilMoney, jlparam->OilPrice, jlparam->VolumeSum, jlparam->MoneySum);
				sprintf(tmp_buffer+strlen(tmp_buffer), "[税控数据:升数=%d;金额=%d;单价=%d]", volume,money,price);
				jljUserLog("%s\n", tmp_buffer);
				/*保存加油数据*/
				jlparam->OilVolume=volume;	jlparam->OilMoney=money;	jlparam->OilPrice=price;
				/*计算,保存总累*/
				jlparam->VolumeSum=jlparam->VolumeSum+jlparam->OilVolume;
				jlparam->MoneySum=jlparam->MoneySum+jlparam->OilMoney;
				/*计量状态恢复为空闲*/
				jlparam->JlState=JL_STATE_IDLE;
				/*存储当次数据*/
				jlOilDataWrite(jlparam);
				break;
			}
		}
	}
	/*显示当次加油数据*/
	for(i=0; i<5; i++)
	{
		if(0==jlNoteDsp(jlparam->Nozzle))	break;
		else														taskDelay(ONE_SECOND);
	}

	/*B1枪互斥信号量*/
	jlparam->SemId=semMCreate(SEM_DELETE_SAFE|SEM_Q_FIFO);
	if(NULL==jlparam->SemId)	printf("Error!Create semaphore 'JlParamB1.SemId' failed!\n");

	/*A1枪初始化计量任务*/
	jlparam->TaskId=taskSpawn("tJlB1", 150, 0, 0x4000, (FUNCPTR)jlOilTask, 1,1,2,3,4,5,6,7,8,9);
	if(ERROR==jlparam->TaskId)
	{
		printf("Error! Creat task 'tJlB1' failed!\n");
	}


	return 0;
}



/*
int ttTest()
{
	int istate=0, volume, money, price;

	istate=jlTaxNoteRead(&JlParamA1, &volume, &money, &price);
printf("%s istate=%x\n", __FUNCTION__, istate);

	return 0;
}


int ttTest11()
{
	int istate=0, kvalue, money, price;

	istate=jlTaxEquivalentRead(0, &kvalue);
printf("%s istate=%x kvalue=%x\n", __FUNCTION__, istate, kvalue);

	return 0;
}
*/


