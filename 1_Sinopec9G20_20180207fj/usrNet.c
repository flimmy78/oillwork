
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

/*此处定义的USR_NVRAMPATH必须与sysNvRam.c中NVRAMPATH定义的文件路径一致*/
#define USR_NVRAMPATH "/flash/nvram.txt"

extern char OurEmacAddr[6];


/*****************************************************************************
*Name				:usrNetHex2Ascii
*Description		:HEX数据转换为ASCII数据，HEX数据高低半字节拆分为2字节ASCII，字母大写
*Input				:hex_buffer			HEX数据
*						:hex_nbytes			HEX数据长度
*Output			:ascii_buffer			ASCII数据
*						:ascii_maxbytes	ASCII数据长度
*Return				:成功返回0；失败返回ERROR
*History			:2013-07-01,modified by syj
*/
static int usrNetHex2Ascii(unsigned char *hex_buffer, int hex_nbytes, unsigned char *ascii_buffer, int ascii_maxbytes)
{
	int i=0;
	unsigned char hex_data=0;

	/*判断长度，输出应不小于输入的两倍*/
	if(ascii_maxbytes<(2*hex_nbytes))
	{
		return ERROR;
	}

	/*转换*/
	for(i=0; i<hex_nbytes; i++)
	{
		/*高半字节转换*/
		hex_data=(hex_buffer[i]>>4)&0x0f;
		if(hex_data<10)	ascii_buffer[2*i+0]=hex_data+0x30;
		else						ascii_buffer[2*i+0]=hex_data+0x41-10;

		/*低半字节转换*/
		hex_data=(hex_buffer[i]>>0)&0x0f;
		if(hex_data<10)	ascii_buffer[2*i+1]=hex_data+0x30;
		else						ascii_buffer[2*i+1]=hex_data+0x41-10;
	}

	return 0;
}


/*****************************************************************************
*Name				:usrNetIpGet
*Description		:获取IP地址配置信息
*Input				:interfaceName			网络设备名，如eth0
*Output			:interfaceAddress		网络设备IP地址，如"192.168.2.10"
*Return				:成功返回OK；失败返回其它值；
*History			:2015-10-25,modified by syj
*/
int usrNetIpGet(char *interfaceName, char *interfaceAddress)
{
	  int sock_get_ip;  
    char ipaddr[50];  
  
    struct   sockaddr_in *sin;  
    struct   ifreq ifr_ip;     
  
    if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
         printf("socket create failse...GetLocalIp!/n");  
         return ERROR;  
    }  
     
    memset(&ifr_ip, 0, sizeof(ifr_ip));     
    strncpy(ifr_ip.ifr_name, interfaceName, sizeof(ifr_ip.ifr_name) - 1);     
   
    if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )     
    {     
         return ERROR;    
    }       
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;     
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));         
      
    printf("local ip:%s /n",ipaddr);      
    close( sock_get_ip ); 
    
    //memcpy(emacAddr, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
    strcpy(interfaceAddress,ipaddr); 
    
    return OK;
}


