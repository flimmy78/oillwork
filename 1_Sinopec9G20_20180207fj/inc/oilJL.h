#ifndef __OILJL_H__
#define __OILJL_H__




#define JL_IOSTATE_ON			0					//IO端口处于打开状态
#define JL_IOSTATE_OFF			1					//IO端口处于关闭状态

#define JL_ALGORITHM_45		0x00				    //加油数据采用四舍五入算法
#define JL_ALGORITHM_UP		0x01				    //加油数据采用油量舍金额入的算法

//计量枪号
#define JL_NOZZLE_1			    0					//计量1号枪
#define JL_NOZZLE_2			    1					//计量2号枪

//计量加油状态
#define JL_STATE_IDLE			0				//计量状态空闲
#define JL_STATE_STARTING	    1					    //计量状态加油启动中
#define JL_STATE_OILLING		2					//计量状态加油中
#define JL_STATE_FINISHING   	3					    //计量状态加油结束中

//计量参数铁电片内地址
#define JL_FM_LEN				(FM_SIZE_JL/2)		/*计量铁电参数单枪数据长度*/
#define JL_FM_DATA				0								/*单枪加油数据，共64字节:
																					*	油量(3bytes，偏移0)
																					*	金额(3bytes，偏移3)
																					*	单价(2bytes，偏移6)
																					*	总累油量(6bytes，偏移8)
																					*	总累金额(6bytes，偏移14)
																					*	停机代码(1byte，偏移21)
																					*	计量状态(1byte，偏移22)
																					*	备份(偏移~61)
																					*	CRC校验(2bytes，偏移62)*/
#define JL_FM_DATA2				(JL_FM_LEN/2)			/*单枪加油数据备份，格式同上*/


/*计量参数定义*/
#define JL_VERSION							"V1.00"			/*计量软件版本号*/

#if _TYPE_BIG260_
#define JL_MONEY_MAX					10000000			/*最大允许预置金额，260升大流量为10万元*/
#define JL_VOLUME_MAX					10000000			/*最大允许预置油量，260升大流量为10万升*/
#else
//#define JL_VOLUME_MAX					999900			/*最大允许加油量，单位0.01升，普通流量机型为9999.00元*/
//#define JL_MONEY_MAX					999900			/*最大允许加油金额，单位0.01元，普通流量机型为9999.00升*/
#define JL_MONEY_MAX					990000			/*最大允许加油金额，单位0.01元，普通流量机型为9999.00升*/
#define JL_VOLUME_MAX					990000			/*最大允许加油量，单位0.01升，普通流量机型为9999.00元*/
#endif

#define JL_PRICE_MIN						100				/*单价允许最小值，单位0.01元/升*/
#define JL_PRICE_MAX						9999				/*单价允许最大值，单位0.01元/升*/
#define JL_VOLUME_MIN					1					/*最小允许加油量，单位0.01升*/
#define JL_MONEY_MIN						1					/*最小允许加油金额，单位0.01元*/
#define JL_ROUNDING						5					/*计算数据舍入边界，小于此值舍去，否则进位加一*/
#define JL_PULSE_DIFFERENCE		20					/*允许各路脉冲相差最大脉冲数*/
#define JL_SPEED_HI							600				/*超过此值即为大流速，单位0.01L/分钟*/
#define JL_ADVANCE_MIN					5					/*提前量最小值，0.05升*/
#define JL_ADVANCE_MAX					999				/*提前量最大值，9.99升*/
#define JL_UNPULSE_TIME_MIN		30					/*无脉冲超时时间最小值，30秒*/
#define JL_UNPULSE_TIME_MAX		180				/*无脉冲超时时间最大值，180秒*/
#define JL_EQUIVALENT_MIN			1000				/*当量最小值，1000*/
#define JL_EQUIVALENT_MAX			19999			/*当量最大值，19999*/
#define JL_EQUIVALENT_DEFAULT	5000				/*默认当量 5000*/
//#define JL_WDG_TICKS						(10*sysClkRateGet()/1000)	/*计量看门狗定时器时间间隔10ms*/
//#define JL_SEM_TIMEOUT					(10*ONE_SECOND)				/*(10*sysClkRateGet()/1000)	//计量操作获取信号量超时时间10ms*/

/*计量启动失败原因说明*/
extern const char *jlStartFiledReson[];

/*计量停机代码说明*/
extern const char *jlStopReason[];

