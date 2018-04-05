//#include <VxWorks.h>
//#include "../inc/main.h"
//#include <string.h>
//#include <stdarg.h>
//#include "../inc/oilFile.h"
//#include "../inc/oilLog.h"
//#include <sys/types.h>		// 消息队列使用头文件
//#include <sys/ipc.h>		// 消息队列使用头文件
//#include <sys/msg.h>		// 消息队列使用头文件
//#include <pthread.h>		// 线程使用头文件

#include "../inc/main.h"


int exg_nMsgID = ERROR;

//打印日志信息的文件名和行号
unsigned char thisFileName[LOG_NAME_MAX_SIZE+1]={0};
unsigned int thisLineNO=0;


int msgIdUserLog=-1;   //日志文件保存消息队列
int tIdUserLog=1;     //日志文件保存任务ID
int fdUserLog=ERROR;  //日志文件描述符
int fdOilErrLogA=ERROR, fdOilErrLogB=ERROR;                //加油异常日志记录文件描述
unsigned int numberOilErrLogA=0, numberOilErrLogB=0;   //加油异常日志记录总条数
unsigned int numberOilErrUnsaveA=0, numberOilErrUnsaveB=0;   //加油异常日志记录总条数
unsigned int oilErrLogInqOffsetA=0, oilErrLogInqOffsetB=0; //加油异常记录当前查询的以文件头第一条数据为基础0的偏移条数

//日志文件操作函数
//void jljLogSaveTask(void);
void __jljLogMsgSend(char *inbuffer, int nbytes);



/*******************************************************************
Name				:initLogTask
*Description		:初始化写日志任务
*Input				:None
*Output				:None
*Return				:None
*History			:2017-09-08
*/

bool initLog()
{
   int msgID = msgget(IPC_PRIVATE,IPC_CREAT|0666);
   if(msgID == ERROR)
   {
	   printf("msgget log failure!\n");
	   return false;
   }

   msgIdUserLog = msgID;

   return true;
}

