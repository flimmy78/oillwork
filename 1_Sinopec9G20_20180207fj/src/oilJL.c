#include "../inc/main.h"

//������������ʧ��ԭ��
const char *jlStartFiledReson[]=
{
	"�ɹ�",
	"ǹѡ�Ƿ�",
	"���۲�һ��",
	"�������۷Ƿ�",
	"�����Ƿ�",
	"�����峬ʱʱ��Ƿ�",
	"��ǰ���Ƿ�",
	"�������Ƿ�",
	"�����������Ƿ�",
	"�󷧿����ӳٳ������Ƿ�",
	"�������ͷǷ�",
	"Ԥ�÷�ʽ�Ƿ�",
	"Ԥ�����Ƿ�",
	"��Դ״̬�Ƿ�",
	"˰������ʧ��",
	"ȱһ·���峬��",
	"����ͳ���",
	"������ͣ������",
	"ȱһ·����������",
};


//��������ͣ������
const char *jlStopReason[]=
{
	"����",
	"ȱһ·���壬ͣ��",
	"�ﵽԤ������ͣ��",
	"˰�ؽ�ֹ��ͣ��",
	"�����峬ʱ��ͣ��",
	"�������磬ͣ��",
	"A1ȱһ·����",
	"A2ȱһ·����",
	"B1ȱһ·����",
	"B2ȱһ·����",
	"A1ȱһ������",
    "A2ȱһ������",
    "B1ȱһ������",
    "B2ȱһ������",
};

JLClass jlOptClass;

int g_nJLLen = 0;
int g_nFd = -1;
unsigned char g_uchRsctl = 0;
unsigned char g_uchJLBuff[512];

pthread_mutex_t  g_comMutexJL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  g_dataMutexJL = PTHREAD_MUTEX_INITIALIZER;

static JlParamStruct JlParamA1, JlParamB1;

static int jlOilDataWrite(JlParamStruct *jlparam)
{
	unsigned int crc=0, offset=0, offset_backup=0;
	unsigned char wrbuffer[64]={0};

	//���㱾ǹ������Ϣ��ʼλ��ƫ��
	if(JL_NOZZLE_1==jlparam->Nozzle)	
	{
		offset=JL_FM_DATA;			
		offset_backup=JL_FM_DATA2;
	}
	else
	{
		offset=JL_FM_DATA+JL_FM_LEN;	
		offset_backup=JL_FM_DATA2+JL_FM_LEN;
	}

	//��
	//jlLock(jlparam);

	//��õ�ǰ��������
	wrbuffer[0]=(char)(jlparam->OilVolume>>16);
	wrbuffer[1]=(char)(jlparam->OilVolume>>8);	
	wrbuffer[2]=(char)(jlparam->OilVolume>>0);
	wrbuffer[3]=(char)(jlparam->OilMoney>>16);	
	wrbuffer[4]=(char)(jlparam->OilMoney>>8);	
	wrbuffer[5]=(char)(jlparam->OilMoney>>0);
	wrbuffer[6]=(char)(jlparam->OilPrice>>8);		
	wrbuffer[7]=(char)(jlparam->OilPrice>>0);
	wrbuffer[8]=(char)(jlparam->VolumeSum>>40);
	wrbuffer[9]=(char)(jlparam->VolumeSum>>32);
	wrbuffer[10]=(char)(jlparam->VolumeSum>>24);
	wrbuffer[11]=(char)(jlparam->VolumeSum>>16);
	wrbuffer[12]=(char)(jlparam->VolumeSum>>8);
	wrbuffer[13]=(char)(jlparam->VolumeSum>>0);
	wrbuffer[14]=(char)(jlparam->MoneySum>>40);
	wrbuffer[15]=(char)(jlparam->MoneySum>>32);
	wrbuffer[16]=(char)(jlparam->MoneySum>>24);
	wrbuffer[17]=(char)(jlparam->MoneySum>>16);
	wrbuffer[18]=(char)(jlparam->MoneySum>>8);		
	wrbuffer[19]=(char)(jlparam->MoneySum>>0);
	wrbuffer[20]=jlparam->StopNo;
	wrbuffer[21]=jlparam->JlState;

	//����
	//jlUnlock(jlparam);

	//����CRC
	crc=crc16Get(wrbuffer, 62);
	wrbuffer[62]=(unsigned char)(crc>>8);	
	wrbuffer[63]=(unsigned char)(crc>>0);

	//�洢������Ϣ������
	framWrite(FM_ADDR_JL, offset, wrbuffer, 64);
	framWrite(FM_ADDR_JL, offset_backup, wrbuffer, 64);

	return 0;
}

static void GetSendJLData(unsigned char uchCmd,JLReportStruct* pReportStruct,unsigned char* puchSendData,int* pnSendLen);
static bool ParseRecvJLData(unsigned char* puchReadData,int nReadLen,int* pnDeleteLen,JlParamStruct* pParamStruct);
static unsigned char GetGunState();

unsigned char GetGunState()
{
	unsigned char uchGunState = 0;
	int nChg = 0;
	int nPowerState = powerStateRead();
	int nMac1State = spi1PumpPermitRead(DEV_PUMP_PERMITA,(char*)&nChg);
	int nMac2State = spi1PumpPermitRead(DEV_PUMP_PERMITB,(char*)&nChg);

	int nLockState = spi1LockRead((char*)&nChg);
	uchGunState = ((nLockState&0x01)<<4)|((nLockState&0x01)<<3)|((nMac2State&0x01)<<2) | ((nMac1State&0x01)<<1) | (nPowerState&0x01);
    //printf("nMac1 = %d,nMac2 = %d,uchGunState = %02x\n",nMac1State,nMac2State,uchGunState);
	
	return uchGunState;
}

static bool jlSendAndRecv(unsigned char uchCmd,JLReportStruct* pReport,JlParamStruct* pParam)
{
	int nReadLen = 0;
    int nSendLen = 0;
	int nDeleteLen = 0;
	int nAllLen = 0;
    int nWriteLen = 0;
	unsigned char uchReadBuff[256];
	unsigned char uchSendBuff[256];
	unsigned char uchAllBuff[512]; 
	memset(uchReadBuff,0,sizeof(uchReadBuff));
	memset(uchSendBuff,0,sizeof(uchSendBuff));
    memset(uchAllBuff,0,sizeof(uchAllBuff));

	pthread_mutex_lock(&g_comMutexJL);
	GetSendJLData(uchCmd,pReport,uchSendBuff,&nSendLen);
	nWriteLen = write(g_nFd,uchSendBuff,nSendLen);
	
	//printf("send jl data is success:Type = %02x\n",uchCmd);
	//PrintH(nSendLen,uchSendBuff);

	if(nWriteLen > 0)
	{
		if(!((uchCmd == 0x14) || (uchCmd == 0x10)))
		{
	        g_fjLog.WriteLog("jlSendAndRecv  ","send  ",uchSendBuff,nSendLen);
		}
		//printf("recv jl data:\n");
		while((nReadLen = read(g_nFd,uchReadBuff,256)) > 0)
		{
			memcpy(uchAllBuff + nAllLen,uchReadBuff,nReadLen);
			nAllLen += nReadLen;
			
			//PrintH(nAllLen,uchAllBuff);
			if(!((uchCmd == 0x14) || (uchCmd == 0x10)))
			{
			    g_fjLog.WriteLog("jlSendAndRecv  ","recv  ",uchAllBuff,nAllLen);
			}

			if(ParseRecvJLData(uchAllBuff,nAllLen,&nDeleteLen,pParam))
			{
				//if(pReport->uchParaIndex == 19)
				//{
				//	printf("recv jl data:\n");
				//	PrintH(nAllLen,uchAllBuff);
				//}
				nAllLen = 0;
			    nReadLen = 0;
				memset(uchAllBuff,0,sizeof(uchAllBuff));
				memset(uchReadBuff,0,sizeof(uchReadBuff));
                pthread_mutex_unlock(&g_comMutexJL);
				return true;
			}
			else
			{
				if(nDeleteLen == -2 || nDeleteLen == -3 || nDeleteLen == -4)
				{
					nAllLen;
					nReadLen = 0;
					memset(uchAllBuff,0,sizeof(uchAllBuff));
					memset(uchReadBuff,0,sizeof(uchReadBuff));
					pthread_mutex_unlock(&g_comMutexJL);
					printf("******jl report error ,nDeleteLen = %d !\n",nDeleteLen);
                    g_fjLog.WriteLog("jlSendAndRecv  ","recv  ","******jl report error",0);
					return false;
				}
			}
		}	
	}
	else
	{
        pthread_mutex_unlock(&g_comMutexJL);
		printf("send jl data is failure,error = %d:\n,nWriteLen");
        g_fjLog.WriteLog("jlSendAndRecv  ","send  ","send jl data is failure",0);
		return false; //��������ʧ��
	}
    pthread_mutex_unlock(&g_comMutexJL);
	printf("recv jl data timeout******\n");
	PrintH(nSendLen,uchSendBuff);
	PrintH(nAllLen,uchAllBuff);
    g_fjLog.WriteLog("jlSendAndRecv  ","recv  ","recv jl data timeout******",0);
	return false; //���ճ�ʱ
}

