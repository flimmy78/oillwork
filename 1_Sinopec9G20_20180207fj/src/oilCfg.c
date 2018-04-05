//#include "oilCfg.h"
#include "../inc/main.h"

/*****************************************************************************
*Name				:myItoa
*Description		:将一个整数转换为字符串形式
*Input				:num	要转换的整数
*						:str		转换后存储的字符串数组
*						:radix	转换进制数，如2，8，10，16
*Output			:无
*Return			:指向生成的字符串，同str
*History			:2014-04-22,modified by syj
*/
char *myItoa(int num, char*str, int radix)
{
	/*索引表*/
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int unum = 0;/*中间变量*/
	int i = 0, j = 0, k = 0;
	char temp = 0;

	/*判断转换进制数*/
	if(radix<1 || radix>36)
	{
		return str;
	}
	
	/*确定unum的值*/
	if(radix==10 && num<0)/*十进制负数*/
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num;/*其他情况*/
	
	/*转换*/
	do
	{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	
	/*逆序*/
	if(str[0]=='-')	k=1;/*十进制负数*/
	else					k=0;
	for(j=k;j<=(i-1)/2;j++)
	{
		temp=str[j];
		str[j]=str[i-1+k-j];
		str[i-1+k-j]=temp;
	}
	
	return str;
}


/*****************************************************************************
*Name				:myNetDotAddrCheck
*Description		:检验点分十进制网络地址是否合法
*Input				:addr	网络地址，点分十进制ASCII码
*Output			:无
*Return			:成功返回0；失败返回ERROR
*History			:2016-01-29,modified by syj
*/
int myNetDotAddrCheck(const char *addr)
{
	unsigned int p1 = 0, p2 = 0, p3 = 0, p4 = 0, pstep = 0;
	int len = 0, istate = 0;

	/*判断长度*/
	len = strlen(addr);
	if(len < 7 || len > 15)
	{
		printf("%s:%d: Error, internet address length [strlen(addr) = %d] is invalid!\n", __FUNCTION__, __LINE__, len);
		return ERROR;
	}

	/*因为inet_addr在字符串中没有字符"."时并不返回错误，所以提前判断是否字符串中包含*/
	if(NULL == strchr(addr, '.'))
	{
		printf("%s:%d: Error, internet address not include dot character!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	istate = inet_addr(addr);
	if(0 == istate || ERROR == istate)
	{
		return ERROR;
	}

	return 0;
}


/*****************************************************************************
*Name				:bcdSum
*Description		:获取BCD数的和
*Input				:in_buffer1		加数1
*						:in_nbytes1		加数1长度
*						:in_buffer2		加数2
*						:in_nbytes2		加数2长度
*						:out_maxbytes	和最大长度
*Output			:None
*Return				:和
*History			:2014-04-22,modified by syj
*/
int bcdSum(const unsigned char *in_buffer1, int in_nbytes1, const unsigned char *in_buffer2, int in_nbytes2, unsigned char *out_buffer, int out_maxbytes)
{
 	unsigned int i=0, sum1=0, sum2=0, sum=0, number=0;
	
	/*判断数据合法性，即均为压缩BCD码*/
	for(i=0; i<in_nbytes1; i++)
	{
		if(((in_buffer1[i]>>0)&0x0f)>=0x0a)	return 1;
		if(((in_buffer1[i]>>4)&0x0f)>=0x0a)	return 1;
	}
	for(i=0; i<in_nbytes2; i++)
	{
		if(((in_buffer2[i]>>0)&0x0f)>=0x0a)	return 1;
		if(((in_buffer2[i]>>4)&0x0f)>=0x0a)	return 1;
	}
	
	/*计算和*/
	for(i=0, number=0; (i<in_nbytes1)|(i<in_nbytes2); i++)
	{
		sum1=0;	sum2=0;
		if(i<in_nbytes1)	sum1=(in_buffer1[in_nbytes1-1-i]>>0)&0x0f;		//加数1
		if(i<in_nbytes2)	sum2=(in_buffer2[in_nbytes2-1-i]>>0)&0x0f;		//加数2
		sum=sum1+sum2+number;																						//和=加数1+加数2+进位
		if(i<out_maxbytes)	out_buffer[out_maxbytes-1-i]=sum%10;				//和
		number=sum/10;																									//进位
		
		sum1=0;	sum2=0;
		if(i<in_nbytes1)	sum1=(in_buffer1[in_nbytes1-1-i]>>4)&0x0f;		//加数1
		if(i<in_nbytes2)	sum2=(in_buffer2[in_nbytes2-1-i]>>4)&0x0f;		//加数2
		sum=sum1+sum2+number;																						//和=加数1+加数2+进位
		if(i<out_maxbytes)	out_buffer[out_maxbytes-1-i]=((sum%10)<<4)|out_buffer[out_maxbytes-1-i];//和
		number=sum/10;																									//进位
		
		/*有进位，和缓存无溢出，加数1或加数2缓存会溢出时进行仅为计算，以免退出后此进位未被计算*/
		if((number>0)&&((i+1)<out_maxbytes)&&(((i+1)>=in_nbytes1)&&((i+1)>=in_nbytes2)))
		{
			out_buffer[out_maxbytes-1-i-1]=number;
		}
	}

	return 0;
}


/*****************************************************************************
*Name				:xorGet
*Description	:获取异或值
*Input				:buffer		用于计算的数据缓冲
*						:nbytes	用于计算的数据缓冲长度
*Output			:None
*Return			:异或值
*History			:2013-07-01,modified by syj
*/
unsigned char xorGet(unsigned char *buffer, int nbytes)
{
	unsigned xor=0;
	int i=0;

	for(i=0; i<nbytes; i++)	
	{
		xor^=buffer[i];
	}

	return xor;
}


/*****************************************************************************
*Name				:crc16Get
*Description		:获取CRC校验值
*Input				:buffer		用于计算的数据缓冲
*						:nbytes	用于计算的数据缓冲长度
*Output			:None
*Return			:16位CRC校验值
*History			:2013-07-01,modified by syj
*/
unsigned int crc16Get(unsigned char *buffer, int nbytes)
{
	unsigned int pCrc=0,i=0, j=0;
	unsigned int PC_POLYNOMIAL=0xA001;

	for(i=0; i<nbytes; i++)
	{
		pCrc ^= buffer[i];
		for(j=0; j<8; j++)
		{
			if(pCrc&0x0001){	pCrc>>=1;	pCrc^=PC_POLYNOMIAL;}
			else						pCrc>>=1; 
		}
	}

	return pCrc;
}


/*****************************************************************************
*Name				:hex2Bcd
*Description		:整形hex格式数据转为bcd格式数据
*Input				:hex_data	hex数据，最大值为0xffffffff
*Output			:None
*Return				:bcd格式数据
*History			:2013-07-01,modified by syj
*/
long long hex2Bcd(unsigned int hex_data)
{
	long long ireturn=0;
	unsigned int hex=0, bcd=0, i=0;

	hex=hex_data;
	for(i=0; i<10;	i++)
	{
		bcd=hex%10;											//获取最低一位
		hex=hex/10;											//舍掉最低一位
		ireturn|=((long long)bcd<<(i*4));			//保存bcd数据
	}

	return ireturn;
}

int hexbcd2int(int nData)
{
    int nTemp = 0;
	unsigned char uchTemp[4] = {0};
	uchTemp[0] = (nData>>24)&0xff;
	uchTemp[1] = (nData>>16)&0xff;
	uchTemp[2] = (nData>>8)&0xff;
	uchTemp[3] = nData&0xff;
	
    nTemp += (uchTemp[0]/0x10 * 10 + uchTemp[0]%0x10)*1000000;
	nTemp += (uchTemp[1]/0x10 * 10 + uchTemp[1]%0x10)*10000;
	nTemp += (uchTemp[2]/0x10 * 10 + uchTemp[2]%0x10)*100;
	nTemp += (uchTemp[3]/0x10 * 10 + uchTemp[3]%0x10);
	return nTemp;
	
}

long long hexbcd2longlong(unsigned long long llData)
{
    unsigned long long llTemp = 0;
	unsigned char uchTemp[8] = {0};
	uchTemp[0] = (llData>>56)&0xff;
	uchTemp[1] = (llData>>48)&0xff;
	uchTemp[2] = (llData>>40)&0xff;
	uchTemp[3] = (llData>>32)&0xff;
	uchTemp[4] = (llData>>24)&0xff;
	uchTemp[5] = (llData>>16)&0xff;
	uchTemp[6] = (llData>>8)&0xff;
	uchTemp[7] = llData&0xff;

    llTemp += (uchTemp[0]/0x10 * 10 + uchTemp[0]%0x10)*100000000000000;
	llTemp += (uchTemp[1]/0x10 * 10 + uchTemp[1]%0x10)*1000000000000;
	llTemp += (uchTemp[2]/0x10 * 10 + uchTemp[2]%0x10)*10000000000;
	llTemp += (uchTemp[3]/0x10 * 10 + uchTemp[3]%0x10)*100000000;
    llTemp += (uchTemp[4]/0x10 * 10 + uchTemp[4]%0x10)*1000000;
	llTemp += (uchTemp[5]/0x10 * 10 + uchTemp[5]%0x10)*10000;
	llTemp += (uchTemp[6]/0x10 * 10 + uchTemp[6]%0x10)*100;
	llTemp += (uchTemp[7]/0x10 * 10 + uchTemp[7]%0x10);
	return llTemp;
	
}

/*****************************************************************************
*Name				:hex2Ascii
*Description		:HEX数据转换为ASCII数据，HEX数据高低半字节拆分为2字节ASCII，字母大写
*Input				:hex_buffer			HEX数据
*						:hex_nbytes			HEX数据长度
*Output			:ascii_buffer			ASCII数据
*						:ascii_maxbytes	ASCII数据长度

 *Return				:成功返回0；失败返回ERROR
*History			:2013-07-01,modified by syj
*/
int hex2Ascii(const unsigned char *hex_buffer, int hex_nbytes, unsigned char *ascii_buffer, int ascii_maxbytes)
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
*Name				:timeVerification
*Description		:时间数据合法性验证
*Input				:buffer		7bytes压缩BCD，格式如YYYYMMDDHHMMSS
*Output			:None
*Return				:0=合法；其它=不合法
*History			:2014-04-09,modified by syj
*/
int timeVerification(const unsigned char *buffer, int nbytes)
{
	unsigned char arr1[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* 闰年调用这个数组 */
	unsigned char arr2[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* 平年调用这个数组 */
	unsigned int year=0, month=0, date=0, hour=0, minite=0, second=0;
	int i=0;

	/*长度有效范围1~7字节*/
	if(!((nbytes>=1)&&(nbytes<=7)))	return 1;

	/*解析年月日时分秒*/
	if(nbytes>=1)	year=((buffer[0]>>4)&0x0f)*1000+((buffer[0]>>0)&0x0f)*100;
	if(nbytes>=2)	year=((buffer[0]>>4)&0x0f)*1000+((buffer[0]>>0)&0x0f)*100+((buffer[1]>>4)&0x0f)*10+((buffer[1]>>0)&0x0f)*1;	
	if(nbytes>=3)	month=((buffer[2]>>4)&0x0f)*10+((buffer[2]>>0)&0x0f)*1;	
	if(nbytes>=4)	date=((buffer[3]>>4)&0x0f)*10+((buffer[3]>>0)&0x0f)*1;	
	if(nbytes>=5)	hour=((buffer[4]>>4)&0x0f)*10+((buffer[4]>>0)&0x0f)*1;	
	if(nbytes>=6)	minite=((buffer[5]>>4)&0x0f)*10+((buffer[5]>>0)&0x0f)*1;
	if(nbytes>=7)	second=((buffer[6]>>4)&0x0f)*10+((buffer[6]>>0)&0x0f)*1;

	/*各位不能为全0*/
	if(0==memcmp(buffer, "\x00\x00\x00\x00\x00\x00\x00", nbytes))
	{
		return 1;
	}

	/*各位不能有字母*/
	for(i=0; i<nbytes; i++)
	{
		if(((buffer[i]>>4)&0x0f)>9)	return 1;
		if(((buffer[i]>>0)&0x0f)>9)	return 1;
	}

//szb_fj_20171120,去掉世纪的判断
#if 0
	/*只支持20世纪和21世纪*/
	if(!((19==(year/100))||(20==(year/100))))
	{
		return 1;
    }
#endif

	if(nbytes<=2)	return 0;

	/*月份1~12*/
	if(!((month>=1)&&(month<=12)))
	{
		return 1;
	}
	if(nbytes<=3)	return 0;

	/*日期*/
	if(((0==year%4)&&(0!=year%100))||(0==year%400))
	{
		if(!((date>=1)&&(date<=arr1[month-1])))	return 1;
	}
	else
	{
		if(!((date>=1)&&(date<=arr2[month-1])))	return 1;
	}
	if(nbytes<=4)	return 0;

	/*时:0~23*/
	if(!((hour>=0)&&(hour<=23)))
	{
		return 1;
	}
	if(nbytes<=5)	return 0;

	/*分:0~59*/
	if(!((minite>=0)&&(minite<=59)))
	{
		return 1;
	}
	if(nbytes<=6)	return 0;

	/*秒:0~59*/
	if(!((second>=0)&&(second<=59)))
	{
		return 1;
	}

	return 0;
}


/*****************************************************************************
*Name				:dealWithSigalrm
*Description		:SIGALRM信号响应函数
*Input				:None
*Output			:None
*Return				:None
*History			:2014-04-09,modified by syj
*/
void dealWithSigalrm(int sigalrm)
{
	return;
}


/*****************************************************************************
*Name				:WriteByLengthInTime
*Description		:向描述符中写入数据
*Input				:fd			描述符
*						:buffer		要写入的数据
*						:nbytes		要写入的数据长度
*						:timeout	超时时间，单位为秒
*Output			:None
*Return				:=nbytes	成功；>=0 && <nbytes 超时；ERROR 失败；
*History			:2014-04-09,modified by syj
*/
int WriteByLengthInTime(int fd, void *buffer, int nbytes, int timeout )
{
	int	nleft;
	int nwritten;
	char	*ptr;

	/*数据发送起始下标及未发送长度赋值*/
	ptr = buffer;
	nleft = nbytes;

	/*设置信号超时*/
	//alarm(timeout);

	/*写数据，直到发送完成或超时或错误退出*/
	while(nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) < 0 )
		{
			/*超时，取消信号定时器，设置错误代码ETIME*/
			if(signal(SIGALRM, dealWithSigalrm))
			{
				//errnoSet(ETIME);
				strerror(ETIME);  //fj:
				break;
			}
			/*被其他信号打断则继续写*/
			if(EINTR==errno)
			{
				continue;
			}
			/*错误退出*/
			//errnoSet(errno);
			strerror(errno); //fj:
			return ERROR;
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	/*取消信号定时器*/
	//alarm(0);

	return (nbytes-nleft);
}


/*****************************************************************************
*Name				:ReadByLengthInTime
*Description		:从描述符中读取数据
*Input				:fd				描述符，阻塞I/O
*						:buffer			读取数据的缓存
*						:maxbytes	读取数据的缓存长度
*						:timeout		超时时间，单位为秒
*Output			:None
*Return				:=nbytes	成功；>=0 && <nbytes 读取结束或超时；ERROR 失败；
*History			:2014-04-09,modified by syj
*/
int ReadByLengthInTime(int fd, void *buffer, int maxbytes, int timeout )
{
	int	nleft;
	int nread;
	char	*ptr;

	/*数据读取起始下标及未读取长度赋值*/
	ptr = buffer;
	nleft = maxbytes;

	/*设置信号超时*/
	//alarm(timeout);

	/*读取数据，读取完成或超时等退出*/
	while( nleft > 0 )
	{
		nread = read( fd, ptr, nleft );
		if( nread < 0 )
		{
			/*超时，取消信号定时器，设置错误代码ETIME*/
			if(signal(SIGALRM, dealWithSigalrm))
			{
				//errnoSet(ETIME);
				strerror(ETIME);
				break;
			}
			/*暂无数据时继续读取*/
			if( EAGAIN == errno )
			{
				continue;
			}
			/*被其他信号打断则继续写*/
			if(EINTR==errno)
			{
				continue;
			}
			/*错误退出*/
			//errnoSet(errno);
			strerror(errno); //fj:
			return ERROR;
		}
		else if(0==nread)
		{
			/*无数据可读*/
			//errnoSet(COMM_EOF);
			break;
		}

		nleft -= nread;
		ptr   += nread;
	}

	/*取消信号定时器*/
	//alarm(0);

	return (maxbytes - nleft);
}


#if 0
	while( nleft > 0) {
		/*阻塞write除非被信号打断或者出错,否则不会写入0个字符*/
		if ((nwritten = write(fd, ptr, nleft)) <= 0 ) {
			if( CatchSigAlrm )	/*超时*//*不管错误码,只要捕捉到超时都返回*/
			{
				UnSetAlrm();
				RptLibErr( ETIME );
				break;
			}

			
			if(errno == EINTR) {
				nwritten = 0;
			} else {
				UnSetAlrm();
				RptLibErr( errno );
				return(-1);
			}
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	UnSetAlrm();

	return (nbytes-nleft);
}



/*从描述符中读数据									*/
/*返回值:							*/
/*	-1			出错失败			*/
/*	=n			成功				*/
/*	>=0 && <n	文件结束（连接关闭）或者超时		*/
/*	注: fd应该是阻塞IO*/

ssize_t ReadByLengthInTime( int fd, void *vptr, size_t n, int timeout )
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	SetAlrm( timeout, NULL );

	ptr = vptr;
	nleft = n;
	while( nleft > 0 ) {
		if( ( nread = read( fd, ptr, nleft ) ) < 0 ) {
			if( CatchSigAlrm )	/*超时*//*不管错误码,只要捕捉到超时都返回*/
			{
				UnSetAlrm();
				RptLibErr( ETIME );
				break;
			}
			if( errno == EAGAIN ) {	/*无数据*/
				continue;
			}
			if( errno == EINTR )
			{
				/*被其他信号打断继续读*/
				nread = 0;
			}
			else	/*出错*/
			{
				UnSetAlrm();
				RptLibErr( errno );
				return(-1);
			}
		}
		else if( nread == 0 )
		{
			RptLibErr( COMM_EOF );
			break;				/* EOF */
		}

		nleft -= nread;
		ptr   += nread;
	}

	UnSetAlrm( );

	return (n - nleft);
}
#endif


bool JudgeFFAndCrcProtocol(unsigned char* puchRecvData,int nRecvLen,unsigned char* puchReturnData,int* pnReturnLen,int* pnDeleteLen)
{
	if(nRecvLen < 9) //长度不够，继续接收
	{
		pnDeleteLen = nRecvLen;
		return false;
	}
	int nffRecvLen = nRecvLen;
	int nStep = 0;
	int nCount = 0;
	int nCurRecvLen = 0;
    unsigned char uchCurData = 0;

	int nNotEscLen = 0;   //没有转义字符，没有ff头尾和CRC的长度

	unsigned char uchAllData[1024] = {0};     //包含转义字符的数据
	unsigned char uchNotEscData[1024] = {0};  //没有转义字符的数据

	int i = 0;
	for( i = 0; i < nffRecvLen; i++)
	{
        switch(nStep)
		{
		case 0: //包头
			{
				if((uchCurData == 0xff) && (puchRecvData[i] != 0xff))
				{
                    nCount = 0;
                    uchAllData[nCount++] = puchRecvData[i];
					nStep = 1;
				}
			}
			break;
		case 1: //包尾
			{
				if((uchCurData != 0xff) && (puchRecvData[i] == 0xff))
				{
                    int nDataAreaLen = uchAllData[1]; //长度，第uchAllData[0]帧序号，uchAllData[1]是数据区长度
                    if(nRecvLen < 7+nDataAreaLen) //数据长度错误
					{
						*pnDeleteLen = -3; //数据的长度错误
						printf("aaaaaaaaa----\n");
                        PrintH(nRecvLen,puchRecvData);
						return false;
					}

					int crc = 0;
					int nCrcLen = 0;
					unsigned char uchTempCrc[2] = {0};
					if(puchRecvData[4+nDataAreaLen] != 0xfe)
					{
						uchTempCrc[0] = puchRecvData[4+nDataAreaLen];
						if(puchRecvData[5+nDataAreaLen] != 0xfe)
						{
							uchTempCrc[1] = puchRecvData[5+nDataAreaLen];
							nCrcLen = 2;
						}
						else 
						{
							if(puchRecvData[6+nDataAreaLen] == 0x01)
							{
								uchTempCrc[1] = 0xff;
							}
							else
							{
								uchTempCrc[1] = 0xfe;
							}
							nCrcLen = 3;
						}
					}
					else
					{
                        if(puchRecvData[5+nDataAreaLen]==0x01)
						{
                            uchTempCrc[0] = 0xff;
                            if(puchRecvData[6+nDataAreaLen] != 0xfe)
							{
								uchTempCrc[1] = puchRecvData[6+nDataAreaLen];
								nCrcLen = 3;
							}
							else
							{
								if(puchRecvData[7+nDataAreaLen] == 0x01)
								{
									uchTempCrc[1] = 0xff;
								}
								else
								{
									uchTempCrc[1] = 0xfe;
								}
								nCrcLen = 4;
							}
						}
						else
						{
							uchTempCrc[0] = 0xfe;
                            if(puchRecvData[6+nDataAreaLen] != 0xfe)
							{
								uchTempCrc[1] = puchRecvData[6+nDataAreaLen];
								nCrcLen = 3;
							}
							else
							{
								if(puchRecvData[7+nDataAreaLen] == 0x01)
								{
									uchTempCrc[1] = 0xff;
								}
								else
								{
									uchTempCrc[1] = 0xfe;
								}
								nCrcLen = 4;
							}
						}
					}

					crc = crc16Get(uchAllData,nDataAreaLen+2);
					//if(crc == ((int)uchTempCrc[0] << 8) | uchTempCrc[1]) //CRC_OK
					if(crc == (((int)uchTempCrc[0] << 8) | uchTempCrc[1]))  //fj:20171206
					{
					    int j;
						int nIndex = nCount - nCrcLen;
						for(j = 0; j < nIndex; j++)
						{
                            if(uchAllData[j] == 0xfe && uchAllData[j+1] == 0x00)
							{
								uchNotEscData[nNotEscLen++] = 0xfe;
								j++;
							}
							else if(uchAllData[j] == 0xfe && uchAllData[j+1] == 0x01)
							{
								uchNotEscData[nNotEscLen++] = 0xff;
								j++;
							}
							else 
							{
								uchNotEscData[nNotEscLen++] = uchAllData[j];
							}
						}	

						memcpy(puchReturnData,uchNotEscData,nNotEscLen);
						*pnReturnLen = nNotEscLen;
						*pnDeleteLen = -1;
						return true;
					}
					else
					{
						*pnDeleteLen = -2; //CRC错误
						PrintH(nRecvLen,puchRecvData);
						return false;
					}
				}
				else
				{
					uchAllData[nCount++] = puchRecvData[i];
				}
			}
			break;
		default:
			break;
		}
		uchCurData = puchRecvData[i];
		nCurRecvLen++;
	}

    *pnDeleteLen = nRecvLen;
	return false;
}
