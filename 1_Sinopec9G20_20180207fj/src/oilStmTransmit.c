// ****************************************************************************
// 	本文件实现9g20芯片与stm32f205芯片之间的通讯功能
// 
// #include "pio.h"
// #include "pit.h"
// #include "pio_it.h"
// #include "oilCfg.h"
// #include "oilCom.h"
// #include "oilStmTransmit.h"

#include "../inc/main.h"

// SPI1发送缓存节点
typedef struct
{
 	NODE Ndptrs;		//节点指针
 	short Data;			//数据
}Spi1DataNode;
 
 //SPI1通讯发送及接受数据链表
static LIST spi1ListTx;
 
// STM32扩展IO有效状态
typedef struct 
{
 	unsigned char Times[56];		//当前即时状态保持次数
 	char LastState[56];				//上一次状态
 	char ValidState[56];			//有效状态，即消抖之后的状态
 	char Valid[56];					//是否有效，即是否是读取且经过消抖之后有效状态
 	char Chg[56];					//有效状态发生变化0=无变化，1=有变化
}StmPioStruct;
 
static StmPioStruct StmPio;

pthread_rwlock_t rwlock_rwTask = PTHREAD_RWLOCK_INITIALIZER;  //fj:暂时用于taskLock,模拟vxworks的任务锁 

static int g_nfd = -1;


 
// SPI1中断初始化数据结构
//static Pin Pin_Stm32Int={1 <<1,AT91C_BASE_PIOC, AT91C_ID_PIOC, PIO_INPUT,PIO_PULLUP};
 
 
// SPI1发送处理任务ID
static int tIdSpi1=0;
// 
// SPI1发送命令定时读取STM32参数看门狗中断
// static WDOG_ID WdgIdStmPrmRead=NULL;
// 
// SPI1数据接收触发信号量
// static SEM_ID SemIdSpi1Transmit=NULL;
// static SEM_ID SemIdSpi1Rx=NULL;
// 
// SPI1扩展串口接收数据缓存
 static RING_ID RngSpi1Usart1=NULL, RngSpi1Usart2=NULL, RngSpi1Usart3=NULL, RngSpi1Uart4=NULL, RngSpi1Uart5=NULL, RngSpi1Usart6=NULL;


// SPI1扩展串口接收信号量，当有数据接收时释放该信号量通知读函数有数据
// static SEM_ID SemIdSpi1Usart1=NULL, SemIdSpi1Usart2=NULL, SemIdSpi1Usart3=NULL, SemIdSpi1Uart4=NULL, SemIdSpi1Uart5=NULL, SemIdSpi1Usart6=NULL;
static sem_t SemIdSpi1Usart1, SemIdSpi1Usart2, SemIdSpi1Usart3, SemIdSpi1Uart4, SemIdSpi1Uart5, SemIdSpi1Usart6;
 
// SPI1设置/查询操作RTC互斥信号量，以保证在同一时刻只有一个用户进行此操作
// static SEM_ID SemIdSpi1RtcSet=NULL, SemIdSpi1RtcRead=NULL;
// 
// 扩展即时IO采集，当值为-1时表示当前状态无效，即stm未报告状态
 static char StmPioState[56]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,\
 													-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
 
 
// 时钟，用于存储当前时钟
 static RTCTime RtcTime;
 
 //stm32返回数据
typedef struct 
 {
 	unsigned char SetBuffer[STM_SPI1_MAX_LEN];			//数据缓存
 	unsigned int SetLen;														//数据缓存字节数
 	char SetPack;																//是否保存了一帧数据，0=否，1=是
 
 	unsigned char RtcSetBuffer[STM_SPI1_MAX_LEN];		//RTC设置
 	unsigned char RtcSetLen;
 	char RtcSetPack;
 
 	unsigned char RtcReadBuffer[STM_SPI1_MAX_LEN];	//RTC读取
 	unsigned char RtcReadLen;
 	char RtcReadPack;
 
 	unsigned char RstBuffer[16];										//SPI同步复位
 	unsigned char RstLen;
 }StmRxStruct;
 
 static StmRxStruct StmRx;
 
 
// 从机是否有发送数据的请求0=无；1=有
 static int spi1StmAsk=0;
 
// 时间及IO状态采集定时器
 unsigned int spi1TransTimer=0;
 
 
// 文件内函数声明
// static int spi1Write(char *buffer, int nbytes, int port);
// static void tSpi1Transmit(void);
// static void tSpi1StmAskIsr(void);
 
/****************************************************************************
 *Name				:stmSpi1DevInit
 *Description	:spi1设备初始化
 *Input				:None
 *Output			:None
 *Return			:None
 *History			:2013-07-01,modified by syj
 */
/* 
static bool stmSpi1DevInit ()
 {
	 g_nfd = open("/dev/NUC970_PE2",O_RDWR);

	 if(g_nfd < 0)
	 {
		 printf("open NUC970_PE2 failure!\n");
         return false;
	 }
 
 	return true;
}*/
 
 
/****************************************************************************
 *Name				:stmSpi1DataTx
 *Description		:spi1数据
 *Input				:None
 *Output			:None
 *Return				:spi1同步接收到的数据
 *History			:2013-07-01,modified by syj
 */
 
 static unsigned short stmSpi1DataTx(short data)
 {
 	/*while(!(AT91C_BASE_SPI1->SPI_SR & BIT9 ));	//判断发送寄存器空
 	AT91C_BASE_SPI1->SPI_TDR=data	;
 	while(!(AT91C_BASE_SPI1->SPI_SR & BIT9 ));	//判断发送寄存器空
 	while(!(AT91C_BASE_SPI1->SPI_SR & BIT0 ));	//判断接收寄存器满
 
 	return (AT91C_BASE_SPI1->SPI_RDR);*/

	 return 0;
 }
 
 