/*
static bool jlSendAndRecv(unsigned char uchCmd,JLReportStruct* pReportStruct,JlParamStruct* pParamStruct)
{
    int nfd = 0;
	int nReadLen = 0;
    int nSendLen = 0;
	int nDeleteLen = 0;
	g_nJLLen = 0;
	unsigned char uchReadBuff[256];
	unsigned char uchSendBuff[256];
	memset(uchReadBuff,0,sizeof(uchReadBuff));
	memset(uchSendBuff,0,sizeof(uchSendBuff));
    memset(g_uchJLBuff,0,sizeof(g_uchJLBuff));

	int nWriteLen = 0;
    
	pthread_mutex_lock(&g_comMutexJL);
	GetSendJLData(uchCmd,pReportStruct,uchSendBuff,&nSendLen);
	//pthread_mutex_lock(&g_comMutexJL);
	nWriteLen = write(g_nFd,uchSendBuff,nSendLen);
	

	//printf("send jl data is success:\n");
	//PrintH(nSendLen,uchSendBuff);

	if(nWriteLen > 0)
	{
		//printf("recv jl data:\n");

		while((nReadLen = read(g_nFd,uchReadBuff,256)) > 0)
		{
			memcpy(g_uchJLBuff + g_nJLLen,uchReadBuff,nReadLen);
			g_nJLLen += nReadLen;
			//PrintH(g_nJLLen,g_uchJLBuff);

			if(ParseRecvJLData(g_uchJLBuff,g_nJLLen,&nDeleteLen,pParamStruct))
			{
				g_nJLLen = 0;
				memset(g_uchJLBuff,0,sizeof(g_uchJLBuff));
				nReadLen = 0;
				memset(uchReadBuff,0,sizeof(uchReadBuff));
                pthread_mutex_unlock(&g_comMutexJL);
				return true;
			}
			else
			{
				if(nDeleteLen == -2)
				{
					printf("******jl crc error !\n");
					g_nJLLen = 0;
					memset(g_uchJLBuff,0,sizeof(g_uchJLBuff));
					nReadLen = 0;
					memset(uchReadBuff,0,sizeof(uchReadBuff));
                    pthread_mutex_unlock(&g_comMutexJL);
					return false;
				}
				else if(nDeleteLen == -3)
				{
					printf("******jl data len error!\n");
					g_nJLLen = 0;
					memset(g_uchJLBuff,0,sizeof(g_uchJLBuff));
					nReadLen = 0;
					memset(uchReadBuff,0,sizeof(uchReadBuff));
                    pthread_mutex_unlock(&g_comMutexJL);
					return false;
				}
			}
		}	
	}
	else
	{
		printf("send jl data is failure,error = %d:\n,nWriteLen");
        pthread_mutex_unlock(&g_comMutexJL);
		return false; //��������ʧ��
	}
    pthread_mutex_unlock(&g_comMutexJL);

	printf("recv jl data timeout******\n");
	return false; //���ճ�ʱ
}*/


static void sendJLInit(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x1B;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[3];
    puchOutData[nOutDtLen++] = pReportStruct->uchSendData[4];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[5];
	*pnOutDtLen = nOutDtLen;
}

static void sendGetRand(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x1A;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	*pnOutDtLen = nOutDtLen;
}

static void sendJLParamSet(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x19;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	printf("set JLParam:jlOptClass.pCurJLReportStruct->uchGunNo = %d\n",pReportStruct->uchGunNo);
	unsigned char uchParaIndex = pReportStruct->uchParaIndex;
	switch(uchParaIndex)
	{
	case 1:  //�״μ춨
	case 2:  //�����춨
	case 9:  //��ǰ�ط���
	case 11: //��ʾ������
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 18:
	case 21:
	case 22:
	case 24:
    case 25:
	case 26:
	case 27:
		puchOutData[nOutDtLen++] = uchParaIndex;
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
		break;
	case 3:
		puchOutData[nOutDtLen++] = uchParaIndex;
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[3];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[4];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[5];
		break;
//	case 4: case 5: case 6: case 7:
//		break;
	case 8: //ֱͨ��ʾ
        *(puchOutData++) = uchParaIndex;
//		*(puchOutData++) = puchData[0];
//	    *(puchOutData++) = puchData[1];
//	    *(puchOutData++) = puchData[2];
//		*(puchOutData++) = puchData[3];
//	    *(puchOutData++) = puchData[4];
//	    *(puchOutData++) = puchData[5];
//	    *(puchOutData++) = puchData[6];
//	    *(puchOutData++) = puchData[7];
//	    *(puchOutData++) = puchData[8];
		break;
	case 10://����
		puchOutData[nOutDtLen++] = uchParaIndex;
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
		break;
	case 17://Kֵ�����嵱����
	    puchOutData[nOutDtLen++] = uchParaIndex;
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
		break;
	case 20://��������汾
	case 23://���������
		puchOutData[nOutDtLen++] = uchParaIndex;
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
		break;
	default:
		break;
	}
	*pnOutDtLen = nOutDtLen;
}

static void sendJLParamQuery(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x18;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	unsigned char uchParaIndex = pReportStruct->uchParaIndex;
	puchOutData[nOutDtLen++] = uchParaIndex;
	switch(uchParaIndex)
	{
	case 4://���ۼ�
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
        puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
        puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
        puchOutData[nOutDtLen++] = pReportStruct->uchSendData[3];
		break;
	case 5://���ۼ�
		puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
        puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
        puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
		break;
    default:
		break;
	}
    *pnOutDtLen = nOutDtLen;
}

static void sendJLOilRounding(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x17;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
	*pnOutDtLen = nOutDtLen;
}

static void sendJLTradeOK(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x16;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	*pnOutDtLen = nOutDtLen;
}

static void sendJLAddOilEnd(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x15;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
    *pnOutDtLen = nOutDtLen;
}

static void sendGetOilData(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x14;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
	*pnOutDtLen = nOutDtLen;
}

static void sendJLAddOilStart(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x13;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[1];
    puchOutData[nOutDtLen++] = pReportStruct->uchSendData[2];
    puchOutData[nOutDtLen++] = pReportStruct->uchSendData[3];
    puchOutData[nOutDtLen++] = pReportStruct->uchSendData[4];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[5];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[6];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendData[7];
	*pnOutDtLen = nOutDtLen;
}

static void sendClearOilData(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x12;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
    *pnOutDtLen = nOutDtLen;
}

static void sendShowOilData(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x11;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	*pnOutDtLen = nOutDtLen;
}

static void sendJLStateQuery(unsigned char* puchOutData,int* pnOutDtLen,JLReportStruct* pReportStruct)
{
	int nOutDtLen = 0;
	puchOutData[nOutDtLen++] = 0x10;
	puchOutData[nOutDtLen++] = pReportStruct->uchGunNo;
	//puchOutData[nOutDtLen++] = pReportStruct->uchSendData[0];
	puchOutData[nOutDtLen++] = pReportStruct->uchSendGunStateData;  //fj:20171110,update
	*pnOutDtLen = nOutDtLen;
}

//------------fj:˰�ز�ѯ
/********************************************************************
/********************************************************************
*Name				:jlTaxTimeDsp
*Description		:˰��ʱ����ʾ
*Input				:None
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlTaxTimeDsp(void)
{
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)//�ж�״̬�������ǿ���״̬����������
	{
		return ERROR;
	}

	JlParamStruct* pParam = &JlParamA1;
	JLReportStruct* pReport =  &jlOptClass.jlStruReportA; 
	pReport->uchParaIndex = 3;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{ 
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 3,jlTaxTimeDsp\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlTaxDayVolDsp
*Description		:˰��������ʾ
*Input				:nozzle		0=1��ǹ(��A1ǹ)��1=2��ǹ(��B1ǹ)
*						:time		���ڣ�4�ֽ�ѹ��BCD�룬��ʽΪCCYYMMDD
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlTaxDayVolDsp(int nozzle, char *time, int nbytes)
{	
	if(0!=timeVerification(time, 4))//�ж�ʱ��Ϸ���
	{
		return ERROR;
	}
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 4;
	pReport->uchSendData[0] = time[0];
    pReport->uchSendData[1] = time[1];
    pReport->uchSendData[2] = time[2];
    pReport->uchSendData[3] = time[3];

    if(jlSendAndRecv(0x18,pReport,pParam))
	{ 
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 4,jlTaxDayVolDsp\n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlTaxMonthVolDsp
*Description		:˰��������ʾ
*Input				:nozzle		0=1��ǹ(��A1ǹ)��1=2��ǹ(��B1ǹ)
*						:time		�·ݣ�3�ֽ�ѹ��BCD�룬��ʽΪYYYYMM
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlTaxMonthVolDsp(int nozzle, char *time, int nbytes)
{	
	if(0!=timeVerification(time, 3))//�ж�ʱ��Ϸ���
	{
		return ERROR;
	}
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 5;
	pReport->uchSendData[0] = time[0];
    pReport->uchSendData[1] = time[1];
    pReport->uchSendData[2] = time[2];

    if(jlSendAndRecv(0x18,pReport,pParam))
	{ 
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 5,jlTaxMonthVolDsp\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlTaxSumDsp
*Description		:˰������������ʾ
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:handle		0=��ʾ�������ݣ�1=��ʾ�����춨���ݣ�2=�׼��ۼ�
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlTaxSumDsp(int nozzle, int handle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	switch(handle)
	{
	case 0: //��ѯ˰���ۼ�
        pReport->uchParaIndex = 6;
		break;
	case 1://��ѯ�׼��ۼ�
        pReport->uchParaIndex = 2;
		break;
	case 2://��ѯ�����ۼ�
        pReport->uchParaIndex = 1;
		break;
	default:
		break;
	}

    if(jlSendAndRecv(0x18,pReport,pParam))
	{ 
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 6,jlTaxSumDsp\n");
		return ERROR;
	}
}

int jlTaxNoteDsp(int nozzle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 7;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{ 
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 7,jlTaxNoteDsp\n");
		return ERROR;
	}
}

//********************fj:˰���������*****************
/********************************************************************
*Name				:jlTaxDsp
*Description		:˰�ش�����ʾ
*Input				:nozzle		ǹ��0=1��ǹ(��A1ǹ)��1=2��ǹ(��B1ǹ)
*						:buffer		��ʾ���棬�̶�����Ϊ16�ֽ�ASCII�ַ�������ʾ�ַ�����\
*						:				����0~9����ĸ'A','L','P','H','E'������ʾ��Ϊ�ո����' '��\
*						:				��ʾ˳������������λ����λ->�������λ����λ->��������λ����λ
*						:point		0=��������ʾС���㣻1=������ʾС����
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlTaxDsp(unsigned char nozzle, unsigned char *buffer, unsigned char point)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 8;
    pReport->uchSendData[0] = buffer[0];
    pReport->uchSendData[1] = buffer[1];
    pReport->uchSendData[2] = buffer[2];
    pReport->uchSendData[3] = buffer[3];
    pReport->uchSendData[4] = buffer[4];
    pReport->uchSendData[5] = buffer[5];
    pReport->uchSendData[6] = buffer[6];
    pReport->uchSendData[7] = buffer[7];
    pReport->uchSendData[8] = point;

    if(jlSendAndRecv(0x19,pReport,pParam))  
	{
		return pParam->uchResult_Set;
	}
	else
	{
        printf("set jlparam failure,type = 8,jlTaxDsp\n");
		return ERROR;
	}
}

extern int jlTaxVerifyWrite(int handle)
{
	if(JlParamA1.JlState != 0 || JlParamB1.JlState != 0)
	{
		return ERROR;
	}

	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	pParam = &JlParamA1;
	pReport = &jlOptClass.jlStruReportA;

	switch(handle)
	{
	case 0:
		{
            pReport->uchParaIndex = 2;
	        pReport->uchSendData[0] = (unsigned char)handle;
		}
		break;
	case 1:
		{
            pReport->uchParaIndex = 1;
        	pReport->uchSendData[0] = (unsigned char)handle;
		}
		break;
	default:
		break;
	}
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		return pParam->uchResult_Set;
	}
	else
	{
        printf("set jlparam failure,type = 1,2,jlTypeWritpe\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlTaxTimeWrite
*Description		:˰��ʱ�����ã�����״̬��������в���
*Input				:time		ʱ��(BCD��6bytes��YYYYMMDDHHMM)
*						:nbytes		ʱ�����ݳ��ȣ��̶�6�ֽ�
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlTaxTimeWrite(char *time, int nbytes)
{
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}
	if(6!=nbytes)//�ж�ʱ�䳤��
	{
		return ERROR;
	}	
	if(0!=timeVerification(time, 6))//�ж�ʱ��Ϸ���
	{
		return ERROR;
	}

	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	pParam = &JlParamA1;
	pReport = &jlOptClass.jlStruReportA;

	pReport->uchParaIndex = 3;
    pReport->uchSendData[0] = time[0];
    pReport->uchSendData[1] = time[1];
    pReport->uchSendData[2] = time[2];
    pReport->uchSendData[3] = time[3];
    pReport->uchSendData[4] = time[4];
    pReport->uchSendData[5] = time[5];

    if(jlSendAndRecv(0x19,pReport,pParam))  
	{
		return pParam->uchResult_Set;
	}
	else
	{
        printf("set jlparam failure,type = 3,jlTaxTimeWrite\n");
		return ERROR;
	}
}


//------------fj:������ѯ
/********************************************************************
*Name				:jlSpeedRead
*Description		:��ȡ��������
*Input				:maxbytes	���������󳤶�
*Output			:buffer			�������
*Return			:���٣���λ0.01��/�룻���󷵻�ERROR
*History			:2013-08-05,modified by syj
*/
int jlSpeedRead(int nozzle)
{
	int pspeed_1 = 0, pspeed_2 = 0, pspeed_3 = 0, pspeed_4 = 0, pspeed = 0;
	int oilspeed = 0;
	JlParamStruct *jlparam=NULL;

	//�ж�ǹѡ
	if(0==nozzle)			
	{
		jlparam=&JlParamA1;
	}
	else if(1==nozzle)	
	{
		jlparam=&JlParamB1;
	}
	else							
		return ERROR;

	//�����ٶ�,fj:
/*	if(1 == ((jlparam->Pulse>>0)&1))	//A1·
	{	
		pspeed_1 = sensorSpeedRead(SENSOR_A11) + sensorSpeedRead(SENSOR_A12);
	}
	if(1 == ((jlparam->Pulse>>1)&1))	//A2·
	{	
		pspeed_2 = sensorSpeedRead(SENSOR_A21) + sensorSpeedRead(SENSOR_A22);
	}
	if(1 == ((jlparam->Pulse>>2)&1))	//B1·
	{	
		pspeed_3 = sensorSpeedRead(SENSOR_B11) + sensorSpeedRead(SENSOR_B12);
	}
	if(1 == ((jlparam->Pulse>>3)&1))	//B2·
	{	
		pspeed_4 = sensorSpeedRead(SENSOR_B21) + sensorSpeedRead(SENSOR_B22);
	}
	pspeed = pspeed_1 + pspeed_2 + pspeed_3 + pspeed_4;

	//��������
	oilspeed = pspeed * jlparam->Equivalent / 10000;*/
	
	return oilspeed;
}

