//#include "../inc/main.h"
//#include "../inc/oilCfg.h"
//#include "../inc/oilFRam.h"
//#include "../inc/oilFile.h"
//#include <pthread.h>

#include "../inc/main.h"


pthread_mutex_t lock;

#define LED_MAGIC  'L'
#define NO_PROTECT      _IO(LED_MAGIC,0) 
#define HIGH4_PROTECT   _IO(LED_MAGIC,1) 
#define HIGH2_PROTECT   _IO(LED_MAGIC,2) 
#define ALL_PROTECT     _IO(LED_MAGIC,3) 


static int g_nfdFram = -1;

/*********************************************************************************************************
**函数名称： unsigned char FM25CLxx_RDByte(void)
**函数功能： 读字节
**入口参数：
**          
**说明：
********************************************************************************************************/
static unsigned char FM25CLxx_RDByte(void)
{	   
	//unsigned char i=0,j=0;
	unsigned char dat=0;                  
                 
	/*for (j = 0; j <= 7; j++)
	{             
		dat = dat << 1; 
	
		if(1==FM25CLxx_SO_READ)	dat = dat | 0x01;
		else											dat = dat & (~1);
		
		FM25CLxx_SCK_H;    
		for(i=0;i<1;i++); //延迟600ns
		FM25CLxx_SCK_L;
		for(i=0;i<1;i++);
	}*/
		
	return(dat);
}
/*********************************************************************************************************
**函数名称：void FM25CLxx_WRByte(unsigned char dat)
**函数功能： 写字节
**入口参数：
**          
**说明：
********************************************************************************************************/
static void FM25CLxx_WRByte(unsigned char dat)
{	   
	unsigned char i=0,j=0;

	for (j = 0; j <= 7; j++)
	{                                
		if((dat & 0x80) == 0) 	FM25CLxx_SI_L;
		else 					FM25CLxx_SI_H;
		
		FM25CLxx_SCK_H;    //<20MHz
		for(i=0;i<1;i++);
		FM25CLxx_SCK_L;
		for(i=0;i<1;i++);
		
		dat = dat << 1;
	}
	
	return;
}
/*********************************************************************************************************
**函数名称： unsigned char FM25CLxx_RDSR(void)
**函数功能： 读状态字
**入口参数：
**          
**说明：
*********************************************************************************************************/
static unsigned char FM25CLxx_RDSR(void)
{	   
   unsigned char dat,i;
   FM25CLxx_CS_L;  
   for(i=0;i<1;i++);
   FM25CLxx_WRByte(CMD_RDSR);
   dat=FM25CLxx_RDByte();

   FM25CLxx_CS_H; 
   for(i=0;i<1;i++);
   return(dat);	 
}
/*********************************************************************************************************
**函数名称：void FM25CLxx_WREn(unsigned char en)
**函数功能：1 写使能  0  禁止 
**入口参数：
**          
**说明：
********************************************************************************************************/
static void FM25CLxx_WREn(unsigned char en)
{	   
	unsigned char i;
	FM25CLxx_CS_L;
	for(i=0;i<1;i++);
	if(en)FM25CLxx_WRByte(CMD_WREN);
	else FM25CLxx_WRByte(CMD_WRDI);
	FM25CLxx_CS_H; 
   	for(i=0;i<1;i++);

	return;
}
/*********************************************************************************************************
**函数名称：void FM25CLxx_WRSR(unsigned char dat)
**函数功能：写状态字
**入口参数：
**          
**说明：
********************************************************************************************************/
static void FM25CLxx_WRSR(unsigned char dat)
{	   
	int i=0;
 
	FM25CLxx_WREn(1);
	FM25CLxx_CS_L;		//gly20130401
	for(i=0; i<5; i++)	;

	FM25CLxx_WRByte(CMD_WRSR);
	FM25CLxx_WRByte(dat);
	FM25CLxx_WREn(0);
	FM25CLxx_CS_H; 
   	for(i=0; i<5; i++)	;

 	return;
}
/*********************************************************************************************************
**函数名称：void FM25CLxx_AreaWP(unsigned char area)
**函数功能： WP有效		区域写保护
            0 不保护 
			1 高1/4保护 
			2 高1/2保护 
			3 全保护 
**入口参数：
**          
**说明：
********************************************************************************************************/
static void FM25CLxx_AreaWP(unsigned char area)
{	   
   switch(area)
	{				 
	case 0:
		//FM25CLxx_WRSR(0x80);
		ioctl(g_nfdFram,NO_PROTECT);
		break;
	case 1:
		ioctl(g_nfdFram,HIGH4_PROTECT);
		//FM25CLxx_WRSR(0x84);
		break;
	case 2:
		ioctl(g_nfdFram,HIGH2_PROTECT);
		//FM25CLxx_WRSR(0x88);
		break;
	case 3:
		ioctl(g_nfdFram,ALL_PROTECT);
		//FM25CLxx_WRSR(0x8c); 
		break;
	default:
		break;
	}

	return;
}

