#ifndef _OIL_STM_TRANSMIT_H_
#define _OIL_STM_TRANSMIT_H_

//#include "oilPio.h"

//����SPI1ͨѶ��һЩ�궨��
#define STM_SPI1_PORT_SET					1				//��ѯ/���ö˿�
#define STM_SPI1_PORT_USART1				2			 	//ARM7����1
#define STM_SPI1_PORT_USART2				3			 	//ARM7����2
#define STM_SPI1_PORT_USART3				4			 	//ARM7����3
#define STM_SPI1_PORT_UART4					5			 	//ARM7����4
#define STM_SPI1_PORT_UART5					6			 	//ARM7����5
#define STM_SPI1_PORT_USART6				7			 	//ARM7����6
#define STM_SPI1_PORT_CAN1					8			 	//ARM7 CAN1
#define STM_SPI1_PORT_CAN2					9			 	//ARM7 CAN2
#define STM_SPI1_PORT_TIME					10				//ʱ�Ӷ˿�
#define STM_SPI1_PORT_IOWRITE				11				//дIO�˿�
#define STM_SPI1_PORT_IOREAD				12				//��IO�˿�

#define STM_SPI1_NODE_MAX					1024
#define STM_SPI1_MAX_LEN					128			//stm32�������ݻ�����󳤶�
#define STM_SPI1_RNG_LEN					2048			//stm_spi1���ջ��峤��
#define STM_SPI1_WDG_TIME					(50*ONE_MILLI_SECOND)	//��ʱ��stm32����ѯ����
//#define STM_SPI1_IO_DEBOUNCE				5				//IO��������
#define STM_SPI1_IO_DEBOUNCE				10				//IO��������

#define STM_MC1							stmSpi1IoRead(34)							//A���������źţ�0=����1=������
#define STM_MC2							stmSpi1IoRead(35)							//B���������źţ�0=����1=������

#define STM_SLAVE_ENABLE()				pioReset(AT91C_BASE_PIOB, BIT3)		//��λƬѡ��ʹ�ܴӻ�
#define STM_SLAVE_DISABLE()				pioSet(AT91C_BASE_PIOB, BIT3)			//����Ƭѡ�����ôӻ�


//ʱ��ṹ����
typedef struct {
	unsigned char century;   		// CC   century - [20,21] 
	unsigned char year;    			// YY   Year value - [0,99] 
	unsigned char month;    		// MM	Month value - [1,12] 
	unsigned char date;    			// DD	Day of the month value - [1,31] 
	unsigned char hour;     			// HH	Hour value - [0,23] 
	unsigned char minute;   		// MM	Minute value - [0,59] 
	unsigned char second;   		// SS	Second value - [0,59] 
   unsigned char week;    			// WW	WEEK - [1,7] 
   unsigned char param[12]; 		// 07H-12H�Ĵ������� 
}RTCTime;


//ʱ�估IO״̬�ɼ���ʱ��
extern unsigned int spi1TransTimer;

//��������
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