/********************************************************************
*Name				:jlVersionRead
*Description		:��ȡ��������汾
*Input				:maxbytes	���������󳤶�
*Output			:buffer			�������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlVersionRead(char *buffer, int maxbytes)
{
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	pParam = &JlParamA1;
	pReport = &jlOptClass.jlStruReportA;

	pReport->uchParaIndex = 20;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
        sprintf(buffer,"V%d.%d",pParam->uchData_Query[0],pParam->uchData_Query[1]);
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 19,jl sum\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlSumRead
*Description		:��ȡ������������
*Input				:nozzle			0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:volume		���ۼ�������
*						:money			���ۼ��ͽ��
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlSumRead(int nozzle, unsigned long long *volume, unsigned long long *money)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 19;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		long long llVolumeTemp = ((unsigned long long)pParam->uchData_Query[0]<<32)|((unsigned long long)pParam->uchData_Query[1]<<24)|((unsigned long long)pParam->uchData_Query[2]<<16)|((unsigned long long)pParam->uchData_Query[3]<<8)|pParam->uchData_Query[4];
		long long llAmountTemp = ((unsigned long long)pParam->uchData_Query[5]<<32)|((unsigned long long)pParam->uchData_Query[6]<<24)|((unsigned long long)pParam->uchData_Query[7]<<16)|((unsigned long long)pParam->uchData_Query[8]<<8)|pParam->uchData_Query[9];
    	//printf("price --%02x%02x%02x\n",g_pCurJLParam->uchData_Query[0],g_pCurJLParam->uchData_Query[1],g_pCurJLParam->uchData_Query[2]);  
        //unsigned int nPriceHex = (int)g_pCurJLParam->uchData_Query[0] << 16 | (int)g_pCurJLParam->uchData_Query[1] << 8 | g_pCurJLParam->uchData_Query[2];
		//unsigned int nPriceDec = hexbcd2int(nPriceHex);
		long long llVolume = hexbcd2longlong(llVolumeTemp);
		long long llAmount = hexbcd2longlong(llAmountTemp);
		printf("long long jl sum volume,amount--%lld,%lld\n",llVolume,llAmount);
		*volume = llVolume;
        *money = llAmount;
		pParam->VolumeSum = llVolume;
		pParam->MoneySum = llAmount;
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 19,jl sum\n");
		return ERROR;
	}
}

int GetjlSumRead(int nozzle, unsigned long long *volume, unsigned long long *money)
{
	JlParamStruct* pParam = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
	}
	else
	{
		return ERROR;
	}

   //pthread_mutex_lock(&g_dataMutexJL);
	*volume = pParam->VolumeSum;
	*money = pParam->MoneySum;
	//pthread_mutex_unlock(&g_dataMutexJL);
	return 0;
}

/********************************************************************
*Name				:jlPriceRead
*Description		:��ȡ��������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:price		��������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlPriceRead(int nozzle, unsigned int *price)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}
	pReport->uchParaIndex = 10;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
    	//printf("price --%02x%02x%02x\n",g_pCurJLParam->uchData_Query[0],g_pCurJLParam->uchData_Query[1],g_pCurJLParam->uchData_Query[2]);  
		//pthread_mutex_lock(&g_dataMutexJL);
		int nState = pParam->uchResult_Query;
		if(nState == 0)
		{
			unsigned int nPriceHex = (int)pParam->uchData_Query[0] << 16 | (int)pParam->uchData_Query[1] << 8 | pParam->uchData_Query[2];
			unsigned int nPriceDec = hexbcd2int(nPriceHex);
			pParam->Price = nPriceDec;
			//printf("int price --%d\n",nPriceDec);
			*price = nPriceDec;	
		}
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 10,oil price\n");
		return ERROR;
	}
}

int GetjlPriceRead(int nozzle, unsigned int *price)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	*price = pParam->Price;

    return 0;
}

/********************************************************************
*Name				:jlEquivalentRead
*Description		:��ȡ��������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:equivalent		��������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlEquivalentRead(int nozzle, unsigned int *equivalent)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 17;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		//printf("equivalent --%02x%02x%02x\n",g_pCurJLParam->uchData_Query[0],g_pCurJLParam->uchData_Query[1],g_pCurJLParam->uchData_Query[2]);  
        unsigned int nEquivalentHex = (int)pParam->uchData_Query[0] << 16 | (int)pParam->uchData_Query[1] << 8 | pParam->uchData_Query[2];
	    int nEquivalentDec = hexbcd2int(nEquivalentHex);

		//printf("equivalent --%d\n",nEquivalent);
		//nEquivalent = 11111;
		*equivalent = nEquivalentDec;
		return pParam->uchResult_Query;
	}
	else
	{
        printf("query jlparam failure,type = 17,k value,equivalent\n");
		return ERROR;
	}	
}

/********************************************************************
*Name				:jlUnPulseTimeRead
*Description		:��ȡ���������峬ʱͣ��ʱ��
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:param		���������峬ʱͣ��ʱ��
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlUnPulseTimeRead(int nozzle, unsigned int *param)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 13;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*param = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 13,jl timeout stop times \n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlAdvanceRead
*Description		:��ȡ������ǰ��
*Input				:nozzle			0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:advance		��ǰ��
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlAdvanceRead(int nozzle, unsigned int *advance)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 9;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*advance = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 9,jl timeout stop times \n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlShieldRead
*Description		:��ȡ������,fj:����ʾ��Ļ����?
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:shield		������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlShieldRead(int nozzle, unsigned int *shield)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 11;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*shield = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 11,view shield \n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlOverShieldRead
*Description		:��ȡ����������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:overShield		����������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlOverShieldRead(int nozzle, unsigned int *overShield)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 12;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*overShield = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 12,jlOverShieldRead\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlValveVolumeRead
*Description		:��ȡ��������������
*Input				:nozzle					0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:valveVolume		��������������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlValveVolumeRead(int nozzle, unsigned int *valveVolume)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 15;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*valveVolume = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 15,jlValveVolumeRead\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlValveStopTimeRead
*Description		:��ȡ���������峬ʱ�رմ�ʱ��
*Input				:nozzle					0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:valveStopTime		���������峬ʱ�رմ�ʱ�䣬��λ:��
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlValveStopTimeRead(int nozzle, unsigned int *valveStopTime)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 14;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*valveStopTime = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 14,jlValveStopTimeRead\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlAlgorithmRead
*Description		:��ȡ�����������㷨����
*Input				:nozzle			0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:algorithm		�����������㷨���� 0x00=�������룻0x01=���������룻
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlAlgorithmRead(int nozzle, unsigned int *algorithm)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 16;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		*algorithm = pParam->uchData_Query[0];
		nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 16,jlAlgorithmRead\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlNoteDsp
*Description		:��������������ʾ
*Input				:nozzle					0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlNoteDsp(int nozzle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    if(jlSendAndRecv(0x11,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nState = pParam->uchResult_ViewOilData;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 16,jlAlgorithmRead\n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlTypeRead
*Description		:��ȡ��������
*Input				:None
*Output			:None
*Return				:��������
*History			:2013-08-05,modified by syj
*/
int jlTypeRead(int nozzle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
	    return pParam->Type;
	}

	pReport->uchParaIndex = 18;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nType = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nType = pParam->uchData_Query[0];
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nType;
	}
	else
	{
        printf("query jlparam failure,type = 18,jlTypeRead\n");
		return 1; //��ͨ��˫ǹ
	}
}

