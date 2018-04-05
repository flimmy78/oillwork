//#include "../inc/main.h"
//#include "../inc/oilCfg.h"
//#include "../inc/oilDsp.h"
//#include "../inc/oilKb.h"
//#include <pthread.h>

//del #include "oilTerminalDevice.h"
//del #include "oilCom.h"
//del #include "oilPC.h"

#include "../inc/main.h"

//yym 下面内容定义在“oilCom.h”文件中
#define COM7		DEV_COM7						//COM7(ARM7_UART1)	7	备用
#define COM8		DEV_COM8						//COM8(ARM7_UART2)	8	备用
//yym 下面内容定义在“oilPC.h”文件中
#define PC_FUN_GRADE_OK				0x30			/*启用油品确认*/
#define PC_FUN_GRADE_NO				0x31			/*禁用油品确认*/
//yym 下面内容定义在“oilTerminalDevice.h”文件中
//yym 备注：最好把buffer的数据类型从unsigned char*改成char*
//int tdDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, char *buffer, int nbytes) {return 0;};
//int tdDsp(int DEV_DSP_KEYx, int Contrast, int IsClr) {return 0;};



/*显示封装数据节点定义*/
typedef struct
{
	NODE	Ndptrs;
	unsigned char buffer[DSP_LEN_MAX];
	int len;
}DspDataNodeType;

/*显示参数数据结构*/
typedef struct
{
	/*显示器选择0=键盘点阵屏；1=android平板显示屏*/
	unsigned char DEV;
	/*键盘点阵屏显示设备号*/
	int DEVKeyx;
	/*平板显示通讯串口*/
	int comFd;
	/*前一次显示的界面ID号*/
	int interfaceID;

	/*显示发送数据缓存*/
	unsigned char txBuffer[DSP_LEN_MAX];
	/*显示发送数据长度*/
	int txLen;
	/*显示发送标识*/
	unsigned char txMark;
	/*数据发送帧号*/
	unsigned char txFrame;
	/*数据发送操作信号量*/
	//del SEM_ID semIdTx;
	pthread_mutex_t semIdTx;
	
	/*接收数据缓存*/
	unsigned char rxBuffer[DSP_LEN_MAX];
	/*接收数据长度*/
	int rxLen;
	/*接收完整标识*/
	unsigned char rxMark;

	/*显示接收任务ID*/
	int tIdDspRx;
	/*显示任务ID*/
	int tIdDsp;
	
	/*显示对比度*/
	int Contrast;
}DspParamStructType;


/*显示功能参数*/
static DspParamStructType DspParamA, DspParamB;

/*显示空闲时间计数器*/
unsigned int dspTimerA=0, dspTimerB=0;


