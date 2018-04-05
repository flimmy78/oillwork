//#include <VxWorks.h>
//#include "../inc/main.h"
//#include <string.h>
//#include <stdarg.h>
//#include "../inc/oilFile.h"
//#include "../inc/oilLog.h"
//#include <sys/types.h>		// ��Ϣ����ʹ��ͷ�ļ�
//#include <sys/ipc.h>		// ��Ϣ����ʹ��ͷ�ļ�
//#include <sys/msg.h>		// ��Ϣ����ʹ��ͷ�ļ�
//#include <pthread.h>		// �߳�ʹ��ͷ�ļ�

#include "../inc/main.h"


int exg_nMsgID = ERROR;

//��ӡ��־��Ϣ���ļ������к�
unsigned char thisFileName[LOG_NAME_MAX_SIZE+1]={0};
unsigned int thisLineNO=0;


int msgIdUserLog=-1;   //��־�ļ�������Ϣ����
int tIdUserLog=1;     //��־�ļ���������ID
int fdUserLog=ERROR;  //��־�ļ�������
int fdOilErrLogA=ERROR, fdOilErrLogB=ERROR;                //�����쳣��־��¼�ļ�����
unsigned int numberOilErrLogA=0, numberOilErrLogB=0;   //�����쳣��־��¼������
unsigned int numberOilErrUnsaveA=0, numberOilErrUnsaveB=0;   //�����쳣��־��¼������
unsigned int oilErrLogInqOffsetA=0, oilErrLogInqOffsetB=0; //�����쳣��¼��ǰ��ѯ�����ļ�ͷ��һ������Ϊ����0��ƫ������

//��־�ļ���������
//void jljLogSaveTask(void);
void __jljLogMsgSend(char *inbuffer, int nbytes);



/*******************************************************************
Name				:initLogTask
*Description		:��ʼ��д��־����
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
*Description		:��־��д�뼰�ϵ籣��
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/
void jljLogSaveTask(void)
{
	prctl(PR_SET_NAME,(unsigned long)"jljLogSaveTask");
	char msg_buffer[LOG_MSSAGE_MAX_SIZE+1]={0};		//��Ϣ����
	int msg_len=0;									//��ȡ������Ϣ����
	
	int *fd=NULL;									//���β������ļ�������
	char *path=NULL;								//���β������ļ�·��
	char rdbuffer[LOG_OILERR_SIZE+64]={0};			//���ݶ�ȡ����
	int rdbytes=0;									//���ݶ�ȡ����
	unsigned int *i_number=NULL;					//�����쳣��¼��������
	unsigned int *i_number_unsave=NULL;				//�����쳣��¼��������
	unsigned int i_offset=0;						//�������ļ��е�ƫ��λ��
	int i=0;

	struct msg_struct msg_st;
	msg_st.msgType = 0;

	while ( 1 )
	{
		//�ȴ�������־����
		memset(msg_buffer, 0, LOG_MSSAGE_MAX_SIZE+1);
		memset(msg_st.msgBuffer,0,LOG_MSSAGE_MAX_SIZE);
		msg_len = 0;
		msg_len=msgrcv(msgIdUserLog,&msg_st, LOG_MSSAGE_MAX_SIZE, 0, IPC_NOWAIT);

		if(msg_len > 0)
		{
           memcpy(msg_buffer,msg_st.msgBuffer,msg_len);
		}

		//��������쳣��¼����,���ļ��Թ̶�����LOG_OILERR_SIZE�����洢�쳣��¼���ļ����ֽ�λ�ñ������洢��Ŀ������
		//ƽʱ�򿪺󲻹رգ���⵽�����ź�֮�����йرձ���	
		if(msg_len>0 && (LOG_FILE_OILERRA==*msg_buffer || LOG_FILE_OILERRB==*msg_buffer))
		{			
			if(LOG_FILE_OILERRA==*msg_buffer)//��ʼ����������
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

			//����־�ļ�
			if(ERROR==*fd)	
			{
				*fd=fileOpen(path, O_CREAT|O_RDWR, S_IREAD | S_IWRITE);
			}

			//��δ��ȡ��¼����ʱ��ȡ��¼��������ʽΪ"��¼����=XXXXXXXXXX"
			if(0==(*i_number) && LOG_OILERR_SIZE==(rdbytes=fileRead(*fd, 0, rdbuffer, LOG_OILERR_SIZE)))
			{
				*i_number=atoi(rdbuffer+9);	
			}

			//���ݼ�¼���������´δ洢��λ��
			i_offset=(1*LOG_OILERR_SIZE+(*i_number)*LOG_OILERR_SIZE)%LOG_OILERR_NUMBER_MAX;

			//д�����ݲ��ۼӼ�¼����
			if(LOG_OILERR_SIZE==fileWrite(*fd, i_offset, msg_buffer+1, LOG_OILERR_SIZE))
			{
				(*i_number)++;
			}

			//�ۼ���20������δ����ʱд���ļ�
			if(*i_number_unsave>=0 && ERROR!=*fd)
			{
				memcpy(rdbuffer, "��¼����=", 9);
				sprintf(rdbuffer+9, "%d", *i_number);
				for(i=strlen(rdbuffer); i<LOG_OILERR_SIZE-1; i++)
				{
					*(rdbuffer+i)=' ';
				}
				*(rdbuffer+LOG_OILERR_SIZE-1)='\n';
				fileWrite(*fd, 0, rdbuffer, LOG_OILERR_SIZE);

				*i_number_unsave=0;
			}

			//��ӡ��־��Ϣ
			printf("%s\n", msg_buffer+1);
		}

		//�����û���¼
		//����־�洢�û��ճ���Ϊ��¼����Ϣ���������ļ�LOG_FILE_PATH_USRLOG��LOG_FILE_PATH_USRLOG_OLD��
		//LOG_FILE_PATH_USRLOG_OLDΪ�洢������һ����¼�ļ���LOG_FILE_PATH_USRLOGΪ��ǰ�洢�ļ�
		
		if(msg_len>0 && LOG_FILE_USRLOG==*msg_buffer)
		{
			printf("%s\n", msg_buffer+1);
		}

		//�ͻ��ϵ磬�ر��ļ�
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
				memcpy(rdbuffer, "��¼����=", 9);
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
				memcpy(rdbuffer, "��¼����=", 9);
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
*Description		:������־��Ϣ���û���־�ļ���
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
*Description		:���õ�ǰ�ļ�������
*Input				:None
*Output				:None
*Return				:None
*History			:2015-07-07,modified by syj
*/

