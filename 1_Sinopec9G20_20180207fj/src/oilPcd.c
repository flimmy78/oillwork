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
//#include "oilIpt.h"/*2017-01-22��ȼ����ҿ����*/

#include "../inc/main.h"


//LOCAL SEM_ID semIDPcdBill = NULL;
pthread_mutex_t mutexIDPcdBill;

static PcdListInfoType pcdBaseList;							//����������
static PcdListInfoType pcdAddList;							//����������
static PcdListInfoType pcdDelList;							//��ɾ������
static PcdListInfoType pcdWhiteList;						//������
static PcdOilRecordType pcdOilInfo;							//��Ʒ�ͼ۱�
static PcdStationInfoType pcdStationInfo;				    //ͨ����Ϣ
static PcdParamStructType pcdParam;						    //PCD����������ݽṹ
unsigned char Roll_Flag = 0;                                //��ѯ��־,szb_fj_20171120


/********************************************************************
*Name				:pcdFmRead
*Description		:PCD���ݴ��������
*Input				:param_offset	����
*						:buffer				��Ч����
*						:maxbytes		��Ч���ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
static int pcdFmRead(off_t param_offset, unsigned char *buffer, int maxbytes)
{
	  return framRead(FM_ADDR_PCD_SINO, param_offset, buffer, maxbytes); //fj:���������oilFRam.c��
	  //return 0;
}


/********************************************************************
*Name				:pcdFmWrite
*Description		:PCD���ݴ������磬У�鲢����
*Input				:param_offset	����
*						:buffer				��Ч����
*						:nbytes			��Ч���ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
int pcdFmWrite(off_t param_offset, unsigned char *buffer, int nbytes)  //fj:typedef long off_t;�粻֧��ֱ���޸ĳ�long
{
	unsigned char wrbuffer[PCD_FM_DATALEN+2]={0};  //fj:20170929,��ֹ����޸�,fj:20171120
	int crc_return=0;

	//printf("param_offset = %d\n",param_offset);
	
	if((param_offset+nbytes)>PCD_FM_DATALEN) //�жϳ��ȣ����ܳ�����Ч���ݳ���
	{
		return ERROR;
	}

	//printf("--fmwrite aaaaa\n");

	//fj:20170920,�����������غ�����ע��
	if(0!=framRead(FM_ADDR_PCD_SINO, 0, wrbuffer, PCD_FM_DATALEN)) //����ȫ����Ч���� ,fj:ͬ�ϣ�����Ŀǰ������
	{
		  return ERROR;
	}

	//printf("--fmwrite bbbbb\n");

	//printf("pcdFmWrite 11111\n");
	memcpy(&wrbuffer[param_offset], buffer, nbytes);//��������Ҫ�洢������
	crc_return=crc16Get(wrbuffer, PCD_FM_DATALEN); //������Ч����У��ֵ
	wrbuffer[PCD_FM_DATALEN+0]=(char)(crc_return>>8); //fj:���ԵĴ����������
	wrbuffer[PCD_FM_DATALEN+1]=(char)(crc_return>>0);

	//printf("--fmwrite cccc\n");

	//printf("pcdFmWrite 22222\n");	
	framWrite(FM_ADDR_PCD_SINO, 0, wrbuffer, PCD_FM_DATALEN+2);		//������Ч����,fj:ͬ��										
	framWrite(FM_ADDR_PCD_SINO, PCD_FM_BACKUP, wrbuffer, PCD_FM_DATALEN+2);	 //�������ݣ�fj:ͬ��

	//printf("--fmwrite dddd\n");

	return 0;
}


/********************************************************************
*Name				:pcd2OtherSend
*Description		:PCD���������巢������
*Input				:mboard_id		�����
*						:buffer				�����͵�����
*						:nbytes				�����͵����ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
static int pcd2OtherSend(unsigned char mboard_id, unsigned char *buffer, int nbytes)
{
	return 0;
}


/********************************************************************
*Name				:pcd2OtherSend
*Description		:PCD��IPT��������
*Input				:mboard_id		�����
*						:buffer				�����͵�����
*						:nbytes				�����͵����ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
static int pcd2IptSend(
	unsigned char msg_type, 		 //��Ϣ����
	unsigned char mboard_id,		 //�����
	unsigned char fuelling_point,//֧���ն˺�
	unsigned char phynozzle,		 //����ǹ��
	unsigned char command,			 //������
	unsigned char *buffer,			 //����
	int nbytes)								   //���ݳ���
{
	unsigned char tx_buffer[PCD_MSGMAX]={0};
	int tx_len=0;
	struct msg_struct msg_stPcd;
	msg_stPcd.msgType = 2;  //fj:20170918

	if(5+nbytes > PCD_MSGMAX)	 //�ж����ݳ���
		return ERROR;

	//��֯����
	tx_buffer[0]=msg_type;							    //��Ϣ����
	tx_buffer[1]=mboard_id;							    //�����
	tx_buffer[2]=fuelling_point;				    //֧���ն˺�   
	tx_buffer[3]=phynozzle;							    //����ǹ��
	tx_buffer[4]=command;							      //������  
	memcpy(&tx_buffer[5], buffer, nbytes);	//����
	tx_len=5+nbytes;

	//�������巢�͵���Ӧ��IPT������Ϣ���У�����������ת������Ӧ����
	if(pcdParam.mboardID==mboard_id)	
	{
		 //printf("Msg Buffer ,tx_len = %d: \n",tx_len);
	     //PrintH(tx_len,tx_buffer);
		 //msgQSend((MSG_Q_ID)iptMsgIdRead(fuelling_point), tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL); //fj:��ע�ͣ���һ����
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
*Description		:IPT����ͨ��PCDת����PC��̨
*Input				:
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
static int pcdIpt2PcSend(
	unsigned char msg_type, 			 //��Ϣ����
	unsigned char mboard_id, 			 //�����
	unsigned char fuelling_point,  //֧���ն˺�
	unsigned char phynozzle, 			 //����ǹ��
	unsigned char command,				 //������
	unsigned char *buffer,				 //����
	int nbytes)									   //���ݳ���
{
	PcdIpt2PcNode *node=NULL;  //fj:�ô���һ��VXWORKS��NODE���ܲ����ã��Ƿ��linux��һ��

	if(nbytes>128) //�ж����ݳ���
	{
		return 1;
	}
	
	if(lstCount(&pcdParam.ipt2PcList)>=10) //���ڵ���10
	{
		return 1;
	}

	node=(PcdIpt2PcNode*)malloc(sizeof(PcdIpt2PcNode)); //����ռ�
	if(NULL==node)
	{
		return 1;
	}

	//��ӽڵ�����
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
	lstAdd(&pcdParam.ipt2PcList, (NODE*)node); //fj:vxworks�ĺ����Ȳ�����

	return 0;
}


/********************************************************************
*Name				:pcd2PcSend
*Description		:���ݷ�װ����PC��̨����
*Input				:pos_p		ͨѶPOS_P
*						:frame		֡�ţ�0~3f
*						:buffer		��Ч����
*						:nbytes		��Ч���ݳ���
*Output			:None
*Return				:0=�ɹ�������=ʧ��
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

	if(nbytes+8>512)	 //�жϳ���
		return ERROR;

	//��֯����
	data_buffer[0]=0xfa;														//���ݰ�ͷ
	data_buffer[1]=0;																//Ŀ���ַ
	data_buffer[2]=pcdStationInfo.POS_P;						//Դ��ַ
	pcdParam.pcFrame++;													    //֡��/����
	if(pcdParam.pcFrame>0x3f)	
		pcdParam.pcFrame=0;
	data_buffer[3]=(0<<7)|(0<<6)|(pcdParam.pcFrame);	
	data=hex2Bcd(nbytes);													   //��Ч���ݳ���
	data_buffer[4]=(char)(data>>8);	data_buffer[5]=(char)(data>>0);
	memcpy(&data_buffer[6], buffer, nbytes);						//��Ч����							

	//CRCУ��
	crc_data=crc16Get(&data_buffer[1], 5+nbytes);							
	data_buffer[6+nbytes]=(char)(crc_data>>8);
	data_buffer[7+nbytes]=(char)(crc_data>>0);

	//���0xfa
	memcpy(&tx_buffer[0], &data_buffer[0], 6);	tx_len=6;
	for(i=6; i<8+nbytes; i++)
	{	
		tx_buffer[tx_len]=data_buffer[i];	//�������ݸ�ֵ
		tx_len++;

		if(tx_len>=512)	//��ֹ���ͳ������
			break;
			
		if(0xfa==data_buffer[i]) //���͵�����Ϊ0xfaʱ���һλ0xfa
		{
			tx_buffer[tx_len]=data_buffer[i];	
			tx_len++;
		}
	
		if(tx_len>=512)	//��ֹ���ͳ������
			break;
	}


	//printf("--------->oil to pc server data \n");
	//PrintH(tx_len,tx_buffer);

	//���ݷ���
	kjldWrite(tx_buffer, tx_len); //fj:�ú�����oilKJLD.c�����ʯ�������������ͺ�̨����

	
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
*Description		:��/���������ݲ���
*Input				:fd			�ļ�
*						:number	������Ŀ
*						:buffer		����
*Output			:None
*Return				:0=ƥ�䣻����=��ƥ��
*History			:2014-03-25,modified by syj
*/
static int pcdListSearch(const char *path, unsigned int number, const unsigned char *buffer)
{
	unsigned int mid=0, start=0, end=0;
	unsigned char read_buffer[10]={0};
	int i=0;

	if(0==number) //�ж�������Ŀ��Ϊ0
	{
		return 1;
	}

	//�ж���ʼλ������
	start=0;
	fileReadForPath(path, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 16+10*start, read_buffer, 10);
	if(0==memcmp(buffer, read_buffer, 10))	
		return 0;

	//�жϽ���λ������
	end=number;
	fileReadForPath(path, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  16+10*end, read_buffer, 10);
	if(0==memcmp(buffer, read_buffer, 10))	
		return 0;

	//�ж��м�λ������
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
*Description		:�ڱ���������TTC
*Input				:phynozzle	����ǹ��
*						:buffer			�˵����ݣ��̶�128bytes
*						:nbytes			�˵����ݳ��ȣ��̶�Ϊ128
*Output			:TTC				�˵�TTC
*Return				:0=�ɹ���1=����ǹ�ŷǷ�������=����
*History			:2014-03-25,modified by syj
*/
static int pcdLocalTTCGet(int phynozzle, char *buffer, int nbytes, unsigned int *TTC)
{
	unsigned char read_buffer[256]={0}, wrbuffer[32]={0};
	unsigned int last_ttc=0, i_ttc=0;
	off_t i_offset=0;
	int istate = 0;

	//semTake(semIDPcdBill, WAIT_FOREVER); //fj:��ע�ͣ������
    pthread_mutex_lock(&mutexIDPcdBill);   //fj:20170917
	if(phynozzle<1 || phynozzle>PCD_NOZZLE_MAX) //�ж�����ǹ�źϷ���
	{
		istate = 1;
		goto DONE;
	}

	//�ж��Ƿ��ظ�����
	if(1==phynozzle)			last_ttc=pcdParam.TTC1;
	else if(2==phynozzle)	last_ttc=pcdParam.TTC2;
	else if(3==phynozzle)	last_ttc=pcdParam.TTC3;
	else if(4==phynozzle)	last_ttc=pcdParam.TTC4;
	else if(5==phynozzle)	last_ttc=pcdParam.TTC5;
	else if(6==phynozzle)	last_ttc=pcdParam.TTC6;

	//������һ�ʴ洢���˵��������TTC���˵���ͬ����Ϊ�Ѿ������TTC
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

	//�����ۼӺ��TTC
	i_ttc=pcdParam.TTC+1;

	//��������TTC���˵�
	buffer[0]=(char)(i_ttc>>24);	buffer[1]=(char)(i_ttc>>16);
	buffer[2]=(char)(i_ttc>>8);		buffer[3]=(char)(i_ttc>>0);
	i_offset=PCD_ZD_LEN*((i_ttc-1)%PCD_RECORD_MAX);
	fileWriteForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, buffer, PCD_ZD_LEN);

	//У���˵��洢
	fileReadForPath(PCD_FILE_OILRECORD, O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(0!=memcmp(buffer, read_buffer, PCD_ZD_LEN))
	{
		jljUserLog("����!����ǹ��:%d��TTC:%d������TTCʧ�ܣ�д���������һ��!\n", phynozzle, pcdParam.TTC); 
		istate = 2;
		goto DONE;
	}

	//POS_TTC�ۼӲ�����
	pcdParam.TTC=i_ttc;
	read_buffer[0]=(char)(pcdParam.TTC>>24);	read_buffer[1]=(char)(pcdParam.TTC>>16);
	read_buffer[2]=(char)(pcdParam.TTC>>8);		read_buffer[3]=(char)(pcdParam.TTC>>0);
	pcdFmWrite(PCD_FM_TTC, read_buffer, 4); //fj:���غ���

	//����TTC
	*TTC=pcdParam.TTC;

	//�����˵���Ŀ�ۼ�
	pcdParam.UnloadNumber++;
	read_buffer[0]=(char)(pcdParam.UnloadNumber>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber>>16);
	read_buffer[2]=(char)(pcdParam.UnloadNumber>>8);		read_buffer[3]=(char)(pcdParam.UnloadNumber>>0);
	pcdFmWrite(PCD_FM_UNLOAD, read_buffer, 4);

	
	//�ӿ��˵�����������Ϣ��������ұ��ػҿ���Ϣʱ�Ĳ�ѯ
	//���쳣�˵������ļ���ѭ���洢�쳣�˵���TTC�ţ�ʱ�����Ϣ
	
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

	//��������ǹ�ţ���������ÿ��������ǹ������
	if(1==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC1=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC1>>24);		wrbuffer[1]=(char)(pcdParam.TTC1>>16);
		wrbuffer[2]=(char)(pcdParam.TTC1>>8);		wrbuffer[3]=(char)(pcdParam.TTC1>>0);
		pcdFmWrite(PCD_FM_TTC1, wrbuffer, 4);

	   // g_fjLog.WriteLog("pcdLocalTTCGet  ","pcdFmWrite_PCD_FM_TTC1  ",wrbuffer,4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber1++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber1>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber1>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber1>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber1>>0);
		pcdFmWrite(PCD_FM_ZDNUM1, wrbuffer, 4);

        //g_fjLog.WriteLog("pcdLocalTTCGet  ","pcdFmWrite_PCD_FM_ZDNUM1  ",wrbuffer,4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber1++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber1>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber1>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber1>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber1>>0);
		pcdFmWrite(PCD_FM_UNLOAD1, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC1>>24);		wrbuffer[1]=(char)(pcdParam.TTC1>>16);
		wrbuffer[2]=(char)(pcdParam.TTC1>>8);		wrbuffer[3]=(char)(pcdParam.TTC1>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_1, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber1-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(2==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC2=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC2>>24);		wrbuffer[1]=(char)(pcdParam.TTC2>>16);
		wrbuffer[2]=(char)(pcdParam.TTC2>>8);		wrbuffer[3]=(char)(pcdParam.TTC2>>0);
		pcdFmWrite(PCD_FM_TTC2, wrbuffer, 4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber2++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber2>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber2>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber2>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber2>>0);
		pcdFmWrite(PCD_FM_ZDNUM2, wrbuffer, 4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber2++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber2>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber2>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber2>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber2>>0);
		pcdFmWrite(PCD_FM_UNLOAD2, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC2>>24);		wrbuffer[1]=(char)(pcdParam.TTC2>>16);
		wrbuffer[2]=(char)(pcdParam.TTC2>>8);		wrbuffer[3]=(char)(pcdParam.TTC2>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_2, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber2-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(3==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC3=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC3>>24);		wrbuffer[1]=(char)(pcdParam.TTC3>>16);
		wrbuffer[2]=(char)(pcdParam.TTC3>>8);		wrbuffer[3]=(char)(pcdParam.TTC3>>0);
		pcdFmWrite(PCD_FM_TTC3, wrbuffer, 4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber3++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber3>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber3>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber3>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber3>>0);
		pcdFmWrite(PCD_FM_ZDNUM3, wrbuffer, 4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber3++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber3>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber3>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber3>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber3>>0);
		pcdFmWrite(PCD_FM_UNLOAD3, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC3>>24);		wrbuffer[1]=(char)(pcdParam.TTC3>>16);
		wrbuffer[2]=(char)(pcdParam.TTC3>>8);		wrbuffer[3]=(char)(pcdParam.TTC3>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_3, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber3-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(4==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC4=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC4>>24);		wrbuffer[1]=(char)(pcdParam.TTC4>>16);
		wrbuffer[2]=(char)(pcdParam.TTC4>>8);		wrbuffer[3]=(char)(pcdParam.TTC4>>0);
		pcdFmWrite(PCD_FM_TTC4, wrbuffer, 4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber4++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber4>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber4>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber4>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber4>>0);
		pcdFmWrite(PCD_FM_ZDNUM4, wrbuffer, 4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber4++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber4>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber4>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber4>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber4>>0);
		pcdFmWrite(PCD_FM_UNLOAD4, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC4>>24);		wrbuffer[1]=(char)(pcdParam.TTC4>>16);
		wrbuffer[2]=(char)(pcdParam.TTC4>>8);		wrbuffer[3]=(char)(pcdParam.TTC4>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_4, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber4-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(5==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC5=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC5>>24);		wrbuffer[1]=(char)(pcdParam.TTC5>>16);
		wrbuffer[2]=(char)(pcdParam.TTC5>>8);		wrbuffer[3]=(char)(pcdParam.TTC5>>0);
		pcdFmWrite(PCD_FM_TTC5, wrbuffer, 4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber5++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber5>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber5>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber5>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber5>>0);
		pcdFmWrite(PCD_FM_ZDNUM5, wrbuffer, 4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber5++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber5>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber5>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber5>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber5>>0);
		pcdFmWrite(PCD_FM_UNLOAD5, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC5>>24);		wrbuffer[1]=(char)(pcdParam.TTC5>>16);
		wrbuffer[2]=(char)(pcdParam.TTC5>>8);		wrbuffer[3]=(char)(pcdParam.TTC5>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_5, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber5-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}
	else if(6==phynozzle)
	{
		//���汾ǹ���һ��TTC
		pcdParam.TTC6=pcdParam.TTC;
		wrbuffer[0]=(char)(pcdParam.TTC6>>24);		wrbuffer[1]=(char)(pcdParam.TTC6>>16);
		wrbuffer[2]=(char)(pcdParam.TTC6>>8);		wrbuffer[3]=(char)(pcdParam.TTC6>>0);
		pcdFmWrite(PCD_FM_TTC6, wrbuffer, 4);

		//�˵���Ŀ�ۼӲ�����
		pcdParam.ZDNumber6++;
		wrbuffer[0]=(char)(pcdParam.ZDNumber6>>24);	wrbuffer[1]=(char)(pcdParam.ZDNumber6>>16);
		wrbuffer[2]=(char)(pcdParam.ZDNumber6>>8);		wrbuffer[3]=(char)(pcdParam.ZDNumber6>>0);
		pcdFmWrite(PCD_FM_ZDNUM6, wrbuffer, 4);

		//δ���˵���Ŀ�ۼӲ�����
		pcdParam.UnloadNumber6++;
		wrbuffer[0]=(char)(pcdParam.UnloadNumber6>>24);	wrbuffer[1]=(char)(pcdParam.UnloadNumber6>>16);
		wrbuffer[2]=(char)(pcdParam.UnloadNumber6>>8);	wrbuffer[3]=(char)(pcdParam.UnloadNumber6>>0);
		pcdFmWrite(PCD_FM_UNLOAD6, wrbuffer, 4);

		//�����˵�������4bytes TTC+7bytesʱ��
		memset(wrbuffer, 0, 16);
		wrbuffer[0]=(char)(pcdParam.TTC6>>24);		wrbuffer[1]=(char)(pcdParam.TTC6>>16);
		wrbuffer[2]=(char)(pcdParam.TTC6>>8);		wrbuffer[3]=(char)(pcdParam.TTC6>>0);
		memcpy(&wrbuffer[4], &buffer[5], 7);
		fileWriteForPath(PCD_FILE_ZDINDEX_6, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  PCD_ZD_INDEX_SIZE*((pcdParam.ZDNumber6-1)%PCD_ZD_INDEX_MAX), wrbuffer, PCD_ZD_INDEX_SIZE);

	}

DONE:
	//semGive(semIDPcdBill); //fj:��ע�ͣ�����
	pthread_mutex_unlock(&mutexIDPcdBill); //fj:20170917
	
	return istate;
}


/********************************************************************
*Name				:pcdLocalBillSave
*Description		:�ڱ����屣���˵�������ֵΪ1ʱ����������TTC�����д洢
*Input				:phynozzle	����ǹ��
*						:buffer			�˵����ݣ��̶�128bytes
*						:nbytes			�˵����ݳ��ȣ��̶�Ϊ128
*Output			:None
*Return				:0=�ɹ���1=TTC����;2=TTCָ��λ�����ݲ�һ�£�3=�洢ʧ�ܣ�4=�洢У��ʧ��
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

	//semTake(semIDPcdBill, WAIT_FOREVER); //fj:��ע�ͣ�����
	pthread_mutex_lock(&mutexIDPcdBill); //fj:20170917

	//�����˵���TTC
	pos_ttc=(buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|(buffer[3]<<0);

	if(0==pos_ttc) //�ж�TTC
	{
		istate = 1;
		goto DONE;
	}

	//����ƫ��
	i_offset=PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX);

	//�ж��Ƿ�������TTCʱ�洢һ�£��ȶ�T_MAC֮ǰ������(������T_MAC)
	fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(0!=memcmp(read_buffer, buffer, 95-4))
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("�˵��洢д�����!������TTCʱ�洢���ݲ�һ��!\n");  
		istate = 2;
		goto DONE;
	}

	//�����˵�ʧ���򷵻�ʧ��
	wlen=fileWriteForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, buffer, PCD_ZD_LEN);
	if(ERROR==wlen)
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("�˵��洢д�����![%s]\n", printbuffer); 
		istate = 3;
		goto DONE;
	}

	//��ȡ�˵�����д��У��
	wlen=fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE,  i_offset, read_buffer, PCD_ZD_LEN);
	if(ERROR!=wlen && 0!=memcmp(read_buffer, buffer, PCD_ZD_LEN))
	{
		for(i=0; i<PCD_ZD_LEN; i++)	sprintf(printbuffer+strlen(printbuffer), "_%2x", buffer[i]);
		jljUserLog("�˵��洢У�����![%s]\n", printbuffer); 
		istate = 4;
		goto DONE;
	}

DONE:
	
	//semGive(semIDPcdBill); //fj:�Ȳ�����
	pthread_mutex_unlock(&mutexIDPcdBill); //fj:20170917
	
	return istate;
}


/********************************************************************
*Name				:pcdLocalWBListGet
*Description		:�ڱ��غڰ�������ѯ
*						:inbuffer		���뻺�棬��Ҫ��ѯ�Ŀ�������Ϣ
*						:					����(10ѹ��BCD) +���һ�ʽ�������(4ѹ��BCD���û�����Ч�������������޽���ʱ��0)
*						:nbytes			���뻺�泤��
*						:maxbytes	���������󳤶�
*Output			:isMatch		ƥ���ʶ(1Bin, b0=0:ƥ��/����:��ƥ��) 
*Return				:0=�ɹ�������=ʧ�ܣ���������ѯ
*History			:2013-08-05,modified by syj
*/
static int pcdLoaclWBListGet(int phynozzle, char *inbuffer, int nbytes, char *isMatch)
{
	unsigned char real_time[7]={0}, card_number[10]={0}, bill_time[4]={0}, card_type=0, card_area=0, is_ok=0;
	RTCTime rtime;
	unsigned int number=0;

	memcpy(card_number, &inbuffer[0], 10); //��������
	memcpy(bill_time, &inbuffer[10], 4); //���������һ�ʽ�������
	card_type=card_number[2]; //����������
	card_area=((card_number[3]&0x0f)<<4)|((card_number[4]>>4)&0x0f); //������������

	//������ǰʱ��
	timeRead(&rtime); //fj:�ú�����oilStmTransmit.c��
	real_time[0]=rtime.century;	real_time[1]=rtime.year;	real_time[2]=rtime.month;		real_time[3]=rtime.date;
	real_time[4]=rtime.hour;		real_time[5]=rtime.minute;	real_time[6]=rtime.second;

	/*
	*	������������ʱ�û�����ѯ���ذ�������
	*	�ÿ�Ϊ�ڲ���(���û���)��
	*	δ���������ذ������Ĳ�����
	*	��ǰʱ��Ϸ���
	*	��������ʼʱ�䣬��ֹʱ����Ϸ���
	*	�������汾�Ϸ���������Ч���ڣ�
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
	*	������������ʱ�û�����ѯ���غ�������
	*	�ÿ�Ϊ�û�����
	*	δ���������ػ�����������
	*	δ����������������������
	*	δ������������ɾ�������Ĳ�����
	*	�ÿ�Ϊ��ʡ����
	*	��ǰʱ��Ϸ���
	*	������������ʼʱ�䣬��ֹʱ����Ϸ���
	*	�����������汾�Ϸ���������Ч���ڣ�
	*	������������ʼʱ�䣬��ֹʱ����Ϸ���
	*	�����������汾�Ϸ���������Ч���ڣ�
	*	��ɾ��������ʼʱ�䣬��ֹʱ����Ϸ���
	*	��ɾ�������汾�Ϸ���������Ч���ڣ�
	*/
	//*
	
	printf("card_area = %d,pcdStationInfo.Province = %d\n",card_area,pcdStationInfo.Province);

	if(!(PCD_DOWN_BASELIST!=pcdParam.PcDownloadContent ))	printf("%s:�������ػ������������޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(PCD_DOWN_ADDLIST!=pcdParam.PcDownloadContent))		printf("%s:���������������������޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(PCD_DOWN_DELLIST!=pcdParam.PcDownloadContent))		printf("%s:����������ɾ���������޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(card_area==pcdStationInfo.Province))								printf("%s:�Ǳ�ʡ�û������޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(0==timeVerification(real_time, 7)))										printf("%s:����ʱ��Ƿ����޷���ѯ���غ�����!\n", __FUNCTION__);

	if(!(0==timeVerification(pcdBaseList.TimeStart, 4)))				printf("%s:�����������������ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdBaseList.TimeFinish, 4)))				printf("%s:������������ֹ���ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdBaseList.TimeStart, real_time, 4)<=0))			printf("%s:����������δ���ã��޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdBaseList.TimeFinish, real_time, 4)>=0))			printf("%s:�����������ѹ��ڣ��޷���ѯ���غ�����!\n", __FUNCTION__);

	printf("real_time:\n");
	PrintH(4,real_time);
	printf("pcdBaseList.TimeStart:\n");
	PrintH(4,pcdBaseList.TimeStart);

	if(!(0==timeVerification(pcdAddList.TimeStart, 4)))					printf("%s:�����������������ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdAddList.TimeFinish, 4)))				printf("%s:������������ֹ���ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdAddList.TimeStart, real_time, 4)<=0))			printf("%s:����������δ���ã��޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdAddList.TimeFinish, real_time, 4)>=0))			printf("%s:�����������ѹ��ڣ��޷���ѯ���غ�����!\n", __FUNCTION__);

	if(!(0==timeVerification(pcdDelList.TimeStart, 4)))					printf("%s:�����������������ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(0==timeVerification(pcdDelList.TimeFinish, 4)))					printf("%s:������������ֹ���ڷǷ����޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdDelList.TimeStart, real_time, 4)<=0))				printf("%s:����������δ���ã��޷���ѯ���غ�����!\n", __FUNCTION__);
	if(!(memcmp(pcdDelList.TimeFinish, real_time, 4)>=0))				printf("%s:�����������ѹ��ڣ��޷���ѯ���غ�����!\n", __FUNCTION__);
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
		//��ɾ�������鵽�򷵻طǺ�������
		number=(pcdDelList.Number[0]<<24)|(pcdDelList.Number[1]<<16)|(pcdDelList.Number[2]<<8)|(pcdDelList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_DELLIST, number, card_number))
		{
			*isMatch=1;
			return 0;
		}

		//�����������鵽�򷵻غ�������
		number=(pcdAddList.Number[0]<<24)|(pcdAddList.Number[1]<<16)|(pcdAddList.Number[2]<<8)|(pcdAddList.Number[3]<<0);
		if(0==pcdListSearch(PCD_FILE_ADDLIST, number, card_number))
		{
			*isMatch=0;
			return 0;
		}

		//�����������鵽��Ϊ��������������Ϊ�Ǻ�������
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
*Description		:��ȡ�ҿ����׼�¼��Ϣ�����ܻ���һ���ĳ�ʱ�ȴ�ʱ��
*Input				:phynozzle	����ǹ��
*						:inbuffer		���뻺�棬��Ҫ��ѯ�Ļ������ײ�����Ϣ
*						:					����(10ѹ��BCD) +���(4HEX) +CTC(2HEX) +�ۿ���Դ(1HEX) +���ڼ�ʱ��(7ѹ��BCD)
*						:nbytes			���뻺�泤��
*						:maxbytes	���������󳤶�
*Output			:outbuffer		������棬��ѯ���Ļ���������Ϣ
*						:					����(10ѹ��BCD) +���(4HEX) +���׶�(3HEX) +CTC(2HEX) 
*						:					+�ۿ���Դ(1HEX) +���ڼ�ʱ��(7ѹ��BCD) +�����֤��(4Bin) +PSAMӦ�ñ��(6Bin) +PSAM��TTC(4Bin)
*Return				:0=�ɹ�������=����
*History			:2013-08-05,modified by syj
*/
static int pcdLoaclGreyBillGet(int phynozzle, char *inbuffer, char *outbuffer)
{
	int i=0;
	unsigned char read_buffer[128]={0};
	unsigned int pos_ttc=0, last_ttc=0, serch_number = 0;

	if(pcdParam.TTC<1) //��ǰ���˵�������ʧ��
	{
		return ERROR;
	}

	if(pcdParam.AbnormalNumber<1) //��ǰ���쳣�˵�������ʧ��
	{
		return ERROR;
	}

	/*�����쳣�˵������м�¼��������Ϣ�������һ���쳣�˵���������
	*	����״̬ʱ���ҵ����δ�ϴ��ĵ�һ����Ϣ��ֹ��
	*	����״̬ʱ���ҵ���ǰ�洢������һ����Ϣ��ֹ��
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

	//�ж�����쳣�˵����࣬����ѯ�����500�� �޸���2016-04-01
	if(pcdParam.AbnormalNumber%PCD_ZD_INDEX_MAX > 500)	
	{
		serch_number = 500%PCD_ZD_INDEX_MAX;
	}
	else	
	{																						
		serch_number = pcdParam.AbnormalNumber%PCD_ZD_INDEX_MAX;
	}

	//����Ч��Χ�ڲ�ѯ�쳣�˵�
	for(i=0; i<(serch_number%PCD_ZD_INDEX_MAX); i++)
	{
		/*�����쳣�˵������������Ҫ��ѯ���˵�TTC*/
		fileReadForPath(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, ((pcdParam.AbnormalNumber-1-i)%PCD_ZD_INDEX_MAX)*PCD_ZDERRINFO_SIZE, read_buffer, 16);
		pos_ttc=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

		/*���ݴ�TTC��ȡ��Ӧλ����Ϣ
		*	����һ��
		*	���һ��CTCһ�¿ۿ���Դһ��ʱ��һ��
		*/
		fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), read_buffer, 95);
		if((0==memcmp(&inbuffer[0], &read_buffer[12], 10))&&\
			(0==memcmp(&inbuffer[10], &read_buffer[22], 4))&&\
			(0==memcmp(&inbuffer[14], &read_buffer[29], 2))&&\
			(inbuffer[16]==read_buffer[63])&&\
			(0==memcmp(&inbuffer[17], &read_buffer[5], 7))){
	
			memcpy(&outbuffer[0], &read_buffer[12], 10);						//����
			memcpy(&outbuffer[10], &read_buffer[22], 4);						//���
			
			//2017-01-22��ȼ�������ۿ�������, fj:iptIsLianda������oilIpt.c��
			if(1 == iptIsLianda(IptParamA.Id)||1 == iptIsLianda(IptParamB.Id))
			{
				memcpy(&outbuffer[14], &read_buffer[80], 3);						//���׶�
			}
			else
			{
				memcpy(&outbuffer[14], &read_buffer[26], 3);						//���׶�
			}
				
			memcpy(&outbuffer[17], &read_buffer[29], 2);						//CTC
			memcpy(&outbuffer[19], &read_buffer[63], 1);						//�ۿ���Դ
			memcpy(&outbuffer[20], &read_buffer[5], 7);							//ʱ��
			memcpy(&outbuffer[27], &read_buffer[35], 4);						//�����֤��
			memcpy(&outbuffer[31], &read_buffer[53], 6);						//PSAM���
			memcpy(&outbuffer[37], &read_buffer[59], 4);						//PSAM��TTC

			return 0;
		}

		//�Ѳ����������һ�ʣ��˳�
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
*Description		:��ĳTTCΪ��������ѯָ��������ǹ��/��һ��TTC
*						:phynozzle	����ǹ��
*						:base_ttc		����TTC���Դ�TTCΪ������ǹ��ѯ��һ�ʻ���һ�ʣ�0��ʾ���һ��
*						:handle			0=��ѯ�����˵���1=��ѯ����TTC��һ�ʣ�2=��ѯ����TTC��һ��
*Output			:isMatch		ƥ���ʶ(1Bin, b0=0:ƥ��/����:��ƥ��) 
*Return				:Ҫ��ѯ��TTC���޴��˵�����0
*History			:2013-08-05,modified by syj
*/
unsigned int pcdLocalPhsicalTTCGet(int phynozzle, unsigned int base_ttc, char handle)
{
	char local_bill_buffer[PCD_ZD_INDEX_SIZE]={0}, next_bill_buffer[PCD_ZD_INDEX_SIZE]={0};
	off_t ttc_offset=0;					  //���β�ѯ�����ļ���ƫ��
	unsigned int ireturn=0;				//����ֵ
	unsigned int number=0;			  //��������ǹ���˵���Ŀ
	unsigned int last_ttc=0;			//��������ǹ��ǰTTC
	unsigned int i_base_ttc=0;		//�Դ�TTCΪ����
	unsigned int tmp_ttc=0;			  //���������ļ��е�TTC
	int fd=ERROR, i=0;

	//�ж�����ǹ�źϷ���
	if(!(phynozzle>=1 && phynozzle<=PCD_NOZZLE_MAX))
	{
		return 0;
	}

	//�����˵�������TTC
	if(1==phynozzle){number=pcdParam.ZDNumber1;	last_ttc=pcdParam.TTC1;}
	if(2==phynozzle){number=pcdParam.ZDNumber2;	last_ttc=pcdParam.TTC2;}
	if(3==phynozzle){number=pcdParam.ZDNumber3;	last_ttc=pcdParam.TTC3;}
	if(4==phynozzle){number=pcdParam.ZDNumber4;	last_ttc=pcdParam.TTC4;}
	if(5==phynozzle){number=pcdParam.ZDNumber5;	last_ttc=pcdParam.TTC5;}
	if(6==phynozzle){number=pcdParam.ZDNumber6;	last_ttc=pcdParam.TTC6;}

	//�������TTCΪ0���ʾ��ѯ���һ���˵�
	if(0==base_ttc)	
		i_base_ttc=last_ttc;
	else						
		i_base_ttc=base_ttc;

	//�жϱ�ǹ���˵��������޴��˵�
	if(number<=0)
	{
		return 0;
	}

	//�жϲ�ѯTTC������ǰTTC�������޴��˵�
	if(base_ttc>last_ttc)
	{
		return 0;
	}

	//�жϲ�ѯ��ǰ���TTC����һ�ʣ������޴��˵�
	if(2==handle && base_ttc>=last_ttc)
	{
		return 0;
	}

	printf("pcdLocalPhsicalTTCGet ffffffffff,phynozzle = %d\n",phynozzle);

	//�򿪼�¼�ļ�
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


	//���������ļ����ݣ��������������������Ŀ����ǰ������Ŀ
	for(i=0; i<PCD_ZD_INDEX_MAX && i<number; )
	{
		ttc_offset=PCD_ZD_INDEX_SIZE*((number-1-i)%PCD_ZD_INDEX_MAX); //����Ҫ��ѯ�ļ�ƫ��λ��
		if(ERROR==fileRead(fd, ttc_offset, local_bill_buffer, PCD_ZD_INDEX_SIZE)) //���һ���TTC���ڵ�λ��
		{
			fileClose(fd);
			return 0;
		}

		tmp_ttc=(local_bill_buffer[0]<<24)|(local_bill_buffer[1]<<16)|(local_bill_buffer[2]<<8)|(local_bill_buffer[3]<<0); //���һ���TTC
		if(i_base_ttc==tmp_ttc)
		{	
			if(0==handle)			 //���ݲ����жϲ��Ҹñ��˵�������һ�ʻ���һ��
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

			if(i>=PCD_ZD_INDEX_MAX || i>=number) //�ж�����Ŀ���Ƿ�Ϸ�
			{
				fileClose(fd);
				return 0;
			}

			ttc_offset=PCD_ZD_INDEX_SIZE*((number-1-i)%PCD_ZD_INDEX_MAX); //����Ҫ��ѯ�ļ�ƫ��λ��
			
			if(ERROR==fileRead(fd, ttc_offset, local_bill_buffer, PCD_ZD_INDEX_SIZE)) //���һ���TTC���ڵ�λ��
			{ 
				fileClose(fd);
				return 0;
			}
			//���ҵ���TTC
			ireturn=(local_bill_buffer[0]<<24)|(local_bill_buffer[1]<<16)|(local_bill_buffer[2]<<8)|(local_bill_buffer[3]<<0);
			break;
		}

		//������Ŀ�ۼ�
		i++;
	}

	int nFileClose = fileClose(fd);

	//printf("pcdLocalPhsicalTTCGet hhhhhhhhhh,nFileClose = %d\n",nFileClose);

	//fileClose(fd); //�ر��ļ�
	return ireturn;
}


