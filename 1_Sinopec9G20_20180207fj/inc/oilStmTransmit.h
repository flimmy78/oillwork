#ifndef _OIL_STM_TRANSMIT_H_
#define _OIL_STM_TRANSMIT_H_

//#include "oilPio.h"

//关于SPI1通讯的一些宏定义
#define STM_SPI1_PORT_SET					1				//查询/设置端口
#define STM_SPI1_PORT_USART1				2			 	//ARM7串口1
#define STM_SPI1_PORT_USART2				3			 	//ARM7串口2
#define STM_SPI1_PORT_USART3				4			 	//ARM7串口3
#define STM_SPI1_PORT_UART4					5			 	//ARM7串口4
#define STM_SPI1_PORT_UART5					6			 	//ARM7串口5
#define STM_SPI1_PORT_USART6				7			 	//ARM7串口6
#define STM_SPI1_PORT_CAN1					8			 	//ARM7 CAN1
#define STM_SPI1_PORT_CAN2					9			 	//ARM7 CAN2
#define STM_SPI1_PORT_TIME					10				//时钟端口
#define STM_SPI1_PORT_IOWRITE				11				//写IO端口
#define STM_SPI1_PORT_IOREAD				12				//读IO端口

#define STM_SPI1_NODE_MAX					1024
#define STM_SPI1_MAX_LEN					128			//stm32返回数据缓存最大长度
#define STM_SPI1_RNG_LEN					2048			//stm_spi1接收缓冲长度
#define STM_SPI1_WDG_TIME					(50*ONE_MILLI_SECOND)	//定时向stm32发查询命令
//#define STM_SPI1_IO_DEBOUNCE				5				//IO消抖次数
#define STM_SPI1_IO_DEBOUNCE				10				//IO消抖次数

#define STM_MC1							stmSpi1IoRead(34)							//A面电机允许信号；0=允许，1=不允许
#define STM_MC2							stmSpi1IoRead(35)							//B面电机允许信号；0=允许，1=不允许

#define STM_SLAVE_ENABLE()				pioReset(AT91C_BASE_PIOB, BIT3)		//复位片选，使能从机
#define STM_SLAVE_DISABLE()				pioSet(AT91C_BASE_PIOB, BIT3)			//拉高片选，禁用从机


//时间结构定义
typedef struct {
	unsigned char century;   		// CC   century - [20,21] 
	unsigned char year;    			// YY   Year value - [0,99] 
	unsigned char month;    		// MM	Month value - [1,12] 
	unsigned char date;    			// DD	Day of the month value - [1,31] 
	unsigned char hour;     			// HH	Hour value - [0,23] 
	unsigned char minute;   		// MM	Minute value - [0,59] 
	unsigned char second;   		// SS	Second value - [0,59] 
   unsigned char week;    			// WW	WEEK - [1,7] 
   unsigned char param[12]; 		// 07H-12H寄存器参数 
}RTCTime;


//时间及IO状态采集定时器
extern unsigned int spi1TransTimer;

//函数声明
extern int spi1PumpPermitRead(int dev_num, char *chg);
extern int spi1GunRead(int gun_num, char *chg);
extern int spi1LockRead(int *chg);
extern int spi1UartWrite(int uartx, char *buffer, int nbytes);
extern int spi1UartRead(int uartx, char *buffer, int maxbytes);
extern int timeWrite(RTCTime time);
extern int timeRead(RTCTime *time);
extern bool spi1Init(void);
// extern void spi1Exit(void);
extern void tSpi1Transmit();

#endif

