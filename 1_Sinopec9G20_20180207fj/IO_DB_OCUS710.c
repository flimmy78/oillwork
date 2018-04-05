#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <pthread.h>
#include "../inc/main.h"

pthread_rwlock_t  rwlock_ReadDataBase = PTHREAD_RWLOCK_INITIALIZER; //可读数据库锁
pthread_rwlock_t  rwlock_WriteDataBase = PTHREAD_RWLOCK_INITIALIZER;//可写数据库锁
pthread_rwlock_t  rwlock_WriteState = PTHREAD_RWLOCK_INITIALIZER;

FILE* g_pFile = NULL;
int g_nfdAlarm = 0;

typedef struct
{
	int m_nIndex;            //报警索引
	int m_nDevAddr;          //设备地址，哪个设备报警   
	int m_nOffsetAddr;       //偏移地址，哪个项报警
	char m_chAlarmTime[32];    //发生报警的时间
	int m_nAlarmState;       //报警的状态
	char m_chAlarmRecord[64];
}structAlarmSOE; //报警记录结构体

Queue* g_pQueue = NULL;


/********************数据的读、写和更新******************************************************************************/
//x,y为DATABASE的行和列,p为输入数据头指针,len为写入长度,mutex_en为互斥使能1为开启
unsigned char DataBaseWrite(unsigned int x,unsigned int y,	unsigned char *p,unsigned int len,unsigned char mutex_en)
{
	int i=0;
	unsigned char *temp;
	temp=&DataBase_IO_Unit_W[x][y];

	if((x*DATABASE_LEN+y+len) > (DATABASE_LEN*DATABASE_NUM))
	{
		return 1;
	}

	pthread_rwlock_wrlock(&rwlock_WriteDataBase);//加写锁(写区)
	//printf("database writing...\n");
	for(i=0;i<len;i++)
	{
		*(temp++)=p[i];
	}
	pthread_rwlock_unlock(&rwlock_WriteDataBase);//解锁

	return 0;
}

//x,y为DATABASE的行和列,p为输出数据头指针,len为输出长度,mutex_en为互斥使能1为开启
unsigned char DataBaseRead(unsigned int x,unsigned int y,unsigned char *p,unsigned int len,unsigned char mutex_en)
{
	int i=0;
	unsigned char *temp;
	temp=&DataBase_IO_Unit_R[x][y];

	if((x*DATABASE_LEN+y+len) > (DATABASE_LEN*DATABASE_NUM))
	{
		return 1;
	}

	pthread_rwlock_rdlock(&rwlock_ReadDataBase);//加读锁(读区)
	for(i=0;i<len;i++)
	{
		p[i]=*(temp++);
	}
	pthread_rwlock_unlock(&rwlock_ReadDataBase);//解锁
	
	return 0;
}

unsigned char DataBaseUpdata(void) //数据更新
{
	int i=0,j=0;
	unsigned char database_io_unit_tempbuf[DATABASE_NUM][DATABASE_LEN];
	memset(&database_io_unit_tempbuf,0,sizeof(database_io_unit_tempbuf));

	pthread_rwlock_rdlock(&rwlock_ReadDataBase);//加读锁(写区)
	for(i=0;i<DATABASE_NUM;i++)
	{
		for(j=0;j<DATABASE_LEN;j++)
		{
			database_io_unit_tempbuf[i][j]=DataBase_IO_Unit_W[i][j];
		}
	}
	pthread_rwlock_unlock(&rwlock_ReadDataBase);//解锁

	for(i=0;i<DATABASE_NUM;i++)
	{
		for(j=0;j<DATABASE_LEN;j++)
		{
			if(database_io_unit_tempbuf[i][j]!=DataBase_IO_Unit_R[i][j])
			{
				pthread_rwlock_wrlock(&rwlock_WriteDataBase);//加写锁（读区）
				DataBase_IO_Unit_R[i][j]=database_io_unit_tempbuf[i][j]; 
				DataBase_State[i][j]=1;
				pthread_rwlock_unlock(&rwlock_WriteDataBase);//解锁
				//printf("the changed data:(x:%d,y:%d) %02x\n ",i,j,DataBase_IO_Unit_R[i][j]);
				if(j >= 72 && struSysConFile.struBasic.IO_Type == OCUS710_DEV)
				{
					//printf("---j = %d \n",j);
					SaveAlarmData(i,j);
					if(j >= 102)
					{
						g_FlagDOchanged = 1;
					}
				}
			}
		}
	}

	if(g_FlagDOchanged == 1 && struSysConFile.struBasic.IO_Type == OCUS710_DEV)
	{
		sem_post(&sem_idSend);
		g_FlagDOchanged = 0;
	}

    return 0;
}

