#include "../inc/main.h"


void td_LogSave()
{
	//char msgBuffer[LOG_MSSAGE_MAX_SIZE+1]={0};		//消息缓存
	struct msg_st_log msg_st = {0,""};
	int msg_len=0;									//读取到的消息长度	
	//int *fd=NULL;									//当次操作的文件描述符
	char *path=NULL;								//当次操作的文件路径
	char rdbuffer[LOG_OILERR_SIZE+64]={0};			//数据读取缓存
	int rdbytes=0;									//数据读取长度
	//unsigned int *i_number=NULL;					//加油异常记录数据条数
	//unsigned int *i_number_unsave=NULL;			//加油异常记录数据条数
	unsigned int i_offset=0;						//数据在文件中的偏移位置
	int i=0;
    int nfd = -1;

	unsigned int i_number = 0;
	unsigned int i_number_unsave = 0;

	while ( 1 )
	{
		//等待接收日志数据
		//memset(msgBuffer, 0, LOG_MSSAGE_MAX_SIZE+1);	msg_len = 0;
		//msg_len=msgQReceive(msgIdUserLog, msgBuffer, LOG_MSSAGE_MAX_SIZE+1, NO_WAIT);
		
		msg_len=msgrcv(exg_nMsgID,&msg_st, LOG_MSSAGE_MAX_SIZE+1,0, 0);


		//处理加油异常记录数据
		//本文件以固定长度LOG_OILERR_SIZE按条存储异常记录，文件首字节位置保存所存储条目总数；
		//平时打开后不关闭，检测到掉电信号之后再行关闭保存
		
		if(msg_len>0 && (LOG_FILE_OILERRA==*(msg_st.msgBuffer) || LOG_FILE_OILERRB==*msg_st.msgBuffer))
		{
			//初始化操作数据
			if(LOG_FILE_OILERRA==*msg_st.msgBuffer)
			{
				//fd=&fdOilErrLogA;	
				path=LOG_FILE_PATH_OILERRA;	
				//i_number=&numberOilErrLogA;	
				//i_number_unsave=&numberOilErrUnsaveA;
				i_number = 0;
				i_number_unsave = 0;
			}
			else
			{
				//fd=&fdOilErrLogB;
				path=LOG_FILE_PATH_OILERRB;	
				//i_number=&numberOilErrLogB;	
				//i_number_unsave=&numberOilErrUnsaveB;
				i_number = -1;
				i_number_unsave = 0;
			}

			//打开日志文件,fj
			//if(ERROR==*fd)	*fd=fileOpen(path, O_CREAT|O_RDWR, S_IREAD | S_IWRITE);
			if(nfd == -1)
				nfd = fileOpen(path,O_CREAT|O_RDWR,S_IREAD | S_IWRITE);

			//尚未读取记录条数时读取记录条数，格式为"记录条数=XXXXXXXXXX"
			//if(0==(*i_number) && LOG_OILERR_SIZE==(rdbytes=fileRead(*fd, 0, rdbuffer, LOG_OILERR_SIZE)))
			//{
			//	*i_number=atoi(rdbuffer+9);	
			//}

			if(i_number == 0 && LOG_OILERR_SIZE == (rdbytes=fileRead(nfd,0,rdbuffer,LOG_OILERR_SIZE)))
			{
				i_number = atoi(rdbuffer+9);
			}

			//根据记录条数计算下次存储的位置
			i_offset=(1*LOG_OILERR_SIZE+i_number*LOG_OILERR_SIZE)%LOG_OILERR_NUMBER_MAX;

			//写入数据并累加记录条数
			if(LOG_OILERR_SIZE==fileWrite(nfd, i_offset, msg_st.msgBuffer+1, LOG_OILERR_SIZE))
			{
				//(*i_number)++;
				i_number++;
			}

			//累计有20条数据未保存时写入文件
			if(i_number_unsave>=20 && ERROR!=nfd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d",i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)	*(rdbuffer+i)=' ';
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(nfd, 0, rdbuffer, LOG_OILERR_SIZE);
				i_number_unsave=0;
			}

			//打印日志信息
			printf("%s\n", msg_st.msgBuffer+1);
		}

		//处理用户记录
		//该日志存储用户日常行为记录等信息，共两个文件LOG_FILE_PATH_USRLOG和LOG_FILE_PATH_USRLOG_OLD，
		//LOG_FILE_PATH_USRLOG_OLD为存储满的上一个记录文件，LOG_FILE_PATH_USRLOG为当前存储文件
		
		if(msg_len>0 && LOG_FILE_USRLOG==*msg_st.msgBuffer)
		{
			printf("%s\n", msg_st.msgBuffer+1);
		}

		//油机断电，关闭文件
		if(POWER_STATE_OK!=powerStateRead())
		{
			if(nfd > 0)
			{
				close(nfd);
				nfd = -1;
			}

			//nfd = 0;
			//i_number = 0;
			//fd=&fdOilErrLogA;	i_number=&numberOilErrLogA;
			if(ERROR!=nfd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d", i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)	*(rdbuffer+i)=' ';
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(nfd, 0, rdbuffer, LOG_OILERR_SIZE);
				close(nfd);
				nfd = ERROR;
				//yym yaffs_close(*fd);	*fd=ERROR;
			}


			//fd=&fdOilErrLogB;	i_number=&numberOilErrLogB;
			if(ERROR!=nfd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d", i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)	*(rdbuffer+i)=' ';
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(nfd, 0, rdbuffer, LOG_OILERR_SIZE);
				close(nfd);
				nfd = ERROR;
			}
		}

		//taskDelay(1);
		//printf("msg = %s\n", msg_st.msgBuffer);
		usleep(1000);
	}

	return;
}