/********************************************************************
*Name				:tdspRecive
*Description		:显示数据通讯接收
*Input				:nozzle	面板号
*Output			:None
*Return				:None
*History			:2014-10-10,modified by syj
*/
static void tdspRecive(int nozzle)
{
	unsigned char rxbuffer[16]={0};
	int rxlen=0;
	int datalen=0, crc=0;
	DspParamStructType *param=NULL;

	/*判断枪选*/
	if(0==nozzle)		param=&DspParamA;
	else if(1==nozzle)	param=&DspParamB;
	else				return;

	while ( 1 )
	{
		/*读取并根据协议缓存数据*/
		rxlen=comRead(param->comFd, (char*)rxbuffer, 1);
//printf("__%x\n", rxbuffer[0]);
		if(rxlen>0)
		{
			/*长度溢出则清零*/
			if(param->rxLen>=DSP_LEN_MAX)	param->rxLen=0;

			/*缓存并校验数据*/
			param->rxBuffer[param->rxLen]=rxbuffer[0];
			if((0==param->rxLen)&&(0xfa==rxbuffer[0]))	param->rxLen++;
			else	if(0!=param->rxLen)										param->rxLen++;

			datalen=(param->rxBuffer[1]<<8)|(param->rxBuffer[2]<<0);
			if((param->rxLen>=8)&&(param->rxLen>=(5+datalen)))
			{
		
				crc=(param->rxBuffer[3+datalen]<<8)|(param->rxBuffer[4+datalen]<<0);
				if(crc==crc16Get(&param->rxBuffer[1], 2+datalen))
				{
					param->rxMark=1;
				}
				else
				{
					param->rxLen=0;	param->rxMark=0;
				}
			}
		}
		
		usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:tdsp
*Description		:显示数据通讯
*Input				:nozzle	枪选0=1号；1=2号
*Output			:None
*Return				:None
*History			:2014-10-10,modified by syj
*/
static void tdsp(int nozzle)
{
	DspParamStructType *dsp=NULL;
	unsigned int *timer=NULL;

	/*判断枪选*/
	if(0==nozzle)			{dsp=&DspParamA;	timer=&dspTimerA;}
	else if(1==nozzle)	{dsp=&DspParamB;	timer=&dspTimerB;}
	else							return;

	while ( 1 )
	{
		/*新的发送会清空时间，刷新显示应有一定间隔*/
		if(*timer>=ONE_SECOND && dsp->txLen>0 && DSP_DEV_PCMONITOR==dsp->DEV)
		{
			//semTake(dsp->semIdTx, WAIT_FOREVER);
			
			dsp->rxLen=0;	dsp->rxMark=0;	*timer=0;	
			comWrite(dsp->comFd, (char*)dsp->txBuffer, dsp->txLen);
			
			//semGive(dsp->semIdTx);
		}
		
		usleep(5*1000);
	}
	
	return;
}

/********************************************************************
*Name				:dspVideo
*Description		:播放视频
*Input				:nozzle	枪选0=1号；1=2号
*Output			:None
*Return				:成功返回0；其它表示失败
*History			:2014-10-10,modified by syj
*/
int dspVideo(int nozzle)
{
	DspParamStructType *dsp=NULL;
	unsigned int *timer=NULL;
	unsigned char tx_buffer[DSP_LEN_MAX]={0};
	unsigned int crc=0, tx_len=0;

	/*判断面板号选择*/
	if(0==nozzle)		{dsp=&DspParamA;	timer=&dspTimerA;}
	else if(0==nozzle)	{dsp=&DspParamB;	timer=&dspTimerB;}
	else				return 0;//yym 增加返回0，原始无返回值

	/*判断帧号不为0xfa*/
	dsp->txFrame++;	if(0xfa==dsp->txFrame)	dsp->txFrame++;	

	tx_buffer[0]=0xfa;
	tx_buffer[1]=(unsigned char)(4>>8);
	tx_buffer[2]=(unsigned char)(4>>0);
	tx_buffer[3]=dsp->txFrame;
	tx_buffer[4]=0x52;
	tx_buffer[5]=0;
	tx_buffer[6]=0;
	crc=crc16Get(&tx_buffer[1], 6);
	tx_buffer[7]=(unsigned char)(crc>>8);
	tx_buffer[8]=(unsigned char)(crc>>0);
	tx_len=9;
	comWrite(dsp->comFd, (char*)tx_buffer, tx_len);

	/*发送间隔定时器清零并将数据保存为最后一笔发送数据*/
	//semTake(dsp->semIdTx, WAIT_FOREVER);
	
	*timer=0;
	memcpy(dsp->txBuffer, tx_buffer, tx_len);	dsp->txLen=tx_len;
	
	//semGive(dsp->semIdTx);

	return 0;
}


/********************************************************************
*Name				:dsp
*Description		:显示，该函数对执行结果不做确认，由底层函数负责数据执行的成功
*Input				:nozzle	枪选0=1号；1=2号
*						:id			显示界面ID
*						:buffer	显示参数
*						:nbytes	显示参数长度
*Output			:None
*Return				:成功返回0；其它表示失败
*History			:2014-10-10,modified by syj
*/
int dsp(int nozzle, int id, unsigned char *buffer, int nbytes)
{
	DspParamStructType *dsp=NULL;
	unsigned char tmp_buffer[128]={0};
	unsigned int crc=0, *timer=NULL;
	int i=0, offset_x=0, offset_y=0, temp_len = 0;

	/*键盘点阵屏显示时需要的参数*/
	unsigned char dsp_buffer[128]={0};				/*分行显示数据*/
	unsigned char dsp_line_size[4]={0};				/*各行显示数据长度*/
	unsigned int x=0, y=0, dsp_len=0;					/*显示光标位置*/
	unsigned long long bcd_value=0, hex_value=0;

	/*判断枪选*/
	if(DSP_NOZZLE_1==nozzle)			{dsp=&DspParamA;	timer=&dspTimerA;	}
	else if(DSP_NOZZLE_2==nozzle)		{dsp=&DspParamB;	timer=&dspTimerB;	}
	else								return 0;//yym 增加返回0，原始无返回值

//myDEBUG
//	printf("dsp_______");
//	for(i=0; i<nbytes; i++)	printf("_%x", buffer[i]);
//	printf("\n");

	/*
	*键盘点阵屏显示
	*根据界面号组织各行显示内容，一般首次显示时清屏非首次时不清屏
	*处理中在不清屏的情况下以空格覆盖不再显示可变区域
	*/
	if(DSP_DEV_KEYBOARD==dsp->DEV)
	{
		switch(id)
		{
		case DSP_TEXT_INFO:									
			/*
			*	文本显示界面
			*	文本显示以16*16字库显示，每行最多显示8个汉字长度，
			*	共4个汉字行，从屏幕起始显示位置起依次显示
			*	不超过1行时在中间位置居中显示，不超过2行时从第二行开始显示
			*/
			memset(dsp_line_size, 0, 4);	x=0;	y=0;
			if(nbytes<=16)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, (16-nbytes)/2*8, (char*)buffer, nbytes);
			}
			else
			{
				/*不超过2行显示从第2行开始显示*/
				if(nbytes<=32)			x=1;

				/*将显示数据按行进行封装*/
				for(i=0; i<nbytes;)
				{
					/*如果是字母,保存字母并将坐标y后移一位*/
					if(buffer[i]<128)
					{
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*如果是汉字,如果高位在行尾不够显示则转入下一行显示*/
					else
					{
						if(y>=15)	{x++;	y=0;}
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
						dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*如果y坐标超过1行则转入下一行*/
					if(y>=16)	{x++;	y=0;}
					/*如果x坐标超过4行则不再显示剩余部分*/
					if(x>=4)	break;
				}
				
				/*根据每行的显示长度判断是否显示*/
				for(i=0; i<4; i++)
				{
					if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
				}
			}
			
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_STANDBY:				
			/*
			*	加油卡加油待机界面
			*	本界面首次显示则刷屏并显示全部信息
			*	本界面非首次显示仅显示可变区域
			*/
			/*第0行显示:联网状态+时间，中间空与部分填充空格*/
			if(1==buffer[7])	memcpy(&dsp_buffer[0], "佳", 2);
			else						memcpy(&dsp_buffer[0], "  ", 2);
			memcpy(&dsp_buffer[2], "      ", 6);
			hex2Ascii(&buffer[0], 7, tmp_buffer, 14);
			dsp_buffer[8]=tmp_buffer[8];		dsp_buffer[9]=tmp_buffer[9];
			dsp_buffer[10]=':';
			dsp_buffer[11]=tmp_buffer[10];		dsp_buffer[12]=tmp_buffer[11];
			dsp_buffer[13]=':';
			dsp_buffer[14]=tmp_buffer[12];	dsp_buffer[15]=tmp_buffer[13];
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第3行显示:"加油机已下班"或"请插入IC卡"或"请插入IC卡或选择油品后扫描条码"*/
#if _TYPE_BIG260_
			if(0==buffer[8])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   加油机下班   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else	if(0!=buffer[8] && 0==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "   请插入IC卡   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "按'↓'键启动加油", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "按确认键结束加油", 16);
			}
#else
			if(0==buffer[8])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   加油机下班   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else	if(0!=buffer[8] && 0==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   请插入IC卡   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "                ", 16);
			}
			else if(0!=buffer[8] && 2==buffer[9])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "请插入IC卡或选择", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "油品后扫描条码  ", 16);
			}
#endif
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PRETREAT:							
			/*加油卡预处理提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油卡处理中", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, "请稍候....", 10);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_UNLOCK_FINISH:				
			/*加油卡补扣处理提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "灰卡，补扣处理中", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_TACCLR_FINISH:
			/*加油卡补充处理提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "加油卡补充处理中", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PASSIN:								
			/*加油卡密码输入界面*/
			/*第2行:显示"请输入卡密码:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入卡密码:", 13);
			/*第4行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_CARLIMIT:							
			/*限车号卡员工密码输入界面*/
			/*第0行:显示"限车号卡"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "限车号卡:", 9);
			/*第2行:显示车号*/
			for(i=0; i<16; i++)
			{
				if(0xff==buffer[0+i])	break;
			}
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)&buffer[0], i);
			/*第4行:显示"请输入员工密码:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "请输入员工密码:", 15);
			/*第6行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[16]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_BALANCE:							
			/*加油卡余额界面*/
			/*第0行:显示优惠价格*/
			if(0 != memcmp(buffer + 16, "\x00\x00", 2))
			{
				memset(dsp_buffer, 0, sizeof(dsp_buffer));
				strcpy((char*)dsp_buffer, "优惠价格:");
				bcd_value=hex2Bcd((buffer[16]<<8)|(buffer[17]<<0));
				tmp_buffer[0]=(char)((bcd_value>>12)&0x0f)+0x30;	tmp_buffer[1]=(char)((bcd_value>>8)&0x0f)+0x30;	
				tmp_buffer[2]=(char)((bcd_value>>4)&0x0f)+0x30;	tmp_buffer[3]=(char)((bcd_value>>0)&0x0f)+0x30;
				if('0' == tmp_buffer[0])	*(dsp_buffer + strlen((char*)dsp_buffer)) = ' ';
				else						*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[0];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[1];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = '.';
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[2];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[3];
				strcpy((char*)dsp_buffer + strlen((char*)dsp_buffer), "元");
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
			}
			/*第2行:显示对应卡类型余额*/
			if(0xff == buffer[18] && 1 == buffer[2])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡余额:", strlen("用户卡余额:"));
			else if(0xff == buffer[18] && 2 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "管理卡余额:", strlen("管理卡余额:"));
			else if(0xff == buffer[18] && 4 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡余额:", strlen("员工卡余额:"));
			else if(0xff == buffer[18] && 5 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "验泵卡余额:", strlen("验泵卡余额:"));
			else if(0xff == buffer[18] && 6 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "维修卡余额:", strlen("维修卡余额:"));
			else if(0x01 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡余额:", strlen("用户卡余额:"));
			else if(0x04 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡余额:", strlen("员工卡余额:"));
			else if(0x05 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "验泵卡余额:", strlen("验泵卡余额:"));
			else if(0x21 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡主卡余额:", strlen("用户卡主卡余额:"));
			else if(0x22 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡主卡余额:", strlen("代储卡主卡余额:"));
			else if(0x11 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡司机卡余额", strlen("用户卡司机卡余额"));
			else if(0x12 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "记账卡余额:", strlen("记账卡余额:"));
			else if(0x13 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡余额:", strlen("代储卡余额:"));
			else if(0x14 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "自用卡余额:", strlen("自用卡余额:"));
			else if(0x15 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "调拨卡余额:", strlen("调拨卡余额:"));
			else if(0x16 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "过表卡余额:", strlen("过表卡余额:"));
			else if(0x17 == buffer[18])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡司机卡余额", strlen("代储卡司机卡余额"));
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油卡余额:", strlen("加油卡余额:"));
			/*第4行:显示卡余额*/
			bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[14])				memcpy(&dsp_buffer[11], "元", 2);
			else if(1==buffer[14])		memcpy(&dsp_buffer[11], "点", 2);
			else									memcpy(&dsp_buffer[11], "升", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);
			/*第6行:显示结算方式，只有员工卡才有此内容显示*/
			if(4==buffer[2] && 0==buffer[15])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "现金", 4);
			else if(4==buffer[2] && 1==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "油票", 4);
			else if(4==buffer[2] && 2==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "提油凭证", 8);
			else if(4==buffer[2] && 3==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "银行卡", 6);
			else if(4==buffer[2] && 4==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "其它一", 6);
			else if(4==buffer[2] && 5==buffer[15])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "其它二", 6);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_PRESET:								
			/*加油卡加油预置界面*/
			/*第2行:显示"请输入预置量:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入预置量:", 13);
			/*第4行:显示预置量及单位*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])		memcpy(&dsp_buffer[11], "元", 2);
			else if(1==buffer[4])	memcpy(&dsp_buffer[11], "升", 2);
			else					memcpy(&dsp_buffer[11], "点", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_UNIT_SELECT	:
			/*加油卡结算单位选择界面*/
			/*第2行:显示"1.电子油票"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "1.电子油票", 10);
			/*第4行:显示"2.积分应用"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, "2.积分应用", 10);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_SETTLE_SELECT:				
			/*加油卡结算方式选择界面*/
			/*第0行:显示"1.现金  5.其它一"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.现金  5.其它一", 16);
			/*第2行:显示"2.油票  6.其它二"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.油票  6.其它二", 16);
			/*第4行:显示"3.提油凭证"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.提油凭证", 10);
			/*第6行:显示"4.银行卡"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.银行卡", 8);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_STAFF_PASSIN:					
			/*加油卡加油员工密码输入界面*/
			/*第4行:显示"请输入员工密码:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入员工密码:", 15);
			/*第6行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_OIL_START:						
			/*加油卡加油启动提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油启动中...", 13);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 8*8, "请勿拔卡", 8);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_OILLING:							
			/*加油卡加油中界面*/
			/*第0行:显示预置量*/
			memcpy(&dsp_buffer[0], "预置:", 5);
			bcd_value=hex2Bcd((buffer[15]<<24)|(buffer[16]<<16)|(buffer[17]<<8)|(buffer[18]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[19])			memcpy(&dsp_buffer[14], "元", 2);
			else if(1==buffer[19])		memcpy(&dsp_buffer[14], "升", 2);
			else if(2==buffer[19])		memcpy(&dsp_buffer[14], "点", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, 16);
			/*第2行:显示对应卡类型余额*/
			//if(1==buffer[2])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡余额:", 11);
			//else if(2==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "管理卡余额:", 11);
			//else if(4==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡余额:", 11);
			//else if(5==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "验泵卡余额:", 11);
			//else if(6==buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "维修卡余额:", 11);
			//else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油卡余额:", 11);
			if(0xff == buffer[23] && 1 == buffer[2])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡余额:", strlen("用户卡余额:"));
			else if(0xff == buffer[23] && 2 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "管理卡余额:", strlen("管理卡余额:"));
			else if(0xff == buffer[23] && 4 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡余额:", strlen("员工卡余额:"));
			else if(0xff == buffer[23] && 5 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "验泵卡余额:", strlen("验泵卡余额:"));
			else if(0xff == buffer[23] && 6 == buffer[2])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "维修卡余额:", strlen("维修卡余额:"));
			else if(0x01 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡余额:", strlen("用户卡余额:"));
			else if(0x04 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡余额:", strlen("员工卡余额:"));
			else if(0x05 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "验泵卡余额:", strlen("验泵卡余额:"));
			else if(0x21 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡主卡余额:", strlen("用户卡主卡余额:"));
			else if(0x22 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡主卡余额:", strlen("代储卡主卡余额:"));
			else if(0x11 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "用户卡司机卡余额", strlen("用户卡司机卡余额"));
			else if(0x12 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "记账卡余额:", strlen("记账卡余额:"));
			else if(0x13 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡余额:", strlen("代储卡余额:"));
			else if(0x14 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "自用卡余额:", strlen("自用卡余额:"));
			else if(0x15 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "调拨卡余额:", strlen("调拨卡余额:"));
			else if(0x16 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "过表卡余额:", strlen("过表卡余额:"));
			else if(0x17 == buffer[23])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "代储卡司机卡余额", strlen("代储卡司机卡余额"));
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油卡余额:", strlen("加油卡余额:"));
			/*第4行:显示卡余额*/
			bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;		
			if(1==buffer[14])		memcpy(&dsp_buffer[11], "点", 2);
			else if(2==buffer[14])	memcpy(&dsp_buffer[11], "升", 2);
			else					memcpy(&dsp_buffer[11], "元", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);
			/*第6行:显示是否凑整*/
#if _TYPE_BIG260_
			if(1==buffer[20])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "凑整            ", 16);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "按确认键结束加油", 16);
#else
			if(1==buffer[20])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "凑整            ", 16);
			else if(0 != memcmp(buffer + 21, "\x00\x00", 2))
			{
				memset(dsp_buffer, 0, sizeof(dsp_buffer));
				strcpy((char*)dsp_buffer, "优惠价格:");
				bcd_value=hex2Bcd((buffer[21]<<8)|(buffer[22]<<0));
				tmp_buffer[0]=(char)((bcd_value>>12)&0x0f)+0x30;	tmp_buffer[1]=(char)((bcd_value>>8)&0x0f)+0x30;	
				tmp_buffer[2]=(char)((bcd_value>>4)&0x0f)+0x30;	tmp_buffer[3]=(char)((bcd_value>>0)&0x0f)+0x30;
				if('0' == tmp_buffer[0])	*(dsp_buffer + strlen((char*)dsp_buffer)) = ' ';
				else									*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[0];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[1];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = '.';
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[2];
				*(dsp_buffer + strlen((char*)dsp_buffer)) = tmp_buffer[3];
				strcpy((char*)dsp_buffer + strlen((char*)dsp_buffer), "元");
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
			}
#endif

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);			
			break;
			
		case DSP_CARD_OIL_FINISH:						
			/*加油卡加油结束提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油结束中...", 13);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 8*8, "请勿拔卡", 8);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_OILEND:
			/*加油卡加油完成界面*/
			/*第0行:显示"加油完成"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "加油完成", 8);
			/*第2行:显示"余额:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "余额:", 5);
			/*第4行:显示卡余额*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(1==buffer[4])			memcpy(&dsp_buffer[11], "点", 2);
			else	if(2==buffer[4])	memcpy(&dsp_buffer[11], "升", 2);
			else								memcpy(&dsp_buffer[11], "元", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_CARD_ERR_INFO:
			/*加油卡加油含代码的错误提示界面*/
			if(buffer[2]<=16)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, (16-buffer[2])/2*8, (char*)&buffer[3], buffer[2]);
			else
			{
				/*将显示数据按行进行封装*/
				for(i=0, x=0; i<buffer[2] && i<30;)
				{
					/*如果是字母,保存字母并将坐标y后移一位*/
					if(buffer[3+i]<128)
					{
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*如果是汉字,如果高位在行尾不够显示则转入下一行显示*/
					else
					{
						if(y>=15)	x++;
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
						dsp_buffer[x*16+y]=buffer[3+i];		y++;	i++;	dsp_line_size[x]++;
					}
					/*如果y坐标超过1行则转入下一行*/
					if(y>=16)	{x++;	y=0;}
				}
				
				/*根据每行的显示长度判断是否显示*/
				for(i=0; i<2; i++)
				{
					if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
				}
			}

			/*第4行显示*/
			hex2Ascii(&buffer[0], 2, dsp_buffer, 4);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_PASSWORD_INPUT:
			/*密码输入界面*/
			/*第2行:显示"请输入密码:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入密码:", 11);
			/*第4行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OPERATE_SELECT:
			/*操作项选择界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, "1.查询", 6);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, "2.设置", 6);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_SELECT:
			/*查询项选择界面，共5页*/
			if(0==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.总累", 6);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.加油明细", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.重打印", 8);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.油枪信息", 10);
			}
			else if(1==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "5.油机信息", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "6.时间", 6);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "7.语音信息", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "8.打印信息", 10);
			}
			else if(2==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "9.税务累计", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "10.出厂累计", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "11.首检累计", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "12.税务当次数据", 15);
			}
			else if(3==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "13.税务时间", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "14.税务月累", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "15.税务日累", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "16.限制信息", 11);
			}
			else if(4==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "17.提前量", 9);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "18.超时停机时间", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "19.绑定信息", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "20.机型信息", 11);
			}
			else if(5==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "21.打印自助小票", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "22.油品语音", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "23.异常日志", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "24.对比度查询", 13);
			}
			else if(6==buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "25.条码模块品牌", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "26.本地网络信息", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "27.油机后台信息", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "28.平板电脑信息", 15);
			}

			/*翻页显示的界面因为覆盖前一页界面所以清屏显示*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_JLSUM:
			/*查询计量总累界面*/
			/*第0行:显示"总累金额:"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "总累金额:", 9);
			
			/*第2行:显示总累金额*/
			hex_value=((long long)buffer[0]<<56)|((long long)buffer[1]<<48)|((long long)buffer[2]<<40)|\
								((long long)buffer[3]<<32)|((long long)buffer[4]<<24)|((long long)buffer[5]<<16)|\
								((long long)buffer[6]<<8)|((long long)buffer[7]<<0);
			/*计算BCD码，最大计算13位，为'.'及"元"预留显示空间*/
			for(i=0; i<13; i++)
			{
				dsp_buffer[12-i]=(char)(hex_value%10)+0x30;
				hex_value=hex_value/10;
			}
			/*填充小数点及单位显示*/
			memcpy(&dsp_buffer[14], "元", 2);
			dsp_buffer[13]=dsp_buffer[12];	
			dsp_buffer[12]=dsp_buffer[11];
			dsp_buffer[11]='.';
			/*高位连续的0不显示*/
			for(i=0; i<10; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示"总累升数:"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "总累升数:", 9);

			/*第6行:显示总累升数*/
			hex_value=((long long)buffer[8]<<56)|((long long)buffer[9]<<48)|((long long)buffer[10]<<40)|\
								((long long)buffer[11]<<32)|((long long)buffer[12]<<24)|((long long)buffer[13]<<16)|\
								((long long)buffer[14]<<8)|((long long)buffer[15]<<0);
			/*计算BCD码，最大计算13位，为'.'及"升"预留显示空间*/
			for(i=0; i<13; i++)
			{
				dsp_buffer[12-i]=(char)(hex_value%10)+0x30;
				hex_value=hex_value/10;
			}
			/*填充小数点及单位显示*/
			memcpy(&dsp_buffer[14], "升", 2);
			dsp_buffer[13]=dsp_buffer[12];	
			dsp_buffer[12]=dsp_buffer[11];
			dsp_buffer[11]='.';
			/*高位连续的0不显示*/
			for(i=0; i<10; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_SELECT:
			/*查询账单明细操作对象选择界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 4*8, "1.本油机", 8);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, "2.本枪", 6);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_TTCINPUT:
			/*查询账单明细TTC输入界面*/
			/*第2行:显示"请输入TTC:"*/
			if(id!=dsp->interfaceID)tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入TTC:", 10);

			/*第4行:显示TTC号*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<=10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_INDEX:
			/*查询账单明细索引界面*/
			/*第0行:显示TTC号*/
			memcpy(&dsp_buffer[0], "TTC:", 4);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			for(i=0; i<10; i++)
			{
				dsp_buffer[4+i]=(char)((bcd_value>>(9*4-i*4))&0x0f)+0x30;
			}
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[4+i])	dsp_buffer[4+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第2行:显示时间*/
			memcpy(&dsp_buffer[0], "T:", 2);	hex2Ascii(&buffer[4], 7, &dsp_buffer[2], 14);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示卡号后16位*/
			hex2Ascii(&buffer[11], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示加油金额+物理枪号*/
			bcd_value=hex2Bcd((buffer[19]<<16)|(buffer[20]<<8)|(buffer[21]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "元", 2);

			memset(&dsp_buffer[11], ' ', 3);

			hex2Ascii(&buffer[22], 1, &dsp_buffer[14], 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BILL_DATA:
			/*查询账单明细原始数据界面*/
			if(buffer[0]<4){
				/*前四页每页显示三行明细，末行显示分页数*/
				hex2Ascii(&buffer[1+8*0+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				hex2Ascii(&buffer[1+8*1+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				hex2Ascii(&buffer[1+8*2+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				dsp_buffer[0]=buffer[0]+1+0x30;	dsp_buffer[1]='/';		dsp_buffer[2]='5';
				dsp_len=3;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 13*8, (char*)dsp_buffer, dsp_len);
			}
			else if(4==buffer[0]){
				/*第5页即最后一页显示一行明细，末行显示分页数*/
				hex2Ascii(&buffer[1+8*0+24*buffer[0]], 8, dsp_buffer, 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				memset(dsp_buffer, ' ', 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				memset(dsp_buffer, ' ', 16);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				dsp_buffer[0]=buffer[0]+1+0x30;	dsp_buffer[1]='/';		dsp_buffer[2]='5';
				dsp_len=3;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 13*8, (char*)dsp_buffer, dsp_len);
			}
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_NOZZLE_INFO:
			/*查询油枪信息界面*/
			if(0==buffer[0]){
				
				/*第0行:显示逻辑枪号*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "逻辑枪号:", 9);
				bcd_value=hex2Bcd(buffer[1]);
				dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
				for(i=0; i<2; i++)
				{
					if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
					else									break;
				}
				dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				/*第2行:显示物理枪号*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "物理枪号:", 9);
				bcd_value=hex2Bcd(buffer[2]);
				dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
				for(i=0; i<2; i++)
				{
					if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
					else									break;
				}
				dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				/*第4行:显示面板号*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "  面板号:", 9);
				if(0==buffer[3])			memcpy(&dsp_buffer[9], "  1(A)", 6);
				else if(1==buffer[3])	memcpy(&dsp_buffer[9], "  2(B)", 6);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				/*第6行:显示向下箭头*/
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "翻页↓", 6);
			}
			else{

				/*第2行:显示油品代码*/
				memset(dsp_buffer, ' ', 16);
				memcpy(&dsp_buffer[0], "油品代码:", 9);
				hex2Ascii(&buffer[4], 2, &dsp_buffer[9], 4);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				/*第4行:显示当前总计TTC*/
				memcpy(&dsp_buffer[0], "TTC:  ", 6);
				bcd_value=hex2Bcd((buffer[6]<<24)|(buffer[7]<<16)|(buffer[8]<<8)|(buffer[9]<<0));
				for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
				for(i=0; i<9; i++)
				{
					if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
					else									break;
				}
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				/*第6行:显示未上传账单*/
				memcpy(&dsp_buffer[0], "待传: ", 6);
				bcd_value=hex2Bcd((buffer[10]<<24)|(buffer[11]<<16)|(buffer[12]<<8)|(buffer[13]<<0));
				for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
				for(i=0; i<9; i++)
				{
					if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
					else									break;
				}
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				/*第6行:显示向上箭头*/
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "翻页↑", 6);
			}

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BOARD_INFO:
			/*主板信息界面*/
			/*第0行:显示主板号*/
			memcpy(&dsp_buffer[0], "主板号:      ", 13);	
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[13]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[14]=(char)((bcd_value>>4)&0x0f)+0x30;
			if('0'==dsp_buffer[13] && '0'!=dsp_buffer[14])			{dsp_buffer[13]=' ';}
			else if('0'==dsp_buffer[13] && '0'==dsp_buffer[14])	{dsp_buffer[13]=' ';	dsp_buffer[14]=' ';}
			dsp_buffer[15]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第2行:显示当前总计TTC*/
			memcpy(&dsp_buffer[0], "TTC:  ", 6);
			bcd_value=hex2Bcd((buffer[1]<<24)|(buffer[2]<<16)|(buffer[3]<<8)|(buffer[4]<<0));
			for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示未上传账单*/
			memcpy(&dsp_buffer[0], "待传: ", 6);
			bcd_value=hex2Bcd((buffer[5]<<24)|(buffer[6]<<16)|(buffer[7]<<8)|(buffer[8]<<0));
			for(i=0; i<10; i++)	dsp_buffer[6+i]=(char)((bcd_value>>((9-i)*4))&0x0f)+0x30;
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[6+i])	dsp_buffer[6+i]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行显示上下键提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "更多信息请按↑↓", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_TIME:
			/*时间界面*/
			/*第2行:显示日期*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示时间*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_buffer[5]=':';
			dsp_buffer[6]=tmp_buffer[12];	dsp_buffer[7]=tmp_buffer[13];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VOICE:
			/*查询语音信息界面*/
			/*第0行:显示扬声器设置*/
			memcpy(&dsp_buffer[0], "  扬声器:", 9);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[9], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[9], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[9], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[9], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[9], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[9], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[9], "      ", 6);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第2行:显示语音类型*/
			memcpy(&dsp_buffer[0], "语音类型:", 9);
			if(0==buffer[1])			memcpy(&dsp_buffer[9], "女声", 4);
			else if(1==buffer[1])	memcpy(&dsp_buffer[9], "男声", 4);
			else								memcpy(&dsp_buffer[9], "    ", 4);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示语音音量*/
			memcpy(&dsp_buffer[0], "语音音量:", 9);
			bcd_value=hex2Bcd(buffer[2]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
				else									break;
			}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;	
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT:
			/*查询打印信息界面*/
			/*第0行:显示打印机*/
			memcpy(&dsp_buffer[0], "  打印机:", 9);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[9], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[9], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[9], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[9], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[9], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[9], "3-2(B)", 6);
			else						memcpy(&dsp_buffer[9], "      ", 6);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第2行:显示是否自动打印*/
			memcpy(&dsp_buffer[0], "自动打印:", 9);
			if(0==buffer[1])		memcpy(&dsp_buffer[9], "否", 2);
			else if(1==buffer[1])	memcpy(&dsp_buffer[9], "是", 2);
			else					memcpy(&dsp_buffer[9], "  ", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示打印联数*/
			memcpy(&dsp_buffer[0], "打印联数:", 9);
			if(0==buffer[2])		memcpy(&dsp_buffer[9], "1联", 3);
			else if(1==buffer[2])	memcpy(&dsp_buffer[9], "2联", 3);
			else					memcpy(&dsp_buffer[9], "   ", 3);
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示按确认键查询更多*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 10*8, "确认键", 6);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT_CARD:
			/*查询打印信息卡类型选择界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "请选择卡类型", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "1.用户卡4.验泵卡", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "2.管理卡5.维修卡", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "3.员工卡", 8);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_PRINT_BILLTYPE:
			/*查询打印信息卡类型选择界面*/
			hex_value=(buffer[1]<<8)|(buffer[2]<<0);
			if(0==buffer[0])
			{
				/*第一页前三行显示账单类型，末尾提示向下翻页*/
				if(0==((hex_value>>0)&1))			memcpy(&dsp_buffer[0], "正常    ", 8);
				else if(1==((hex_value>>0)&1))		memcpy(&dsp_buffer[0], "正常√  ", 8);
				if(0==((hex_value>>1)&1))			memcpy(&dsp_buffer[8], "逃卡    ", 8);
				else if(1==((hex_value>>1)&1))		memcpy(&dsp_buffer[8], "逃卡√  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
				
				if(0==((hex_value>>2)&1))			memcpy(&dsp_buffer[0], "错卡    ", 8);
				else if(1==((hex_value>>2)&1))		memcpy(&dsp_buffer[0], "错卡√  ", 8);
				if(0==((hex_value>>3)&1))			memcpy(&dsp_buffer[8], "补扣    ", 8);
				else if(1==((hex_value>>3)&1))		memcpy(&dsp_buffer[8], "补扣√  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);
				
				if(0==((hex_value>>4)&1))			memcpy(&dsp_buffer[0], "补充    ", 8);
				else if(1==((hex_value>>4)&1))		memcpy(&dsp_buffer[0], "补充√  ", 8);
				if(0==((hex_value>>5)&1))			memcpy(&dsp_buffer[8], "上班    ", 8);
				else if(1==((hex_value>>5)&1))		memcpy(&dsp_buffer[8], "上班√  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 14*8, "↓", 2);
			}
			else
			{
				/*第二页前三行显示账单类型，末尾提示向上翻页*/
				if(0==((hex_value>>6)&1))			memcpy(&dsp_buffer[0], "下班    ", 8);
				else if(1==((hex_value>>6)&1))		memcpy(&dsp_buffer[0], "下班√  ", 8);
				if(0==((hex_value>>7)&1))			memcpy(&dsp_buffer[8], "非卡    ", 8);
				else if(1==((hex_value>>7)&1))		memcpy(&dsp_buffer[8], "非卡√  ", 8);
				dsp_len=16;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

				if(0==((hex_value>>8)&1))			memcpy(&dsp_buffer[0], "油价接收√", 10);
				else if(1==((hex_value>>8)&1))		memcpy(&dsp_buffer[0], "油价接收√", 10);
				dsp_len=10;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

				if(0==((hex_value>>9)&1))			memcpy(&dsp_buffer[0], "卡错拒绝√", 10);
				else if(1==((hex_value>>9)&1))		memcpy(&dsp_buffer[0], "卡错拒绝√", 10);
				dsp_len=10;
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 14*8, "↑", 2);
			}

			/*翻页显示的界面因为覆盖前一页界面所以清屏显示*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_MONTH_INPUT:
			/*月份输入界面*/
			/*第2行:显示请输入月份*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "请输入月份", 10);

			/*第4行:显示月份*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_DATE_INPUT:
			/*日期输入界面*/
			/*第0行:显示请输入日期*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "请输入日期", 10);

			/*第2行:显示日期*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_LIMIT_INFO:
			/*查询限制信息界面*/
			/*第0行:显示员工卡是否禁止*/
			memcpy(&dsp_buffer[0], "员工卡加油:", 11);
			if(0==buffer[0])			memcpy(&dsp_buffer[11], "允许", 4);
			else	if(1==buffer[0])	memcpy(&dsp_buffer[11], "禁止", 4);
			else						memcpy(&dsp_buffer[11], "    ", 4);
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ADVANCE_INFO:
			/*查询提前量界面*/
			/*第3行:显示提前量*/
			memcpy(&dsp_buffer[0], "提前量:", 7);
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;	
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;	
			memcpy(&dsp_buffer[11], "升", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_UNPULSE_TIME:
			/*查询无脉冲停机超时时间界面*/
			/*第2行:显示"超时停机时间"文本*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "无脉冲停机时间", 14);

			/*第4行:显示超时停机时间*/
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;	
			memcpy(&dsp_buffer[3], "秒", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BIND_INFO:
			/*查询绑定信息界面*/
			/*第0行:显示绑定时间*/
			dsp_buffer[0]='T';	dsp_buffer[1]=':';
			hex2Ascii(&buffer[0], 7, &dsp_buffer[2], 14);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*第2行:显示芯片ID*/
			hex2Ascii(&buffer[7], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*第4行:显示ACT卡号*/
			hex2Ascii(&buffer[15], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*第4行:显示IRD卡号*/
			hex2Ascii(&buffer[23], 8, &dsp_buffer[0], 16);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VERSION_INFO:
			/*查询版本信息界面*/
			/*第0行:显示计量版本号*/
			memcpy(&dsp_buffer[0], "JL: ", 4);		memcpy(&dsp_buffer[4], &buffer[0], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);
			
			/*第2行:显示IPT版本号*/
			memcpy(&dsp_buffer[0], "IPT:", 4);		memcpy(&dsp_buffer[4], &buffer[12], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示计量版本号*/
			memcpy(&dsp_buffer[0], "PCD:", 4);	memcpy(&dsp_buffer[4], &buffer[24], 12);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_SELECT:
			/*查询项选择界面，共5页*/
			if(0==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.单价设置", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.时间设置", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.首次检定", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.出厂检定", 10);
			}
			else if(1==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "5.背光设置", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "6.夜间锁定", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "7.操作密码设置", 14);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "8.物理枪号设置", 14);
			}
			else if(2==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "9.税务时间", 10);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "10.扬声器选择", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "11.语音音量", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "12.语音类型", 11);
			}
			else if(3==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "13.打印机选择", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "14.自动打印设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "15.打印联设置", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "16.自动打印类型", 15);
			}
			else if(4==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "17.提前量设置", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "18.超时停机时间", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "19.员工卡加油", 13);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "20.模式设置", 11);
			}
			else if(5==buffer[0]){
				
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "21.油品语音设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "22.油机枪数设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "23.条码品牌设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "24.后台联网方式", 15);
			}
			else if(6==buffer[0]){

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "25.本地网络设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "26.后台网络设置", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "27.平板电脑信息", 15);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "28.对比度设置", 13);
			}
			
			/*翻页显示的界面因为覆盖前一页界面所以清屏显示*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRICE:
			/*设置单价界面*/
			/*第2行:显示请输入单价*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入单价:", 11);

			/*第4行:显示单价*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>12)&0x0f)+0x30;	
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[3]='.';
			dsp_buffer[4]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[6], "元/升", 5);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_TIME:
			/*设置时间界面*/
			/*第0行:显示请输入时间*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "请输入时间", 10);

			/*第2行:显示日期*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示时间*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];	
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_buffer[5]=':';
			dsp_buffer[6]=tmp_buffer[12];	dsp_buffer[7]=tmp_buffer[13];
			dsp_len=8;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_BACKLIT:
			/*设置背光界面*/
			/*第2行:显示背光设置*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "背光设置:常亮", 13);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "背光设置:关闭", 13);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "背光设置:省电", 13);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "背光设置:    ", 13);

			/*第6行:显示上下键提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_NIGHTLOCK:
			/*设置夜间锁定*/
			/*第2行:显示是否夜间锁定*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "夜间锁定:无  ", 13);
			else if(1==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "夜间锁定:锁定", 13);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "夜间锁定:    ", 13);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_OLD:
			/*设置夜间锁定*/
			/*第2行:显示请输入旧密码*/
			memcpy(dsp_buffer, "请输入旧密码:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示输入的密码*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_NEW:
			/*设置夜间锁定*/
			/*第2行:显示请输入新密码*/
			memcpy(dsp_buffer, "请输入新密码:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示输入的密码*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PASSWORD_ACK:
			/*设置夜间锁定*/
			/*第2行:显示请确认新密码*/
			memcpy(dsp_buffer, "请确认新密码:", 13);	dsp_len=13;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示输入的密码*/
			memset(dsp_buffer, ' ', 12);
			for(i=0; i<buffer[0] && i<12; i++)	dsp_buffer[11-i]='*';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 4*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PHYSICAL_NOZZLE:
			/*设置物理枪号*/
			/*第3行:显示输入的枪号*/
			memcpy(&dsp_buffer[0], "物理枪号:", 9);
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[9+i])	dsp_buffer[9+i]=' ';
				else									break;
			}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 2*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_TAX_TIME:
			/*设置税控时间*/
			/*第0行:显示请输入时间*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, "请输入时间", 10);

			/*第2行:显示日期*/
			hex2Ascii(buffer, 7, tmp_buffer, 14);
			dsp_buffer[0]=tmp_buffer[0];	dsp_buffer[1]=tmp_buffer[1];
			dsp_buffer[2]=tmp_buffer[2];	dsp_buffer[3]=tmp_buffer[3];	
			dsp_buffer[4]='-';
			dsp_buffer[5]=tmp_buffer[4];	dsp_buffer[6]=tmp_buffer[5];
			dsp_buffer[7]='-';
			dsp_buffer[8]=tmp_buffer[6];	dsp_buffer[9]=tmp_buffer[7];
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示时间*/
			dsp_buffer[0]=tmp_buffer[8];	dsp_buffer[1]=tmp_buffer[9];	
			dsp_buffer[2]=':';
			dsp_buffer[3]=tmp_buffer[10];	dsp_buffer[4]=tmp_buffer[11];
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_SPEAKER:
			/*设置扬声器*/
			/*第2行:显示扬声器选择*/
			memcpy(&dsp_buffer[0], "扬声器:", 7);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[7], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[7], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[7], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[7], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[7], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[7], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[7], "      ", 6);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_VOICE_VOLUME:
			/*设置语音音量*/
			/*第3行:显示输入的枪号*/
			memcpy(&dsp_buffer[0], "请输入音量:", 11);
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[11+i])	dsp_buffer[11+i]=' ';
				else									break;
			}
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_VOICE_TYPE:
			/*设置语音类型*/
			/*第2行:显示语音类型选择*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "语音类型:女声", 13);
			else	if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "语音类型:男声", 13);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 1*8, "语音类型:    ", 13);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINTER:
			/*设置打印机*/
			/*第2行:显示打印机选择*/
			memcpy(&dsp_buffer[0], "打印机:", 7);
			if(0x10==buffer[0])			memcpy(&dsp_buffer[7], "1-1(A)", 6);
			else if(0x11==buffer[0])	memcpy(&dsp_buffer[7], "1-2(B)", 6);
			else if(0x20==buffer[0])	memcpy(&dsp_buffer[7], "2-1(A)", 6);
			else if(0x21==buffer[0])	memcpy(&dsp_buffer[7], "2-2(B)", 6);
			else if(0x30==buffer[0])	memcpy(&dsp_buffer[7], "3-1(A)", 6);
			else if(0x31==buffer[0])	memcpy(&dsp_buffer[7], "3-2(B)", 6);
			else									memcpy(&dsp_buffer[7], "      ", 6);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_AUTO:
			/*设置是否自动打印*/
			/*第2行:显示是否自动打印*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "自动打印:否", 11);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "自动打印:是", 11);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "自动打印:  ", 11);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_UNION:
			/*设置自动打印联数*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "打印联数:1联", 12);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "打印联数:2联", 12);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 2*8, "打印联数:   ", 12);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_CARD:
			/*设置自动打印卡类型选择界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "请选择卡类型", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "1.用户卡4.验泵卡", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "2.管理卡5.维修卡", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "3.员工卡", 8);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_PRINT_BILLTYPE:
			/*设置自动打印账单类型界面*/
			hex_value=(buffer[1]<<8)|(buffer[2]<<0);
			if(0==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "正常账单:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "正常账单:√", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "逃卡账单:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "逃卡账单:√", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "错卡账单:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "错卡账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ↓", 4);
			}
			else if(1==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "正常账单:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "正常账单:√", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "逃卡账单:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "逃卡账单:√", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "错卡账单:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "错卡账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ↓", 4);
			}
			else if(2==buffer[0])
			{
				if(0==((hex_value>>0)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "正常账单:  ", 11);
				else if(1==((hex_value>>0)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "正常账单:√", 11);

				if(0==((hex_value>>1)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "逃卡账单:  ", 11);
				else if(1==((hex_value>>1)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "逃卡账单:√", 11);

				if(0==((hex_value>>2)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "错卡账单:  ", 11);
				else if(1==((hex_value>>2)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "错卡账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ↓", 4);
			}
			else if(3==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "补扣账单:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "补扣账单:√", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "补充账单:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "补充账单:√", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "上班账单:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "上班账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(4==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "补扣账单:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "补扣账单:√", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "补充账单:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "补充账单:√", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "上班账单:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "上班账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(5==buffer[0])
			{
				if(0==((hex_value>>3)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "补扣账单:  ", 11);
				else if(1==((hex_value>>3)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "补扣账单:√", 11);

				if(0==((hex_value>>4)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "补充账单:  ", 11);
				else if(1==((hex_value>>4)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "补充账单:√", 11);

				if(0==((hex_value>>5)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "上班账单:  ", 11);
				else if(1==((hex_value>>5)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "上班账单:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(6==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "下班账单:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "下班账单:√", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "非卡账单:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "非卡账单:√", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "油价接收:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "油价接收:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(7==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "下班账单:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "下班账单:√", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "非卡账单:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 2, 0*8, "非卡账单:√", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "油价接收:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "油价接收:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(8==buffer[0])
			{
				if(0==((hex_value>>6)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "下班账单:  ", 11);
				else if(1==((hex_value>>6)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "下班账单:√", 11);

				if(0==((hex_value>>7)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "非卡账单:  ", 11);
				else if(1==((hex_value>>7)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "非卡账单:√", 11);

				if(0==((hex_value>>8)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "油价接收:  ", 11);
				else if(1==((hex_value>>8)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 4, 0*8, "油价接收:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else
			{
				if(0==((hex_value>>9)&1))		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "卡错拒绝:  ", 11);
				else if(1==((hex_value>>9)&1))	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY, 0, 0*8, "卡错拒绝:√", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "           ", 11);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "           ", 11);

				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "  ↑", 4);
			}

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_ADVANCE:
			/*设置提前量界面*/
			/*第2行:显示请输入提前量*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入提前量:", 13);

			/*第4行:显示提前量*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[3]='.';
			dsp_buffer[4]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[6], "升", 2);
			dsp_len=9;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_UNPULSE_TIME:
			/*设置无脉冲超时停机时间界面*/
			/*第2行:显示无脉冲超时时间*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "无脉冲超时时间:", 15);

			/*第4行:显示无脉冲超时停机时间*/
			bcd_value=hex2Bcd((buffer[0]<<8)|(buffer[1]<<0));
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "秒", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 11*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_STAFF_LIMIT:
			/*设置员工卡加油限制界面*/
			/*第2行:显示员工卡加油限制*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡加油:允许", 15);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡加油:禁止", 15);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "员工卡加油:    ", 15);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_MODE:
			/*设置模式界面*/
			/*第2行:显示员工卡加油限制*/
			if(0==buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, " 卡机联动模式 ", 14);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "非卡机联动模式", 14);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, " 条码自助模式 ", 14);
			else					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 1*8, "              ", 14);

			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OTHER_OPERATE_PASSSIN:
			/*其它操作密码输入界面*/
			/*第0行:显示明文数据*/
			dsp_buffer[0]='C';	dsp_buffer[1]=':';
			dsp_buffer[2]=((buffer[0]>>4)&0x0f)+0x30;	dsp_buffer[3]=((buffer[0]>>0)&0x0f)+0x30;
			dsp_buffer[4]=((buffer[1]>>4)&0x0f)+0x30;	dsp_buffer[5]=((buffer[1]>>0)&0x0f)+0x30;
			dsp_buffer[6]=((buffer[2]>>4)&0x0f)+0x30;	dsp_buffer[7]=((buffer[2]>>0)&0x0f)+0x30;
			dsp_len=8;
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第2行:显示员工卡加油限制*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入密码:", 11);

			/*第4行:显示请输入密码*/
			memset(dsp_buffer, ' ', 6);	dsp_len=6;
			for(i=0; i<6 && i<buffer[3]; i++)	dsp_buffer[5-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 10*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OTHER_OPERATE:
			/*其它操作界面*/
			/*第2行:显示请输入操作代码*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入操作代码:", 15);

			/*第4行:显示输入的代码ASCII*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)buffer, 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_JLTYPE:
			/*计量机型显示界面*/
			/*第3行:显示计量机型*/
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "机型:普通      ", 15);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "机型:单枪大流量", 15);
			else if(2==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "机型:双枪大流量", 15);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "机型:          ", 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_SHILED:
			/*计量屏蔽量及过冲屏蔽量显示界面*/
			/*第0行:显示屏蔽量字样*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "屏蔽量:", 7);

			/*第2行:显示屏蔽量*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "升", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 3*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示过冲屏蔽量字样*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "过冲屏蔽量:", 11);

			/*第6行:显示过冲屏蔽量*/
			bcd_value=hex2Bcd((buffer[4]<<24)|(buffer[5]<<16)|(buffer[6]<<8)|(buffer[7]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "升", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 3*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_EQUIVALENT:
			/*显示计量当量界面*/
			/*第3行:显示过冲屏蔽量*/
			memcpy(&dsp_buffer[0], "当量:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>12)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>4)&0x0f)+0x30;	
			for(i=0; i<9; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[14]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=15;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_VALVE_VOLUME:
			/*显示大阀打开时最低油量*/
			/*第2行:显示大阀开启屏蔽量字样*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "大阀开启屏蔽量:", 15);
			
			/*第4行:显示大阀开启屏蔽量*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "升", 2);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_STANDBY:
			/*非卡机联动待机界面*/
			/*第0行显示:联网状态+时间，中间空与部分填充空格*/
			if(1==buffer[7])	memcpy(&dsp_buffer[0], "佳", 2);
			else						memcpy(&dsp_buffer[0], "  ", 2);
			memcpy(&dsp_buffer[2], "      ", 6);
			hex2Ascii(&buffer[0], 7, tmp_buffer, 14);
			dsp_buffer[8]=tmp_buffer[8];		dsp_buffer[9]=tmp_buffer[9];
			dsp_buffer[10]=':';
			dsp_buffer[11]=tmp_buffer[10];		dsp_buffer[12]=tmp_buffer[11];
			dsp_buffer[13]=':';
			dsp_buffer[14]=tmp_buffer[12];	dsp_buffer[15]=tmp_buffer[13];
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, (char*)dsp_buffer, dsp_len);

			/*第3行显示:"欢迎光临"*/
#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 4*8, "欢迎光临", 8);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "按'↓'键启动加油", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "按确认键结束加油", 16);
#else
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 4*8, "欢迎光临", 8);
#endif

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_PRESET:
			/*非卡机联动预置界面*/
			/*第2行:显示"请输入预置量:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入预置量:", 13);
			/*第4行:显示预置量及单位*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])	memcpy(&dsp_buffer[11], "元", 2);
			else						memcpy(&dsp_buffer[11], "升", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_UNSELF_OILLING:
			/*非卡机联动加油中界面*/
			/*第0行:显示"加油中"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "加油中", 6);
			/*第3行:显示金额*/
			memcpy(&dsp_buffer[0], "金额:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[14], "元", 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "按确认键结束加油", 16);
#endif

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_STATION_INFO1:
			/*油站通用信息界面，第一部分*/
			/*第0行:显示"通用信息"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "通用信息", 8);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "版本号:    ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示省代码*/
			hex2Ascii(&buffer[1], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "省代码:    ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示地市代码*/
			hex2Ascii(&buffer[2], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "地市代码:  ", 11);	memcpy(&dsp_buffer[11], tmp_buffer, 2);	dsp_buffer[13]='H';
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_STATION_INFO2:
			/*油站通用信息界面，第二部分*/
			/*第0行:显示"通用信息"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "通用信息", 8);

			/*第2行:显示上级单位代码*/
			hex2Ascii(&buffer[0], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "上级ID: ", 8);	memcpy(&dsp_buffer[8], tmp_buffer, 8);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示油站ID*/
			hex2Ascii(&buffer[4], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "油站ID: ", 8);	memcpy(&dsp_buffer[8], tmp_buffer, 8);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILINFO:
			/*油品油价表信息界面*/
			/*第0行:显示"油品信息"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "油品信息", 8);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "版本号:  ", 9);	memcpy(&dsp_buffer[9], tmp_buffer, 2);	dsp_buffer[11]='H';
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示油品代码*/
			memcpy(&dsp_buffer[0], "油品数目:", 9);
			bcd_value=hex2Bcd(buffer[1]);
			dsp_buffer[9]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>4)&0x0f)+0x30;
			if('0'==dsp_buffer[9] && '0'==dsp_buffer[10])			{dsp_buffer[9]=' ';	dsp_buffer[10]=' ';}
			else if('0'==dsp_buffer[9] && '0'!=dsp_buffer[10])	{dsp_buffer[9]=' ';}
			dsp_buffer[11]=(char)((bcd_value>>0)&0x0f)+0x30;
			dsp_len=12;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示生效时间*/
			hex2Ascii(&buffer[2], 6, tmp_buffer, 12);
			memcpy(&dsp_buffer[0], "T:", 2);	memcpy(&dsp_buffer[2], tmp_buffer, 12);
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BASELIST:
			/*基础黑名单信息界面*/
			/*第0行:显示"基础黑名单"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "基础黑名单", 10);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 2, tmp_buffer, 4);
			memcpy(&dsp_buffer[0], "版本:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 4);	dsp_buffer[9]='H';
			dsp_buffer[10]=':';
			bcd_value=hex2Bcd((buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[11]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[14]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[15]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+11])	dsp_buffer[i+11]=' ';
				else									break;
			}
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示生效时间*/
			hex2Ascii(&buffer[4], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "生效:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示截止时间*/
			hex2Ascii(&buffer[8], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "截止:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ADDLIST:
			/*新增黑名单信息界面*/
			/*第0行:显示"新增黑名单"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "新增黑名单", 10);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "版本:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示生效时间*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "生效:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示截止时间*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "截止:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_DELLIST:
			/*新删黑名单信息界面*/
			/*第0行:显示"新删黑名单"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "新删黑名单", 10);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "版本:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示生效时间*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "生效:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示截止时间*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "截止:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_WHITELIST:
			/*白名单信息界面*/
			/*第0行:显示"白名单"*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "白名单", 6);

			/*第2行:显示版本号*/
			hex2Ascii(&buffer[0], 1, tmp_buffer, 2);
			memcpy(&dsp_buffer[0], "版本:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 2);	dsp_buffer[7]='H';
			dsp_buffer[8]=':';
			bcd_value=hex2Bcd((buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[9]=(char)((bcd_value>>16)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>12)&0x0f)+0x30;
			dsp_buffer[11]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;
			dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			for(i=0; i<4; i++)
			{
				if('0'==dsp_buffer[i+9])	dsp_buffer[i+9]=' ';
				else									break;
			}
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示生效时间*/
			hex2Ascii(&buffer[3], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "生效:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示截止时间*/
			hex2Ascii(&buffer[7], 4, tmp_buffer, 8);
			memcpy(&dsp_buffer[0], "截止:", 5);	memcpy(&dsp_buffer[5], tmp_buffer, 8);
			dsp_len=13;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OIL_OVER_INFO:
			/*过冲加油信息界面*/
			/*第1行:显示加油过冲*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 1, 0*8, "加油量超过卡余额", 16);

			/*第3行:显示过冲金额字样*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "过冲金额:", 9);

			/*第5行:显示过冲金额*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "元", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_OIL_OVER_STAFFIN:
			/*过冲加油员工密码输入界面*/
			/*第0行:显示过冲金额字样*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "过冲金额:", 9);

			/*第2行:显示过冲金额*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "元", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, (char*)dsp_buffer, dsp_len);

			/*第4行:显示请输入员工密码*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "请输入员工密码:", 15);

			/*第6行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[3]; i++)	dsp_buffer[16-1-i]='*';
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_TEST:
			/*卡座电特性,协议等测试界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1-电单  2-电连续", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "3-协单  4-协连续", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "5-协连续测试时间", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "6-退出测试", 10);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_CARD_PROTOCOL_TIME:
			/*卡座协议连续测试轮询时间设置界面*/
			/*第2行:显示"请输入协议时间:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入协议时间:", 15);

			/*第4行:显示输入的时间*/
			bcd_value=hex2Bcd(buffer[0]);
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;
			for(i=0; i<2; i++){
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "秒", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 11*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_SCAN:
			/*请扫描或输入条码的提示界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请扫描条码，或按", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "确认键后输入条码", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_BARCODE_INPUT:
			/*条码输入界面*/
			/*第2行:显示"请输入条码"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入条码:", 11);

			/*第4行:显示输入的条码*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 6*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZING:
			/*条码授权申请中界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "授权申请中，", 12);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "请稍候...", 9);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZE:
			/*授权申请结果显示界面*/
			/*第2行:显示"授权金额:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "授权金额:", 9);

			/*第4行:显示授权金额*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[5]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[6]='.';
			dsp_buffer[7]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[9], "元", 2);
			dsp_len=11;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_AUTHORIZE_CANCEL:
			/*条码自助加油启动中*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "授权取消中，", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "请稍候...", 9);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILSTART:
			/*条码自助加油启动中*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油启动中，", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "请稍候...", 9);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILLING:
			/*条码自助加油中*/
			/*第0行:显示"加油中"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "加油中", 6);

			/*第2行:显示"授权余额:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "授权余额:", 9);

			/*第4行:显示授权余额*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "元", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

#if _TYPE_BIG260_
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "按确认键结束加油", 16);
#endif

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OILFINISH:
			/*条码自助加油结束中*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "加油结束中，", 10);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 7*8, "请稍候...", 9);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TM_OIL_FINAL:
			/*条码自助加油结果显示界面*/
			/*第0行:显示"加油结束"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "加油结束", 8);

			/*第2行:显示"授权余额:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "授权余额:", 9);

			/*第4行:显示授权余额*/
			bcd_value=hex2Bcd((buffer[0]<<16)|(buffer[1]<<8)|(buffer[2]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "元", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_UNPULSE_OVERTIME:	/*无脉冲超时关大阀时间界面*/
			/*第0行:显示无脉冲关大阀时间*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "无脉冲关大阀时间", 16);

			/*第4行:显示无脉冲关大阀时间，单位为秒*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>8)&0x0f)+0x30;	if('0'==dsp_buffer[0])	dsp_buffer[0]=' ';
			dsp_buffer[1]=(char)((bcd_value>>4)&0x0f)+0x30;	if(' '==dsp_buffer[0] && '0'==dsp_buffer[1])	dsp_buffer[1]=' ';
			dsp_buffer[2]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[3], "秒", 2);
			dsp_len=5;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_ISAUTHEN:	/*加油是否需要DES认证的界面*/
			/*第3行:显示是否需要认证*/
			memcpy(dsp_buffer, "是否DES认证:", 12);
			if(0==buffer[0])	memcpy(&dsp_buffer[12], "是", 2);
			else						memcpy(&dsp_buffer[12], "否", 2);
			dsp_len=14;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILVOICE:		/*油品语音查询界面*/
			/*第0行:显示油品语音的提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "油品语音", 8);

			/*第2~7行:显示油品语音文件名*/
			memset(dsp_buffer,  ' ', 16);
			for(i=0, offset_x=2, offset_y=0; ; )
			{
				/*字符保存一字节，汉字则保存两字节*/
				if(buffer[1+i]<128){
					dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
				}
				else{
					if(offset_y<15){
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
					}
					else{
						offset_y++;
					}
				}

				/*换行*/
				if(offset_y>=16){
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					memset(dsp_buffer,  ' ', 16);
					offset_x+=2;	offset_y=0;
				}

				/*数据操作溢出*/
				if(!(i<buffer[0] && offset_x<8)){
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					break;
				}
			}

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_OILVOICE_SELECT:		/*油品语音大项选择界面*/
			/*第0行:显示油品语音大项选择*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 5*8, "1、汽油", 7);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 5*8, "2、柴油", 7);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_OILVOICE:		/*油品语音选择界面*/
			/*第0行:显示油品语音的提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "请选择油品语音", 14);

			/*第2~7行:显示油品语音文件名*/
			memset(dsp_buffer,  ' ', 48);
			for(i=0, offset_x=2, offset_y=0; ; )
			{
				/*字符保存一字节，汉字则保存两字节*/
				if(buffer[1+i]<128)
				{
					dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
				}
				else
				{
					if(offset_y<15)
					{
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
						dsp_buffer[offset_y]=buffer[1+i];	offset_y++;	i++;
					}
					else
					{
						offset_y++;
					}
				}

				/*换行*/
				if(offset_y>=16)
				{
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					memset(dsp_buffer,  ' ', 16);
					offset_x+=2;	offset_y=0;
				}

				/*数据操作溢出*/
				if(!(i<buffer[0] && offset_x<8))
				{
					tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, offset_x, 0*8, (char*)dsp_buffer, strlen((char*)dsp_buffer));
					break;
				}
			}

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_OILERRLOG:	/*查询错误日志界面*/
			dsp_len=strlen((char*)buffer);
			memset(dsp_line_size, 0, 4);	x=0;	y=0;
			/*将显示数据按行进行封装*/
			for(i=0; i<dsp_len;)
			{
				/*如果是字母,保存字母并将坐标y后移一位*/
				if(buffer[i]<128)
				{
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
				}
				/*如果是汉字,如果高位在行尾不够显示则转入下一行显示*/
				else
				{
					if(y>=15){x++;	y=0;}
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
					dsp_buffer[x*16+y]=buffer[i];		y++;	i++;	dsp_line_size[x]++;
				}
				/*如果y坐标超过1行则转入下一行*/
				if(y>=16)	{x++;	y=0;}
				/*如果x坐标超过4行则不再显示剩余部分*/
				if(x>=4)	break;
			}

			/*根据每行的显示长度判断是否显示*/
			for(i=0; i<4; i++)
			{
				if(dsp_line_size[i]>0)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, i*2, 0, (char*)&dsp_buffer[i*16+0], dsp_line_size[i]);
			}

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BARGUNNUMBER:	/*条码枪数查询界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "条码单面枪数", 12);
			
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "单枪", 4);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "多枪", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_INQ_BARBRAND:	/*条码品牌查询界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "条码模块品牌", 12);
			
			if('1'==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "远景达二维模块  ", 16);
			else if('2'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "远景达一维LV1000", 16);
			else if('3'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "霍尼韦尔二维模块", 16);
			else if('4'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "霍尼韦尔IS4125  ", 16);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_NOZZLE_NUMBER:	/*单面枪数设置界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "油机枪数设置", strlen("油机枪数设置"));
			
			if(0==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "双枪机", 6);
			else if(1==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "四枪机", 6);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_SET_BARBRAND:	/*条码品牌设置界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "条码模块品牌设置", 16);
			
			if('1'==buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "远景达二维模块  ", 16);
			else if('2'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "远景达一维LV1000", 16);
			else if('3'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "霍尼韦尔二维模块", 16);
			else if('4'==buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "霍尼韦尔IS4125  ", 16);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_LOCAL_NETINFO:				/*本地网络信息项选择界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.IP地址", strlen("1.IP地址"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.子网掩码", strlen("2.子网掩码"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.默认网关", strlen("3.默认网关"));

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_IP:						/*本地IP地址界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "本地IP地址", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_MASK:					/*本地掩码界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "本地子网掩码", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_LOCAL_GATEWAY:			/*本地默认网关界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "本地默认网关", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_INFO:			/*石化后台配置信息*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.后台联网方式", strlen("1.后台联网方式"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.后台IP地址", strlen("2.后台IP地址"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.后台端口号", strlen("3.后台端口号"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "4.本地端口号", strlen("4.本地端口号"));

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_IP:				/*石化后台IP地址*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "后台IP地址", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_PORT:			/*石化后台通讯端口号信息*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "后台端口号", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_BACKSTAGE_LOCAL_PORT:			/*石化后台通讯本地服务器端口号信息*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "本地端口号", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_INFO:				/*平板电脑网络信息项选择界面*/
			if(0 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "1.平板电脑IP地址", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "2.平板子网掩码  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "3.平板默认网关  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(1 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "4.平板首选DNS   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "5.平板备用DNS   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "6.平板FTP地址   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(2 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "7.平板FTP端口号 ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "8.平板后台地址  ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "9.平板音量      ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}
			else if(3 == buffer[0])
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "10.对讲后台IP   ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "11.是否启用促销 ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "12.油品确认功能 ", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);
			}

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_IP:					/*平板电脑IP信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "平板IP地址", 10);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_MASK:				/*平板电脑掩码信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板子网掩码", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_GATEWAY:		/*平板电脑网关信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板默认网关", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FIRSTDNS:		/*平板电脑首选DNS信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板首选DNS", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_SECONDDNS:	/*平板电脑备用DNS信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板备用DNS", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FTP_IP	:			/*平板电脑FTP地址信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板FTP地址", 11);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_FTP_PORT:		/*平板电脑FTP端口号信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "平板FTP端口号", 13);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>5)?5:temp_len;
			memset(dsp_buffer, ' ', 5);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, (char*)dsp_buffer, 5);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_SERVERIP:		/*平板电脑连接的后台IP地址信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 2*8, "平板后台地址", 12);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TABLETPC_VOLUME:			/*平板电脑音量信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "  平板电脑音量  ", 16);
			sprintf((char*)dsp_buffer, "%d", buffer[0]);
			if(strlen((char*)dsp_buffer) < 3)	memset(dsp_buffer + strlen((char*)dsp_buffer), ' ', 3 - strlen((char*)dsp_buffer));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 7*8, (char*)dsp_buffer, 3);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_TABLETPC_TELE_IP:		/*平板电脑语音对讲后台IP地址信息界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 1*8, "平板对讲后台IP", 14);
			temp_len = strlen((char*)buffer);
			temp_len = (temp_len>15)?15:temp_len;
			memset(dsp_buffer, ' ', 15);
			memcpy(dsp_buffer, buffer, temp_len);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, (char*)dsp_buffer, 15);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		
		case DSP_CONNECT_TYPE_SET:		/*后台连接方式设置界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, " 请选择连接方式 ", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "  1.电流环串口  ", 16);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 5, 0*8, "  2.RJ45网口    ", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else									tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CONNECT_TYPE_DSP:		/*后台连接方式查询界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "石化后台连接方式", 16);
			if('0' == buffer[0])		tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "   电流环串口   ", 16);
			else if('1' == buffer[0])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "    RJ45网口    ", 16);
			else						tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "                ", 16);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CONTRAST:						/*显示对比度*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 3*8, "显示对比度", 10);
			sprintf((char*)dsp_buffer, "%d", buffer[0]);	memset(dsp_buffer+strlen((char*)dsp_buffer), ' ', 3);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 7*8, (char*)dsp_buffer, 3);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_DICOUNT_FAIL_KEEP:		/*申请折扣失败后人工确认界面*/
			if(id!=dsp->interfaceID)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "折扣价格申请失败", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "继续以原价格加油", 16);
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "请输入员工密码: ", 16);
			}
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else						tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_MODEL:							/*机型操作界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "机型", 16);
			
			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_AUTH_BALANCE:				/*授权加油余额界面*/
			/*第0行*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 4*8, "授权余额", strlen("授权余额"));
			/*第3行*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "元", 2);
			dsp_len = 13-i;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, (16-dsp_len)/2*8, (char*)dsp_buffer+i, dsp_len);

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_CARD_DEBIT:				/*非油消费扣款界面*/
			/*第0行*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "支付金额", strlen("支付金额"));
			/*第3行*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[11], "元", 2);
			dsp_len = 13-i;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, (16-dsp_len)/2*8, (char*)dsp_buffer+i, dsp_len);

			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, "确认--进行卡支付", strlen("确认--进行卡支付"));
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 0*8, "更改--取消卡支付", strlen("更改--取消卡支付"));

			tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_PROMOTION:					/*是否启用促销界面*/
			/*第0行:显示提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "是否启用促销功能", 16);
			/*第2行:显示是否启用促销功能*/
			if(1 == buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, " 启用 ", 6);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "不启用", 6);
			
			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID && 1 == buffer[1])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_YuLe_Grade_Fun:					/*是否启用油品确认功能*/
			/*第0行:显示提示*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "油品确认功能", 16);
			/*第2行:显示是否启用促销功能*/
			if(PC_FUN_GRADE_OK == buffer[0])			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, " 启用 ", 6);
			else								tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 5*8, "不启用", 6);
			
			/*第6行:显示上下键提示*/
			if(id!=dsp->interfaceID && 1 == buffer[1])	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 12*8, "↑↓", 4);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
		case DSP_TM_SCAN_AND_INPUT:	/*条码扫描及输入界面*/
			/*第0,2行:显示"请扫描条码，或在键盘上输入验证码"*/
			if(id!=dsp->interfaceID)
			{
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 0*8, "请扫描条码，或在", strlen("请扫描条码，或在"));
				tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "键盘上输入验证码", strlen("键盘上输入验证码"));
			}
			
			/*第4行:显示输入的条码*/
			memset(dsp_buffer, ' ', 10);
			if(buffer[0]<10)	memcpy(&dsp_buffer[10-buffer[0]], &buffer[1], buffer[0]);
			else						memcpy(&dsp_buffer[0], &buffer[1], 10);
			dsp_len=10;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, dsp_len);

			/*第6行:显示下划线*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 6, 3*8, "----------", strlen("----------"));

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_PRESET:				/*银行卡预置界面*/
			/*第2行:显示"请输入预置量:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入预置量:", 13);

			/*第4行:显示预置量及单位*/
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[0]=(char)((bcd_value>>36)&0x0f)+0x30;	dsp_buffer[1]=(char)((bcd_value>>32)&0x0f)+0x30;
			dsp_buffer[2]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[3]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[4]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[5]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[6]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<7; i++)
			{
				if('0'==dsp_buffer[i])	dsp_buffer[i]=' ';
				else								break;
			}
			dsp_buffer[7]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[8]='.';
			dsp_buffer[9]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[10]=(char)((bcd_value>>0)&0x0f)+0x30;
			if(0==buffer[4])	memcpy(&dsp_buffer[11], "元", 2);
			else						memcpy(&dsp_buffer[11], "升", 2);
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 3*8, (char*)dsp_buffer, 13);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_PIN_INPUT:			/*银行卡密码输入界面*/
			/*第2行:显示"请输入卡密码:"*/
			if(id!=dsp->interfaceID)	tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 2, 0*8, "请输入卡密码:", 13);
			/*第4行:根据密码位数显示*号，没有*号需要显示的地方以空格*/
			memset(dsp_buffer, ' ', 16);
			for(i=0; i<16 && i<buffer[0]; i++)	dsp_buffer[16-1-i]='*';
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;

		case DSP_BANK_AUTH_REGUEST:	/*银行卡预授权*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "授权申请中请稍候", 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_BANK_AUTH_RESULT:	/*银行卡预授权成功界面*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 3, 0*8, "授权成功,请提枪.", 16);
			
			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);
			break;
			
		case DSP_BANK_OILLING:				/*银行卡加油中界面*/
			/*第0行:加油中*/
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 0, 5*8, "加油中", 6);

			/*第4行:当前加油金额*/
			memcpy(&dsp_buffer[0], "金额:", 5);
			bcd_value=hex2Bcd((buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0));
			dsp_buffer[5]=(char)((bcd_value>>28)&0x0f)+0x30;	dsp_buffer[6]=(char)((bcd_value>>24)&0x0f)+0x30;
			dsp_buffer[7]=(char)((bcd_value>>20)&0x0f)+0x30;	dsp_buffer[8]=(char)((bcd_value>>16)&0x0f)+0x30;
			dsp_buffer[9]=(char)((bcd_value>>12)&0x0f)+0x30;
			for(i=0; i<5; i++)
			{
				if('0'==dsp_buffer[5+i])	dsp_buffer[5+i]=' ';
				else									break;
			}
			dsp_buffer[10]=(char)((bcd_value>>8)&0x0f)+0x30;
			dsp_buffer[11]='.';
			dsp_buffer[12]=(char)((bcd_value>>4)&0x0f)+0x30;	dsp_buffer[13]=(char)((bcd_value>>0)&0x0f)+0x30;
			memcpy(&dsp_buffer[14], "元", 2);
			dsp_len=16;
			tdDspContent(dsp->DEVKeyx, KB_FONT16, KB_CONTRAY_NO, 4, 0*8, (char*)dsp_buffer, dsp_len);

			/*首次显示的界面需要清屏，非首次显示界面不清屏*/
			if(id==dsp->interfaceID)	tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR_NO);
			else										tdDsp(dsp->DEVKeyx, dsp->Contrast, KB_DSPCLR);			
			break;

		default:
			break;
		}

		/*存储当前显示界面号*/
		dsp->interfaceID=id;

		return 0;
	}


	/*平板电脑显示屏显示*/
	if(DSP_DEV_PCMONITOR==dsp->DEV)
	{
		/*判断长度*/
		if(8+nbytes > DSP_LEN_MAX)	return ERROR;

		/*清空显示刷新定时器*/
		*timer=0;
			
		/*判断帧号不为0xfa*/
		dsp->txFrame++;	if(0xfa==dsp->txFrame)	dsp->txFrame++;	

		dsp->txBuffer[0]=0xfa;
		dsp->txBuffer[1]=(char)((3+nbytes)>>8);
		dsp->txBuffer[2]=(char)((3+nbytes)>>0);
		dsp->txBuffer[3]=dsp->txFrame;
		dsp->txBuffer[4]=0x55;
		dsp->txBuffer[5]=id;
		memcpy(&dsp->txBuffer[6], buffer, nbytes);
		crc=crc16Get(&dsp->txBuffer[1], 5+nbytes);
		dsp->txBuffer[6+nbytes]=(char)(crc>>8);
		dsp->txBuffer[7+nbytes]=(char)(crc>>0);
		dsp->txLen=8+nbytes;
		comWrite(dsp->comFd, (char*)dsp->txBuffer, dsp->txLen);
	
		return 0;
	}

	return 0;
}


/********************************************************************
*Name				:dspInit
*Description		:显示模块功能初始化
*Input				:None
*Output				:None
*Return				:0=成功；其它=失败
*History			:2014-10-10,modified by syj
*/
int dspInit(void)
{
	DspParamStructType *dsp=NULL;

	/*A面显示任务*/
	dsp=&DspParamA;

	/*显示对比度*/
	dsp->Contrast=32;

/*键盘点阵屏显示相关参数*/
	/*键盘点阵显示屏*/
	dsp->DEV=DSP_DEV_KEYBOARD;
	/*点阵屏设备号*/
	dsp->DEVKeyx=DEV_DSP_KEYA;

/*andriod平板显示屏相关参数*/	
	/*平板通讯串口*/
	dsp->comFd=COM7;
	/*创建发送缓存保护信号量*/
	//del dsp->semIdTx=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==dsp->semIdTx)	printf("Error! Create semaphore 'dspA.semIdTx' failed!\n");
	pthread_mutex_init(&dsp->semIdTx, NULL);

	/*显示返回接收任务初始化*/
	//del dsp->tIdDspRx=taskSpawn("tDspRxA", 156, 0, 0x2000, (FUNCPTR)tdspRecive, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDspRx))	printf("Error!	Creat task 'tDspRxA' failed!\n");
	pthread_t tDspRxA;
	dsp->tIdDspRx=pthread_create(&tDspRxA, NULL, (void*)tdspRecive, NULL);
	if(0!=dsp->tIdDspRx) printf("Error!	Creat task 'tDspRxA' failed!\n");
	pthread_detach(tDspRxA);

	/*显示刷新任务初始化*/
	//del dsp->tIdDsp=taskSpawn("tDspA", 156, 0, 0x2000, (FUNCPTR)tdsp, 0,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDsp))	printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_t tDspA;
	dsp->tIdDsp=pthread_create(&tDspA, NULL, (void*)tdsp, NULL);
	if(0!=dsp->tIdDsp) printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_detach(tDspA);




	/*B面显示任务*/
	dsp=&DspParamB;

	/*显示对比度*/
	dsp->Contrast=32;

/*键盘点阵屏显示相关参数*/
	/*键盘点阵显示屏*/
	dsp->DEV=DSP_DEV_KEYBOARD;
	/*点阵屏设备号*/
	dsp->DEVKeyx=DEV_DSP_KEYB;	

	
/*andriod平板显示屏相关参数*/
	/*平板通讯串口*/
	dsp->comFd=COM8;					
	/*创建发送缓存保护信号量*/
	//del dsp->semIdTx=semBCreate(SEM_Q_FIFO, SEM_FULL);
	//del if(NULL==dsp->semIdTx)	printf("Error! Create semaphore 'dspB.semIdTx' failed!\n");
	pthread_mutex_init(&dsp->semIdTx, NULL);

	/*显示返回接收任务初始化*/
	//del dsp->tIdDspRx=taskSpawn("tDspRxB", 156, 0, 0x2000, (FUNCPTR)tdspRecive, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDspRx))	printf("Error!	Creat task 'tDspRxB' failed!\n");
	pthread_t tDspRxB;
	dsp->tIdDspRx=pthread_create(&tDspRxB, NULL, (void*)tdspRecive, NULL);
	if(0!=dsp->tIdDspRx) printf("Error!	Creat task 'tDspRxB' failed!\n");
	pthread_detach(tDspRxB);

	/*显示刷新任务初始化*/
	//del dsp->tIdDsp=taskSpawn("tDspB", 156, 0, 0x2000, (FUNCPTR)tdsp, 1,1,2,3,4,5,6,7,8,9);
	//del if(OK!=taskIdVerify(dsp->tIdDsp))	printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_t tDspB;
	dsp->tIdDsp=pthread_create(&tDspB, NULL, (void*)tdsp, NULL);
	if(0!=dsp->tIdDsp) printf("Error!	Creat task 'tDspA' failed!\n");
	pthread_detach(tDspB);

	return 0;
}