/****************************************************************************
 *Name				:spi1Write
 *Description		:将向SPI1发送的数据拷贝到发送缓存
 *Input				:buffer		需要发送的数据
 *						:nbytes		需要发送的数据长度
 *						:port			SPI1端口，参入定义如STM_SPI1_PORT_x	(定义包含在oilStmTransmit.h)
 *Output			:None
 *Return				:实际发送长度，错误返回ERROR
 *History			:2014-12-05,modified by syj
 */
 
static int spi1Write(char *buffer, int nbytes, int port)
{
 	int i=0;
 	Spi1DataNode *node=NULL;
 
 	//数据添加端口号后保存到发送链表
 	//发送链表长度达到最大值或申请节点空间失败时退出
 	
 	for(i=0; i<nbytes && lstCount(&spi1ListTx)<=STM_SPI1_NODE_MAX; i++)
 	{
 		node=malloc(sizeof(Spi1DataNode));
 		if(NULL!=node)
 		{
			node->Data=(port<<8)|buffer[i];
 
 			//taskLock(); //fj:
			pthread_rwlock_wrlock(&rwlock_rwTask);
 			lstAdd(&spi1ListTx, (NODE*)node);
			pthread_rwlock_unlock(&rwlock_rwTask);
 			//taskUnlock(); //fj:
 		}
 		else
 		{
 			break;
 		}
 	}
 
 	return i;
}
 
 
/****************************************************************************
 *Name				:stmSpi1Pars
 *Description	:spi1数据接收根据端口号进行分析
 *Input				:None
 *Output			:None
 *Return			:None
 *History			:2013-07-01,modified by syj
*/
static int stmSpi1Pars(unsigned short spi_data)
{
 	unsigned char port=0, data=0, io_id=0;
 	int i=0;
 
 	port=(spi_data>>8)&0xff;	
	data=spi_data&0xff;
 	switch(port)
 	{
 	case STM_SPI1_PORT_SET:
 		break;
 	case STM_SPI1_PORT_USART1:
 		//semTake(SemIdSpi1Usart1, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart1);
 		rngBufPut(RngSpi1Usart1, &data, 1);
		sem_post(&SemIdSpi1Usart1);
 		//semGive(SemIdSpi1Usart1);
 		break;
 	case STM_SPI1_PORT_USART2:
 		//semTake(SemIdSpi1Usart2, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart2);
 		rngBufPut(RngSpi1Usart2, &data, 1);
		sem_post(&SemIdSpi1Usart2);
 		//semGive(SemIdSpi1Usart2);
 		break;
 	case STM_SPI1_PORT_USART3:
 		//semTake(SemIdSpi1Usart3, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart3);
 		rngBufPut(RngSpi1Usart3, &data, 1);
		sem_post(&SemIdSpi1Usart3);
 		//semGive(SemIdSpi1Usart3);
 		break;
 	case STM_SPI1_PORT_UART4:
 		//semTake(SemIdSpi1Uart4, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Uart4);
 		rngBufPut(RngSpi1Uart4, &data, 1);
		sem_post(&SemIdSpi1Uart4);
 		//semGive(SemIdSpi1Uart4);
 		break;
 	case STM_SPI1_PORT_UART5:
 		//semTake(SemIdSpi1Uart5, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Uart5);
 		rngBufPut(RngSpi1Uart5, &data, 1);
		sem_post(&SemIdSpi1Uart5);
 		//semGive(SemIdSpi1Uart5);
 		break;
 	case STM_SPI1_PORT_USART6:
 		//semTake(SemIdSpi1Usart6, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart6);
 		rngBufPut(RngSpi1Usart6, &data, 1);
		sem_post(&SemIdSpi1Usart6);
 		//semGive(SemIdSpi1Usart6);
 		break;
 	case STM_SPI1_PORT_CAN1:
 		break;
 	case STM_SPI1_PORT_CAN2:
 		break;
 	case STM_SPI1_PORT_TIME: 
 		//设置时钟操作数据
 		if(0==StmRx.RtcSetPack)
 		{	
 			if(StmRx.RtcSetLen>=STM_SPI1_MAX_LEN)//防止溢出
 			{
 				StmRx.RtcSetLen=0;
 			}
 			//解析数据
 			StmRx.RtcSetBuffer[StmRx.RtcSetLen]=data;
 			if(0==StmRx.RtcSetLen)
 			{
 				if(0xfe==StmRx.RtcSetBuffer[StmRx.RtcSetLen])	
					StmRx.RtcSetLen++;
 				else																					
					;
 			}
 			else 
 			if(2==StmRx.RtcSetLen)
 			{
 				if(0x01==StmRx.RtcSetBuffer[StmRx.RtcSetLen])	
					StmRx.RtcSetLen++;
 				else																					
					StmRx.RtcSetLen=0;
 			}
 			else	
 				StmRx.RtcSetLen++;
 			
 			//解析数据帧
 			if((StmRx.RtcSetLen>=4)&&(StmRx.RtcSetLen>=(StmRx.RtcSetBuffer[1]+2)))
 			{
 				if(StmRx.RtcSetBuffer[StmRx.RtcSetLen-1]==xorGet(&StmRx.RtcSetBuffer[1], StmRx.RtcSetBuffer[1]+1-1))
 				{
 					StmRx.RtcSetPack=1;
 				}
 				else
				{
 					StmRx.RtcSetLen=0;	
					StmRx.RtcSetPack=0;
 				}
 			}
 			else if(StmRx.RtcSetLen>=STM_SPI1_MAX_LEN)
 			{
 				StmRx.RtcSetLen=0;	
				StmRx.RtcSetPack=0;
 			}
 		}
 
 		//读取时钟操作数据
 		if(0==StmRx.RtcReadPack)
 		{
 			//防止溢出
 			if(StmRx.RtcReadLen>=STM_SPI1_MAX_LEN)
 			{
 				StmRx.RtcReadLen=0;
 			}
 		
 			//解析数据		
 			StmRx.RtcReadBuffer[StmRx.RtcReadLen]=data;
 			if(0==StmRx.RtcReadLen)
 			{
 				if(0xfe==StmRx.RtcReadBuffer[StmRx.RtcReadLen])	
					StmRx.RtcReadLen++;
 				else	
					;
 			}
 			else if(2==StmRx.RtcReadLen)
 			{
 				if(0x02==StmRx.RtcReadBuffer[StmRx.RtcReadLen])	
					StmRx.RtcReadLen++;
 				else	
					StmRx.RtcReadLen=0;
 			}
 			else	
				StmRx.RtcReadLen++;
 
 			//解析数据帧
 			if((StmRx.RtcReadLen>=4)&&(StmRx.RtcReadLen>=(StmRx.RtcReadBuffer[1]+2)))
 			{
 				if(StmRx.RtcReadBuffer[StmRx.RtcReadLen-1]==xorGet(&StmRx.RtcReadBuffer[1], StmRx.RtcReadBuffer[1]+1-1))
 				{
 					//taskLock();
					pthread_rwlock_wrlock(&rwlock_rwTask);
 					RtcTime.century	=StmRx.RtcReadBuffer[4];
 					RtcTime.year		=StmRx.RtcReadBuffer[5];
 					RtcTime.month	=StmRx.RtcReadBuffer[6];
 					RtcTime.date		=StmRx.RtcReadBuffer[7];
 					RtcTime.hour		=StmRx.RtcReadBuffer[8];
 					RtcTime.minute	=StmRx.RtcReadBuffer[9];
 					RtcTime.second	=StmRx.RtcReadBuffer[10];

					//szb_fj_20171120:add
					if(StmRx.RtcReadBuffer[1]>=10+4)
					{
						memcpy(Ipt205IAP_Ver,StmRx.RtcReadBuffer+11,2);
						memcpy(Ipt205APP_Ver,StmRx.RtcReadBuffer+13,2);
					}
					else
					{
						memcpy(Ipt205IAP_Ver,"\x00\x00",2);
						memcpy(Ipt205APP_Ver,"\x00\x00",2);
					}

					pthread_rwlock_unlock(&rwlock_rwTask);
 					//taskUnlock();
 					
 					StmRx.RtcReadPack=1;
 				}
 				else
				{
 					StmRx.RtcReadLen=0;	
					StmRx.RtcReadPack=0;
 				}
 			}
 			else if(StmRx.RtcReadLen>=STM_SPI1_MAX_LEN)
 			{
 				StmRx.RtcReadLen=0;	
				StmRx.RtcReadPack=0;
 			}
 		}
 		
 		break;
 	case STM_SPI1_PORT_IOWRITE:
 		break;
 	case STM_SPI1_PORT_IOREAD:
 		//第一位IO状态
 		io_id=((data>>2)&0x3f);
 		if(io_id<56)
 		{
 			//即时状态
 			StmPioState[io_id]=(data>>1)&1;
 
 			//消抖操作
 			if(StmPio.LastState[io_id]==StmPioState[io_id])	
 			{
 				StmPio.Times[io_id]++;
 			}
 			else
 			{
 				StmPio.LastState[io_id]=StmPioState[io_id];	StmPio.Times[io_id]=0;
				//printf("LastState change------,%d\n",StmPioState[io_id]);
 			}
 
 			//保存有效状态，消抖次数达到时保存有效状态，变化时保存一次变化
 			if(StmPio.Times[io_id]>=STM_SPI1_IO_DEBOUNCE)
 			{
 				//taskLock();
				pthread_rwlock_wrlock(&rwlock_rwTask);
 				if(StmPio.ValidState[io_id]!=StmPioState[io_id])	
					StmPio.Chg[io_id]=1;
 				StmPio.ValidState[io_id]=StmPioState[io_id];	
				StmPio.Valid[io_id]=1;
			    pthread_rwlock_unlock(&rwlock_rwTask);
 				//taskUnlock();
 			}
 		}
 		//第二位IO状态
 		io_id=((data>>2)&0x3f)+1;
 		if(io_id<56)
 		{
 			//即时状态
 			StmPioState[io_id]=(data>>0)&1;
 			//消抖操作
 			if(StmPio.LastState[io_id]==StmPioState[io_id])	StmPio.Times[io_id]++;
 			else
 			{
 				StmPio.LastState[io_id]=StmPioState[io_id];	StmPio.Times[io_id]=0;
 			}
 
 			//保存有效状态，消抖次数达到时保存有效状态，变化时保存一次变化
 			if(StmPio.Times[io_id]>=STM_SPI1_IO_DEBOUNCE)
 			{
 				//taskLock();
				pthread_rwlock_wrlock(&rwlock_rwTask);
 				if(StmPio.ValidState[io_id]!=StmPioState[io_id])	
					StmPio.Chg[io_id]=1;
 				StmPio.ValidState[io_id]=StmPioState[io_id];	
				StmPio.Valid[io_id]=1;
				pthread_rwlock_unlock(&rwlock_rwTask);
 				//taskUnlock();
 			}
 		}
 		break;
 	default:
 		break;
 	}
 
 	return 0;
}
 
 
/****************************************************************************
 *Name				:tSpi1Transmit
 *Description		:spi1通讯处理，当从机有数据发送时会产生中断，中断处理程序会释放
 *						:信号量触发此任务执行发送时钟并解析从机返回数据
 *Input				:None
 *Output			:None
 *Return				:None
 *History			:2013-03-27,modified by syj
*/
void tSpi1Transmit(void)
{
	prctl(PR_SET_NAME,(unsigned long)"tSpi1Transmit");
    printf("spi thread is success!\n");
 	int i=0;
 	unsigned short rx=0, tx=0;
 	Spi1DataNode *node=NULL;
 	char tx_buffer[16]={0};
 	int tx_len=0;

	//unsigned short tx[9] = {0x0000,0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888,0x9999};
	//unsigned short tx = 0x2222;
	//unsigned short rx[20];
	
	//unsigned short rxData[1024] = {0};
	//unsigned short txData[1024] = {0};
 
 	FOREVER
 	{
 		if(spi1TransTimer>=50*ONE_MILLI_SECOND)
 		{	
 			spi1Write("\x00", 1, STM_SPI1_PORT_IOREAD);//IO读取命令
 
 			//时钟读取命令
 			StmRx.RtcReadLen=0;	StmRx.RtcReadPack=0;
 			tx_buffer[0]=0xfe;	
			tx_buffer[1]=2;	
			tx_buffer[2]=0x02;	
			tx_buffer[3]=xorGet(&tx_buffer[1], tx_buffer[1]+1-1);
 			tx_len=4;
 			spi1Write(tx_buffer, tx_len, STM_SPI1_PORT_TIME); 
 			spi1TransTimer=0;
 		}
 	
 		//当发送链表有发送数据或从机有发送请求时执行操作
 	    //read(g_nfd,&spi1StmAsk,4);
		spi1StmAsk = READ_PE2_FLAG();
 		if(lstCount(&spi1ListTx)>0 || 0!=spi1StmAsk)
 		{ 		
 		
 			//STM_SLAVE_ENABLE();	//复位片选，从机使能
			CSLOW();  //qg
 			for(i=0; i<16; i++);

			//printf("spi low \n");

			//transfer(&tx,rx,1);
	        //printf("rx= %d\n",rx[0]);
			//PrintH(9,tx);


 			//发送数据
 		    for(i=0; i<1024; i++)
 			{
 				if(lstCount(&spi1ListTx)>0)
				{
 					node=(Spi1DataNode*)lstGet(&spi1ListTx);
 					if(NULL!=node)
					{	
						tx=node->Data;	
						free(node);
					}
 					else					
					{	
						tx=0;
					}
 				}
 				else
				{
					tx=0;
				}
 
 				//rx=stmSpi1DataTx(tx);
                transfer(&tx,&rx,1);  //qg

				//printf("rx= %d,tx= %d\n",rx,tx);

 				stmSpi1Pars(rx);
 
 				if((0==rx || 0xffff==rx) && i>=10)	
				{
					spi1StmAsk=0;
					//write(g_nfd,&spi1StmAsk,4);
					WRITE_PE2_FLAG(spi1StmAsk);
				}
 				if(0==tx && (0==rx || 0xffff==rx) && i>=10)
				{
					break;
				}
 			}
 
 			//拉高片选，从机失能
 			//STM_SLAVE_DISABLE();
			CSHIGH();//qg
 			for(i=0; i<16; i++);

			//printf("spi high");
 		}
 		//taskDelay(1);
		usleep(1000);
 	} 
 	return;
}
 
 
// ****************************************************************************
// *Name				:tSpi1StmAskIsr
// *Description		:spi1中断处理函数，当9g20-stm32通讯spi中断产生时执行本函数
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj
// */
// static void tSpi1StmAskIsr(void)
// {
// 	//判断并响应中断
// 	if ((Pin_Stm32Int.pio->PIO_ISR&Pin_Stm32Int.mask)==Pin_Stm32Int.mask) 
// 	{
// 		spi1StmAsk=1;
// 	}
// 
// 	return;
// }
// 
// 
// ****************************************************************************
// *Name				:tStmPrmRead
// *Description		:定时向stm发送rtc及io查询命令
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-03-27,modified by syj
// */
// static void stmPrmReadWdgIsr()
// {
// 	char tx_buffer[8]={0};
// 	int tx_len=0;
// 
// 	//IO读取命令
// 	spi1Write("\x00", 1, STM_SPI1_PORT_IOREAD);
// 
// 	//时钟读取命令
// 	StmRx.RtcReadLen=0;	StmRx.RtcReadPack=0;
// 	tx_buffer[0]=0xfe;	tx_buffer[1]=2;	tx_buffer[2]=0x02;	tx_buffer[3]=xorGet(&tx_buffer[1], tx_buffer[1]+1-1);
// 	tx_len=4;
// 	spi1Write(tx_buffer, tx_len, STM_SPI1_PORT_TIME);
// 
// 	wdStart(WdgIdStmPrmRead, STM_SPI1_WDG_TIME, (FUNCPTR)stmPrmReadWdgIsr, 0);
// 
// 	return;
// }
// 
// 
// ****************************************************************************
// *Name				:spi1PumpPermitRead
// *Description		:ARM7扩展IO税控启泵允许信号
// *Input				:dev_num		引脚选择DEV_PUMP_PERMITA/DEV_PUMP_PERMITB
// *Output			:chg				此刻自上一次读取状态之后是否发生变化0=未变化；1=有变化
// *Return				:当前有效状态
// *						:失败返回ERROR，即此时IO非STM返回且经过消抖之后的状态
// *History			:2013-07-01,modified by syj
// */
 
 int spi1PumpPermitRead(int dev_num, char *chg)
 {
 	int istate=ERROR, io_id=0;
 
 	//taskLock(); //fj:
	pthread_rwlock_wrlock(&rwlock_rwTask);
 	
 	switch(dev_num)
 	{
 	case DEV_PUMP_PERMITA:	//MC1
 		io_id=34;
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];
			*chg=StmPio.Chg[io_id];
			StmPio.Chg[io_id]=0;
 		}
 		else	
 		{
 			istate=ERROR;
 		}
 		break;
 	case DEV_PUMP_PERMITB:	//MC2
 		io_id=35;
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];	
			*chg=StmPio.Chg[io_id];	
			StmPio.Chg[io_id]=0;
 		}
 		else	
 		{
 			istate=ERROR;
 		}
 		break;
 	default:
 		istate=ERROR;
 		break;
 	}
 
 	//taskUnlock(); //fj:
	pthread_rwlock_unlock(&rwlock_rwTask);
 
 	return istate;
}
 