void jljSetNameAndLine(const unsigned char *file_name, unsigned int line_no)
{
	//�ж��ļ�������������ڻ����ļ����Ŀռ䳤�����ٱ��棬Ϊ�˱�����ң���ɴ�ӡ��ǰ�洢���������������
	if(strlen((char*)file_name)+1 > sizeof(thisFileName))
	{
		memset(thisFileName, 0, sizeof(thisFileName));	
		thisLineNO=0;
		return;
	}

	//���浱ǰ�ļ�������
	memset(thisFileName, 0, sizeof(thisFileName));
	strcpy((char*)thisFileName, (char*)file_name);	
	thisLineNO=line_no;

	return;
}


/*******************************************************************
*Name				:jljGetNameAndLine
*Description		:��ȡ��ǰ�ļ�������
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
*Description		:��ӡ��־
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

	//��ǰʱ��
	timeRead(&sTime);
	sprintf(sMessage, "%02X-%02X %02X:%02X:%02X", sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//��ǰ�ļ���������
	jljGetNameAndLine(sName, &sLineNO);
	sprintf(sMessage + strlen(sMessage), "[%s:%d]", sName, sLineNO);

	//��ǰ������
	//yym sTaskName = taskName(taskIdSelf());
	//yym if(NULL != sTaskName)
	//yym {
	//yym 	sprintf(sMessage + strlen(sMessage), "[%s]:", sTaskName);
	//yym }
	char chThreadName[32] = {0};
	prctl(PR_GET_NAME,(unsigned long)chThreadName);
	sprintf(sMessage + strlen(sMessage), "[%s]:", chThreadName);
	

	//��������ͷ
	if(strlen(sMessage) < LOG_HEAD_MAX_SIZE)
	{
		memset(sMessage + strlen(sMessage), ' ', LOG_HEAD_MAX_SIZE-strlen(sMessage));
		sMessage[LOG_HEAD_MAX_SIZE]='\0';
	}

	//�����־��Ϣ
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//��ӡ����
	//printf("%s\n", sMessage); //20171207

	return;
}


/*******************************************************************
*Name				:__jljUsrLogWrite
*Description		:������־��Ϣ���û���־�ļ���
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

	//��־����
	*sMessage=LOG_FILE_USRLOG;

	//��ǰʱ��
	timeRead(&sTime);
	sprintf(sMessage + strlen(sMessage), "%02X-%02X %02X:%02X:%02X", sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//��ǰ�ļ���������
	jljGetNameAndLine(sName, &sLineNO);
	sprintf(sMessage + strlen(sMessage), "[%s:%d]", sName, sLineNO);

	//��ǰ������
	//yym sTaskName=taskName(taskIdSelf());
	//yym if(NULL!=sTaskName)
	//yym {
	//yym 	sprintf(sMessage + strlen(sMessage), "[%s]:", sTaskName);
	//yym }
	char chThreadName[32] = {0};
	prctl(PR_GET_NAME,(unsigned long)chThreadName);
	sprintf(sMessage + strlen(sMessage), "[%s]:", chThreadName);

	//��������ͷ
	if(strlen(sMessage) < LOG_HEAD_MAX_SIZE)
	{
		memset(sMessage + strlen(sMessage), ' ', LOG_HEAD_MAX_SIZE-strlen(sMessage));
		sMessage[LOG_HEAD_MAX_SIZE]='\0';
	}

	//�����־��Ϣ
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//����־���ݴ�����־�ļ�
	__jljLogMsgSend(sMessage, strlen(sMessage));

	return;
}


/*******************************************************************
*Name			:__jljOilErrLogWrite
*Description	:��ӡ����������쳣��־�ļ����Թ̶�LOG_OILERR_SIZE�洢һ����д������ʱ�Զ���ĩβ���'\n'
*Input			:argument		���� 0/1
*				:fmt					��ʽ���ַ���
*				:[argument]...	��ѡ����
*Output			:None
*Return			:�ɹ�����0��ʧ�ܷ�������
*History		:2015-10-27,modified by syj
*/