//计量参数
typedef struct
{
	unsigned char Nozzle;						//0=1号枪；1=2号枪
	unsigned char ErrNO;						//错误代码0=正常；1=计量异常
	unsigned char DevPumpPermit;		         //税控启泵允许信号设备ID

	//计量加油参数,fj:
	unsigned char JlState;					//计量状态0=空闲；1=加油中；2=加油结束中
	unsigned int LastSensor;				//前一刻脉冲总数
	unsigned int Speed;						//实时流速，单位:0.01升/分钟
	unsigned int NoSensorTimer;		        //加油状态无脉冲超时定时器
	unsigned char PresetMode;			    //预置方式0=任意，1=定油量，2=定金额
	unsigned long long PresetMoney;			//预置金额，计算时可能会有超过4字节的情况，故此处采用长整形
	unsigned long long PresetVolume;		//预置油量，计算时可能会有超过4字节的情况，故此处采用长整形
	unsigned int OilVolume;				    //本次加油量
	unsigned int OilMoney;					//本次加油金额
	unsigned int OilPrice;					//本次加油价格
	unsigned char StopNo;					//停机代码，即加油中停机的原因
											//0x00=正常
											//0x01=缺一路脉冲，需停机
											//0x02=达到预置量，需停机
											//0x03=税控禁止，需停机
											//0x04=无脉冲超时，需停机
											//0x05=主机掉电，需停机
	unsigned long long MoneySum;		    //总累计金额 HEX
	unsigned long long VolumeSum;		    //总累计油量 HEX

	//计量基本参数
	unsigned int Price;						//当前有效价格,	hex，单位:0.01升
	unsigned int Equivalent;				//当量,	hex，单位:0.00001升/脉冲
	unsigned int UnPulseTime;				//无脉冲超时时间，无脉冲时间超过此值关闭油机停止加油,	hex，单位:秒
	unsigned int UnPulseStopTime;		    //无脉冲超时时间，无脉冲时间超过此值关闭大阀,	hex，单位:秒
	unsigned int Advance;					//提前关阀量,	hex，单位:0.01升
	unsigned int Shield;					//计数屏蔽量,	hex，小于此出油量按未出油计算，单位:0.01升
	unsigned int OverShield;			    //计量过冲屏蔽量,	hex，过冲量小于此值则按无过冲操作，单位:0.01升
	unsigned int Type;						//计量机型,	hex，根据机型配置下面三个参数'Motor','Valve','Pulse'
	unsigned char Motor;					//操作的电机选择b3~b0分别代表电机B2,B1,A2,A1；0=需要操作,1=不操作
	unsigned char Valve;					//操作的电磁阀选择b3~b0分别代表阀门B2,B1,A2,A1；0=需要操作,1=不操作
	unsigned char Pulse;					//操作的编码器选择b3~b0分别代表编码器B2,B1,A2,A1；0=需要操作,1=不操作
	unsigned int ValveVolume;				//油量达到一定值时开启大阀，0表示立即开启，单位:0.01升
	unsigned int Algorithm;					//加油数据算法 0x00=四舍五入；0x01=油量舍金额入(让利客户)；

	//szb_fj_20171120,add
	unsigned int bigVolTime;				//大流量缺一组脉冲超时时间
	unsigned int bigVolSpeed;				//大流量缺一组脉冲流速控制
	unsigned int bigTimeA1;					//大流量A1缺一组脉冲超时
	unsigned int bigTimeA2;					//大流量A2缺一组脉冲超时
	unsigned int bigTimeB1;					//大流量B1缺一组脉冲超时
	unsigned int bigTimeB2;					//大流量B2缺一组脉冲超时

	//fj:add
	
	unsigned char uchResult_JLInit;  //计量初始化应答结果
	unsigned char uchRand[6];        //计量返回的随机数
	unsigned char uchParamType_Set;  //计量设置的参数代号
    unsigned char uchResult_Set;     //计量设置结果
	unsigned char uchParamType_Query;//计量查询的参数代号
	unsigned char uchResult_Query;   //计量查询结果
	unsigned char uchData_Query[32];  //计量查询返回的数据
    unsigned char uchResult_Rounding;//计量加油凑整结果
    unsigned char uchResult_TradeOK; //计量交易确认结果
	unsigned char uchResult_AddOilEnd;//加油结束结果
	unsigned char uchResult_ClearOilData; //清除当前加油数据的结果
	unsigned char uchResult_ViewOilData;  //显示计量当前加油数据的结果

	unsigned char uchState_GunStateQ;   //油枪状态
    unsigned short ushPara_GunStateQ;    //油枪参数  
	unsigned int  nOilMoney_GunStateQ;  //加油金额
    unsigned int  nOilVolume_GunStateQ; //加油升数
	unsigned int  nOilPrice_GunStateQ;  //加油单价
    unsigned long long llOilVolumeSum_GunStateQ; //总油量
	unsigned long long llOilMoneySum_GunStateQ;  //总价格

    //unsigned char uchPresetMode_OilStart; //加油启动预置方式 0x00:预置金额 0x01:预置升数 0x02:任意加油
	//unsigned int  nPresetCount_OilStart;  //预置数额
	//unsigned int  nPresetPrice_OilStart;  //预置单价
	unsigned char uchState_OilStart; //计量加油启动返回状态
	unsigned short ushStateParam_OilStart; //计量加油状态参数，失败的原因。

	unsigned char uchState_ReadOiling; //读取加油中数据返回的状态
	unsigned int  nOilMoney_ReadOiling;  //加油金额
    unsigned int  nOilVolume_ReadOiling; //加油升数
	unsigned int  nOilPrice_ReadOiling;  //加油单价
    unsigned short ushOilSpeed_ReadOiling; //加油流速，0.1L每分钟

}JlParamStruct;