/****************************************************************************
 *Name				:spi1GunRead
 *Description		:ARM7扩展IO油枪信号读取
 *Input				:gun_num	枪选	DEV_GUNA1	/DEV_GUNB1/DEV_GUNA2/DEV_GUNB2/DEV_GUN5/DEV_GUN6/DEV_GUN7/DEV_GUN8/DEV_GUN9/DEV_GUN10
 *Output			:chg				此刻自上一次读取状态之后是否发生变化0=未变化；1=有变化
 *Return				:当前有效状态
 *						:失败返回ERROR，即此时IO非STM返回且经过消抖之后的状态
 *History			:2013-07-01,modified by syj
*/
int spi1GunRead(int gun_num, char *chg)
{
	int istate=ERROR, io_id=0;
 
 	//taskLock(); //fj:
	pthread_rwlock_wrlock(&rwlock_rwTask);
 
 	switch(gun_num)
 	{
 	case DEV_GUNA1:
 		io_id=36;
		//printf("Valid = %d\n",StmPio.Valid[io_id]);
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];
			*chg=StmPio.Chg[io_id];	

			if(*chg == 1)
			{
				//printf("char change -----\n");
			}
			StmPio.Chg[io_id]=0;
 		}
 		else
 		{
 			istate=ERROR;
 		}
 		break;
 	case DEV_GUNB1:
 		io_id=37;
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];	
			*chg=StmPio.Chg[io_id];	
			StmPio.Chg[io_id]=0;
 		}
 		else
 		{
 			istate=ERROR;
 		}
 		break;
 	case DEV_GUNA2:
 		io_id=16;
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];
			*chg=StmPio.Chg[io_id];	
			StmPio.Chg[io_id]=0;
 		}
 		else
 		{
 			istate=ERROR;
 		}
 		break;
 	case DEV_GUNB2:
 		io_id=17;
 		if(1==StmPio.Valid[io_id])
 		{
 			istate=StmPio.ValidState[io_id];	
			*chg=StmPio.Chg[io_id];	
			StmPio.Chg[io_id]=0;
 		}
 		else
 		{
 			istate=ERROR;
 		}
 		break;
 	default:
 		istate=ERROR;
 		break;
 	}
 
 	//taskUnlock(); //fj:
	pthread_rwlock_unlock(&rwlock_rwTask);
 
 	return istate;
 }
 
 
