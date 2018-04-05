//#include "oilIpt.h"
//#include "oilFram.h"
//#include "oilParam.h"
//#include "oilStmTransmit.h"
//#include "oilCom.h"
//#include "oilCfg.h"
//#include "oilLog.h"
//#include "oilSpk.h"
//#include "oilTcpip.h"
//#include "oilKJLD.h"
//#include "oilPcd.h"
//#include "oilIpt.h"/*2017-01-22中燃联达灰卡解扣*/

#include "../inc/main.h"


//LOCAL SEM_ID semIDPcdBill = NULL;
pthread_mutex_t mutexIDPcdBill;

static PcdListInfoType pcdBaseList;							//基础黑名单
static PcdListInfoType pcdAddList;							//新增黑名单
static PcdListInfoType pcdDelList;							//新删黑名单
static PcdListInfoType pcdWhiteList;						//白名单
static PcdOilRecordType pcdOilInfo;							//油品油价表
static PcdStationInfoType pcdStationInfo;				    //通用信息
static PcdParamStructType pcdParam;						    //PCD操作相关数据结构
unsigned char Roll_Flag = 0;                                //轮询标志,szb_fj_20171120


/********************************************************************
*Name				:pcdFmRead
*Description		:PCD数据从铁电读出
*Input				:param_offset	数据
*						:buffer				有效数据
*						:maxbytes		有效数据长度
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
static int pcdFmRead(off_t param_offset, unsigned char *buffer, int maxbytes)
{
	  return framRead(FM_ADDR_PCD_SINO, param_offset, buffer, maxbytes); //fj:在铁电类里，oilFRam.c里
	  //return 0;
}


/********************************************************************
*Name				:pcdFmWrite
*Description		:PCD数据存入铁电，校验并备份
*Input				:param_offset	数据
*						:buffer				有效数据
*						:nbytes			有效数据长度
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
int pcdFmWrite(off_t param_offset, unsigned char *buffer, int nbytes)  //fj:typedef long off_t;如不支持直接修改成long
{
	unsigned char wrbuffer[PCD_FM_DATALEN+2]={0};  //fj:20170929,防止溢出修改,fj:20171120
	int crc_return=0;

	//printf("param_offset = %d\n",param_offset);
	
	if((param_offset+nbytes)>PCD_FM_DATALEN) //判断长度，不能超过有效数据长度
	{
		return ERROR;
	}

	//printf("--fmwrite aaaaa\n");

	//fj:20170920,与铁电操作相关函数先注释
	if(0!=framRead(FM_ADDR_PCD_SINO, 0, wrbuffer, PCD_FM_DATALEN)) //读出全部有效数据 ,fj:同上，函数目前不能用
	{
		  return ERROR;
	}

	//printf("--fmwrite bbbbb\n");

	//printf("pcdFmWrite 11111\n");
	memcpy(&wrbuffer[param_offset], buffer, nbytes);//拷贝当次要存储的数据
	crc_return=crc16Get(wrbuffer, PCD_FM_DATALEN); //计算有效数据校验值
	wrbuffer[PCD_FM_DATALEN+0]=(char)(crc_return>>8); //fj:明显的错误，数组溢出
	wrbuffer[PCD_FM_DATALEN+1]=(char)(crc_return>>0);

	//printf("--fmwrite cccc\n");

	//printf("pcdFmWrite 22222\n");	
	framWrite(FM_ADDR_PCD_SINO, 0, wrbuffer, PCD_FM_DATALEN+2);		//保存有效数据,fj:同上										
	framWrite(FM_ADDR_PCD_SINO, PCD_FM_BACKUP, wrbuffer, PCD_FM_DATALEN+2);	 //备份数据，fj:同上

	//printf("--fmwrite dddd\n");

	return 0;
}


/********************************************************************
*Name				:pcd2OtherSend
*Description		:PCD向其它主板发送数据
*Input				:mboard_id		主板号
*						:buffer				待发送的数据
*						:nbytes				待发送的数据长度
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
static int pcd2OtherSend(unsigned char mboard_id, unsigned char *buffer, int nbytes)
{
	return 0;
}


/********************************************************************
*Name				:pcd2OtherSend
*Description		:PCD向IPT发送数据
*Input				:mboard_id		主板号
*						:buffer				待发送的数据
*						:nbytes				待发送的数据长度
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
static int pcd2IptSend(
	unsigned char msg_type, 		 //消息类型
	unsigned char mboard_id,		 //主板号
	unsigned char fuelling_point,//支付终端号
	unsigned char phynozzle,		 //物理枪号
	unsigned char command,			 //命令字
	unsigned char *buffer,			 //数据
	int nbytes)								   //数据长度
{
	unsigned char tx_buffer[PCD_MSGMAX]={0};
	int tx_len=0;
	struct msg_struct msg_stPcd;
	msg_stPcd.msgType = 2;  //fj:20170918

	if(5+nbytes > PCD_MSGMAX)	 //判断数据长度
		return ERROR;

	//组织数据
	tx_buffer[0]=msg_type;							    //消息类型
	tx_buffer[1]=mboard_id;							    //主板号
	tx_buffer[2]=fuelling_point;				    //支付终端号   
	tx_buffer[3]=phynozzle;							    //物理枪号
	tx_buffer[4]=command;							      //命令字  
	memcpy(&tx_buffer[5], buffer, nbytes);	//数据
	tx_len=5+nbytes;

	//本地主板发送到对应的IPT接收消息队列，其它主板则转发给对应主板
	if(pcdParam.mboardID==mboard_id)	
	{
		 //printf("Msg Buffer ,tx_len = %d: \n",tx_len);
	     //PrintH(tx_len,tx_buffer);
		 //msgQSend((MSG_Q_ID)iptMsgIdRead(fuelling_point), tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL); //fj:先注释，后一起处理
		 memcpy(msg_stPcd.msgBuffer,tx_buffer,tx_len);
		 int nMsgID = iptMsgIdRead(fuelling_point);
		 if(nMsgID != ERROR)
		 {
		     msgsnd(nMsgID,&msg_stPcd,tx_len,IPC_NOWAIT);
			 //PrintH(tx_len,msg_stPcd.msgBuffer);
		 }
		 else
		 {
			 printf("pay terminal no. error!\n");  //fj:
		 }

         //printf("local board send\n");
	}
	else	
	{													
	   pcd2OtherSend(mboard_id, tx_buffer, tx_len);

	   //printf("other board send \n");
	}

	return 0;
}

/********************************************************************
*Name				:pcdIpt2PcSend
*Description		:IPT数据通过PCD转发到PC后台
*Input				:
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
static int pcdIpt2PcSend(
	unsigned char msg_type, 			 //消息类型
	unsigned char mboard_id, 			 //主板号
	unsigned char fuelling_point,  //支付终端号
	unsigned char phynozzle, 			 //物理枪号
	unsigned char command,				 //命令字
	unsigned char *buffer,				 //数据
	int nbytes)									   //数据长度
{
	PcdIpt2PcNode *node=NULL;  //fj:该处有一个VXWORKS的NODE看能不能用，是否跟linux里一样

	if(nbytes>128) //判断数据长度
	{
		return 1;
	}
	
	if(lstCount(&pcdParam.ipt2PcList)>=10) //最大节点数10
	{
		return 1;
	}

	node=(PcdIpt2PcNode*)malloc(sizeof(PcdIpt2PcNode)); //申请空间
	if(NULL==node)
	{
		return 1;
	}

	//添加节点数据
	node->msgType=msg_type;
	node->mboardId=mboard_id;
	node->fuellingPoint=fuelling_point;
	node->phynozzle=phynozzle;
	node->command=command;
	if(nbytes>128)	
		node->Nbytes=128;
	else					
		node->Nbytes=nbytes;
	memset(node->Buffer, 0, 128);
	memcpy(node->Buffer, buffer, node->Nbytes);
	lstAdd(&pcdParam.ipt2PcList, (NODE*)node); //fj:vxworks的函数先不处理

	return 0;
}


/********************************************************************
*Name				:pcd2PcSend
*Description		:数据封装后向PC后台发送
*Input				:pos_p		通讯POS_P
*						:frame		帧号，0~3f
*						:buffer		有效数据
*						:nbytes		有效数据长度
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
int pcd2PcSend(unsigned char *buffer, int nbytes)
{
	unsigned char data_buffer[512]={0},tx_buffer[512]={0};
	unsigned int tx_len=0, crc_data=0, i=0;
	long long data=0;

	//szb_fj_20171120,add
	if(buffer[0]==0x80)
	for(i=0;i<nbytes;i++)
	{
		if(i==0)
			printf("etc_tx:%x_",buffer[i]);
		else if(i==nbytes-1)
			printf("%x\n",buffer[i]);
		else 
			printf("%x_",buffer[i]);
	}

	if(nbytes+8>512)	 //判断长度
		return ERROR;

	//组织数据
	data_buffer[0]=0xfa;														//数据包头
	data_buffer[1]=0;																//目标地址
	data_buffer[2]=pcdStationInfo.POS_P;						//源地址
	pcdParam.pcFrame++;													    //帧号/控制
	if(pcdParam.pcFrame>0x3f)	
		pcdParam.pcFrame=0;
	data_buffer[3]=(0<<7)|(0<<6)|(pcdParam.pcFrame);	
	data=hex2Bcd(nbytes);													   //有效数据长度
	data_buffer[4]=(char)(data>>8);	data_buffer[5]=(char)(data>>0);
	memcpy(&data_buffer[6], buffer, nbytes);						//有效数据							

	//CRC校验
	crc_data=crc16Get(&data_buffer[1], 5+nbytes);							
	data_buffer[6+nbytes]=(char)(crc_data>>8);
	data_buffer[7+nbytes]=(char)(crc_data>>0);

	//添加0xfa
	memcpy(&tx_buffer[0], &data_buffer[0], 6);	tx_len=6;
	for(i=6; i<8+nbytes; i++)
	{	
		tx_buffer[tx_len]=data_buffer[i];	//发送数据赋值
		tx_len++;

		if(tx_len>=512)	//防止发送长度溢出
			break;
			
		if(0xfa==data_buffer[i]) //发送的数据为0xfa时添加一位0xfa
		{
			tx_buffer[tx_len]=data_buffer[i];	
			tx_len++;
		}
	
		if(tx_len>=512)	//防止发送长度溢出
			break;
	}


	//printf("--------->oil to pc server data \n");
	//PrintH(tx_len,tx_buffer);

	//数据发送
	kjldWrite(tx_buffer, tx_len); //fj:该函数在oilKJLD.c里，是向石化卡机联动发送后台数据

	
	/*
	if(0 == pcdParam.SinopecChannel){
		
		comWrite(pcdParam.comFdPc, tx_buffer, tx_len);	
	}
	else{

		netSWrite(NETS_PROTOCOL_UDP, pcdParam.NetSPort, tx_buffer, tx_len);
	}
*/

	return 0;
}


/********************************************************************
*Name				:pcdListSearch
*Description		:黑/白名单数据查找
*Input				:fd			文件
*						:number	卡号数目
*						:buffer		卡号
*Output			:None
*Return				:0=匹配；其它=不匹配
*History			:2014-03-25,modified by syj
*/
static int pcdListSearch(const char *path, unsigned int number, const unsigned char *buffer)
{
	unsigned int mid=0, start=0, end=0;
	unsigned char read_buffer[10]={0};
	int i=0;

	if(0==number) //判断名单数目不为0
	{
		return 1;
	}

	//判断起始位置名单
	start=0;
	fileReadForPath(path, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 16+10*start, read_buffer, 10);
	if(0==memcmp(buffer, read_buffer, 10))	
		return 0;

	//判断结束位置名单
	end=number;
	fileReadForPath(path, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  16+10*end, read_buffer, 10);
	if(0==memcmp(buffer, read_buffer, 10))	
		return 0;

	//判断中间位置名单
	mid=(start+end)/2;
	while((start<=end)&&(start<mid))
	{
		fileReadForPath(path, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  16+10*mid, read_buffer, 10);
		if(0==memcmp(buffer, read_buffer, 10))	
		{
			return 0;
		}
		else if(memcmp(buffer, read_buffer, 10)>0)		
		{ 
			start=mid;	mid=(start+end)/2;
		}
		else if(memcmp(buffer, read_buffer, 10)<0)		
		{
			end=mid;	mid=(start+end)/2;
		}
	}
	
	return 1;
}


/********************************************************************
*Name				:pcdLocalTTCGet
*Description		:在本主板申请TTC
*Input				:phynozzle	物理枪号
*						:buffer			账单数据，固定128bytes
*						:nbytes			账单数据长度，固定为128
*Output			:TTC				账单TTC
*Return				:0=成功；1=物理枪号非法；其它=错误
*History			:2014-03-25,modified by syj
*/
static int pcdLocalTTCGet(int phynozzle, char *buffer, int nbytes, unsigned int *TTC)
{
	unsigned char read_buffer[256]={0}, wrbuffer[32]={0};
	unsigned int last_ttc=0, i_ttc=0;
	off_t i_offset=0;
	int istate = 0;

	//semTake(semIDPcdBill, WAIT_FOREVER); //fj:先注释，最后处理
    pthread_mutex_lock(&mutexIDPcdBill);   //fj:20170917
	if(phynozzle<1 || phynozzle>PCD_NOZZLE_MAX) //判断物理枪号合法性
	{
		istate = 1;
		goto DONE;
	}

	//判断是否重复申请
	if(1==phynozzle)			last_ttc=pcdParam.TTC1;
	else if(2==phynozzle)	last_ttc=pcdParam.TTC2;
	else if(3==phynozzle)	last_ttc=pcdParam.TTC3;
	else if(4==phynozzle)	last_ttc=pcdParam.TTC4;
	else if(5==phynozzle)	last_ttc=pcdParam.TTC5;
	else if(6==phynozzle)	last_ttc=pcdParam.TTC6;

	//如果最后一笔存储的账单与待申请TTC的账单相同则认为已经申请过TTC
	if(last_ttc>0)
	{
		fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((last_ttc-1)%PCD_RECORD_MAX), read_buffer, PCD_ZD_LEN);
		if(0==memcmp(&read_buffer[4], &buffer[4], 89))
		{
			*TTC=last_ttc;
			istate = 0;
			goto DONE;
		}
	}

	//计算累加后的TTC
	i_ttc=pcdParam.TTC+1;

	//保存申请TTC的账单
	buffer[0]=(char)(i_ttc>>24);	buffer[1]=(char)(i_ttc>>16);
	buffer[2]=(char)(i_ttc>>8);		buffer[3]=(char)(i_ttc>>0);
	i_offset=PCD_ZD_LEN*((i_ttc-1)%PCD_RECORD_MAX);
	fileWriteForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, buffer, PCD_ZD_LEN);

	//校验账单存储
	fileReadForPath(PCD_FILE_OILRECORD, O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(0!=memcmp(buffer, read_buffer, PCD_ZD_LEN))
	{
		jljUserLog("错误!物理枪号:%d，TTC:%d，申请TTC失败，写入与读出不一致!\n", phynozzle, pcdParam.TTC); 
		istate = 2;
		goto DONE;
	}

	//POS_TTC累加并保存
	pcdParam.TTC=i_ttc;
	read_buffer[0]=(char)(pcdParam.TTC>>24);	read_buffer[1]=(char)(pcdParam.TTC>>16);
	read_buffer[2]=(char)(pcdParam.TTC>>8);		read_buffer[3]=(char)(pcdParam.TTC>>0);
	pcdFmWrite(PCD_FM_TTC, read_buffer, 4); //fj:本地函数

	//返回TTC
	*TTC=pcdParam.TTC;

	//待传账单数目累加
	pcdParam.UnloadNumber++;
	read_buffer[0]=(char)(pcdParam.UnloadNumber>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber>>16);
	read_buffer[2]=(char)(pcdParam.UnloadNumber>>8);		read_buffer[3]=(char)(pcdParam.UnloadNumber>>0);
	pcdFmWrite(PCD_FM_UNLOAD, read_buffer, 4);

	
	//逃卡账单保存索引信息，方便查找本地灰卡信息时的查询
	//在异常账单索引文件中循环存储异常账单的TTC号，时间等信息
	
	if(1==(buffer[4]&0x0f))
	{
		pcdParam.AbnormalNumber++;
		read_buffer[0]=(char)(pcdParam.AbnormalNumber>>24);	read_buffer[1]=(char)(pcdParam.AbnormalNumber>>16);
		read_buffer[2]=(char)(pcdParam.AbnormalNumber>>8);		read_buffer[3]=(char)(pcdParam.AbnormalNumber>>0);
		pcdFmWrite(PCD_FM_YCZDNUM, read_buffer, 4);

		read_buffer[0]=(unsigned char)(pcdParam.TTC>>24);	read_buffer[1]=(unsigned char)(pcdParam.TTC>>16);
		read_buffer[2]=(unsigned char)(pcdParam.TTC>>8);	read_buffer[3]=(unsigned char)(pcdParam.TTC>>0);
		memcpy(&read_buffer[4], &buffer[5], 7);	
		memset(&read_buffer[11], 0, 5);
		fileWriteForPath(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZDERRINFO_SIZE*((pcdParam.AbnormalNumber-1)%PCD_ZD_INDEX_MAX), read_buffer, 16);
	}

	//根据物理枪号，单独保存每条物理油枪的数据
	if(1==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC1=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC1>>24);		wrbuffer[1]=(char)(pcdParam.TTC1>>16);
		wrbuffer[2]=(char)(pcdParam.TTC1>>8);		wrbuffer[3]=(char)(pcdParam.TTC1>>0);
		pcdFmWrite(PCD_FM_TTC1, wrbuffer, 4);

	   // g_fjLog.WriteLog("pcdLocalTTCGet  ","pcdFmWrite_PCD_FM_TTC1  ",wrbuffer,4);

		//账单数目累加并保存
		pcdParam.ZDNumber1++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber1>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber1>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber1>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber1>>0);
		pcdFmWrite(PCD_FM_ZDNUM1, wrbuffer, 4);

        //g_fjLog.WriteLog("pcdLocalTTCGet  ","pcdFmWrite_PCD_FM_ZDNUM1  ",wrbuffer,4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber1++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber1>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber1>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber1>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber1>>0);
		pcdFmWrite(PCD_FM_UNLOAD1, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC1>>24);		wrbuffer[1]=(char)(pcdParam.TTC1>>16);
		wrbuffer[2]=(char)(pcdParam.TTC1>>8);		wrbuffer[3]=(char)(pcdParam.TTC1>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_1, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber1-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(2==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC2=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC2>>24);		wrbuffer[1]=(char)(pcdParam.TTC2>>16);
		wrbuffer[2]=(char)(pcdParam.TTC2>>8);		wrbuffer[3]=(char)(pcdParam.TTC2>>0);
		pcdFmWrite(PCD_FM_TTC2, wrbuffer, 4);

		//账单数目累加并保存
		pcdParam.ZDNumber2++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber2>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber2>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber2>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber2>>0);
		pcdFmWrite(PCD_FM_ZDNUM2, wrbuffer, 4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber2++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber2>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber2>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber2>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber2>>0);
		pcdFmWrite(PCD_FM_UNLOAD2, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC2>>24);		wrbuffer[1]=(char)(pcdParam.TTC2>>16);
		wrbuffer[2]=(char)(pcdParam.TTC2>>8);		wrbuffer[3]=(char)(pcdParam.TTC2>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_2, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber2-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(3==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC3=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC3>>24);		wrbuffer[1]=(char)(pcdParam.TTC3>>16);
		wrbuffer[2]=(char)(pcdParam.TTC3>>8);		wrbuffer[3]=(char)(pcdParam.TTC3>>0);
		pcdFmWrite(PCD_FM_TTC3, wrbuffer, 4);

		//账单数目累加并保存
		pcdParam.ZDNumber3++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber3>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber3>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber3>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber3>>0);
		pcdFmWrite(PCD_FM_ZDNUM3, wrbuffer, 4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber3++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber3>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber3>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber3>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber3>>0);
		pcdFmWrite(PCD_FM_UNLOAD3, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC3>>24);		wrbuffer[1]=(char)(pcdParam.TTC3>>16);
		wrbuffer[2]=(char)(pcdParam.TTC3>>8);		wrbuffer[3]=(char)(pcdParam.TTC3>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_3, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber3-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(4==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC4=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC4>>24);		wrbuffer[1]=(char)(pcdParam.TTC4>>16);
		wrbuffer[2]=(char)(pcdParam.TTC4>>8);		wrbuffer[3]=(char)(pcdParam.TTC4>>0);
		pcdFmWrite(PCD_FM_TTC4, wrbuffer, 4);

		//账单数目累加并保存
		pcdParam.ZDNumber4++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber4>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber4>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber4>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber4>>0);
		pcdFmWrite(PCD_FM_ZDNUM4, wrbuffer, 4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber4++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber4>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber4>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber4>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber4>>0);
		pcdFmWrite(PCD_FM_UNLOAD4, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC4>>24);		wrbuffer[1]=(char)(pcdParam.TTC4>>16);
		wrbuffer[2]=(char)(pcdParam.TTC4>>8);		wrbuffer[3]=(char)(pcdParam.TTC4>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_4, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber4-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(5==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC5=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC5>>24);		wrbuffer[1]=(char)(pcdParam.TTC5>>16);
		wrbuffer[2]=(char)(pcdParam.TTC5>>8);		wrbuffer[3]=(char)(pcdParam.TTC5>>0);
		pcdFmWrite(PCD_FM_TTC5, wrbuffer, 4);

		//账单数目累加并保存
		pcdParam.ZDNumber5++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber5>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber5>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber5>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber5>>0);
		pcdFmWrite(PCD_FM_ZDNUM5, wrbuffer, 4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber5++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber5>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber5>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber5>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber5>>0);
		pcdFmWrite(PCD_FM_UNLOAD5, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC5>>24);		wrbuffer[1]=(char)(pcdParam.TTC5>>16);
		wrbuffer[2]=(char)(pcdParam.TTC5>>8);		wrbuffer[3]=(char)(pcdParam.TTC5>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_5, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber5-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(6==phynozzle)
	{
		//保存本枪最后一笔TTC
		pcdParam.TTC6=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC6>>24);		wrbuffer[1]=(char)(pcdParam.TTC6>>16);
		wrbuffer[2]=(char)(pcdParam.TTC6>>8);		wrbuffer[3]=(char)(pcdParam.TTC6>>0);
		pcdFmWrite(PCD_FM_TTC6, wrbuffer, 4);

		//账单数目累加并保存
		pcdParam.ZDNumber6++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber6>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber6>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber6>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber6>>0);
		pcdFmWrite(PCD_FM_ZDNUM6, wrbuffer, 4);

		//未传账单数目累加并保存
		pcdParam.UnloadNumber6++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber6>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber6>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber6>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber6>>0);
		pcdFmWrite(PCD_FM_UNLOAD6, wrbuffer, 4);

		//保存账单索引，4bytes TTC+7bytes时间
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC6>>24);		wrbuffer[1]=(char)(pcdParam.TTC6>>16);
		wrbuffer[2]=(char)(pcdParam.TTC6>>8);		wrbuffer[3]=(char)(pcdParam.TTC6>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_6, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber6-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}

