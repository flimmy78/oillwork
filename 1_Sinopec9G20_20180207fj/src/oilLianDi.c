// #include <VxWorks.h>
// #include <lstLib.h>
// #include "oilCfg.h"
// #include "oilKb.h"
// #include "oilCom.h"
// #include "oilLog.h"
// #include "oilLianDi.h"


#include "../inc/main.h"

 LDStructParamType ldStructA1, ldStructB1;
 
// //�ӿ�����
 void tLdRecv(int DEV_DSP_KEYx);
 int ldRecvPars(int DEV_DSP_KEYx, char *inbuffer, int nbytes);
 int ldSend(int DEV_DSP_KEYx, char *inbuffer, int nbtes, char *outbuffer, int maxbytes, int seconds);
 void tLdPoll(int DEV_DSP_KEYx);

 
// /*****************************************************************************
// *Name				:tLdPoll
// *Description		:������ѯ����POS����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *Output			:��
// *Return			:��
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
 
 	//�ж��豸���ӵĴ���
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;	devx = DEV_DSP_KEYA;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;	devx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return;
 	}
 
 	FOREVER
 	{
 		//��ѯ����״̬��Ϣ
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
// *Description		:�������ն��豸ͨѶ���ݽ��ռ���������
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *Output			:��
// *Return			:��
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
 
 	//�ж��豸���ӵĴ���
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;	devx = DEV_DSP_KEYA;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;	devx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return;
 	}
 
 	//���ݽ��ռ�����
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
 				//��������ͷ
 				if(0xfa == recvbuffer[recvlenght])	{	step = 1;	recvlenght = 1;	}
 				else													{	step = 0;	recvlenght = 0;	}
 				break;
 				
 			case 1:
 				//��������Ŀ���ַ��Դ��ַ��֡��/���ơ���Ч���ݳ��ȣ�������0xFA��
 				if(0xfa == recvbuffer[recvlenght])	{	step = 0;	recvlenght = 0;	}
 				else													{	recvlenght++;}
 
 				if(recvlenght >=6 )							{	step = 2;}
 				break;
 				
 			case 2:
 				//����λ0xfaʱ������Ϊ���ֽ�Ϊת���ַ�ִ����һ���������򱣴���ֽ�����
 				if(0xfa == recvbuffer[recvlenght])	{	step=3;	}
 				else													{	recvlenght++;	}
 
 				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
 				data_len = ((((int)recvbuffer[4])>>4)&0x0f)*1000 + ((((int)recvbuffer[4])>>0)&0x0f)*100+\
 							((((int)recvbuffer[5])>>4)&0x0f)*10 + ((((int)recvbuffer[5])>>0)&0x0f)*1;
 
 				//�ж���Ч���ݳ��ȺϷ���
 				if(data_len + 8 >= LD_BUFFER_SIZE)
 				{
 					step = 0;	recvlenght = 0;
 				}
 
 				//�ж��Ƿ�������
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
 				//�����0xfa����Ϊת�屣�浱ǰ���ݣ��������0xfa����Ϊǰһ��0xfaΪ��ͷ
 				if(0xfa == recvbuffer[recvlenght])	{	step = 2;	recvlenght++;	}
 				else									
 				{
 					recvbuffer[0] = 0xfa;	recvbuffer[1] = rdbuffer[i];	step = 1;	recvlenght = 2;
 				}
 
 				//�жϽ��ս��������Ȳ��ܹ����ֹ��������ȺϷ����ж�CRC
 				data_len = ((((int)recvbuffer[4])>>4)&0x0f)*1000 + ((((int)recvbuffer[4])>>0)&0x0f)*100+\
 							((((int)recvbuffer[5])>>4)&0x0f)*10 + ((((int)recvbuffer[5])>>0)&0x0f)*1;
 
 				//�ж���Ч���ݳ��ȺϷ���
 				if(data_len + 8 >= LD_BUFFER_SIZE)
 				{
 					step = 0;	recvlenght = 0;
 				}
 
 				//�ж��Ƿ�������
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
// *Description		:�ն����ݽ��ս���
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:inbuffer		��Ч����
// *						:nbtes			��Ч���ݳ���
// *						:seconds		�ȴ���ʱʱ�䣬��λ�룬0��ʾ���ȴ�����
// *Output			:outbuffer	��Ч���ݻ���
// *						:maxbytes	��Ч���ݻ����С
// *Return			:�ɹ��������ݳ��ȣ�ʧ�ܷ���-1��
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
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return -1;
 	}
 
 	if(nbytes > LD_BUFFER_SIZE)
 	{
 		jljRunLog("���ݳ���Խ��!\n");
 		return -1;
 	}
 