/********************************************************************
*Name				:jlTaxEquivalentRead
*Description		:ͨ��˰�ر�˰�ڣ���ȡ˰��Kֵ
*Input				:nozzle			0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:equivalent	��ȡ���ĵ���(ѹ��BCD��ʽ����Kֵ5000�������ֵΪ0x5000)
*Return				:0=�ɹ�������=ʧ��
*History			:2016-03-01,modified by syj
*/
int jlTaxEquivalentRead(int nozzle, int *equivalent)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 27;

    if(jlSendAndRecv(0x18,pReport,pParam))
	{
		int nState = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		unsigned char uchTemp = pParam->uchData_Query[0];
		*equivalent = uchTemp / 0x10 * 10 + uchTemp % 0x10;
        nState = pParam->uchResult_Query;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nState;
	}
	else
	{
        printf("query jlparam failure,type = 27,jlTaxEquivalentRead\n");
		return ERROR;
	}
}

//szb_fj_20171120,add
int jlBigVolTimeRead(int nozzle,unsigned int *time)
{
	JlParamStruct *jlparam=NULL;
	if(0==nozzle)			
	{
		jlparam=&JlParamA1;
	}
	else if(1==nozzle)	
	{
		jlparam=&JlParamB1;
	}
	else		
		return ERROR;

	//jlLock(jlparam);

	*time=jlparam->bigVolTime;

	//jlUnlock(jlparam);
	
	return	0;
}

//szb_fj_20171120,add
int jlBigVolSpeedRead(int nozzle,unsigned int *Speed)
{
	JlParamStruct *jlparam=NULL;

	if(0==nozzle)			
	{
		jlparam=&JlParamA1;
	}
	else if(1==nozzle)	
	{
		jlparam=&JlParamB1;
	}
	else
		return ERROR;

	//jlLock(jlparam);

	*Speed=jlparam->bigVolSpeed;

	//jlUnlock(jlparam);
	
	return	0;
}

//------------fj:����������ز���
/********************************************************************
*Name				:jlPriceWrite
*Description		:��������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:price		����
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlPriceWrite(int nozzle, unsigned int price)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
		//printf("----nozzle = %d,jlOptClass nozzle = %d\n",nozzle,jlOptClass.pCurJLReportStruct->uchGunNo);
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	    //printf("*****nozzle = %d,jlOptClass nozzle = %d\n",nozzle,jlOptClass.pCurJLReportStruct->uchGunNo);
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 10;
	unsigned long long llTemp = hex2Bcd(price);
    pReport->uchSendData[0] = (llTemp>>16) & 0xff;
	pReport->uchSendData[1] = (llTemp>>8) & 0xff;
	pReport->uchSendData[2] = llTemp & 0xff;

    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 10,oil price\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlEquivalentWrite
*Description		:������������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:equivalent		����
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlEquivalentWrite(int nozzle, unsigned int equivalent)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}

	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	pReport->uchParaIndex = 17;
	unsigned long long llTemp = hex2Bcd(equivalent);
	pReport->uchSendData[0] = (llTemp>>16) & 0xff;
	pReport->uchSendData[1] = (llTemp>>8) & 0xff;
	pReport->uchSendData[2] = llTemp & 0xff;

    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 17,k value,equivalent\n");
		return ERROR;
	}
}

//���������峬ʱʱ��
int jlUnPulseTimeWrite(int nozzle_num, unsigned int unPulseTime)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle_num)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle_num)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    pReport->uchParaIndex = 13;
	pReport->uchSendData[0] = (unsigned char)unPulseTime;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 13,jlUnPulseTimeWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlAdvanceWrite
*Description		:������ǰ�����ã���Χ0.05��~9.99��
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:advance	��ǰ������λ:0.01��
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlAdvanceWrite(int nozzle, unsigned int advance)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}
	//printf("advance = %d\n",advance);
    pReport->uchParaIndex = 9;
	pReport->uchSendData[0] = (unsigned char)advance;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 9,jlAdvanceWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlShieldWrite
*Description		:��������������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:shiled		������
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlShieldWrite(int nozzle, unsigned int shiled)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}
    pReport->uchParaIndex = 11;
	pReport->uchSendData[0] = (unsigned char)shiled;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 11,jlShieldWrite\n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlOverShieldWrite
*Description		:������������������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:overShield		����������
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlOverShieldWrite(int nozzle, unsigned int overShield)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    pReport->uchParaIndex = 12;
	pReport->uchSendData[0] = (unsigned char)overShield;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 12,jlOverShieldWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlValveVolumeWrite
*Description		:�����ﵽһ��ֵʱ������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:valveVolume	�󷧿�������
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlValveVolumeWrite(int nozzle, unsigned int valveVolume)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    pReport->uchParaIndex = 15;
	pReport->uchSendData[0] = (unsigned char)valveVolume;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 15,jlValveVolumeWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlValveStopTimeWrite
*Description		:������ʱ�䳬����ֵʱ�رմ�
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:valveStop		�󷧹ر�ʱ�䣬��λ:��
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlValveStopTimeWrite(int nozzle, unsigned int valveStop)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    pReport->uchParaIndex = 14;
	pReport->uchSendData[0] = (unsigned char)valveStop;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 14,jlValveStopTimeWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlAlgorithmWrite
*Description		:�������㷨��������
*Input				:nozzle				0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:algorithm			�������㷨���� 0x00=��������(JL_ALGORITHM_45)��0x01=����������(JL_ALGORITHM_UP)��
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2016-01-04,modified by syj
*/
int jlAlgorithmWrite(int nozzle, unsigned int algorithm)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}
    pReport->uchParaIndex = 16;
	pReport->uchSendData[0] = (unsigned char)algorithm;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 16,jlAlgorithmWrite\n");
		return ERROR;
	}
}

/********************************************************************
*Name				:jlTypeWrite
*Description		:������������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:type		����
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2013-08-05,modified by syj
*/
int jlTypeWrite(int nozzle, unsigned int type)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    pReport->uchParaIndex = 18;
	pReport->uchSendData[0] = (unsigned char)type;
	
    if(jlSendAndRecv(0x19,pReport,pParam))
	{
		int nSet = 0;
		//pthread_mutex_lock(&g_dataMutexJL);
		nSet = pParam->uchResult_Set;
		//pthread_mutex_unlock(&g_dataMutexJL);
		return nSet;
	}
	else
	{
        printf("set jlparam failure,type = 18,jlTypeWrite\n");
		return ERROR;
	}
}

//szb_fj_20171120,add
int jlBigVolTimeWrite(int nozzle,unsigned int time)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	if(0==nozzle)			
	{
		jlparam=&JlParamA1;	
		offset=JL0_BIG_VOL_TIME;
	}
	else if(1==nozzle)	
	{
		jlparam=&JlParamB1;	
		offset=JL1_BIG_VOL_TIME;
	}
	else
		return ERROR;

	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	wrbuffer[0]=(unsigned char)(time>>0);
	istate=paramSetupWrite(offset, wrbuffer, 1);
	if(0!=istate)
	{
		return ERROR;
	}
	
	//jlLock(jlparam);

	jlparam->bigVolTime=(time>>4)*10+(time&0x0f);

	//jlUnlock(jlparam);

	return	0;
}

//szb_fj_20171120,add
int jlBigVolSpeedWrite(int nozzle,unsigned int Speed)
{
	off_t offset=0;
	int istate=0;
	unsigned char wrbuffer[8]={0};
	JlParamStruct *jlparam=NULL;

	
	if(0==nozzle)		
	{
		jlparam=&JlParamA1;	
		offset=JL0_BIG_VOL_SPEED;
	}
	else if(1==nozzle)	
	{
		jlparam=&JlParamB1;	
		offset=JL1_BIG_VOL_SPEED;
	}
	else
		return ERROR;

	if(JL_STATE_IDLE!=jlparam->JlState)
	{
		return ERROR;
	}

	wrbuffer[0]=(unsigned char)(Speed>>0);
	istate=paramSetupWrite(offset, wrbuffer, 1);
	if(0!=istate)
	{
		return ERROR;
	}

	//jlLock(jlparam);

	jlparam->bigVolSpeed=(Speed>>4)*10+(Speed&0x0f);

	//jlUnlock(jlparam);

	return	0;
}

/********************************************************************
*Name				:jlOilDataClr
*Description		:����������۵����� 
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:type		����
*Output			:None
*Return				:0=�ɹ�������=ʧ�� 
*History			:2016-03-11,modified by syj
*/
int jlOilDataClr(void)
{
	JlParamStruct *jlparam = NULL;

	//�ж�״̬�������ǿ���״̬����������
	if(JL_STATE_IDLE!=JlParamA1.JlState || JL_STATE_IDLE!=JlParamB1.JlState)
	{
		return ERROR;
	}

/*	jlparam = &JlParamA1;
	jlparam->OilMoney = 0;	jlparam->OilVolume = 0;	jlparam->OilPrice = 0;
	jlparam->VolumeSum = 0;	jlparam->MoneySum = 0;
	jlparam->StopNo = 0;	jlparam->JlState = 0;
	jlOilDataWrite(jlparam);

	jlparam = &JlParamB1;
	jlparam->OilMoney = 0;	jlparam->OilVolume = 0;	jlparam->OilPrice = 0;
	jlparam->VolumeSum = 0;	jlparam->MoneySum = 0;
	jlparam->StopNo = 0;	jlparam->JlState = 0;
	jlOilDataWrite(jlparam);*/

	return	0;
}