// ****************************************************************************
// *Name				:spi1LockRead
// *Description		:读取配置锁，将B2枪作为配置锁
// *Input				:无
// *Output			:chg		此刻自上一次读取状态之后是否发生变化0=未变化；1=有变化
// *Return				:当前有效状态
// *						:失败返回ERROR，即此时IO非STM返回且经过消抖之后的状态
// *History			:2016-01-28,modified by syj
// */
 int spi1LockRead(int *chg)
 {
 	return spi1GunRead(DEV_GUNB2, (char*)chg);
 }
 
 
// ****************************************************************************
// *Name				:spi1UartWrite
// *Description		:ARM7扩展串口数据发送
// *Input				:uartx		串口选择STM_SPI1_PORT_USARTx，如STM_SPI1_PORT_USART1
// *						:buffer		数据缓存
// *						:nbytes	数据缓存大小
// *Output			:None
// *Return				:实际发送字节，错误返回ERROR
// *History			:2013-07-01,modified by syj
// */
int spi1UartWrite(int uartx, char *buffer, int nbytes)
{
 	int i=0;
 
 	switch(uartx)
 	{
	case STM_SPI1_PORT_USART1:
 	case STM_SPI1_PORT_USART2:
 	case STM_SPI1_PORT_USART3:
 	case STM_SPI1_PORT_UART4:
 	case STM_SPI1_PORT_UART5:
 	case STM_SPI1_PORT_USART6:
 		i=spi1Write(buffer, nbytes, uartx);
 		break;
 	default:
 		break;
 	}
 
 	return i;
 }
 
 