/*********************读DI DO数据，为了处理变为103数据********************************************************************/
void ReadDIO(unsigned short* p_sDIOstate)
{
	unsigned int i=0;
	unsigned int j=0;
	UINT_DC83 dc83;
	unsigned char l_sDIOstate[16];
	unsigned int l_chYpos=(unsigned char*)(&dc83.DI[0])-(unsigned char*)&dc83;
	for(i=0;i<g_sIODeviceNum;i++)
	{
		DataBaseRead(i,l_chYpos,l_sDIOstate,16,1);
		for(j=0;j<16;j++)
		{
			p_sDIOstate[i] = p_sDIOstate[i]>>1;
			p_sDIOstate[i] |= (unsigned short)l_sDIOstate[j]<<15;
		}
		//	  printf("p_sDIOstate[%d]=%04x\n",i,p_sDIOstate[i]);
	 }
}




/*********************报警文件的读取和存储************************************************************************/
void SaveCSVFile() //把队列里的报警记录存在CSV文件里
{
	char chAlarmRecord[6400000];
	if(g_pQueue != NULL && g_pFile != NULL)
	{
		PNode pnode = NULL;
		structAlarmSOE* pTempAS = NULL;
		pnode = g_pQueue->front;
		if(pnode)
		{
			int i = g_pQueue->size;
			//printf("size_i = %d\n",i);
			int nOffset = 0;
			while(i--)
			{
				pTempAS = pnode->data;	
				if(pTempAS)
				{
					memcpy(&chAlarmRecord[nOffset],pTempAS->m_chAlarmRecord,strlen(pTempAS->m_chAlarmRecord));
					nOffset += strlen(pTempAS->m_chAlarmRecord);
					pnode = pnode->next;
					if(pnode == NULL)
					{
						break;
					}
				}
			}
		}
		rewind(g_pFile);
		fwrite(chAlarmRecord,strlen(chAlarmRecord),1,g_pFile);
		fflush(g_pFile); 
		fdatasync(g_nfdAlarm);
	}	
}

void SaveAlarmData(int nDevAddr,int nOffsetAddr) //把当前报警记录存在队列里
{
	structAlarmSOE* pAlarmSOE;
    pAlarmSOE = (structAlarmSOE*)malloc(sizeof(structAlarmSOE));

	if(g_pQueue == NULL)
	{
	    g_pQueue = InitQueue();
		printf("aaaa\n");
	}

	if(g_pFile == NULL)
	{
		g_pFile = fopen("../config/Alarm.csv","r+");
	}

	char chAlarmNum[8];
    char chDevAddr[8];
	char chOffsetAddr[8];
	char chAlarmTime[32];
	char chAlarmState[8];
	int i;
	int nStrLen = 0;

	for(i = 0 ; i < 64 ; i++)
	{
		pAlarmSOE->m_chAlarmRecord[i] = 0;
	} 
	//memset(pAlarmSOE->m_chAlarmRecord,0,64);
	sprintf(chDevAddr,"%02d;",nDevAddr);
	for(i = 0; i < strlen(chDevAddr); i++)
	{
		pAlarmSOE->m_chAlarmRecord[i] = chDevAddr[i];
	}
	nStrLen = strlen(chDevAddr);
    //memcpy(pAlarmSOE->m_chAlarmRecord,chDevAddr,strlen(chDevAddr));
	sprintf(chOffsetAddr,"%03d;",nOffsetAddr);
	for(i = 0; i < strlen(chOffsetAddr); i++)
	{
        pAlarmSOE->m_chAlarmRecord[i+nStrLen] = chOffsetAddr[i];
	}
	nStrLen += strlen(chOffsetAddr);
    //memcpy(&pAlarmSOE->m_chAlarmRecord[3],chOffsetAddr,strlen(chOffsetAddr));
	
    time_t tmNow;
	struct tm* ptmNow;
	struct timespec tmSpec;
	time(&tmNow);
	ptmNow = localtime(&tmNow);
	clock_gettime(CLOCK_REALTIME,&tmSpec);
    sprintf(chAlarmTime,"%d-%02d-%02d%c%02d:%02d:%02d.%03d;",1900+ptmNow->tm_year,1+ptmNow->tm_mon,ptmNow->tm_mday,'\040',ptmNow->tm_hour,ptmNow->tm_min,ptmNow->tm_sec,tmSpec.tv_nsec / 1000000);
    for(i = 0; i < strlen(chAlarmTime); i++)
	{
		pAlarmSOE->m_chAlarmRecord[i+nStrLen] = chAlarmTime[i];
	}
	nStrLen += strlen(chAlarmTime);
   //memcpy(&pAlarmSOE->m_chAlarmRecord[7],chAlarmTime,strlen(chAlarmTime));

    unsigned char uchState;
    DataBaseRead(nDevAddr,nOffsetAddr,&uchState,1,1);
	sprintf(chAlarmState,"%d\r\n",uchState);
    for(i = 0 ; i < strlen(chAlarmState); i++)
	{
		pAlarmSOE->m_chAlarmRecord[i+nStrLen] = chAlarmState[i];
	}
    //memcpy(&pAlarmSOE->m_chAlarmRecord[7],chAlarmState,strlen(chAlarmState));

	if(g_pQueue->size < 1000)
	{
        EnQueue(g_pQueue,pAlarmSOE);
	}
	else
	{
		EnQueue(g_pQueue,pAlarmSOE);
		DeleteQueue(g_pQueue);	
	}
}