int jlSumWrite(int nozzle, int element, unsigned long long value)
{
	JlParamStruct *jlparam=NULL;

	//�ж�ǹѡ
	if(0==nozzle)
	{
		jlparam = &JlParamA1;
	}
	else if(1==nozzle)
	{
		jlparam = &JlParamB1;
	}
	else
	{
		printf("%s:%d:ǹѡ���� [nozzle = %d] !\n", __FUNCTION__, __LINE__, nozzle);
		return ERROR;
	}

	//�жϼ���״̬��ֻ�п���״̬�������
/*	if(JL_STATE_IDLE != jlparam->JlState)
	{
		return ERROR;
	}

	//��������
	jlLock(jlparam);
	if(0 == element)	jlparam->VolumeSum = value;	
	else						jlparam->MoneySum = value;
	jlUnlock(jlparam);  */

	

	return 0;
}


/**********************************************************************
*Name				:jlParamInit
*Description		:����ģ�������ʼ��������������Ϣ����������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-06-09,modified by syj
*/
int jlParamInit(int nozzle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)		
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)	
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

    if(jlSendAndRecv(0x1a,pReport,pParam))
	{
		//pthread_mutex_lock(&g_dataMutexJL);
		int i;
		for(i = 0; i < 6; i++)
		{
            pReport->uchSendData[i] = pParam->uchRand[i];
		}
		//pthread_mutex_unlock(&g_dataMutexJL);
		//printf("------jlrand \n");
		//PrintH(6,jlOptClass.pCurJLReportStruct->uchSendData);
	}
	else
	{
        printf("init jlparam failure,get rand error!\n");
		return ERROR;
	}

	if(jlSendAndRecv(0x1b,pReport,pParam))
	{
		return pParam->uchResult_JLInit;
	}
	else
	{
        printf("init jlparam failure,0x1b error!\n");
		return ERROR;
	}
}

//fj:------------���͹��̲���
/********************************************************************
*Name				:jlPresetAmountCalculate
*Description		:����Ԥ����������Ԥ�ý��(����)����Ԥ������(���)��������������������������������Ľ�СֵΪ��������
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:data		Ԥ������:
*						:				����� 4Hex +
*						:				������� 4Hex +
*						:				Ԥ�ý�� 4Hex +
*						:				Ԥ������ 4Hex +
*						:				�۸�		  4Hex +
*						:				Ԥ�÷�ʽ 1Bin(0=���⣻1=��������2=�����)
*Output			:data		Ԥ������:
*						:				����� 4Hex +
*						:				������� 4Hex +
*						:				Ԥ�ý�� 4Hex +
*						:				Ԥ������ 4Hex +
*						:				�۸�		  4Hex +
*						:				Ԥ�÷�ʽ 1Bin(0=���⣻1=��������2=�����)
*Return				:�ɹ�����0���������ش���
*History			:2015-08-05,modified by syj
*/
int jlPresetAmountCalculate(int nozzle, unsigned char *data)
{
	unsigned long long temp_volume = 0;
	unsigned int pmoney_max = 0, pvolume_max = 0;
	unsigned int pmoney = 0, pvolume = 0, pprice = 0;
	unsigned char pmode = 0;
	unsigned char tmp = 0;
	JlParamStruct *jlparam=NULL;

	/*????????*/
	pmoney_max = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|(data[3]<<0);
	pvolume_max = (data[4]<<24)|(data[5]<<16)|(data[6]<<8)|(data[7]<<0);
	pmoney = (data[8]<<24)|(data[9]<<16)|(data[10]<<8)|(data[11]<<0);
	pvolume = (data[12]<<24)|(data[13]<<16)|(data[14]<<8)|(data[15]<<0);
	pprice = (data[16]<<24)|(data[17]<<16)|(data[18]<<8)|(data[19]<<0);
	pmode = data[20];

	/*?��????*/
	if(0 == pprice)
	{
		printf("%s:%d:??????!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	/*?��???*/
	if(0==nozzle)			{jlparam=&JlParamA1;}
	else if(1==nozzle)	{jlparam=&JlParamB1;}
	else							{printf("%s:%d:?????? [nozzle = %d] !\n", __FUNCTION__, __LINE__, nozzle);return ERROR;}

	/*??????????????????????????????????????��?????????????
	*	???????????????????????????????????��???????????????????????????????????????????
	*/
	if(1 != pmode && 2 != pmode)
	{
		if(pmoney_max <= JL_MONEY_MAX)	pmoney_max = pmoney_max;
		else														pmoney_max = JL_MONEY_MAX;
		if(pvolume_max <= JL_MONEY_MAX)pvolume_max = pvolume_max;
		else														pvolume_max = JL_MONEY_MAX;
		//if(pprice < 100)	{tmp = 1;	pvolume = pvolume_max;}
		//else					{tmp = 2;	pmoney = pmoney_max;}

		if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney_max*10000/pprice%100) > 0)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney_max*10000/pprice%100) == 0)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney_max*1000/pprice%10) >= JL_ROUNDING)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney_max*1000/pprice%10) < JL_ROUNDING)
		{
			temp_volume = (unsigned long long)pmoney_max*100/pprice;
		}
		if(temp_volume <= pvolume_max)	{tmp = 2;	pmoney = pmoney_max;}
		else														{tmp = 1;	pvolume = pvolume_max;}
	}
	/*??????????????��??????: ??????????????????????*/
	if(1 == pmode || 1 == tmp)
	{
		if(JL_ALGORITHM_UP == jlparam->Algorithm)
		{
			pmoney= (unsigned long long)pvolume*pprice/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pvolume*pprice/10%10)<JL_ROUNDING)
		{
			pmoney = (unsigned long long)pvolume*pprice/100;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pvolume*pprice/10%10)>=JL_ROUNDING)
		{
			pmoney = (unsigned long long)pvolume*pprice/100 + 1;
		}

		pmode = 1;
	}
	/*??y????????��???????: ????????????�˧�???0????????0???��????????????????�˧�??��??5????????��??5???��*/
	if(2 == pmode || 2 == tmp)
	{
		if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney*10000/pprice%100) > 0)
		{
			pvolume = (unsigned long long)pmoney*100/pprice+1;
		}
		else if(JL_ALGORITHM_UP == jlparam->Algorithm && ((unsigned long long)pmoney*10000/pprice%100) == 0)
		{
			pvolume = (unsigned long long)pmoney*100/pprice;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney*1000/pprice%10) >= JL_ROUNDING)
		{
			pvolume = (unsigned long long)pmoney*100/pprice + 1;
		}
		else if(JL_ALGORITHM_UP != jlparam->Algorithm && ((unsigned long long)pmoney*1000/pprice%10) < JL_ROUNDING)
		{
			pvolume = (unsigned long long)pmoney*100/pprice;
		}

		pmode = 2;
	}

	/*????????*/
	data[0] = (char)(pmoney_max>>24);	data[1] = (char)(pmoney_max>>16);
	data[2] = (char)(pmoney_max>>8);	data[3] = (char)(pmoney_max>>0);
	data[4] = (char)(pvolume_max>>24);	data[5] = (char)(pvolume_max>>16);
	data[6] = (char)(pvolume_max>>8);	data[7] = (char)(pvolume_max>>0);
	data[8] = (char)(pmoney>>24);			data[9] = (char)(pmoney>>16);
	data[10] = (char)(pmoney>>8);			data[11] = (char)(pmoney>>0);
	data[12] = (char)(pvolume>>24);		data[13] = (char)(pvolume>>16);
	data[14] = (char)(pvolume>>8);			data[15] = (char)(pvolume>>0);
	data[16] = (char)(pprice>>24);			data[17] = (char)(pprice>>16);
	data[18] = (char)(pprice>>8);				data[19] = (char)(pprice>>0);
	data[20] = pmode;
	return 0;
}

/********************************************************************
*Name				:jlOilStart
*Description		:����������������
*Input				:nozzle					ǹѡ0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:preset_value		Ԥ�������������ʱΪ�����
*						:preset_price			Ԥ֧����
*						:preset_mode		Ԥ�÷�ʽ0=���⣻1=��������2=�����
*Output			:None
*Return				:0x00=�ɹ�������=ʧ��
*						:0x01=ǹѡ�Ƿ���
*						:0x02=���۲�һ�£�
*						:0x03=�������۷Ƿ���
*						:0x04=�����Ƿ���
*						:0x05=�����峬ʱʱ��Ƿ���
*						:0x06=��ǰ���Ƿ���
*						:0x07=�������Ƿ���
*						:0x08=�����������Ƿ���
*						:0x09=�󷧿����ӳٳ������Ƿ���
*						:0x10=�������ͷǷ���
*						:0x11=Ԥ�÷�ʽ�Ƿ���
*						:0x12=Ԥ�����Ƿ���
*						:0x13=��Դ״̬�Ƿ���
*						:0x14=˰������ʧ�ܣ�
*						:0x15=ȱһ·���峬�Σ�
*						:0x16=����ͳ��Σ�
*						:0x17=������ͣ�����Σ�
*						:0x18=ȱһ·���������Σ�
*						:��ֵ���Ӧ jlStartFiledReson ��ͣ��ԭ�򣬹����ʱ����BCD��˳����ӣ�����jlStartFiledReson����Ӷ�Ӧ��Ϣ
*History			:2014-03-03,modified by syj
*/