DONE:
	//semGive(semIDPcdBill); //fj:先注释，后处理
	pthread_mutex_unlock(&mutexIDPcdBill); //fj:20170917
	
	return istate;
}


/********************************************************************
*Name				:pcdLocalBillSave
*Description		:在本主板保存账单，返回值为1时可重新申请TTC后再行存储
*Input				:phynozzle	物理枪号
*						:buffer			账单数据，固定128bytes
*						:nbytes			账单数据长度，固定为128
*Output			:None
*Return				:0=成功；1=TTC错误;2=TTC指定位置数据不一致；3=存储失败；4=存储校验失败
*History			:2014-03-25,modified by syj
*/
static int pcdLocalBillSave(int phynozzle, char *buffer, int nbytes)
{
	unsigned char read_buffer[256]={0};
	unsigned int pos_ttc=0;
	off_t i_offset=0;
	int i=0, wlen=0;
	unsigned char printbuffer[512]={0};
	int istate = 0;

	//semTake(semIDPcdBill, WAIT_FOREVER); //fj:先注释，后处理
	pthread_mutex_lock(&mutexIDPcdBill); //fj:20170917

	//计算账单的TTC
	pos_ttc=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);

	if(0==pos_ttc) //判断TTC
	{
		istate = 1;
		goto DONE;
	}

	//计算偏移
	i_offset=PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX);

	//判断是否与申请TTC时存储一致，比对T_MAC之前的数据(不包含T_MAC)
	fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(0!=memcmp(read_buffer, buffer, 95-4))
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("账单存储写入错误!与申请TTC时存储数据不一致!\n");  
		istate = 2;
		goto DONE;
	}

	//保存账单失败则返回失败
	wlen=fileWriteForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, buffer, PCD_ZD_LEN);
	if(ERROR==wlen)
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("账单存储写入错误![%s]\n", printbuffer); 
		istate = 3;
		goto DONE;
	}

	//读取账单进行写入校验
	wlen=fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(ERROR!=wlen && 0!=memcmp(read_buffer, buffer, PCD_ZD_LEN))
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("账单存储校验错误![%s]\n", printbuffer); 
		istate = 4;
		goto DONE;
	}

DONE:
	
	//semGive(semIDPcdBill); //fj:先不处理
	pthread_mutex_unlock(&mutexIDPcdBill); //fj:20170917
	
	return istate;
}


/********************************************************************
*Name				:pcdLocalWBListGet
*Description		:在本地黑白名单查询
*						:inbuffer		输入缓存，需要查询的卡部分信息
*						:					卡号(10压缩BCD) +最后一笔交易日期(4压缩BCD，用户卡有效，其它卡或卡内无交易时填0)
*						:nbytes			输入缓存长度
*						:maxbytes	输出缓存最大长度
*Output			:isMatch		匹配标识(1Bin, b0=0:匹配/其它:不匹配) 
*Return				:0=成功；其它=失败，需联机查询
*History			:2013-08-05,modified by syj
*/
static int pcdLoaclWBListGet(int phynozzle, char *inbuffer, int nbytes, char *isMatch)
{
	unsigned char real_time[7]={0}, card_number[10]={0}, bill_time[4]={0}, card_type=0, card_area=0, is_ok=0;
	RTCTime rtime;
	unsigned int number=0;

	memcpy(card_number, &inbuffer[0], 10); //解析卡号
	memcpy(bill_time, &inbuffer[10], 4); //解析卡最后一笔交易日期
	card_type=card_number[2]; //解析卡类型
	card_area=((card_number[3]&0x0f)<<4)|((card_number[4]>>4)&0x0f); //解析卡地区号

	//解析当前时间
	timeRead(&rtime); //fj:该函数在oilStmTransmit.c里
	real_time[0]=rtime.century;	real_time[1]=rtime.year;	real_time[2]=rtime.month;		real_time[3]=rtime.date;
	real_time[4]=rtime.hour;		real_time[5]=rtime.minute;	real_time[6]=rtime.second;

	/*
	*	满足以下条件时用户卡查询本地白名单；
	*	该卡为内部卡(非用户卡)；
	*	未有正在下载白名单的操作；
	*	当前时间合法；
	*	白名单起始时间，截止时间均合法；
	*	白名单版本合法并处于有效期内；
	*/
	if(1 != card_type &&\
		PCD_DOWN_WHITELIST != pcdParam.PcDownloadContent &&\
		0==timeVerification(real_time, 7) &&\
		0==timeVerification(pcdWhiteList.TimeStart, 4) && 0==timeVerification(pcdWhiteList.TimeFinish, 4) &&\
		memcmp(pcdWhiteList.TimeStart, real_time, 4)<=0 && memcmp(pcdWhiteList.TimeFinish, real_time, 4)>=0)
	{
		number=(pcdWhiteList.Number[0]<<24)|(pcdWhiteList.Number[1]<<16)|(pcdWhiteList.Number[2]<<8)|(pcdWhiteList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_WLIST, number, card_number))
		{
			*isMatch=0;
		}
		else
		{
			*isMatch=1;
		}

		return 0;
	}

	/*
	*	满足以下条件时用户卡查询本地黑名单；
	*	该卡为用户卡；
	*	未有正在下载基础黑名单；
	*	未有正在下载新增黑名单；
	*	未有正在下载新删黑名单的操作；
	*	该卡为本省卡；
	*	当前时间合法；
	*	基础黑名单起始时间，截止时间均合法；
	*	基础黑名单版本合法并处于有效期内；
	*	新增黑名单起始时间，截止时间均合法；
	*	新增黑名单版本合法并处于有效期内；
	*	新删黑名单起始时间，截止时间均合法；
	*	新删黑名单版本合法并处于有效期内；
	*/
	//*
	
	printf("card_area = %d,pcdStationInfo.Province = %d\n",card_area,pcdStationInfo.Province);

	if(!(PCD_DOWN_BASELIST!=pcdParam.PcDownloadContent ))	printf("%s:正在下载基础黑名单，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(PCD_DOWN_ADDLIST!=pcdParam.PcDownloadContent))		printf("%s:正在下载新增黑名单，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(PCD_DOWN_DELLIST!=pcdParam.PcDownloadContent))		printf("%s:正在下载新删黑名单，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(card_area==pcdStationInfo.Province))								printf("%s:非本省用户卡，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(0==timeVerification(real_time, 7)))										printf("%s:主板时间非法，无法查询本地黑名单!\n", __FUNCTION__);

	if(!(0==timeVerification(pcdBaseList.TimeStart, 4)))				printf("%s:基础黑名单启用日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdBaseList.TimeFinish, 4)))				printf("%s:基础黑名单截止日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdBaseList.TimeStart, real_time, 4)<=0))			printf("%s:基础黑名单未启用，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdBaseList.TimeFinish, real_time, 4)>=0))			printf("%s:基础黑名单已过期，无法查询本地黑名单!\n", __FUNCTION__);

	printf("real_time:\n");
	PrintH(4,real_time);
	printf("pcdBaseList.TimeStart:\n");
	PrintH(4,pcdBaseList.TimeStart);

	if(!(0==timeVerification(pcdAddList.TimeStart, 4)))					printf("%s:新增黑名单启用日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdAddList.TimeFinish, 4)))				printf("%s:新增黑名单截止日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdAddList.TimeStart, real_time, 4)<=0))			printf("%s:新增黑名单未启用，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdAddList.TimeFinish, real_time, 4)>=0))			printf("%s:新增黑名单已过期，无法查询本地黑名单!\n", __FUNCTION__);

	if(!(0==timeVerification(pcdDelList.TimeStart, 4)))					printf("%s:新增黑名单启用日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdDelList.TimeFinish, 4)))					printf("%s:新增黑名单截止日期非法，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdDelList.TimeStart, real_time, 4)<=0))				printf("%s:新增黑名单未启用，无法查询本地黑名单!\n", __FUNCTION__);
	if(!(memcmp(pcdDelList.TimeFinish, real_time, 4)>=0))				printf("%s:新增黑名单已过期，无法查询本地黑名单!\n", __FUNCTION__);
	//*/
	if(1==card_type &&\
		PCD_DOWN_BASELIST!=pcdParam.PcDownloadContent &&\
		PCD_DOWN_ADDLIST!=pcdParam.PcDownloadContent &&\
		PCD_DOWN_DELLIST!=pcdParam.PcDownloadContent &&\
		card_area==pcdStationInfo.Province &&\
		0==timeVerification(real_time, 7) &&\
		0==timeVerification(pcdBaseList.TimeStart, 4) && 0==timeVerification(pcdBaseList.TimeFinish, 4) &&\
		memcmp(pcdBaseList.TimeStart, real_time, 4)<=0 && memcmp(pcdBaseList.TimeFinish, real_time, 4)>=0 &&\
		0==timeVerification(pcdAddList.TimeStart, 4) && 0==timeVerification(pcdAddList.TimeFinish, 4) &&\
		memcmp(pcdAddList.TimeStart, real_time, 4)<=0 && memcmp(pcdAddList.TimeFinish, real_time, 4)>=0 &&\
		0==timeVerification(pcdDelList.TimeStart, 4) && 0==timeVerification(pcdDelList.TimeFinish, 4) &&\
		memcmp(pcdDelList.TimeStart, real_time, 4)<=0 && memcmp(pcdDelList.TimeFinish, real_time, 4)>=0)
	{
		//新删黑名单查到则返回非黑名单卡
		number=(pcdDelList.Number[0]<<24)|(pcdDelList.Number[1]<<16)|(pcdDelList.Number[2]<<8)|(pcdDelList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_DELLIST, number, card_number))
		{
			*isMatch=1;
			return 0;
		}

		//新增黑名单查到则返回黑名单卡
		number=(pcdAddList.Number[0]<<24)|(pcdAddList.Number[1]<<16)|(pcdAddList.Number[2]<<8)|(pcdAddList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_ADDLIST, number, card_number))
		{
			*isMatch=0;
			return 0;
		}

		//基础黑名单查到则为黑名单卡，否则为非黑名单卡
		number=(pcdBaseList.Number[0]<<24)|(pcdBaseList.Number[1]<<16)|(pcdBaseList.Number[2]<<8)|(pcdBaseList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_BASELIST, number, card_number))
		{
			*isMatch=0;
		}
		else
		{
			*isMatch=1;
		}

		return 0;
	}

	return ERROR;
}


/********************************************************************
*Name				:pcdLoaclGreyBillGet
*Description		:获取灰卡交易记录信息，可能会有一定的超时等待时间
*Input				:phynozzle	物理枪号
*						:inbuffer		输入缓存，需要查询的灰锁交易部分信息
*						:					卡号(10压缩BCD) +余额(4HEX) +CTC(2HEX) +扣款来源(1HEX) +日期及时间(7压缩BCD)
*						:nbytes			输入缓存长度
*						:maxbytes	输出缓存最大长度
*Output			:outbuffer		输出缓存，查询到的灰锁交易信息
*						:					卡号(10压缩BCD) +余额(4HEX) +交易额(3HEX) +CTC(2HEX) 
*						:					+扣款来源(1HEX) +日期及时间(7压缩BCD) +解灰认证码(4Bin) +PSAM应用编号(6Bin) +PSAM的TTC(4Bin)
*Return				:0=成功；其它=错误
*History			:2013-08-05,modified by syj
*/
static int pcdLoaclGreyBillGet(int phynozzle, char *inbuffer, char *outbuffer)
{
	int i=0;
	unsigned char read_buffer[128]={0};
	unsigned int pos_ttc=0, last_ttc=0, serch_number = 0;

	if(pcdParam.TTC<1) //当前无账单，返回失败
	{
		return ERROR;
	}

	if(pcdParam.AbnormalNumber<1) //当前无异常账单，返回失败
	{
		return ERROR;
	}

	/*根据异常账单索引中记录的索引信息，从最后一笔异常账单倒查数据
	*	联网状态时查找到最后未上传的第一笔信息即止；
	*	断网状态时查找到当前存储的最早一笔信息即止；
	*/
	if(PCD_PC_ONLINE==pcdParam.PcOnline && 0==pcdParam.UnloadNumber)
	{
		return ERROR;
	}
	else if(PCD_PC_ONLINE==pcdParam.PcOnline && 0!=pcdParam.UnloadNumber)
	{
		last_ttc=pcdParam.TTC-pcdParam.UnloadNumber+1;
	}
	else if(PCD_PC_ONLINE!=pcdParam.PcOnline && pcdParam.TTC<=PCD_RECORD_MAX)
	{
		last_ttc=1;
	}
	else if(PCD_PC_ONLINE!=pcdParam.PcOnline && pcdParam.TTC>PCD_RECORD_MAX)
	{
		last_ttc=pcdParam.TTC-PCD_RECORD_MAX+1;
	}

	//判断如果异常账单过多，最多查询最近的500笔 修改于2016-04-01
	if(pcdParam.AbnormalNumber%PCD_ZD_INDEX_MAX > 500)	
	{
		serch_number = 500%PCD_ZD_INDEX_MAX;
	}
	else	
	{																						
		serch_number = pcdParam.AbnormalNumber%PCD_ZD_INDEX_MAX;
	}

	//在有效范围内查询异常账单
	for(i=0; i<(serch_number%PCD_ZD_INDEX_MAX); i++)
	{
		/*根据异常账单索引计算计算要查询的账单TTC*/
		fileReadForPath(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, ((pcdParam.AbnormalNumber-1-i)%PCD_ZD_INDEX_MAX)*PCD_ZDERRINFO_SIZE, read_buffer, 16);
		pos_ttc=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

		/*根据此TTC读取相应位置信息
		*	卡号一致
		*	余额一致CTC一致扣款来源一致时间一致
		*/
		fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), read_buffer, 95);
		if((0==memcmp(&inbuffer[0], &read_buffer[12], 10))&&\
			(0==memcmp(&inbuffer[10], &read_buffer[22], 4))&&\
			(0==memcmp(&inbuffer[14], &read_buffer[29], 2))&&\
			(inbuffer[16]==read_buffer[63])&&\
			(0==memcmp(&inbuffer[17], &read_buffer[5], 7))){
	
			memcpy(&outbuffer[0], &read_buffer[12], 10);						//卡号
			memcpy(&outbuffer[10], &read_buffer[22], 4);						//余额
			
			//2017-01-22中燃联达解扣折扣有问题, fj:iptIsLianda函数在oilIpt.c里
			if(1 == iptIsLianda(IptParamA.Id)||1 == iptIsLianda(IptParamB.Id))
			{
				memcpy(&outbuffer[14], &read_buffer[80], 3);						//交易额
			}
			else
			{
				memcpy(&outbuffer[14], &read_buffer[26], 3);						//交易额
			}
				
			memcpy(&outbuffer[17], &read_buffer[29], 2);						//CTC
			memcpy(&outbuffer[19], &read_buffer[63], 1);						//扣款来源
			memcpy(&outbuffer[20], &read_buffer[5], 7);							//时间
			memcpy(&outbuffer[27], &read_buffer[35], 4);						//解灰认证码
			memcpy(&outbuffer[31], &read_buffer[53], 6);						//PSAM编号
			memcpy(&outbuffer[37], &read_buffer[59], 4);						//PSAM的TTC

			return 0;
		}

		//已查找完最早的一笔，退出
		if(pos_ttc<=last_ttc)
		{
			break;
		}

		 //taskDelay(0); //fj:?
	}

	return ERROR;
}