void GetAlarmCSV() //从Alarm.csv文件中获得历史报警记录存在队列里
{
	if(g_pQueue == NULL)
	{
		g_pQueue = InitQueue();
	}

	FILE* pFile;
	pFile = fopen("../config/Alarm.csv","a+");
	   
	char chLine[256];
	char* pchResult;
	int i;

	char chAlarmRecord[6400000];
	char* pchAlarmItem = NULL;

	if(pFile != NULL)
	{
		while(fgets(chAlarmRecord,64,pFile) != NULL)
		{
			pchAlarmItem = strtok(chAlarmRecord,",");
        	structAlarmSOE* pAlarmSOE;
            pAlarmSOE = (structAlarmSOE*)malloc(sizeof(structAlarmSOE));
            memset(pAlarmSOE->m_chAlarmRecord,0,64);
			strncpy(pAlarmSOE->m_chAlarmRecord,pchAlarmItem,strlen(pchAlarmItem));

			char chTemp[64];
			char chValue[64];

			if(pchAlarmItem == NULL)
			{
				break;
			}
			int nLen = strlen(pchAlarmItem);
			//printf("nLen = %d\n",nLen);

			int nItem = 0;
			for(i = 0; i < nLen; i++)
			{
				if(pchAlarmItem[i] == ';')
				{
					strncpy(chTemp,pchAlarmItem,i);            //提取;前的字符
                    nItem++;
					switch(nItem)
					{
					case 1:
						pAlarmSOE->m_nIndex = atoi(chTemp);
						break;
					case 2:
						pAlarmSOE->m_nDevAddr = atoi(chTemp);
						break;
					case 3:
						pAlarmSOE->m_nOffsetAddr = atoi(chTemp);
						break;
					case 4:
						strncpy(pAlarmSOE->m_chAlarmTime,chTemp,strlen(chTemp));
						break;
					case 5:
						pAlarmSOE->m_nAlarmState = atoi(chTemp);
						break;
					default:
						break;
					}

                    //删除提取了的字符，把;后的字符重新赋给pchAlarmItem.
                    strncpy(chValue,pchAlarmItem+i+1,nLen-i-1);   
					bzero(pchAlarmItem,strlen(pchAlarmItem));  
					strncpy(pchAlarmItem,chValue,nLen-i-1);    


					nLen = strlen(pchAlarmItem);
					i = 0;
					memset(chTemp,0,sizeof(chTemp));			
				}
			}

			pAlarmSOE->m_nAlarmState = atoi(pchAlarmItem);
			//printf("++%s",pchAlarmItem);
            EnQueue(g_pQueue,pAlarmSOE); //报警记录存入队列
			pchAlarmItem = strtok(NULL,",");
		}
	}
    close(pFile);

   if(g_pFile == NULL)
	{
		g_pFile = fopen("../config/Alarm.csv","r+");
        g_nfdAlarm = fileno(g_pFile);
	}
	else
	{
		printf("fopen error \n");
	}

	//pthread_rwlock_init(&rwlock_Alarm,NULL);
}



/**********************数据的初始化，未使用********************************************************************************/
void data_do(int num,UINT_DC83 *dc83)
{
	int i;
	for(i=0;i<3;i++)
	{
		dc83->U1_f[i]=20000.0+num*100.0+i;
		dc83->U2_f[i]=20000.0+num*100.0+i;
		dc83->I_f[i]=10000.0+num*100.0+i;
		dc83->U1[i]=2000+num*10+i;
		dc83->U2[i]=2000+num*10+i;
		dc83->I[i]=1000+num*10+i;
	}
	dc83->F1_f=50000.0+num*100.0+1;
	dc83->F2_f=50000.0+num*100.0+2;

	dc83->F1=5000+num*10+1;
	dc83->F2=5000+num*10+2;

	dc83->AnUab=0+num*10+1;
	dc83->AnUbc=12000+num*10+2;
	dc83->AnUca=24000+num*10+3;

	for(i=0;i<4;i++)
	{
		dc83->U1_L_flag[i]=i%2;
		dc83->U2_L_flag[i]=i%2;
		dc83->I_L_flag[i]=i%2;
		dc83->U1_H_flag[i]=i%2;
		dc83->U2_H_flag[i]=i%2;
		dc83->I_H_flag[i]=i%2;
	}
	for(i=0;i<6;i++)
	{
		dc83->DI[i]=i%2;
	}
	for(i=0;i<16;i++)
	{
		dc83->DO[i]=i%2;
	}
}

void DataBaseInit(void)
{
	int i=0;
	UINT_DC83 dc83;
	memset(&DataBase_IO_Unit_W,0,sizeof(DataBase_IO_Unit_W));
	memset(&DataBase_IO_Unit_R,0,sizeof(DataBase_IO_Unit_R));
	memset(DataBase_State,0,sizeof(DataBase_State));
	for(i=0;i<DATABASE_NUM;i++)
	{
		data_do( i,&dc83);
		DataBaseWrite( i,0,(unsigned char *)&dc83,sizeof(dc83),1);
	}
	DataBaseUpdata();
}