void td_Print(void* argc)
{
	int printer = (int*)argc;
	printf("------%d\n",printer);
	PrintParamStruct *param=NULL;
	PrintListNode *node=NULL;
	int lastbytes = 0;
	const PRINTER_MAX_SIZE_ONE_TIME = 1024;

	//判断设备选择
	if(PRINTER_A==printer)
	{
		param=&printParamA;
		param->comFd = COM13;  //fj:先注释
		//printf("aa\n");
	}
	else if(PRINTER_B==printer)
	{
		param=&printParamB;
		param->comFd = COM17;  //fj:先注释
		//printf("bb\n");
	}
	else
	{
		printf("%s:%d:printer select error!\n", __FUNCTION__, __LINE__);
		return;
	}

	//读取打印数据队列并完成打印数据的发送
	/*	为避免大数据量造成打印机缓存溢出，当打印数据超过单次允许发送的
	*	数据长度(PRINTER_MAX_SIZE_ONE_TIME)时分次打印并在中间增加一定的延时
	*/

	FOREVER
	{
		node = (PrintListNode*)lstGet(&(param->list));
		if(NULL == node)
		{
			//taskDelay(10*sysClkRateGet()/1000);
			usleep(10000); //fj:
			continue;
		}

		lastbytes = node->length;
		while(lastbytes>0)
		{
			if(lastbytes > PRINTER_MAX_SIZE_ONE_TIME)
			{
				comWriteInTime(param->comFd, node->buffer + (node->length-lastbytes), PRINTER_MAX_SIZE_ONE_TIME, 2);  //fj:先注释
				lastbytes = lastbytes - PRINTER_MAX_SIZE_ONE_TIME;
			}
			else
			{
				comWriteInTime(param->comFd, node->buffer+ (node->length-lastbytes), lastbytes, 2); //fj:都在oilCom.c类里
				lastbytes = lastbytes - lastbytes;
			}

			//taskDelay(200*ONE_MILLI_SECOND);
			usleep(200000);  //fj:
		}

		free(node->buffer);
		node->buffer = NULL;
		free(node);	
		node = NULL;

		//taskDelay(10*sysClkRateGet()/1000);
		usleep(10000); //fj:
	}

	return;
}