// ****************************************************************************
// *Name				:spi1UartRead
// *Description		:ARM7扩展串口数据读取
// *Input				:uartx			串口选择STM_SPI1_PORT_USARTx，如STM_SPI1_PORT_USART1
// *						:buffer			数据缓存
// *						:maxbytes	数据缓存大小
// *Output			:None
// *Return				:实际读取的字节数
// *History			:2013-07-01,modified by syj
// */
 int spi1UartRead(int uartx, char *buffer, int maxbytes)
 {
 	int i=0;
 
 	switch(uartx)
 	{
 	case STM_SPI1_PORT_USART1:
 		if(NULL==RngSpi1Usart1)
			break; 
 		while(0 == rngNBytes(RngSpi1Usart1))	
			usleep(1000);
 		
 		//semTake(SemIdSpi1Usart1, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart1);
 		i=rngBufGet(RngSpi1Usart1, buffer, maxbytes);
		sem_post(&SemIdSpi1Usart1);
 		//semGive(SemIdSpi1Usart1);
 		break;
 	case STM_SPI1_PORT_USART2:
 		if(NULL==RngSpi1Usart2)
			break;
		while(0 == rngNBytes(RngSpi1Usart2))	
			usleep(1000);
 		
 		//semTake(SemIdSpi1Usart2, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart2);
 		i=rngBufGet(RngSpi1Usart2, buffer, maxbytes);
		sem_post(&SemIdSpi1Usart2);
 		//semGive(SemIdSpi1Usart2);
 		break;
 	case STM_SPI1_PORT_USART3:
 		if(NULL==RngSpi1Usart3)	
			break;
 		while(0 == rngNBytes(RngSpi1Usart3))	
			usleep(1);
 
 		//semTake(SemIdSpi1Usart3, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart3);
 		i=rngBufGet(RngSpi1Usart3, buffer, maxbytes);
		sem_post(&SemIdSpi1Usart3);
 		//semGive(SemIdSpi1Usart3);
 		break;
 	case STM_SPI1_PORT_UART4:
 		if(NULL==RngSpi1Uart4)
			break;
 		while(0 == rngNBytes(RngSpi1Uart4))	
			usleep(1000);
 
 		//semTake(SemIdSpi1Uart4, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Uart4);
 		i=rngBufGet(RngSpi1Uart4, buffer, maxbytes);
		sem_post(&SemIdSpi1Uart4);
 		//semGive(SemIdSpi1Uart4);
 		break;
 	case STM_SPI1_PORT_UART5:
 		if(NULL==RngSpi1Uart5)	
			break;		
 		while(0 == rngNBytes(RngSpi1Uart5))	
			usleep(1000);
 
 		//semTake(SemIdSpi1Uart5, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Uart5);
 		i=rngBufGet(RngSpi1Uart5, buffer, maxbytes);
		sem_post(&SemIdSpi1Uart5);
 		//semGive(SemIdSpi1Uart5);
 		break;
 	case STM_SPI1_PORT_USART6:		
 		if(NULL==RngSpi1Usart6)	
			break;
 		while(0 == rngNBytes(RngSpi1Usart6))	
			usleep(1000);
 
 		//semTake(SemIdSpi1Usart6, WAIT_FOREVER);
		sem_wait(&SemIdSpi1Usart6);
 		i=rngBufGet(RngSpi1Usart6, buffer, maxbytes);
		sem_post(&SemIdSpi1Usart6);
 		//semGive(SemIdSpi1Usart6);
 		break;
 	default:
 		break;
 	}
 
 	return i;
 }

