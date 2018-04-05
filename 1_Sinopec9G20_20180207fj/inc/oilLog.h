#ifndef _OIL_LOG_H_
#define _OIL_LOG_H_

#define LOG_NAME_MAX_SIZE			51					//文件名能够存储的最大长度
#define LOG_HEAD_MAX_SIZE			50					//日志文件自动添加的数据头最大长度
#define LOG_
#define LOG_MSSAGE_MAX_SIZE			1024				//日志信息允许存储的最大长度
#define LOG_FILE_MAX_SIZE			0x500000			//日志信息文件最大字节数 5M
#define LOG_USER_TASK_PRI			250					//日志信息文件保存任务优先级
#define LOG_OILERR_SIZE				64					//加油异常记录每条的长度
#define LOG_OILERR_NUMBER_MAX		50000				//加油异常记录最大条数
#define LOG_FILE_PATH_USRLOG		"../config/mboardFiles/UserLog.txt"		//用户日志文件路径
#define LOG_FILE_PATH_USRLOG_OLD	"../config/mboardFiles/UserLogOld.txt"	//用户日志文件旧文件路径
#define LOG_FILE_PATH_OILERRA		"../config/mboardFiles/OilErrLogA.txt"	//A枪加油异常记录文件路径
#define LOG_FILE_PATH_OILERRB		"../config/mboardFiles/OilErrLogB.txt"	//B枪加油异常记录文件路径
#define LOG_FILE_USRLOG				0x01				//用户行为日志记录文件		
#define LOG_FILE_OILERRA			0x02				//A枪加油异常记录文件
#define LOG_FILE_OILERRB			0x03				//B枪加油异常记录文件

struct msg_st_log
{
	long msgType;
	char msgBuffer[1025];
};

extern int exg_nMsgID;

extern bool initLog();

//日志打印相关接口
//extern void initLogTask(void);

extern void jljSetNameAndLine(const unsigned char *file_name, unsigned int line_no);
extern void jljGetNameAndLine(unsigned char *file_name, unsigned int *line_no);
extern void __jljRunLog(const char * fmt, ...);
extern void __jljLogWrite(const char *fmt, ...);
extern void __jljUsrLogWrite(const char *fmt, ...);
extern int  __jljOilErrLogWrite(char argument, const char *fmt, ...);
extern int  __jljOilErrLogRead(char argument, unsigned int direction,	char *outbuffer,int maxbytes);


//打印运行日志，不保存日志文件
#define jljRunLog jljSetNameAndLine(__FILE__, __LINE__),	__jljRunLog
//打印用户日志，保存用户日志文件
#define jljUserLog	jljSetNameAndLine(__FILE__, __LINE__), __jljUsrLogWrite
//打印异常日志，保存加油异常日志文件
#define jljOilErrLogWrite	__jljOilErrLogWrite
//读取加油异常日志文件
#define jljOilErrLogRead	__jljOilErrLogRead


void jljLogSaveTask(void);

#endif