/****************************************************************************
*Name				:pcdLocalWBListGet
*Description		:以某TTC为基础，查询指定物理油枪上/下一笔TTC
*						:phynozzle	物理枪号
*						:base_ttc		基础TTC，以此TTC为基础本枪查询上一笔或下一笔，0表示最近一笔
*						:handle			0=查询当笔账单；1=查询基础TTC上一笔；2=查询基础TTC下一笔
*Output			:isMatch		匹配标识(1Bin, b0=0:匹配/其它:不匹配) 
*Return				:要查询的TTC；无此账单返回0
*History			:2013-08-05,modified by syj
*/
unsigned int pcdLocalPhsicalTTCGet(int phynozzle, unsigned int base_ttc, char handle)
{
	char local_bill_buffer[PCD_ZD_INDEX_SIZE]={0}, next_bill_buffer[PCD_ZD_INDEX_SIZE]={0};
	off_t ttc_offset=0;					  //当次查询索引文件的偏移
	unsigned int ireturn=0;				//返回值
	unsigned int number=0;			  //该物理油枪的账单数目
	unsigned int last_ttc=0;			//该物理油枪当前TTC
	unsigned int i_base_ttc=0;		//以此TTC为基础
	unsigned int tmp_ttc=0;			  //读到索引文件中的TTC
	int fd=ERROR, i=0;

	//判断物理枪号合法性
	if(!(phynozzle>=1 && phynozzle<=PCD_NOZZLE_MAX))
	{
		return 0;
	}

	//计算账单总数及TTC
	if(1==phynozzle){number=pcdParam.ZDNumber1;	last_ttc=pcdParam.TTC1;}
	if(2==phynozzle){number=pcdParam.ZDNumber2;	last_ttc=pcdParam.TTC2;}
	if(3==phynozzle){number=pcdParam.ZDNumber3;	last_ttc=pcdParam.TTC3;}
	if(4==phynozzle){number=pcdParam.ZDNumber4;	last_ttc=pcdParam.TTC4;}
	if(5==phynozzle){number=pcdParam.ZDNumber5;	last_ttc=pcdParam.TTC5;}
	if(6==phynozzle){number=pcdParam.ZDNumber6;	last_ttc=pcdParam.TTC6;}

	//如果基础TTC为0则表示查询最近一笔账单
	if(0==base_ttc)	
		i_base_ttc=last_ttc;
	else						
		i_base_ttc=base_ttc;

	//判断本枪无账单，返回无此账单
	if(number<=0)
	{
		return 0;
	}

	//判断查询TTC超过当前TTC，返回无此账单
	if(base_ttc>last_ttc)
	{
		return 0;
	}

	//判断查询当前最大TTC的下一笔，返回无此账单
	if(2==handle && base_ttc>=last_ttc)
	{
		return 0;
	}

	printf("pcdLocalPhsicalTTCGet ffffffffff,phynozzle = %d\n",phynozzle);

	//打开记录文件
	if(1==phynozzle)
	{
		fd=fileOpen(PCD_FILE_ZDINDEX_1, O_RDWR , S_IREAD | S_IWRITE);
	}
	
	if(2==phynozzle)	{fd=fileOpen(PCD_FILE_ZDINDEX_2, O_RDWR , S_IREAD | S_IWRITE);}
	if(3==phynozzle)	{fd=fileOpen(PCD_FILE_ZDINDEX_3, O_RDWR , S_IREAD | S_IWRITE);}
	if(4==phynozzle){fd=fileOpen(PCD_FILE_ZDINDEX_4, O_RDWR , S_IREAD | S_IWRITE);}
	if(5==phynozzle){fd=fileOpen(PCD_FILE_ZDINDEX_5, O_RDWR , S_IREAD | S_IWRITE);}
	if(6==phynozzle){fd=fileOpen(PCD_FILE_ZDINDEX_6, O_RDWR , S_IREAD | S_IWRITE);}

	//printf("pcdLocalPhsicalTTCGet ffffffffff-----,fd----- = %d\n",fd);

	if(ERROR==fd)
	{
		return 0;
	}

	//printf("pcdLocalPhsicalTTCGet ggggggggg\n");


	//搜索索引文件内容，搜索不超过最大索引数目及当前索引数目
	for(i=0; i<PCD_ZD_INDEX_MAX && i<number; )
	{
		ttc_offset=PCD_ZD_INDEX_SIZE*((number-1-i)%PCD_ZD_INDEX_MAX); //计算要查询文件偏移位置
		if(ERROR==fileRead(fd, ttc_offset, local_bill_buffer, PCD_ZD_INDEX_SIZE)) //查找基础TTC所在的位置
		{
			fileClose(fd);
			return 0;
		}

		tmp_ttc=(local_bill_buffer[0]<<24)|(local_bill_buffer[1]<<16)|(local_bill_buffer[2]<<8)|(local_bill_buffer[3]<<0); //查找基础TTC
		if(i_base_ttc==tmp_ttc)
		{	
			if(0==handle)			 //根据操作判断查找该笔账单还是上一笔或下一笔
			{
				ireturn=tmp_ttc;	
				break;
			}
			else if(1==handle)
			{	
				 i++;
			}
			else
			{							
				i--;
			}

			if(i>=PCD_ZD_INDEX_MAX || i>=number) //判断搜索目标是否合法
			{
				fileClose(fd);
				return 0;
			}

			ttc_offset=PCD_ZD_INDEX_SIZE*((number-1-i)%PCD_ZD_INDEX_MAX); //计算要查询文件偏移位置
			
			if(ERROR==fileRead(fd, ttc_offset, local_bill_buffer, PCD_ZD_INDEX_SIZE)) //查找基础TTC所在的位置
			{ 
				fileClose(fd);
				return 0;
			}
			//查找到的TTC
			ireturn=(local_bill_buffer[0]<<24)|(local_bill_buffer[1]<<16)|(local_bill_buffer[2]<<8)|(local_bill_buffer[3]<<0);
			break;
		}

		//搜索数目累加
		i++;
	}

	int nFileClose = fileClose(fd);

	//printf("pcdLocalPhsicalTTCGet hhhhhhhhhh,nFileClose = %d\n",nFileClose);

	//fileClose(fd); //关闭文件
	return ireturn;
}