// ****************************************************************************
// *Name				:timeWrite
// *Description		:时间设置
// *Input				:None
// *Output			:None
// *Return				:成功返回0；其它=失败
// *History			:2013-07-01,modified by syj
// */
int timeWrite(RTCTime time)
{
 	char tx_buffer[16]={0};
 	int tx_len=0, i=0;
 
 	tx_buffer[0]=0xfe;	 
	tx_buffer[1]=9;	
	tx_buffer[2]=0x01;	
	tx_buffer[3]=time.century;	
	tx_buffer[4]=time.year;
 	tx_buffer[5]=time.month;	
	tx_buffer[6]=time.date;	
	tx_buffer[7]=time.hour;	
	tx_buffer[8]=time.minute;
 	tx_buffer[9]=time.second;

	printf("timeWrite:\n");
/*	tx_buffer[3]=21;	
	tx_buffer[4]=17;
 	tx_buffer[5]=9;	
	tx_buffer[6]=17;	
	tx_buffer[7]=9;	
	tx_buffer[8]=30;
 	tx_buffer[9]=30;*/	
	tx_buffer[10]=xorGet(&tx_buffer[1], tx_buffer[1]+1-1);	
	tx_len=11;
 	spi1Write(tx_buffer, tx_len, STM_SPI1_PORT_TIME);
 
 	return 0;
}


/****************************************************************************
*Name				:timeRead
*Description		:时间读取
*Input				:None
*Output			:None
*Return				:成功返回0；其它=失败
*History			:2013-07-01,modified by syj
*/
int timeRead(RTCTime* time) //szb_fj_20171120,update
{
	unsigned char buff[10] = {0};
	//taskLock(); //fj:
	pthread_rwlock_wrlock(&rwlock_rwTask);
	buff[0]=RtcTime.century;
	buff[1]=RtcTime.year;
	buff[2]=RtcTime.month;
	buff[3]=RtcTime.date;
	buff[4]=RtcTime.hour;
	buff[5]=RtcTime.minute;
	buff[6]=RtcTime.second;
	if(0==timeVerification(buff, 7))
	{
		time->century=RtcTime.century;
		time->year=RtcTime.year;
		time->month=RtcTime.month;
		time->date=RtcTime.date;
		time->hour=RtcTime.hour;
		time->minute=RtcTime.minute;
		time->second=RtcTime.second;
	}
    pthread_rwlock_unlock(&rwlock_rwTask);
	//taskUnlock();

	return 0;
}


