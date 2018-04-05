// #include <VxWorks.h>
// #include <lstLib.h>
// #include "oilCfg.h"
// #include "oilKb.h"
// #include "oilCom.h"
// #include "oilLog.h"
// #include "oilLianDi.h"


#include "../inc/main.h"

 LDStructParamType ldStructA1, ldStructB1;
 
// //接口声明
 void tLdRecv(int DEV_DSP_KEYx);
 int ldRecvPars(int DEV_DSP_KEYx, char *inbuffer, int nbytes);
 int ldSend(int DEV_DSP_KEYx, char *inbuffer, int nbtes, char *outbuffer, int maxbytes, int seconds);
 void tLdPoll(int DEV_DSP_KEYx);

 
// /*****************************************************************************
// *Name				:tLdPoll
// *Description		:主板轮询联迪POS数据
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *Output			:无
// *Return			:无
// *History			:2016-08-29,modified by syj
// */
 void tLdPoll(int DEV_DSP_KEYx)
 {
 	LDStructParamType *ldstruct = NULL;
 	unsigned char tx_buffer[16] = {0};
 	int tx_len = 0;
 	unsigned char rx_buffer[64] = {0};
 	int istate = 0;
 	int devx = 0;
 
 	//判断设备连接的串口
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;	devx = DEV_DSP_KEYA;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;	devx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return;
 	}
 
 	FOREVER
 	{
 		//轮询卡座状态信息
 		tx_len = 0;
 		tx_buffer[tx_len++] = (char)(LD_CMD_CARD_POLL >> 8);
 		tx_buffer[tx_len++] = (char)(LD_CMD_CARD_POLL >> 0);
 		istate = ldSend(devx, tx_buffer, tx_len, rx_buffer, 64, 3);
 		if(istate > 0 && 0 == ((rx_buffer[2] << 8) || (rx_buffer[3] << 0)))
 		{
// 			taskLock();
 			ldstruct->DeckStateS1 = rx_buffer[4];
 			ldstruct->IcTypeS2 = rx_buffer[5];
 			ldstruct->IcStateS3 = rx_buffer[6];
// 			taskUnlock();
 		}
 	
 		//taskDelay(500 * sysClkRateGet() / 1000);
		usleep(500000);
 	}
 
 	return;
}
 
 
// /*****************************************************************************
// *Name				:tLdRecv
// *Description		:主板与终端设备通讯数据接收及解析任务
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *Output			:无
// *Return			:无
// *History			:2016-08-29,modified by syj
// */
void tLdRecv(int DEV_DSP_KEYx)
{
 	LDStructParamType *ldstruct = NULL;
 	int devx = 0;
 
 	char rdbuffer[64] = {0};
 	int rdlenght = 0;
 
 	char recvbuffer[LD_BUFFER_SIZE + 1] = {0};
 	int recvlenght = 0;
 	char step = 0;
 	int i = 0;
 	int data_len = 0;
 	int pcrc = 0;
 
 	//判断设备连接的串口
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;	devx = DEV_DSP_KEYA;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;	devx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return;
 	}
 
 	//数据接收及解析
 	FOREVER
 	{
 		rdlenght = comRead(ldstruct->ComX, rdbuffer, 64);
 		
 		if(rdlenght <= 0)
 		{
 			//taskDelay(1);
			usleep(1000);
 			continue;
 		}
 
 		for(i = 0; i < rdlenght; i++)
 		{
 			recvbuffer[recvlenght] = rdbuffer[i];
 			switch(step)
 			{
 			case 0:
 				//接收数据头
 				if(0xfa == recvbuffer[recvlenght])	{	step = 1;	recvlenght = 1;	}
 				else													{	step = 0;	recvlenght = 0;	}
 				break;
 				
 			case 1:
 				//接收数据目标地址、源地址、帧号/控制、有效数据长度（不能有0xFA）
 				if(0xfa == recvbuffer[recvlenght])	{	step = 0;	recvlenght = 0;	}
 				else													{	recvlenght++;}
 
 				if(recvlenght >=6 )							{	step = 2;}
 				break;
 				
 			case 2:
 				//数据位0xfa时初步认为该字节为转意字符执行下一步处理，否则保存该字节数据
 				if(0xfa == recvbuffer[recvlenght])	{	step=3;	}
 				else													{	recvlenght++;	}
 
 				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
 				data_len = ((((int)recvbuffer[4])>>4)&0x0f)*1000 + ((((int)recvbuffer[4])>>0)&0x0f)*100+\
 							((((int)recvbuffer[5])>>4)&0x0f)*10 + ((((int)recvbuffer[5])>>0)&0x0f)*1;
 
 				//判断有效数据长度合法性
 				if(data_len + 8 >= LD_BUFFER_SIZE)
 				{
 					step = 0;	recvlenght = 0;
 				}
 
 				//判断是否接收完成
 				if(recvlenght>=8 && recvlenght >= (data_len+8))
 				{
 					pcrc = crc16Get(&recvbuffer[1], 5 + data_len);
 					if(pcrc == (recvbuffer[6 + data_len]<<8) | (recvbuffer[7 + data_len]<<0))
 					{
 						ldRecvPars(devx, recvbuffer, recvlenght);
 					}
 
 					step = 0;	recvlenght=0;
 				}
 				break;
 				
 			case 3:
 				//如果是0xfa则作为转义保存当前数据，如果不是0xfa则认为前一个0xfa为包头
 				if(0xfa == recvbuffer[recvlenght])	{	step = 2;	recvlenght++;	}
 				else									
 				{
 					recvbuffer[0] = 0xfa;	recvbuffer[1] = rdbuffer[i];	step = 1;	recvlenght = 2;
 				}
 
 				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
 				data_len = ((((int)recvbuffer[4])>>4)&0x0f)*1000 + ((((int)recvbuffer[4])>>0)&0x0f)*100+\
 							((((int)recvbuffer[5])>>4)&0x0f)*10 + ((((int)recvbuffer[5])>>0)&0x0f)*1;
 
 				//判断有效数据长度合法性
 				if(data_len + 8 >= LD_BUFFER_SIZE)
 				{
 					step = 0;	recvlenght = 0;
 				}
 
 				//判断是否接收完成
 				if(recvlenght >= 8 && recvlenght >= (data_len+8))
 				{
 					pcrc = crc16Get(&recvbuffer[1], 5 + data_len);
 					if(pcrc == (recvbuffer[6 + data_len]<<8)|(recvbuffer[7 + data_len]<<0))
 					{
 						ldRecvPars(devx, recvbuffer, recvlenght);
 					}
 
 					step = 0;	recvlenght = 0;	
 				}
 				break;
 				
 			default:
 				break;
 			}
 		}
 
 		//taskDelay(1);
		usleep(1000);
 	}
 
 	return;
}
 
 
// /*****************************************************************************
// *Name				:ldRecvProcess
// *Description		:终端数据接收解析
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *						:inbuffer		有效数据
// *						:nbtes			有效数据长度
// *						:seconds		等待超时时间，单位秒，0表示不等待返回
// *Output			:outbuffer	有效数据缓存
// *						:maxbytes	有效数据缓存大小
// *Return			:成功返回数据长度；失败返回-1；
// *History			:2016-08-29,modified by syj
// */
int ldRecvPars(int DEV_DSP_KEYx, char *inbuffer, int nbytes)
{
 	unsigned char pdst = 0, psrc = 0;
 	int pcommand = 0;
 	LDStructParamType *ldstruct = NULL;
 	LDButtonNodeStruct *pnode=NULL;
 
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			ldstruct = &ldStructA1;
 	else	if(DEV_DSP_KEYB == DEV_DSP_KEYx)	ldstruct = &ldStructB1;
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	if(nbytes > LD_BUFFER_SIZE)
 	{
 		jljRunLog("数据长度越界!\n");
 		return -1;
 	}
 
// 	/*根据命令字高位判断主板主动命令的返回还是终端模块主动发送的命令
// 	*	主板命令的返回则将数据拷贝到返回数据缓存
// 	*	终端模块主动命令则将数据拷贝到处理序列等待处理
// 	*/
 	pdst = inbuffer[1];
 	psrc = inbuffer[2];
 	pcommand = ( inbuffer[6] << 8 ) | ( inbuffer[7] );
 
 	if(0 == pcommand & 0xf000)
 	{
 		if(nbytes < LD_BUFFER_SIZE)	ldstruct->AckLenght = nbytes;
 		else												ldstruct->AckLenght = LD_BUFFER_SIZE;
 		memcpy(ldstruct->AckBuffer, inbuffer, nbytes);
 	}
 	else if(LD_CMD_BUTTON == pcommand)
 	{
// 		taskLock();
 
 		/*节点数大于10个则删除第一个*/
 		if(lstCount(&(ldstruct->ButtonList)) >= 10)
 		{
 			pnode = (LDButtonNodeStruct*)lstGet(&(ldstruct->ButtonList));
 			if(NULL != pnode)
 			{
 				free(pnode);
 			}
 		}
 
 		/*创建一个节点并添加到尾部*/
 		pnode = (LDButtonNodeStruct*)malloc(sizeof(LDButtonNodeStruct));
 		if(NULL != pnode)
 		{
 			pnode->Value = (inbuffer[8] << 24) | (inbuffer[9] << 16) | (inbuffer[10] << 8) | (inbuffer[11] << 0);
 			lstAdd(&(ldstruct->ButtonList), (NODE*)pnode);
 		}
 		
// 		taskUnlock();
 	}
 
 	return 0;
}
 
 
// /*****************************************************************************
// *Name				:ldSend
// *Description		:终端数据发送
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *						:inbuffer		有效数据
// *						:nbtes			有效数据长度
// *						:seconds		等待超时时间，单位秒，0表示不等待返回
// *Output			:outbuffer	有效数据缓存
// *						:maxbytes	有效数据缓存大小
// *Return			:成功返回数据长度；失败返回-1；
// *History			:2016-08-29,modified by syj
// */
int ldSend(int DEV_DSP_KEYx, char *inbuffer, int nbytes, char *outbuffer, int maxbytes, int seconds)
{
 	char tx_buffer[1024] = {0};
 	int tx_len = 0;
 	int tmp_data = 0;
 	int tmp_timer = 0;
 	int i = 0, j = 0;
 	unsigned char tmp_frame = 0;
 	LDStructParamType *ldstruct = NULL;
 
 	//判断设备
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	//判断有效数据长度
 	if(8 + nbytes >= 1024)
 	{
 		jljRunLog("发送的数据长度过长[nbytes = %x].\n", nbytes);
 		return -1;
 	}
	 
 	//组织数据发送
 	tx_buffer[tx_len++] = 0xfa;
 
 	tx_buffer[tx_len++] = 1;
 	
 	tx_buffer[tx_len++] = 0;
 	
 	tmp_frame = ldstruct->SendFrame;
 	tmp_frame++;
 	if(tmp_frame >= 0xff)	tmp_frame = 0;
 	ldstruct->SendFrame = tmp_frame;
 	tx_buffer[tx_len++] = tmp_frame;
 	
 	tmp_data = hex2Bcd(nbytes);								
 	tx_buffer[tx_len++] = (char)(tmp_data >> 8);	
 	tx_buffer[tx_len++] = (char)(tmp_data >> 0);
 	
 	memcpy(tx_buffer + tx_len, inbuffer, nbytes);
 	tx_len += nbytes;
 	
 	tmp_data = crc16Get(tx_buffer + 1, 5 + nbytes);
 	tx_buffer[tx_len++] = (char)(tmp_data >> 8);	
 	tx_buffer[tx_len++] = (char)(tmp_data >> 0);
 
 	//添加转义字符
 	for(i = 6; i < tx_len && i < 1024; i++)
 	{
 		if(0xfa == tx_buffer[i])
 		{
 			for(j = tx_len - 1; j > i; j--)
 			{
 				tx_buffer[j + 1] = tx_buffer[j];
 			}
 
 			tx_buffer[i + 1] = 0xfa;
 			i++;	tx_len++;
 		}
 	}
 
 	/*myDEBUG
 	printf("[%s][%d][%x]Send=", __FUNCTION__, __LINE__, DEV_DSP_KEYx);
 	for(i = 0; i < tx_len; i++)
 	{
 		printf(":%x", tx_buffer[i]);
 	}
 	printf("\n");
 	//*/
 
 	//发送数据
 	ldstruct->AckLenght = 0;
 	comWriteInTime(ldstruct->ComX, tx_buffer, tx_len, 1);
 
 	//不等待返回
 	if(0 == seconds)
 	{
 		return 0;
 	}
 
 	//等待返回数据，判断帧号一致时为有数据返回
 	while(1)
 	{
 		if(ldstruct->AckLenght > 0 && tmp_frame == ldstruct->AckBuffer[3])
 		{
 			tmp_data = ((((int)ldstruct->AckBuffer[4])>>4)&0x0f)*1000 + ((((int)ldstruct->AckBuffer[4])>>0)&0x0f)*100+\
 							((((int)ldstruct->AckBuffer[5])>>4)&0x0f)*10 + ((((int)ldstruct->AckBuffer[5])>>0)&0x0f)*1;
 
 			if(tmp_data > maxbytes)
 			{
 				jljRunLog("接收到的数据长度过大!\n");	
 				return -1;
 			}
 
 			memcpy(outbuffer, ldstruct->AckBuffer + 6, tmp_data);	
 			return tmp_data;
 		}
 
 		if(tmp_timer >= ( 1000 * seconds ))
 		{
 			break;
 		}
 
 		tmp_timer += 10;
 		//taskDelay(10 * ONE_MILLI_SECOND);
		usleep(10000);
 	}
 
 	return -1;
}

 
// /********************************************************************
// *Name				:ldDspContent
// *Description		:键盘显示,每行最多可显示8个汉字，最多显示四行
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *						:FONTx					字库类型选择	=1		16*8,16*16字库 
//        																					=2    	14*7,14*14字库 
// 	   																						=3   	8*8字库
// 	   																						=4    	12*6,12*12字库
// *						:IsContrary			是否反显0=正常显示；1=反显
// *						:Offsetx				行坐标0~7
// *						:Offsety				列坐标0~127
// *						:buffer					显示数据
// *						:nbytes				显示数据长度
// *Output			:None
// *Return			:0=成功；其它=错误
// *History			:2016-09-08,modified by syj
// */
int ldDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes)
{
 	LDStructParamType *ldstruct = NULL;
 	unsigned char poffset_x = 0;
 	int i = 0, j = 0;
 
 	//判断设备连接的串口
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	/*判断行坐标*/
 	if(Offsetx > 7)
 	{
 		return -1;
 	}
 
// 	/*将显示内容添加到缓存
// 	*
// 	*	注意以下说明!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 	*
// 	*	为了兼容金属键盘所以此处的输入一致
// 	*	联迪模块显示行号范围为1~4，此处许特殊处理
// 	*	联迪模块12*12字库每行显示十个字符，此处在每行前加2个空格以对齐操作
// 	*	联迪如果列坐标不为0则在其前方做补空格处理
// 	*
// 	*/
 	poffset_x = Offsetx / 2;
 	
 	ldstruct->Content.BitMap |= (1 << poffset_x);
 	ldstruct->Content.Font[poffset_x] = 0;
 	ldstruct->Content.IsContrary[poffset_x] = 0;
 	ldstruct->Content.OffsetY[poffset_x] = 1;
 
 	memset(ldstruct->Content.Buffer[poffset_x], 0, 32 + 1);
 	i = 0;
 	for(j = 0; j < 2 && i < 32; j++, i++)
 	{
 		ldstruct->Content.Buffer[poffset_x][i] = ' ';
 	}
 	for(j = 0; j < ( Offsety / 8 ) && i < 32; j++, i++)
 	{
 		ldstruct->Content.Buffer[poffset_x][i] = ' ';
 	}
 	for(j = 0; j < nbytes && i < 32; j++, i++)
 	{
 		ldstruct->Content.Buffer[poffset_x][i] = buffer[j];
 	}
 	ldstruct->Content.Nbytes[poffset_x] = strlen(ldstruct->Content.Buffer[poffset_x]);
 
 	return 0;
}
 
 
// /********************************************************************
// *Name				:ldDsp
// *Description		:终端显示，应首先调用lbDspContent添加显示内容后调用本函数
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *						:Contrast				对比度HEX格式
// *						:IsClr					是否清空全屏0=不清空；1=清空
// *Output			:None
// *Return			:0=成功；其它=失败
// *History			:2016-09-08,modified by syj
// */
int ldDsp(int DEV_DSP_KEYx, int Contrast, int IsClr)
{
 	LDStructParamType *ldstruct = NULL;
 	char tx_buffer[128] = {0};
 	int tx_len = 0;
 	char outbuffer[64] = {0};
 	int i = 0, plen = 0;
 
 	//判断设备连接的串口
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	/*显示数据组帧*/
 	tx_buffer[tx_len++] = (char)(LD_CMD_DSP >> 8);
 	tx_buffer[tx_len++] = (char)(LD_CMD_DSP >> 0);
 	tx_buffer[tx_len++] = IsClr;
 	tx_buffer[tx_len++] = ldstruct->Content.BitMap;
 	for(i=0; i<8; i++)
 	{
 		if(1 == ((ldstruct->Content.BitMap >> i) & 1))
 		{
 			tx_buffer[tx_len++] = ldstruct->Content.Font[i];
 			tx_buffer[tx_len++] = ldstruct->Content.IsContrary[i];
			tx_buffer[tx_len++] = ldstruct->Content.OffsetY[i];
 			tx_buffer[tx_len++] = ldstruct->Content.Nbytes[i];
 			memcpy(&tx_buffer[tx_len], ldstruct->Content.Buffer[i], ldstruct->Content.Nbytes[i]);
 			tx_len += ldstruct->Content.Nbytes[i];
 		}
 	}
 
// 	/*清除数据*/
 	ldstruct->Content.BitMap = 0;
 	for(i=0; i<8; i++)
 	{
 		ldstruct->Content.Font[i] = 0;
 		ldstruct->Content.IsContrary[i] = 0;
 		ldstruct->Content.OffsetY[i] = 0;
 		ldstruct->Content.Nbytes[i] = 0;
 	}
 
// 	/*发送显示数据，不等待返回*/
 	ldSend(DEV_DSP_KEYx, tx_buffer, tx_len, outbuffer, 64, 0);
 
 	return 0;
}
 
 
 