int __jljOilErrLogWrite(char argument, const char *fmt, ...)
{
	va_list sVaArgs;
	char sMessage[LOG_MSSAGE_MAX_SIZE+1]={0};
	unsigned int i=0;
	RTCTime sTime;

	//��־����
	if(0==argument)	
		*sMessage=LOG_FILE_OILERRA;
	else 
		*sMessage=LOG_FILE_OILERRB;

	//��ǰʱ��
	timeRead(&sTime);
	sprintf(sMessage+strlen(sMessage), "%02X%02X%02X %02X:%02X:%02X:", sTime.year, sTime.month, sTime.date, sTime.hour, sTime.minute, sTime.second);

	//�����־��Ϣ
	va_start(sVaArgs, fmt);
	vsprintf(sMessage + strlen(sMessage), fmt, sVaArgs);
	va_end(sVaArgs);

	//�Կո����ʣ���ֽ�
	for(i=(int)strlen(sMessage); i<LOG_OILERR_SIZE; i++)	
		*(sMessage+i)=' ';

	//��任�з�
	*(sMessage+LOG_OILERR_SIZE)='\n';

	//����־���ݴ�����־�ļ�
	__jljLogMsgSend(sMessage, (int)strlen(sMessage));

	return 0;
}


/*******************************************************************
*Name				:__jljOilErrLogRead
*Description		:��������쳣��־��Ϣ����־�ļ���
*Input				:argument				�������� 0/1
*					:direction				���ҷ���0=��������һ����¼;	1=���Ҹ������Ŀ��2=���Ҹ������Ŀ
*					:maxbytes			��־���ݻ��泤�ȣ�Ӧ��С��LOG_OILERR_SIZE
*Output				:outbuffer				��־����
*Return				:�ɹ�����0��ʧ�ܷ�������
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

	//��ʼ����������
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

	//�����δ��ȡ��¼�������ȡ��¼����
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

	//�ж���������
	if(0==*i_number || 0xffffffff==*i_number)
	{
		istate=ERROR;
		goto END;
	}

	//printf("jljOilErrLogRead dddddd\n");

	//���㵱�β�ѯ��ƫ����������ѯ����Ϊ0ʱ�����ѯ����һ����¼
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

	//��ȡ����
	if(0==(rdbytes=fileRead(*fd, i_offset, rdbuffer, LOG_OILERR_SIZE)))
	{
		istate=ERROR;
		goto END;
	}

	//printf("jljOilErrLogRead eeeeee\n");

	//�������
	if(maxbytes<LOG_OILERR_SIZE)		
		memcpy(outbuffer, rdbuffer, maxbytes);
	else													
		memcpy(outbuffer, rdbuffer, LOG_OILERR_SIZE);

	//printf("jljOilErrLogRead ffffff\n");

END:
	return istate;
}