/********************************************************************
*Name				:pcd2IptProcess
*Description		:PCD与IPT间通讯处理
*Input				:None
*Output			:None
*Return				:None
*History			:2014-03-25,modified by syj
*/
static void pcd2IptProcess()
{
	/*PCD与支付终端(IPT)之间通讯
	*	IPT平时通过轮询命令将实时信息发送到PCD，PCD将实时信息进行缓存
	*	某条支付终端长时间无数据上送时认为此支付终端无效；
	*/
	PcdFuellingPointInfoType *fp_info=NULL;				//支付终端信息
	PcdPcInfoType pc_info;											  //后台PC数据信息
	unsigned char msg_type=0;									    //消息类型
	unsigned char mboard_id=0;								    //物理主板号
	unsigned char fuelling_point=0;							  //物理面板号
	unsigned char physical_nozzle=0;						  //物理枪号
	unsigned char command=0;									    //命令字
	unsigned char msg_buffer[PCD_MSGMAX]={0}; 	  //接收的消息数据
	int msg_len=0;													      //接收的消息数据长度
	unsigned char tx_buffer[PCD_MSGMAX]={0};		  //发送时数据缓存
	int tx_len=0;														      //发送数据长度
	unsigned char tmp_buffer[128]={0};					  //用于存储临时数据
	unsigned int pos_ttc=0;										    //申请到的账单TTC
	unsigned int unload_number=0;							    //本加油点未上传账单数量
	unsigned int fuel_ttc=0;										  //本加油点当前TTC
	int i=0, istate=0;
	int voice[SPK_MAX_NUMBER]={0}, voice_len=0;		//语音列表及语音列表数目
	//int voice[32]={0}, voice_len=0;		//语音列表及语音列表数,fj:修改

	//判断某条枪超时2分钟无连接则判定此枪离线，不再主动向此枪发送数据
	if(pcdParam.FP_1.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_1.IsOnline=PCD_FP_OFFLINE;
	if(pcdParam.FP_2.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_2.IsOnline=PCD_FP_OFFLINE;
	if(pcdParam.FP_3.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_3.IsOnline=PCD_FP_OFFLINE;
	if(pcdParam.FP_4.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_4.IsOnline=PCD_FP_OFFLINE;
	if(pcdParam.FP_5.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_5.IsOnline=PCD_FP_OFFLINE;
	if(pcdParam.FP_6.OfflineTimer>=2*60*ONE_SECOND)	pcdParam.FP_6.IsOnline=PCD_FP_OFFLINE;

	//printf("pcd ------\n");

	//fj:
	struct msg_struct msg_stPcdRx;
	msg_stPcdRx.msgType = 1;
    memset(msg_stPcdRx.msgBuffer,0,1024);
	msg_len = msgrcv(pcdParam.msgIdFromIpt,&msg_stPcdRx,1024,0,IPC_NOWAIT);
    //printf("msg_len = %d\n",msg_len);

	//接收IPT数据
	//msg_len=msgQReceive(pcdParam.msgIdFromIpt, msg_buffer, PCD_MSGMAX, NO_WAIT); //fj:该函数之后再处理
	
	if(msg_len<0)	 //未接收到消息时不作继续处理
		return;

	memcpy(msg_buffer,msg_stPcdRx.msgBuffer,msg_len);

	//printf("ipc data:\n");
	//PrintH(msg_len,msg_stPcdRx.msgBuffer);

/*/
////////////////////////////////////////////
	printf("PCD_MSG_FROM_IPT:");
	for(i=0; i<msg_len; i++)	printf("_%x", msg_buffer[i]);
	printf("\n");
/////////////////////////////////////////////
//*/

	//解析帧号，主板号，物理面板号，物理枪号，命令字
	msg_type=msg_buffer[0];	mboard_id=msg_buffer[1];	fuelling_point=msg_buffer[2];
	physical_nozzle=msg_buffer[3];	command=msg_buffer[4];

	//printf("command = %d\n",command);

	//判断处理的物理枪号
	if(1==physical_nozzle)			
	{fp_info=&pcdParam.FP_1;	unload_number=pcdParam.UnloadNumber1;	fuel_ttc=pcdParam.TTC1;}
	else if(2==physical_nozzle)	
	{fp_info=&pcdParam.FP_2;	unload_number=pcdParam.UnloadNumber2;	fuel_ttc=pcdParam.TTC2;}
	else if(3==physical_nozzle)	
	{fp_info=&pcdParam.FP_3;	unload_number=pcdParam.UnloadNumber3;	fuel_ttc=pcdParam.TTC3;}
	else if(4==physical_nozzle)	
	{fp_info=&pcdParam.FP_4;	unload_number=pcdParam.UnloadNumber4;	fuel_ttc=pcdParam.TTC4;}
	else if(5==physical_nozzle)	
	{fp_info=&pcdParam.FP_5;	unload_number=pcdParam.UnloadNumber5;	fuel_ttc=pcdParam.TTC5;}
	else if(6==physical_nozzle)	
	{fp_info=&pcdParam.FP_6;	unload_number=pcdParam.UnloadNumber6;	fuel_ttc=pcdParam.TTC6;}
	else	return;

	//记录该物理油枪为在线
	fp_info->OfflineTimer=0;	
	fp_info->IsOnline=PCD_FP_ONLINE;	
	fp_info->PhysicalNozzle=physical_nozzle;
	
	//轮寻命令记录油枪信息并返回数据
	if(PCD_CMD_POLL==command)
	{
		fp_info->NozzleNumber=msg_buffer[5]; //油枪数
		for(i=0; i<3 && i<fp_info->NozzleNumber; i++) //保存各支付终端信息
		{
			memcpy(&fp_info->Nozzle[i].State, &msg_buffer[6+i*37], 1);					//记录IPT状态
			memcpy(&fp_info->Nozzle[i].LogicNozzle, &msg_buffer[7+i*37], 1);		//记录逻辑枪号
			memcpy(&fp_info->Nozzle[i].SumMoney, &msg_buffer[8+i*37], 4);			  //记录总累金额
			memcpy(&fp_info->Nozzle[i].SumVolume, &msg_buffer[12+i*37], 4);		  //记录总累油量
			memcpy(&fp_info->Nozzle[i].CardID, &msg_buffer[16+i*37], 10);			  //记录卡应用号
			memcpy(&fp_info->Nozzle[i].CardState, &msg_buffer[26+i*37], 2);			//记录卡状态
			memcpy(&fp_info->Nozzle[i].CardBalance, &msg_buffer[28+i*37], 4);		//记录卡余额
			memcpy(&fp_info->Nozzle[i].PayUnit, &msg_buffer[32+i*37], 1);				//记录结算单位/方式		
			memcpy(&fp_info->Nozzle[i].Money, &msg_buffer[33+i*37], 4);				  //记录加油金额		
			memcpy(&fp_info->Nozzle[i].Volume, &msg_buffer[36+i*37], 4);				//记录加油金额
			memcpy(&fp_info->Nozzle[i].Price, &msg_buffer[39+i*37], 2);					//记录加油价格
		}

		//组织后台PC数据
		memcpy(&pc_info.SInfo, &pcdStationInfo, sizeof(PcdStationInfoType));
		memcpy(&pc_info.BList, &pcdBaseList, sizeof(PcdListInfoType));
		memcpy(&pc_info.ABList, &pcdAddList, sizeof(PcdListInfoType));
		memcpy(&pc_info.DBList, &pcdDelList, sizeof(PcdListInfoType));
		memcpy(&pc_info.WList, &pcdWhiteList, sizeof(PcdListInfoType));
		memcpy(&pc_info.OilInfo, &pcdOilInfo, sizeof(PcdOilRecordType));
		memcpy(&pc_info.Time, pcdParam.PcTime, 7);
	
		//返回应用数据
		tx_buffer[0]=0;											//执行结果
		tx_buffer[1]=pcdParam.PcOnline;			//PC后台在线状态
		tx_buffer[2]=0;											//异常代码
																		    //整机当前TTC
		tx_buffer[3]=(char)(pcdParam.TTC>>24);	tx_buffer[4]=(char)(pcdParam.TTC>>16);
		tx_buffer[5]=(char)(pcdParam.TTC>>8);	tx_buffer[6]=(char)(pcdParam.TTC>>0);
																		
		tx_buffer[7]=(char)(fuel_ttc>>24);	tx_buffer[8]=(char)(fuel_ttc>>16); //本枪当前TTC
		tx_buffer[9]=(char)(fuel_ttc>>8);		tx_buffer[10]=(char)(fuel_ttc>>0);
																		
		tx_buffer[11]=(char)(pcdParam.UnloadNumber>>24);	tx_buffer[12]=(char)(pcdParam.UnloadNumber>>16); //整机未上传账单数目
		tx_buffer[13]=(char)(pcdParam.UnloadNumber>>8);	tx_buffer[14]=(char)(pcdParam.UnloadNumber>>0);
																		
		tx_buffer[15]=(char)(unload_number>>24);	tx_buffer[16]=(char)(unload_number>>16); //本枪未上传账单数目
		tx_buffer[17]=(char)(unload_number>>8);	tx_buffer[18]=(char)(unload_number>>0);
		i=sizeof(PcdPcInfoType);							//PC信息长度
		tx_buffer[19]=(unsigned char)(i>>8);
		tx_buffer[20]=(unsigned char)(i>>0);
		memcpy(&tx_buffer[21], &pc_info, sizeof(PcdPcInfoType));	//PC信息
		tx_len=21+i;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//申请TTC命令
	if(PCD_CMD_FORTTC==command)
	{
		istate=pcdLocalTTCGet(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN, &pos_ttc); //本地申请TTC

		char chTemp[8] = {0};
		sprintf(chTemp,"%d",istate);
		//g_fjLog.WriteLog("pcd2IptProcess  ","pcdLocalTTCGet  ",chTemp,0);  //fj:20171027
		//printf("pcdLocalTTCGet: istate = %d",istate);

		//返回TTC
		if(0==istate)	
			tx_buffer[0]=0;						//结果
		else				
			tx_buffer[0]=1;
			
		tx_buffer[1]=(char)(pos_ttc>>24);			//申请得到的TTC
		tx_buffer[2]=(char)(pos_ttc>>16);
		tx_buffer[3]=(char)(pos_ttc>>8);	
		tx_buffer[4]=(char)(pos_ttc>>0);
		memcpy(&tx_buffer[5], &msg_buffer[5+4], PCD_ZD_LEN-4);	//TTC之后自交易类型起的账单数据
		tx_len=5+PCD_ZD_LEN-4;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//保存账单命令
	if(PCD_CMD_ZDSAVE==command)
	{
		
		istate=pcdLocalBillSave(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN); //本地存出账单

		//返回TTC结果
		if(0==istate)	
			tx_buffer[0]=0;							
		else				
			tx_buffer[0]=1;
			
		memcpy(&tx_buffer[1], &msg_buffer[5], 4);	//存储的账单TTC
		tx_len=5;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//查询黑白名单命令
	if(PCD_CMD_LIST==command)
	{
		if(ICTYPE_USER==msg_buffer[5+2]) //fj:ICTYPE_USER,在oilIC.h
		{
			/*用户卡查找黑名单
			*	查找成功，黑名单匹配，直接返回查找结果
			*	查找成功，黑名单不匹配，联网状态向后台查
			*	查找成功，黑名单不匹配，非联网状态卡内最后一笔加油日期早于基础黑生效日期则提示需联机查询；
			*	查找成功，黑名单不匹配，非联网状态卡内最后一笔加油日期不早于基础黑生效日期则允许加油，即返回不匹配；
			*	查找失败，联网状态向后台查
			*	查找失败，非联网状态直接返回
			*/
			printf("ICTYPE_USER:  pcdLoaclWBListGet \n");
			istate=pcdLoaclWBListGet(physical_nozzle, &msg_buffer[5], 14, tmp_buffer);
			if(0==istate && 0==tmp_buffer[0]) //查找成功，黑名单匹配，直接返回查找结果
			{
				tx_buffer[0]=0;													    //结果0=查找成功；其它=需联机查询
				tx_buffer[1]=1;													    //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								  //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE==pcdParam.PcOnline) //查找成功，黑名单不匹配，联网状态向后台查
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			//查找成功，黑名单不匹配，非联网状态卡内最后一笔加油日期早于基础黑生效日期则提示需联机查询；
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline && memcmp(&msg_buffer[15], pcdBaseList.TimeStart, 4)<0)
			{
				tx_buffer[0]=1;													     //结果0=查找成功；其它=需联机查询
				tx_buffer[1]=1;													     //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								   //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	 //卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			//查找成功，黑名单不匹配，非联网状态卡内最后一笔加油日期不早于基础黑生效日期则允许加油，即返回不匹配；
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline && memcmp(&msg_buffer[15], pcdBaseList.TimeStart, 4)>=0)
			{
				tx_buffer[0]=0;													    //结果0=查找成功；其它=需联机查询
				tx_buffer[1]=1;													    //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								  //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if(0!=istate && PCD_PC_ONLINE==pcdParam.PcOnline) //查找失败，联网状态向后台查
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			else if(0!=istate && PCD_PC_ONLINE!=pcdParam.PcOnline) //查找失败，非联网状态直接返回
			{
				tx_buffer[0]=1;													    //结果0=查找成功；其它=需联机查询
				tx_buffer[1]=1;													    //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								  //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
		}
		else
		{
			/*内部卡查询白名单
			*	如果查找成功且匹配，直接返回查到；
			*	如果查找成功且不匹配，断网状态返回不匹配的查找结果；
			*	如果查找失败或不匹配且联网断开状态则直接返回需联机查询；
			*	如果查找失败且联网正常则向后台查询；
			*/
        	printf("----not ICTYPE_USER:  pcdLoaclWBListGet \n");
			istate=pcdLoaclWBListGet(physical_nozzle, &msg_buffer[5], 14, tmp_buffer);
			if((0==istate && 0==tmp_buffer[0])|| (0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline))
			{
				tx_buffer[0]=0;													    //结果0=查找成功；其它=需联机查询
				tx_buffer[1]=1;													    //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								  //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if((0!=istate || 0!=tmp_buffer[0]) && PCD_PC_ONLINE==pcdParam.PcOnline)
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			else
			{	
				tx_buffer[0]=1;													    //结果0=查到；其它=需联机查询
				tx_buffer[1]=1;													    //查找源0=后台黑/白名单;1=本地黑/白名单
				tx_buffer[2]=tmp_buffer[0];								  //匹配标识
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//卡号
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
		}
	}

	//查询灰记录命令
	if(PCD_CMD_GREYINFO==command)
	{	
		/*首先在本地查找，如果找到则直接返回，
		*	本地未找到未找到匹配记录，联网状态向后台查询
		*	本地未找到未找到匹配记录，断网状态返回无匹配记录
		*/
		istate=pcdLoaclGreyBillGet(physical_nozzle, &msg_buffer[5], tmp_buffer);
		if(0==istate)
		{
			tx_buffer[0]=0x01;												//查找源0=后台；1=本地
			tx_buffer[1]=0;														//查找成功返回查找结果，成功
			memcpy(&tx_buffer[2], tmp_buffer, 41);		//查找到的数据	
			tx_len=43;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
 		else if(PCD_PC_ONLINE==pcdParam.PcOnline)
 		{
			pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 24);
		}
		else
		{
			tx_buffer[0]=0x01;												//查找源0=后台；1=本地
			tx_buffer[1]=1;														//查找失败返回无匹配项，失败
			memset(&tx_buffer[2], 0, 41);							//查找到的数据清零
			tx_len=43;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}

	//打印数据命令
	if(PCD_CMD_PRINTE==command)
	{
		
		/*IPT将打印数据发送到PCD，打印机选择b7~b4代表主板号,b3~b0代表支付终端号
		*	打印机选择为本主板连接的打印机时直接将数据发送到打印机
		*	打印机选择非本地主板时将数据转发到其它主板
		*/
		i=(msg_buffer[5]>>0)&0x0f;
		
		if(0==i)	
			comWrite(pcdParam.comFdPrint1, &msg_buffer[8], (msg_buffer[6]<<8)|(msg_buffer[7]<<0)); //fj:在别的文件里
		else			
			comWrite(pcdParam.comFdPrint2, &msg_buffer[8], (msg_buffer[6]<<8)|(msg_buffer[7]<<0));
	}

	//语音播放命令
	if(PCD_CMD_SPK==command)
	{
		i=(msg_buffer[5]>>0)&0x0f;
		for(voice_len=0; voice_len<32 && voice_len<msg_buffer[7]; voice_len++) //fj:SPK_MAX_NUMBER,32
		{
			voice[voice_len]=(msg_buffer[8+2*voice_len]<<8)|(msg_buffer[9+2*voice_len]<<0);
		}
		if(0==i)	//fj:spkPlay在oilSpk.c里实现的。
			spkPlay(SPK_NOZZLE_1, msg_buffer[6], voice, voice_len);
		else			
			spkPlay(SPK_NOZZLE_2, msg_buffer[6], voice, voice_len);
	}

//	printf("command = %d,msg_buffer[5] = %d\n",command,msg_buffer[5]);

	//查询账单，查询整机账单
	if(PCD_CMD_ZDCHECK==command && 0==msg_buffer[5])
	{
		printf("zhengji ---- pos_ttc = %d,physical_nozzle = %d\n",pos_ttc,physical_nozzle);

		pos_ttc=(msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0);
		if(0==pos_ttc)				pos_ttc=pcdParam.TTC;
		if(1==msg_buffer[10])	pos_ttc--;
		if(2==msg_buffer[10])	pos_ttc++;

		//要查询的TTC查过范围，没有此笔账单
		if(pos_ttc<1 || pos_ttc>pcdParam.TTC || (pcdParam.TTC>PCD_RECORD_MAX && pos_ttc<=(pcdParam.TTC-PCD_RECORD_MAX)))
		{
			tx_buffer[0]=1;
			tx_len=1;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
		else //要查询的TTC在范围内则返回账单数据
		{
			tx_buffer[0]=0;
			fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), &tx_buffer[1], PCD_ZD_LEN);
			tx_len=1+PCD_ZD_LEN;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}	
	else if(PCD_CMD_ZDCHECK==command && msg_buffer[5]>=1 && msg_buffer[5]<=PCD_NOZZLE_MAX) //查询账单，查询单枪账单
	{
		int base_ttc = (msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0);
		printf("pos_ttc = %d,physical_nozzle = %d,base_ttc = %d,handle = %d\n",pos_ttc,physical_nozzle,base_ttc,msg_buffer[10]);

		//查询指定油枪的TTC
		pos_ttc=pcdLocalPhsicalTTCGet(msg_buffer[5], (msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0), msg_buffer[10]);

		//要查询的TTC查过范围，没有此笔账单
		if(pos_ttc<1 || pos_ttc>pcdParam.TTC || (pcdParam.TTC>PCD_RECORD_MAX && pos_ttc<=(pcdParam.TTC-PCD_RECORD_MAX)))
		{
			tx_buffer[0]=1;
			tx_len=1;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
		else //要查询的TTC在范围内则返回账单数据
		{
			tx_buffer[0]=0;
			fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), &tx_buffer[1], PCD_ZD_LEN);
			tx_len=1+PCD_ZD_LEN;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}

	if(PCD_CMD_BARCODE == command) //IPT向后台转发的条码操作命令
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[7], (msg_buffer[5]<<8)|(msg_buffer[6]<<0));
	}

	if(PCD_CMD_FOR_TMAC==command) //IPT返回重新计算TMAC的账单
	{
		istate=pcdLocalBillSave(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN);
	}

	if(PCD_CMD_ERRINFO_UPLOAD==command) //IPT通过PCD上传内部出错信息到后台
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[7], (msg_buffer[5]<<8)|(msg_buffer[6]<<0));
	}

	if(PCD_CMD_DISCOUNT_ASK == command) //IPT通过PCD向后台申请折扣额
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 11);
	}


	return;
}


/********************************************************************
*Name				:pcd2PcProcess
*Description		:PCD与PC间通讯处理
*Input				:None
*Output			:None
*Return				:None
*History			:2014-03-25,modified by syj
*/
static void pcd2PcProcess()
{
	/*加油机与PC间通讯
	*
	*	1.对于发送与接收的同步处理
	*	加油机为主动方，PC为被动方，加油机在一定时间内只发送一条主动命令
	*	直到PC返回对应信息或无返回超时，允许发送下次命令；
	*	例外情况是IPT有主动的查询黑/白名单或灰记录的命令直接向后台发送；
	*
	*	2.关于帧号，每次发送一次加油机自动累加，当PC返回的帧号与当前帧号不一致时
	*	加油机帧号同步后台帧号，帧号在解析判断命令时并不使用
	*
	*	3.普通查询命令
	*  以一定时间间隔向后台发送普通查询命令，后台返回时立即上送实时数据
	*
	*	4.后台有主动的命令时根据具体命令返回数据
	*
	*	5.数据接收任务接收数据后将"有效数据"通过消息发送到本处处理
	*/
	unsigned char msg_buffer[PCD_MSGMAX]={0};		  //接收的消息数据
	int msg_len=0;														    //接收的消息数据长度
	unsigned char tx_buffer[PCD_PCDATA_MAX]={0};	//发送数据缓存
	int tx_len=0;															    //发 送数据长度
	unsigned char read_buffer[256]={0};						//读取的数据
	int read_len=0;														    //读取的数据长度
	unsigned char tmp_buffer[128]={0};						//临时使用的缓存
	unsigned int tmp_int1=0;											//临时使用的整形数据
	PcdFuellingPointInfoType *fp_info=NULL;				//支付终端信息
	RTCTime time;															    //时间信息
	unsigned int pos_ttc=0;											  //账单TTC
	int i=0, j=0;
	char is_bill_upload=0;												//是否有账单需要上传,0=无，非0=有
	off_t download_offset=0;										  //接收到的下载数据偏移
	int download_bytes=0;											    //接收到的下载数据长度
	static char command=0;											  //当前处理的数据命令
	unsigned char rolldata[50]={0};                 //szb_fj_20171120:
	

	struct msg_struct msg_stPcdRx;
	msg_stPcdRx.msgType = 1;
    memset(msg_stPcdRx.msgBuffer,0,512);
	msg_len = msgrcv(pcdParam.msgIdFromPc,&msg_stPcdRx,512,0,IPC_NOWAIT|MSG_NOERROR);
	if(msg_len > 0)
	{
		memcpy(msg_buffer,msg_stPcdRx.msgBuffer,msg_len);
	
		//printf("pc data \n");
		//PrintH(msg_len,msg_buffer);
	}
	//else
	//{
	//	printf("recv pc data ,len = %d\n",msg_len);
	//}


	//printf("jjjjjjjjjjjjjjj\n");

	/*********************************************************************
			处理PC主动数据命令
			PCD2PC_CMD_SUMREAD:		PC读取加油机累计数命令
			PCD2PC_CMD_INFOREAD:	PC读取加油机信息命令
			PCD2PC_CMD_ZDREAD:		PC读取加油数据命令
	**********************************************************************/
	
	//PC读取加油机累计数命令，将所有在线有效的逻辑油枪累计上送
	if(msg_len>0 && PCD2PC_CMD_SUMREAD==msg_buffer[0])
	{
		tx_buffer[0]=PCD2PC_CMD_SUMREAD;
		tx_buffer[1]=0;
		tx_len=2;
		for(i=0; i<PCD_NOZZLE_MAX; i++)
		{
			if(0==i && PCD_FP_ONLINE==pcdParam.FP_1.IsOnline)		fp_info=&pcdParam.FP_1;
			else if(1==i && PCD_FP_ONLINE==pcdParam.FP_2.IsOnline)	fp_info=&pcdParam.FP_2;
			else if(1==i && PCD_FP_ONLINE==pcdParam.FP_3.IsOnline)	fp_info=&pcdParam.FP_3;
			else if(1==i && PCD_FP_ONLINE==pcdParam.FP_4.IsOnline)	fp_info=&pcdParam.FP_4;
			else if(1==i && PCD_FP_ONLINE==pcdParam.FP_5.IsOnline)	fp_info=&pcdParam.FP_5;
			else if(1==i && PCD_FP_ONLINE==pcdParam.FP_6.IsOnline)	fp_info=&pcdParam.FP_6;
			else				break;

			for(j=0; j<3 && j<fp_info->NozzleNumber; j++)
			{
				tx_buffer[tx_len+0]=fp_info->Nozzle[j].LogicNozzle;
				memcpy(&tx_buffer[tx_len+1], fp_info->Nozzle[j].SumVolume, 4);
				tx_len+=5;	tx_buffer[1]++;
			}
		}

		pcd2PcSend(tx_buffer, tx_len);
	}
	//PC读取加油机信息命令
	if(msg_len>0 && PCD2PC_CMD_INFOREAD==msg_buffer[0])
	{
		tx_len=0;
		tx_buffer[tx_len]=PCD2PC_CMD_INFOREAD;
		tx_len+=1;
		memcpy(&tx_buffer[tx_len], PCD_MBOARD_INFO, 12);
		tx_len+=12;
		tx_buffer[tx_len]=pcdStationInfo.Province;
		tx_len+=1;
		tx_buffer[tx_len]=pcdStationInfo.City;
		tx_len+=1;
		memcpy(&tx_buffer[tx_len], pcdStationInfo.Superior, 4);
		tx_len+=4;
		memcpy(&tx_buffer[tx_len], pcdStationInfo.S_ID, 4);
		tx_len+=4;
		timeRead(&time); //fj:在别的.c文件里

		tx_buffer[tx_len++]=time.century;	tx_buffer[tx_len++]=time.year;	tx_buffer[tx_len++]=time.month;
		tx_buffer[tx_len++]=time.date;		tx_buffer[tx_len++]=time.hour;	tx_buffer[tx_len++]=time.minute;
		tx_buffer[tx_len++]=time.second;
		if(pcdStationInfo.GUN_N<=PCD_NOZZLE_MAX)		
			tx_buffer[tx_len]=pcdStationInfo.GUN_N;
		else																			
			tx_buffer[tx_len]=PCD_NOZZLE_MAX;
		tx_len+=1;
		memcpy(&tx_buffer[tx_len], pcdStationInfo.NZN, tx_buffer[tx_len-1]);
		tx_len+=tx_buffer[tx_len-1];
		memcpy(&tx_buffer[tx_len], pcdBaseList.Version, 2);
		tx_len+=2;
		tx_buffer[tx_len]=pcdAddList.Version[1];
		tx_len+=1;
		tx_buffer[tx_len]=pcdDelList.Version[1];
		tx_len+=1;
		tx_buffer[tx_len]=pcdWhiteList.Version[1];
		tx_len+=1;
		tx_buffer[tx_len]=pcdOilInfo.Version;
		tx_len+=1;
		tx_buffer[tx_len]=pcdStationInfo.Version;
		tx_len+=1;
		
		pcd2PcSend(tx_buffer, tx_len);
	}
	//PC读取加油数据命令
	if(msg_len>0 && PCD2PC_CMD_ZDREAD==msg_buffer[0])
	{
		pos_ttc=(msg_buffer[1]<<24)|(msg_buffer[2]<<16)|(msg_buffer[3]<<8)|(msg_buffer[4]<<0);

		//读取指定账单，读取TTC合法且与读取到的账单TTC一致上传账单，否则回应无此账单
		if(pos_ttc>0)	fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), read_buffer, 95);
		if(pos_ttc>0 && msg_buffer[1]==read_buffer[0] && msg_buffer[2]==read_buffer[1] && msg_buffer[3]==read_buffer[2] && msg_buffer[4]==read_buffer[3])
		{
			tx_buffer[0]=PCD2PC_CMD_ZDUPLOAD;
			memcpy(&tx_buffer[1], read_buffer, 95);
			tx_len=96;
		}
		else
		{
			tx_buffer[0]=PCD2PC_CMD_ZDNO;
			tx_buffer[1]=(char)(pos_ttc>>24);	tx_buffer[2]=(char)(pos_ttc>>16);	tx_buffer[3]=(char)(pos_ttc>>8);		tx_buffer[4]=(char)(pos_ttc>>0);
			tx_len=5;
		}

		pcd2PcSend(tx_buffer, tx_len);
	}

	//增加ETC加油消费命令
	if((msg_len>0) && (msg_len<=ETC_DATA_MAX)  && PCD2PC_CMD_ETC_FUN==msg_buffer[0])
	{
		for(i=0;i<msg_len;i++)
		{
			if(i==0)
				printf("etc_rx:%x_",msg_buffer[i]);
			else if(i==msg_len-1)
				printf("%x\n",msg_buffer[i]);
			else 
				printf("%x_",msg_buffer[i]);
		}

		if(IptParamA.LogicNozzle==msg_buffer[3])
		{
			IptParamA.etc_rec_len=msg_len;
			memcpy(IptParamA.etc_rec_buff,msg_buffer,msg_len);
		}
		else if(IptParamB.LogicNozzle==msg_buffer[3])
		{
			IptParamB.etc_rec_len=msg_len;
			memcpy(IptParamB.etc_rec_buff,msg_buffer,msg_len);
		}
	}


	/*PCD主动的发送命令
	*	PCD主动发送数据，每次只允许一条数据发送并等待返回或超时；
	*	注意，查询黑白名单及查询灰交易等个别命令会有其它地方主动上送；
	*	PCD定时向PC发送轮训命令，当轮训返回后判断是否有其它命令需要执行；
	*	执行其他主动上送的命令，执行完毕后要上送普通轮询命令；
	*	使用pcdParam.pcCommand参数来判定当次已发送的命令，=0代表空闲；
	*	使用command参数来判断当前需要或正在处理的命令
	*/
	switch(command)
	{
	case PCD2PC_CMD_POLL: //普通查询命令，发送数据的处理	
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_POLL;
			tx_len=1;
			//tx_buffer[0] = 0x55; //广方测试
			//tx_buffer[1] = 0xaa;
			//tx_len = 2;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_POLL;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			//printf("xxxxxxxx\n");
			break;
		}

		//printf("zzzzzzzzzzzzzzzzz\n");
		
		if(PCD2PC_CMD_POLL==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_POLL==msg_buffer[0]) //接收数据的处理
		{
			//返回实时信息
			//	最多支持PCD_NOZZLE_MAX个支付终端，每个支付终端最多支持三条逻辑油枪；
			//	根据本支付终端是否在线，将在线的支付终端支持的逻辑油枪信息上传；
			//	逻辑油枪信息根据状态选择(1=卡插入；2=加油中)需要上传的数据；
			//	非"卡插入或加油中"状态之一则不上传数据；
			
		
			tx_buffer[0]=PCD2PC_CMD_REALINFO;
			tx_buffer[1]=0;
			tx_len=2;
			for(i=0; i<PCD_NOZZLE_MAX; i++)
			{
				if(0==i && PCD_FP_ONLINE==pcdParam.FP_1.IsOnline)		fp_info=&pcdParam.FP_1;
				else if(1==i && PCD_FP_ONLINE==pcdParam.FP_2.IsOnline)	fp_info=&pcdParam.FP_2;
				else if(1==i && PCD_FP_ONLINE==pcdParam.FP_3.IsOnline)	fp_info=&pcdParam.FP_3;
				else if(1==i && PCD_FP_ONLINE==pcdParam.FP_4.IsOnline)	fp_info=&pcdParam.FP_4;
				else if(1==i && PCD_FP_ONLINE==pcdParam.FP_5.IsOnline)	fp_info=&pcdParam.FP_5;
				else if(1==i && PCD_FP_ONLINE==pcdParam.FP_6.IsOnline)	fp_info=&pcdParam.FP_6;
				else	break;

				for(j=0; j<3 && j<fp_info->NozzleNumber; j++)
				{
					if(1==fp_info->Nozzle[j].State)
					{
						tx_buffer[tx_len+0]=fp_info->Nozzle[j].State;
						tx_buffer[tx_len+1]=fp_info->Nozzle[j].LogicNozzle;
						tx_buffer[tx_len+2]=16;
						memcpy(&tx_buffer[tx_len+3], fp_info->Nozzle[j].CardID, 10);
						memcpy(&tx_buffer[tx_len+13], fp_info->Nozzle[j].CardState, 2);
						memcpy(&tx_buffer[tx_len+15], fp_info->Nozzle[j].CardBalance, 4);
						tx_len+=19;	tx_buffer[1]++;
					}
					else if(2==fp_info->Nozzle[j].State)
					{
						tx_buffer[tx_len+0]=fp_info->Nozzle[j].State;
						tx_buffer[tx_len+1]=fp_info->Nozzle[j].LogicNozzle;
						tx_buffer[tx_len+2]=fp_info->Nozzle[j].PayUnit;
						memcpy(&tx_buffer[tx_len+3], fp_info->Nozzle[j].Money, 3);
						memcpy(&tx_buffer[tx_len+6], fp_info->Nozzle[j].Volume, 3);
						memcpy(&tx_buffer[tx_len+9], fp_info->Nozzle[j].Price, 2);
						tx_len+=11;	tx_buffer[1]++;
					}
				}
			}

			//szb_fj_20171120
			if(tx_buffer[1]==2 && Roll_Flag==0)
			{
				Roll_Flag=1;
			}
			else if(tx_buffer[1]==2 && Roll_Flag==1)/*调换*/
			{
				Roll_Flag=0;
				if(tx_buffer[2]==1)/*卡插入*/
				{
					memcpy(rolldata,tx_buffer+2,19);
					memcpy(tx_buffer+2,tx_buffer+2+19,tx_len-2-19);
					memcpy(tx_buffer+tx_len-19,rolldata,19);
				}
				else if(tx_buffer[2]==2)/*加油中*/
				{
					memcpy(rolldata,tx_buffer+2,11);
					memcpy(tx_buffer+2,tx_buffer+2+11,tx_len-2-11);
					memcpy(tx_buffer+tx_len-11,rolldata,11);
				}
				memset(rolldata,0,sizeof(rolldata));
			}
			//tx_buffer[1] = 0; //fj:20170919,为了测试修改位无实时信息。
			//tx_len = 2;
			//pcd2PcSend(tx_buffer,tx_len);

			//if(0==tx_buffer[1])	tx_len=1;		//实时信息条数为0则枪数也无效，不上送,fj:20171120
			pcd2PcSend(tx_buffer, tx_len);	//返回实时信息 
			memcpy(&pcdParam.PcTime, &msg_buffer[1], 7); //保存后台时间

			//保存状态为PC在线，清除超时次数计数器
			pcdParam.PcOnline=PCD_PC_ONLINE;	
			pcdParam.pcOverTimes=0;

			//查询是否有IPT向PC转发的查询数据
			//	当前无正在等待转发的数据时读取是否有未转发数据；
			//	读取到有待转发数据但类型非法时清除数据
			
			if(NULL==pcdParam.ipt2PcNode)	pcdParam.ipt2PcNode=(PcdIpt2PcNode*)lstGet(&pcdParam.ipt2PcList);
			if(NULL!=pcdParam.ipt2PcNode && \
				PCD_CMD_LIST!=pcdParam.ipt2PcNode->command && \
				PCD_CMD_GREYINFO!=pcdParam.ipt2PcNode->command && \
				PCD_CMD_BARCODE!=pcdParam.ipt2PcNode->command &&\
				PCD_CMD_ERRINFO_UPLOAD!=pcdParam.ipt2PcNode->command &&\
				PCD_CMD_DISCOUNT_ASK!=pcdParam.ipt2PcNode->command&&\
				PCD_CMD_APPLYFOR_DEBIT != pcdParam.ipt2PcNode->command)
			{
				 free(pcdParam.ipt2PcNode);	
				 pcdParam.ipt2PcNode=NULL;  //fj:
			}

			
			//判断是否有账单需要上传
			//	账单的存储申请TTC时保存未计算T_MAC的账单，然后再保存计算过T_MAC的账单
			//	有未传账单时，读到未传账单标识置位，持续判断本账单是否已计算T_MAC
			//	账单已计算T_MAC或未计算T_MAC但已超时2分钟则上传本账单；
			
			//if(ERROR == semTake(semIDPcdBill, 5*sysClkRateGet()))
			//{
				 //jljRunLog("判断是否有未上传账单时获取信号量[semIDPcdBill]失败!\n"); //fj:该函数在oilLog.c里
			//}
			
            int nMutexRet = -1;
			int nIndex = 0;
			do
			{
                nMutexRet = pthread_mutex_trylock(&mutexIDPcdBill);
				if(nMutexRet != 0)
				{
					//sleep(1);
                    usleep(200000);
				}
				else
				{
					break;
				}
				nIndex++;
			}while(nIndex < 5);

			if(nMutexRet != 0)
			{
                jljRunLog("判断是否有未上传账单时获取信号量[semIDPcdBill]失败!\n"); //fj:该函数在oilLog.c里
			}

			if(pcdParam.UnloadNumber>0 && pcdParam.TTC>=pcdParam.UnloadNumber && 0==pcdParam.NewZD)
			{
				 pcdParam.NewZD=1;	
				 pcdParam.UploadTimer=0;
			}
			if(pcdParam.UnloadNumber>0 && pcdParam.TTC>=pcdParam.UnloadNumber && 0!=pcdParam.NewZD)
			{
				pcdParam.UploadTTC=pcdParam.TTC+1-pcdParam.UnloadNumber;
				fileReadForPath(PCD_FILE_OILRECORD, O_RDONLY, S_IREAD | S_IWRITE, PCD_ZD_LEN*((pcdParam.UploadTTC-1)%PCD_RECORD_MAX), pcdParam.UploadZD, PCD_ZD_LEN);

				if(0!=memcmp(&pcdParam.UploadZD[PCD_OFFSET_T_MAC], "\x00\x00\x00\x00", 4) || (0==memcmp(&pcdParam.UploadZD[PCD_OFFSET_T_MAC], "\x00\x00\x00\x00", 4) && pcdParam.UploadTimer>=2*60*ONE_SECOND))
				{
					is_bill_upload=1;
				}
			}
			
			//semGive(semIDPcdBill); //fj:先不处理
			pthread_mutex_unlock(&mutexIDPcdBill);

			//当信息版本与PC后台不一致时累计不一致次数，当累计达到一定次数时下载该信息，防止有错误数据导致频繁下载
			if(pcdStationInfo.Version!=msg_buffer[14] && 0!=msg_buffer[14])	
				pcdParam.PcStaionInfoTimes++;
			else																										
				pcdParam.PcStaionInfoTimes=0;

			if(pcdOilInfo.Version!=msg_buffer[13] && 0!=msg_buffer[13])			
				pcdParam.PcPriceInfoTimes++;
			else																										
				pcdParam.PcPriceInfoTimes=0;

			if(0!=memcmp(pcdBaseList.Version, &msg_buffer[8], 2) && 0!=memcmp("\x00\x00", &msg_buffer[8], 2))	
					pcdParam.PcBaseListTimes++;
			else	
				pcdParam.PcBaseListTimes=0;

			if(pcdAddList.Version[1]!=msg_buffer[10] && 0!=msg_buffer[10])		
				pcdParam.PcAddListTimes++;
			else																										
				pcdParam.PcAddListTimes=0;
			
			if(pcdDelList.Version[1]!=msg_buffer[11] && 0!=msg_buffer[11])		
				pcdParam.PcDelListTimes++;
			else																										
				pcdParam.PcDelListTimes=0;
			
			if(pcdWhiteList.Version[1]!=msg_buffer[12] && 0!=msg_buffer[12])	
				pcdParam.PcWListTimes++;
			else																										
				pcdParam.PcWListTimes=0;

			//如果有加油机内部问题则转入加油机报错过程；
			//	如果有灰记录查询则转入灰记录查询过程；
			//	如果有黑/白名单查询转入黑/白名单查询过程；
			//	如果有账单需要上传则转入账单上传过程；
			//	如果有数据未下载完全或版本号不一致需要下载则转入申请下载过程；	
			
			if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_ERRINFO_UPLOAD==pcdParam.ipt2PcNode->command)
			{
				command=PCD2PC_CMD_ERRINFO;
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_BARCODE==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_BAR_CHECK; //需要向后台转发条码操作命令
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_GREYINFO==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_GREYINFO; //需要查询灰记录，转入灰记录查询过程
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_LIST==pcdParam.ipt2PcNode->command)
			{		
				command=PCD2PC_CMD_LISTINFO; //需要查询黑/白名单，转入黑/白名单查询过程
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_DISCOUNT_ASK==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_DISCOUT_ASK; //需要向后台申请折扣额，转入折扣额申请过程
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_APPLYFOR_DEBIT==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_APPLY_DEBIT; //需要向后台申请扣款
			}
//szb_fj_20171120:删除
#if 0
			else if(0!=is_bill_upload)
			{	
				//有账单需要上传，转入账单上传过程
				command=PCD2PC_CMD_ZDUPLOAD;	
				pcdParam.NewZD=0;
			}
#endif
			else if(	PCD_DOWN_BASELIST==pcdParam.PcDownloadContent || PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent ||\
				PCD_DOWN_DELLIST==pcdParam.PcDownloadContent || PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent ||\
				PCD_DOWN_OILINFO==pcdParam.PcDownloadContent || PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent)
			{	
				if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent || PCD_DOWN_OILINFO==pcdParam.PcDownloadContent)
				{
					//szb_fj_20171120:有正在执行的下载操作则直接转入下载过程
					command=PCD2PC_CMD_DOWNLOAD;
				}
				else if(pcdStationInfo.Version!=msg_buffer[14] && 0!=msg_buffer[14] && pcdParam.PcStaionInfoTimes>=5)
				{	
					//szb_fj_20171120:转入启动油站通用信息下载过程
					pcdParam.PcDownloadContent=PCD_DOWN_STATIONINFO;	
					pcdParam.PcDownloadLen=0;		
					pcdParam.PcOffsetLen=0;	
					command=PCD2PC_CMD_DOWNSTART;	
					pcdParam.PcStaionInfoTimes=0;
				}
				else if(pcdOilInfo.Version!=msg_buffer[13] && 0!=msg_buffer[13] && pcdParam.PcPriceInfoTimes>=5)
				{	
					//szb_fj_20171120:转入启动油品油价表下载过程
					pcdParam.PcDownloadContent=PCD_DOWN_OILINFO;
					pcdParam.PcDownloadLen=0;	
					pcdParam.PcOffsetLen=0;	
					command=PCD2PC_CMD_DOWNSTART;	
					pcdParam.PcPriceInfoTimes=0;
				}
				else
				{
					//szb_fj_20171120:有正在执行的下载操作则直接转入下载过程
					command=PCD2PC_CMD_DOWNLOAD;
				}
			}
			else if(pcdStationInfo.Version!=msg_buffer[14] && 0!=msg_buffer[14] && pcdParam.PcStaionInfoTimes>=5)
			{	//转入启动油站通用信息下载过程
				pcdParam.PcDownloadContent=PCD_DOWN_STATIONINFO;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcStaionInfoTimes=0;
			}
			else if(pcdOilInfo.Version!=msg_buffer[13] && 0!=msg_buffer[13] && pcdParam.PcPriceInfoTimes>=5)
			{	//转入启动油品油价表下载过程
				pcdParam.PcDownloadContent=PCD_DOWN_OILINFO;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcPriceInfoTimes=0;
			}
			else if(0!=memcmp(pcdBaseList.Version, &msg_buffer[8], 2) && 0!=memcmp("\x00\x00", &msg_buffer[8], 2) && pcdParam.PcBaseListTimes>=5)
			{	//转入启动基础黑名单下载过程
				pcdParam.PcDownloadContent=PCD_DOWN_BASELIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcBaseListTimes=0;
			}
			else if(pcdAddList.Version[1]!=msg_buffer[10] && 0!=msg_buffer[10] && pcdParam.PcAddListTimes>=5)
			{	//转入启动新增黑名单下载过程
				pcdParam.PcDownloadContent=PCD_DOWN_ADDLIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcAddListTimes=0;
			}
			else if(pcdDelList.Version[1]!=msg_buffer[11] && 0!=msg_buffer[11] && pcdParam.PcDelListTimes>=5)
			{	//转入启动新删黑名单下载过程
				pcdParam.PcDownloadContent=PCD_DOWN_DELLIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcDelListTimes=0;
			}
			else if(pcdWhiteList.Version[1]!=msg_buffer[12] && 0!=msg_buffer[12] && pcdParam.PcWListTimes>=5)
			{	//转入启动白名单下载过程	
				pcdParam.PcDownloadContent=PCD_DOWN_WHITELIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcWListTimes=0;
			}

			if((0!=is_bill_upload)&&(command==PCD2PC_CMD_DOWNSTART || \
				command==PCD2PC_CMD_DOWNLOAD || command==PCD2PC_CMD_POLL))
			{	
				if(command!=PCD2PC_CMD_DOWNSTART)
				{
					if(pcdParam.UnloadFlag==1)//szb_fj_20171120:执行过帐账单上传
					{
						pcdParam.UnloadFlag=0;//szb_fj_20171120:清零下次轮询进行账单上传
					}
					else
					{
						//szb_fj_20171120:有账单需要上传，转入账单上传过程
						command=PCD2PC_CMD_ZDUPLOAD;
						pcdParam.NewZD=0;
						pcdParam.UnloadFlag=1;
					}
				}
			}

			pcdParam.pcCommand=0;
			pcdParam.pcOvertimer=0;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_POLL==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//三次查询超时则认为油机离线
			pcdParam.pcOverTimes++;
			if(pcdParam.pcOverTimes>=3)
			{
				pcdParam.PcOnline=PCD_PC_OFFLINE;	
				memset(&pcdParam.PcTime, 0, 7);
			}

			pcdParam.pcCommand=0;	
			pcdParam.pcOvertimer=0;
			break;
		}
		break;

	case PCD2PC_CMD_ZDUPLOAD: //加油机向PC上传加油数据，PC返回后加油机未传条数递减
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_ZDUPLOAD;
			memcpy(&tx_buffer[1], pcdParam.UploadZD, 95);		
			tx_len=96;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_ZDUPLOAD;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理
		if(PCD2PC_CMD_ZDUPLOAD==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_ZDUPLOAD==msg_buffer[0])
		{
			//if(ERROR == semTake(semIDPcdBill, 5*sysClkRateGet()))
			//{
				 //jljRunLog("上传账单成功时获取信号量[semIDPcdBill]失败!\n"); //fj:该函数在oilLog.c里处理
			//}
            int nMutexRet = -1;
			int nIndex = 0;
			do
			{
                nMutexRet = pthread_mutex_trylock(&mutexIDPcdBill);
				if(nMutexRet != 0)
				{
					//sleep(1);
					usleep(200000);
				}
				else
				{
					break;
				}
				nIndex++;
			}while(nIndex < 5);

			if(nMutexRet != 0)
			{
                jljRunLog("判断是否有未上传账单时获取信号量[semIDPcdBill]失败!\n"); //fj:该函数在oilLog.c里
			}

			//未上传账单递减
			pcdParam.UnloadNumber--;
			read_buffer[0]=(char)(pcdParam.UnloadNumber>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber>>16);
			read_buffer[2]=(char)(pcdParam.UnloadNumber>>8);		read_buffer[3]=(char)(pcdParam.UnloadNumber>>0);
			pcdFmWrite(PCD_FM_UNLOAD, read_buffer, 4);

			//根据物理枪号，各物理油枪待传账单递减
			if(1==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber1>0)
			{
				pcdParam.UnloadNumber1--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber1>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber1>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber1>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber1>>0);
				pcdFmWrite(PCD_FM_UNLOAD1, read_buffer, 4);
			}
			else if(2==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber2>0)
			{
				pcdParam.UnloadNumber2--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber2>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber2>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber2>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber2>>0);
				pcdFmWrite(PCD_FM_UNLOAD2, read_buffer, 4);
			}
			else if(3==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber3>0)
			{	
				pcdParam.UnloadNumber3--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber3>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber3>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber3>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber3>>0);
				pcdFmWrite(PCD_FM_UNLOAD3, read_buffer, 4);
			}
			else if(4==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber4>0)
			{
				pcdParam.UnloadNumber4--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber4>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber4>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber4>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber4>>0);
				pcdFmWrite(PCD_FM_UNLOAD4, read_buffer, 4);
			}
			else if(5==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber5>0)
			{
				pcdParam.UnloadNumber5--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber5>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber5>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber5>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber5>>0);
				pcdFmWrite(PCD_FM_UNLOAD5, read_buffer, 4);
			}
			else if(6==pcdParam.UploadZD[PCD_OFFSET_PHYGUN] && pcdParam.UnloadNumber6>0)
			{
				pcdParam.UnloadNumber6--;
				read_buffer[0]=(char)(pcdParam.UnloadNumber6>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber6>>16);
				read_buffer[2]=(char)(pcdParam.UnloadNumber6>>8);	read_buffer[3]=(char)(pcdParam.UnloadNumber6>>0);
				pcdFmWrite(PCD_FM_UNLOAD6, read_buffer, 4);
			}

			//semGive(semIDPcdBill);  //fj:先不处理
			pthread_mutex_unlock(&mutexIDPcdBill);

			//上传账单缓存清零
			memset(pcdParam.UploadZD, 0, PCD_ZD_LEN);
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_ZDUPLOAD==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DOWNSTART: //0x33:加油机向PC机申请下载数据
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND) //加油机启动下载数据命令
		{
			tx_buffer[0]=PCD2PC_CMD_DOWNSTART;
			if(PCD_DOWN_BASELIST == pcdParam.PcDownloadContent)			tx_buffer[1] = 0;
			else if(PCD_DOWN_ADDLIST == pcdParam.PcDownloadContent)		tx_buffer[1] = 1;
			else if(PCD_DOWN_DELLIST == pcdParam.PcDownloadContent)		tx_buffer[1] = 2;
			else if(PCD_DOWN_WHITELIST == pcdParam.PcDownloadContent)	tx_buffer[1] = 3;
			else if(PCD_DOWN_OILINFO == pcdParam.PcDownloadContent)		tx_buffer[1] = 4;
			else	
				tx_buffer[1] = 5;
			//tx_buffer[1]=pcdParam.PcDownloadContent;
			tx_buffer[2]=pcdBaseList.Version[0];
			tx_buffer[3]=pcdBaseList.Version[1];
			tx_len=4;
			pcd2PcSend(tx_buffer, tx_len); 

			//printf("test 00000\n");

			pcdParam.pcCommand=PCD2PC_CMD_DOWNSTART;	
			pcdParam.pcSendTimer=0;	
			pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，未上传账单递减
		if(PCD2PC_CMD_DOWNSTART==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DOWNSTART==msg_buffer[0])
		{

			//printf("test 1111\n");
			//需要下载的数据文件长度
			pcdParam.PcDownloadLen=(msg_buffer[1]<<24)|(msg_buffer[2]<<16)|(msg_buffer[3]<<8)|(msg_buffer[4]<<0);
			pcdFmWrite(PCD_FM_DLEN, &msg_buffer[1], 4); //21*4

			//printf("test 2222\n");

			//下载内容
			pcdParam.PcDownloadContent=msg_buffer[5];
			if(0 == msg_buffer[5])			
				pcdParam.PcDownloadContent = PCD_DOWN_BASELIST;
			else if(1 == msg_buffer[5])	
				pcdParam.PcDownloadContent = PCD_DOWN_ADDLIST;
			else if(2 == msg_buffer[5])	
				pcdParam.PcDownloadContent = PCD_DOWN_DELLIST;
			else if(3 == msg_buffer[5])
				pcdParam.PcDownloadContent = PCD_DOWN_WHITELIST;
			else if(4 == msg_buffer[5])	
				pcdParam.PcDownloadContent = PCD_DOWN_OILINFO;
			else										
				pcdParam.PcDownloadContent = PCD_DOWN_STATIONINFO;
		
			//printf("test 3333\n");

			pcdFmWrite(PCD_FM_DCONTENT, &pcdParam.PcDownloadContent, 1);

			//已下载长度
			pcdParam.PcOffsetLen=0;
			pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);

			//清空原文件内容，之所以清空为4是因为清空为0会有操作问题
			if(PCD_DOWN_BASELIST==pcdParam.PcDownloadContent)	
			{
				fileTruncateForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}
			else	if(PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent)
			{
				fileTruncateForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}
			else	if(PCD_DOWN_DELLIST==pcdParam.PcDownloadContent)
			{
				fileTruncateForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}
			else	if(PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent)
			{
				fileTruncateForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}
			else	if(PCD_DOWN_OILINFO==pcdParam.PcDownloadContent)
			{
				fileTruncateForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}
			else	if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent)
			{
				fileTruncateForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
				fileWriteForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
			}

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;

			//printf("test 8888\n");
			break;
		}

		//超时处理
		if(PCD2PC_CMD_DOWNSTART==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.PcDownloadContent=0xff;	pcdParam.PcDownloadLen=0;	pcdParam.PcOffsetLen=0;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DOWNLOAD: 	//下载PC数据,0x34,申请下载具体数据内容
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_DOWNLOAD;
			if(PCD_DOWN_BASELIST == pcdParam.PcDownloadContent)			tx_buffer[1] = 0;
			else if(PCD_DOWN_ADDLIST == pcdParam.PcDownloadContent)		tx_buffer[1] = 1;
			else if(PCD_DOWN_DELLIST == pcdParam.PcDownloadContent)		tx_buffer[1] = 2;
			else if(PCD_DOWN_WHITELIST == pcdParam.PcDownloadContent)	tx_buffer[1] = 3;
			else if(PCD_DOWN_OILINFO == pcdParam.PcDownloadContent)		tx_buffer[1] = 4;
			else																									tx_buffer[1] = 5;
			//tx_buffer[1]=pcdParam.PcDownloadContent;
			tx_buffer[2]=(char)((pcdParam.PcOffsetLen/16)>>8);
			tx_buffer[3]=(char)((pcdParam.PcOffsetLen/16)>>0);
			tx_buffer[4]=PCD_DOWNLOAD_SEG;
			tx_len=5;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_DOWNLOAD;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，未上传账单递减
		if(PCD2PC_CMD_DOWNLOAD==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DOWNLOAD==msg_buffer[0])
		{
			//根据有效数据长度计算下载的数据长度并保存
			download_bytes=msg_len-5;
			download_offset=((msg_buffer[2]<<8)|(msg_buffer[3]<<0))*16;
			if(PCD_DOWN_BASELIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		//基础黑名单
			}
			else if(PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		  //新增黑名单
			}
			else if(PCD_DOWN_DELLIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		   //新删黑名单
			}
			else if(PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		     //白名单
			}
			else if(PCD_DOWN_OILINFO==pcdParam.PcDownloadContent)
			{
				//PrintH(download_bytes,&msg_buffer[5]);
				fileWriteForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);			//油品油价表
			}
			else if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		//油站通用信息
			}

			//计算已下载长度并保存
			pcdParam.PcOffsetLen+=download_bytes;
			tmp_buffer[0]=(char)(pcdParam.PcOffsetLen>>24);	tmp_buffer[1]=(char)(pcdParam.PcOffsetLen>>16);
			tmp_buffer[2]=(char)(pcdParam.PcOffsetLen>>8);		tmp_buffer[3]=(char)(pcdParam.PcOffsetLen>>0);
			pcdFmWrite(PCD_FM_DOFFSET, tmp_buffer, 4);

			//判断下载完毕，退出下载并更新缓存中相应信息，基础黑名单最多下载20000笔，超过部分不再下载
			
			if((download_bytes+download_offset)>=pcdParam.PcDownloadLen || 0==download_bytes)
			{
				if(PCD_DOWN_BASELIST==pcdParam.PcDownloadContent) //更新基础黑名单信息
				{
					fileReadForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdBaseList.Version, &read_buffer[0], 2);
					memcpy(pcdBaseList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdBaseList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdBaseList.Area, &read_buffer[10], 2);
					memcpy(pcdBaseList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent) //更新新增黑名单信息
				{
					fileReadForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdAddList.Version, &read_buffer[0], 2);
					memcpy(pcdAddList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdAddList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdAddList.Area, &read_buffer[10], 2);
					memcpy(pcdAddList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_DELLIST==pcdParam.PcDownloadContent) //更新新删黑名单信息
				{
					fileReadForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdDelList.Version, &read_buffer[0], 2);				
					memcpy(pcdDelList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdDelList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdDelList.Area, &read_buffer[10], 2);
					memcpy(pcdDelList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent) //更新白名单信息
				{
					fileReadForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdWhiteList.Version, &read_buffer[0], 2);
					memcpy(pcdWhiteList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdWhiteList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdWhiteList.Area, &read_buffer[10], 2);
					memcpy(pcdWhiteList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_OILINFO==pcdParam.PcDownloadContent) //打开油品油价信息文件并读取信息
				{
					fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 8);
					memcpy(&pcdOilInfo.Version, &read_buffer[0], 1);
					memcpy(pcdOilInfo.ValidTime, &read_buffer[1], 6);
					memcpy(&pcdOilInfo.FieldNumber, &read_buffer[7], 1);
					download_offset=8;
					
					for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++) //当前油品油价记录i
					{
						fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 8);
						memcpy(&pcdOilInfo.Field[i].NZN, &read_buffer[0], 1);
						memcpy(pcdOilInfo.Field[i].O_TYPE, &read_buffer[1], 2);
						memcpy(pcdOilInfo.Field[i].Density, &read_buffer[3], 4);
						memcpy(&pcdOilInfo.Field[i].Price_n, &read_buffer[7], 1);
						download_offset+=8;

						if(pcdOilInfo.Field[i].Price_n<=3)
						{
							fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 2*pcdOilInfo.Field[i].Price_n);
							memcpy(pcdOilInfo.Field[i].Price, read_buffer, 2*pcdOilInfo.Field[i].Price_n);
						}
						else
						{
							fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 2*3);
							memcpy(pcdOilInfo.Field[i].Price, read_buffer, 2*3);
						}
						download_offset+=(2*pcdOilInfo.Field[i].Price_n);
					}
					
					for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++) //新油品油价记录i
					{
						fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 8);
						memcpy(&pcdOilInfo.FieldNew[i].NZN, &read_buffer[0], 1);
						memcpy(pcdOilInfo.FieldNew[i].O_TYPE, &read_buffer[1], 2);
						memcpy(pcdOilInfo.FieldNew[i].Density, &read_buffer[3], 4);
						memcpy(&pcdOilInfo.FieldNew[i].Price_n, &read_buffer[7], 1);
						download_offset+=8;

						if(pcdOilInfo.FieldNew[i].Price_n<=3)
						{
							fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 2*pcdOilInfo.FieldNew[i].Price_n);
							memcpy(pcdOilInfo.FieldNew[i].Price, read_buffer, 2*pcdOilInfo.FieldNew[i].Price_n);
						}
						else
						{
							fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, read_buffer, 2*3);
							memcpy(pcdOilInfo.FieldNew[i].Price, read_buffer, 2*3);
						}
						download_offset+=(2*pcdOilInfo.FieldNew[i].Price_n);
					}

					
					memset(tmp_buffer, 0, 128);	 //新油品油价表接收完成生成一笔油价接收账单并向IPT计算TMAC
					tmp_buffer[PCD_OFFSET_T_TYPE]=0x08;	 //交易类型
					timeRead(&time);			 //时间，fj:在oilFRam.c里						
					tmp_buffer[PCD_OFFSET_TIME+0]=time.century;	tmp_buffer[PCD_OFFSET_TIME+1]=time.year;	
					tmp_buffer[PCD_OFFSET_TIME+2]=time.month;	tmp_buffer[PCD_OFFSET_TIME+3]=time.date;
					tmp_buffer[PCD_OFFSET_TIME+4]=time.hour;		tmp_buffer[PCD_OFFSET_TIME+5]=time.minute;
					tmp_buffer[PCD_OFFSET_TIME+6]=time.second;
					tmp_buffer[PCD_OFFSET_PHYGUN]=iptPhysicalNozzleRead(IPT_NOZZLE_1); //物理枪号,fj:oilIpt.c里定义
					pcdLocalTTCGet(iptPhysicalNozzleRead(IPT_NOZZLE_1), tmp_buffer, 128, &tmp_int1); //TTC,fj:同上
					tmp_buffer[PCD_OFFSET_TTC+0]=(char)(tmp_int1>>24);	tmp_buffer[PCD_OFFSET_TTC+1]=(char)(tmp_int1>>16);
					tmp_buffer[PCD_OFFSET_TTC+2]=(char)(tmp_int1>>8);		tmp_buffer[PCD_OFFSET_TTC+3]=(char)(tmp_int1>>0);
                    
					//fj:iptPhysicalNozzleRead在oilIpt.c里定义 
					pcd2IptSend(PCD_MSGTYPE_IPT, pcdMboardIDRead(), 0, iptPhysicalNozzleRead(IPT_NOZZLE_1), PCD_CMD_FOR_TMAC, tmp_buffer, 128);
				}
				
				if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent) //更新油站通用信息
				{
					fileReadForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 13);
					memcpy(&pcdStationInfo.Version, &read_buffer[0], 1);
					memcpy(&pcdStationInfo.Province, &read_buffer[1], 1);
					memcpy(&pcdStationInfo.City, &read_buffer[2], 1);
					memcpy(pcdStationInfo.Superior, &read_buffer[3], 4);
					memcpy(pcdStationInfo.S_ID, &read_buffer[7], 4);
					memcpy(&pcdStationInfo.POS_P, &read_buffer[11], 1);
					memcpy(&pcdStationInfo.GUN_N, &read_buffer[12], 1);
					if(pcdStationInfo.GUN_N<=6)
					{
						fileReadForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 13, read_buffer, pcdStationInfo.GUN_N);
						memcpy(pcdStationInfo.NZN, read_buffer, pcdStationInfo.GUN_N);
					}
					else
					{
						fileReadForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 13, read_buffer, 6);
						memcpy(pcdStationInfo.NZN, read_buffer, 6);
					}
				}


				//需要下载的数据文件长度
				pcdParam.PcDownloadLen=0;
				pcdFmWrite(PCD_FM_DLEN, "\x00\x00\x00\x00", 4);
				//下载内容
				pcdParam.PcDownloadContent=0xff;
				pcdFmWrite(PCD_FM_DCONTENT, "\xff", 1);
				//已下载长度
				pcdParam.PcOffsetLen=0;
				pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);
			}
	
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_DOWNLOAD==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;

			printf(" pcd timeout \n");
			break;
		}
		break;
	case PCD2PC_CMD_GREYINFO: //加油机向PC查询灰记录,0x35
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_GREYINFO;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_GREYINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，有正确的返回则向
		if(PCD2PC_CMD_GREYINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_GREYINFO==msg_buffer[0]) //查询成功向IPT返回查询结果
		{
			tx_buffer[0]=0;													    //查找源0=后台黑/白名单;1=本地黑/白名单
			memcpy(&tx_buffer[1], &msg_buffer[1], 42);	//灰记录
			tx_len=43;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_GREYINFO, tx_buffer, tx_len);

			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	
			pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_GREYINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	
			pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	
			pcdParam.pcOvertimer=0;	
			command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_LISTINFO: //加油机向PC查询黑/白名单信息,0x36
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_LISTINFO;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_LISTINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		
		if(PCD2PC_CMD_LISTINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_LISTINFO==msg_buffer[0]) //返回数据的处理，有正确的返回则向
		{
			//查询成功向IPT返回查询结果
			tx_buffer[0]=0;							 //结果0=成功
			tx_buffer[1]=0;							//查找源0=后台黑/白名单;1=本地黑/白名单
			tx_buffer[2]=msg_buffer[1];				//匹配标识
			memcpy(&tx_buffer[3], &msg_buffer[2], 10); //卡号
			tx_len=13;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_LIST, tx_buffer, tx_len);
			
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_LISTINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_ERRINFO: //加油机上送内部出错信息命令,0x3B	
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_ERRINFO;	tx_buffer[1]=0;
			memcpy(&tx_buffer[2], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=2+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_ERRINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，有正确的返回则向
		if(PCD2PC_CMD_ERRINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_ERRACK==msg_buffer[0] && PCD2PC_CMD_ERRINFO==msg_buffer[1]){

			//向IPT返回结果，结果0=成功
			tx_buffer[0]=0;													
			tx_len=1;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_ERRINFO_UPLOAD, tx_buffer, tx_len);
			
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_ERRINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{	
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DISCOUT_ASK: //0x70//向后台申请折扣额，联达系统加油机命令，20160401 by SYJ		
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_DISCOUT_ASK;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_DISCOUT_ASK;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，有正确的返回则向IPT返回结果
		if(PCD2PC_CMD_DISCOUT_ASK==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DISCOUT_ASK==msg_buffer[0])
		{
			memcpy(tx_buffer, &msg_buffer[1], msg_len - 1);
			tx_len = msg_len - 1;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_DISCOUNT_ASK, tx_buffer, tx_len);

			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_DISCOUT_ASK==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{	
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_BAR_CHECK:
	case PCD2PC_CMD_BAR_RESULT:
	case PCD2PC_CMD_BAR_ACK:
	case PCD2PC_CMD_BAR_ACKDONE: //向后台转发条码数据，联达系统加油机命令，返回的数据会直接被接收任务转发给IPT，20160401 by SYJ
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			memcpy(&tx_buffer[0], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_BAR_CHECK;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理，返回的数据已在接收到之后直接发送到IPT
		if(PCD2PC_CMD_BAR_CHECK==pcdParam.pcCommand && msg_len>0 && \
			(PCD2PC_CMD_BAR_CHECK==msg_buffer[0]||PCD2PC_CMD_BAR_RESULT==msg_buffer[0]||PCD2PC_CMD_BAR_ACK==msg_buffer[0]||PCD2PC_CMD_BAR_ACKDONE==msg_buffer[0]))
		{
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_BAR_CHECK==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;
	case PCD2PC_CMD_APPLY_DEBIT: //0x71 //向后台申请扣款数据，返回的数据转发给源消息队列，20160419 by SYJ		
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			memcpy(tx_buffer , pcdParam.ipt2PcNode->Buffer + 13, 38);
			tx_len = 38;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_APPLY_DEBIT;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//返回数据的处理
		if(PCD2PC_CMD_APPLY_DEBIT==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_APPLY_DEBIT==msg_buffer[0])
		{
			memcpy(tx_buffer, msg_buffer, msg_len);
			tx_len = msg_len;
			//fj: 先注释
			//msgTx = (MSG_Q_ID)((pcdParam.ipt2PcNode->Buffer[1]<<24)|(pcdParam.ipt2PcNode->Buffer[2]<<16)|(pcdParam.ipt2PcNode->Buffer[3]<<8)|(pcdParam.ipt2PcNode->Buffer[4]<<0));
			//if(NULL != msgTx)	msgQSend(msgTx, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);

			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL; 
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//超时处理
		if(PCD2PC_CMD_APPLY_DEBIT==pcdParam.pcCommand && pcdParam.pcOvertimer>=10*ONE_SECOND)
		{
			//清除发送的节点数据
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;
	default:
		command=PCD2PC_CMD_POLL;	pcdParam.pcCommand=0;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
		break;
	}


	//printf("ssssssss\n");

	return;
}


/********************************************************************
*Name				:tPcd2PcRx
*Description		:接受并解析PC管控后台返回的数据
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void tPcd2PcRx()
{
	prctl(PR_SET_NAME,(unsigned long)"tPcd2PcRx");
	unsigned char rx_buffer[PCD_PCDATA_MAX]={0};	//存储读取到的数据
	int rx_len=0;																	//存储的数据长度
	unsigned char read_buffer[32]={0};						//读取到的串口数据
	int read_len=0;																//读取到的串口数据长度
	unsigned char step=0;													//数据处理步骤
	unsigned short crc=0;													//CRC校验
	unsigned short data_len=0;										//有效数据长度
	int i=0;

	struct msg_struct msg_stPcdSend;
	msg_stPcdSend.msgType = 1;

	FOREVER
	{
		//printf("111111111\n");
		
		read_len = kjldRead(read_buffer, 32); //接收并解析PC卡机联动管控数据,fj,之后再改
		
        //PrintH(read_len,read_buffer);

		/*
		if(0 == pcdParam.SinopecChannel){
			read_len=comRead(pcdParam.comFdPc, read_buffer, 32);
		}
		else{
			read_len=netSRead(NETS_PROTOCOL_UDP, pcdParam.NetSPort, read_buffer, 32);
		}*/

		//printf("\n\npcrx===");
		//for(i=0; i<read_len; i++)	printf("%x:",read_buffer[i] );
		//printf("\n\n");
		
		for(i=0; i<read_len; i++)
		{	
			if(rx_len>=PCD_PCDATA_MAX) //判断接收长度溢出
			{
				step=0;	rx_len=0;
			}

			rx_buffer[rx_len]=read_buffer[i]; //接收并解析PC数据
			switch(step)
			{
			case 0: //接收数据开始字节		
				if(0xfa==rx_buffer[rx_len])	
					{	step=1;	rx_len=1;	}
				else										
					{	step=0;	rx_len=0;	}
				break;
			case 1: //接收数据目标地址、源地址、帧号/控制、有效数据长度（不能有0xFA）	
				if(0xfa==rx_buffer[rx_len])	
					{	step=0;	rx_len=0;	}
				else										
					{	rx_len++;}
		
				if(rx_len>=6) //下次开始处理有效数据
					{	step=2;}
				break;
			case 2: //数据位0xfa时初步认为该字节为转意字符执行下一步处理，否则保存该字节数据
				if(0xfa==rx_buffer[rx_len])	
					{	step=3;	}
				else										
					{	rx_len++;	}

				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
				data_len=((((int)rx_buffer[4])>>4)&0x0f)*1000+((((int)rx_buffer[4])>>0)&0x0f)*100+\
							((((int)rx_buffer[5])>>4)&0x0f)*10+((((int)rx_buffer[5])>>0)&0x0f)*1;

				//判断有效数据长度合法性
				if(data_len+8>=PCD_PCDATA_MAX)
				{
					step=0;	rx_len=0;
				}

				//判断是否接收完成
				if((rx_len>=8)&&(rx_len>=(data_len+8)))
				{
					crc=crc16Get(&rx_buffer[1], 5+data_len);
					if(crc==((rx_buffer[6+data_len]<<8)|(rx_buffer[7+data_len]<<0))) //szb_fj_20171120
					{
						//struct msg_struct msg_stPcdSend;
					    //msg_stPcdSend.msgType = 2;

						//判断如果是联达加油站系统油机且命令非联网命令时发送给支付模块
						if(MODEL_LIANDA == paramModelGet()&&\
							(PCD2PC_CMD_BAR_CHECK==rx_buffer[6] || PCD2PC_CMD_BAR_RESULT==rx_buffer[6] || PCD2PC_CMD_BAR_ACK==rx_buffer[6] || PCD2PC_CMD_BAR_ACKDONE==rx_buffer[6]))
						{
							//msgQSend((MSG_Q_ID)iptMsgIdRead(0), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL); //fj:
							//msgQSend((MSG_Q_ID)iptMsgIdRead(1), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);
							memcpy(msg_stPcdSend.msgBuffer,rx_buffer,rx_len);
							msgsnd(iptMsgIdRead(0),&msg_stPcdSend,rx_len,IPC_NOWAIT);
							msgsnd(iptMsgIdRead(1),&msg_stPcdSend,rx_len,IPC_NOWAIT);

							//printf("iiiiiiiiiiiiiiiii\n");
						}
						
						//msgQSend(pcdParam.msgIdFromPc, &rx_buffer[6], data_len, NO_WAIT, MSG_PRI_NORMAL); //fj:需要考虑怎么做
						
						memcpy(msg_stPcdSend.msgBuffer,&rx_buffer[6],data_len);
						int nRet = msgsnd(pcdParam.msgIdFromPc,&msg_stPcdSend,data_len,IPC_NOWAIT);
						//if(nRet != 0)
						//{
							  //printf("pc msg send ,nRet = %d, msgid = %d\n",nRet,pcdParam.msgIdFromPc);
						//}
					}

					step=0;	rx_len=0;	memset(rx_buffer, 0, PCD_PCDATA_MAX);
				}
				break;
			case 3://如果是0xfa则作为转义保存当前数据，如果不是0xfa则认为前一个0xfa为包头		
				if(0xfa==rx_buffer[rx_len])	
					{	step=2;	rx_len++;	}
				else									
				{
					rx_buffer[0]=0xfa;	rx_buffer[1]=read_buffer[i];	step=1;	rx_len=2;
				}

				//判断接收结束，长度不能过大防止溢出，长度合法则判断CRC
				data_len=((((int)rx_buffer[4])>>4)&0x0f)*1000+((((int)rx_buffer[4])>>0)&0x0f)*100+\
							((((int)rx_buffer[5])>>4)&0x0f)*10+((((int)rx_buffer[5])>>0)&0x0f)*1;

				//判断有效数据长度合法性
				if(data_len+8>=PCD_PCDATA_MAX)
				{
					step=0;	rx_len=0;
				}

				//判断是否接收完成
				if((rx_len>=8)&&(rx_len>=(data_len+8)))
				{
					crc=crc16Get(&rx_buffer[1], 5+data_len);
					if(crc==((rx_buffer[6+data_len]<<8)|(rx_buffer[7+data_len]<<0))) //szb_fj_20171120,update
					{
						struct msg_struct msg_stPcdSend;
						msg_stPcdSend.msgType = 2;
						//判断如果是联达加油站系统油机且命令非联网命令时发送给支付模块
						if(MODEL_LIANDA == paramModelGet()&&\
							(PCD2PC_CMD_BAR_CHECK==rx_buffer[6] || PCD2PC_CMD_BAR_RESULT==rx_buffer[6] || PCD2PC_CMD_BAR_ACK==rx_buffer[6] || PCD2PC_CMD_BAR_ACKDONE==rx_buffer[6]))
						{
							 //msgQSend((MSG_Q_ID)iptMsgIdRead(0), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:同上
							 //msgQSend((MSG_Q_ID)iptMsgIdRead(1), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:同上
							memcpy(msg_stPcdSend.msgBuffer,rx_buffer,rx_len);
							msgsnd(iptMsgIdRead(0),&msg_stPcdSend,rx_len,IPC_NOWAIT);
                            msgsnd(iptMsgIdRead(1),&msg_stPcdSend,rx_len,IPC_NOWAIT);
							
							//printf("iiiiiiiiiiiiiiiii\n");
						}
						
						//msgQSend(pcdParam.msgIdFromPc, &rx_buffer[6], data_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:同上
						memcpy(&msg_stPcdSend.msgBuffer,&rx_buffer[6],data_len);
						msgsnd(pcdParam.msgIdFromPc,&msg_stPcdSend,data_len,IPC_NOWAIT);
						//printf("iiiiiii\n");
					}

					step=0;	rx_len=0;	memset(rx_buffer, 0, PCD_PCDATA_MAX);
				}
				break;
			default:
				break;
			}
		}

		//taskDelay(1); //fj:
		usleep(1000);
	}

	return;
}


/********************************************************************
*Name				:tPcdProcess
*Description		:PCD数据处理任务函数入口
*Input				:None
*Output			:None
*Return				:None
*History			:2015-01-28,modified by syj
*/
void tPcdProcess(void)
{
	prctl(PR_SET_NAME,(unsigned long)"tPcdProcess");
	FOREVER
	{
		//PCD与IPT间通讯处理
        pcd2IptProcess();

		//PCD与PC间通讯处理,中石化PC后台的通讯
		pcd2PcProcess();

		//taskDelay(1);
		usleep(1000);

        //printf("sssssssssss\n");
	}

	return;
}


/********************************************************************
*Name				:pcdWdgIsr
*Description		:PCD模块看门狗定时器函数入口
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void pcdWdgIsr()
{

//	pcdParam.pcSendTimer++;		//PCD向PC发送数据间隔定时器
//	pcdParam.pcOvertimer++;		//PCD向PC发送数据后等待返回超时定时器
//	pcdParam.UploadTimer++;		//PCD读到有新的账单到产生T_MAC的超时定时器
	pcdParam.pcSendTimer+=10;		//PCD向PC发送数据间隔定时器
	pcdParam.pcOvertimer+=10;		//PCD向PC发送数据后等待返回超时定时器
	pcdParam.UploadTimer+=10;		//PCD读到有新的账单到产生T_MAC的超时定时器
	//wdStart(pcdParam.wdgId, 1, (FUNCPTR)pcdWdgIsr, 0); //fj:vxworks的函数，最后再解决

	return;
}


/********************************************************************
*Name				:pcdMsgIdGet
*Description		:获取PCD接收消息队列ID
*Input				:None
*Output			:None
*Return				:None
*History			:2013-08-05,modified by syj
*/

int pcdMsgIdRead()   //fj:先注释
{
	return pcdParam.msgIdFromIpt;
}


/********************************************************************
*Name				:pcdMboardIDwrite
*Description		:设置本地PCD主板号
*Input				:mboard_id	主板号
*Output			:None
*Return				:0=成功；其它=失败
*History			:2013-07-01,modified by syj
*/
int pcdMboardIDWrite(unsigned char mboard_id)
{
	char wrbuffer[4] = {0};
	int istate = 0;

	*wrbuffer = mboard_id;
	istate = paramSetupWrite(PRM_MBOARD_ID, wrbuffer, 1);
	if(0 != istate)
	{
		return ERROR;
	}

	pcdParam.mboardID=mboard_id;

	return 0;
}


/********************************************************************
*Name				:pcdMboardIDRead
*Description		:获取本地PCD主板号
*Input				:None
*Output			:None
*Return				:本地PCD主板号
*History			:2013-07-01,modified by syj
*/
unsigned char pcdMboardIDRead(void)
{
	return pcdParam.mboardID;
}


/********************************************************************
*Name				:pcdApplyForTTC
*Description		:申请TTC
*Input				:phynozzle	物理枪号
*						:inbuffer		账单数据(无TTC)
*						:nbytes			账单数据长度(暂固定128)
*Output			:ttc				申请到的TTC
*Return				:0=成功；1=物理枪号非法；其它=错误
*History			:2016-04-13,modified by syj
*/
int pcdApplyForTTC(int phynozzle, const char *inbuffer, int nbytes, unsigned int *ttc)
{
	char ibuffer[256] = {0};
	int ilenght = 0;

	memcpy(ibuffer, inbuffer, nbytes);	ilenght = nbytes;
	
	return pcdLocalTTCGet(phynozzle, ibuffer, ilenght, ttc);
}


/********************************************************************
*Name				:pcdApplyForBillSave
*Description		:保存账单
*Input				:phynozzle	物理枪号
*						:inbuffer		账单数据
*						:nbytes			账单数据长度(暂固定128)
*Output			:None
*Return				:成功返回0；失败返回其它；
*History			:2016-04-13,modified by syj
*/
int pcdApplyForBillSave(int phynozzle, const char *inbuffer, int nbytes)
{
	char ibuffer[256] = {0};
	int istate = 0, ilenght = 0;

	memcpy(ibuffer, inbuffer, nbytes);	ilenght = nbytes;
	istate = pcdLocalBillSave(phynozzle, ibuffer, ilenght);

	return istate;
}


/********************************************************************
*Name				:pcdApplyForBillSave
*Description		:向后台发送数据并等待返回
*Input				:command	命令字
*						:inbuffer		输入数据(帧号(1Hex) + 命令字(2Bin) + 数据长度(2Hex) + 数据(nbytes))
*						:timeout		超时时间，单位为秒
*Output			:outbuffer		输出数据(帧号(1Hex) + 命令字(2Bin) + 数据长度(2Hex) + 数据(nbytes))
*Return				:成功返回0；超时返回1；失败返回其它值；
*History			:2016-04-13,modified by syj
*/

//fj:先都注释了,ETC授权加油,先不测试
int pcdPcSend(int command, char *inbuffer, int nbytes, char *outbuffer, int maxbytes, int timeout)
{
	/*char tx_buffer[PCD_MSGMAX + 1] = {0};
	int tx_len = 0;
	char rx_buffer[PCD_MSGMAX + 1] = {0};
	int rx_len = 0;
	MSG_Q_ID msgRx = NULL, msgTx = NULL;
	int istate = 0;
	int ilength = 0;

	//判断是否处于联网状态
	if(1 != pcdParam.PcOnline)
	{
		printf("[Function:%s][Line:%d]PCD离线!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//判断发送的消息队列
	msgTx = pcdParam.msgIdRx;
	if(NULL == msgTx)
	{
		printf("[Function:%s][Line:%d]PCD接收消息队列非法!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//创建接收的消息队列
	msgRx = msgQCreate(1, PCD_MSGMAX + 1, MSG_Q_FIFO);
	if(NULL == msgRx)
	{
		printf("[Function:%s][Line:%d]创建存储返回数据的消息队列失败!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//发送(帧号(1Hex) + 源消息队列地址(4bytes) + 目的消息队列地址(4bytes) + 命令字(2Bin) +  数据长度(2Hex) + 数据(nbytes))
	*(tx_buffer + 0) = 0xff;
	*(tx_buffer + 1) = (char)((int)msgRx>>24);		*(tx_buffer + 2) = (char)((int)msgRx>>16);
	*(tx_buffer + 3) = (char)((int)msgRx>>8);		*(tx_buffer + 4) = (char)((int)msgRx>>0);
	*(tx_buffer + 5) = (char)((int)msgTx>>24);		*(tx_buffer + 6) = (char)((int)msgTx>>16);
	*(tx_buffer + 7) = (char)((int)msgTx>>8);		*(tx_buffer + 8) = (char)((int)msgTx>>0);
	*(tx_buffer + 9) = (char)(command>>8);
	*(tx_buffer + 10) = (char)(command>>0);
	*(tx_buffer + 11) = (char)(nbytes>>8);
	*(tx_buffer + 12) = (char)(nbytes>>0);
	memcpy(tx_buffer + 13, inbuffer, nbytes);
	tx_len = 13 + nbytes;
	//istate = msgQSend(msgTx, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);
	istate = pcdIpt2PcSend(0, 0, 0, 0, command, tx_buffer, tx_len);
	if(OK != istate)
	{
		printf("[Function:%s][Line:%d]发送数据时失败!\n", __FUNCTION__, __LINE__);
		msgQDelete(msgRx);
		return ERROR;
	}

	//接收(帧号(1Hex) + 源消息队列地址(4bytes) + 目的消息队列地址(4bytes) + 命令字(2Bin) +  数据长度(2Hex) + 数据(nbytes))
	rx_len = msgQReceive(msgRx, rx_buffer, PCD_MSGMAX, timeout*sysClkRateGet());
	if(ERROR == rx_len)
	{
		printf("[Function:%s][Line:%d]接收数据时失败!\n", __FUNCTION__, __LINE__);
		msgQDelete(msgRx);
		return ERROR;
	}

	//输出数据
	memcpy(outbuffer, rx_buffer, rx_len);

	msgQDelete(msgRx);*/
	return 0;
}


/********************************************************************
*Name				:pcdApplyForAuthETCDebit
*Description		:向后台申请ETC卡扣款
*Input				:inbuffer		输入数据
*						:					命令字(1byte) + 逻辑枪号(1byte) + 天线ID(1byte) + 
*						:					标签MAC号(4bytes) + OBU合同编号(8bytes) + 车牌号(12bytes) +
*						:					扣款额(4bytes) + 时间(7bytes)
*Output			:outbuffer		输出数据
*						:					命令字(1byte) + 逻辑枪号(1Byte) + 结果(1byte)
*Return				:成功返回0；超时返回1；失败返回其它值；
*History			:2016-04-13,modified by syj
*/
int pcdApplyForAuthETCDebit(char *inbuffer, int nbytes, char *outbuffer, int maxbytes)
{
	return pcdPcSend(PCD_CMD_APPLYFOR_DEBIT, inbuffer, nbytes, outbuffer, maxbytes, 10);
	//return  0;    //fj:之后再解决
}


/********************************************************************
*Name				:pcdOilRecordRead
*Description		:获取油品油价纪录表数据
*Input				:无
*Output			:oilinfo	油品油价表数据
*Return			:成功返回0；失败返回其它值；
*History			:2016-05-04,modified by syj
*/
int pcdOilRecordRead(PcdOilRecordType *oilinfo)
{
	if(NULL == oilinfo)
	{
		printf("%s:%d:输出缓存地址非法!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//taskLock();  //fj:先注释
	memcpy(oilinfo, &pcdOilInfo, sizeof(PcdOilRecordType));
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcdStationInfoRead
*Description		:获取油站通用信息
*Input				:无
*Output			:oilinfo	油品油价表数据
*Return				:成功返回0；失败返回其它值；
*History			:2016-05-04,modified by syj
*/
int pcdStationInfoRead(PcdStationInfoType *stationinfo)
{
	if(NULL == stationinfo)
	{
		printf("%s:%d:输出缓存地址非法!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//taskLock(); //fj:先注释
	memcpy(stationinfo, &pcdStationInfo, sizeof(PcdStationInfoType));
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcdParamInit
*Description		:PCD初始化
*Input				:None
*Output			:None
*Return				:0=成功；其它=失败
*History			:2014-03-25,modified by syj
*/
int pcdParamInit(void)
{
	unsigned char rdbuffer[16]={0};
	int i=0, istate=0, istate2=0, ireturn=0, rdbytes=0;

	//铁电参数初始化,fj:先注视，在oilFRam.c里
	for(i=0; i<(FM_SIZE_PCD_SINO/8); i++)	
		framWrite(FM_ADDR_PCD_SINO, i*8, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
	for(i=0; i<(FM_SIZE_PCD_SINO/8); i++)
	{
		 ireturn=framRead(FM_ADDR_PCD_SINO, i*8, rdbuffer, 8); //fj:在oilFRam.c里
		if((0!=ireturn)||(0!=memcmp(rdbuffer, "\x00\x00\x00\x00\x00\x00\x00\x00", 8)))	istate=1;
	}

	//下载内容写为无下载
	pcdParam.PcDownloadContent=0xff;	
	pcdFmWrite(PCD_FM_DCONTENT, "\xff", 1);  

	//主板号:1
	if(0!=paramSetupWrite(PRM_MBOARD_ID, "\x01", 1))		istate=1;

	//已下载长度
	pcdParam.PcOffsetLen=0;
	pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);

	//清除缓存数据结构
	memset(&pcdBaseList, 0, sizeof(PcdListInfoType));				//基础黑名单
	memset(&pcdAddList, 0, sizeof(PcdListInfoType));				//新增黑名单
	memset(&pcdDelList, 0, sizeof(PcdListInfoType));				//新删黑名单
	memset(&pcdWhiteList, 0, sizeof(PcdListInfoType));			//白名单
	memset(&pcdOilInfo, 0, sizeof(PcdOilRecordType));		  	//油品油价表
	memset(&pcdStationInfo, 0, sizeof(PcdStationInfoType));	//通用信息
	memset(&pcdParam, 0, sizeof(PcdParamStructType));		    //PCD操作相关数据结构
 
	//文件清空，因截取文件为0时有问题，所以只好截取为4，并初始化为0
	istate2=fileTruncateForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_1, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_1, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_2, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_2, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_3, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_3, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_4, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_4, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_5, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_5, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	istate2=fileTruncateForPath(PCD_FILE_ZDINDEX_6, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 4);
	if(0==istate2)	
		fileWriteForPath(PCD_FILE_ZDINDEX_6, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, "\x00\x00\x00\x00", 4);
	else					
		istate=ERROR;

	#if 0
	if(0!=fileTruncate(pcdParam.FdBaseList, 4))			istate=1;
	fileWrite(pcdParam.FdBaseList, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdAddList, 4))			istate=1;
	fileWrite(pcdParam.FdAddList, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdDelList, 4))				istate=1;
	fileWrite(pcdParam.FdDelList, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdWhiteList, 4))			istate=1;
	fileWrite(pcdParam.FdWhiteList, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdOilInfo, 4))				istate=1;
	fileWrite(pcdParam.FdOilInfo, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdStationInfo, 4))		istate=1;
	fileWrite(pcdParam.FdStationInfo, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdRecord, 4))				istate=1;
	fileWrite(pcdParam.FdRecord, 0, "\x00\x00\x00\x00", 4);
	if(0!=fileTruncate(pcdParam.FdZDUnnormal, 4))		istate=1;
	fileWrite(pcdParam.FdZDUnnormal, 0, "\x00\x00\x00\x00", 4);
	#endif

	return istate;
}

/********************************************************************
*Name				:pcdInit
*Description		:PCD模块功能初始化
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
bool pcdInit(void)
{
	unsigned char read_buffer[256]={0};
	int read_len=0, istate=0, i=0;
	off_t offset=0;

//pcdFmWrite(PCD_FM_UNLOAD, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD1, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD2, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD3, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD4, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD5, "\x00\x00\x00\x00", 4);
//pcdFmWrite(PCD_FM_UNLOAD6, "\x00\x00\x00\x00", 4);

	 int nInitRet = pthread_mutex_init(&mutexIDPcdBill,NULL);
	 if(nInitRet != 0)
	 {
     	jljUserLog("PCD初始化创建账单操作信号量[semIDPcdBill]失败!\n");
	 }

/**********************************************************
*	PCD配置相关信息
***********************************************************/

	//获取主板号，获取失败或主板号非法则默认为1号主板
	istate=paramSetupRead(PRM_MBOARD_ID, read_buffer, 1);
	if(0==istate)
	{
		pcdParam.mboardID=read_buffer[0];
	}
	if(pcdParam.mboardID<1 || pcdParam.mboardID>PCD_MBOARD_MAX)
	{
		pcdParam.mboardID=1;
		printf("Error! Read the param PcdParam.MboardId failed!\n");
	}

	//初始化主板与开机联动后台的通讯接口功能
	kjldInit();  //fj:卡机联动在oilKJLD.c里


/**********************************************************
*	后台下载相关信息
***********************************************************/
	//正在向后台PC下载的信息内容
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DCONTENT, read_buffer, 1); //fj:在oilFRam.c里
	if(0==istate)
	{
		pcdParam.PcDownloadContent=read_buffer[0];
	}
	else
	{
		printf("Error!	Read the param 'PCD_FM_DCONTENT' failed!\n");
	}

	//正在向后台PC下载的信息内容总长度
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DLEN, read_buffer, 4);   //fj:同上
	if(0==istate)
	{
		pcdParam.PcDownloadLen=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	}
	else	
	{
		printf("Error!	Read the param 'PCD_FM_DLEN' failed!\n");
	}

	//正在向后台PC下载的信息内容已下载长度
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DOFFSET, read_buffer, 4);  //fj:同上
	if(0==istate)
	{
		pcdParam.PcOffsetLen=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	}
	else	
	{	
		printf("Error!	Read the param 'PCD_FM_DOFFSET' failed!\n");
	}


	//打开基础黑名单文件并读取信息
	istate=fileReadForPath(PCD_FILE_BASELIST,  O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 16);
	if(ERROR!=istate)
	{
		memcpy(pcdBaseList.Version, &read_buffer[0], 2);
		memcpy(pcdBaseList.TimeStart, &read_buffer[2], 4);
		memcpy(pcdBaseList.TimeFinish, &read_buffer[6], 4);
		memcpy(pcdBaseList.Area, &read_buffer[10], 2);
		memcpy(pcdBaseList.Number, &read_buffer[12], 4);

		int nRead = fileReadForPath(PCD_FILE_BASELIST,  O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 36);
		if(ERROR != nRead)
		{
			PrintH(36,read_buffer);
		}
	}
	else
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_BASELIST);
		return false;
	}

	//打开新增黑名单文件并读取信息
	istate=fileReadForPath(PCD_FILE_ADDLIST, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 16);
	if(ERROR!=istate)
	{
		memcpy(pcdAddList.Version, &read_buffer[0], 2);
		memcpy(pcdAddList.TimeStart, &read_buffer[2], 4);
		memcpy(pcdAddList.TimeFinish, &read_buffer[6], 4);
		memcpy(pcdAddList.Area, &read_buffer[10], 2);
		memcpy(pcdAddList.Number, &read_buffer[12], 4);
	}
	else
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_ADDLIST);
		return false;
	}

	//打开新删黑名单文件并读取信息
	istate=fileReadForPath(PCD_FILE_DELLIST, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 16);
	if(ERROR!=istate)
	{
		memcpy(pcdDelList.Version, &read_buffer[0], 2);				
		memcpy(pcdDelList.TimeStart, &read_buffer[2], 4);
		memcpy(pcdDelList.TimeFinish, &read_buffer[6], 4);
		memcpy(pcdDelList.Area, &read_buffer[10], 2);
		memcpy(pcdDelList.Number, &read_buffer[12], 4);
	}
	else 
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_DELLIST);
		return false;
	}

	//打开白名单文件并读取信息
	istate=fileReadForPath(PCD_FILE_WLIST, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 16);
	if(ERROR!=istate)
	{
		memcpy(pcdWhiteList.Version, &read_buffer[0], 2);
		memcpy(pcdWhiteList.TimeStart, &read_buffer[2], 4);
		memcpy(pcdWhiteList.TimeFinish, &read_buffer[6], 4);
		memcpy(pcdWhiteList.Area, &read_buffer[10], 2);
		memcpy(pcdWhiteList.Number, &read_buffer[12], 4);
	}
	else
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_WLIST);
		return false;
	}

	//打开油品油价信息文件并读取信息
	istate=fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 8);
	if(ERROR!=istate)
	{
		memcpy(&pcdOilInfo.Version, &read_buffer[0], 1);
		memcpy(pcdOilInfo.ValidTime, &read_buffer[1], 6);
		memcpy(&pcdOilInfo.FieldNumber, &read_buffer[7], 1);
		offset=8;
		//当前油品油价记录i
		for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++)
		{
			fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 8);
			memcpy(&pcdOilInfo.Field[i].NZN, &read_buffer[0], 1);
			memcpy(pcdOilInfo.Field[i].O_TYPE, &read_buffer[1], 2);
			memcpy(pcdOilInfo.Field[i].Density, &read_buffer[3], 4);
			memcpy(&pcdOilInfo.Field[i].Price_n, &read_buffer[7], 1);
			offset+=8;

			if(pcdOilInfo.Field[i].Price_n<=3)
			{
				fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 2*pcdOilInfo.Field[i].Price_n);
				memcpy(pcdOilInfo.Field[i].Price, read_buffer, 2*pcdOilInfo.Field[i].Price_n);
			}
			else
			{
				fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 2*3);
				memcpy(pcdOilInfo.Field[i].Price, read_buffer, 2*3);
			}
			offset+=(2*pcdOilInfo.Field[i].Price_n);
		}
		//新油品油价记录i
		for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++)
		{
			fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 8);
			memcpy(&pcdOilInfo.FieldNew[i].NZN, &read_buffer[0], 1);
			memcpy(pcdOilInfo.FieldNew[i].O_TYPE, &read_buffer[1], 2);
			memcpy(pcdOilInfo.FieldNew[i].Density, &read_buffer[3], 4);
			memcpy(&pcdOilInfo.FieldNew[i].Price_n, &read_buffer[7], 1);
			offset+=8;

			if(pcdOilInfo.FieldNew[i].Price_n<=3)
			{
				fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 2*pcdOilInfo.FieldNew[i].Price_n);
				memcpy(pcdOilInfo.FieldNew[i].Price, read_buffer, 2*pcdOilInfo.FieldNew[i].Price_n);
			}
			else
			{
				fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, offset, read_buffer, 2*3);
				memcpy(pcdOilInfo.FieldNew[i].Price, read_buffer, 2*3);
			}
			offset+=(2*pcdOilInfo.FieldNew[i].Price_n);
		}
	}
	else
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_PRICEINFO);
		return false;
	}

	//打开油站通用信息文件并读取信息
	istate=fileReadForPath(PCD_FILE_STATIONINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 13);
	if(ERROR!=istate)
	{
		memcpy(&pcdStationInfo.Version, &read_buffer[0], 1);
		memcpy(&pcdStationInfo.Province, &read_buffer[1], 1);
		memcpy(&pcdStationInfo.City, &read_buffer[2], 1);
		memcpy(pcdStationInfo.Superior, &read_buffer[3], 4);
		memcpy(pcdStationInfo.S_ID, &read_buffer[7], 4);
		memcpy(&pcdStationInfo.POS_P, &read_buffer[11], 1);
		memcpy(&pcdStationInfo.GUN_N, &read_buffer[12], 1);

		printf("stationinfo :\n");
		PrintH(13,read_buffer);

		if(pcdStationInfo.GUN_N<=6)
		{
			fileReadForPath(PCD_FILE_STATIONINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 13, read_buffer, pcdStationInfo.GUN_N);
			memcpy(pcdStationInfo.NZN, read_buffer, pcdStationInfo.GUN_N);
		}
		else
		{
			fileReadForPath(PCD_FILE_STATIONINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 13, read_buffer, 6);
			memcpy(pcdStationInfo.NZN, read_buffer, 6);
		}
	}
	else
	{
		printf("Error!	Open %s failed!\n", PCD_FILE_STATIONINFO);
		return false;
	}


