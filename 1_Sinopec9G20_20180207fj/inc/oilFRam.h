#ifndef _OIL_FRAM_H_
#define _OIL_FRAM_H_

//yym #include "oilPio.h"
//yym #include "AT91SAM9G20.h"


/* �궨��*/
#define FM25CLxx_CS_H			/*pioSet(AT91C_BASE_PIOC, BIT2)*/							//CS���ø�(PC2)		
#define FM25CLxx_CS_L			/*pioReset(AT91C_BASE_PIOC, BIT2)*/							//CS���õ�(PC2)
#define FM25CLxx_SCK_H			/*pioSet(AT91C_BASE_PIOC, BIT9)*/							//SCK���ø�(PC9)
#define FM25CLxx_SCK_L			/*pioReset(AT91C_BASE_PIOC, BIT9)*/							//SCK���õ�(PC9)
#define FM25CLxx_SI_H			/*pioSet(AT91C_BASE_PIOA, BIT5)*/							//SI���ø�(PA5)
#define FM25CLxx_SI_L			/*pioReset(AT91C_BASE_PIOA, BIT5)*/							//SI���õ�(PA5)
#define FM25CLxx_SO_READ		/*pioReadBit( AT91C_BASE_PIOA,  BIT4)*/						//SO�����ݶ�ȡ(PA4)
//------------------------------------------------------------------------
#define CMD_WRITE				0x02  	/* Write to Memory instruction */
#define CMD_WRSR				0x01  	/* Write Status Register instruction */
#define CMD_WREN				0x06  	/* Write enable instruction */
#define CMD_WRDI				0x04  	/* Write dis instruction */
#define CMD_READ				0x03  	/* Read from Memory instruction */
#define CMD_RDSR				0x05  	/* Read Status Register instruction  */	

#define FM25CLxx_Size			8192

/*��ģ������ʹ�������ַ����С*/
#define FM_ADDR_CONFIG						0x0000		//������Ϣ��ʼ��ַ
#define FM_ADDR_JL							0x0400		//����ģ����ʼ��ַ
#define FM_ADDR_IPT_CNPC					0x0800		//IC֧��ģ����ʼ��ַ����ʯ��Ӧ��
#define FM_ADDR_IPT_SINO					0x0C00		//IC֧��ģ����ʼ��ַ����ʯ��Ӧ��
#define FM_ADDR_IPT_NONCONTACT				0x1000		//IC֧��ģ����ʼ��ַ���ǽӴ�Ӧ��
#define FM_ADDR_PCD_CNPC					0x1400		//ʯ�Ϳ�������PCDģ��
#define FM_ADDR_PCD_SINO					0x1C00		//ʯ����������
#define FM_ADDR_PCD_NONCONTACT				0x1E00		//ͨ�÷ǽӴ�

#define FM_SIZE_CONFIG						0x0400		//������Ϣռ�ô�СFM_SIZE_CONFIG
#define FM_SIZE_JL							0x0400		//����ģ��ռ�ô�С
#define FM_SIZE_IPT_CNPC					0x0400		//IC֧��ģ��ռ�ô�С����ʯ��Ӧ��
#define FM_SIZE_IPT_SINO					0x0400		//IC֧��ģ��ռ�ô�С����ʯ��Ӧ��
#define FM_SIZE_IPT_NONCONTACT				0x0400		//IC֧��ģ��ռ�ô�С���ǽӴ�Ӧ��
#define FM_SIZE_PCD_CNPC					0x0800		//ʯ�Ϳ�������PCDģ��
#define FM_SIZE_PCD_SINO					0x0200		//ʯ����������
#define FM_SIZE_PCD_NONCONTACT				0x0200		//ͨ�÷ǽӴ�


/*��������*/
extern int framRead(int base_addr, int offset_addr, unsigned char *buffer, int maxbytes);
extern int framWrite(int base_addr, int offset_addr, unsigned char *buffer, int nbytes);
extern bool framDevInit(void);


int FMWrite( int nAddr, unsigned char * pbyWriteBuf, int nWriteLen);
int FMRead( int nAddr, unsigned char * pbyReadBuf, int nReadLen);
#endif