/********************************************************************
*Name				:pcd2IptProcess
*Description		:PCD��IPT��ͨѶ����
*Input				:None
*Output			:None
*Return				:None
*History			:2014-03-25,modified by syj
*/
static void pcd2IptProcess()
{
	/*PCD��֧���ն�(IPT)֮��ͨѶ
	*	IPTƽʱͨ����ѯ���ʵʱ��Ϣ���͵�PCD��PCD��ʵʱ��Ϣ���л���
	*	ĳ��֧���ն˳�ʱ������������ʱ��Ϊ��֧���ն���Ч��
	*/
	PcdFuellingPointInfoType *fp_info=NULL;				//֧���ն���Ϣ
	PcdPcInfoType pc_info;											  //��̨PC������Ϣ
	unsigned char msg_type=0;									    //��Ϣ����
	unsigned char mboard_id=0;								    //���������
	unsigned char fuelling_point=0;							  //��������
	unsigned char physical_nozzle=0;						  //����ǹ��
	unsigned char command=0;									    //������
	unsigned char msg_buffer[PCD_MSGMAX]={0}; 	  //���յ���Ϣ����
	int msg_len=0;													      //���յ���Ϣ���ݳ���
	unsigned char tx_buffer[PCD_MSGMAX]={0};		  //����ʱ���ݻ���
	int tx_len=0;														      //�������ݳ���
	unsigned char tmp_buffer[128]={0};					  //���ڴ洢��ʱ����
	unsigned int pos_ttc=0;										    //���뵽���˵�TTC
	unsigned int unload_number=0;							    //�����͵�δ�ϴ��˵�����
	unsigned int fuel_ttc=0;										  //�����͵㵱ǰTTC
	int i=0, istate=0;
	int voice[SPK_MAX_NUMBER]={0}, voice_len=0;		//�����б������б���Ŀ
	//int voice[32]={0}, voice_len=0;		//�����б������б���,fj:�޸�

	//�ж�ĳ��ǹ��ʱ2�������������ж���ǹ���ߣ������������ǹ��������
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

	//����IPT����
	//msg_len=msgQReceive(pcdParam.msgIdFromIpt, msg_buffer, PCD_MSGMAX, NO_WAIT); //fj:�ú���֮���ٴ���
	
	if(msg_len<0)	 //δ���յ���Ϣʱ������������
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

	//����֡�ţ�����ţ��������ţ�����ǹ�ţ�������
	msg_type=msg_buffer[0];	mboard_id=msg_buffer[1];	fuelling_point=msg_buffer[2];
	physical_nozzle=msg_buffer[3];	command=msg_buffer[4];

	//printf("command = %d\n",command);

	//�жϴ��������ǹ��
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

	//��¼��������ǹΪ����
	fp_info->OfflineTimer=0;	
	fp_info->IsOnline=PCD_FP_ONLINE;	
	fp_info->PhysicalNozzle=physical_nozzle;
	
	//��Ѱ�����¼��ǹ��Ϣ����������
	if(PCD_CMD_POLL==command)
	{
		fp_info->NozzleNumber=msg_buffer[5]; //��ǹ��
		for(i=0; i<3 && i<fp_info->NozzleNumber; i++) //�����֧���ն���Ϣ
		{
			memcpy(&fp_info->Nozzle[i].State, &msg_buffer[6+i*37], 1);					//��¼IPT״̬
			memcpy(&fp_info->Nozzle[i].LogicNozzle, &msg_buffer[7+i*37], 1);		//��¼�߼�ǹ��
			memcpy(&fp_info->Nozzle[i].SumMoney, &msg_buffer[8+i*37], 4);			  //��¼���۽��
			memcpy(&fp_info->Nozzle[i].SumVolume, &msg_buffer[12+i*37], 4);		  //��¼��������
			memcpy(&fp_info->Nozzle[i].CardID, &msg_buffer[16+i*37], 10);			  //��¼��Ӧ�ú�
			memcpy(&fp_info->Nozzle[i].CardState, &msg_buffer[26+i*37], 2);			//��¼��״̬
			memcpy(&fp_info->Nozzle[i].CardBalance, &msg_buffer[28+i*37], 4);		//��¼�����
			memcpy(&fp_info->Nozzle[i].PayUnit, &msg_buffer[32+i*37], 1);				//��¼���㵥λ/��ʽ		
			memcpy(&fp_info->Nozzle[i].Money, &msg_buffer[33+i*37], 4);				  //��¼���ͽ��		
			memcpy(&fp_info->Nozzle[i].Volume, &msg_buffer[36+i*37], 4);				//��¼���ͽ��
			memcpy(&fp_info->Nozzle[i].Price, &msg_buffer[39+i*37], 2);					//��¼���ͼ۸�
		}

		//��֯��̨PC����
		memcpy(&pc_info.SInfo, &pcdStationInfo, sizeof(PcdStationInfoType));
		memcpy(&pc_info.BList, &pcdBaseList, sizeof(PcdListInfoType));
		memcpy(&pc_info.ABList, &pcdAddList, sizeof(PcdListInfoType));
		memcpy(&pc_info.DBList, &pcdDelList, sizeof(PcdListInfoType));
		memcpy(&pc_info.WList, &pcdWhiteList, sizeof(PcdListInfoType));
		memcpy(&pc_info.OilInfo, &pcdOilInfo, sizeof(PcdOilRecordType));
		memcpy(&pc_info.Time, pcdParam.PcTime, 7);
	
		//����Ӧ������
		tx_buffer[0]=0;											//ִ�н��
		tx_buffer[1]=pcdParam.PcOnline;			//PC��̨����״̬
		tx_buffer[2]=0;											//�쳣����
																		    //������ǰTTC
		tx_buffer[3]=(char)(pcdParam.TTC>>24);	tx_buffer[4]=(char)(pcdParam.TTC>>16);
		tx_buffer[5]=(char)(pcdParam.TTC>>8);	tx_buffer[6]=(char)(pcdParam.TTC>>0);
																		
		tx_buffer[7]=(char)(fuel_ttc>>24);	tx_buffer[8]=(char)(fuel_ttc>>16); //��ǹ��ǰTTC
		tx_buffer[9]=(char)(fuel_ttc>>8);		tx_buffer[10]=(char)(fuel_ttc>>0);
																		
		tx_buffer[11]=(char)(pcdParam.UnloadNumber>>24);	tx_buffer[12]=(char)(pcdParam.UnloadNumber>>16); //����δ�ϴ��˵���Ŀ
		tx_buffer[13]=(char)(pcdParam.UnloadNumber>>8);	tx_buffer[14]=(char)(pcdParam.UnloadNumber>>0);
																		
		tx_buffer[15]=(char)(unload_number>>24);	tx_buffer[16]=(char)(unload_number>>16); //��ǹδ�ϴ��˵���Ŀ
		tx_buffer[17]=(char)(unload_number>>8);	tx_buffer[18]=(char)(unload_number>>0);
		i=sizeof(PcdPcInfoType);							//PC��Ϣ����
		tx_buffer[19]=(unsigned char)(i>>8);
		tx_buffer[20]=(unsigned char)(i>>0);
		memcpy(&tx_buffer[21], &pc_info, sizeof(PcdPcInfoType));	//PC��Ϣ
		tx_len=21+i;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//����TTC����
	if(PCD_CMD_FORTTC==command)
	{
		istate=pcdLocalTTCGet(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN, &pos_ttc); //��������TTC

		char chTemp[8] = {0};
		sprintf(chTemp,"%d",istate);
		//g_fjLog.WriteLog("pcd2IptProcess  ","pcdLocalTTCGet  ",chTemp,0);  //fj:20171027
		//printf("pcdLocalTTCGet: istate = %d",istate);

		//����TTC
		if(0==istate)	
			tx_buffer[0]=0;						//���
		else				
			tx_buffer[0]=1;
			
		tx_buffer[1]=(char)(pos_ttc>>24);			//����õ���TTC
		tx_buffer[2]=(char)(pos_ttc>>16);
		tx_buffer[3]=(char)(pos_ttc>>8);	
		tx_buffer[4]=(char)(pos_ttc>>0);
		memcpy(&tx_buffer[5], &msg_buffer[5+4], PCD_ZD_LEN-4);	//TTC֮���Խ�����������˵�����
		tx_len=5+PCD_ZD_LEN-4;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//�����˵�����
	if(PCD_CMD_ZDSAVE==command)
	{
		
		istate=pcdLocalBillSave(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN); //���ش���˵�

		//����TTC���
		if(0==istate)	
			tx_buffer[0]=0;							
		else				
			tx_buffer[0]=1;
			
		memcpy(&tx_buffer[1], &msg_buffer[5], 4);	//�洢���˵�TTC
		tx_len=5;
		pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
	}

	//��ѯ�ڰ���������
	if(PCD_CMD_LIST==command)
	{
		if(ICTYPE_USER==msg_buffer[5+2]) //fj:ICTYPE_USER,��oilIC.h
		{
			/*�û������Һ�����
			*	���ҳɹ���������ƥ�䣬ֱ�ӷ��ز��ҽ��
			*	���ҳɹ�����������ƥ�䣬����״̬���̨��
			*	���ҳɹ�����������ƥ�䣬������״̬�������һ�ʼ����������ڻ�������Ч��������ʾ��������ѯ��
			*	���ҳɹ�����������ƥ�䣬������״̬�������һ�ʼ������ڲ����ڻ�������Ч������������ͣ������ز�ƥ�䣻
			*	����ʧ�ܣ�����״̬���̨��
			*	����ʧ�ܣ�������״ֱ̬�ӷ���
			*/
			printf("ICTYPE_USER:  pcdLoaclWBListGet \n");
			istate=pcdLoaclWBListGet(physical_nozzle, &msg_buffer[5], 14, tmp_buffer);
			if(0==istate && 0==tmp_buffer[0]) //���ҳɹ���������ƥ�䣬ֱ�ӷ��ز��ҽ��
			{
				tx_buffer[0]=0;													    //���0=���ҳɹ�������=��������ѯ
				tx_buffer[1]=1;													    //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								  //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE==pcdParam.PcOnline) //���ҳɹ�����������ƥ�䣬����״̬���̨��
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			//���ҳɹ�����������ƥ�䣬������״̬�������һ�ʼ����������ڻ�������Ч��������ʾ��������ѯ��
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline && memcmp(&msg_buffer[15], pcdBaseList.TimeStart, 4)<0)
			{
				tx_buffer[0]=1;													     //���0=���ҳɹ�������=��������ѯ
				tx_buffer[1]=1;													     //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								   //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	 //����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			//���ҳɹ�����������ƥ�䣬������״̬�������һ�ʼ������ڲ����ڻ�������Ч������������ͣ������ز�ƥ�䣻
			else if(0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline && memcmp(&msg_buffer[15], pcdBaseList.TimeStart, 4)>=0)
			{
				tx_buffer[0]=0;													    //���0=���ҳɹ�������=��������ѯ
				tx_buffer[1]=1;													    //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								  //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if(0!=istate && PCD_PC_ONLINE==pcdParam.PcOnline) //����ʧ�ܣ�����״̬���̨��
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			else if(0!=istate && PCD_PC_ONLINE!=pcdParam.PcOnline) //����ʧ�ܣ�������״ֱ̬�ӷ���
			{
				tx_buffer[0]=1;													    //���0=���ҳɹ�������=��������ѯ
				tx_buffer[1]=1;													    //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								  //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
		}
		else
		{
			/*�ڲ�����ѯ������
			*	������ҳɹ���ƥ�䣬ֱ�ӷ��ز鵽��
			*	������ҳɹ��Ҳ�ƥ�䣬����״̬���ز�ƥ��Ĳ��ҽ����
			*	�������ʧ�ܻ�ƥ���������Ͽ�״̬��ֱ�ӷ�����������ѯ��
			*	�������ʧ�����������������̨��ѯ��
			*/
        	printf("----not ICTYPE_USER:  pcdLoaclWBListGet \n");
			istate=pcdLoaclWBListGet(physical_nozzle, &msg_buffer[5], 14, tmp_buffer);
			if((0==istate && 0==tmp_buffer[0])|| (0==istate && 0!=tmp_buffer[0] && PCD_PC_ONLINE!=pcdParam.PcOnline))
			{
				tx_buffer[0]=0;													    //���0=���ҳɹ�������=��������ѯ
				tx_buffer[1]=1;													    //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								  //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
			else if((0!=istate || 0!=tmp_buffer[0]) && PCD_PC_ONLINE==pcdParam.PcOnline)
			{
				pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 10);
			}
			else
			{	
				tx_buffer[0]=1;													    //���0=�鵽������=��������ѯ
				tx_buffer[1]=1;													    //����Դ0=��̨��/������;1=���غ�/������
				tx_buffer[2]=tmp_buffer[0];								  //ƥ���ʶ
				memcpy(&tx_buffer[3], &msg_buffer[5], 10);	//����
				tx_len=13;
				pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
			}
		}
	}

	//��ѯ�Ҽ�¼����
	if(PCD_CMD_GREYINFO==command)
	{	
		/*�����ڱ��ز��ң�����ҵ���ֱ�ӷ��أ�
		*	����δ�ҵ�δ�ҵ�ƥ���¼������״̬���̨��ѯ
		*	����δ�ҵ�δ�ҵ�ƥ���¼������״̬������ƥ���¼
		*/
		istate=pcdLoaclGreyBillGet(physical_nozzle, &msg_buffer[5], tmp_buffer);
		if(0==istate)
		{
			tx_buffer[0]=0x01;												//����Դ0=��̨��1=����
			tx_buffer[1]=0;														//���ҳɹ����ز��ҽ�����ɹ�
			memcpy(&tx_buffer[2], tmp_buffer, 41);		//���ҵ�������	
			tx_len=43;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
 		else if(PCD_PC_ONLINE==pcdParam.PcOnline)
 		{
			pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 24);
		}
		else
		{
			tx_buffer[0]=0x01;												//����Դ0=��̨��1=����
			tx_buffer[1]=1;														//����ʧ�ܷ�����ƥ���ʧ��
			memset(&tx_buffer[2], 0, 41);							//���ҵ�����������
			tx_len=43;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}

	//��ӡ��������
	if(PCD_CMD_PRINTE==command)
	{
		
		/*IPT����ӡ���ݷ��͵�PCD����ӡ��ѡ��b7~b4���������,b3~b0����֧���ն˺�
		*	��ӡ��ѡ��Ϊ���������ӵĴ�ӡ��ʱֱ�ӽ����ݷ��͵���ӡ��
		*	��ӡ��ѡ��Ǳ�������ʱ������ת������������
		*/
		i=(msg_buffer[5]>>0)&0x0f;
		
		if(0==i)	
			comWrite(pcdParam.comFdPrint1, &msg_buffer[8], (msg_buffer[6]<<8)|(msg_buffer[7]<<0)); //fj:�ڱ���ļ���
		else			
			comWrite(pcdParam.comFdPrint2, &msg_buffer[8], (msg_buffer[6]<<8)|(msg_buffer[7]<<0));
	}

	//������������
	if(PCD_CMD_SPK==command)
	{
		i=(msg_buffer[5]>>0)&0x0f;
		for(voice_len=0; voice_len<32 && voice_len<msg_buffer[7]; voice_len++) //fj:SPK_MAX_NUMBER,32
		{
			voice[voice_len]=(msg_buffer[8+2*voice_len]<<8)|(msg_buffer[9+2*voice_len]<<0);
		}
		if(0==i)	//fj:spkPlay��oilSpk.c��ʵ�ֵġ�
			spkPlay(SPK_NOZZLE_1, msg_buffer[6], voice, voice_len);
		else			
			spkPlay(SPK_NOZZLE_2, msg_buffer[6], voice, voice_len);
	}

//	printf("command = %d,msg_buffer[5] = %d\n",command,msg_buffer[5]);

	//��ѯ�˵�����ѯ�����˵�
	if(PCD_CMD_ZDCHECK==command && 0==msg_buffer[5])
	{
		printf("zhengji ---- pos_ttc = %d,physical_nozzle = %d\n",pos_ttc,physical_nozzle);

		pos_ttc=(msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0);
		if(0==pos_ttc)				pos_ttc=pcdParam.TTC;
		if(1==msg_buffer[10])	pos_ttc--;
		if(2==msg_buffer[10])	pos_ttc++;

		//Ҫ��ѯ��TTC�����Χ��û�д˱��˵�
		if(pos_ttc<1 || pos_ttc>pcdParam.TTC || (pcdParam.TTC>PCD_RECORD_MAX && pos_ttc<=(pcdParam.TTC-PCD_RECORD_MAX)))
		{
			tx_buffer[0]=1;
			tx_len=1;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
		else //Ҫ��ѯ��TTC�ڷ�Χ���򷵻��˵�����
		{
			tx_buffer[0]=0;
			fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), &tx_buffer[1], PCD_ZD_LEN);
			tx_len=1+PCD_ZD_LEN;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}	
	else if(PCD_CMD_ZDCHECK==command && msg_buffer[5]>=1 && msg_buffer[5]<=PCD_NOZZLE_MAX) //��ѯ�˵�����ѯ��ǹ�˵�
	{
		int base_ttc = (msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0);
		printf("pos_ttc = %d,physical_nozzle = %d,base_ttc = %d,handle = %d\n",pos_ttc,physical_nozzle,base_ttc,msg_buffer[10]);

		//��ѯָ����ǹ��TTC
		pos_ttc=pcdLocalPhsicalTTCGet(msg_buffer[5], (msg_buffer[6]<<24)|(msg_buffer[7]<<16)|(msg_buffer[8]<<8)|(msg_buffer[9]<<0), msg_buffer[10]);

		//Ҫ��ѯ��TTC�����Χ��û�д˱��˵�
		if(pos_ttc<1 || pos_ttc>pcdParam.TTC || (pcdParam.TTC>PCD_RECORD_MAX && pos_ttc<=(pcdParam.TTC-PCD_RECORD_MAX)))
		{
			tx_buffer[0]=1;
			tx_len=1;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
		else //Ҫ��ѯ��TTC�ڷ�Χ���򷵻��˵�����
		{
			tx_buffer[0]=0;
			fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, PCD_ZD_LEN*((pos_ttc-1)%PCD_RECORD_MAX), &tx_buffer[1], PCD_ZD_LEN);
			tx_len=1+PCD_ZD_LEN;
			pcd2IptSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, tx_buffer, tx_len);
		}
	}

	if(PCD_CMD_BARCODE == command) //IPT���̨ת���������������
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[7], (msg_buffer[5]<<8)|(msg_buffer[6]<<0));
	}

	if(PCD_CMD_FOR_TMAC==command) //IPT�������¼���TMAC���˵�
	{
		istate=pcdLocalBillSave(physical_nozzle, &msg_buffer[5], PCD_ZD_LEN);
	}

	if(PCD_CMD_ERRINFO_UPLOAD==command) //IPTͨ��PCD�ϴ��ڲ�������Ϣ����̨
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[7], (msg_buffer[5]<<8)|(msg_buffer[6]<<0));
	}

	if(PCD_CMD_DISCOUNT_ASK == command) //IPTͨ��PCD���̨�����ۿ۶�
	{
		pcdIpt2PcSend(msg_type, mboard_id, fuelling_point, physical_nozzle, command, &msg_buffer[5], 11);
	}


	return;
}


/********************************************************************
*Name				:pcd2PcProcess
*Description		:PCD��PC��ͨѶ����
*Input				:None
*Output			:None
*Return				:None
*History			:2014-03-25,modified by syj
*/
static void pcd2PcProcess()
{
	/*���ͻ���PC��ͨѶ
	*
	*	1.���ڷ�������յ�ͬ������
	*	���ͻ�Ϊ��������PCΪ�����������ͻ���һ��ʱ����ֻ����һ����������
	*	ֱ��PC���ض�Ӧ��Ϣ���޷��س�ʱ���������´����
	*	���������IPT�������Ĳ�ѯ��/��������Ҽ�¼������ֱ�����̨���ͣ�
	*
	*	2.����֡�ţ�ÿ�η���һ�μ��ͻ��Զ��ۼӣ���PC���ص�֡���뵱ǰ֡�Ų�һ��ʱ
	*	���ͻ�֡��ͬ����̨֡�ţ�֡���ڽ����ж�����ʱ����ʹ��
	*
	*	3.��ͨ��ѯ����
	*  ��һ��ʱ�������̨������ͨ��ѯ�����̨����ʱ��������ʵʱ����
	*
	*	4.��̨������������ʱ���ݾ������������
	*
	*	5.���ݽ�������������ݺ�"��Ч����"ͨ����Ϣ���͵���������
	*/
	unsigned char msg_buffer[PCD_MSGMAX]={0};		  //���յ���Ϣ����
	int msg_len=0;														    //���յ���Ϣ���ݳ���
	unsigned char tx_buffer[PCD_PCDATA_MAX]={0};	//�������ݻ���
	int tx_len=0;															    //�� �����ݳ���
	unsigned char read_buffer[256]={0};						//��ȡ������
	int read_len=0;														    //��ȡ�����ݳ���
	unsigned char tmp_buffer[128]={0};						//��ʱʹ�õĻ���
	unsigned int tmp_int1=0;											//��ʱʹ�õ���������
	PcdFuellingPointInfoType *fp_info=NULL;				//֧���ն���Ϣ
	RTCTime time;															    //ʱ����Ϣ
	unsigned int pos_ttc=0;											  //�˵�TTC
	int i=0, j=0;
	char is_bill_upload=0;												//�Ƿ����˵���Ҫ�ϴ�,0=�ޣ���0=��
	off_t download_offset=0;										  //���յ�����������ƫ��
	int download_bytes=0;											    //���յ����������ݳ���
	static char command=0;											  //��ǰ�������������
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
			����PC������������
			PCD2PC_CMD_SUMREAD:		PC��ȡ���ͻ��ۼ�������
			PCD2PC_CMD_INFOREAD:	PC��ȡ���ͻ���Ϣ����
			PCD2PC_CMD_ZDREAD:		PC��ȡ������������
	**********************************************************************/
	
	//PC��ȡ���ͻ��ۼ������������������Ч���߼���ǹ�ۼ�����
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
	//PC��ȡ���ͻ���Ϣ����
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
		timeRead(&time); //fj:�ڱ��.c�ļ���

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
	//PC��ȡ������������
	if(msg_len>0 && PCD2PC_CMD_ZDREAD==msg_buffer[0])
	{
		pos_ttc=(msg_buffer[1]<<24)|(msg_buffer[2]<<16)|(msg_buffer[3]<<8)|(msg_buffer[4]<<0);

		//��ȡָ���˵�����ȡTTC�Ϸ������ȡ�����˵�TTCһ���ϴ��˵��������Ӧ�޴��˵�
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

	//����ETC������������
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


	/*PCD�����ķ�������
	*	PCD�����������ݣ�ÿ��ֻ����һ�����ݷ��Ͳ��ȴ����ػ�ʱ��
	*	ע�⣬��ѯ�ڰ���������ѯ�ҽ��׵ȸ���������������ط��������ͣ�
	*	PCD��ʱ��PC������ѵ�������ѵ���غ��ж��Ƿ�������������Ҫִ�У�
	*	ִ�������������͵����ִ����Ϻ�Ҫ������ͨ��ѯ���
	*	ʹ��pcdParam.pcCommand�������ж������ѷ��͵����=0������У�
	*	ʹ��command�������жϵ�ǰ��Ҫ�����ڴ��������
	*/
	switch(command)
	{
	case PCD2PC_CMD_POLL: //��ͨ��ѯ����������ݵĴ���	
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_POLL;
			tx_len=1;
			//tx_buffer[0] = 0x55; //�㷽����
			//tx_buffer[1] = 0xaa;
			//tx_len = 2;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_POLL;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			//printf("xxxxxxxx\n");
			break;
		}

		//printf("zzzzzzzzzzzzzzzzz\n");
		
		if(PCD2PC_CMD_POLL==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_POLL==msg_buffer[0]) //�������ݵĴ���
		{
			//����ʵʱ��Ϣ
			//	���֧��PCD_NOZZLE_MAX��֧���նˣ�ÿ��֧���ն����֧�������߼���ǹ��
			//	���ݱ�֧���ն��Ƿ����ߣ������ߵ�֧���ն�֧�ֵ��߼���ǹ��Ϣ�ϴ���
			//	�߼���ǹ��Ϣ����״̬ѡ��(1=�����룻2=������)��Ҫ�ϴ������ݣ�
			//	��"������������"״̬֮һ���ϴ����ݣ�
			
		
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
			else if(tx_buffer[1]==2 && Roll_Flag==1)/*����*/
			{
				Roll_Flag=0;
				if(tx_buffer[2]==1)/*������*/
				{
					memcpy(rolldata,tx_buffer+2,19);
					memcpy(tx_buffer+2,tx_buffer+2+19,tx_len-2-19);
					memcpy(tx_buffer+tx_len-19,rolldata,19);
				}
				else if(tx_buffer[2]==2)/*������*/
				{
					memcpy(rolldata,tx_buffer+2,11);
					memcpy(tx_buffer+2,tx_buffer+2+11,tx_len-2-11);
					memcpy(tx_buffer+tx_len-11,rolldata,11);
				}
				memset(rolldata,0,sizeof(rolldata));
			}
			//tx_buffer[1] = 0; //fj:20170919,Ϊ�˲����޸�λ��ʵʱ��Ϣ��
			//tx_len = 2;
			//pcd2PcSend(tx_buffer,tx_len);

			//if(0==tx_buffer[1])	tx_len=1;		//ʵʱ��Ϣ����Ϊ0��ǹ��Ҳ��Ч��������,fj:20171120
			pcd2PcSend(tx_buffer, tx_len);	//����ʵʱ��Ϣ 
			memcpy(&pcdParam.PcTime, &msg_buffer[1], 7); //�����̨ʱ��

			//����״̬ΪPC���ߣ������ʱ����������
			pcdParam.PcOnline=PCD_PC_ONLINE;	
			pcdParam.pcOverTimes=0;

			//��ѯ�Ƿ���IPT��PCת���Ĳ�ѯ����
			//	��ǰ�����ڵȴ�ת��������ʱ��ȡ�Ƿ���δת�����ݣ�
			//	��ȡ���д�ת�����ݵ����ͷǷ�ʱ�������
			
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

			
			//�ж��Ƿ����˵���Ҫ�ϴ�
			//	�˵��Ĵ洢����TTCʱ����δ����T_MAC���˵���Ȼ���ٱ�������T_MAC���˵�
			//	��δ���˵�ʱ������δ���˵���ʶ��λ�������жϱ��˵��Ƿ��Ѽ���T_MAC
			//	�˵��Ѽ���T_MAC��δ����T_MAC���ѳ�ʱ2�������ϴ����˵���
			
			//if(ERROR == semTake(semIDPcdBill, 5*sysClkRateGet()))
			//{
				 //jljRunLog("�ж��Ƿ���δ�ϴ��˵�ʱ��ȡ�ź���[semIDPcdBill]ʧ��!\n"); //fj:�ú�����oilLog.c��
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
                jljRunLog("�ж��Ƿ���δ�ϴ��˵�ʱ��ȡ�ź���[semIDPcdBill]ʧ��!\n"); //fj:�ú�����oilLog.c��
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
			
			//semGive(semIDPcdBill); //fj:�Ȳ�����
			pthread_mutex_unlock(&mutexIDPcdBill);

			//����Ϣ�汾��PC��̨��һ��ʱ�ۼƲ�һ�´��������ۼƴﵽһ������ʱ���ظ���Ϣ����ֹ�д������ݵ���Ƶ������
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

			//����м��ͻ��ڲ�������ת����ͻ�������̣�
			//	����лҼ�¼��ѯ��ת��Ҽ�¼��ѯ���̣�
			//	����к�/��������ѯת���/��������ѯ���̣�
			//	������˵���Ҫ�ϴ���ת���˵��ϴ����̣�
			//	���������δ������ȫ��汾�Ų�һ����Ҫ������ת���������ع��̣�	
			
			if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_ERRINFO_UPLOAD==pcdParam.ipt2PcNode->command)
			{
				command=PCD2PC_CMD_ERRINFO;
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_BARCODE==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_BAR_CHECK; //��Ҫ���̨ת�������������
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_GREYINFO==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_GREYINFO; //��Ҫ��ѯ�Ҽ�¼��ת��Ҽ�¼��ѯ����
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_LIST==pcdParam.ipt2PcNode->command)
			{		
				command=PCD2PC_CMD_LISTINFO; //��Ҫ��ѯ��/��������ת���/��������ѯ����
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_DISCOUNT_ASK==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_DISCOUT_ASK; //��Ҫ���̨�����ۿ۶ת���ۿ۶��������
			}
			else if(NULL!=pcdParam.ipt2PcNode && PCD_CMD_APPLYFOR_DEBIT==pcdParam.ipt2PcNode->command)
			{	
				command=PCD2PC_CMD_APPLY_DEBIT; //��Ҫ���̨����ۿ�
			}
//szb_fj_20171120:ɾ��
#if 0
			else if(0!=is_bill_upload)
			{	
				//���˵���Ҫ�ϴ���ת���˵��ϴ�����
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
					//szb_fj_20171120:������ִ�е����ز�����ֱ��ת�����ع���
					command=PCD2PC_CMD_DOWNLOAD;
				}
				else if(pcdStationInfo.Version!=msg_buffer[14] && 0!=msg_buffer[14] && pcdParam.PcStaionInfoTimes>=5)
				{	
					//szb_fj_20171120:ת��������վͨ����Ϣ���ع���
					pcdParam.PcDownloadContent=PCD_DOWN_STATIONINFO;	
					pcdParam.PcDownloadLen=0;		
					pcdParam.PcOffsetLen=0;	
					command=PCD2PC_CMD_DOWNSTART;	
					pcdParam.PcStaionInfoTimes=0;
				}
				else if(pcdOilInfo.Version!=msg_buffer[13] && 0!=msg_buffer[13] && pcdParam.PcPriceInfoTimes>=5)
				{	
					//szb_fj_20171120:ת��������Ʒ�ͼ۱����ع���
					pcdParam.PcDownloadContent=PCD_DOWN_OILINFO;
					pcdParam.PcDownloadLen=0;	
					pcdParam.PcOffsetLen=0;	
					command=PCD2PC_CMD_DOWNSTART;	
					pcdParam.PcPriceInfoTimes=0;
				}
				else
				{
					//szb_fj_20171120:������ִ�е����ز�����ֱ��ת�����ع���
					command=PCD2PC_CMD_DOWNLOAD;
				}
			}
			else if(pcdStationInfo.Version!=msg_buffer[14] && 0!=msg_buffer[14] && pcdParam.PcStaionInfoTimes>=5)
			{	//ת��������վͨ����Ϣ���ع���
				pcdParam.PcDownloadContent=PCD_DOWN_STATIONINFO;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcStaionInfoTimes=0;
			}
			else if(pcdOilInfo.Version!=msg_buffer[13] && 0!=msg_buffer[13] && pcdParam.PcPriceInfoTimes>=5)
			{	//ת��������Ʒ�ͼ۱����ع���
				pcdParam.PcDownloadContent=PCD_DOWN_OILINFO;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcPriceInfoTimes=0;
			}
			else if(0!=memcmp(pcdBaseList.Version, &msg_buffer[8], 2) && 0!=memcmp("\x00\x00", &msg_buffer[8], 2) && pcdParam.PcBaseListTimes>=5)
			{	//ת�������������������ع���
				pcdParam.PcDownloadContent=PCD_DOWN_BASELIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcBaseListTimes=0;
			}
			else if(pcdAddList.Version[1]!=msg_buffer[10] && 0!=msg_buffer[10] && pcdParam.PcAddListTimes>=5)
			{	//ת�������������������ع���
				pcdParam.PcDownloadContent=PCD_DOWN_ADDLIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcAddListTimes=0;
			}
			else if(pcdDelList.Version[1]!=msg_buffer[11] && 0!=msg_buffer[11] && pcdParam.PcDelListTimes>=5)
			{	//ת��������ɾ���������ع���
				pcdParam.PcDownloadContent=PCD_DOWN_DELLIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcDelListTimes=0;
			}
			else if(pcdWhiteList.Version[1]!=msg_buffer[12] && 0!=msg_buffer[12] && pcdParam.PcWListTimes>=5)
			{	//ת���������������ع���	
				pcdParam.PcDownloadContent=PCD_DOWN_WHITELIST;		pcdParam.PcDownloadLen=0;		pcdParam.PcOffsetLen=0;	
				command=PCD2PC_CMD_DOWNSTART;	pcdParam.PcWListTimes=0;
			}

			if((0!=is_bill_upload)&&(command==PCD2PC_CMD_DOWNSTART || \
				command==PCD2PC_CMD_DOWNLOAD || command==PCD2PC_CMD_POLL))
			{	
				if(command!=PCD2PC_CMD_DOWNSTART)
				{
					if(pcdParam.UnloadFlag==1)//szb_fj_20171120:ִ�й����˵��ϴ�
					{
						pcdParam.UnloadFlag=0;//szb_fj_20171120:�����´���ѯ�����˵��ϴ�
					}
					else
					{
						//szb_fj_20171120:���˵���Ҫ�ϴ���ת���˵��ϴ�����
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

		//��ʱ����
		if(PCD2PC_CMD_POLL==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//���β�ѯ��ʱ����Ϊ�ͻ�����
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

	case PCD2PC_CMD_ZDUPLOAD: //���ͻ���PC�ϴ��������ݣ�PC���غ���ͻ�δ�������ݼ�
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_ZDUPLOAD;
			memcpy(&tx_buffer[1], pcdParam.UploadZD, 95);		
			tx_len=96;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_ZDUPLOAD;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ���
		if(PCD2PC_CMD_ZDUPLOAD==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_ZDUPLOAD==msg_buffer[0])
		{
			//if(ERROR == semTake(semIDPcdBill, 5*sysClkRateGet()))
			//{
				 //jljRunLog("�ϴ��˵��ɹ�ʱ��ȡ�ź���[semIDPcdBill]ʧ��!\n"); //fj:�ú�����oilLog.c�ﴦ��
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
                jljRunLog("�ж��Ƿ���δ�ϴ��˵�ʱ��ȡ�ź���[semIDPcdBill]ʧ��!\n"); //fj:�ú�����oilLog.c��
			}

			//δ�ϴ��˵��ݼ�
			pcdParam.UnloadNumber--;
			read_buffer[0]=(char)(pcdParam.UnloadNumber>>24);	read_buffer[1]=(char)(pcdParam.UnloadNumber>>16);
			read_buffer[2]=(char)(pcdParam.UnloadNumber>>8);		read_buffer[3]=(char)(pcdParam.UnloadNumber>>0);
			pcdFmWrite(PCD_FM_UNLOAD, read_buffer, 4);

			//��������ǹ�ţ���������ǹ�����˵��ݼ�
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

			//semGive(semIDPcdBill);  //fj:�Ȳ�����
			pthread_mutex_unlock(&mutexIDPcdBill);

			//�ϴ��˵���������
			memset(pcdParam.UploadZD, 0, PCD_ZD_LEN);
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_ZDUPLOAD==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DOWNSTART: //0x33:���ͻ���PC��������������
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND) //���ͻ�����������������
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

		//�������ݵĴ���δ�ϴ��˵��ݼ�
		if(PCD2PC_CMD_DOWNSTART==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DOWNSTART==msg_buffer[0])
		{

			//printf("test 1111\n");
			//��Ҫ���ص������ļ�����
			pcdParam.PcDownloadLen=(msg_buffer[1]<<24)|(msg_buffer[2]<<16)|(msg_buffer[3]<<8)|(msg_buffer[4]<<0);
			pcdFmWrite(PCD_FM_DLEN, &msg_buffer[1], 4); //21*4

			//printf("test 2222\n");

			//��������
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

			//�����س���
			pcdParam.PcOffsetLen=0;
			pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);

			//���ԭ�ļ����ݣ�֮�������Ϊ4����Ϊ���Ϊ0���в�������
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

		//��ʱ����
		if(PCD2PC_CMD_DOWNSTART==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.PcDownloadContent=0xff;	pcdParam.PcDownloadLen=0;	pcdParam.PcOffsetLen=0;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DOWNLOAD: 	//����PC����,0x34,�������ؾ�����������
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

		//�������ݵĴ���δ�ϴ��˵��ݼ�
		if(PCD2PC_CMD_DOWNLOAD==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DOWNLOAD==msg_buffer[0])
		{
			//������Ч���ݳ��ȼ������ص����ݳ��Ȳ�����
			download_bytes=msg_len-5;
			download_offset=((msg_buffer[2]<<8)|(msg_buffer[3]<<0))*16;
			if(PCD_DOWN_BASELIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		//����������
			}
			else if(PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		  //����������
			}
			else if(PCD_DOWN_DELLIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		   //��ɾ������
			}
			else if(PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		     //������
			}
			else if(PCD_DOWN_OILINFO==pcdParam.PcDownloadContent)
			{
				//PrintH(download_bytes,&msg_buffer[5]);
				fileWriteForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);			//��Ʒ�ͼ۱�
			}
			else if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent)
			{
				fileWriteForPath(PCD_FILE_STATIONINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, download_offset, &msg_buffer[5], download_bytes);		//��վͨ����Ϣ
			}

			//���������س��Ȳ�����
			pcdParam.PcOffsetLen+=download_bytes;
			tmp_buffer[0]=(char)(pcdParam.PcOffsetLen>>24);	tmp_buffer[1]=(char)(pcdParam.PcOffsetLen>>16);
			tmp_buffer[2]=(char)(pcdParam.PcOffsetLen>>8);		tmp_buffer[3]=(char)(pcdParam.PcOffsetLen>>0);
			pcdFmWrite(PCD_FM_DOFFSET, tmp_buffer, 4);

			//�ж�������ϣ��˳����ز����»�������Ӧ��Ϣ�������������������20000�ʣ��������ֲ�������
			
			if((download_bytes+download_offset)>=pcdParam.PcDownloadLen || 0==download_bytes)
			{
				if(PCD_DOWN_BASELIST==pcdParam.PcDownloadContent) //���»�����������Ϣ
				{
					fileReadForPath(PCD_FILE_BASELIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdBaseList.Version, &read_buffer[0], 2);
					memcpy(pcdBaseList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdBaseList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdBaseList.Area, &read_buffer[10], 2);
					memcpy(pcdBaseList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_ADDLIST==pcdParam.PcDownloadContent) //����������������Ϣ
				{
					fileReadForPath(PCD_FILE_ADDLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdAddList.Version, &read_buffer[0], 2);
					memcpy(pcdAddList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdAddList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdAddList.Area, &read_buffer[10], 2);
					memcpy(pcdAddList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_DELLIST==pcdParam.PcDownloadContent) //������ɾ��������Ϣ
				{
					fileReadForPath(PCD_FILE_DELLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdDelList.Version, &read_buffer[0], 2);				
					memcpy(pcdDelList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdDelList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdDelList.Area, &read_buffer[10], 2);
					memcpy(pcdDelList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_WHITELIST==pcdParam.PcDownloadContent) //���°�������Ϣ
				{
					fileReadForPath(PCD_FILE_WLIST, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 16);
					memcpy(pcdWhiteList.Version, &read_buffer[0], 2);
					memcpy(pcdWhiteList.TimeStart, &read_buffer[2], 4);
					memcpy(pcdWhiteList.TimeFinish, &read_buffer[6], 4);
					memcpy(pcdWhiteList.Area, &read_buffer[10], 2);
					memcpy(pcdWhiteList.Number, &read_buffer[12], 4);
				}
				
				if(PCD_DOWN_OILINFO==pcdParam.PcDownloadContent) //����Ʒ�ͼ���Ϣ�ļ�����ȡ��Ϣ
				{
					fileReadForPath(PCD_FILE_PRICEINFO, O_CREAT|O_RDWR , S_IREAD | S_IWRITE, 0, read_buffer, 8);
					memcpy(&pcdOilInfo.Version, &read_buffer[0], 1);
					memcpy(pcdOilInfo.ValidTime, &read_buffer[1], 6);
					memcpy(&pcdOilInfo.FieldNumber, &read_buffer[7], 1);
					download_offset=8;
					
					for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++) //��ǰ��Ʒ�ͼۼ�¼i
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
					
					for(i=0; (i<pcdOilInfo.FieldNumber)&&(i<6); i++) //����Ʒ�ͼۼ�¼i
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

					
					memset(tmp_buffer, 0, 128);	 //����Ʒ�ͼ۱�����������һ���ͼ۽����˵�����IPT����TMAC
					tmp_buffer[PCD_OFFSET_T_TYPE]=0x08;	 //��������
					timeRead(&time);			 //ʱ�䣬fj:��oilFRam.c��						
					tmp_buffer[PCD_OFFSET_TIME+0]=time.century;	tmp_buffer[PCD_OFFSET_TIME+1]=time.year;	
					tmp_buffer[PCD_OFFSET_TIME+2]=time.month;	tmp_buffer[PCD_OFFSET_TIME+3]=time.date;
					tmp_buffer[PCD_OFFSET_TIME+4]=time.hour;		tmp_buffer[PCD_OFFSET_TIME+5]=time.minute;
					tmp_buffer[PCD_OFFSET_TIME+6]=time.second;
					tmp_buffer[PCD_OFFSET_PHYGUN]=iptPhysicalNozzleRead(IPT_NOZZLE_1); //����ǹ��,fj:oilIpt.c�ﶨ��
					pcdLocalTTCGet(iptPhysicalNozzleRead(IPT_NOZZLE_1), tmp_buffer, 128, &tmp_int1); //TTC,fj:ͬ��
					tmp_buffer[PCD_OFFSET_TTC+0]=(char)(tmp_int1>>24);	tmp_buffer[PCD_OFFSET_TTC+1]=(char)(tmp_int1>>16);
					tmp_buffer[PCD_OFFSET_TTC+2]=(char)(tmp_int1>>8);		tmp_buffer[PCD_OFFSET_TTC+3]=(char)(tmp_int1>>0);
                    
					//fj:iptPhysicalNozzleRead��oilIpt.c�ﶨ�� 
					pcd2IptSend(PCD_MSGTYPE_IPT, pcdMboardIDRead(), 0, iptPhysicalNozzleRead(IPT_NOZZLE_1), PCD_CMD_FOR_TMAC, tmp_buffer, 128);
				}
				
				if(PCD_DOWN_STATIONINFO==pcdParam.PcDownloadContent) //������վͨ����Ϣ
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


				//��Ҫ���ص������ļ�����
				pcdParam.PcDownloadLen=0;
				pcdFmWrite(PCD_FM_DLEN, "\x00\x00\x00\x00", 4);
				//��������
				pcdParam.PcDownloadContent=0xff;
				pcdFmWrite(PCD_FM_DCONTENT, "\xff", 1);
				//�����س���
				pcdParam.PcOffsetLen=0;
				pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);
			}
	
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_DOWNLOAD==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;

			printf(" pcd timeout \n");
			break;
		}
		break;
	case PCD2PC_CMD_GREYINFO: //���ͻ���PC��ѯ�Ҽ�¼,0x35
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_GREYINFO;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_GREYINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ�������ȷ�ķ�������
		if(PCD2PC_CMD_GREYINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_GREYINFO==msg_buffer[0]) //��ѯ�ɹ���IPT���ز�ѯ���
		{
			tx_buffer[0]=0;													    //����Դ0=��̨��/������;1=���غ�/������
			memcpy(&tx_buffer[1], &msg_buffer[1], 42);	//�Ҽ�¼
			tx_len=43;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_GREYINFO, tx_buffer, tx_len);

			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	
			pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_GREYINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	
			pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	
			pcdParam.pcOvertimer=0;	
			command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_LISTINFO: //���ͻ���PC��ѯ��/��������Ϣ,0x36
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_LISTINFO;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_LISTINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		
		if(PCD2PC_CMD_LISTINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_LISTINFO==msg_buffer[0]) //�������ݵĴ�������ȷ�ķ�������
		{
			//��ѯ�ɹ���IPT���ز�ѯ���
			tx_buffer[0]=0;							 //���0=�ɹ�
			tx_buffer[1]=0;							//����Դ0=��̨��/������;1=���غ�/������
			tx_buffer[2]=msg_buffer[1];				//ƥ���ʶ
			memcpy(&tx_buffer[3], &msg_buffer[2], 10); //����
			tx_len=13;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_LIST, tx_buffer, tx_len);
			
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_LISTINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_ERRINFO: //���ͻ������ڲ�������Ϣ����,0x3B	
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_ERRINFO;	tx_buffer[1]=0;
			memcpy(&tx_buffer[2], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=2+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_ERRINFO;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ�������ȷ�ķ�������
		if(PCD2PC_CMD_ERRINFO==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_ERRACK==msg_buffer[0] && PCD2PC_CMD_ERRINFO==msg_buffer[1]){

			//��IPT���ؽ�������0=�ɹ�
			tx_buffer[0]=0;													
			tx_len=1;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_ERRINFO_UPLOAD, tx_buffer, tx_len);
			
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;

			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_ERRINFO==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{	
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_DISCOUT_ASK: //0x70//���̨�����ۿ۶����ϵͳ���ͻ����20160401 by SYJ		
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			tx_buffer[0]=PCD2PC_CMD_DISCOUT_ASK;
			memcpy(&tx_buffer[1], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=1+pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_DISCOUT_ASK;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ�������ȷ�ķ�������IPT���ؽ��
		if(PCD2PC_CMD_DISCOUT_ASK==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_DISCOUT_ASK==msg_buffer[0])
		{
			memcpy(tx_buffer, &msg_buffer[1], msg_len - 1);
			tx_len = msg_len - 1;
			pcd2IptSend(pcdParam.ipt2PcNode->msgType, pcdParam.ipt2PcNode->mboardId, pcdParam.ipt2PcNode->fuellingPoint, pcdParam.ipt2PcNode->phynozzle, PCD_CMD_DISCOUNT_ASK, tx_buffer, tx_len);

			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_DISCOUT_ASK==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{	
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;

	case PCD2PC_CMD_BAR_CHECK:
	case PCD2PC_CMD_BAR_RESULT:
	case PCD2PC_CMD_BAR_ACK:
	case PCD2PC_CMD_BAR_ACKDONE: //���̨ת���������ݣ�����ϵͳ���ͻ�������ص����ݻ�ֱ�ӱ���������ת����IPT��20160401 by SYJ
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			memcpy(&tx_buffer[0], pcdParam.ipt2PcNode->Buffer, pcdParam.ipt2PcNode->Nbytes);
			tx_len=pcdParam.ipt2PcNode->Nbytes;
			pcd2PcSend(tx_buffer, tx_len);
			pcdParam.pcCommand=PCD2PC_CMD_BAR_CHECK;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ������ص��������ڽ��յ�֮��ֱ�ӷ��͵�IPT
		if(PCD2PC_CMD_BAR_CHECK==pcdParam.pcCommand && msg_len>0 && \
			(PCD2PC_CMD_BAR_CHECK==msg_buffer[0]||PCD2PC_CMD_BAR_RESULT==msg_buffer[0]||PCD2PC_CMD_BAR_ACK==msg_buffer[0]||PCD2PC_CMD_BAR_ACKDONE==msg_buffer[0]))
		{
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_BAR_CHECK==pcdParam.pcCommand && pcdParam.pcOvertimer>=2*ONE_SECOND)
		{
			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL;
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}
		break;
	case PCD2PC_CMD_APPLY_DEBIT: //0x71 //���̨����ۿ����ݣ����ص�����ת����Դ��Ϣ���У�20160419 by SYJ		
		if(0==pcdParam.pcCommand && pcdParam.pcSendTimer>=500*ONE_MILLI_SECOND)
		{
			memcpy(tx_buffer , pcdParam.ipt2PcNode->Buffer + 13, 38);
			tx_len = 38;
			pcd2PcSend(tx_buffer, tx_len);

			pcdParam.pcCommand=PCD2PC_CMD_APPLY_DEBIT;	pcdParam.pcSendTimer=0;	pcdParam.pcOvertimer=0;
			break;
		}

		//�������ݵĴ���
		if(PCD2PC_CMD_APPLY_DEBIT==pcdParam.pcCommand && msg_len>0 && PCD2PC_CMD_APPLY_DEBIT==msg_buffer[0])
		{
			memcpy(tx_buffer, msg_buffer, msg_len);
			tx_len = msg_len;
			//fj: ��ע��
			//msgTx = (MSG_Q_ID)((pcdParam.ipt2PcNode->Buffer[1]<<24)|(pcdParam.ipt2PcNode->Buffer[2]<<16)|(pcdParam.ipt2PcNode->Buffer[3]<<8)|(pcdParam.ipt2PcNode->Buffer[4]<<0));
			//if(NULL != msgTx)	msgQSend(msgTx, tx_buffer, tx_len, NO_WAIT, MSG_PRI_NORMAL);

			//������͵Ľڵ�����
			free(pcdParam.ipt2PcNode);	pcdParam.ipt2PcNode=NULL; 
			pcdParam.pcCommand=0;	pcdParam.pcOvertimer=0;	command=PCD2PC_CMD_POLL;
			break;
		}

		//��ʱ����
		if(PCD2PC_CMD_APPLY_DEBIT==pcdParam.pcCommand && pcdParam.pcOvertimer>=10*ONE_SECOND)
		{
			//������͵Ľڵ�����
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
*Description		:���ܲ�����PC�ܿغ�̨���ص�����
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void tPcd2PcRx()
{
	prctl(PR_SET_NAME,(unsigned long)"tPcd2PcRx");
	unsigned char rx_buffer[PCD_PCDATA_MAX]={0};	//�洢��ȡ��������
	int rx_len=0;																	//�洢�����ݳ���
	unsigned char read_buffer[32]={0};						//��ȡ���Ĵ�������
	int read_len=0;																//��ȡ���Ĵ������ݳ���
	unsigned char step=0;													//���ݴ�����
	unsigned short crc=0;													//CRCУ��
	unsigned short data_len=0;										//��Ч���ݳ���
	int i=0;

	struct msg_struct msg_stPcdSend;
	msg_stPcdSend.msgType = 1;

	FOREVER
	{
		//printf("111111111\n");
		
		read_len = kjldRead(read_buffer, 32); //���ղ�����PC���������ܿ�����,fj,֮���ٸ�
		
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
			if(rx_len>=PCD_PCDATA_MAX) //�жϽ��ճ������
			{
				step=0;	rx_len=0;
			}

			rx_buffer[rx_len]=read_buffer[i]; //���ղ�����PC����
			switch(step)
			{
			case 0: //�������ݿ�ʼ�ֽ�		
				if(0xfa==rx_buffer[rx_len])	
					{	step=1;	rx_len=1;	}
				else										
					{	step=0;	rx_len=0;	}
				break;
			case 1: //��������Ŀ���ַ��Դ��ַ��֡��/���ơ���Ч���ݳ��ȣ�������0xFA��	
				if(0xfa==rx_buffer[rx_len])	
					{	step=0;	rx_len=0;	}
				else										
					{	rx_len++;}
		
				if(rx_len>=6) //�´ο�ʼ������Ч����
					{	step=2;}
				break;
			case 2: //����λ0xfaʱ������Ϊ���ֽ�Ϊת���ַ�ִ����һ���������򱣴���ֽ�����
				if(0xfa==rx_buffer[rx_len])	
					{	step=3;	}
				else										
					{	rx_len++;	}

				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
				data_len=((((int)rx_buffer[4])>>4)&0x0f)*1000+((((int)rx_buffer[4])>>0)&0x0f)*100+\
							((((int)rx_buffer[5])>>4)&0x0f)*10+((((int)rx_buffer[5])>>0)&0x0f)*1;

				//�ж���Ч���ݳ��ȺϷ���
				if(data_len+8>=PCD_PCDATA_MAX)
				{
					step=0;	rx_len=0;
				}

				//�ж��Ƿ�������
				if((rx_len>=8)&&(rx_len>=(data_len+8)))
				{
					crc=crc16Get(&rx_buffer[1], 5+data_len);
					if(crc==((rx_buffer[6+data_len]<<8)|(rx_buffer[7+data_len]<<0))) //szb_fj_20171120
					{
						//struct msg_struct msg_stPcdSend;
					    //msg_stPcdSend.msgType = 2;

						//�ж�������������վϵͳ�ͻ����������������ʱ���͸�֧��ģ��
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
						
						//msgQSend(pcdParam.msgIdFromPc, &rx_buffer[6], data_len, NO_WAIT, MSG_PRI_NORMAL); //fj:��Ҫ������ô��
						
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
			case 3://�����0xfa����Ϊת�屣�浱ǰ���ݣ��������0xfa����Ϊǰһ��0xfaΪ��ͷ		
				if(0xfa==rx_buffer[rx_len])	
					{	step=2;	rx_len++;	}
				else									
				{
					rx_buffer[0]=0xfa;	rx_buffer[1]=read_buffer[i];	step=1;	rx_len=2;
				}

				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
				data_len=((((int)rx_buffer[4])>>4)&0x0f)*1000+((((int)rx_buffer[4])>>0)&0x0f)*100+\
							((((int)rx_buffer[5])>>4)&0x0f)*10+((((int)rx_buffer[5])>>0)&0x0f)*1;

				//�ж���Ч���ݳ��ȺϷ���
				if(data_len+8>=PCD_PCDATA_MAX)
				{
					step=0;	rx_len=0;
				}

				//�ж��Ƿ�������
				if((rx_len>=8)&&(rx_len>=(data_len+8)))
				{
					crc=crc16Get(&rx_buffer[1], 5+data_len);
					if(crc==((rx_buffer[6+data_len]<<8)|(rx_buffer[7+data_len]<<0))) //szb_fj_20171120,update
					{
						struct msg_struct msg_stPcdSend;
						msg_stPcdSend.msgType = 2;
						//�ж�������������վϵͳ�ͻ����������������ʱ���͸�֧��ģ��
						if(MODEL_LIANDA == paramModelGet()&&\
							(PCD2PC_CMD_BAR_CHECK==rx_buffer[6] || PCD2PC_CMD_BAR_RESULT==rx_buffer[6] || PCD2PC_CMD_BAR_ACK==rx_buffer[6] || PCD2PC_CMD_BAR_ACKDONE==rx_buffer[6]))
						{
							 //msgQSend((MSG_Q_ID)iptMsgIdRead(0), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:ͬ��
							 //msgQSend((MSG_Q_ID)iptMsgIdRead(1), rx_buffer, rx_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:ͬ��
							memcpy(msg_stPcdSend.msgBuffer,rx_buffer,rx_len);
							msgsnd(iptMsgIdRead(0),&msg_stPcdSend,rx_len,IPC_NOWAIT);
                            msgsnd(iptMsgIdRead(1),&msg_stPcdSend,rx_len,IPC_NOWAIT);
							
							//printf("iiiiiiiiiiiiiiiii\n");
						}
						
						//msgQSend(pcdParam.msgIdFromPc, &rx_buffer[6], data_len, NO_WAIT, MSG_PRI_NORMAL);  //fj:ͬ��
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
*Description		:PCD���ݴ������������
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
		//PCD��IPT��ͨѶ����
        pcd2IptProcess();

		//PCD��PC��ͨѶ����,��ʯ��PC��̨��ͨѶ
		pcd2PcProcess();

		//taskDelay(1);
		usleep(1000);

        //printf("sssssssssss\n");
	}

	return;
}


/********************************************************************
*Name				:pcdWdgIsr
*Description		:PCDģ�鿴�Ź���ʱ���������
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void pcdWdgIsr()
{

//	pcdParam.pcSendTimer++;		//PCD��PC�������ݼ����ʱ��
//	pcdParam.pcOvertimer++;		//PCD��PC�������ݺ�ȴ����س�ʱ��ʱ��
//	pcdParam.UploadTimer++;		//PCD�������µ��˵�������T_MAC�ĳ�ʱ��ʱ��
	pcdParam.pcSendTimer+=10;		//PCD��PC�������ݼ����ʱ��
	pcdParam.pcOvertimer+=10;		//PCD��PC�������ݺ�ȴ����س�ʱ��ʱ��
	pcdParam.UploadTimer+=10;		//PCD�������µ��˵�������T_MAC�ĳ�ʱ��ʱ��
	//wdStart(pcdParam.wdgId, 1, (FUNCPTR)pcdWdgIsr, 0); //fj:vxworks�ĺ���������ٽ��

	return;
}


/********************************************************************
*Name				:pcdMsgIdGet
*Description		:��ȡPCD������Ϣ����ID
*Input				:None
*Output			:None
*Return				:None
*History			:2013-08-05,modified by syj
*/

int pcdMsgIdRead()   //fj:��ע��
{
	return pcdParam.msgIdFromIpt;
}


/********************************************************************
*Name				:pcdMboardIDwrite
*Description		:���ñ���PCD�����
*Input				:mboard_id	�����
*Output			:None
*Return				:0=�ɹ�������=ʧ��
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
*Description		:��ȡ����PCD�����
*Input				:None
*Output			:None
*Return				:����PCD�����
*History			:2013-07-01,modified by syj
*/
unsigned char pcdMboardIDRead(void)
{
	return pcdParam.mboardID;
}


/********************************************************************
*Name				:pcdApplyForTTC
*Description		:����TTC
*Input				:phynozzle	����ǹ��
*						:inbuffer		�˵�����(��TTC)
*						:nbytes			�˵����ݳ���(�ݹ̶�128)
*Output			:ttc				���뵽��TTC
*Return				:0=�ɹ���1=����ǹ�ŷǷ�������=����
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
*Description		:�����˵�
*Input				:phynozzle	����ǹ��
*						:inbuffer		�˵�����
*						:nbytes			�˵����ݳ���(�ݹ̶�128)
*Output			:None
*Return				:�ɹ�����0��ʧ�ܷ���������
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
*Description		:���̨�������ݲ��ȴ�����
*Input				:command	������
*						:inbuffer		��������(֡��(1Hex) + ������(2Bin) + ���ݳ���(2Hex) + ����(nbytes))
*						:timeout		��ʱʱ�䣬��λΪ��
*Output			:outbuffer		�������(֡��(1Hex) + ������(2Bin) + ���ݳ���(2Hex) + ����(nbytes))
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ�������ֵ��
*History			:2016-04-13,modified by syj
*/

//fj:�ȶ�ע����,ETC��Ȩ����,�Ȳ�����
int pcdPcSend(int command, char *inbuffer, int nbytes, char *outbuffer, int maxbytes, int timeout)
{
	/*char tx_buffer[PCD_MSGMAX + 1] = {0};
	int tx_len = 0;
	char rx_buffer[PCD_MSGMAX + 1] = {0};
	int rx_len = 0;
	MSG_Q_ID msgRx = NULL, msgTx = NULL;
	int istate = 0;
	int ilength = 0;

	//�ж��Ƿ�������״̬
	if(1 != pcdParam.PcOnline)
	{
		printf("[Function:%s][Line:%d]PCD����!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//�жϷ��͵���Ϣ����
	msgTx = pcdParam.msgIdRx;
	if(NULL == msgTx)
	{
		printf("[Function:%s][Line:%d]PCD������Ϣ���зǷ�!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//�������յ���Ϣ����
	msgRx = msgQCreate(1, PCD_MSGMAX + 1, MSG_Q_FIFO);
	if(NULL == msgRx)
	{
		printf("[Function:%s][Line:%d]�����洢�������ݵ���Ϣ����ʧ��!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//����(֡��(1Hex) + Դ��Ϣ���е�ַ(4bytes) + Ŀ����Ϣ���е�ַ(4bytes) + ������(2Bin) +  ���ݳ���(2Hex) + ����(nbytes))
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
		printf("[Function:%s][Line:%d]��������ʱʧ��!\n", __FUNCTION__, __LINE__);
		msgQDelete(msgRx);
		return ERROR;
	}

	//����(֡��(1Hex) + Դ��Ϣ���е�ַ(4bytes) + Ŀ����Ϣ���е�ַ(4bytes) + ������(2Bin) +  ���ݳ���(2Hex) + ����(nbytes))
	rx_len = msgQReceive(msgRx, rx_buffer, PCD_MSGMAX, timeout*sysClkRateGet());
	if(ERROR == rx_len)
	{
		printf("[Function:%s][Line:%d]��������ʱʧ��!\n", __FUNCTION__, __LINE__);
		msgQDelete(msgRx);
		return ERROR;
	}

	//�������
	memcpy(outbuffer, rx_buffer, rx_len);

	msgQDelete(msgRx);*/
	return 0;
}


/********************************************************************
*Name				:pcdApplyForAuthETCDebit
*Description		:���̨����ETC���ۿ�
*Input				:inbuffer		��������
*						:					������(1byte) + �߼�ǹ��(1byte) + ����ID(1byte) + 
*						:					��ǩMAC��(4bytes) + OBU��ͬ���(8bytes) + ���ƺ�(12bytes) +
*						:					�ۿ��(4bytes) + ʱ��(7bytes)
*Output			:outbuffer		�������
*						:					������(1byte) + �߼�ǹ��(1Byte) + ���(1byte)
*Return				:�ɹ�����0����ʱ����1��ʧ�ܷ�������ֵ��
*History			:2016-04-13,modified by syj
*/
int pcdApplyForAuthETCDebit(char *inbuffer, int nbytes, char *outbuffer, int maxbytes)
{
	return pcdPcSend(PCD_CMD_APPLYFOR_DEBIT, inbuffer, nbytes, outbuffer, maxbytes, 10);
	//return  0;    //fj:֮���ٽ��
}


/********************************************************************
*Name				:pcdOilRecordRead
*Description		:��ȡ��Ʒ�ͼۼ�¼������
*Input				:��
*Output			:oilinfo	��Ʒ�ͼ۱�����
*Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
*History			:2016-05-04,modified by syj
*/
int pcdOilRecordRead(PcdOilRecordType *oilinfo)
{
	if(NULL == oilinfo)
	{
		printf("%s:%d:��������ַ�Ƿ�!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//taskLock();  //fj:��ע��
	memcpy(oilinfo, &pcdOilInfo, sizeof(PcdOilRecordType));
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcdStationInfoRead
*Description		:��ȡ��վͨ����Ϣ
*Input				:��
*Output			:oilinfo	��Ʒ�ͼ۱�����
*Return				:�ɹ�����0��ʧ�ܷ�������ֵ��
*History			:2016-05-04,modified by syj
*/
int pcdStationInfoRead(PcdStationInfoType *stationinfo)
{
	if(NULL == stationinfo)
	{
		printf("%s:%d:��������ַ�Ƿ�!\n", __FUNCTION__, __LINE__);
		return ERROR;
	}

	//taskLock(); //fj:��ע��
	memcpy(stationinfo, &pcdStationInfo, sizeof(PcdStationInfoType));
	//taskUnlock();

	return 0;
}


/********************************************************************
*Name				:pcdParamInit
*Description		:PCD��ʼ��
*Input				:None
*Output			:None
*Return				:0=�ɹ�������=ʧ��
*History			:2014-03-25,modified by syj
*/
int pcdParamInit(void)
{
	unsigned char rdbuffer[16]={0};
	int i=0, istate=0, istate2=0, ireturn=0, rdbytes=0;

	//���������ʼ��,fj:��ע�ӣ���oilFRam.c��
	for(i=0; i<(FM_SIZE_PCD_SINO/8); i++)	
		framWrite(FM_ADDR_PCD_SINO, i*8, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
	for(i=0; i<(FM_SIZE_PCD_SINO/8); i++)
	{
		 ireturn=framRead(FM_ADDR_PCD_SINO, i*8, rdbuffer, 8); //fj:��oilFRam.c��
		if((0!=ireturn)||(0!=memcmp(rdbuffer, "\x00\x00\x00\x00\x00\x00\x00\x00", 8)))	istate=1;
	}

	//��������дΪ������
	pcdParam.PcDownloadContent=0xff;	
	pcdFmWrite(PCD_FM_DCONTENT, "\xff", 1);  

	//�����:1
	if(0!=paramSetupWrite(PRM_MBOARD_ID, "\x01", 1))		istate=1;

	//�����س���
	pcdParam.PcOffsetLen=0;
	pcdFmWrite(PCD_FM_DOFFSET, "\x00\x00\x00\x00", 4);

	//����������ݽṹ
	memset(&pcdBaseList, 0, sizeof(PcdListInfoType));				//����������
	memset(&pcdAddList, 0, sizeof(PcdListInfoType));				//����������
	memset(&pcdDelList, 0, sizeof(PcdListInfoType));				//��ɾ������
	memset(&pcdWhiteList, 0, sizeof(PcdListInfoType));			//������
	memset(&pcdOilInfo, 0, sizeof(PcdOilRecordType));		  	//��Ʒ�ͼ۱�
	memset(&pcdStationInfo, 0, sizeof(PcdStationInfoType));	//ͨ����Ϣ
	memset(&pcdParam, 0, sizeof(PcdParamStructType));		    //PCD����������ݽṹ
 
	//�ļ���գ����ȡ�ļ�Ϊ0ʱ�����⣬����ֻ�ý�ȡΪ4������ʼ��Ϊ0
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
*Description		:PCDģ�鹦�ܳ�ʼ��
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
     	jljUserLog("PCD��ʼ�������˵������ź���[semIDPcdBill]ʧ��!\n");
	 }

/**********************************************************
*	PCD���������Ϣ
***********************************************************/

	//��ȡ����ţ���ȡʧ�ܻ�����ŷǷ���Ĭ��Ϊ1������
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

	//��ʼ�������뿪��������̨��ͨѶ�ӿڹ���
	kjldInit();  //fj:����������oilKJLD.c��


/**********************************************************
*	��̨���������Ϣ
***********************************************************/
	//�������̨PC���ص���Ϣ����
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DCONTENT, read_buffer, 1); //fj:��oilFRam.c��
	if(0==istate)
	{
		pcdParam.PcDownloadContent=read_buffer[0];
	}
	else
	{
		printf("Error!	Read the param 'PCD_FM_DCONTENT' failed!\n");
	}

	//�������̨PC���ص���Ϣ�����ܳ���
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DLEN, read_buffer, 4);   //fj:ͬ��
	if(0==istate)
	{
		pcdParam.PcDownloadLen=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	}
	else	
	{
		printf("Error!	Read the param 'PCD_FM_DLEN' failed!\n");
	}

	//�������̨PC���ص���Ϣ���������س���
	istate=framRead(FM_ADDR_PCD_SINO, PCD_FM_DOFFSET, read_buffer, 4);  //fj:ͬ��
	if(0==istate)
	{
		pcdParam.PcOffsetLen=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	}
	else	
	{	
		printf("Error!	Read the param 'PCD_FM_DOFFSET' failed!\n");
	}


	//�򿪻����������ļ�����ȡ��Ϣ
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

	//�������������ļ�����ȡ��Ϣ
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

	//����ɾ�������ļ�����ȡ��Ϣ
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

	//�򿪰������ļ�����ȡ��Ϣ
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

	//����Ʒ�ͼ���Ϣ�ļ�����ȡ��Ϣ
	istate=fileReadForPath(PCD_FILE_PRICEINFO, O_RDWR|O_CREAT , S_IREAD | S_IWRITE, 0, read_buffer, 8);
	if(ERROR!=istate)
	{
		memcpy(&pcdOilInfo.Version, &read_buffer[0], 1);
		memcpy(pcdOilInfo.ValidTime, &read_buffer[1], 6);
		memcpy(&pcdOilInfo.FieldNumber, &read_buffer[7], 1);
		offset=8;
		//��ǰ��Ʒ�ͼۼ�¼i
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
		//����Ʒ�ͼۼ�¼i
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

	//����վͨ����Ϣ�ļ�����ȡ��Ϣ
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
*	�˵���ȡ�����Ϣ
***********************************************************/
	//����1��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC1, read_buffer, 4);      //fj:��oilFRam.c��
	pcdParam.TTC1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	printf("pcdParam.TTC1 = %d\n",pcdParam.TTC1);
	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_TTC1  ",read_buffer,4);

	//����1��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM1, read_buffer, 4);    //fj:��oilFRam.c��
	pcdParam.ZDNumber1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	
	printf("pcdParam.ZDNumber1 = %d\n",pcdParam.ZDNumber1);
	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_ZDNUM1  ",read_buffer,4);

   
	//pcdFmWrite(PCD_FM_ZDNUM1,"\x00\x00\x01\x02",4);
	//unsigned char buff[8];
	//pcdFmRead(PCD_FM_ZDNUM1,buff,8);
	//PrintH(4,buff);


	//����1��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD1, read_buffer, 4);    //fj:��oilFRam.c��
	pcdParam.UnloadNumber1=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//����2��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC2, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.TTC2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	g_fjLog.WriteLog("pcdInit  ","pcdFmRead_PCD_FM_TTC2  ",read_buffer,4);

	//����2��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM2, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.ZDNumber2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����2��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD2, read_buffer, 4);      //fj:��oilFRam.c��
	pcdParam.UnloadNumber2=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//����3��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC3, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.TTC3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����3��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM3, read_buffer, 4);     //fj:��oilFRam.c��
	pcdParam.ZDNumber3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����3��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD3, read_buffer, 4);  //fj:��oilFRam.c��
	pcdParam.UnloadNumber3=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//����4��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC4, read_buffer, 4);     //fj:��oilFRam.c��
	pcdParam.TTC4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����4��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM4, read_buffer, 4);   //fj:��oilFRam.c��
	pcdParam.ZDNumber4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����4��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD4, read_buffer, 4);     //fj:��oilFRam.c��
	pcdParam.UnloadNumber4=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//����5��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC5, read_buffer, 4);      //fj:��oilFRam.c��
	pcdParam.TTC5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����5��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM5, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.ZDNumber5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����5��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD5, read_buffer, 4);     //fj:��oilFRam.c��
	pcdParam.UnloadNumber5=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//����6��ǹ�˵�TTC
	pcdFmRead(PCD_FM_TTC6, read_buffer, 4);         //fj:��oilFRam.c��
	pcdParam.TTC6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����6��ǹ�˵���Ŀ
	pcdFmRead(PCD_FM_ZDNUM6, read_buffer, 4);        //fj:��oilFRam.c��
	pcdParam.ZDNumber6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);
	//����6��ǹδ���˵���Ŀ
	pcdFmRead(PCD_FM_UNLOAD6, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.UnloadNumber6=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//�ӿ��˵���Ŀ
	pcdFmRead(PCD_FM_YCZDNUM, read_buffer, 4);       //fj:��oilFRam.c��
	pcdParam.AbnormalNumber=(read_buffer[0]<<24)|(read_buffer[1]<<16)|(read_buffer[2]<<8)|(read_buffer[3]<<0);

	//pcdFmWrite(PCD_FM_UNLOAD,"\x00\x00\x00\x00",4);
	//printf("start : %02x,%02x,%02x,%02x,",read_buffer[0],read_buffer[1],read_buffer[2],read_buffer[3]);

	//�����˵�����
	istate=pcdFmRead(PCD_FM_UNLOAD, read_buffer, 4);   //fj:��oilFRam.c��
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


	//��ǰTTC
	istate=pcdFmRead(PCD_FM_TTC, read_buffer, 4);       //fj:��oilFRam.c��
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
	//���˵�����
	pcdParam.FdRecord=fileReadForPath(PCD_FILE_OILRECORD, O_CREAT|O_RDWR , S_IREAD | S_IWRITE);
	if(ERROR==pcdParam.FdRecord)	printf("Error!	Open the file %s failed!\n", PCD_FILE_OILRECORD);
	//���쳣�˵����������ļ�
	pcdParam.FdZDUnnormal=fileOpen(PCD_FILE_ZD_UNNORMAL, O_CREAT|O_RDWR , S_IREAD | S_IWRITE);
	if(ERROR==pcdParam.FdZDUnnormal)	printf("Error!	Open the file %s failed!\n", PCD_FILE_ZD_UNNORMAL);

#endif



/**********************************************************
*	��Ϣ������Ȼ���������س�ʼ��
***********************************************************/
	//PCD��PC��ͨѶ����

	pcdParam.comFdPc=COM16;

	//��ʼ��IPT��PC��ѯ���ݵ����ݽڵ�Ϊ��
	pcdParam.ipt2PcNode=NULL;

	//������������IPT������Ҫת����PCʱ�洢�ڴ�����
	lstInit(&pcdParam.ipt2PcList);

	//���Ź���ʱ����ʱ����50 ticks,fj:���Ź��Ȳ����ϡ�
	//pcdParam.wdgId=wdCreate();
	//if(NULL==pcdParam.wdgId)	printf("Error! Create watch dog timer 'PcdWdgId' failed!\n");
	//else										wdStart(pcdParam.wdgId, 1, (FUNCPTR)pcdWdgIsr, 0);
	
	pcdParam.comFdPrint1=COM13;  //������1�Ŵ�ӡ�����Ӵ���
	pcdParam.comFdPrint2=COM17;  //������2�Ŵ�ӡ�����Ӵ���

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


	//����PCD������Ϣ����Ϣ����
	//pcdParam.msgIdRx = msgQCreate(10, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdRx)		printf("Error!	Creat messages  'pcdParam.msgIdRx' failed!\n");

	//����PCD����IPT������Ϣ����
	//pcdParam.msgIdFromIpt=msgQCreate(PCD_MSGNB, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdFromIpt)		printf("Error!	Creat messages  'pcdParam.msgIdFromIpt' failed!\n");

	//����PCD����PC������Ϣ����
	//pcdParam.msgIdFromPc=msgQCreate(PCD_MSGNB, PCD_MSGMAX, MSG_Q_FIFO);
	//if(NULL==pcdParam.msgIdFromPc)		printf("Error!	Creat messages  'pcdParam.msgIdFromPc' failed!\n");
	
	
	
	
	//PCDģ�����PC�ܿط������������ʼ��
	//pcdParam.tIdPcReceive=taskSpawn("tPcd2PcRx", 153, 0, 0x1000, (FUNCPTR)tPcd2PcRx, 0,1,2,3,4,5,6,7,8,9);
	//if(!(OK==taskIdVerify(pcdParam.tIdPcReceive)))		printf("Error!	Creat task 'tIdPcReceive' failed!\n");

	//����PCD��������
	//pcdParam.tIdProcess=taskSpawn("tPcd", 153, 0, 0x8000, (FUNCPTR)tPcdProcess, 0,1,2,3,4,5,6,7,8,9);
	//if(!(OK==taskIdVerify(pcdParam.tIdProcess)))		printf("Error!	Creat task 'tPcd' failed!\n");   


	return true;
}


/********************************************************************
*Name				:pcdExit
*Description		:PCDģ�鹦��ע��
*Input				:None
*Output			:None
*Return				:None
*History			:2013-07-01,modified by syj
*/
void pcdExit(void)
{
	return;
}