/*******************************************************************
Name				:jljLogSaveTask
*Description		:日志的写入及断电保存
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/
void jljLogSaveTask(void)
{
	prctl(PR_SET_NAME,(unsigned long)"jljLogSaveTask");
	char msg_buffer[LOG_MSSAGE_MAX_SIZE+1]={0};		//消息缓存
	int msg_len=0;									//读取到的消息长度
	
	int *fd=NULL;									//当次操作的文件描述符
	char *path=NULL;								//当次操作的文件路径
	char rdbuffer[LOG_OILERR_SIZE+64]={0};			//数据读取缓存
	int rdbytes=0;									//数据读取长度
	unsigned int *i_number=NULL;					//加油异常记录数据条数
	unsigned int *i_number_unsave=NULL;				//加油异常记录数据条数
	unsigned int i_offset=0;						//数据在文件中的偏移位置
	int i=0;

	struct msg_struct msg_st;
	msg_st.msgType = 0;

	while ( 1 )
	{
		//等待接收日志数据
		memset(msg_buffer, 0, LOG_MSSAGE_MAX_SIZE+1);
		memset(msg_st.msgBuffer,0,LOG_MSSAGE_MAX_SIZE);
		msg_len = 0;
		msg_len=msgrcv(msgIdUserLog,&msg_st, LOG_MSSAGE_MAX_SIZE, 0, IPC_NOWAIT);

		if(msg_len > 0)
		{
           memcpy(msg_buffer,msg_st.msgBuffer,msg_len);
		}

		//处理加油异常记录数据,本文件以固定长度LOG_OILERR_SIZE按条存储异常记录，文件首字节位置保存所存储条目总数；
		//平时打开后不关闭，检测到掉电信号之后再行关闭保存	
		if(msg_len>0 && (LOG_FILE_OILERRA==*msg_buffer || LOG_FILE_OILERRB==*msg_buffer))
		{			
			if(LOG_FILE_OILERRA==*msg_buffer)//初始化操作数据
			{
				fd=&fdOilErrLogA;
				path=LOG_FILE_PATH_OILERRA;	
				i_number=&numberOilErrLogA;	
				i_number_unsave=&numberOilErrUnsaveA;
			}
			else
			{
				fd=&fdOilErrLogB;	
				path=LOG_FILE_PATH_OILERRB;	
				i_number=&numberOilErrLogB;	
				i_number_unsave=&numberOilErrUnsaveB;
			}

			//打开日志文件
			if(ERROR==*fd)	
			{
				*fd=fileOpen(path, O_CREAT|O_RDWR, S_IREAD | S_IWRITE);
			}

			//尚未读取记录条数时读取记录条数，格式为"记录条数=XXXXXXXXXX"
			if(0==(*i_number) && LOG_OILERR_SIZE==(rdbytes=fileRead(*fd, 0, rdbuffer, LOG_OILERR_SIZE)))
			{
				*i_number=atoi(rdbuffer+9);	
			}

			//根据记录条数计算下次存储的位置
			i_offset=(1*LOG_OILERR_SIZE+(*i_number)*LOG_OILERR_SIZE)%LOG_OILERR_NUMBER_MAX;

			//写入数据并累加记录条数
			if(LOG_OILERR_SIZE==fileWrite(*fd, i_offset, msg_buffer+1, LOG_OILERR_SIZE))
			{
				(*i_number)++;
			}

			//累计有20条数据未保存时写入文件
			if(*i_number_unsave>=0 && ERROR!=*fd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d", *i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)
				{
					*(rdbuffer+i)=' ';
				}
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(*fd, 0, rdbuffer, LOG_OILERR_SIZE);

				*i_number_unsave=0;
			}

			//打印日志信息
			printf("%s\n", msg_buffer+1);
		}

		//处理用户记录
		//该日志存储用户日常行为记录等信息，共两个文件LOG_FILE_PATH_USRLOG和LOG_FILE_PATH_USRLOG_OLD，
		//LOG_FILE_PATH_USRLOG_OLD为存储满的上一个记录文件，LOG_FILE_PATH_USRLOG为当前存储文件
		
		if(msg_len>0 && LOG_FILE_USRLOG==*msg_buffer)
		{
			printf("%s\n", msg_buffer+1);
		}

		//油机断电，关闭文件
		if(POWER_STATE_OK!=powerStateRead())
		{
			if(ERROR!=fdUserLog)
			{
				close(fdUserLog);
				fdUserLog = ERROR;
			}

			fd=&fdOilErrLogA;
			i_number=&numberOilErrLogA;

			if(ERROR!=*fd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d", *i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)	*(rdbuffer+i)=' ';
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(*fd, 0, rdbuffer, LOG_OILERR_SIZE);
				close(*fd);
				*fd = ERROR;
			}

			fd=&fdOilErrLogB;	i_number=&numberOilErrLogB;
			if(ERROR!=*fd)
			{
				memcpy(rdbuffer, "记录条数=", 9);
				sprintf(rdbuffer+9, "%d", *i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)
				{
					*(rdbuffer+i)=' ';
				}
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(*fd, 0, rdbuffer, LOG_OILERR_SIZE);
				close(*fd);
				*fd = ERROR;
			}
		}

		//taskDelay(1);
		//printf("msg = %s\n", msg_buffer);
		usleep(1000);
	}

	return;
}


/*******************************************************************
*Name				:__jljUsrLogWrite
*Description		:保存日志信息到用户日志文件中
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void __jljLogMsgSend(char *inbuffer, int nbytes)
{
	struct msg_struct msg_st;
	msg_st.msgType = 1;

	//printf("jljLogMsgSend aaaaaa\n");

    if(msgIdUserLog != ERROR)
	{
	    //printf("jljLogMsgSend bbbbbb,nbytes = %d\n",nbytes);
		memcpy(msg_st.msgBuffer,inbuffer,nbytes);
		msgsnd(msgIdUserLog,&msg_st,nbytes,IPC_NOWAIT);
	}
}


/*******************************************************************
*Name				:jljSetNameAndLine
*Description		:设置当前文件名和行
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void jljSetNameAndLine(const unsigned char *file_name, unsigned int line_no)
{
	//判断文件名长度如果大于缓存文件名的空间长度则不再保存，为了避免混乱，造成打印以前存储的内容则将内容清空
	if(strlen((char*)file_name)+1 > sizeof(thisFileName))
	{
		memset(thisFileName, 0, sizeof(thisFileName));	
		thisLineNO=0;
		return;
	}

	//保存当前文件名及行
	memset(thisFileName, 0, sizeof(thisFileName));
	strcpy((char*)thisFileName, (char*)file_name);	
	thisLineNO=line_no;

	return;
}


/*******************************************************************
*Name				:jljGetNameAndLine
*Description		:获取当前文件名和行
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void jljGetNameAndLine(unsigned char *file_name, unsigned int *line_no)
{
	strcpy((char*)file_name, (char*)thisFileName);
	*line_no=thisLineNO;

	return;
}


/*******************************************************************
*Name				:__jljRunLog
*Description		:打印日志
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void __jljRunLog(const char *fmt, ...)
{
	va_list sVaArgs;
    //char *sTaskName = NULL; 
	char sMessage[LOG_MSSAGE_MAX_SIZE+1] = {0};
	unsigned char sName[LOG_NAME_MAX_SIZE+1] = {0};
	unsigned int sLineNO = 0;
	RTCTime sTime;

	//当前时间
	timeRead(&sTime);
	sprintf(sMessage, "%02X-%02X %02X:%02X:%02X", sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//当前文件名及行数
	jljGetNameAndLine(sName, &sLineNO);
	sprintf(sMessage + strlen(sMessage), "[%s:%d]", sName, sLineNO);

	//当前任务名
	//yym sTaskName = taskName(taskIdSelf());
	//yym if(NULL != sTaskName)
	//yym {
	//yym 	sprintf(sMessage + strlen(sMessage), "[%s]:", sTaskName);
	//yym }
	char chThreadName[32] = {0};
	prctl(PR_GET_NAME,(unsigned long)chThreadName);
	sprintf(sMessage + strlen(sMessage), "[%s]:", chThreadName);
	

	//对齐数据头
	if(strlen(sMessage) < LOG_HEAD_MAX_SIZE)
	{
		memset(sMessage + strlen(sMessage), ' ', LOG_HEAD_MAX_SIZE-strlen(sMessage));
		sMessage[LOG_HEAD_MAX_SIZE]='\0';
	}

	//填充日志信息
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//打印数据
	//printf("%s\n", sMessage); //20171207

	return;
}


/*******************************************************************
*Name				:__jljUsrLogWrite
*Description		:保存日志信息到用户日志文件中
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void __jljUsrLogWrite(const char *fmt, ...)
{
	va_list sVaArgs;
	char *sTaskName=NULL; 
	char sMessage[LOG_MSSAGE_MAX_SIZE+1]={0};
	unsigned char sName[LOG_NAME_MAX_SIZE+1]={0};
	unsigned int sLineNO=0;
	RTCTime sTime;

	//日志类型
	*sMessage=LOG_FILE_USRLOG;

	//当前时间
	timeRead(&sTime);
	sprintf(sMessage + strlen(sMessage), "%02X-%02X %02X:%02X:%02X", sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//当前文件名及行数
	jljGetNameAndLine(sName, &sLineNO);
	sprintf(sMessage + strlen(sMessage), "[%s:%d]", sName, sLineNO);

	//当前任务名
	//yym sTaskName=taskName(taskIdSelf());
	//yym if(NULL!=sTaskName)
	//yym {
	//yym 	sprintf(sMessage + strlen(sMessage), "[%s]:", sTaskName);
	//yym }
	char chThreadName[32] = {0};
	prctl(PR_GET_NAME,(unsigned long)chThreadName);
	sprintf(sMessage + strlen(sMessage), "[%s]:", chThreadName);

	//对齐数据头
	if(strlen(sMessage) < LOG_HEAD_MAX_SIZE)
	{
		memset(sMessage + strlen(sMessage), ' ', LOG_HEAD_MAX_SIZE-strlen(sMessage));
		sMessage[LOG_HEAD_MAX_SIZE]='\0';
	}

	//填充日志信息
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//将日志数据存入日志文件
	__jljLogMsgSend(sMessage, strlen(sMessage));

	return;
}


/*******************************************************************
*Name			:__jljOilErrLogWrite
*Description	:打印并保存加油异常日志文件，以固定LOG_OILERR_SIZE存储一条，写入数据时自动在末尾添加'\n'
*Input			:argument		面板号 0/1
*				:fmt					格式化字符串
*				:[argument]...	可选参数
*Output			:None
*Return			:成功返回0；失败返回其它
*History		:2015-10-27,modified by syj
*/

