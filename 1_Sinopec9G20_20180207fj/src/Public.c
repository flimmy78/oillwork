#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>
#include <net/if_arp.h>


#include "../inc/main.h"

#define ETHER_ADDR_LEN 6
#define UP 1
#define DOWN 0


structMyInformation struMyInf;
structSysConFile struSysConFile;
structProtect g_struProtect[16];

bool g_bIsCalibrate = false;
unsigned char g_uchCalibrateBuff[8];

structFunctionList g_struFunctionList;

bool g_bIsInternalComm = false;
bool g_bIsExternalComm = false;

structSubList g_stru972Ip;









/* CRC 高位字节值表 */
unsigned char const   auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};
//* CRC 低位字节值表
unsigned char const auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40};


unsigned short acsiitodec(char* c)
{
	unsigned short dec=0;
	//printf("%c\n",*c);
	while(*c != '\0'&& *c != '_')
	{
		dec = dec*10;	
		if ((*c >= '0') && (*c <= '9'))
		{
			dec +=  *c - '0';
		}
		c++;
	//	printf("dec=%d\n",dec);
	}

	return dec;

}

unsigned char acsiitohex(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	return 16; /* error */
}

unsigned short crc_check( unsigned char  *puchMsg,unsigned short usDataLen) 
{	 
	unsigned char uchCRCHi = 0xFF ; //高CRC 字节初始化 
	unsigned char uchCRCLo = 0xFF ; // 低CRC 字节初始化 
	unsigned char uIndex ; // CRC 循环中的索引 
	while ( usDataLen--)  // 传输消息缓冲区 
	{					    
		uIndex = uchCRCHi ^ *puchMsg++ ;//*计算CRC 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[ uIndex];
	} 
	return ( uchCRCHi << 8 | uchCRCLo);
}

void PrintTime(char* chSecond,char* chMillisecond)
{
	struct timespec time;
	clock_gettime(CLOCK_REALTIME,&time);
	printf(chSecond,time.tv_sec);
	printf(chMillisecond,time.tv_nsec/1000000);
	//printf(chMillisecond,time.tv_nsec);
}

void PrintH(int nLength,unsigned char* pchBuff)
{
	int nIndex;
	for(nIndex = 0; nIndex < nLength; nIndex++)
	{
		printf("%02x ",pchBuff[nIndex]);
	}
	printf(" \n");
}


int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway)
{
	int fd;
	int rc;
	struct ifreq ifr; 
	struct sockaddr_in *sin;
	struct rtentry  rt;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		perror("socket   error");     
		return -1;     
	}
	memset(&ifr,0,sizeof(ifr)); 
	strcpy(ifr.ifr_name,ifname); 
	sin = (struct sockaddr_in*)&ifr.ifr_addr;     
	sin->sin_family = AF_INET;     
	//ipaddr
	if(inet_aton(Ipaddr,&(sin->sin_addr))< 0)   
	{     
		perror("inet_aton error");     
		return -2;     
	}    

	if(ioctl(fd,SIOCSIFADDR,&ifr)<0)   
	{     
		perror("ioctl SIOCSIFADDRerror");     
		return -3;     
	}
	//netmask
	if(inet_aton(mask,&(sin->sin_addr))	<0)   
	{     
		perror("inet_ptonerror");     
		return -4;     
	}    
	if(ioctl(fd,SIOCSIFNETMASK,	&ifr)<0)
	{
		perror("ioctl");
		return -5;
	}
	//gateway
	memset(&rt,
			0,
			sizeof(struct
				rtentry));
	memset(sin,
			0,
			sizeof(struct
				sockaddr_in));
	sin->sin_family
		=
		AF_INET;
	sin->sin_port
		=
		0;
	if(inet_aton(gateway,
				&sin->sin_addr)<0)
	{
		printf("inet_atonerror\n");
	}
	memcpy(&rt.rt_gateway,sin,sizeof(struct sockaddr_in));
	((struct sockaddr_in*)&rt.rt_dst)->sin_family=AF_INET;
	((struct sockaddr_in*)&rt.rt_genmask)->sin_family=AF_INET;
	rt.rt_flags=RTF_GATEWAY;
	if(ioctl(fd,SIOCADDRT,&rt)<0)
	{
		close(fd);
		return	-1;
	}
	close(fd);
	return rc;
}



//设置mac地址

int get_mac_addr(char *ifname, char *mac)
{
	int fd, rtn;
	struct ifreq ifr;

	if( !ifname || !mac ) {
		return -1;
	}
	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror("socket");
		return -1;
	}
	ifr.ifr_addr.sa_family = AF_INET;    
	strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1 );

	if ( (rtn = ioctl(fd, SIOCGIFHWADDR, &ifr) ) == 0 )
		memcpy(    mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
	close(fd);
	return rtn;
}