/**********************************************************
*	账单存取相关信息
***********************************************************/
	//物理1号枪账单TTC
	pcdFmRead(PCD_FM_TTC1, read_buffer, 4);      //fj:在oilFRam.c里
	pcdParam.TTC1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	printf("pcdParam.TTC1 = %d\n",pcdParam.TTC1);
	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_TTC1  ",read_buffer,4);

	//物理1号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM1, read_buffer, 4);    //fj:在oilFRam.c里
	pcdParam.ZDNumber1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	
	printf("pcdParam.ZDNumber1 = %d\n",pcdParam.ZDNumber1);
	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_ZDNUM1  ",read_buffer,4);

   
	//pcdFmWrite(PCD_FM_ZDNUM1,"\x00\x00\x01\x02",4);
	//unsigned char buff[8];
	//pcdFmRead(PCD_FM_ZDNUM1,buff,8);
	//PrintH(4,buff);


	//物理1号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD1, read_buffer, 4);    //fj:在oilFRam.c里
	pcdParam.UnloadNumber1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//物理2号枪账单TTC
	pcdFmRead(PCD_FM_TTC2, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.TTC2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_TTC2  ",read_buffer,4);

	//物理2号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM2, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.ZDNumber2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理2号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD2, read_buffer, 4);      //fj:在oilFRam.c里
	pcdParam.UnloadNumber2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//物理3号枪账单TTC
	pcdFmRead(PCD_FM_TTC3, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.TTC3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理3号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM3, read_buffer, 4);     //fj:在oilFRam.c里
	pcdParam.ZDNumber3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理3号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD3, read_buffer, 4);  //fj:在oilFRam.c里
	pcdParam.UnloadNumber3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//物理4号枪账单TTC
	pcdFmRead(PCD_FM_TTC4, read_buffer, 4);     //fj:在oilFRam.c里
	pcdParam.TTC4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理4号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM4, read_buffer, 4);   //fj:在oilFRam.c里
	pcdParam.ZDNumber4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理4号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD4, read_buffer, 4);     //fj:在oilFRam.c里
	pcdParam.UnloadNumber4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//物理5号枪账单TTC
	pcdFmRead(PCD_FM_TTC5, read_buffer, 4);      //fj:在oilFRam.c里
	pcdParam.TTC5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理5号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM5, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.ZDNumber5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理5号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD5, read_buffer, 4);     //fj:在oilFRam.c里
	pcdParam.UnloadNumber5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//物理6号枪账单TTC
	pcdFmRead(PCD_FM_TTC6, read_buffer, 4);         //fj:在oilFRam.c里
	pcdParam.TTC6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理6号枪账单数目
	pcdFmRead(PCD_FM_ZDNUM6, read_buffer, 4);        //fj:在oilFRam.c里
	pcdParam.ZDNumber6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//物理6号枪未传账单数目
	pcdFmRead(PCD_FM_UNLOAD6, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.UnloadNumber6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//逃卡账单数目
	pcdFmRead(PCD_FM_YCZDNUM, read_buffer, 4);       //fj:在oilFRam.c里
	pcdParam.AbnormalNumber=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//pcdFmWrite(PCD_FM_UNLOAD,"\x00\x00\x00\x00",4);
	//printf("start : %02x,%02x,%02x,%02x,",read_buffer[0],read_buffer[1],read_buffer[2],read_buffer[3]);

	//待传账单数量
	istate=pcdFmRead(PCD_FM_UNLOAD, read_buffer, 4);   //fj:在oilFRam.c里
	if(0==istate)
	{
		pcdParam.UnloadNumber=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
		printf("pcdParam.UnloadNumber = %d\n",pcdParam.UnloadNumber);
		//printf("%02x,%02x,%02x,%02x,",read_buffer[0],read_buffer[1],read_buffer[2],read_buffer[3]);
	}
	else	
	{
		printf("Error!	Read the param 'PCD_FM_UNLOAD' failed!\n");
		return false;
	}


	//当前TTC
	istate=pcdFmRead(PCD_FM_TTC, read_buffer, 4);       //fj:在oilFRam.c里
	if(0==istate)
	{	
		pcdParam.TTC=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
    	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_TTC  ",read_buffer,4);
	}
	else	
	{
		printf("Error!	Read the param 'PCD_FM_TTC' failed!\n");
		return false;
	}

#if 0
	//打开账单数据
	pcdParam.FdRecord=fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE);
	if(ERROR==pcdParam.FdRecord)	printf("Error!	Open the file %s failed!\n", PCD_FILE_OILRECORD);
	//打开异常账单索引数据文件
	pcdParam.FdZDUnnormal=fileOpen(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE);
	if(ERROR==pcdParam.FdZDUnnormal)	printf("Error!	Open the file %s failed!\n", PCD_FILE_ZD_UNNORMAL);