int __jljOilErrLogWrite(char argument, const char *fmt, ...)
{
	va_list sVaArgs;
	char sMessage[LOG_MSSAGE_MAX_SIZE+1]={0};
	unsigned int i=0;
	RTCTime sTime;

	//日志类型
	if(0==argument)	
		*sMessage=LOG_FILE_OILERRA;
	else 
		*sMessage=LOG_FILE_OILERRB;

	//当前时间
	timeRead(&sTime);
	sprintf(sMessage+strlen(sMessage), "%02X%02X%02X %02X:%02X:%02X:", sTime.year, sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//填充日志信息
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//以空格填充剩余字节
	for(i=(int)strlen(sMessage); i<LOG_OILERR_SIZE; i++)	
		*(sMessage+i)=' ';

	//填充换行符
	*(sMessage+LOG_OILERR_SIZE)='\n';

	//将日志数据存入日志文件
	__jljLogMsgSend(sMessage, (int)strlen(sMessage));

	return 0;
}


/*******************************************************************
*Name				:__jljOilErrLogRead
*Description		:保存加油异常日志信息到日志文件中
*Input				:argument				物理面板号 0/1
*					:direction				查找方向，0=查找最晚一条记录;	1=查找更早的条目；2=查找更晚的条目
*					:maxbytes			日志数据缓存长度，应不小于LOG_OILERR_SIZE
*Output				:outbuffer				日志数据
*Return				:成功返回0；失败返回其它
*History			:2015-10-27,modified by syj
*/

int __jljOilErrLogRead(
	char argument, 
	unsigned int direction, 
	char *outbuffer,
	int maxbytes)
{
	int *fd=NULL, istate=0;
	char *path=NULL;
	unsigned int *i_number=NULL, *seach_offset=NULL, i_offset=0;
	char rdbuffer[LOG_OILERR_SIZE+16]={0};
	int rdbytes=0;

	//printf("jljOilErrLogRead aaaaaa\n");

	//初始化操作数据
	if(0==argument)
	{
		fd=&fdOilErrLogA;
		path=LOG_FILE_PATH_OILERRA;
		i_number=&numberOilErrLogA;	
		seach_offset=&oilErrLogInqOffsetA;
	}
	else if(1==argument)
	{
		fd=&fdOilErrLogB;	
		path=LOG_FILE_PATH_OILERRB;	
		i_number=&numberOilErrLogB;	
		seach_offset=&oilErrLogInqOffsetB;
	}
	else
	{
		istate=ERROR;
		goto END;
	}

	//printf("jljOilErrLogRead bbbbbb,i_number = %d,fd=%d,path = %s\n",*i_number,*fd,path);

	//int nFd = fileOpen(path,O_RDWR, S_IREAD | S_IWRITE);
	//int nSize = fileRead(nFd,0,rdbuffer,LOG_OILERR_SIZE);
	//printf("nFd = %d,nSize = %d\n",nFd,nSize);

	//如果尚未读取记录条数则读取记录条数
	if(0==*i_number &&\
		ERROR==*fd &&\
		ERROR!=(*fd=fileOpen(path, O_RDWR, S_IREAD | S_IWRITE)) &&\
		LOG_OILERR_SIZE==(rdbytes=fileRead(*fd, 0, rdbuffer, LOG_OILERR_SIZE)))
	{
		*i_number=atoi(rdbuffer+9);
	    //printf("jljOilErrLogRead i_number = %d,rdbuffer = %s\n",*i_number,rdbuffer);
		//PrintH(rdbytes,rdbuffer);
	}

	//printf("jljOilErrLogRead cccccc\n");

	//判断有无数据
	if(0==*i_number || 0xffffffff==*i_number)
	{
		istate=ERROR;
		goto END;
	}

	//printf("jljOilErrLogRead dddddd\n");

	//计算当次查询的偏移条数，查询方向为0时代表查询最晚一条记录
	if(0==direction)	*seach_offset=(*i_number-1)%LOG_OILERR_NUMBER_MAX;
	else if(1==direction && 0==*seach_offset && *i_number<=LOG_OILERR_NUMBER_MAX)
	{
		istate=ERROR;
		goto END;
	}
	else if(1==direction && 0==*seach_offset && *i_number>LOG_OILERR_NUMBER_MAX)
	{
		*seach_offset=LOG_OILERR_NUMBER_MAX-1;
	}
	else if(1==direction)
	{
		(*seach_offset)--;
	}
	else if(2==direction && (*seach_offset>=LOG_OILERR_NUMBER_MAX || *seach_offset>=*i_number-1) && *i_number<=LOG_OILERR_NUMBER_MAX)
	{
		istate=ERROR;
		goto END;
	}
	else if(2==direction && *seach_offset>=LOG_OILERR_NUMBER_MAX-1 && *i_number>LOG_OILERR_NUMBER_MAX)
	{
		*seach_offset=0;
	}
	else if(2==direction)
	{
		(*seach_offset)++;
	}
	i_offset=*seach_offset%LOG_OILERR_NUMBER_MAX*LOG_OILERR_SIZE + 1*LOG_OILERR_SIZE;

	//读取数据
	if(0==(rdbytes=fileRead(*fd, i_offset, rdbuffer, LOG_OILERR_SIZE)))
	{
		istate=ERROR;
		goto END;
	}

	//printf("jljOilErrLogRead eeeeee\n");

	//输出数据
	if(maxbytes<LOG_OILERR_SIZE)		
		memcpy(outbuffer, rdbuffer, maxbytes);
	else													
		memcpy(outbuffer, rdbuffer, LOG_OILERR_SIZE);

	//printf("jljOilErrLogRead ffffff\n");

END:
	return istate;
}


