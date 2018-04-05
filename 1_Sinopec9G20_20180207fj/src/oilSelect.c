//#include "oilCfg.h"
//#include "yaffs22/Yaffs_guts.h"
//#include "yaffs22/Yaffsfs.h"
//#include "oilFile.h"
//#include "oilSelect.h"

#include "../inc/main.h"

/********************************************************************
*Name			:oilNameGet
*Description	:��ȡ��Ʒ�����Ӧ��Ʒ����
*Input			:code			��Ʒ����,4λASCII
*					:maxbytes	��Ʒ���ƻ�����󳤶ȣ�Ӧ��С��16�ֽ�
*Output		:buffer			��Ʒ����
*Return			:ʵ����Ʒ���Ƴ��ȣ����󷵻�ERROR
*History		:2014-10-17,modified by syj
*/
int oilNameGet(unsigned char *code, unsigned char *buffer, int maxbytes)
{
	unsigned char read_buffer[128+16]={0};
	int fd=0, read_len=0, ireturn=0, i=0;
	off_t offset=0;

	//����Ʒ���Ʊ��ļ�
	fd=fileOpen(FILE_OILCODE_NAME, O_RDONLY, S_IREAD);

	//���ļ�����
	if(ERROR==fd){

		return ERROR;
	}

	/*��ȡ��Ʒ�����Ӧ����
	*	���ļ�ͷ��ʼ��ȡ���жϣ���ȡ��Ʒ��������Ӧ��Ʒ���ƣ�
	*	ÿ�д洢һ�����ݣ�ÿ��������˫���Ű��������ݣ�����Ʒ1010Ϊ��
	*	"1010_90#����"
	*/
	while(read_len=fileRead(fd, offset, read_buffer, 128))
	{
		//�ж��Ƿ�ƥ��
		if(0==memcmp(&read_buffer[1], code, 4)){

			//�����ȡ������Ʒ���ƣ��ӹ̶�λ��6��˫����֮��
			for(i=0; i<read_len && i<maxbytes; i++) 
			{
				if('"'==read_buffer[6+i])	break;
				buffer[i]=read_buffer[6+i];
			}

			fileClose(fd);
			return i;
		}

		//������һ����ȡλ��
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

		//��ն�ȡ������
		memset(read_buffer, 0, 128);
	}

	//�ر���Ʒ���Ʊ��ļ�
	fileClose(fd);

	//�˴���δ�˳������δ�鵽��Ӧ���ƣ��������Ͳ���Ϊ����*/
	if('1'==code[0]){

		memcpy(buffer, "����", 4);	ireturn=4;
	}
	else if('2'==code[0]){

		memcpy(buffer, "����", 4);	ireturn=4;
	}
	else if('2'==code[0]){

		memcpy(buffer, "δ֪��Ʒ", 8);	ireturn=8;
	}

	return ireturn;
}


/********************************************************************
*Name			:YPLightTurnOn
*Description	:������Ʒѡ���
*Input			:panel		���� 0=A�棻1=B��
*					:YPx			��Ʒ��ť��� 0=YP1��1=YP2��2=YP3
*Output		:None
*Return			:0=�ɹ�;����=ʧ��
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
*Description	:�ر���Ʒѡ���
*Input			:panel		���� 0=A�棻1=B��
*					:YPx			��Ʒ��ť��� 0=YP1��1=YP2��2=YP3
*Output		:None
*Return			:0=�ɹ�;����=ʧ��
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
*Description	:������Ʒѡ��ť��
*Input			:nozzle			ǹѡ0=1��ǹ��1=2��ǹ��2=3��ǹ
*Output		:None
*Return			:0=�ɹ�;����=ʧ��
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
*Description	:�ر���Ʒѡ��ť��
*Input			:nozzle			ǹѡ0=1��ǹ��1=2��ǹ
*Output		:None
*Return			:0=�ɹ�;����=ʧ��
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
*Description	:��ȡ�Ƿ�����Ʒѡ�����
*Input			:nozzle			ǹѡ0=1��ǹ��1=2��ǹ
*Output		:None
*Return			:0=�޲�����1=��ѡ�����
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