/*********************************************************************************************************
**函数名称：void FM25CLxx_RD(unsigned short adr,unsigned short n,unsigned char *dat)
**函数功能： 从adr地址读n个数放到dat开始地址
**入口参数：   wp脚使能	  不保护所有区域
**        adr   读地址
          n     字节数
		  dat   指针  
**说明：
********************************************************************************************************/
static void FM25CLxx_RD(unsigned char *dat,unsigned short adr,unsigned short n)
{  
	int i=0;
			 
   if((n==0) || (adr+n)>FM25CLxx_Size)return;
   FM25CLxx_WREn(1);
   FM25CLxx_CS_L;
	for(i=0; i<5; i++)	;
   FM25CLxx_WRByte(CMD_READ);
   FM25CLxx_WRByte(adr>>8);
   FM25CLxx_WRByte(adr);
 
   while(n--)
	{
		*dat=FM25CLxx_RDByte();
		dat++;
   }
	FM25CLxx_CS_H;
	FM25CLxx_WREn(0);

	return;
}

/*********************************************************************************************************
**函数名称：void FM25CLxx_WR(unsigned short adr,unsigned short n,unsigned char *dat)
**函数功能： 线adr地址写n个数 dat 为要写入的数据的起址
**入口参数：   wp脚使能	  不保护所有区域
**        adr   读地址
          n     字节数
		  dat   指针  
**说明：
********************************************************************************************************/
static void FM25CLxx_WR(unsigned char *dat,unsigned short adr,unsigned short n)
{	   
   if( n==0 || (adr+n)>FM25CLxx_Size)return;
 	FM25CLxx_WREn(1);
	FM25CLxx_CS_L;

	FM25CLxx_WRByte(CMD_WRITE);
	FM25CLxx_WRByte(adr>>8);
	FM25CLxx_WRByte(adr);
   
   while(n--)
	{
		FM25CLxx_WRByte(*dat);
		dat++;
	}

	FM25CLxx_CS_H;
	FM25CLxx_WREn(0);

	return;
}

/***************************************
作用：往铁电里写入数据
返回值：0=成功，其它值=失败，错误值，由当时项目决定
nAddr ：数据写入地址
pbyWriteBuf：写入数据缓冲区地址
nWriteLen:写入数据的长度

地址空间：0x0000-0x1FFF
***************************************/
int FMWrite( int nAddr, unsigned char * pbyWriteBuf, int nWriteLen)
{
   /*if( nWriteLen==0 || (nAddr+nWriteLen)>FM25CLxx_Size)return -1;
   FM25CLxx_WREn(1);
   FM25CLxx_CS_L;

   FM25CLxx_WRByte(CMD_WRITE);
   FM25CLxx_WRByte(nAddr>>8);
   FM25CLxx_WRByte(nAddr);
   
   while(nWriteLen--)
	{
		FM25CLxx_WRByte(*pbyWriteBuf);
		pbyWriteBuf++;
	}

	FM25CLxx_CS_H;
	FM25CLxx_WREn(0);*/

	lseek(g_nfdFram,nAddr,SEEK_SET);
	int nRet = write(g_nfdFram,pbyWriteBuf,nWriteLen);
	if(nRet < 0)
	{
		perror("write /dev/FRAMdev");
	}

	return nRet; 
}