/****************************************************************************
*Name				:spi1Init
*Description		:spi1初始化
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/

bool spi1Init()
{
	//初始化SPI1中断；初始化设备SPI1；
	//stmSpi1DevInit();
	//PIO_InitializeInterrupts(0);
	//PIO_Configure(&Pin_Stm32Int, 1);
	//intConnect(INUM_TO_IVEC(AT91C_ID_PIOC),tSpi1StmAskIsr,(int)AT91C_BASE_PIOC);
	//intEnable((int)(AT91C_ID_PIOC));
	//PIO_EnableIt(&Pin_Stm32Int);
	
	//if(false == stmSpi1DevInit())
	if(false == InitIoPinConfig())
	{
		printf("stm ----- \n");
		return false;
	}

	if(false == initSpiDev())
	{
		printf(" stm spi dev init\n");
		return false;
	}

	//创建SPI1扩展串口1(USART1)数据接收环形缓冲
	RngSpi1Usart1=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart1)
		printf("Error! Create 'RngSpi1Usart1' failed! \n");
	else	
		rngFlush(RngSpi1Usart1);

	//创建SPI1扩展串口2(USART2)数据接收环形缓冲
	RngSpi1Usart2=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart2)
		printf("Error! Create 'RngSpi1Usart2' failed! \n");
	else
		rngFlush(RngSpi1Usart2);

	//创建SPI1扩展串口3(USART3)数据接收环形缓冲
	RngSpi1Usart3=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart3)
		printf("Error! Create 'RngSpi1Usart3' failed! \n");
	else	
		rngFlush(RngSpi1Usart3);

	//创建SPI1扩展串口4(UART4)数据接收环形缓冲
	RngSpi1Uart4=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Uart4)	
		printf("Error! Create 'RngSpi1Uart4' failed! \n");
	else	
		rngFlush(RngSpi1Uart4);

	//创建SPI1扩展串口5(UART5)数据接收环形缓冲
	RngSpi1Uart5=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Uart5)	
		printf("Error! Create 'RngSpi1Uart5' failed! \n");
	else	
		rngFlush(RngSpi1Uart5);

	//创建SPI1扩展串口6(USART6)数据接收环形缓冲
	RngSpi1Usart6=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart6)	
		printf("Error! Create 'RngSpi1Usart6' failed! \n");
	else	
		rngFlush(RngSpi1Usart6);

	int nRet = -1;

	//初始化SPI1通讯信号量USART1，该信号量保护接收环形缓冲区
	nRet = sem_init(&SemIdSpi1Usart1,0,1);  //空闲
	if(nRet != 0)
	{
	    printf("Error! Create semaphore 'SemIdSpi1Usart1' failed!\n");
		return false;
	}

	nRet = sem_init(&SemIdSpi1Usart2,0,1);
	if(nRet != 0)
	{
    	printf("Error! Create semaphore 'SemIdSpi1Usart2' failed!\n");
		return false;
	}

	nRet = sem_init(&SemIdSpi1Usart3,0,1);
	if(nRet != 0)
	{
	    printf("Error! Create semaphore 'SemIdSpi1Usart3' failed!\n");
		return false;
	}

	nRet = sem_init(&SemIdSpi1Uart4,0,1);
	if(nRet != 0)
	{
	   printf("Error! Create semaphore 'SemIdSpi1Uart4' failed!\n");
	   return false;
	}

	nRet = sem_init(&SemIdSpi1Uart5,0,1);
	if(nRet != 0)
	{
	    printf("Error! Create semaphore 'SemIdSpi1Uart5' failed!\n");
		return false;
	}

    nRet = sem_init(&SemIdSpi1Usart6,0,1);
	if(nRet != 0)
	{
	    printf("Error! Create semaphore 'SemIdSpi1Usart6' failed!\n");
		return false;
	}
	
	lstInit(&spi1ListTx);//创建发送缓存链表

//	RtcTime.century = 0x20;
//	RtcTime.year = 0x17;
//	RtcTime.month = 0x09;
//	RtcTime.date = 0x30;
//	RtcTime.hour = 0x16;
//	RtcTime.minute = 0x56;
//	RtcTime.second = 0x59;

	return true;
}

/*
void spi1Init()
{
	//初始化SPI1中断；初始化设备SPI1；
	stmSpi1DevInit();
	PIO_InitializeInterrupts(0);
	PIO_Configure(&Pin_Stm32Int, 1);
	intConnect(INUM_TO_IVEC(AT91C_ID_PIOC),tSpi1StmAskIsr,(int)AT91C_BASE_PIOC);
	intEnable((int)(AT91C_ID_PIOC));
	PIO_EnableIt(&Pin_Stm32Int);

	//创建SPI1扩展串口1(USART1)数据接收环形缓冲
	RngSpi1Usart1=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart1)
		printf("Error! Create 'RngSpi1Usart1' failed! \n");
	else	
		rngFlush(RngSpi1Usart1);

	//创建SPI1扩展串口2(USART2)数据接收环形缓冲
	RngSpi1Usart2=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart2)
		printf("Error! Create 'RngSpi1Usart2' failed! \n");
	else
		rngFlush(RngSpi1Usart2);

	//创建SPI1扩展串口3(USART3)数据接收环形缓冲
	RngSpi1Usart3=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart3)
		printf("Error! Create 'RngSpi1Usart3' failed! \n");
	else	
		rngFlush(RngSpi1Usart3);

	//创建SPI1扩展串口4(UART4)数据接收环形缓冲
	RngSpi1Uart4=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Uart4)	
		printf("Error! Create 'RngSpi1Uart4' failed! \n");
	else	
		rngFlush(RngSpi1Uart4);

	//创建SPI1扩展串口5(UART5)数据接收环形缓冲
	RngSpi1Uart5=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Uart5)	
		printf("Error! Create 'RngSpi1Uart5' failed! \n");
	else	
		rngFlush(RngSpi1Uart5);

	//创建SPI1扩展串口6(USART6)数据接收环形缓冲
	RngSpi1Usart6=rngCreate(STM_SPI1_RNG_LEN);
	if(NULL==RngSpi1Usart6)	
		printf("Error! Create 'RngSpi1Usart6' failed! \n");
	else	
		rngFlush(RngSpi1Usart6);


	//初始化SPI1通讯信号量USART1，该信号量保护接收环形缓冲区
	SemIdSpi1Usart1=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Usart1)	printf("Error! Create semaphore 'SemIdSpi1Usart1' failed!\n");

	//初始化SPI1通讯信号量USART2，该信号量保护接收环形缓冲区
	SemIdSpi1Usart2=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Usart2)	printf("Error! Create semaphore 'SemIdSpi1Usart2' failed!\n");

	//初始化SPI1通讯信号量USART3，该信号量保护接收环形缓冲区
	SemIdSpi1Usart3=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Usart3)	printf("Error! Create semaphore 'SemIdSpi1Usart3' failed!\n");

	//初始化SPI1通讯信号量UART4，该信号量保护接收环形缓冲区
	SemIdSpi1Uart4=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Uart4)		printf("Error! Create semaphore 'SemIdSpi1Uart4' failed!\n");

	//初始化SPI1通讯信号量UART5，该信号量保护接收环形缓冲区
	SemIdSpi1Uart5=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Uart5)		printf("Error! Create semaphore 'SemIdSpi1Uart5' failed!\n");

	//初始化SPI1通讯信号量USART6，该信号量保护接收环形缓冲区
	SemIdSpi1Usart6=semCCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL==SemIdSpi1Usart6)	printf("Error! Create semaphore 'SemIdSpi1Usart6' failed!\n");

	//创建发送缓存链表
	lstInit(&spi1ListTx);

	//RTC设置操作互斥信号量
	SemIdSpi1RtcSet=semMCreate(SEM_Q_FIFO);
	if(NULL==SemIdSpi1RtcSet)	printf("Error! Create semaphore 'SemIdSpi1RtcSet' failed!\n");

	//RTC读取操作互斥信号量
	SemIdSpi1RtcRead=semMCreate(SEM_Q_FIFO);
	if(NULL==SemIdSpi1RtcRead)	printf("Error! Create semaphore 'SemIdSpi1RtcRead' failed!\n");

	//初始化SPI1通讯信号量，该信号量触发任务发送数据
	SemIdSpi1Transmit=semCCreate(SEM_Q_FIFO, SEM_EMPTY);
	if(NULL==SemIdSpi1Transmit)	printf("Error! Create semaphore 'SemIdSpi1Transmit' failed!\n");

	//初始化SPI1发送及接收任务
	//tIdSpi1=taskSpawn("tSpi1Transmit", 150, 0, 0x4000, (FUNCPTR)tSpi1Transmit, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
	//if(!(OK==taskIdVerify(tIdSpi1)))	printf("Error! Create task 'tSpi1Transmit' failed!\n");

	return;
}*/


 
// ****************************************************************************
// *Name				:spi1Exit
// *Description		:spi1模块注销
// *Input				:None
// *Output			:None
// *Return				:None
// *History			:2013-07-01,modified by syj
// */
// void spi1Exit()
// {
// 	//删除STM32参数定时读取看门狗定时器
// 	if(!(NULL==WdgIdStmPrmRead))
// 	{
// 		wdCancel(WdgIdStmPrmRead);	
// 		wdDelete(WdgIdStmPrmRead);	WdgIdStmPrmRead=NULL;
// 	}
// 
// 	//删除SPI1通讯任务
// 	if(OK==taskIdVerify(tIdSpi1))
// 	{
// 		taskDelete(tIdSpi1);	tIdSpi1=0;
// 	}
// 
// 	//删除SPI1读取RTC互斥信号量
// 	if(!(NULL==SemIdSpi1RtcRead))
// 	{
// 		semDelete(SemIdSpi1RtcRead);	SemIdSpi1RtcRead=NULL;
// 	}
// 
// 	//删除SPI1设置RTC互斥信号量
// 	if(!(NULL==SemIdSpi1RtcSet))
// 	{
// 		semDelete(SemIdSpi1RtcSet);	SemIdSpi1RtcSet=NULL;
// 	}
// 
// 	//删除SPI1通讯信号量
// 	if(!(NULL==SemIdSpi1Transmit))
// 	{
// 		semDelete(SemIdSpi1Transmit);	SemIdSpi1Transmit=NULL;
// 	}
// 
// 
// 
// /删除环形缓冲区
// 	if(!(NULL==RngSpi1Usart1))	
// 	{
// 		rngDelete(RngSpi1Usart1);	RngSpi1Usart1=NULL;
// 	}
// 	////
// 	///删除环形缓冲区
// 	if(!(NULL==RngSpi1Usart2))	
// 	{
// 		rngDelete(RngSpi1Usart2);	RngSpi1Usart2=NULL;
// 	}
// 	////
// 	///删除环形缓冲区
// 	if(!(NULL==RngSpi1Usart3))	
// 	{
// 		rngDelete(RngSpi1Usart3);	RngSpi1Usart3=NULL;
// 	}
// 	////
// 	///删除环形缓冲区
// 	if(!(NULL==RngSpi1Uart4))	
// 	{
// 		rngDelete(RngSpi1Uart4);	RngSpi1Uart4=NULL;
// 	}
// 	////
// 	///删除环形缓冲区
// 	if(!(NULL==RngSpi1Uart5))	
// 	{
// 		rngDelete(RngSpi1Uart5);	RngSpi1Uart5=NULL;
// 	}
// 	////
// 	///删除环形缓冲区
// 	if(!(NULL==RngSpi1Usart6))	
// 	{
// 		rngDelete(RngSpi1Usart6);	RngSpi1Usart6=NULL;
// 	}
// 	////
// 
// 释放缓存链表所有节点
// 	lstFree(&spi1ListTx);
// 
// 禁止中断
// 	PIO_DisableIt(&Pin_Stm32Int);
// 	intDisable((int)(AT91C_ID_PIOC));
// 	
// 	
// 	return;
// }
// 
// 
// 
// 
// 
// 
// 