int jlOilStart(int nozzle, unsigned int preset, unsigned int preset_price, unsigned int preset_mode)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return 0x01;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	//printf("preset = %d,preset_price = %d\n",preset,preset_price);
    switch(preset_mode)
	{
	case 0: //������� ---- Ԥ�ý��
		{
			pReport->uchSendData[0] = 0;
	        unsigned long long llTempCount = hex2Bcd(preset);
			unsigned long long llTempPrice = hex2Bcd(preset_price);
			pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			pReport->uchSendData[5] = (llTempPrice>>16)&0xff;
	        pReport->uchSendData[6] = (llTempPrice>>8)&0xff;
	        pReport->uchSendData[7] = llTempPrice&0xff; 
		}
		break;
	case 1: //Ԥ������ ----- Ԥ������
		{
			pReport->uchSendData[0] = preset_mode;
	        unsigned long long llTempCount = hex2Bcd(preset);
	        pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			unsigned long long llTemp = hex2Bcd(preset_price);
			pReport->uchSendData[5] = (llTemp>>16) & 0xff;
	        pReport->uchSendData[6] = (llTemp>>8) & 0xff;
	        pReport->uchSendData[7] = llTemp & 0xff;   
		}
		break;
	case 2: //Ԥ�ý�� ---- ������� 
		{
			pReport->uchSendData[0] = 0;
	        unsigned long long llTempCount = hex2Bcd(preset);
			unsigned long long llTempPrice = hex2Bcd(preset_price);
			pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			pReport->uchSendData[5] = (llTempPrice>>16)&0xff;
	        pReport->uchSendData[6] = (llTempPrice>>8)&0xff;
	        pReport->uchSendData[7] = llTempPrice&0xff; 
		}
		break; 
	default:
		break;
	}
	pParam->JlState = 0x01;

	int nClearOilData = 0;
	int nOilStartState = 0;
	int nResult_GunState = 0;

	//printf("start ready add oil,0x12\n");
	
	if(jlSendAndRecv(0x12,pReport,pParam))
	{
		nClearOilData = pParam->uchResult_ClearOilData;
		if(nClearOilData == 0)
		{
			if(jlSendAndRecv(0x13,pReport,pParam))
			{
				int nParamState = 0;
				nOilStartState = pParam->uchState_OilStart;
				nParamState = pParam->ushStateParam_OilStart; 

				if(nOilStartState != 0)
				{
                    //printf("----add oil start failure----,state = %d,param = %d\n",nOilStartState,nParamState);
					if(nParamState == 14)
					{
						int i = 0; 
						for( i = 0 ; i < 4; i++)
						{
							pReport->uchSendGunStateData = GetGunState();
							if(jlSendAndRecv(0x10,pReport,pParam))
							{
								nResult_GunState = pParam->uchState_GunStateQ;
								if(nResult_GunState == 0x02) 
								{
                                     printf("****clear success,add oil start success****,time = %d\n",i);
									 return 0;
								}
								else if(nResult_GunState == 0x01)
								{
                                    printf("****add oil start failure****,time = %d\n",i);
									unsigned char stop_no=0;
									unsigned long long money_sum=0, volume_sum=0;
									unsigned int money=0, volume=0, price=0;
									jlOilFinish(pParam->Nozzle, &money_sum, &volume_sum, &money, &volume, &price, &stop_no);
									pParam->JlState = 0;
									return 1;
								}
							}
						}
					}
					else
					{
						return nOilStartState;
					}

				}
                else
				{
	                //printf("----clear success,add oil start----,state = %d,param = %d\n",nOilStartState,nParamState);
				}		
				return nOilStartState;
			}
			else
			{
				printf("request jl add oil failure!,error = %d\n",pParam->ushStateParam_OilStart);
				return ERROR;
			}
		}
		else
		{
            printf("----clear failure,add oil start failure\n");
		}
	}	
	
	return ERROR;
}

/*
int jlOilStart(int nozzle, unsigned int preset, unsigned int preset_price, unsigned int preset_mode)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(JL_NOZZLE_1==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(JL_NOZZLE_2==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return 0x01;
	}
	if(pParam->JlState != 0)
	{
		return ERROR;
	}

	//printf("preset = %d,preset_price = %d\n",preset,preset_price);
    switch(preset_mode)
	{
	case 0: //������� ---- Ԥ�ý��
		{
			pReport->uchSendData[0] = 0;
	        unsigned long long llTempCount = hex2Bcd(preset);
			unsigned long long llTempPrice = hex2Bcd(preset_price);
			pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			pReport->uchSendData[5] = (llTempPrice>>16)&0xff;
	        pReport->uchSendData[6] = (llTempPrice>>8)&0xff;
	        pReport->uchSendData[7] = llTempPrice&0xff; 
		}
		break;
	case 1: //Ԥ������ ----- Ԥ������
		{
			pReport->uchSendData[0] = preset_mode;
	        unsigned long long llTempCount = hex2Bcd(preset);
	        pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			unsigned long long llTemp = hex2Bcd(preset_price);
			pReport->uchSendData[5] = (llTemp>>16) & 0xff;
	        pReport->uchSendData[6] = (llTemp>>8) & 0xff;
	        pReport->uchSendData[7] = llTemp & 0xff;   
		}
		break;
	case 2: //Ԥ�ý�� ---- ������� 
		{
			pReport->uchSendData[0] = 0;
	        unsigned long long llTempCount = hex2Bcd(preset);
			unsigned long long llTempPrice = hex2Bcd(preset_price);
			pReport->uchSendData[1] = (llTempCount>>24)&0xff;
			pReport->uchSendData[2] = (llTempCount>>16)&0xff;
	        pReport->uchSendData[3] = (llTempCount>>8)&0xff;
            pReport->uchSendData[4] = llTempCount&0xff;
			pReport->uchSendData[5] = (llTempPrice>>16)&0xff;
	        pReport->uchSendData[6] = (llTempPrice>>8)&0xff;
	        pReport->uchSendData[7] = llTempPrice&0xff; 
		}
		break; 
	default:
		break;
	}
	pParam->JlState = 0x01;

	int nClearOilData = 0;
	int nOilStartState = 0;

	//printf("start ready add oil,0x12\n");
	
	if(jlSendAndRecv(0x12,pReport,pParam))
	{
		nClearOilData = pParam->uchResult_ClearOilData;
		if(nClearOilData == 0)
		{
			if(jlSendAndRecv(0x13,pReport,pParam))
			{
				int nParamState = 0;
				nOilStartState = pParam->uchState_OilStart;
				nParamState = pParam->ushStateParam_OilStart; 

                if(nOilStartState != 0)
		        {
	                printf("----add oil start failure----,state = %d,param = %d\n",nOilStartState,nParamState);
					unsigned char stop_no=0;
					unsigned long long money_sum=0, volume_sum=0;
					unsigned int money=0, volume=0, price=0;
					jlOilFinish(pParam->Nozzle, &money_sum, &volume_sum, &money, &volume, &price, &stop_no);
					pParam->JlState = 0;
                 	//printf("----add oil start failure----,state = %d,param = %d\n",nOilStartState,nParamState);
				} 
                else
				{
	                printf("----clear success,add oil start----,state = %d,param = %d\n",nOilStartState,nParamState);
				}
			
				return nOilStartState;
			}
			else
			{
				printf("request jl add oil failure!,error = %d\n",pParam->ushStateParam_OilStart);
				return ERROR;
			}
		}
		else
		{
            printf("----clear failure,add oil start failure\n");
		}
	}	
	
	return ERROR;
}*/