/***************************************
作用：读取铁电数据内容
返回值：0=成功，其它值=失败，错误值，由当时项目决定
nAddr ：读出数据的地址
pbyReadBuf：读出数据缓冲区地址
nReadLen:读出数据的长度

地址空间：0x0000-0x1FFF
***************************************/
int FMRead( int nAddr, unsigned char * pbyReadBuf, int nReadLen)
{
/*	int i=0;

   if((nReadLen==0) || (nAddr+nReadLen)>FM25CLxx_Size)return -1;
   FM25CLxx_WREn(1);
   FM25CLxx_CS_L;													
	for(i=0; i<5; i++);
   FM25CLxx_WRByte(CMD_READ);
   FM25CLxx_WRByte(nAddr>>8);
   FM25CLxx_WRByte(nAddr);
   
   while(nReadLen--)
	{
		*pbyReadBuf=FM25CLxx_RDByte();
		pbyReadBuf++;
	}
   FM25CLxx_CS_H;
   FM25CLxx_WREn(0);*/

	lseek(g_nfdFram,nAddr,SEEK_SET);
	int nRet = read(g_nfdFram,pbyReadBuf,nReadLen);
	if(nRet < 0)
	{
		perror("read /dev/FRAMdev");
	}

	return nRet;
}


/********************************************************************
*Name				:framRead
*Description		:铁电存储器FM25CL64数据读取
*Input				:base_addr		数据操作基址，FM_ADDR_CONFIG/FM_ADDR_JL/FM_ADDR_IPT_CNPC/FM_ADDR_IPT_SINO/FM_ADDR_IPT_NONCONTACT/FM_ADDR_PCD_CNPC/FM_ADDR_NET_SINO/FM_ADDR_NET_NONCONTACT
*					:offfset_addr	数据操作偏移地址
*					:buffer				数据存储缓存地址
*					:maxbytes		数据存储最大长度
*Output				:None
*Return				:0=成功；其它值=失败
*History			:2013-07-01,modified by syj
*/
int framRead(int base_addr, int offset_addr, unsigned char *buffer, int maxbytes)
{
	int i=0, istate=0, maxsize=0;

	pthread_mutex_lock(&lock);

	//判断读取的目的铁电段
	if(FM_ADDR_CONFIG==base_addr)			
		maxsize=FM_SIZE_CONFIG;
	else if(FM_ADDR_JL==base_addr)					
		maxsize=FM_SIZE_JL;	
	else if(FM_ADDR_IPT_CNPC==base_addr)	
		maxsize=FM_SIZE_IPT_CNPC;
	else if(FM_ADDR_IPT_SINO==base_addr)		
		maxsize=FM_SIZE_IPT_SINO;
	else if(FM_ADDR_IPT_NONCONTACT==base_addr)
		maxsize=FM_SIZE_IPT_NONCONTACT;
	else if(FM_ADDR_PCD_CNPC==base_addr)	
		maxsize=FM_SIZE_PCD_CNPC;
	else if(FM_ADDR_PCD_SINO==base_addr)		
		maxsize=FM_SIZE_PCD_SINO;
	else if(FM_ADDR_PCD_NONCONTACT==base_addr)	
		maxsize=FM_SIZE_PCD_NONCONTACT;
	else												
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//判断长度
	if((0==maxbytes)||(offset_addr+maxbytes>maxsize))	
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//读出数据
	i=FMRead(base_addr+offset_addr, buffer, maxbytes);
	//if(0!=i)	
	//{
	//	istate=ERROR;	
	//	goto IRETURN;
	//}
	if(i <= 0)  //fj:20171026
	{
		istate = ERROR;
		goto IRETURN;
	}
IRETURN:
	pthread_mutex_unlock(&lock);

	return istate;
}


