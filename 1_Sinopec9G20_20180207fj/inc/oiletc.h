#ifndef _OIL_ETC_H_
#define _OIL_ETC_H_

#define ETC_05_H_LEN				6			//05����ͷ����

#define ETC_DATA_MAX				600			//������ݳ���
#define ETC_CARD_MAX				1024		//����Ϣ��󻺴�
#define ETCCARDLEN				  26			//������Ϣ����	
#define ETC_SEND_TIMES			3			  //ETC���ʹ�������

#define ETC_FUN_NO				30			  //��ֹETC����	
#define ETC_FUN_OK				31		  	//����ETC����	

#define ETC_CMD					0x80							//������
#define ETC_01					0x01							//�ͻ���ȡ�����б���Ϣ���ͻ�����
#define ETC_02					0x02							//�ͻ�ѡ�����ţ��ͻ�����
#define ETC_03					0x03							//�ͻ���ȡOBU��Ϣ���ͻ�����
#define ETC_04					0x04							//IC���ϵ磨�ͻ�����
#define ETC_05					0x05							//IC���������ͻ�����
#define ETC_06					0x06							//IC���µ磨�ͻ�����
#define ETC_07					0x07							//�ͷ�OBU���ͻ�����
#define ETC_08					0x08							//IC������У��
#define ETC_09					0x09							//һ���Զ�ȡ����Ϣ
#define ETC_0A					0x0A							//������Ʒ��
#define ETC_0B					0x0B							//֪ͨOBU���


#define ETC_PID_CARINF_READ		0x00000801					//ETC��ȡ������Ϣ
#define ETC_PID_SEL_PIN			0x00000802					//ETC������֤��
#define ETC_PID_TO_OBU			0x00000803					//ETCѡ��OBU
#define ETC_PID_INF_READ	    0x00000804					//ETCһ���Զ�ȡ����Ϣ
#define ETC_PID_OIL_SURE	    0x00000805					//��Ʒ��һ����Ҫȷ��
#define ETC_PID_PIN_CHECK		0x00000806					//ETC����


extern void etc_fun_process(unsigned char ID);//ETC���ܴ���
void EtcPidCarinfRead(unsigned char ID);      //ETC��ȡ������Ϣ
void EtcPinSelPin(unsigned char ID);          //ETC������֤��
void EtcPidToObu(unsigned char ID);           //ETCѡ��OBU
void EtcPidInfRead(unsigned char ID);         //ETCһ���Զ�ȡ����Ϣ
char EtcCardInfJieXi(unsigned char ID);       //����Ϣ����
void EtcCardCarHandle(unsigned char ID);      //���ƺŴ���
void EtcOilNameCourse(unsigned char ID);      //��Ʒ������ʾ����
void EtcCardYuehanlde(unsigned char ID);      //ETC����
void EtcPidOilSure(unsigned char ID);         //��Ʒ��һ����Ҫȷ��
void EtcUpdateGrade(unsigned char ID);        //������Ʒ��
void EtcYueDisHandle(unsigned char ID);       //��ʾ���
void EtcCardObuPin(unsigned char ID);         //OBU����
extern void EtcFreeObuCourse(unsigned char ID);//�ͷ�OBU



#endif




