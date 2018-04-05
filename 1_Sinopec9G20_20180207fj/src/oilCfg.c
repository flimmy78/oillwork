//#include "oilCfg.h"
#include "../inc/main.h"

/*****************************************************************************
*Name				:myItoa
*Description		:��һ������ת��Ϊ�ַ�����ʽ
*Input				:num	Ҫת��������
*						:str		ת����洢���ַ�������
*						:radix	ת������������2��8��10��16
*Output			:��
*Return			:ָ�����ɵ��ַ�����ͬstr
*History			:2014-04-22,modified by syj
*/
char *myItoa(int num, char*str, int radix)
{
	/*������*/
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int unum = 0;/*�м����*/
	int i = 0, j = 0, k = 0;
	char temp = 0;

	/*�ж�ת��������*/
	if(radix<1 || radix>36)
	{
		return str;
	}
	
	/*ȷ��unum��ֵ*/
	if(radix==10 && num<0)/*ʮ���Ƹ���*/
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num;/*�������*/
	
	/*ת��*/
	do
	{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	
	/*����*/
	if(str[0]=='-')	k=1;/*ʮ���Ƹ���*/
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
*Description		:������ʮ���������ַ�Ƿ�Ϸ�
*Input				:addr	�����ַ�����ʮ����ASCII��
*Output			:��
*Return			:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2016-01-29,modified by syj
*/
int myNetDotAddrCheck(const char *addr)
{
	unsigned int p1 = 0, p2 = 0, p3 = 0, p4 = 0, pstep = 0;
	int len = 0, istate = 0;

	/*�жϳ���*/
	len = strlen(addr);
	if(len < 7 || len > 15)
	{
		printf("%s:%d: Error, internet address length [strlen(addr) = %d] is invalid!\n", __FUNCTION__, __LINE__, len);
		return ERROR;
	}

	/*��Ϊinet_addr���ַ�����û���ַ�"."ʱ�������ش���������ǰ�ж��Ƿ��ַ����а���*/
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
*Description		:��ȡBCD���ĺ�
*Input				:in_buffer1		����1
*						:in_nbytes1		����1����
*						:in_buffer2		����2
*						:in_nbytes2		����2����
*						:out_maxbytes	����󳤶�
*Output			:None
*Return				:��
*History			:2014-04-22,modified by syj
*/
int bcdSum(const unsigned char *in_buffer1, int in_nbytes1, const unsigned char *in_buffer2, int in_nbytes2, unsigned char *out_buffer, int out_maxbytes)
{
 	unsigned int i=0, sum1=0, sum2=0, sum=0, number=0;
	
	/*�ж����ݺϷ��ԣ�����Ϊѹ��BCD��*/
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
	
	/*�����*/
	for(i=0, number=0; (i<in_nbytes1)|(i<in_nbytes2); i++)
	{
		sum1=0;	sum2=0;
		if(i<in_nbytes1)	sum1=(in_buffer1[in_nbytes1-1-i]>>0)&0x0f;		//����1
		if(i<in_nbytes2)	sum2=(in_buffer2[in_nbytes2-1-i]>>0)&0x0f;		//����2
		sum=sum1+sum2+number;																						//��=����1+����2+��λ
		if(i<out_maxbytes)	out_buffer[out_maxbytes-1-i]=sum%10;				//��
		number=sum/10;																									//��λ
		
		sum1=0;	sum2=0;
		if(i<in_nbytes1)	sum1=(in_buffer1[in_nbytes1-1-i]>>4)&0x0f;		//����1
		if(i<in_nbytes2)	sum2=(in_buffer2[in_nbytes2-1-i]>>4)&0x0f;		//����2
		sum=sum1+sum2+number;																						//��=����1+����2+��λ
		if(i<out_maxbytes)	out_buffer[out_maxbytes-1-i]=((sum%10)<<4)|out_buffer[out_maxbytes-1-i];//��
		number=sum/10;																									//��λ
		
		/*�н�λ���ͻ��������������1�����2��������ʱ���н�Ϊ���㣬�����˳���˽�λδ������*/
		if((number>0)&&((i+1)<out_maxbytes)&&(((i+1)>=in_nbytes1)&&((i+1)>=in_nbytes2)))
		{
			out_buffer[out_maxbytes-1-i-1]=number;
		}
	}

	return 0;
}


/*****************************************************************************
*Name				:xorGet
*Description	:��ȡ���ֵ
*Input				:buffer		���ڼ�������ݻ���
*						:nbytes	���ڼ�������ݻ��峤��
*Output			:None
*Return			:���ֵ
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
*Description		:��ȡCRCУ��ֵ
*Input				:buffer		���ڼ�������ݻ���
*						:nbytes	���ڼ�������ݻ��峤��
*Output			:None
*Return			:16λCRCУ��ֵ
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
*Description		:����hex��ʽ����תΪbcd��ʽ����
*Input				:hex_data	hex���ݣ����ֵΪ0xffffffff
*Output			:None
*Return				:bcd��ʽ����
*History			:2013-07-01,modified by syj
*/
long long hex2Bcd(unsigned int hex_data)
{
	long long ireturn=0;
	unsigned int hex=0, bcd=0, i=0;

	hex=hex_data;
	for(i=0; i<10;	i++)
	{
		bcd=hex%10;											//��ȡ���һλ
		hex=hex/10;											//������һλ
		ireturn|=((long long)bcd<<(i*4));			//����bcd����
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
*Description		:HEX����ת��ΪASCII���ݣ�HEX���ݸߵͰ��ֽڲ��Ϊ2�ֽ�ASCII����ĸ��д
*Input				:hex_buffer			HEX����
*						:hex_nbytes			HEX���ݳ���
*Output			:ascii_buffer			ASCII����
*						:ascii_maxbytes	ASCII���ݳ���

 *Return				:�ɹ�����0��ʧ�ܷ���ERROR
*History			:2013-07-01,modified by syj
*/
int hex2Ascii(const unsigned char *hex_buffer, int hex_nbytes, unsigned char *ascii_buffer, int ascii_maxbytes)
{
	int i=0;
	unsigned char hex_data=0;

	/*�жϳ��ȣ����Ӧ��С�����������*/
	if(ascii_maxbytes<(2*hex_nbytes))
	{
		return ERROR;
	}

	/*ת��*/
	for(i=0; i<hex_nbytes; i++)
	{
		/*�߰��ֽ�ת��*/
		hex_data=(hex_buffer[i]>>4)&0x0f;
		if(hex_data<10)	ascii_buffer[2*i+0]=hex_data+0x30;
		else						ascii_buffer[2*i+0]=hex_data+0x41-10;

		/*�Ͱ��ֽ�ת��*/
		hex_data=(hex_buffer[i]>>0)&0x0f;
		if(hex_data<10)	ascii_buffer[2*i+1]=hex_data+0x30;
		else						ascii_buffer[2*i+1]=hex_data+0x41-10;
	}

	return 0;
}


/*****************************************************************************
*Name				:timeVerification
*Description		:ʱ�����ݺϷ�����֤
*Input				:buffer		7bytesѹ��BCD����ʽ��YYYYMMDDHHMMSS
*Output			:None
*Return				:0=�Ϸ�������=���Ϸ�
*History			:2014-04-09,modified by syj
*/
int timeVerification(const unsigned char *buffer, int nbytes)
{
	unsigned char arr1[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* �������������� */
	unsigned char arr2[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* ƽ������������ */
	unsigned int year=0, month=0, date=0, hour=0, minite=0, second=0;
	int i=0;

	/*������Ч��Χ1~7�ֽ�*/
	if(!((nbytes>=1)&&(nbytes<=7)))	return 1;

	/*����������ʱ����*/
	if(nbytes>=1)	year=((buffer[0]>>4)&0x0f)*1000+((buffer[0]>>0)&0x0f)*100;
	if(nbytes>=2)	year=((buffer[0]>>4)&0x0f)*1000+((buffer[0]>>0)&0x0f)*100+((buffer[1]>>4)&0x0f)*10+((buffer[1]>>0)&0x0f)*1;	
	if(nbytes>=3)	month=((buffer[2]>>4)&0x0f)*10+((buffer[2]>>0)&0x0f)*1;	
	if(nbytes>=4)	date=((buffer[3]>>4)&0x0f)*10+((buffer[3]>>0)&0x0f)*1;	
	if(nbytes>=5)	hour=((buffer[4]>>4)&0x0f)*10+((buffer[4]>>0)&0x0f)*1;	
	if(nbytes>=6)	minite=((buffer[5]>>4)&0x0f)*10+((buffer[5]>>0)&0x0f)*1;
	if(nbytes>=7)	second=((buffer[6]>>4)&0x0f)*10+((buffer[6]>>0)&0x0f)*1;

	/*��λ����Ϊȫ0*/
	if(0==memcmp(buffer, "\x00\x00\x00\x00\x00\x00\x00", nbytes))
	{
		return 1;
	}

	/*��λ��������ĸ*/
	for(i=0; i<nbytes; i++)
	{
		if(((buffer[i]>>4)&0x0f)>9)	return 1;
		if(((buffer[i]>>0)&0x0f)>9)	return 1;
	}

//szb_fj_20171120,ȥ�����͵��ж�
#if 0
	/*ֻ֧��20���ͺ�21����*/
	if(!((19==(year/100))||(20==(year/100))))
	{
		return 1;
    }
#endif

	if(nbytes<=2)	return 0;

	/*�·�1~12*/
	if(!((month>=1)&&(month<=12)))
	{
		return 1;
	}
	if(nbytes<=3)	return 0;

	/*����*/
	if(((0==year%4)&&(0!=year%100))||(0==year%400))
	{
		if(!((date>=1)&&(date<=arr1[month-1])))	return 1;
	}
	else
	{
		if(!((date>=1)&&(date<=arr2[month-1])))	return 1;
	}
	if(nbytes<=4)	return 0;

	/*ʱ:0~23*/
	if(!((hour>=0)&&(hour<=23)))
	{
		return 1;
	}
	if(nbytes<=5)	return 0;

	/*��:0~59*/
	if(!((minite>=0)&&(minite<=59)))
	{
		return 1;
	}
	if(nbytes<=6)	return 0;

	/*��:0~59*/
	if(!((second>=0)&&(second<=59)))
	{
		return 1;
	}

	return 0;
}


/*****************************************************************************
*Name				:dealWithSigalrm
*Description		:SIGALRM�ź���Ӧ����
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
*Description		:����������д������
*Input				:fd			������
*						:buffer		Ҫд�������
*						:nbytes		Ҫд������ݳ���
*						:timeout	��ʱʱ�䣬��λΪ��
*Output			:None
*Return				:=nbytes	�ɹ���>=0 && <nbytes ��ʱ��ERROR ʧ�ܣ�
*History			:2014-04-09,modified by syj
*/
int WriteByLengthInTime(int fd, void *buffer, int nbytes, int timeout )
{
	int	nleft;
	int nwritten;
	char	*ptr;

	/*���ݷ�����ʼ�±꼰δ���ͳ��ȸ�ֵ*/
	ptr = buffer;
	nleft = nbytes;

	/*�����źų�ʱ*/
	//alarm(timeout);

	/*д���ݣ�ֱ��������ɻ�ʱ������˳�*/
	while(nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) < 0 )
		{
			/*��ʱ��ȡ���źŶ�ʱ�������ô������ETIME*/
			if(signal(SIGALRM, dealWithSigalrm))
			{
				//errnoSet(ETIME);
				strerror(ETIME);  //fj:
				break;
			}
			/*�������źŴ�������д*/
			if(EINTR==errno)
			{
				continue;
			}
			/*�����˳�*/
			//errnoSet(errno);
			strerror(errno); //fj:
			return ERROR;
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	/*ȡ���źŶ�ʱ��*/
	//alarm(0);

	return (nbytes-nleft);
}


/*****************************************************************************
*Name				:ReadByLengthInTime
*Description		:���������ж�ȡ����
*Input				:fd				������������I/O
*						:buffer			��ȡ���ݵĻ���
*						:maxbytes	��ȡ���ݵĻ��泤��
*						:timeout		��ʱʱ�䣬��λΪ��
*Output			:None
*Return				:=nbytes	�ɹ���>=0 && <nbytes ��ȡ������ʱ��ERROR ʧ�ܣ�
*History			:2014-04-09,modified by syj
*/
int ReadByLengthInTime(int fd, void *buffer, int maxbytes, int timeout )
{
	int	nleft;
	int nread;
	char	*ptr;

	/*���ݶ�ȡ��ʼ�±꼰δ��ȡ���ȸ�ֵ*/
	ptr = buffer;
	nleft = maxbytes;

	/*�����źų�ʱ*/
	//alarm(timeout);

	/*��ȡ���ݣ���ȡ��ɻ�ʱ���˳�*/
	while( nleft > 0 )
	{
		nread = read( fd, ptr, nleft );
		if( nread < 0 )
		{
			/*��ʱ��ȡ���źŶ�ʱ�������ô������ETIME*/
			if(signal(SIGALRM, dealWithSigalrm))
			{
				//errnoSet(ETIME);
				strerror(ETIME);
				break;
			}
			/*��������ʱ������ȡ*/
			if( EAGAIN == errno )
			{
				continue;
			}
			/*�������źŴ�������д*/
			if(EINTR==errno)
			{
				continue;
			}
			/*�����˳�*/
			//errnoSet(errno);
			strerror(errno); //fj:
			return ERROR;
		}
		else if(0==nread)
		{
			/*�����ݿɶ�*/
			//errnoSet(COMM_EOF);
			break;
		}

		nleft -= nread;
		ptr   += nread;
	}

	/*ȡ���źŶ�ʱ��*/
	//alarm(0);

	return (maxbytes - nleft);
}


#if 0
	while( nleft > 0) {
		/*����write���Ǳ��źŴ�ϻ��߳���,���򲻻�д��0���ַ�*/
		if ((nwritten = write(fd, ptr, nleft)) <= 0 ) {
			if( CatchSigAlrm )	/*��ʱ*//*���ܴ�����,ֻҪ��׽����ʱ������*/
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



/*���������ж�����									*/
/*����ֵ:							*/
/*	-1			����ʧ��			*/
/*	=n			�ɹ�				*/
/*	>=0 && <n	�ļ����������ӹرգ����߳�ʱ		*/
/*	ע: fdӦ��������IO*/

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
			if( CatchSigAlrm )	/*��ʱ*//*���ܴ�����,ֻҪ��׽����ʱ������*/
			{
				UnSetAlrm();
				RptLibErr( ETIME );
				break;
			}
			if( errno == EAGAIN ) {	/*������*/
				continue;
			}
			if( errno == EINTR )
			{
				/*�������źŴ�ϼ�����*/
				nread = 0;
			}
			else	/*����*/
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
	if(nRecvLen < 9) //���Ȳ�������������
	{
		pnDeleteLen = nRecvLen;
		return false;
	}
	int nffRecvLen = nRecvLen;
	int nStep = 0;
	int nCount = 0;
	int nCurRecvLen = 0;
    unsigned char uchCurData = 0;

	int nNotEscLen = 0;   //û��ת���ַ���û��ffͷβ��CRC�ĳ���

	unsigned char uchAllData[1024] = {0};     //����ת���ַ�������
	unsigned char uchNotEscData[1024] = {0};  //û��ת���ַ�������

	int i = 0;
	for( i = 0; i < nffRecvLen; i++)
	{
        switch(nStep)
		{
		case 0: //��ͷ
			{
				if((uchCurData == 0xff) && (puchRecvData[i] != 0xff))
				{
                    nCount = 0;
                    uchAllData[nCount++] = puchRecvData[i];
					nStep = 1;
				}
			}
			break;
		case 1: //��β
			{
				if((uchCurData != 0xff) && (puchRecvData[i] == 0xff))
				{
                    int nDataAreaLen = uchAllData[1]; //���ȣ���uchAllData[0]֡��ţ�uchAllData[1]������������
                    if(nRecvLen < 7+nDataAreaLen) //���ݳ��ȴ���
					{
						*pnDeleteLen = -3; //���ݵĳ��ȴ���
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
						*pnDeleteLen = -2; //CRC����
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