/*****************************************************************************
*Name				:usrNetIpSet
*Description		:设置IP地址配置信息，该配置信息会修改启动参数nvram.txt并修改
*Input				:interfaceName			网络设备名，如eth0
*						:interfaceAddress		网络设备IP地址，如"192.168.2.10"
*Output			:无
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetIpSet( char *interfaceName, char *interfaceAddress)
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
		return ERROR;     
	}
	memset(&ifr,0,sizeof(ifr)); 
	strcpy(ifr.ifr_name,interfaceName); 
	sin = (struct sockaddr_in*)&ifr.ifr_addr;     
	sin->sin_family = AF_INET;     
	//ipaddr
	if(inet_aton(interfaceAddress,&(sin->sin_addr))< 0)   
	{     
		perror("inet_aton error");     
		return ERROR;     
	}    

	if(ioctl(fd,SIOCSIFADDR,&ifr)<0)   
	{     
		perror("ioctl SIOCSIFADDRerror");     
		return ERROR;     
	}
	close(fd);
	return rc;
}


/*****************************************************************************
*Name				:usrNetMaskGet
*Description		:获取掩码配置信息
*Input				:interfaceName			网络设备名，如eth0
*Output			:netMask					网络掩码，如0xFFFFFF00
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj  fj:20171213
*/
int usrNetMaskGet(char *interfaceName, int *netMask)
{
	char netmask_addr[50];    
    
    struct ifreq ifr_mask;    
    struct sockaddr_in *net_mask;    
            
    int sock_netmask = socket( AF_INET, SOCK_STREAM, 0 );    
    if( sock_netmask == -1)    
    {    
        perror("create socket failture...GetLocalNetMask/n");    
        return ERROR;   
    }    
        
    memset(&ifr_mask, 0, sizeof(ifr_mask));       
    strncpy(ifr_mask.ifr_name, interfaceName, sizeof(ifr_mask.ifr_name )-1);       
    
    if( (ioctl( sock_netmask, SIOCGIFNETMASK, &ifr_mask ) ) < 0 )     
    {    
        printf("mac ioctl error/n");    
        return ERROR;   
    }    
        
    net_mask = ( struct sockaddr_in * )&( ifr_mask.ifr_netmask );    
    strcpy( netmask_addr, inet_ntoa( net_mask -> sin_addr ) );    
        
    printf("local netmask:%s/n",netmask_addr);  

	*netMask = net_mask->sin_addr.s_addr;
    
    //int netMaskTemp = 0;
    //netMaskTemp = ((int)netmask_addr[0] <<24 ) | ((int)netmask_addr[1]<<16) | ((int)netmask_addr[2] << 8) | (unsigned char)netmask_addr[3];
    //*netMask = netMaskTemp;
    
    printf("netMask = %08\n",*netMask);
        
    close( sock_netmask );  
    
    return OK;  
}