/********************************************************************
*Name				:jlOilRead
*Description		:�����������ݶ�ȡ
*Input				:nozzle			0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:money		���ν��
*						:volume		��������
*						:price			���ε���
*						:stop_no		����ͣ������	0x00=����
																					0x01=ȱһ·���壬ͣ��
																					0x02=�ﵽԤ������ͣ��
																					0x03=˰�ؽ�ֹ��ͣ��
																					0x04=�����峬ʱ��ͣ��
																					0x05=�������磬ͣ��
*Return			:0=�ɹ�������=ʧ�ܣ�
*History			:2014-03-03,modified by syj
*/
int jlOilRead(int nozzle, unsigned int *money, unsigned int *volume, unsigned int *price, unsigned char *stop_no)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState == 0)
    {
		return ERROR;
	}
	pReport->uchSendData[0] = GetGunState(); 

    if(jlSendAndRecv(0x14,pReport,pParam))
	{
		//pthread_mutex_lock(&g_dataMutexJL);
		//printf("equivalent --%02x%02x%02x\n",g_pCurJLParam->uchData_Query[0],g_pCurJLParam->uchData_Query[1],g_pCurJLParam->uchData_Query[2]);  
        unsigned int nEquivalentHex = (int)pParam->uchData_Query[0] << 16 | (int)pParam->uchData_Query[1] << 8 | pParam->uchData_Query[2];
	    int nEquivalentDec = hexbcd2int(nEquivalentHex);
        *money = hexbcd2int(pParam->nOilMoney_ReadOiling);
		*volume = hexbcd2int(pParam->nOilVolume_ReadOiling);
		*price = hexbcd2int(pParam->nOilPrice_ReadOiling);
		//int nSpeed = g_pCurJLParam->ushOilSpeed_ReadOiling;
        *stop_no =  pParam->uchState_ReadOiling;
		//pthread_mutex_unlock(&g_dataMutexJL);
		//printf("0x14,return -- %d,money = %d,volume=%d\n",g_pCurJLParam->uchState_ReadOiling,g_pCurJLParam->nOilMoney_ReadOiling,g_pCurJLParam->nOilVolume_ReadOiling);
		return 0;
	}
	else
	{
        printf("read oiling data failure,function = 0x14\n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlOilFinish
*Description		:�������ͽ���
*Input				:nozzle					0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*Output			:money_sum		���۽��
*						:volume_sum		��������
*						:pmoney				���μ��ͽ��
*						:pvolume				���μ�������
*						:pprice					���μ��͵���
*						:stop_no				���μ���ͣ������
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-03,modified by syj
*/
int jlOilFinish(int nozzle, unsigned long long *money_sum, unsigned long long *volume_sum, unsigned int *pmoney, unsigned int *pvolume, unsigned int *pprice, unsigned char *stop_no)
{
	//pthread_mutex_lock(&g_dataMutexJL);
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}


	pParam->JlState = 0x02; //���ͽ���������
	int ischg = 0;
	int nResult_AddOilEnd = 0;
	int nResult_GunState = 0;

	//pthread_mutex_unlock(&g_dataMutexJL);


	//printf("start jlOilFinish,ready send 0x15\n");

	if(jlSendAndRecv(0x15,pReport,pParam))
	{
		//pthread_mutex_lock(&g_dataMutexJL);
		nResult_AddOilEnd = pParam->uchResult_AddOilEnd;
		//pthread_mutex_unlock(&g_dataMutexJL);
		if(nResult_AddOilEnd == 0x01 || nResult_AddOilEnd == 0x00) //���ͽ�����
		{
			FOREVER
			{
				//pReport->uchSendData[0] = GetGunState();
				pReport->uchSendGunStateData = GetGunState();
				if(jlSendAndRecv(0x10,pReport,pParam))
				{
	                //pthread_mutex_lock(&g_dataMutexJL);
					nResult_GunState = pParam->uchState_GunStateQ;
					//pthread_mutex_unlock(&g_dataMutexJL);
					if(nResult_GunState == 0x03) //���ͽ��������
					{
                        //pthread_mutex_lock(&g_dataMutexJL);
						//*pmoney = hexbcd2int(pParam->nOilMoney_GunStateQ);
						//*pvolume = hexbcd2int(pParam->nOilVolume_GunStateQ);
						//*pprice = hexbcd2int(pParam->nOilPrice_GunStateQ);
						//*money_sum = hexbcd2longlong(pParam->llOilMoneySum_GunStateQ);
						//*volume_sum = hexbcd2longlong(pParam->llOilVolumeSum_GunStateQ);
                		*pmoney = pParam->OilMoney;
						*pvolume = pParam->OilVolume;
						*pprice = pParam->OilPrice;
						*money_sum = pParam->MoneySum;
						*volume_sum = pParam->VolumeSum;
		                //pthread_mutex_unlock(&g_dataMutexJL);

						if(jlSendAndRecv(0x16,pReport,pParam))
						{
							//printf("0x16,return = %d\n",pParam->uchResult_TradeOK);
							//pthread_mutex_lock(&g_dataMutexJL);
							if(pParam->uchResult_TradeOK == 0) //��������ȷ�ϳɹ����������ڿ���״̬
							{	
								pParam->JlState = 0; //����״̬
								//pthread_mutex_unlock(&g_dataMutexJL);
								//printf("add finish is success!,money = %d\n",*pmoney);
								return 0;
							}
							//pthread_mutex_unlock(&g_dataMutexJL);
							return 0;
						}
						else
						{
							printf("trade ok failure,function = 0x16\n");
							return ERROR;
						}
					}
				}
				else
				{
					printf("request state query failure,function = 0x10\n");
					return ERROR;
				}
			}
		}
		else
		{
         	printf("-----start jlOilFinish,send 0x15,return = %d\n",nResult_AddOilEnd);
		}
	}
	else
	{
		printf("request oil end failure,function = 0x15\n");
		return ERROR;
	}
}


/********************************************************************
*Name				:jlOilCZ
*Description		:��ȡ��������汾
*Input				:nozzle		0=1��ǹ(A1ǹ)��1=2��ǹ(B1ǹ)
*						:handle		0=��������1=��������
*Output			:buffer		�������
*Return				:0=�ɹ�������=ʧ��
*History			:2013-08-05,modified by syj
*/
int jlOilCZ(int nozzle, int handle)
{
	JlParamStruct* pParam = NULL;
	JLReportStruct* pReport = NULL;
	if(0==nozzle)	//�ж�ǹѡ
	{
		pParam = &JlParamA1;
		pReport = &jlOptClass.jlStruReportA;
	}
	else if(1==nozzle)
	{
		pParam = &JlParamB1;
		pReport = &jlOptClass.jlStruReportB;
	}
	else
	{
		return ERROR;
	}
	if(pParam->JlState == 0)
	{
		return ERROR;
	}

	pReport->uchSendData[0] = handle&0xff; //0:�������� 1:��������

	if(jlSendAndRecv(0x17,pReport,pParam))
	{
		int nResult_Rounding = 0;
		pthread_mutex_lock(&g_dataMutexJL);
		nResult_Rounding = pParam->uchResult_Rounding;
		pthread_mutex_unlock(&g_dataMutexJL);
		return nResult_Rounding;
	}
	else
	{
        printf("request add oil rounding failure,function = 0x17\n");
	}
}


bool InitJL()
{
	memset(&jlOptClass.jlStruReportA,0,sizeof(JLReportStruct));
	memset(&jlOptClass.jlStruReportB,0,sizeof(JLReportStruct));

	jlOptClass.jlStruReportA.uchGunNo = 0x00;
	jlOptClass.jlStruReportB.uchGunNo = 0x01;

	jlOptClass.sendJLInit = sendJLInit;
	if(jlOptClass.sendJLInit == NULL)
	{
		printf("initJL_sendJLInit failure!\r\n");
		return false;
	}

	JlParamA1.JlState = 0;    //����
	JlParamB1.JlState = 0;    //����
	JlParamA1.Nozzle = 0x00;
    JlParamB1.Nozzle = 0x01;

	jlOptClass.sendGetRand = sendGetRand;
	if(jlOptClass.sendGetRand == NULL)
	{
		printf("initJL_sendGetRand failure!\r\n");
		return false;
	}

	jlOptClass.sendJLParamSet = sendJLParamSet;
	if(jlOptClass.sendJLParamSet == NULL)
	{
		printf("initJL_sendJLParamSet failure!\r\n");
		return false;
	}

	jlOptClass.sendJLParamQuery = sendJLParamQuery;
	if(jlOptClass.sendJLParamQuery == NULL)
	{
		printf("initJL_sendJLParamQuery failure!\r\n");
		return false;
	}

	jlOptClass.sendJLOilRounding = sendJLOilRounding;
	if(jlOptClass.sendJLInit == NULL)
	{
		printf("initJL_sendJLOilRounding failure!\r\n");
		return false;
	}

	jlOptClass.sendJLTradeOK = sendJLTradeOK;
	if(jlOptClass.sendJLTradeOK == NULL)
	{
		printf("initJL_sendJLTradeOK failure!\r\n");
		return false;
	}
	jlOptClass.sendJLAddOilEnd = sendJLAddOilEnd;
	if(jlOptClass.sendJLAddOilEnd == NULL)
	{
		printf("initJL_sendJLAddOilEnd failure!\r\n");
		return false;
	}

	jlOptClass.sendGetOilData = sendGetOilData;
	if(jlOptClass.sendGetOilData == NULL)
	{
		printf("initJL_sendGetOilData failure!\r\n");
		return false;
	}

	jlOptClass.sendJLAddOilStart = sendJLAddOilStart;
	if(jlOptClass.sendJLAddOilStart == NULL)
	{
		printf("initJL_sendJLAddOilStart failure!\r\n");
		return false;
	}

	jlOptClass.sendClearOilData = sendClearOilData;
	if(jlOptClass.sendClearOilData == NULL)
	{
		printf("initJL_sendClearOilData failure!\r\n");
		return false;
	}

	jlOptClass.sendShowOilData = sendShowOilData;
	if(jlOptClass.sendShowOilData == NULL)
	{
		printf("initJL_sendShowOilData failure!\r\n");
		return false;
	}

	jlOptClass.sendJLStateQuery = sendJLStateQuery;
	if(jlOptClass.sendJLStateQuery == NULL)
	{
		printf("initJL_sendJLStateQuery failure!\r\n");
		return false;
	}

	char* dev[2] = {"/dev/ttyS3",""};
	g_nFd = open_port(dev[0],115200);
	if(g_nFd < 0)
	{
		printf("open jl com is failure1\n");
		return false;
	}

	return true;
}

static void GetSendJLData(unsigned char uchCmd,JLReportStruct* pReportStruct,unsigned char* puchSendData,int* pnSendLen)
{
    int i = 0;
	int crc = 0;
	puchSendData[i++] = 0xff;
	puchSendData[i++] = 0xff; //֡ͷ
	
	unsigned char uchRsctl;
	g_uchRsctl = (++g_uchRsctl)%10;
	if(0x08 == g_uchRsctl)
	{
		g_uchRsctl = 0x00;
	}
	uchRsctl = 0x80|g_uchRsctl;
	puchSendData[i++] = uchRsctl;

	puchSendData[i++] = 0x0;

    unsigned char uchTempData[256] = {0};
	int nTempLen = 0;
	switch(uchCmd)
	{
	case 0x10:
	    jlOptClass.sendJLStateQuery(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x11:
		jlOptClass.sendShowOilData(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x12:
		jlOptClass.sendClearOilData(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x13:
		jlOptClass.sendJLAddOilStart(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x14:
		jlOptClass.sendGetOilData(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x15:
		jlOptClass.sendJLAddOilEnd(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x16:
		jlOptClass.sendJLTradeOK(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x17:
		jlOptClass.sendJLOilRounding(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x18:
		jlOptClass.sendJLParamQuery(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x19:
		jlOptClass.sendJLParamSet(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x1A:
		jlOptClass.sendGetRand(uchTempData,&nTempLen,pReportStruct);
		break;
	case 0x1B:
		jlOptClass.sendJLInit(uchTempData,&nTempLen,pReportStruct);
		break;
	default:
		break;
	}

    int j;
	for(j = 0; j < nTempLen; j++)
	{
		if(uchTempData[j] == 0xff)
		{
			puchSendData[i++] = 0xfe;
			puchSendData[i++] = 0x01;
		}
		else if(uchTempData[j] == 0xfe)
		{
			puchSendData[i++] = 0xfe;
			puchSendData[i++] = 0x01;
		}
		else
		{
			puchSendData[i++] = uchTempData[j];
		}
	}
    puchSendData[3] = i-4;

	crc = crc16Get(&puchSendData[2],i-2); //CRC
	unsigned char uchCrcTemp[2];
	uchCrcTemp[0] = (crc >> 8) & 0xff;
	uchCrcTemp[1] = crc & 0xff;
    
	if(uchCrcTemp[0] == 0xff)
	{
		puchSendData[i++] = 0xfe;
		puchSendData[i++] = 0x01;
	}
	else if(uchCrcTemp[0] == 0xfe)
	{
		puchSendData[i++] = 0xfe;
		puchSendData[i++] = 0x00;
	}
	else
	{
		puchSendData[i++] = uchCrcTemp[0];
	}

	if(uchCrcTemp[1] == 0xff)
	{
		puchSendData[i++] = 0xfe;
		puchSendData[i++] = 0x01;
	}
	else if(uchCrcTemp[1] == 0xfe)
	{
		puchSendData[i++] = 0xfe;
		puchSendData[i++] = 0x00;
	}	
	else
	{
		puchSendData[i++] = uchCrcTemp[1];
	}	

	puchSendData[i++] = 0xff; //֡β
	*pnSendLen = i;
}

static bool ParseRecvJLData(unsigned char* puchReadData,int nReadLen,int* pnDeleteLen,JlParamStruct* pParamStruct)
{
    unsigned char uchReturnData[256] = {0};
	int nDataLen = 0;
	int i = 4;

	if(JudgeFFAndCrcProtocol(puchReadData,nReadLen,uchReturnData,&nDataLen,pnDeleteLen))
	{
		//printf("parse finish data , cmd = %d\n",uchReturnData[2]);
		//PrintH(nDataLen,uchReturnData);
		if(pParamStruct->Nozzle != uchReturnData[3])
		{
			printf("Gun No error,jl return gun no = %d,Nozzle = %d\n",uchReturnData[3],pParamStruct->Nozzle);
            PrintH(nReadLen,puchReadData);
			PrintH(nDataLen,uchReturnData);
			*pnDeleteLen = -4;
			return false;
		}
		switch(uchReturnData[2]) //9720������������ֹ���
		{
		case 0x10: //��ǹ״̬��ѯ
            if(pParamStruct != NULL)
			{
                pParamStruct->uchState_GunStateQ = uchReturnData[i++];
				pParamStruct->ushPara_GunStateQ = ((unsigned short)uchReturnData[i++] << 8) | uchReturnData[i++];
				unsigned int nMoney,nVolume,nPrice;
				unsigned long long llMoneySum,llVolumeSum;
				nMoney = ((unsigned int)uchReturnData[i++] << 24) | ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
                nVolume = ((unsigned int)uchReturnData[i++] << 24) | ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
                nPrice =  ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
				llVolumeSum = ((unsigned long long)uchReturnData[i++]<<32)|((unsigned long long)uchReturnData[i++]<<24)|((unsigned long long)uchReturnData[i++]<<16)|((unsigned long long)uchReturnData[i++]<<8)|uchReturnData[i++];
				llMoneySum =  ((unsigned long long)uchReturnData[i++]<<32)|((unsigned long long)uchReturnData[i++]<<24)|((unsigned long long)uchReturnData[i++]<<16)|((unsigned long long)uchReturnData[i++]<<8)|uchReturnData[i++];
            	pParamStruct->OilMoney = hexbcd2int(nMoney);
				pParamStruct->OilVolume = hexbcd2int(nVolume);
				pParamStruct->OilPrice = hexbcd2int(nPrice);
				pParamStruct->Price = hexbcd2int(nPrice);  //fj:20171117
				//printf("0x10 , Price = %d\n",pParamStruct->Price);
				pParamStruct->MoneySum = hexbcd2longlong(llMoneySum);
				pParamStruct->VolumeSum = hexbcd2longlong(llVolumeSum);
			}
			break;
		case 0x11: //������ʾ��ǰ��������
			if(pParamStruct != NULL)
			{
				pParamStruct->uchResult_ViewOilData = uchReturnData[i];
			}
			break;
		case 0x12: //�����ǰ��������
			if(pParamStruct != NULL)
			{
                //printf("0x12 = aaaaa,i = %d\n",i);
                pParamStruct->uchResult_ClearOilData = uchReturnData[i];
				//printf("0x12 = bbbbb\n");
                //PrintH(nDataLen,uchReturnData);
			}
			break;
		case 0x13: //��������
			if(pParamStruct != NULL)
			{
				pParamStruct->uchState_OilStart = uchReturnData[i++];
				pParamStruct->ushStateParam_OilStart = ((unsigned short)uchReturnData[i++] << 8) | uchReturnData[i++];
			}
			break;
		case 0x14: //��ȡ����������
			if(pParamStruct != NULL)
			{
				pParamStruct->uchState_ReadOiling = uchReturnData[i++];
             	pParamStruct->nOilMoney_ReadOiling = ((unsigned int)uchReturnData[i++] << 24) | ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
                pParamStruct->nOilVolume_ReadOiling = ((unsigned int)uchReturnData[i++] << 24) | ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
                pParamStruct->nOilPrice_ReadOiling =  ((unsigned int)uchReturnData[i++] << 16) | ((unsigned int)uchReturnData[i++]<<8) | uchReturnData[i++];
				pParamStruct->ushOilSpeed_ReadOiling = ((unsigned short)uchReturnData[i++] << 8) | uchReturnData[i++];
			}
            break;
		case 0x15://���ͽ���
			if(pParamStruct != NULL)
			{
		        pParamStruct->uchResult_AddOilEnd = uchReturnData[i];		
			}
			break;
		case 0x16://��������ȷ��
			if(pParamStruct != NULL)
			{
				pParamStruct->uchResult_TradeOK = uchReturnData[i];
			}
			break;
		case 0x17://�����д���
			if(pParamStruct != NULL)
			{
				pParamStruct->uchResult_Rounding = uchReturnData[i];
			}
			break;
		case 0x18: //����������ѯ
			if(pParamStruct != NULL)
			{
				int nJLParamLen = uchReturnData[1] - 4;
				pParamStruct->uchParamType_Query = uchReturnData[i++];
				pParamStruct->uchResult_Query = uchReturnData[i++];
				//printf("nJLParamLen = %d\n",nJLParamLen);

				if(nJLParamLen > 32)
				{
					printf("jl parameter query parma_len error!\n");
				}
				else
				{
					int j;
                    for(j = 0; j < nJLParamLen; j++)
					{
						pParamStruct->uchData_Query[j] = uchReturnData[i+j];
					}
				}
			}
			break;
		case 0x19: //������������
			if(pParamStruct != NULL)
			{
				pParamStruct->uchParamType_Set = uchReturnData[i++];
				pParamStruct->uchResult_Set = uchReturnData[i++];
	            //printf("parse finish data , cmd = %d,uchResult_Set = %d\n",uchReturnData[2],pParamStruct->uchResult_Set);
		        //PrintH(nDataLen,uchReturnData);

                //printf("recv data :\n");
				//PrintH(nReadLen,puchReadData);
			}
			break;
		case 0x1a: //ȡ�����
			if(pParamStruct != NULL)
			{
				pParamStruct->uchRand[0] = uchReturnData[i++];
				pParamStruct->uchRand[1] = uchReturnData[i++];
				pParamStruct->uchRand[2] = uchReturnData[i++];
				pParamStruct->uchRand[3] = uchReturnData[i++];
				pParamStruct->uchRand[4] = uchReturnData[i++];
				pParamStruct->uchRand[5] = uchReturnData[i++];
				//printf("0x1a,rand\n");
				//PrintH(6,g_pCurJLParam->uchRand);
			}
			break;
		case 0x1b: //������ʼ��
			if(pParamStruct != NULL)
			{
				pParamStruct->uchResult_JLInit = uchReturnData[i];
			}
			break;
		default:
			break;
		}

		return true;
	}
	else
	{
        if(*pnDeleteLen == -2 || *pnDeleteLen == -3)
		{
            return false;
		}
	}

	*pnDeleteLen = nReadLen;
	return false;
}

void JL_Process()
{
	prctl(PR_SET_NAME,(unsigned long)"JL_Process");
	JlParamStruct* pJLParamA = NULL;
	JLReportStruct* pJLReportA = NULL;
    pJLParamA = &JlParamA1;
	pJLReportA = &jlOptClass.jlStruReportA;

	JlParamStruct* pJLParamB = NULL;
	JLReportStruct* pJLReportB = NULL;
    pJLParamB = &JlParamB1;
	pJLReportB = &jlOptClass.jlStruReportB;
	
	unsigned int money =0,volume = 0,price=0;
	unsigned char stop_no = 0;
	unsigned long long money_sum = 0, volume_sum = 0;

    FOREVER
	{
		if(pJLParamA->JlState == 0)
		{
			//printf("A Gun 0x10 query:\n");
			//pJLReportA->uchSendData[0] = GetGunState();
			pJLReportA->uchSendGunStateData = GetGunState(); //fj:20171110,update
			jlSendAndRecv(0x10,pJLReportA,pJLParamA);
			//pthread_mutex_lock(&g_dataMutexJL);
			int nState = pJLParamA->uchState_GunStateQ;
			//pthread_mutex_unlock(&g_dataMutexJL);
			if(nState != 0)
			{
				int iState = jlOilFinish(0,&money_sum,&volume_sum,&money,&volume,&price,&stop_no);
				if(0 == iState)
				{
                    //pthread_mutex_lock(&g_dataMutexJL);
					pJLParamA->OilVolume = volume;
					pJLParamA->OilMoney = money;
					pJLParamA->OilPrice = price;
					pJLParamA->VolumeSum = pJLParamA->VolumeSum + pJLParamA->OilVolume;
					pJLParamA->MoneySum = pJLParamA->MoneySum + pJLParamA->OilMoney;
					//pJLParamA->VolumeSum = volume_sum;
					//pJLParamA->MoneySum = money_sum;
					pJLParamA->JlState = JL_STATE_IDLE;
					//pthread_mutex_unlock(&g_dataMutexJL);
				}
			}
		}

		if(pJLParamB->JlState == 0)
		{
	        //printf("B Gun 0x10 query:\n");
			//pJLReportB->uchSendData[0] = GetGunState();
			pJLReportB->uchSendGunStateData = GetGunState();
			jlSendAndRecv(0x10,pJLReportB,pJLParamB);
         	//pthread_mutex_lock(&g_dataMutexJL);
			int nState = pJLParamB->uchState_GunStateQ;
			//pthread_mutex_unlock(&g_dataMutexJL);

			//printf("scan 0x10 , pJLParamB->JlState = %d,nState = %d",pJLParamB->JlState,nState);

			if(nState != 0)
			{
				//printf("JL_Process,fjOilFinish start \n");
				int iState = jlOilFinish(1,&money_sum,&volume_sum,&money,&volume,&price,&stop_no);
				if(0 == iState)
				{
                    //pthread_mutex_lock(&g_dataMutexJL);
					pJLParamB->OilVolume = volume;
					pJLParamB->OilMoney = money;
					pJLParamB->OilPrice = price;
					pJLParamB->VolumeSum = pJLParamB->VolumeSum + pJLParamB->OilVolume;
					pJLParamB->MoneySum = pJLParamB->MoneySum + pJLParamB->OilMoney;
					pJLParamB->JlState = JL_STATE_IDLE;
					//pthread_mutex_unlock(&g_dataMutexJL);
				}
			}
		}
		usleep(500000);
	}
}