// /********************************************************************
// *Name				:ldButtonRead
// *Description		:键盘按键读取
// *Input				:kb_dev_id	设备选择	DEV_BUTTON_KEYA=A1键盘	DEV_BUTTON_KEYB=B1键盘
// *Output			:None
// *Return			:按键值，无按键时返回KB_BUTTON_NO
// *History			:2016-09-08,modified by syj
// */
unsigned int ldButtonRead(int kb_dev_id) 
{
 	unsigned int button=KB_BUTTON_NO, value=0;
 	LDButtonNodeStruct *pnode=NULL;
 
 	//判断按键设备
 	if(DEV_BUTTON_KEYA == kb_dev_id)			pnode = (LDButtonNodeStruct*)lstGet(&ldStructA1.ButtonList);
 	else if(DEV_BUTTON_KEYB == kb_dev_id)	pnode = (LDButtonNodeStruct*)lstGet(&ldStructB1.ButtonList);
 	else																	return KB_BUTTON_NO;
 
    //判断是否有按键
 	if(NULL != pnode)
 	{
 		value = pnode->Value;
 		free(pnode);	
	}
 	else
 	{
 		return KB_BUTTON_NO;
 	}
 
 	//解析按键
 	switch(value)
 	{
 	case 0xffffffff:		//无按键值
 		button=KB_BUTTON_NO;
 		break;
 	case 0x00000031:		// 1
 		button=KB_BUTTON_1;
 		break;
 	case 0x00000034:		// 4
 		button=KB_BUTTON_4;
 		break;
 	case 0x00000037:		// 7
 		button=KB_BUTTON_7;
 		break;
 	case 0x00000045:		// 凑整/.
 		button=KB_BUTTON_CZ;
 		break;
 	case 0x00000061:		// 设置
 		button=KB_BUTTON_SET;
 		break;
 	case 0x00000032:		// 2
 		button=KB_BUTTON_2;
 		break;
 	case 0x00000035:		// 5
 		button=KB_BUTTON_5;
 		break;
 	case 0x00000038:		// 8
 		button=KB_BUTTON_8;
 		break;
 	case 0x00000030:		// 0
 		button=KB_BUTTON_0;
 		break;
 	case 0x00000007:		// 上
 		button=KB_BUTTON_UP;
 		break;
 	case 0x00000033:		// 3
 		button=KB_BUTTON_3;
 		break;
 	case 0x00000036:		// 6
 		button=KB_BUTTON_6;
 		break;
 	case 0x00000039:		// 9
 		button=KB_BUTTON_9;
 		break;
 	case 0x0000000D:		// 确认
 		button=KB_BUTTON_ACK;
 		break;
 	case 0x00000008:		// 下
 		button=KB_BUTTON_DOWN;
 		break;
 	case 0x00000043:		// 定金额
 		button=KB_BUTTON_MON;
 		break;
 	case 0x00000044:		// 定升数
 		button=KB_BUTTON_VOL;
 		break;
 	case 0x0000001B:		// 退卡
 		button=KB_BUTTON_BACK; 
 		break;
 	case 0x00000065:		// 更改
 		button=KB_BUTTON_CHG;
 		break;
 	case 0x00000046:		// 选择
 		button=KB_BUTTON_SEL;
 		break;
 	case 0x00008031:		// 设置+1
 		button=KB_BUTTON_SET1;
 		break;
 	case 0x00008032:		// 设置+2
 		button=KB_BUTTON_SET2;
 		break;
 	case 0x00008033:		// 设置+3
 		button=KB_BUTTON_SET3;
 		break;
 	case 0x00008034:		// 设置+4
 		button=KB_BUTTON_SET4;
 		break;
 	case 0x00008035:		// 设置+5
 		button=KB_BUTTON_SET5;
 		break;
 	case 0x00008036:		// 设置+6
 		button=KB_BUTTON_SET6;
 		break;
 	case 0x00008037:		// 设置+7
 		button=KB_BUTTON_SET7;
 		break;
 	case 0x00008038:		// 设置+8
 		button=KB_BUTTON_SET8;
 		break;
 	case 0x00008039:		// 设置+9
 		button=KB_BUTTON_SET9;
 		break;
 
 	case 0x00000001:		//YP1
 		button = KB_BUTTON_YP1;
 		break;
 	case 0x00000002:		//YP2
 		button = KB_BUTTON_YP2;
 		break;
 	case 0x00000003:		//YP3
 		button = KB_BUTTON_YP3;
 		break;
 
 	default:
 		button=KB_BUTTON_NO;
 		break;
 	}
 
 	return button;
}
 
 
// /********************************************************************
// *Name				:ldICPowerUp
// *Description		:加油IC卡上电
// *Input				:nozzle			枪选0=A1枪，1=B1枪
// *						:dev_x			操作设备号0x00=卡座; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *Output			:无
// *Return			:成功返回0；失败返回其它值；
// *History			:2016-09-08,modified by syj
// */
int ldICPowerUp(int nozzle, unsigned char dev_x)
{
 	unsigned char tx_buffer[64] = {0};
 	int tx_len = 0;
 	unsigned char rx_buffer[64] = {0};
 	int istate = 0;
 	int dev_dsp_keyx = 0;
 
 	//判断设备连接的串口
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -2;
 	}
 
 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_POWER_UP >> 8);
 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_POWER_UP >> 0);
 	tx_buffer[tx_len++] = dev_x;
 	istate = ldSend(dev_dsp_keyx, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer), 3);
 	if(istate > 0 && 0 == ((rx_buffer[2] << 8) | (rx_buffer[3] << 0)) && dev_x == rx_buffer[4])
 	{
 		return 0;
 	}
 
 	return -1;
}
 
 
// /********************************************************************
// *Name				:ldICPowerDown
// *Description		:加油IC卡下电
// *Input				:nozzle			枪选0=A1枪，1=B1枪
// *						:dev_x			操作设备号0x00=卡座; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *Output			:无
// *Return			:成功返回0；失败返回其它值；
// *History			:2016-09-08,modified by syj
// */
// int ldICPowerDown(int nozzle, unsigned char dev_x)
// {
// 	unsigned char tx_buffer[64] = {0};
// 	int tx_len = 0;
// 	unsigned char rx_buffer[64] = {0};
// 	int istate = 0;
// 	int dev_dsp_keyx = 0;
// 
// 	//判断设备连接的串口
// 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
// 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
// 	else
// 	{
// 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
// 		return -2;
// 	}
// 
// 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_POWER_DOWN >> 8);
// 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_POWER_DOWN >> 0);
// 	tx_buffer[tx_len++] = dev_x;
// 	istate = ldSend(dev_dsp_keyx, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer), 3);
// 	if(istate > 0 && 0 == ((rx_buffer[2] << 8) | (rx_buffer[3] << 0)) && dev_x == rx_buffer[4])
// 	{
// 		return 0;
// 	}
// 
// 	return -1;
// }
// 
// 
// /********************************************************************
// *Name				:ldICExchangAPDU
// *Description		:加油IC卡APDU数据通信
// *Input				:nozzle			枪选0=A1枪，1=B1枪
// *						:dev_x			操作设备号0x00=卡座; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *						:inbuffer		输入缓存(APDU长度+APDU数据)
// *						:nbytes		输入缓存数据长度
// *						:maxbytes	输出缓存大小
// *Output			:outbuffer	输出缓存(APDU长度+APDU数据)
// *Return			:成功返回0；失败返回其它值；
// *History			:2016-09-08,modified by syj
// */
int ldICExchangAPDU(int nozzle, unsigned char dev_x, char *inbuffer, int nbytes, char *outbuffer, int maxbytes)
{
 	unsigned char tx_buffer[256] = {0};
 	int tx_len = 0;
 	unsigned char rx_buffer[64] = {0};
 	int apdulen = 0;
 	int istate = 0;
 	int dev_dsp_keyx = 0;
 
 	//判断设备连接的串口
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -2;
 	}
 
 	//判断长度
 	if(4 + nbytes > 256)
 	{
 		return -3;
 	}
 
 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_APDU >> 8);
 	tx_buffer[tx_len++] = (char)(LD_CMD_IC_APDU >> 0);
 	tx_buffer[tx_len++] = (char)(nbytes >> 8);
 	tx_buffer[tx_len++] = (char)(nbytes >> 0);
 	tx_buffer[tx_len++] = dev_x;
 	memcpy(tx_buffer + tx_len, inbuffer, nbytes);
 	istate = ldSend(dev_dsp_keyx, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer), 3);
 	if(istate > 0 && 0 == ((rx_buffer[2] << 8) | (rx_buffer[3] << 0)) && dev_x == rx_buffer[4])
 	{
 		apdulen = (rx_buffer[5] << 8) | (rx_buffer[6] << 0);
 		if(apdulen + 2 < maxbytes)	memcpy(outbuffer, rx_buffer + 5,  apdulen + 2);
 		else											memcpy(outbuffer, rx_buffer + 5,  maxbytes);
 		return 0;
 	}
 	
 	return -1;
}
 
 
// /********************************************************************
// *Name				:ldESEjectCard
// *Description		:退卡
// *Input				:nozzle			枪选0=A1枪，1=B1枪
// *Output			:None
// *Return			:成功返回0；失败返回其它值；
// *History			:2016-09-08,modified by syj
// */
int ldESEjectCard(int nozzle)
{
 	unsigned char tx_buffer[64] = {0};
 	int tx_len = 0;
 	unsigned char rx_buffer[64] = {0};
 	int istate = 0;
 	int iresult = 0;
 	int dev_dsp_keyx = 0;
 
 	//判断设备连接的串口
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	tx_buffer[tx_len++] = (char)(LD_CMD_ESEJECT_CARD >> 8);
 	tx_buffer[tx_len++] = (char)(LD_CMD_ESEJECT_CARD >> 0);
 	istate = ldSend(dev_dsp_keyx, tx_buffer, tx_len, rx_buffer, sizeof(rx_buffer), 3);
 	if(-1 == istate)
 	{
 		return ERROR;
 	}
 	iresult = (rx_buffer[2] << 8) | (rx_buffer[3] << 0);
 	if(0 != iresult)
 	{
 		return ERROR;
 	}
 
 	return 0;
}
 
 
// /********************************************************************
// *Name				:ldICStateRead
// *Description		:读取卡座状态信息接口
// *Input				:nozzle			枪选0=A1枪，1=B1枪
// *Output			:outbuffer	S1卡座状态(1byte) + S2卡类型(1byte) + S3卡状态(1byte)
// 											//S1卡座状态=0x30表示有卡,0x31表示无卡。
// 											//S2:卡类型
// 											//S2=0x30 卡机内无卡
// 											//S2=0x3f 无法识别
// 											//S2=0x31 触点cpu卡
// 											//S2=0x32 RF-- TYPE B CPU卡
// 											//S2=0x33 RF-TYPE A CPU卡
// 											//S2=0x34 RF-M1卡
// 											//S3：卡状态
// 											//S3=0x30 下电状态
// 											//S3=0x31 休眠状态
// 											//S4=0x32 激活状态
// 											//S5=0x33 忙态（通信状态）
// *Return			:成功返回0；失败返回其它值
// *History			:2016-09-08,modified by syj
// */
// int ldICStateRead(int nozzle, char *outbuffer)
// {
// 	LDStructParamType *ldstruct = NULL;
// 
// 	//判断设备连接的串口
// 	if(0 == nozzle)			{ldstruct = &ldStructA1;}
// 	else if(1 == nozzle)	{ldstruct = &ldStructB1;}
// 	else
// 	{
// 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
// 		return -1;
// 	}
// 
// 	taskLock();
// 	outbuffer[0] = ldstruct->DeckStateS1;
// 	outbuffer[1] = ldstruct->IcTypeS2;
// 	outbuffer[2] = ldstruct->IcStateS3;
// 	taskUnlock();
// 
// 	return 0;
// }
// 
// 
// /********************************************************************
// *Name				:ldBankPayment
// *Description		:银行交易接口
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *						:pSend			发送的DATA数据域
// *						:nSendLen	发送的DATA数据域长度
// *Output			:pRec			返回的DATA数据域
// *						:pnRecLen	返回的DATA数据域长度
// *Return			:成功返回0；失败返回其它值
// *History			:2016-09-08,modified by syj
// */
int ldBankPayment(int DEV_DSP_KEYx, const void *pSend, unsigned int nSendLen, void *pRec, int *pnRecLen)
{
 	#define I_BUFFER_SIZE 1024
 	LDStructParamType *ldstruct = NULL;
 	char tx_buffer[I_BUFFER_SIZE + 1] = {0};
 	int tx_len = 0;
 	char rx_buffer[I_BUFFER_SIZE + 1] = {0};
 	int rx_len = 0;
 
 	//判断设备连接的串口
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
 		return -1;
 	}
 
 	tx_buffer[0] = (char)(LD_CMD_BANK_PAYMENT >> 8);	tx_buffer[1] = (char)(LD_CMD_BANK_PAYMENT);
 	if((nSendLen + 2) < I_BUFFER_SIZE)
 	{
 		memcpy(tx_buffer + 2, pSend, nSendLen);
 		tx_len = 2 + nSendLen;
 	}
 	else
 	{
 		memcpy(tx_buffer + 2, pSend, I_BUFFER_SIZE - 2);
 		tx_len = 2 + I_BUFFER_SIZE - 2;
 	}
 	rx_len = ldSend(DEV_DSP_KEYx, tx_buffer, tx_len, rx_buffer, I_BUFFER_SIZE, 0);
 	if(rx_len < 0)
 	{
 		return -1;
 	}
 	if(*pnRecLen >= 4)
 	{
 		*pnRecLen = rx_len - 2 - 2;
 		memcpy(pRec, rx_buffer + 4, *pnRecLen);
 	}
 
 	return 0;
}
 
 
// /*****************************************************************************
// *Name				:ldInit
// *Description		:主板与终端设备通讯功能初始化
// *Input				:DEV_DSP_KEYx	设备选择	DEV_DSP_KEYA=A1键盘	DEV_DSP_KEYB=B1键盘
// *Output			:无
// *Return			:成功返回0；失败返回其它值；
// *History			:2016-08-29,modified by syj
// */

