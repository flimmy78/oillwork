#ifndef __MAIN_H__
#define __MAIN_H__

#define FOREVER      for(;;)
#define ERROR        (-1)
#define OK           (0)
#define ok           (0)
#define POWER_STATE_OK       0   //电源状态正常
#define POWER_STATE_ERROR    1   //电源状态异常

struct msg_struct
{
    long msgType;
	char msgBuffer[1024];
};

#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> //fj:20170916
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <strings.h> 
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <error.h>
#include <dirent.h>    //文件夹操作相关
#include <unistd.h>
#include <linux/prctl.h> //20171115
#include "lstLib.h"    //从vxworks转换过来的
#include "rngLib.h"    //从vxworks转换过来的
//#include "va_list.h" //fj:20170916
//#include "stdarg.h"  //fj:20170916

#include "spiDev.h"      //驱动
#include "IOPinConfig.h"
#include "IOWav.h"

//#define ERROR (0)
//#define ERROR (-1)
#include "Public.h"

#include "fjLog.h"

#include "AT91SAM9G20.h"

#include "usrNet.h"
#include "oilTcpip.h"

#include "oilDes.h"
#include "oilCfg.h"
#include "oilStmTransmit.h"
#include "AppPacket.h"
#include "oilFile.h"
#include "oilLog.h"
#include "oilParam.h"
#include "oilSelect.h"
#include "oilPcd.h"
#include "oilTerminalDevice.h"
#include "oilKb.h"
#include "oilLianDi.h"
#include "oilIC.h"
#include "oilCom.h"
#include "oilJL.h"

#include "oilGas.h"
#include "oilPrinter.h"
#include "oilBarcode.h"
#include "oilDsp.h"
#include "oilFRam.h"

#include "oilPC.h"

#include "oilBoardTrans.h"
#include "oilKJLD.h"
#include "oilSpk.h"

//#include "oilJl.h" //fj:20170915



#include "oilIpt.h"
#include "oilAuthorize.h"

#include "oiletc.h"  //fj:20171120

//#include "oilIptXJ.h"

#include "AppPacket_Low.h"
#include "AppPacket_Mid.h"

#include "UpdateJLexe.h"

extern int powerStateRead(void);

#endif