/********************************************************************
*Name				:framWrite
*Description		:铁电存储器FM25CL64数据写入，不超过本块最大长度，单次写入不超过2K
*Input				:base_addr		数据操作基址，FM_ADDR_CONFIG/FM_ADDR_JL/FM_ADDR_IPT_CNPC/FM_ADDR_IPT_SINO/FM_ADDR_IPT_NONCONTACT/FM_ADDR_PCD_CNPC/FM_ADDR_PCD_SINO/FM_ADDR_PCD_NONCONTACT
*					:offfset_addr	数据操作偏移地址
*					:buffer				数据写入缓存地址
*					:maxbytes		数据写入长度
*Output				:None
*Return				:0=成功，其它值=失败
*History			:2013-07-01,modified by syj
*/
int framWrite(int base_addr, int offset_addr, unsigned char *buffer, int nbytes)
{
	unsigned char rdbuffer[2048]={0};
	int i=0, maxsize=0, istate=0;

	pthread_mutex_lock(&lock);

	//判断长度
	if(nbytes>2048)	
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//判断写入的目的铁电段
	if(FM_ADDR_CONFIG==base_addr)					
		maxsize=FM_SIZE_CONFIG;
	else if(FM_ADDR_JL==base_addr)
		maxsize=FM_SIZE_JL;
	else if(FM_ADDR_IPT_CNPC==base_addr)		
		maxsize=FM_SIZE_IPT_CNPC;
	else if(FM_ADDR_IPT_SINO==base_addr)
		maxsize=FM_SIZE_IPT_SINO;
	else if(FM_ADDR_IPT_NONCONTACT==base_addr)
		maxsize=FM_SIZE_IPT_NONCONTACT;
	else if(FM_ADDR_PCD_CNPC==base_addr)
		maxsize=FM_SIZE_PCD_CNPC;
	else if(FM_ADDR_PCD_SINO==base_addr)				
		maxsize=FM_SIZE_PCD_SINO;
	else if(FM_ADDR_PCD_NONCONTACT==base_addr)		
		maxsize=FM_SIZE_PCD_NONCONTACT;
	else			
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//判断写入地址及长度
	if((0==nbytes)||(offset_addr+nbytes>maxsize))		
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//写入
	i=FMWrite(base_addr+offset_addr, buffer, nbytes);
	//if(0!=i)	
	if(i <= 0) //fj:20171026
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//读出
	i=FMRead(base_addr+offset_addr, rdbuffer, nbytes);
	//if(0!=i)
	if(i <= 0) //fj:20171026
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//校验
	if(0!=memcmp(buffer, rdbuffer, nbytes))				
	{
		istate=ERROR;	
		goto IRETURN;
	}

	//释放信号量
IRETURN:
	pthread_mutex_unlock(&lock);

	return istate;
}

void fmTest();

/********************************************************************
*Name			:framDevInit
*Description	:铁电存储器FM25CL64初始化
*Input			:None
*Output			:None
*Return			:None
*History		:2013-07-01,modified by syj
*/
bool framDevInit()
{
	g_nfdFram = open("/dev/FRAMdev",O_RDWR);
	if(g_nfdFram < 0)
	{
		printf("open FRAMdev file failure\n");
		return false;
	}

	FM25CLxx_AreaWP(0);//不保护

	int nRet = pthread_mutex_init(&lock, NULL);
	if(nRet < 0)
	{
	    printf("Error! Create List 'semIdFM' failed!\n");
		return false;
	}

	//fmTest();

	return true;
}