/*****************************************************************************
*Name				:usrNetMaskSet
*Description		:设置掩码配置信息，该配置信息会修改启动参数nvram.txt并修改
*Input				:interfaceName			网络设备名，如eth0
*						:netMask					网络掩码，如0xFFFFFF00
*Output			:无
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetMaskSet(char *interfaceName, int netMask)
{
	  char hexbuffer[4+1]={0};
	  char asciibuffer[10+1]={0};
	  char maskbuffer[21] = {0};
	  *(maskbuffer+strlen(maskbuffer))=':';
	  hexbuffer[0]=(char)(netMask>>24);	hexbuffer[1]=(char)(netMask>>16);
	  hexbuffer[2]=(char)(netMask>>8);	hexbuffer[3]=(char)(netMask>>0);
	  usrNetHex2Ascii(hexbuffer, 4, asciibuffer, 8);
	  strcpy(maskbuffer+strlen(maskbuffer), asciibuffer);
	  
	   int fd;
	int rc;
	struct ifreq ifr; 
	struct sockaddr_in *sin;
	struct rtentry  rt;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		perror("socket   error");     
		return ERROR;     
	}
	memset(&ifr,0,sizeof(ifr)); 
	strcpy(ifr.ifr_name,interfaceName); 
	sin = (struct sockaddr_in*)&ifr.ifr_addr;     
	sin->sin_family = AF_INET;  
	if(inet_aton(maskbuffer,&(sin->sin_addr))	<0)   
	{     
		perror("inet_ptonerror");     
		return ERROR;     
	}    
	if(ioctl(fd,SIOCSIFNETMASK,	&ifr)<0)
	{
		perror("ioctl");
		return ERROR;
	}
	close(fd);
	return rc;
}

/*****************************************************************************
*Name				:usrNetHostGatewayGet
*Description		:获取目的为主机的默认网关地址
*Input				:interfaceName			网络设备名，如"eth0"
*Output			:broadcastAddress	默认网关，如"192.168.2.1"
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetHostGatewayGet(char *interfaceName, char *gatewayAddress)
{
	  FILE *fp;  
    char buf[512];  
    char cmd[128];  
    char gateway[30];  
    char *tmp;  
  
    strcpy(cmd, "ip route");  
    fp = popen(cmd, "r");  
    if(NULL == fp)  
    {  
        perror("popen error");  
        return ERROR;  
    }  
    while(fgets(buf, sizeof(buf), fp) != NULL)  
    {  
        tmp =buf;  
        while(*tmp && isspace(*tmp))  
            ++ tmp;  
        if(strncmp(tmp, "default", strlen("default")) == 0)  
            break;  
    }  
    sscanf(buf, "%*s%*s%s", gateway);         
    printf("default gateway:%s/n", gateway);  
    pclose(fp);
    
    strcpy(gatewayAddress,gateway);
    return OK;
}

/*****************************************************************************
*Name				:usrNetHostGatewaySet
*Description		:配置目的为主机的默认网关地址，配置此路由时请配置主机地址，此处会添加目的地址为主机地址网段的路由
*Input				:interfaceName			网络设备名，如"eth0"
*Output			:broadcastAddress					默认网关，如"192.168.2.1"
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetHostGatewaySet(char *interfaceName, char *gatewayAddress)
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
	strcpy(ifr.ifr_name,interfaceName); 
	sin = (struct sockaddr_in*)&ifr.ifr_addr;     
	sin->sin_family = AF_INET;  
	
	memset(&rt,0,sizeof(struct rtentry));
	memset(sin,0,sizeof(struct sockaddr_in));
	sin->sin_family	= AF_INET;
	sin->sin_port = 0;
	if(inet_aton(gatewayAddress,&sin->sin_addr)<0)
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

/*****************************************************************************
*Name				:usrNetEMACGet
*Description		:设置掩码配置信息，该配置信息会修改启动参数nvram.txt并修改
*Input				:interfaceName			网络设备名，如eth0
*Output			:emacAddr				EMAC地址，如{0x00,0x5e,0x00,0x01,0x02,0x03}
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetEMACGet(char *interfaceName, char *emacAddr)
{
	int fd, rtn;
	struct ifreq ifr;
	
	if( !interfaceName || !emacAddr )
	{
		return -1;
	}
	
	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) 
	{
		perror("socket");
		return -1;
	}
	ifr.ifr_addr.sa_family = AF_INET;    
	strncpy(ifr.ifr_name, (const char *)interfaceName, IFNAMSIZ - 1 );

	if ( (rtn = ioctl(fd, SIOCGIFHWADDR, &ifr) ) == 0 )
		memcpy(emacAddr, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
	close(fd);
	return rtn;
}

/*****************************************************************************
*Name				:usrNetEMACGet
*Description		:设置掩码配置信息，该配置信息会修改启动参数nvram.txt并修改
:								:注意：设置的MAC地址不能是组播地址(即MAC第48位不能是1，本处第0字节b0不能是1)，不能是广播地址(即全F)，不能是全0地址
*Input				:interfaceName			网络设备名，如eth0
*						:emacAddr				EMAC地址，如{0x00,0x5e,0x00,0x01,0x02,0x03}
*Output			:无
*Return				:成功返回OK；失败返回ERROR；
*History			:2015-10-25,modified by syj
*/
int usrNetEMACSet(char *interfaceName, char *emacAddr)
{
	int fd, rtn;
	struct ifreq ifr;

	if( !interfaceName || !emacAddr ) {
		return -1;
	}
	fd = socket(AF_INET, SOCK_DGRAM, 0 );
	if ( fd < 0 ) {
		perror("socket");
		return -1;
	}
	ifr.ifr_addr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, (const char *)interfaceName, IFNAMSIZ - 1 );
	memcpy((unsigned char *)ifr.ifr_hwaddr.sa_data, emacAddr, 6);

	if ( (rtn = ioctl(fd, SIOCSIFHWADDR, &ifr) ) != 0 ){
		perror("SIOCSIFHWADDR");
	}
	close(fd);
	return rtn;
}

int ttttteset()
{
	usrNetIpSet("eth0", "11.0.31.065");

	return 0;
}