typedef struct 
{
    unsigned char uchCmdNo;
	unsigned char uchGunNo;
	unsigned char uchParaIndex;
	unsigned char uchLength;
	unsigned char uchSendGunStateData;
	unsigned char uchSendData[256];
}JLReportStruct;


typedef struct 
{
	JLReportStruct jlStruReportA;
	JLReportStruct jlStruReportB;
	void (*sendJLInit)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendGetRand)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLParamSet)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLParamQuery)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
    void (*sendJLStateQuery)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendShowOilData)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendClearOilData)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLAddOilStart)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendGetOilData)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLAddOilEnd)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLTradeOK)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
	void (*sendJLOilRounding)(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct);
}JLClass;

extern JLClass jlOptClass;

bool InitJL();
void JL_Process();


//税控查询
extern int jlTaxTimeDsp(void); //3:时间显示
extern int jlTaxDayVolDsp(int nozzle, char *time, int nbytes); //显示日累计
extern int jlTaxMonthVolDsp(int nozzle, char *time, int nbytes);//显示月累计
extern int jlTaxSumDsp(int nozzle, int handle); //显示总累计
extern int jlTaxNoteDsp(int nozzle);

//税控设置
extern int jlTaxDsp(unsigned char nozzle_num, unsigned char *buffer, unsigned char point);//8:税控直通显示设置
extern int jlTaxVerifyWrite(int handle);
extern int jlTaxTimeWrite(char *time, int nbytes); //3：时间设置


//计量查询操作
extern int jlSpeedRead(int nozzle);
extern int jlVersionRead(char *buffer, int maxbytes);
extern int jlSumRead(int nozzle, unsigned long long *volume, unsigned long long *money);
extern int jlPriceRead(int nozzle, unsigned int *price);
extern int jlEquivalentRead(int nozzle, unsigned int *equivalent);
extern int jlUnPulseTimeRead(int nozzle, unsigned int *param);
extern int jlAdvanceRead(int nozzle, unsigned int *advance);
extern int jlShieldRead(int nozzle, unsigned int *shield);
extern int jlOverShieldRead(int nozzle, unsigned int *overShield);
extern int jlValveVolumeRead(int nozzle, unsigned int *valveVolume);
extern int jlValveStopTimeRead(int nozzle, unsigned int *valveStopTime);
extern int jlAlgorithmRead(int nozzle, unsigned int *algorithm);
extern int jlNoteDsp(int nozzle);
extern int jlTypeRead(int nozzle);//18:读计量机型
extern int jlTaxEquivalentRead(int nozzle, int *equivalent); //27:显示

//szb_fj_20171120,add
extern int jlBigVolTimeRead(int nozzle,unsigned int *time);
extern int jlBigVolSpeedRead(int nozzle,unsigned int *Speed);

//计量设置操作
extern int jlPriceWrite(int nozzle, unsigned int price);
extern int jlEquivalentWrite(int nozzle, unsigned int equivalent);
extern int jlUnPulseTimeWrite(int nozzle, unsigned int unPulseTime);
extern int jlAdvanceWrite(int nozzle, unsigned int advance);
extern int jlShieldWrite(int nozzle, unsigned int shield);
extern int jlOverShieldWrite(int nozzle, unsigned int overShield);
extern int jlValveVolumeWrite(int nozzle, unsigned int valveVolume);
extern int jlValveStopTimeWrite(int nozzle, unsigned int valveStop);
extern int jlAlgorithmWrite(int nozzle, unsigned int algorithm);
extern int jlTypeWrite(int nozzle, unsigned int param);
extern int jlOilDataClr(void);
extern int jlSumWrite(int nozzle, int element, unsigned long long value);
extern int jlParamInit(int nozzle);

//szb_fj_20171120,add
extern int jlBigVolTimeWrite(int nozzle,unsigned int time);
extern int jlBigVolSpeedWrite(int nozzle,unsigned int Speed);

//加油过程操作
extern int jlPresetAmountCalculate(int nozzle, unsigned char *data);
extern int jlOilStart(int nozzle, unsigned int preset, unsigned int preset_price, unsigned int preset_mode);
extern int jlOilRead(int nozzle, unsigned int *money, unsigned int *volume, unsigned int *price, unsigned char *state);
extern int jlOilFinish(int nozzle, unsigned long long *money_sum, unsigned long long *volume_sum, unsigned int *pmoney, unsigned int *pvolume, unsigned int *pprice, unsigned char *state);
extern int jlOilCZ(int nozzle, int handle);


extern int GetjlSumRead(int nozzle, unsigned long long *volume, unsigned long long *money); //fj:
extern int GetjlPriceRead(int nozzle, unsigned int *price);








#endif