#endif



/**********************************************************
*	消息，任务等基本操作相关初始化
***********************************************************/
	//PCD与PC间通讯串口

	pcdParam.comFdPc=COM16;

	//初始化IPT向PC查询数据的数据节点为空
	pcdParam.ipt2PcNode=NULL;

	//创建链表，当有IPT数据需要转发到PC时存储于此链表
	lstInit(&pcdParam.ipt2PcList);

	//看门狗定时器，时间间隔50 ticks,fj:看门狗先不加上。
	//pcdParam.wdgId=wdCreate();
	//if(NULL==pcdParam.wdgId)	printf("Error! Create watch dog timer 'PcdWdgId' failed!\n");
	//else										wdStart(pcdParam.wdgId, 1, (FUNCPTR)pcdWdgIsr, 0);
	
	pcdParam.comFdPrint1=COM13;  //本主板1号打印机连接串口
	pcdParam.comFdPrint2=COM17;  //本主板2号打印机连接串口

	pcdParam.msgIdRx = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(pcdParam.msgIdRx < 0)
	{
	    printf("Error!	Creat messages  'pcdParam.msgIdRx' failed!\n");
		perror("get pcdParam.msgIdRx is error");
		return false;
	}
	else
	{
		printf("------pcd:create mssage msgIdRx is success\n");
	}

	pcdParam.msgIdFromIpt = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(pcdParam.msgIdFromIpt < 0)
	{
	    printf("Error!	Creat messages  'pcdParam.msgIdFromIpt' failed!\n");
        perror("get pcdParam.msgIdFromIpt is error");
		return false;
	}
	else
	{
		printf("------pcd:create mssage msgIdFromIpt is success\n");
	}
	
	//int nKey;
	//nKey = ftok(".",0x01);
	//if(nKey == -1)
	//{
	//	printf("create pcd to ipt msg key error!\n");
	//	return false;
	//}

	/*pcdParam.msgIdFromIpt = msgget(IPC_PRIVATE,IPC_CREAT|0666);
	if(pcdParam.msgIdFromIpt < 0)
	{
	    printf("Error!	Creat messages  'pcdParam.msgIdFromIpt' failed!\n");
        perror("get pcdParam.msgIdFromIpt is error");
		return false;
	}*/