void fmTest()
{
	int i=0;

	//for(i=0; i<1024; i++)
	//	{
	//FMWrite(i*8, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
	//}
    int nRet = FMWrite(0x0000, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
    nRet = FMWrite(0x1C00, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);


    //int nRet = FMWrite(0x1C00, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);
	 if(nRet < 0)
	 {
		 printf("write /dev/FRAMdev is error\n");
	 }

	 printf("write fram is success,len = %d\n",nRet);


	 unsigned char buff[8];
	 nRet = FMRead(0x1C00,buff,8);
	 if(nRet < 0)
	 {
		 printf("read /dev/FRAMdev is error\n");
	 }

	 PrintH(nRet,buff);

	return;
}

/*
void fmTest()
{
	int i=0;
	unsigned char wr[16]={0x01,0x03,0x05,0x07,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x55,0x66,0x77,0x88,0x99}; 
	unsigned char rd[16]={0};

	framDevInit();
	taskDelay(20);

		printf("FM_SIZE_CONFIGframWrite kkkk\n");
	for(i=0; i<FM_SIZE_CONFIG; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_CONFIG, i, wr, 16)))	printf("FM_ADDR_CONFIG timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_CONFIG, i, rd, 16)))	printf("FM_ADDR_CONFIG timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}
		
	printf("FM_SIZE_JL	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_CONFIG; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_JL, i, wr, 16)))	printf("FM_ADDR_JL	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_JL, i, rd, 16)))	printf("FM_ADDR_JL	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

		printf("FM_SIZE_IPT_CNPC	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_IPT_CNPC; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_IPT_CNPC, i, wr, 16)))	printf("FM_ADDR_IPT_CNPC	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_IPT_CNPC, i, rd, 16)))	printf("FM_ADDR_IPT_CNPC	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

	printf("FM_SIZE_IPT_SINO	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_IPT_SINO; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_IPT_SINO, i, wr, 16)))	printf("FM_ADDR_IPT_SINO	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_IPT_SINO, i, rd, 16)))	printf("FM_ADDR_IPT_SINO	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

	
	printf("FM_SIZE_IPT_NONCONTACT	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_IPT_NONCONTACT; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_IPT_NONCONTACT, i, wr, 16)))	printf("FM_ADDR_IPT_NONCONTACT	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_IPT_NONCONTACT, i, rd, 16)))	printf("FM_ADDR_IPT_NONCONTACT	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

	printf("FM_SIZE_PCD_CNPC	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_PCD_CNPC; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_PCD_CNPC, i, wr, 16)))	printf("FM_ADDR_PCD_CNPC	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_PCD_CNPC, i, rd, 16)))	printf("FM_ADDR_PCD_CNPC	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

		printf("FM_SIZE_NET_SINO	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_NET_SINO; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_NET_SINO, i, wr, 16)))	printf("FM_ADDR_NET_SINO	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_NET_SINO, i, rd, 16)))	printf("FM_ADDR_NET_SINO	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

	printf("FM_SIZE_NET_NONCONTACT	framWrite kkkk\n");
	for(i=0; i<FM_SIZE_NET_NONCONTACT; i+=16)
		{
			if(!(16==framWrite(FM_ADDR_NET_NONCONTACT, i, wr, 16)))	printf("FM_ADDR_NET_NONCONTACT	framWrite timout................................nnnn\n");
			taskDelay(1);		
			if(!(16==framRead(FM_ADDR_NET_NONCONTACT, i, rd, 16)))	printf("FM_ADDR_NET_NONCONTACT	framRead timout................................nnnn\n");

			if(memcmp(wr, rd, 16)==0)	printf("%4x fm right.......................\n", i);
			else											printf("%4x fm wrong.fffffffffffffffffffffffffffff......................\n", i);

			memset(rd, 0, 16);
		}

	return;
}
//*/



