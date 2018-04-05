//#include "oilCfg.h"
//#include "yaffs22/Yaffs_guts.h"
//#include "yaffs22/Yaffsfs.h"
//#include "oilFile.h"
//#include "oilSelect.h"

#include "../inc/main.h"

/********************************************************************
*Name			:oilNameGet
*Description	:获取油品代码对应油品名称
*Input			:code			油品代码,4位ASCII
*					:maxbytes	油品名称缓存最大长度，应不小于16字节
*Output		:buffer			油品名称
*Return			:实际油品名称长度；错误返回ERROR
*History		:2014-10-17,modified by syj
*/
int oilNameGet(unsigned char *code, unsigned char *buffer, int maxbytes)
{
	unsigned char read_buffer[128+16]={0};
	int fd=0, read_len=0, ireturn=0, i=0;
	off_t offset=0;

	//打开油品名称表文件
	fd=fileOpen(FILE_OILCODE_NAME, O_RDONLY, S_IREAD);

	//打开文件错误
	if(ERROR==fd){

		return ERROR;
	}

	/*获取油品代码对应名称
	*	从文件头开始读取并判断，读取油品代码所对应油品名称，
	*	每行存储一组数据，每组数据以双引号包括其内容，以油品1010为例
	*	"1010_90#汽油"
	*/
	while(read_len=fileRead(fd, offset, read_buffer, 128))
	{
		//判断是否匹配
		if(0==memcmp(&read_buffer[1], code, 4)){

			//保存读取到的油品名称，从固定位置6到双引号之间
			for(i=0; i<read_len && i<maxbytes; i++) 
			{
				if('"'==read_buffer[6+i])	break;
				buffer[i]=read_buffer[6+i];
			}

			fileClose(fd);
			return i;
		}

		//计算下一个读取位置
		for(i=0; i<read_len && i<128; i++)
		{
			if(0x0d==read_buffer[i] && 0x0a==read_buffer[i+1]){
		
				offset+=2;
				break;
			}
			else{
	
				offset++;
			}
		}

		//清空读取的数据
		memset(read_buffer, 0, 128);
	}

	//关闭油品名称表文件
	fileClose(fd);

	//此处仍未退出则标明未查到对应名称，则以汽油柴油为返回*/
	if('1'==code[0]){

		memcpy(buffer, "汽油", 4);	ireturn=4;
	}
	else if('2'==code[0]){

		memcpy(buffer, "柴油", 4);	ireturn=4;
	}
	else if('2'==code[0]){

		memcpy(buffer, "未知油品", 8);	ireturn=8;
	}

	return ireturn;
}


/********************************************************************
*Name			:YPLightTurnOn
*Description	:点亮油品选择灯
*Input			:panel		面板号 0=A面；1=B面
*					:YPx			油品按钮序号 0=YP1、1=YP2、2=YP3
*Output		:None
*Return			:0=成功;其他=失败
*History		:2016-03-07,modified by syj
*/
int YPLightTurnOn(int panel, int YPx)
{
	if(0 == panel && 0 == YPx)	kbIOWrite(DEV_DSP_KEYA, 7, 1);
	if(0 == panel && 1 == YPx)	kbIOWrite(DEV_DSP_KEYA, 8, 1);
	if(0 == panel && 2 == YPx)	kbIOWrite(DEV_DSP_KEYA, 9, 1);
	if(1 == panel && 0 == YPx)	kbIOWrite(DEV_DSP_KEYB, 7, 1);
	if(1 == panel && 1 == YPx)	kbIOWrite(DEV_DSP_KEYB, 8, 1);
	if(1 == panel && 2 == YPx)	kbIOWrite(DEV_DSP_KEYB, 9, 1);

	return 0;
}


/********************************************************************
*Name			:YPLightTurnOff
*Description	:关闭油品选择灯
*Input			:panel		面板号 0=A面；1=B面
*					:YPx			油品按钮序号 0=YP1、1=YP2、2=YP3
*Output		:None
*Return			:0=成功;其他=失败
*History		:2016-03-07,modified by syj
*/
int YPLightTurnOff(int panel, int YPx)
{
	if(0 == panel && 0 == YPx)	kbIOWrite(DEV_DSP_KEYA, 7, 0);
	if(0 == panel && 1 == YPx)	kbIOWrite(DEV_DSP_KEYA, 8, 0);
	if(0 == panel && 2 == YPx)	kbIOWrite(DEV_DSP_KEYA, 9, 0);
	if(1 == panel && 0 == YPx)	kbIOWrite(DEV_DSP_KEYB, 7, 0);
	if(1 == panel && 1 == YPx)	kbIOWrite(DEV_DSP_KEYB, 8, 0);
	if(1 == panel && 2 == YPx)	kbIOWrite(DEV_DSP_KEYB, 9, 0);

	return 0;
}

#if 0

/********************************************************************
*Name			:oilSelectLight
*Description	:点亮油品选择按钮灯
*Input			:nozzle			枪选0=1号枪，1=2号枪，2=3号枪
*Output		:None
*Return			:0=成功;其他=失败
*History		:2014-10-17,modified by syj
*/
int oilSelectLit(int panel, int number)
{
	if(0==nozzle)	kbIOWrite(DEV_DSP_KEYA, 7, 0);
	if(1==nozzle)	kbIOWrite(DEV_DSP_KEYA, 8, 0);
	if(2==nozzle)	kbIOWrite(DEV_DSP_KEYA, 9, 0);
	return 0;
}


/********************************************************************
*Name			:oilSelectLight
*Description	:关闭油品选择按钮灯
*Input			:nozzle			枪选0=1号枪，1=2号枪
*Output		:None
*Return			:0=成功;其他=失败
*History		:2014-10-17,modified by syj
*/
int oilSelectShut(int panel, int number)
{
	if(0==nozzle)	kbIOWrite(DEV_DSP_KEYA, 7, 1);
	if(1==nozzle)	kbIOWrite(DEV_DSP_KEYA, 8, 1);
	if(2==nozzle)	kbIOWrite(DEV_DSP_KEYA, 9, 1);
	return 0;
}


/********************************************************************
*Name			:oilSelectState
*Description	:读取是否有油品选择操作
*Input			:nozzle			枪选0=1号枪，1=2号枪
*Output		:None
*Return			:0=无操作；1=有选择操作
*History		:2014-10-17,modified by syj
*/
int oilSelectState(int DEV_SWITCH_SELxx)
{
	int iscgh=0, istate=0, ireturn=0;

	if(0==nozzle)			istate=kbSwitchRead(DEV_SWITCH_SELA1, &iscgh);
	else if(1==nozzle)	istate=kbSwitchRead(DEV_SWITCH_SELA2, &iscgh);

	if(0==istate && 1==iscgh)	ireturn=1;

	return ireturn;
}
#endif

