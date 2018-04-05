#ifndef _OIL_LOG_H_
#define _OIL_LOG_H_

#define LOG_NAME_MAX_SIZE			51					//�ļ����ܹ��洢����󳤶�
#define LOG_HEAD_MAX_SIZE			50					//��־�ļ��Զ���ӵ�����ͷ��󳤶�
#define LOG_
#define LOG_MSSAGE_MAX_SIZE			1024				//��־��Ϣ����洢����󳤶�
#define LOG_FILE_MAX_SIZE			0x500000			//��־��Ϣ�ļ�����ֽ��� 5M
#define LOG_USER_TASK_PRI			250					//��־��Ϣ�ļ������������ȼ�
#define LOG_OILERR_SIZE				64					//�����쳣��¼ÿ���ĳ���
#define LOG_OILERR_NUMBER_MAX		50000				//�����쳣��¼�������
#define LOG_FILE_PATH_USRLOG		"../config/mboardFiles/UserLog.txt"		//�û���־�ļ�·��
#define LOG_FILE_PATH_USRLOG_OLD	"../config/mboardFiles/UserLogOld.txt"	//�û���־�ļ����ļ�·��
#define LOG_FILE_PATH_OILERRA		"../config/mboardFiles/OilErrLogA.txt"	//Aǹ�����쳣��¼�ļ�·��
#define LOG_FILE_PATH_OILERRB		"../config/mboardFiles/OilErrLogB.txt"	//Bǹ�����쳣��¼�ļ�·��
#define LOG_FILE_USRLOG				0x01				//�û���Ϊ��־��¼�ļ�		
#define LOG_FILE_OILERRA			0x02				//Aǹ�����쳣��¼�ļ�
#define LOG_FILE_OILERRB			0x03				//Bǹ�����쳣��¼�ļ�

struct msg_st_log
{
	long msgType;
	char msgBuffer[1025];
};

extern int exg_nMsgID;

extern bool initLog();

//��־��ӡ��ؽӿ�
//extern void initLogTask(void);

extern void jljSetNameAndLine(const unsigned char *file_name, unsigned int line_no);
extern void jljGetNameAndLine(unsigned char *file_name, unsigned int *line_no);
extern void __jljRunLog(const char * fmt, ...);
extern void __jljLogWrite(const char *fmt, ...);
extern void __jljUsrLogWrite(const char *fmt, ...);
extern int  __jljOilErrLogWrite(char argument, const char *fmt, ...);
extern int  __jljOilErrLogRead(char argument, unsigned int direction,	char *outbuffer,int maxbytes);


//��ӡ������־����������־�ļ�
#define jljRunLog jljSetNameAndLine(__FILE__, __LINE__),	__jljRunLog
//��ӡ�û���־�������û���־�ļ�
#define jljUserLog	jljSetNameAndLine(__FILE__, __LINE__), __jljUsrLogWrite
//��ӡ�쳣��־����������쳣��־�ļ�
#define jljOilErrLogWrite	__jljOilErrLogWrite
//��ȡ�����쳣��־�ļ�
#define jljOilErrLogRead	__jljOilErrLogRead


void jljLogSaveTask(void);

#endif