// 	/*���������ָ�λ�ж�������������ķ��ػ����ն�ģ���������͵�����
// 	*	��������ķ��������ݿ������������ݻ���
// 	*	�ն�ģ���������������ݿ������������еȴ�����
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
 
 		/*�ڵ�������10����ɾ����һ��*/
 		if(lstCount(&(ldstruct->ButtonList)) >= 10)
 		{
 			pnode = (LDButtonNodeStruct*)lstGet(&(ldstruct->ButtonList));
 			if(NULL != pnode)
 			{
 				free(pnode);
 			}
 		}
 
 		/*����һ���ڵ㲢��ӵ�β��*/
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
// *Description		:�ն����ݷ���
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:inbuffer		��Ч����
// *						:nbtes			��Ч���ݳ���
// *						:seconds		�ȴ���ʱʱ�䣬��λ�룬0��ʾ���ȴ�����
// *Output			:outbuffer	��Ч���ݻ���
// *						:maxbytes	��Ч���ݻ����С
// *Return			:�ɹ��������ݳ��ȣ�ʧ�ܷ���-1��
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
 
 	//�ж��豸
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return -1;
 	}
 
 	//�ж���Ч���ݳ���
 	if(8 + nbytes >= 1024)
 	{
 		jljRunLog("���͵����ݳ��ȹ���[nbytes = %x].\n", nbytes);
 		return -1;
 	}
	 
 	//��֯���ݷ���
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
 
 	//���ת���ַ�
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
 
 	//��������
 	ldstruct->AckLenght = 0;
 	comWriteInTime(ldstruct->ComX, tx_buffer, tx_len, 1);
 
 	//���ȴ�����
 	if(0 == seconds)
 	{
 		return 0;
 	}
 
 	//�ȴ��������ݣ��ж�֡��һ��ʱΪ�����ݷ���
 	while(1)
 	{
 		if(ldstruct->AckLenght > 0 && tmp_frame == ldstruct->AckBuffer[3])
 		{
 			tmp_data = ((((int)ldstruct->AckBuffer[4])>>4)&0x0f)*1000 + ((((int)ldstruct->AckBuffer[4])>>0)&0x0f)*100+\
 							((((int)ldstruct->AckBuffer[5])>>4)&0x0f)*10 + ((((int)ldstruct->AckBuffer[5])>>0)&0x0f)*1;
 
 			if(tmp_data > maxbytes)
 			{
 				jljRunLog("���յ������ݳ��ȹ���!\n");	
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
// *Description		:������ʾ,ÿ��������ʾ8�����֣������ʾ����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:FONTx					�ֿ�����ѡ��	=1		16*8,16*16�ֿ� 
//        																					=2    	14*7,14*14�ֿ� 
// 	   																						=3   	8*8�ֿ�
// 	   																						=4    	12*6,12*12�ֿ�
// *						:IsContrary			�Ƿ���0=������ʾ��1=����
// *						:Offsetx				������0~7
// *						:Offsety				������0~127
// *						:buffer					��ʾ����
// *						:nbytes				��ʾ���ݳ���
// *Output			:None
// *Return			:0=�ɹ�������=����
// *History			:2016-09-08,modified by syj
// */
int ldDspContent(int DEV_DSP_KEYx, unsigned char FONTx, unsigned char IsContrary, unsigned char Offsetx, unsigned char Offsety, unsigned char *buffer, int nbytes)
{
 	LDStructParamType *ldstruct = NULL;
 	unsigned char poffset_x = 0;
 	int i = 0, j = 0;
 
 	//�ж��豸���ӵĴ���
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return -1;
 	}
 
 	/*�ж�������*/
 	if(Offsetx > 7)
 	{
 		return -1;
 	}
 
// 	/*����ʾ������ӵ�����
// 	*
// 	*	ע������˵��!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 	*
// 	*	Ϊ�˼��ݽ����������Դ˴�������һ��
// 	*	����ģ����ʾ�кŷ�ΧΪ1~4���˴������⴦��
// 	*	����ģ��12*12�ֿ�ÿ����ʾʮ���ַ����˴���ÿ��ǰ��2���ո��Զ������
// 	*	������������겻Ϊ0������ǰ�������ո���
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
// *Description		:�ն���ʾ��Ӧ���ȵ���lbDspContent�����ʾ���ݺ���ñ�����
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:Contrast				�Աȶ�HEX��ʽ
// *						:IsClr					�Ƿ����ȫ��0=����գ�1=���
// *Output			:None
// *Return			:0=�ɹ�������=ʧ��
// *History			:2016-09-08,modified by syj
// */
int ldDsp(int DEV_DSP_KEYx, int Contrast, int IsClr)
{
 	LDStructParamType *ldstruct = NULL;
 	char tx_buffer[128] = {0};
 	int tx_len = 0;
 	char outbuffer[64] = {0};
 	int i = 0, plen = 0;
 
 	//�ж��豸���ӵĴ���
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return -1;
 	}
 
 	/*��ʾ������֡*/
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
 
// 	/*�������*/
 	ldstruct->Content.BitMap = 0;
 	for(i=0; i<8; i++)
 	{
 		ldstruct->Content.Font[i] = 0;
 		ldstruct->Content.IsContrary[i] = 0;
 		ldstruct->Content.OffsetY[i] = 0;
 		ldstruct->Content.Nbytes[i] = 0;
 	}
 
// 	/*������ʾ���ݣ����ȴ�����*/
 	ldSend(DEV_DSP_KEYx, tx_buffer, tx_len, outbuffer, 64, 0);
 
 	return 0;
}
 
 
 
// /********************************************************************
// *Name				:ldButtonRead
// *Description		:���̰�����ȡ
// *Input				:kb_dev_id	�豸ѡ��	DEV_BUTTON_KEYA=A1����	DEV_BUTTON_KEYB=B1����
// *Output			:None
// *Return			:����ֵ���ް���ʱ����KB_BUTTON_NO
// *History			:2016-09-08,modified by syj
// */
unsigned int ldButtonRead(int kb_dev_id) 
{
 	unsigned int button=KB_BUTTON_NO, value=0;
 	LDButtonNodeStruct *pnode=NULL;
 
 	//�жϰ����豸
 	if(DEV_BUTTON_KEYA == kb_dev_id)			pnode = (LDButtonNodeStruct*)lstGet(&ldStructA1.ButtonList);
 	else if(DEV_BUTTON_KEYB == kb_dev_id)	pnode = (LDButtonNodeStruct*)lstGet(&ldStructB1.ButtonList);
 	else																	return KB_BUTTON_NO;
 
    //�ж��Ƿ��а���
 	if(NULL != pnode)
 	{
 		value = pnode->Value;
 		free(pnode);	
	}
 	else
 	{
 		return KB_BUTTON_NO;
 	}
 
 	//��������
 	switch(value)
 	{
 	case 0xffffffff:		//�ް���ֵ
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
 	case 0x00000045:		// ����/.
 		button=KB_BUTTON_CZ;
 		break;
 	case 0x00000061:		// ����
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
 	case 0x00000007:		// ��
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
 	case 0x0000000D:		// ȷ��
 		button=KB_BUTTON_ACK;
 		break;
 	case 0x00000008:		// ��
 		button=KB_BUTTON_DOWN;
 		break;
 	case 0x00000043:		// �����
 		button=KB_BUTTON_MON;
 		break;
 	case 0x00000044:		// ������
 		button=KB_BUTTON_VOL;
 		break;
 	case 0x0000001B:		// �˿�
 		button=KB_BUTTON_BACK; 
 		break;
 	case 0x00000065:		// ����
 		button=KB_BUTTON_CHG;
 		break;
 	case 0x00000046:		// ѡ��
 		button=KB_BUTTON_SEL;
 		break;
 	case 0x00008031:		// ����+1
 		button=KB_BUTTON_SET1;
 		break;
 	case 0x00008032:		// ����+2
 		button=KB_BUTTON_SET2;
 		break;
 	case 0x00008033:		// ����+3
 		button=KB_BUTTON_SET3;
 		break;
 	case 0x00008034:		// ����+4
 		button=KB_BUTTON_SET4;
 		break;
 	case 0x00008035:		// ����+5
 		button=KB_BUTTON_SET5;
 		break;
 	case 0x00008036:		// ����+6
 		button=KB_BUTTON_SET6;
 		break;
 	case 0x00008037:		// ����+7
 		button=KB_BUTTON_SET7;
 		break;
 	case 0x00008038:		// ����+8
 		button=KB_BUTTON_SET8;
 		break;
 	case 0x00008039:		// ����+9
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
// *Description		:����IC���ϵ�
// *Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
// *						:dev_x			�����豸��0x00=����; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *Output			:��
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
// *History			:2016-09-08,modified by syj
// */
int ldICPowerUp(int nozzle, unsigned char dev_x)
{
 	unsigned char tx_buffer[64] = {0};
 	int tx_len = 0;
 	unsigned char rx_buffer[64] = {0};
 	int istate = 0;
 	int dev_dsp_keyx = 0;
 
 	//�ж��豸���ӵĴ���
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
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
// *Description		:����IC���µ�
// *Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
// *						:dev_x			�����豸��0x00=����; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *Output			:��
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
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
// 	//�ж��豸���ӵĴ���
// 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
// 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
// 	else
// 	{
// 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
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
// *Description		:����IC��APDU����ͨ��
// *Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
// *						:dev_x			�����豸��0x00=����; 0x01=SAM1; 0x02=SAM2; 0x03=SAM3;
// *						:inbuffer		���뻺��(APDU����+APDU����)
// *						:nbytes		���뻺�����ݳ���
// *						:maxbytes	��������С
// *Output			:outbuffer	�������(APDU����+APDU����)
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
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
 
 	//�ж��豸���ӵĴ���
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
 		return -2;
 	}
 
 	//�жϳ���
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
// *Description		:�˿�
// *Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
// *Output			:None
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
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
 
 	//�ж��豸���ӵĴ���
 	if(0 == nozzle)			{dev_dsp_keyx = DEV_DSP_KEYA;}
 	else if(1 == nozzle)	{dev_dsp_keyx = DEV_DSP_KEYB;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
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
// *Description		:��ȡ����״̬��Ϣ�ӿ�
// *Input				:nozzle			ǹѡ0=A1ǹ��1=B1ǹ
// *Output			:outbuffer	S1����״̬(1byte) + S2������(1byte) + S3��״̬(1byte)
// 											//S1����״̬=0x30��ʾ�п�,0x31��ʾ�޿���
// 											//S2:������
// 											//S2=0x30 �������޿�
// 											//S2=0x3f �޷�ʶ��
// 											//S2=0x31 ����cpu��
// 											//S2=0x32 RF-- TYPE B CPU��
// 											//S2=0x33 RF-TYPE A CPU��
// 											//S2=0x34 RF-M1��
// 											//S3����״̬
// 											//S3=0x30 �µ�״̬
// 											//S3=0x31 ����״̬
// 											//S4=0x32 ����״̬
// 											//S5=0x33 æ̬��ͨ��״̬��
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ
// *History			:2016-09-08,modified by syj
// */
// int ldICStateRead(int nozzle, char *outbuffer)
// {
// 	LDStructParamType *ldstruct = NULL;
// 
// 	//�ж��豸���ӵĴ���
// 	if(0 == nozzle)			{ldstruct = &ldStructA1;}
// 	else if(1 == nozzle)	{ldstruct = &ldStructB1;}
// 	else
// 	{
// 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
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
// *Description		:���н��׽ӿ�
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *						:pSend			���͵�DATA������
// *						:nSendLen	���͵�DATA�����򳤶�
// *Output			:pRec			���ص�DATA������
// *						:pnRecLen	���ص�DATA�����򳤶�
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ
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
 
 	//�ж��豸���ӵĴ���
 	if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
 	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
 	else
 	{
 		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
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
// *Description		:�������ն��豸ͨѶ���ܳ�ʼ��
// *Input				:DEV_DSP_KEYx	�豸ѡ��	DEV_DSP_KEYA=A1����	DEV_DSP_KEYB=B1����
// *Output			:��
// *Return			:�ɹ�����0��ʧ�ܷ�������ֵ��
// *History			:2016-08-29,modified by syj
// */

int ldInit(int DEV_DSP_KEYx)
{
	int tid = 0;
	char tmp_buffer[64] = {0};
	
	//LDStructParamType *ldstruct  = NULL;

	//�ж��豸���ӵĴ���
	/*if(DEV_DSP_KEYA == DEV_DSP_KEYx)			{ldstruct = &ldStructA1;}
	else if(DEV_DSP_KEYB == DEV_DSP_KEYx)	{ldstruct = &ldStructB1;}
	else
	{
		jljRunLog("[Function:%s]�豸ѡ�����!\n", __FUNCTION__);
		return -1;
	}

	//ͨѶ����
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	ldstruct->ComX = COM7;
	else																ldstruct->ComX = COM8;

	//���������������Ա�����յ��İ���
	lstInit(&ldstruct->ButtonList);

	//�������ݽ�������
	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	strcpy(tmp_buffer, "tLDRecv");
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	strcpy(tmp_buffer + strlen(tmp_buffer), "A1");
	else																strcpy(tmp_buffer + strlen(tmp_buffer), "B1");
	tid = taskSpawn(tmp_buffer, 155, 0, 0x2000, (FUNCPTR)tLdRecv, DEV_DSP_KEYx,1,2,3,4,5,6,7,8,9);
	if(OK != taskIdVerify(tid))	printf("Error!	Creat task '%s' failed!\n", tmp_buffer);

	//����������ѯ����
	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	strcpy(tmp_buffer, "tLDPoll");
	if(DEV_DSP_KEYA == DEV_DSP_KEYx)	strcpy(tmp_buffer + strlen(tmp_buffer), "A1");
	else																strcpy(tmp_buffer + strlen(tmp_buffer), "B1");
	tid = taskSpawn(tmp_buffer, 155, 0, 0x2000, (FUNCPTR)tLdPoll, DEV_DSP_KEYx,1,2,3,4,5,6,7,8,9);
	if(OK != taskIdVerify(tid))	printf("Error!	Creat task '%s' failed!\n", tmp_buffer);*/


	return 0;
}