int set_mac_addr(char *ifname, char *mac)
{
	int fd, rtn;
	struct ifreq ifr;

	if( !ifname || !mac ) {
		return -1;
	}
	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror("socket");
		return -1;
	}
	ifr.ifr_addr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1 );
	memcpy((unsigned char *)ifr.ifr_hwaddr.sa_data, mac, 6);

	if ( (rtn = ioctl(fd, SIOCSIFHWADDR, &ifr) ) != 0 ){
		perror("SIOCSIFHWADDR");
	}
	close(fd);
	return rtn;
}

int if_updown(char *ifname, int flag)
{
	int fd, rtn;
	struct ifreq ifr;        

	if (!ifname) {
		return -1;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror("socket");
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, (const char *)ifname, IFNAMSIZ - 1 );

	if ( (rtn = ioctl(fd, SIOCGIFFLAGS, &ifr) ) == 0 ) {
		if ( flag == DOWN )
			ifr.ifr_flags &= ~IFF_UP;
		else if ( flag == UP ) 
			ifr.ifr_flags |= IFF_UP;

	}

	if ( (rtn = ioctl(fd, SIOCSIFFLAGS, &ifr) ) != 0) {
		perror("SIOCSIFFLAGS");
	}

	close(fd);

	return rtn;
}

/*
 *  * Convert Ethernet address string representation to binary data
 *   * @param    a    string in xx:xx:xx:xx:xx:xx notation
 *    * @param    e    binary data
 *     * @return    TRUE if conversion was successful and FALSE otherwise
 *      */

int ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}


/*
 *  * Convert Ethernet address binary data to string representation
 *   * @param    e    binary data
 *    * @param    a    string in xx:xx:xx:xx:xx:xx notation
 *     * @return    a
 *      */

char *ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			//*c++ = ':';
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

void SaveCSV(void* pBuff,char* pchPath)
{
	//FILE* pf;
	//pf = fopen(pchBuff,"ab+");
	//fwrite(pchBuff,strlen(),1,pf);
}

void GetCSV(void* pBuff,char* pchPath)
{
	;
}

//获得当前的系统时间 
char* GetSystemTime(int nType)
{
	char chTime[128];
	time_t now;
	struct tm* t_tm;
    time(&now);
	t_tm = localtime(&now);
	printf("totay is %4d-%02d-%02d %02d:%02d:%02d\n",1900+t_tm->tm_year,1+t_tm->tm_mon,t_tm->tm_mday,t_tm->tm_hour,t_tm->tm_min,t_tm->tm_sec);
    sprintf(chTime,"%4d-%02d-%02d %02d:%02d:%02d",1900+t_tm->tm_year,1+t_tm->tm_mon,t_tm->tm_mday,t_tm->tm_hour,t_tm->tm_min,t_tm->tm_sec);
	
	if(nType == 1)
	{
		struct timespec tspec;
		clock_gettime(CLOCK_REALTIME,&tspec);
		int nMilliSecond = 0;
		nMilliSecond = tspec.tv_nsec / 1000000;
        sprintf(chTime,"%4d-%02d-%02d %02d:%02d:%02d.%03d",1900+t_tm->tm_year,1+t_tm->tm_mon,t_tm->tm_mday,t_tm->tm_hour,t_tm->tm_min,t_tm->tm_sec,nMilliSecond);
		return chTime;
	}
	return chTime;
}

//设置当前的系统时间 
int SetSystemTime(char* pchDt)
{
	//struct rtc_time tmData;
	struct tm rtctm;
	struct tm _tm;
	struct timeval tv;
	time_t timep;
    
	sscanf(pchDt,"%d-%d-%d %d:%d:%d",&rtctm.tm_year,&rtctm.tm_mon,&rtctm.tm_mday,&rtctm.tm_hour,&rtctm.tm_min,&rtctm.tm_sec);
	_tm.tm_year = rtctm.tm_year - 1900;
	_tm.tm_mon = rtctm.tm_mon -1;
	_tm.tm_mday = rtctm.tm_mday;
	_tm.tm_hour = rtctm.tm_hour;
	_tm.tm_min = rtctm.tm_min;
	_tm.tm_sec = rtctm.tm_sec;

    timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	//if(settimeofday(&tv,(struct timezone *)0) < 0)
	if(settimeofday(&tv,NULL) < 0)
	{
		printf("Set system datatime error!\n");
		return -1;
	}
	return 0;
}