int ldInit(int DEV_DSP_KEYx)
{
	int tid = 0;
	char tmp_buffer[64] = {0};
	
	//LDStructParamType *ldstruct  = NULL;

	//判断设备连接的串口
	/*if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
	else
	{
		jljRunLog("[Function:%s]设备选择错误!\n", __FUNCTION__);
		return -1;
	}

	//通讯串口
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	ldstruct->ComX = COM7;
	else																ldstruct->ComX = COM8;

	//创建按键链表，用以保存接收到的按键
	lstInit(&ldstruct->ButtonList);

	//创建数据接收任务
	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	strcpy(tmp_buffer, "tLDRecv");
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	strcpy(tmp_buffer + strlen(tmp_buffer), "A1");
	else																strcpy(tmp_buffer + strlen(tmp_buffer), "B1");
	tid = taskSpawn(tmp_buffer, 155, 0, 0x2000, (FUNCPTR)tLdRecv, DEV_DSP_KEYx,1,2,3,4,5,6,7,8,9);
	if(OK != taskIdVerify(tid))	printf("Error!	Creat task '%s' failed!\n", tmp_buffer);

	//创建数据轮询任务
	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	strcpy(tmp_buffer, "tLDPoll");
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	strcpy(tmp_buffer + strlen(tmp_buffer), "A1");
	else																strcpy(tmp_buffer + strlen(tmp_buffer), "B1");
	tid = taskSpawn(tmp_buffer, 155, 0, 0x2000, (FUNCPTR)tLdPoll, DEV_DSP_KEYx,1,2,3,4,5,6,7,8,9);
	if(OK != taskIdVerify(tid))	printf("Error!	Creat task '%s' failed!\n", tmp_buffer);*/


	return 0;
}


