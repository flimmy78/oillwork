#ifndef _OIL_FRAM_H_
#define _OIL_FRAM_H_

//yym #include "oilPio.h"
//yym #include "AT91SAM9G20.h"


/* 宏定义*/
#define FM25CLxx_CS_H			/*pioSet(AT91C_BASE_PIOC, BIT2)*/							//CS线置高(PC2)		
#define FM25CLxx_CS_L			/*pioReset(AT91C_BASE_PIOC, BIT2)*/							//CS线置低(PC2)
#define FM25CLxx_SCK_H			/*pioSet(AT91C_BASE_PIOC, BIT9)*/							//SCK线置高(PC9)
#define FM25CLxx_SCK_L			/*pioReset(AT91C_BASE_PIOC, BIT9)*/							//SCK线置低(PC9)
#define FM25CLxx_SI_H			/*pioSet(AT91C_BASE_PIOA, BIT5)*/							//SI线置高(PA5)
#define FM25CLxx_SI_L			/*pioReset(AT91C_BASE_PIOA, BIT5)*/							//SI线置低(PA5)
#define FM25CLxx_SO_READ		/*pioReadBit( AT91C_BASE_PIOA,  BIT4)*/						//SO线数据读取(PA4)
//------------------------------------------------------------------------
#define CMD_WRITE				0x02  	/* Write to Memory instruction */
#define CMD_WRSR				0x01  	/* Write Status Register instruction */
#define CMD_WREN				0x06  	/* Write enable instruction */
#define CMD_WRDI				0x04  	/* Write dis instruction */
#define CMD_READ				0x03  	/* Read from Memory instruction */
#define CMD_RDSR				0x05  	/* Read Status Register instruction  */	

#define FM25CLxx_Size			8192

/*各模块铁电使用区域地址及大小*/
#define FM_ADDR_CONFIG						0x0000		//配置信息起始地址
#define FM_ADDR_JL							0x0400		//计量模块起始地址
#define FM_ADDR_IPT_CNPC					0x0800		//IC支付模块起始地址，中石油应用
#define FM_ADDR_IPT_SINO					0x0C00		//IC支付模块起始地址，中石化应用
#define FM_ADDR_IPT_NONCONTACT				0x1000		//IC支付模块起始地址，非接触应用
#define FM_ADDR_PCD_CNPC					0x1400		//石油卡机链接PCD模块
#define FM_ADDR_PCD_SINO					0x1C00		//石化卡机联动
#define FM_ADDR_PCD_NONCONTACT				0x1E00		//通用非接触

#define FM_SIZE_CONFIG						0x0400		//配置信息占用大小FM_SIZE_CONFIG
#define FM_SIZE_JL							0x0400		//计量模块占用大小
#define FM_SIZE_IPT_CNPC					0x0400		//IC支付模块占用大小，中石油应用
#define FM_SIZE_IPT_SINO					0x0400		//IC支付模块占用大小，中石化应用
#define FM_SIZE_IPT_NONCONTACT				0x0400		//IC支付模块占用大小，非接触应用
#define FM_SIZE_PCD_CNPC					0x0800		//石油卡机链接PCD模块
#define FM_SIZE_PCD_SINO					0x0200		//石化卡机联动
#define FM_SIZE_PCD_NONCONTACT				0x0200		//通用非接触


/*函数声明*/
extern int framRead(int base_addr, int offset_addr, unsigned char *buffer, int maxbytes);
extern int framWrite(int base_addr, int offset_addr, unsigned char *buffer, int nbytes);
extern bool framDevInit(void);


int FMWrite( int nAddr, unsigned char * pbyWriteBuf, int nWriteLen);
int FMRead( int nAddr, unsigned char * pbyReadBuf, int nReadLen);
#endif