/*	nKey = ftok(".",0x02);
	if(nKey == -1)
	{
        printf("create pcd to pc msg key error!\n");
		return false;
	}

	pcdParam.msgIdFromPc = msgget(nKey,IPC_CREAT|0666);
	if(pcdParam.msgIdFromPc < 0)
	{
	    printf("Error!	Creat messages  'pcdParam.msgIdFromPc' failed!\n");
		perror("get pcdParam.msgIdFromPc is error");
		return false;
	}*/

	pcdParam.msgIdFromPc = msgget(IPC_PRIVATE,IPC_CREAT|IPC_EXCL|0666);
	if(pcdParam.msgIdFromPc < 0)
	{
	    printf("Error!	Creat messages  'pcdParam.msgIdFromPc' failed!\n");
		perror("get pcdParam.msgIdFromPc is error");
		return false;
	}
	else
	{
		printf("------pcd:create mssage msgIdFromPc is success\n");
	}


	//创建PCD接收信息的消息队列
	//pcdParam.msgIdRx = msgQCreate(10, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdRx)		printf("Error!	Creat messages  'pcdParam.msgIdRx' failed!\n");

	//创建PCD接收IPT数据消息队列
	//pcdParam.msgIdFromIpt=msgQCreate(PCD_MSGNB, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdFromIpt)		printf("Error!	Creat messages  'pcdParam.msgIdFromIpt' failed!\n");

	//创建PCD接收PC数据消息队列
	//pcdParam.msgIdFromPc=msgQCreate(PCD_MSGNB, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdFromPc)		printf("Error!	Creat messages  'pcdParam.msgIdFromPc' failed!\n");
	
	
	
	
	//PCD模块接收PC管控返回数据任务初始化
	//pcdParam.tIdPcReceive=taskSpawn("tPcd2PcRx", 153, 0, 0x1000, (FUNCPTR)tPcd2PcRx, 0,1,2,3,4,5,6,7,8,9);
	//if(!(OK==taskIdVerify(pcdParam.tIdPcReceive)))		printf("Error!	Creat task 'tIdPcReceive' failed!\n");

	//创建PCD处理任务
	//pcdParam.tIdProcess=taskSpawn("tPcd", 153, 0, 0x8000, (FUNCPTR)tPcdProcess, 0,1,2,3,4,5,6,7,8,9);
	//if(!(OK==taskIdVerify(pcdParam.tIdProcess)))		printf("Error!	Creat task 'tPcd' failed!\n");   


	return true;
}


/********************************************************************
*Name				:pcdExit
*Description		:PCD模块功能注销
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void pcdExit(void)
{
	return;
}


