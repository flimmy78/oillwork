#ifndef _OIL_IPT_H_
#define _OIL_IPT_H_

//#include <msgQLib.h>
//#include "oilCfg.h"
//#include "oilPcd.h"
#include "oilStmTransmit.h"
//#include "oiletc.h"
//#include "oilIC.h"

//����״̬���壬����ʱ����Ϊ1 ���ǵ���ʱ����Ϊ0
#define _IPT_DEBUG_							0

//��������
#define IPT_VERSION							"V1.00"											//IPT�汾
#define IPT_COMPANY_ID					    "\x30\x33\xff\xff\xff\xff\xff\xff"	            //���ҵ�λID
//#define IPT_DSP_WAIT()					taskDelay(3*ONE_SECOND)			                //��ʾ����ʱ��
#define IPT_DSP_WAIT()						sleep(3)                                        //��ʾ����ʱ��
#define IPT_MSG_MAX_LEN				        512					                            //IPT������Ϣ������󳤶�
#define IPT_MODE_IC							0						                        //��������ģʽ
#define IPT_MODE_UNSELF					    1						//�ǿ�������ģʽ
#define IPT_MODE_BARCODE				    2						//��������ģʽ
#define IPT_WORK_OFFDUTY				    0						//�ͻ��°�״̬
#define IPT_WORK_ONDUTY				        1						//�ͻ��ϰ�״̬
#define IPT_ROUNDING						5						//IPT������������ֽ磬С�ڴ�ֵ����

#if 0
#if _TYPE_BIG260_
#define IPT_MONEY_MAX						10000000			//�������Ԥ�ý�260��������Ϊ10��Ԫ
#define IPT_VOLUME_MAX						10000000			//�������Ԥ��������260��������Ϊ10����
#else
#define IPT_MONEY_MAX						999999				//�������Ԥ�ý���ͨ��������Ϊ9999.99Ԫ
#define IPT_VOLUME_MAX						999999				//�������Ԥ����������ͨ��������Ϊ9999.99��
#endif
#endif
#define IPT_TMAC_DEFUALT					"\x12\x34\x56\x78"	//Ĭ���˵�TMAC
#define IPT_MONEY_MIN						100					//��С����Ԥ�ý��
#define IPT_VOLUME_MIN						100					//��С����Ԥ������
#define IPT_PRICE_MAX						9999				//���������
#define IPT_PRICE_MIN						100					//��С������
#define IPT_BILLUNLOAD_MAX				    2000					//��ǹ�������δ�ϴ��˵���Ŀ
#define IPT_GUN_PUTUP						0						//̧ǹ״̬
#define IPT_GUN_PUTDOWN					    1						//��ǹ״̬
#define IPT_PRESET_NO						0						//Ԥ�÷�ʽ���������
#define IPT_PRESET_VOLUME					1						//Ԥ�÷�ʽ��������
#define IPT_PRESET_MONEY					2						//Ԥ�÷�ʽ�������
#define IPT_PAYUNIT_MONEY				    0						//���㵥λ�����
#define IPT_PAYUNIT_LOYALTY				    1						//���㵥λ�����ֵ���
#define IPT_PAYMENT_MONEY				    0						//���㷽ʽ���ֽ�
#define IPT_PAYMENT_OILTICKET			    1						//���㷽ʽ����Ʊ
#define IPT_PAYMENT_OILCHECK			    2						//���㷽ʽ������ƾ֤
#define IPT_PAYMENT_BANK					3						//���㷽ʽ�����п�
#define IPT_PAYMENT_OTHER1				    4						//���㷽ʽ������1
#define IPT_PAYMENT_OTHER2				    5						//���㷽ʽ������2					
#define IPT_CARDTYPE_SINO					0						//�����ͣ�ʯ���淶��
#define IPT_CARDTYPE_PBOC					1						//�����ͣ�PBOC���ڿ�
#define IPT_PHY_NOZZLE_MIN				    1						//����ǹ����Сֵ
#define IPT_PHY_NOZZLE_MAX				    6						//����ǹ�����ֵ
#define IPT_NIGHT_LOCK						1						//ҹ������״̬
#define IPT_NIGHT_UNLOCK					0						//��ҹ������״̬
#define IPT_SELL_LOCK						1						//��������״̬
#define IPT_SELL_UNLOCK						0						//����������״̬
#define IPT_VOICE_TYPE_WOMAN		        0						//��������:Ů��
#define IPT_VOICE_TYPE_MAN				    1						//��������:����
#define IPT_SERVEPASS_DEFAULT		        "\x99\x99"		        //Ĭ�ϵ��ۺ�ά�޲�������
#define IPT_STAPASS_DEFAULT				    "\x00\x00"		        //Ĭ�ϵ���վ��������
#define IPT_DSPCONTRAST_MIN			        25						//������ʾ�Աȶ���Сֵ
#define IPT_DSPCONTRAST_MAX			        40						//������ʾ�Աȶ����ֵ
#define IPT_DSPCONTRAST_DEFAULT	            32						//������ʾ�Աȶ�Ĭ��ֵ

//����״̬
#define IPT_OIL_IDLE						0				//����
#define IPT_OIL_STARTING					1				//����������
#define IPT_OIL_FUELLING					2				//������
#define IPT_OIL_FINISHING					3				//���ͽ�����

//IPT��PCD��ͨѶ��ʱʹ�ñ�ʶ
#define IPT_2PCD_UNUSED					0				//����
#define IPT_2PCD_POLL					1				//��������ѯ����ռ��
#define IPT_2PCD_OILPROCESS			    2				//���������ʹ������ռ��
#define IPT_2PCD_ZDSAVE					3				//�˵��Ĵ洢����ռ��
#define IPT_2PCD_INQ					4				//��ѯ���������е�ռ��

//IPT��CPOS��ͨѶ��ʱʹ�ñ�ʶ
#define IPT_2CPOS_UNUSED				0				//����
#define IPT_2CPOS_OILPROCESS			1				//�������͹���ռ��
#define IPT_2CPOS_ZDUPLOAD			    2				//��������˵��ϴ�ռ��

//�˵�����
#define IPT_BILLTYPE_NORMAL			    0				//����
#define IPT_BILLTYPE_ESCAPE				1				//�ӿ�
#define IPT_BILLTYPE_ERROR				2				//��
#define IPT_BILLTYPE_UNLOCK			    3				//����
#define IPT_BILLTYPE_FINISH				4				//����
#define IPT_BILLTYPE_WORKON			    5				//�ϰ�
#define IPT_BILLTYPE_WORKOFF			6				//�°�
#define IPT_BILLTYPE_UNSELF				7				//�ǿ�
#define IPT_BILLTYPE_OILINFO			8				//�ͼ۽���
#define IPT_BILLTYPE_REFUSE				9				//����ܾ�

//֧���ն�ǹ�ţ��˶�����Ķ�����ǰ�ڿ�����������0/1���в������Ķ����ܻ���ɴ���
#define IPT_NOZZLE_1							0				//1��֧���ն�
#define IPT_NOZZLE_2							1				//2��֧���ն�

//IPT������̺�
#define IPT_PID_PRETREAT					0x0000		//�ϵ�Ԥ����
#define IPT_PID_STANDBY						0x0001		//��������

#define IPT_PID_OPERATE_PASS			0x0101		//��ѯ:�ͻ�����׼��������֤����
#define IPT_PID_OPERATE_SELECT		    0x0102		//��ѯ:����ѡ��ѡ��
#define IPT_PID_INQUIRY					0x0103		//��ѯ:��ѯѡ��ѡ��
#define IPT_PID_INQUIRY_WAIT			0x0104		//��ѯ:��ʾ��ѯ�����ȴ������������ز�ѯѡ�����
#define IPT_PID_INQ_PRN					0x0105		//��ѯ:��ӡ��Ϣ��ѯ��������
#define IPT_PID_INQ_PRNCARD			    0x0106		//��ѯ:�Զ���ӡ����ѡ��
#define IPT_PID_INQ_PRNTYPE				0x0107		//��ѯ:�Զ���ӡ�˵�����
#define IPT_PID_INQ_TAXWAIT			0x0108		//��ѯ:˰�ز�ѯ�����ʾ��ȴ������������ز�ѯѡ�����
#define IPT_PID_INQ_TAXMSUM			0x0109		//��ѯ:˰�������·��������
#define IPT_PID_INQ_TAXDSUM			0x010a		//��ѯ:˰�����������������
#define IPT_PID_INQ_GUNINFO			0x010b		//��ѯ:��ǹ��Ϣ��ѯ����
#define IPT_PID_INQ_BOARDINFO		0x010c		//��ѯ:������Ϣ��ѯ����
#define IPT_PID_INQ_ZD_SEL				0x010d		//��ѯ:�˵�ѡ��ѡ��ǹ����������������˵�
#define IPT_PID_INQ_ZD_TTCIN			0x010e		//��ѯ:�����ѯ���˵�TTC
#define IPT_PID_INQ_ZD_CHECK			0x010f		//��ѯ:�˵���ϸ��ѯ
#define IPT_PID_INQ_ZD_INDEX			0x0110		//��ѯ:�˵���ϸ����
#define IPT_PID_INQ_ZD_DETAIL			0x0111		//��ѯ:�˵���ϸ
#define IPT_PID_INQ_OILERRLOG		0x0112		//��ѯ:��ѯ�����쳣��־
#define IPT_PID_INQ_LOCAL_NET		0x0113		//��ѯ:����������Ϣ��ѯ��ѡ�����
#define IPT_PID_INQ_LOCAL_WAIT		0x0114		//��ѯ:����������Ϣ��ѯ�����ʾ��ȴ�����
#define IPT_PID_INQ_BACKSTAGE		0x0115		//��ѯ:��̨����ͨѶ���ò�ѯ��ѡ�����
#define IPT_PID_INQ_BACKS_WAIT		0x0116		//��ѯ:��̨����ͨѶ���ò�ѯ�����ʾ��ȴ�����
#define IPT_PID_INQ_TABLETPC			0x0117		//��ѯ:ƽ��������ò�ѯ��ѡ�����
#define IPT_PID_INQ_TABLET_WAIT	        0x0118		//��ѯ:ƽ��������ò�ѯ�����ʾ��ȴ�����
#define IPT_PID_INQ_VERSION	        	0x0119		//��ѯ:�汾��Ϣ,szb_fj20171120

#define IPT_PID_SET							0x0151		//����:����ѡ��ѡ�����
#define IPT_PID_SET_PRICE					0x0152		//����:��������
#define IPT_PID_SET_TIME					0x0153		//����:ʱ������
#define IPT_PID_SET_SCJD					0x0154		//����:�״μ춨����
#define IPT_PID_SET_CCJD					0x0155		//����:�����춨����
#define IPT_PID_SET_BACKLIT				    0x0156		//����:��������
#define IPT_PID_SET_NIGHTLOCK		    0x0157		//����:ҹ����������
#define IPT_PID_SET_PASS_OLD			0x0158		//����:�����������ã����������
#define IPT_PID_SET_PASS_NEW			0x0159		//����:�����������ã�����������
#define IPT_PID_SET_PASS_ACK			0x015a		//����:�����������ã�ȷ��������
#define IPT_PID_SET_PHYNOZZLE		    0x015b		//����:����ǹ������
#define IPT_PID_SET_TAXTIME				0x015c		//����:˰��ʱ������
#define IPT_PID_SET_SPEAKER				0x015d		//����:��������������
#define IPT_PID_SET_SPKVOLUME		    0x015e		//����:������������
#define IPT_PID_SET_SPKTYPE				0x015f		//����:������������
#define IPT_PID_SET_PRINTER				0x0160		//����:��ӡ������
#define IPT_PID_SET_PRNAUTO			    0x0161		//����:�Զ���ӡ����
#define IPT_PID_SET_PRNUNION			0x0162		//����:��ӡ��������
#define IPT_PID_SET_PRNAUTO_IC		0x0163		//����:�Զ���ӡ�˵�����������
#define IPT_PID_SET_PRNAUTO_ZD		0x0164		//����:�Զ���ӡ�˵���������
#define IPT_PID_SET_ADVANCE			0x0165		//����:������ǰ������
#define IPT_PID_SET_UNPULSE_TIME	0x0166		//����:���������峬ʱͣ��ʱ������	
#define IPT_PID_SET_STAFF_LIMIT		0x0167		//����:Ա�����Ƿ������������
#define IPT_PID_SET_MODE				0x0168		//����:ģʽ����
#define IPT_PID_SET_TAX_WAIT			0x0169		//����:˰�����ý���ȴ��������
#define IPT_PID_SET_OILVOICE_SEL	    0x016a		//����:��Ʒ��������ѡ�����
#define IPT_PID_SET_OILVOICE			0x016b		//����:��Ʒ����ѡ�����
#define IPT_PID_SET_NOZZLE_NUM		    0x016c		//����:���뵥��ǹ�����ý���
#define IPT_PID_SET_BARBRAND			0x016d		//����:����ɨ��ģ�����ý���
#define IPT_PID_SET_CONNECT_TYPE	    0x016e		//����:ʯ����̨������ʽ���ý���
#define IPT_PID_SET_LOCAL_NET			0x016f		//����:����������Ϣ���ý���
#define IPT_PID_SET_LOCAL_IP			0x0170		//����:��������IP��ַ���ý���
#define IPT_PID_SET_LOCAL_MASK		0x0171		//����:�������������������ý���
#define IPT_PID_SET_LOCAL_GATE		0x0172		//����:��������Ĭ���������ý���
#define IPT_PID_SET_LOCAL_UNUSE	    0x0173		//����:�����������ý���-����
#define IPT_PID_SET_BACKSTAGE		0x0174		//����:ʯ����̨����ͨѶ���ý���
#define IPT_PID_SET_BACK_CONNECT	0x0175		//����:ʯ����̨����ͨѶ��ʽ���ý���
#define IPT_PID_SET_BACK_IP			0x0176		//����:ʯ����̨����ͨѶIP��ַ���ý���
#define IPT_PID_SET_BACK_PORT		0x0177		//����:ʯ����̨����ͨѶ�˿ں����ý���
#define IPT_PID_SET_BACK_LOCAL_PORT	0x0178		//����:ʯ����̨����ͨѶ���ط������˿ں����ý���
#define IPT_PID_SET_BACK_UNUSE2	    0x0178		//����:ʯ����̨����ͨѶ���ý���-����2
#define IPT_PID_SET_PC_INFO		    0x0179		//����:ƽ�������Ϣ���ý���
#define IPT_PID_SET_PC_IP			0x017a		//����:ƽ�����IP��ַ���ý���
#define IPT_PID_SET_PC_MASK			0x017b		//����:ƽ����������������ý���
#define IPT_PID_SET_PC_GATEWAY		0x017c		//����:ƽ�����Ĭ���������ý���
#define IPT_PID_SET_PC_DNS1			0x017d		//����:ƽ�������ѡDNS���ý���
#define IPT_PID_SET_PC_DNS2			0x017e		//����:ƽ����Ա���DNS���ý���
#define IPT_PID_SET_PC_FTP_IP		0x017f		//����:ƽ�����FTP��������ַ���ý���
#define IPT_PID_SET_PC_FTP_PORT	    0x0180		//����:ƽ�����FTP�������˿����ý���
#define IPT_PID_SET_PC_SERVER		0x0181		//����:ƽ����Եĺ�̨������IP��ַ
#define IPT_PID_SET_PC_VOLUME		0x0182		//����:ƽ�������������
#define IPT_PID_SET_PC_TELE_IP		0x0183		//����:ƽ����������Խ���̨IP��ַ���ý���
#define IPT_PID_SET_PROMOTION		0x0184		//����:���ô��������Ƿ�����
#define IPT_PID_SET_CONTRAST		0x0185		//����:���ü�����ʾ�ԱȶȽ���
#define IPT_PID_SET_GRADE_FUN		0x0186		//����:������Ʒȷ�Ϲ����Ƿ�����
#define IPT_PID_SET_ETC_FUN			0x0187		//����:ETC����,szb_fj20171120

#define IPT_PID_OTH_PASS			0x0191		//����:��������������֤����
#define IPT_PID_OTHER				0x0192		//����:������������
#define IPT_PID_OTHER_KEYLOAD		0x0193		//����:��Կ���ع���
#define IPT_PID_OTHER_MODEL			0x0194		//����:�������ù���
#define IPT_PID_OTHER_SUMWRITE	    0x0195		//����:�ۼ����޸Ĺ���
#define IPT_PID_OTHER_WAIT		    0x0196		//����:����������ʾ����ȴ���������

#define IPT_PID_IC_STANDBY			0x0201		//���Ϳ�:�����������
#define IPT_PID_IC_PRETREAT			0x0202		//���Ϳ�:IC������Ԥ����������Ϣ
#define IPT_PID_PSAM_PRETREAT		0x0203		//���Ϳ�:PSAMԤ����PSAM�Ϸ��Լ��
#define IPT_PID_IC_CHECK			0x0204		//���Ϳ�:PSAM����IC���Ϸ��Լ��
#define IPT_PID_IC_PIN_INPUT		0x0205		//���Ϳ�:IC����������
#define IPT_PID_IC_PIN_CHECK		0x0206		//���Ϳ�:IC��������֤
#define IPT_PID_IC_STAF_PASSIN		0x0207		//���Ϳ�:�޳��ſ�Ա��������֤
#define IPT_PID_IC_NOTES			0x0208		//���Ϳ�:IC����ʷ���׼�¼��ѯ
#define IPT_PID_IC_NOTES_CHECK		0x0209		//���Ϳ�:IC����ʷ���׼�¼�Ϸ��Լ��
#define IPT_PID_IC_BL_CHECK			0x020a		//���Ϳ�:IC����/���������
#define IPT_PID_IC_LOCK_INFO		0x020b		//���Ϳ�:IC���������׼�¼��ѯ
#define IPT_PID_IC_LOCKRECORD		0x020c		//���Ϳ�:��ѯ�ҿ����׼�¼
#define IPT_PID_IC_BAL_READ			0x020d		//���Ϳ�:IC������ȡ
#define IPT_PID_IC_USER_ACK			0x020e		//���Ϳ�:�û������Ͱ�ȷ�ϼ��ͷž���
#define IPT_PID_IC_BALANCE			0x020f		//���Ϳ�:IC�������ʾ����
#define IPT_PID_IC_PAY_UNIT			0x0210		//���Ϳ�:���㵥λѡ��
#define IPT_PID_IC_PAY_MODE			0x0211		//���Ϳ�:���㷽ʽѡ��
#define IPT_PID_IC_LOGIN			0x0212		//���Ϳ�:�ϰ��½��������
#define IPT_PID_IC_LOG_PASSIN		0x0213		//���Ϳ�:�ϰ��½Ա���������������
#define IPT_PID_IC_LOGOUT			0x0214		//���Ϳ�:�°�ȷ�Ͻ���
#define IPT_PID_IC_LOGOUT_PASSIN	0x0215		//���Ϳ�:�°������������
#define IPT_PID_IC_OIL_ACK			0x0216		//���Ϳ�:��Ʒȷ�Ͻ���
#define IPT_PID_IC_OILCHECK			0x0217		//���Ϳ�:�����������ݺϷ��Լ��
#define IPT_PID_IC_OIL_AUTHEN		0x0218		//���Ϳ�:IC������������֤
#define IPT_PID_IC_LOCK_INIT		0x0219		//���Ϳ�:������ʼ��
#define IPT_PID_IC_MAC1				0x021a		//���Ϳ�:PSAM����MAC1
#define IPT_PID_IC_LOCK				0x021b		//���Ϳ�:IC������
#define IPT_PID_IC_LOCK_GREYINFO	0x021c		//���Ϳ�:IC��������ʱ���ȡ����״̬
#define IPT_PID_IC_ZD_ESCAPE		0x021d		//���Ϳ�:���͹��̲����ӿ��˵�
#define IPT_PID_IC_MAC2					0x021e		//���Ϳ�:PSAM��֤MAC2
#define IPT_PID_IC_OILSTART				0x021f		//���Ϳ�:IC����ʼ����
#define IPT_PID_IC_OILLING				0x0220		//���Ϳ�:IC��������
#define IPT_PID_IC_OILFINISH			0x0221		//���Ϳ�:IC����������
#define IPT_PID_IC_PSAM_GMAC			0x0222		//���Ϳ�:PSAM����GMAC
#define IPT_PID_IC_UNLOCK				0x0223		//���Ϳ�:IC��jie��
#define IPT_PID_IC_TAC_CLEAR			0x0224		//���Ϳ�:IC��TAC���
#define IPT_PID_IC_TTC_GET				0x0225		//���Ϳ�:��ȡPCD�����TTC
#define IPT_PID_IC_TMAC					0x0226		//���Ϳ�:�����˵�TMAC
#define IPT_PID_IC_ZD_SAVE				0x0227		//���Ϳ�:���������ϴ�
#define IPT_PID_IC_LASTSTEP				0x0228		//���Ϳ�:���ͽ����Ĵ���
#define IPT_PID_IC_ENDACK				0x0229		//���Ϳ�:���ͽ����ȴ���������
#define IPT_PID_IC_OIL_OVERINFO	        0x022a		//���Ϳ�:���͹�����Ϣ����
#define IPT_PID_IC_ESCAPE_ERR			0x022b		//���Ϳ�:�ӿ���������				
#define IPT_PID_IC_OIL_OVERSTAFF	    0x022c		//���Ϳ�:���͹���Ա�������������
#define IPT_PID_ACT_AUTHEN				0x022d		//���Ϳ�:ACT��֤�������
#define IPT_PID_RID_AUTHEN				0x022e		//���Ϳ�:RID��֤�������
#define IPT_PID_IC_DISCOUNT_ASK	    0x022f		//���Ϳ�:(����ϵͳ)���̨���뱾�μ�����Ҫ������
#define IPT_PID_IC_DISCOUNT_KEEP	0x0230		//���Ϳ�:���̨�����ۿ۶�ʧ�ܣ��˹�ȷ�Ϻ�����Է��ۿۼ۸����
#define IPT_PID_IC_DEBIT_START		0x0231		//���Ϳ�:������Ϳ�֧������
#define IPT_PID_IC_DEBIT_DONE		0x0232		//���Ϳ�:���Ϳ�֧��ȷ�Ϲ���

#define IPT_PID_UNSELF_PRESET			0x0301		//�ǿ�����������Ԥ�ù���
#define IPT_PID_UNSELF_CHECK			0x0302		//�ǿ���������������������
#define IPT_PID_UNSELF_START			0x0303		//�ǿ�������������������
#define IPT_PID_UNSELF_OILLING		    0x0304		//�ǿ�������������
#define IPT_PID_UNSELF_FINISH			0x0305		//�ǿ����������ͽ���

//#define IPT_PID_TM_BAR_SCAN			0x0401		//��������:�ȴ�����ɨ������
//#define IPT_PID_TM_BAR_INPUT			0x0402		//��������:�����������
#define IPT_PID_TM_PRETREAT			0x0401		//��������:����ɨ��ǰ��Ԥ������
#define IPT_PID_TM_SCAN				0x0402		//��������:����ɨ�輰�������
#define IPT_PID_TM_IC_PRETREAT		0x0403		//��������:IC��Ԥ����
#define IPT_PID_TM_PSAM_PRE			0x0404		//��������:PSAMԤ����
#define IPT_PID_TM_CHECK				0x0405		//��������:PSAM����IC���Ϸ��Լ��
#define IPT_PID_TM_PIN_INPUT			0x0406		//��������:IC����������
#define IPT_PID_TM_PIN_CHECK			0x0407		//��������:IC��������֤
#define IPT_PID_TM_BL_CHECK			    0x0408		//��������:IC����/���������
#define IPT_PID_TM_LOCK_INFO			0x0409		//��������:IC���������׼�¼��ѯ
#define IPT_PID_TM_LOCKRECORD		0x040a		//��������:��ѯ�ҿ����׼�¼
#define IPT_PID_TM_BAL_READ			0x040b		//��������:IC������ȡ
#define IPT_PID_TM_CODE_CHECK		0x040c		//��������:���ͻ���PC����ѯ��֤��
#define IPT_PID_TM_CODE_ACK			0x040d		//��������:���ͻ�ȷ��PC���Ĳ�ѯ���
#define IPT_PID_TM_AUTHORIZED		0x040e		//��������:��Ȩ�ɹ�����
#define IPT_PID_TM_PAY_MODE			0x040f		//��������:֧����ʽѡ�����
#define IPT_PID_TM_OIL_ACK				0x0410		//��������:��Ʒȷ�Ͻ���
#define IPT_PID_TM_OILCHECK				0x0411		//��������:�����������ݺϷ��Լ��
#define IPT_PID_TM_OIL_AUTHEN		    0x0412		//��������:IC������������֤
#define IPT_PID_TM_LOCK_INIT			0x0413		//��������:������ʼ��
#define IPT_PID_TM_MAC1						0x0414		//��������:PSAM����MAC1
#define IPT_PID_TM_LOCK						0x0415		//��������:IC������
#define IPT_PID_TM_MAC2						0x0416		//��������:PSAM��֤MAC2
#define IPT_PID_TM_OILSTART				0x0417		//��������:IC����ʼ����
#define IPT_PID_TM_OILLING				0x0418		//��������:IC��������
#define IPT_PID_TM_OILFINISH			0x0419		//��������:IC����������
#define IPT_PID_TM_PSAM_GMAC		    0x041a		//��������:PSAM����GMAC
#define IPT_PID_TM_UNLOCK				0x041b		//��������:IC��jie��
#define IPT_PID_TM_TAC_CLEAR			0x041c		//��������:IC��TAC���
#define IPT_PID_TM_TTC_GET				0x041d		//��������:��ȡPCD�����TTC
#define IPT_PID_TM_TMAC					0x041e		//��������:�����˵�TMAC
#define IPT_PID_TM_ZD_SAVE				0x041f		//��������:�������ݱ���
#define IPT_PID_TM_ZD_UPLOAD			0x0420		//��������:���ͻ������ϴ���Ȩ���ͽ����Ϣ
#define IPT_PID_TM_OIL_END				0x0421		//��������:���ͽ�������
#define IPT_PID_TM_OIL_ENDWAIT		    0x0422		//��������:���ͽ����ȴ�����

#define IPT_PID_AUTH_PRETREAT		0x0501		//��Ȩ����Ԥ����
#define IPT_PID_AUTH_BALANCE		0x0502		//��Ȩ����
#define IPT_PID_AUTH_OIL_CHECK		0x0503		//��Ȩ��������ǰ�����Լ����
#define IPT_PID_AUTH_OIL_START		0x0504		//��Ȩ������������
#define IPT_PID_AUTH_OILLING		0x0505		//��Ȩ������
#define IPT_PID_AUTH_OIL_FINISH	    0x0506		//��Ȩ���ͽ�������
#define IPT_PID_AUTH_DEBIT_APPLY	0x0507		//��Ȩ��������ۿ����
#define IPT_PID_AUTH_BILL_SAVE		0x0508		//��Ȩ�����˵��洢����
#define IPT_PID_AUTH_START_ERR		0x0550		//��Ȩ��������ʧ��
#define IPT_PID_AUTH_PASS_INPUT	    0x0551		//��Ȩ����ETC�������������


#define IPT_PID_ERR_OILOVER			0x0601		//���ͻ����������ȴ��������
#define IPT_PID_ERRINFO_TMAC		0x0602		//�ڲ�������Ϣ����TMAC����
#define IPT_PID_ERRINFO_UPLOAD		0x0603		//�ڲ�������Ϣ�ϴ�����
#define IPT_PID_DEBUG_CARD_TEST	    0x0604		//��⿨������
#define IPT_PID_DEBUG_TIME			0x0605		//��⿨��Э������������ѯʱ�����ù���
#define IPT_PID_ERR_QUEYILU			0x0606		//ȱһ·����ͣ��,szb_fj20171120
#define IPT_PID_ERR_LINGJIAYOU		0x0607		//�����ͣ��,szb_fj20171120
#define IPT_PID_ERR_WUPULSE			0x0608		//�����峬ʱͣ��,szb_fj20171120
#define IPT_PID_ERR_QUEYIZU			0x0609		//ȱһ������ͣ��,szb_fj20171120

#define IPT_PID_BANK_PIN_INPUT		0x0701		//���п�����:PIN�������
#define IPT_PID_BANK_PRESET			0x0702		//���п�����:Ԥ�ý���
#define IPT_PID_BANK_AUTH_REGUST	0x0703		//���п�����:Ԥ��Ȩ����
#define IPT_PID_BANK_AUTH_RESULT	0x0704		//���п�����:Ԥ��Ȩ����ɹ��ȴ���������
#define IPT_PID_BANK_OIL_START		0x0705		//���п�����:����������
#define IPT_PID_BANK_OILLING		0x0706		//���п�����:������
#define IPT_PID_BANK_OIL_FINISH		0x0707		//���п�����:���ͽ�����
#define IPT_PID_BANK_AUTH_CANCEL	0x0708		//���п�����:Ԥ��Ȩ����
#define IPT_PID_BANK_AUTH_ACK		0x0709		//���п�����:Ԥ��Ȩȷ�Ͽۿ�
#define IPT_PID_BANK_END_WAIT		0x070A		//���п�����:���׽�����ȴ�����


//�ͻ�״̬
#define IPT_STATE_IDLE					0x0000		//����
#define IPT_STATE_CARD_PRETREAT			0x0001		//���Ϳ�Ԥ����
#define IPT_STATE_CARD_PASSIN			0x0002		//���Ϳ���������
#define IPT_STATE_CARD_PLATE			0x0003		//���Ϳ��޳���
#define IPT_STATE_CARD_REUNLOCK			0x0004		//���Ϳ�����
#define IPT_STATE_CARD_BALANCE			0x0005		//���Ϳ���Ԥ��
#define IPT_STATE_CARD_APP_SEL			0x0006		//���Ϳ�Ӧ��ѡ��
#define IPT_STATE_CARD_TYPE_SEL			0x0007		//���Ϳ�֧����ʽѡ��
#define IPT_STATE_CARD_OILSTART			0x0008		//���Ϳ���������
#define IPT_STATE_CARD_OILLING			0x0009		//���Ϳ�������
#define IPT_STATE_CARD_OILFINISHING	    0x000A		//���Ϳ����ͽ���������

#define IPT_STATE_UNSELF_PRESET			0x000C		//�ǿ�Ԥ�ý���
#define IPT_STATE_UNSELF_OILSTART		0x000D		//�ǿ�����������
#define IPT_STATE_UNSELF_OILLING		0x000E		//�ǿ�������
#define IPT_STATE_UNSELF_OILEND			0x000F		//�ǿ����ͽ��
#define IPT_STATE_ERROR_INFO			0x0010		//������Ϣ��ʾ
#define IPT_STATE_TM_SCAN					0x0013		//���룬��ɨ��������ڼ�������������
#define IPT_STATE_TM_APPLY_FOR				0x0014		//���룬������Ȩ������
#define IPT_STATE_TM_APPLY_OK				0x0015		//���룬��Ȩ����ɹ�����
#define IPT_STATE_TM_CANCELING				0x0016		//���룬��Ȩȡ����
#define IPT_STATE_TM_OIL_START				0x0017		//���룬����������
#define IPT_STATE_TM_OILLING				0x0018		//���룬������
#define IPT_STATE_TM_FINISHING				0x0019		//���룬���ͽ�����
#define IPT_STATE_TM_OIL_END				0x0020		//���룬���ͽ��
#define IPT_STATE_CARD_OILEND				0x0021		//���Ϳ����ͽ������
#define IPT_STATE_AUTH_BALANCE				0x0022		//��Ȩ���ͣ��ͻ������Ȩ
#define IPT_STATE_AUTH_CANCEL				0x0023		//��Ȩ���ͣ��ͻ�ȡ����Ȩ
#define IPT_STATE_AUTH_OILSTART			    0x0024		//��Ȩ���ͣ��ͻ�����������
#define IPT_STATE_AUTH_OILLING				0x0025		//��Ȩ���ͣ��ͻ�������
#define IPT_STATE_AUTH_OILFINISH			0x0026		//��Ȩ���ͣ��ͻ����ͽ�����
#define IPT_STATE_AUTH_OIL_DATA			    0x0027		//��Ȩ���ͣ��ͻ����ͽ��





//IPT��PCDͨѶ������
#define IPT_CMD_POLL						0x01			//��ͨ��ѯ����
#define IPT_CMD_FORTTC						0x02			//IPT����TTC
#define IPT_CMD_ZDSAVE						0x03			//IPT���뱣���˵�
#define IPT_CMD_LIST						0x04			//IPT��ѯ��/������
#define IPT_CMD_GREYINFO					0x05			//IPT��ѯ������¼
#define IPT_CMD_PRINTE						0x06			//IPT��ӡ����
#define IPT_CMD_SPK							0x07			//IPT��������
#define IPT_CMD_ZD_CHECK					0x08			//IPT��ѯ�����˵�
#define IPT_CMD_ID_SET						0x09			//IPT����PCD����ID
#define IPT_CMD_BARCODE					0x0a			//IPT��������ת��
#define IPT_CMD_FOR_TMAC				0x0b			//PCD����IPT����TMAC
#define IPT_CMD_ERRINFO_UPLOAD	0x0c			//IPTͨ��PCD�ϴ��ڲ�������Ϣ����̨
#define IPT_CMD_DISCOUNT_ASK		0x0d			//IPTͨ��PCD���̨�����ۿ۶�

//�˵�����ƫ�ƶ���
#define IPT_BILL_SIZE									128			//�˵�����
#define IPT_OFFSET_TTC									(0)			//POS_TTC	4bytes
#define IPT_OFFSET_T_TYPE							(4)			//��������1byte
#define IPT_OFFSET_TIME								(5)			//�������ڼ�ʱ��7bytes��Ӧ��
#define IPT_OFFSET_ASN								(12)			//��Ӧ�ú�10bytes
#define IPT_OFFSET_BALANCE						(22)			//���4bytes
#define IPT_OFFSET_AMN								(26)			//����3bytes
#define IPT_OFFSET_CTC									(29)			//���������2bytes
#define IPT_OFFSET_TAC									(31)			//����ǩ��4bytes
#define IPT_OFFSET_GMAC								(35)			//�����֤��4bytes
#define IPT_OFFSET_PSAM_TAC						(39)			//PSAM����ǩ��4bytes
#define IPT_OFFSET_PSAM_ASN					(43)			//PSAMӦ�ú�10bytes
#define IPT_OFFSET_TID									(53)			//PSAM���6bytes
#define IPT_OFFSET_PSAM_TTC						(59)			//PSAM�ն˽������4bytes
#define IPT_OFFSET_DS									(63)			//�ۿ���Դ1byte
#define IPT_OFFSET_UNIT								(64)			//���㵥λ/��ʽ1byte
#define IPT_OFFSET_C_TYPE							(65)			//����1byte
#define IPT_OFFSET_VER								(66)			//���汾1byte	b7~b4:����Կ�����ţ�b3~b0:����Կ�汾��
#define IPT_OFFSET_NZN								(67)			//ǹ��1byte
#define IPT_OFFSET_G_CODE							(68)			//��Ʒ����2bytes
#define IPT_OFFSET_VOL								(70)			//����3bytes
#define IPT_OFFSET_PRC								(73)			//�ɽ��۸�2bytes
#define IPT_OFFSET_EMP								(75)			//Ա����1byte
#define IPT_OFFSET_V_TOT							(76)			//���ۼ�4bytes
#define IPT_OFFSET_RFU								(80)			//���ò���11bytes
#define IPT_OFFSET_MONEY_DISCOUNT		(80)			//(������ϵͳ)�ۿۺ��3HEX
#define IPT_OFFSET_DISCOUNT						(83)			//(������ϵͳ)�ۿ۶2HEX
#define IPT_OFFSET_STATE							(85)			//(��Ȩ����)�˵�״̬ 1byte 0=������1=ETC���ۿ�ʧ��
#define IPT_OFFSET_T_MAC							(91)			//�ն�������֤��4bytes........................................
#define IPT_OFFSET_PHYGUN							(95)			//����ǹ��1byte
#define IPT_OFFSET_STOPNO							(96)			//����ͣ������1byte
#define IPT_OFFSET_BEFOR_BAL					(97)			//��ǰ���4bytes
#define IPT_OFFSET_ZD_STATE						(101)		//�˵�״̬0=������1=δ���
#define IPT_OFFSET_JLNOZZLE						(102)		//����ǹ��1byte
																							//���ã�103~126
#define IPT_OFFSET_INVOICE_TYPE				(103)		//(������ϵͳ)��Ʊ����1HEX
#define IPT_OFFSET_ZDXOR							(127)		//�˵����У��,1byte
#define IPT_OFFSET_ZDBACKUP						(128)		//128~255�˵����ݣ��ܳ�128�ֽ�

//�ǿ�/�����˵����ݱ���ƫ��
#define IPT_UNSELF_BILL_SIZE					32				//�ǿ�/�����˵����ݱ���ƫ��
#define IPT_UNSELF_OFF_MONEY				0					//3HEX	���μ��ͽ�(ƫ��0)
#define IPT_UNSELF_OFF_VOLUME				3				//3HEX	���μ���������(ƫ��3)
#define IPT_UNSELF_OFF_STATE					6				//1byte	�ǿ�����״̬��(ƫ��6)
#define IPT_UNSELF_OFF_SUMMONEY			7				//3HEX	�����ۼƽ�(ƫ��7)
#define IPT_UNSELF_OFF_SUMVOLUME		10				//3HEX	�����ۼ�������(ƫ��10)
#define IPT_UNSELF_OFF_OVERTIMES		13					//1HEX	���������(ƫ��13)
#define IPT_UNSELF_OFF_CHECK					30				//2bytes	CRCУ�飬����ǰ30bytes��(ƫ��30)


//���������˵����ݱ���ƫ��
#define IPT_BAR_BILL_SIZE							32				//�����˵���С
#define IPT_BAR_OFF_NOZZLE						0				//ǹ��1byte
#define IPT_BAR_OFF_TTC							1				//POS_TTC	4bytes
#define IPT_BAR_OFF_TIME							5				//����ʱ��7bytes
#define IPT_BAR_OFF_MONEY						12				//���ͽ��3bytes
#define IPT_BAR_OFF_VOLUME					15				//��������3bytes
#define IPT_BAR_OFF_AUTH_MONEY			18				//��Ȩ���3bytes
#define IPT_BAR_OFF_AUTH_CODE				21				//��Ȩ��֤��5bytes
#define IPT_BAR_OFF_PRICE						26				//����2bytes
#define IPT_BAR_OFF_OIL_CODE					28				//��Ʒ����2bytes
#define IPT_BAR_OFF_STATE						30				//�˵�״̬1byte��0=���ϴ���1=δ�ϴ�
#define IPT_BAR_OFF_CHECK						31				//У����1byte


//����洢��ַ��ÿ��ǹ512bytes
#define IPT_FM_DATALEN						512								//��IPTʹ�õ������СΪ,��ǹ���ݳ���
#define IPT_FM_ZD									0									//�˵���ʼ��ַ
#define IPT_FM_ZDBACKUP					(IPT_FM_ZD+128)			//128~255�˵����ݣ��ܳ�128�ֽ�

#define IPT_FM_ZD_UNSELF					256								//256~287�����ηǿ���������,��32bytes
																			//3HEX	���μ��ͽ�(ƫ��0)
																			//3HEX	���μ���������(ƫ��3)
																			//1byte	�ǿ�����״̬��(ƫ��6)
																			//3HEX	�����ۼƽ�(ƫ��7)
																			//3HEX	�����ۼ�������(ƫ��10)
																			//1HEX	���������(ƫ��13)
																			//2bytes	CRCУ�飬����ǰ30bytes��(ƫ��30)
																									
#define IPT_FM_ZD_UNSELF2				288								//288~�ǿ��������ݱ���,��32bytes																		

#define IPT_FM_ZD_TM							320								//����������������,��32bytes
																				//(ƫ��0)	1byteǹ�ţ�
																				//(ƫ��1)	4bytes POS_TTC��
																				//(ƫ��5)	7bytes����ʱ�䣻
																				//(ƫ��12)	3bytes���ͽ�
																				//(ƫ��15)	3bytes����������
																				//(ƫ��18)	3bytes��Ȩ��
																				//(ƫ��21)	5bytes��Ȩ��֤�룻
																				//(ƫ��26)	2bytes���ۣ�
																				//(ƫ��28)	2bytes��Ʒ���룻
																				//(ƫ��30)	1byte�˵�״̬0=���ϴ���1=δ�ϴ���
																				//(ƫ��31)	1byte���У����
																									
#define IPT_FM_ZD_TM2						352								//���������������ݱ���,��32bytes

#define IPT_FM_OIL_ATUO					384								//�Ƿ��Զ�����

//2017-02-13����ϵͳֱ�Ӱ�����ĵ����ݱ���
#define IPT_LDPowerData					385	//����ϵͳֱ������ε�4�ֽ�(1�ֽ��ۿ۵�λ1�ֽ��Ƿ��ۿۿ�2�ֽ��ۿ۵���)

#define FM_ETC_FREE_FLG_A				389//ETC�ͷű�־����1�ֽ�+4�ֽڵ�MAC��,szb_fj20171120
#define FM_ETC_FREE_FLG_B				394//ETC�ͷű�־����1�ֽ�+4�ֽڵ�MAC��,szb_fj20171120
#define IPT_FM_BILL_FLAG				399//1�ֽڴ洢iptparam->OilBillSave��־,szb_fj20171120
#define FM_ERR_QUEYILU_PULSE			400//1�ֽ�����ȱһ·���峬��6������,szb_fj20171120
#define FM_ERR_LING_JIAYOU				401//1�ֽ���������ͳ���6������,szb_fj20171120
#define FM_ERR_WU_PULSE					402//1�ֽ����������峬ʱ����6������,szb_fj20171120
#define FM_ERR_QUEYIZU_PULSE			403//1�ֽ�����ȱһ�����峬��6������,szb_fj20171120
#define FM_ERR_BIAN_PRICE				404//1�ֽڱ��ʧ�ܱ�־,szb_fj20171120

//��ȼ���￨���Ͷ���
#define IPT_LIANDA_APPTYPE_JIZHANG		0x12			//���˿�
#define IPT_LIANDA_APPTYPE_DAICHU		0x13			//������
#define IPT_LIANDA_APPTYPE_ZIYONG		0x14			//���ÿ�
#define IPT_LIANDA_APPTYPE_DIAOBO		0x15			//������
#define IPT_LIANDA_APPTYPE_GUOBIAO	    0x16			//����
#define IPT_LIANDA_APPTYPE_DAICHUSIJI	0x17		//������˾����

//2017-02-13��Ʒ���Ʒ�ʽ����
//#define IPT_SET_OILLIMIT_ZSH			0x30			//��ʯ����׼
//#define IPT_SET_OILLIMIT_JR			0x31			//������ʯ����׼
#define IPT_SET_OILLIMIT_F				0x30			//F��׼
#define IPT_SET_OILLIMIT_0				0x31			//0��׼
#define IPT_SET_OILLIMIT_0F				0x32			//0F��׼

//IC������ϸ
typedef struct
{
	unsigned char TTC[2];						//ET�������ѻ��������
	unsigned char Limit[3];						//͸֧�޶�
	unsigned char Money[4];					//���׽��
	unsigned char Type;							//�������ͱ�ʶ
	unsigned char TermID[6];				//�ն˻����
	unsigned char Time[7];						//����ʱ��
}IptIcRecordType;

																									

//֧���ն�(IPT)��������������
typedef struct
{
	//����������Ϣ
#if _IPT_DEBUG_
	unsigned int testOilTimes;
#endif

	//������Ϣ
	unsigned char Id;								//֧���ն˺�(������), 0=1��ǹ(IPT_NOZZLE_1);1=2��ǹ(IPT_NOZZLE_2)
	unsigned char PhysicalNozzle;			//����ǹ�ţ�һ��ͨѶ�ն��ڲ�������ǹ�ţ������÷�Χ1~6
	
	//MSG_Q_ID MsgIdRx;						//������Ϣ����ID
	int MsgIdRx;                            //���պͷ�����Ϣ��ID

	int tId;												//֧���ն˴�������ID
	unsigned int ProcessId;					//������̺�
	unsigned char Step;							//������ţ�ĳһ���̺ŵķ�֧���
	unsigned int NousedTimer;				//�˼������ۼ�֧���ն����κβ�����ʱ�䳤��
	unsigned int FMAddrBase;				//����洢���ݻ�����ַ
	unsigned int UserID;							//�û�ID��>0

	//����Դ״̬��ʱ����
	unsigned int PowerStateTimer;

	//��ǹ��Ϣ
	unsigned char NozzleNumber;			//�߼���ǹ��Ŀ
	unsigned char LogicNozzle;				//�߼����ţ�����վ������ǹ��,0��ʾ�Ƿ�
	unsigned char OilVersion;					//��̨�·�����Ʒ�ͼ۱�汾
	unsigned char OilPriceBySNPC[2];	//��̨�·��ļ۸� HEX
	unsigned char OilCode[2];					//��Ʒ����
	unsigned char OilName[32];				//��Ʒ����
	unsigned char JlNozzle;						//����ǹ�ţ�0=1��ǹ��1=2��ǹ

	//��Ӧ������Ϣ
	unsigned char DEVPIR;						//��Ӧ�����豸ѡ��0=A���̣�1=B����

	//�Ƿ���ͣʹ�� 0 = ��1 = ��
	unsigned char IsSuspend;

	//�����Ƿ���Ҫ��֤ ;0=��ҪDES��֤��1=����ҪDES��֤
	unsigned char DESAuthen;			

	//IPTģʽ:0=����������1=�ǿ���������2=��������ģʽ
	unsigned char Mode;		

	//��������
	unsigned char Password[2];				//��վ��������2BCD
	unsigned char ServicePass[2];			//�ۺ��������2BCD
	unsigned char TempBuffer[74];		//��������ʹ�õ���ʱ����

	//��ʾ�豸��Ϣ
	unsigned int DEVDsp;						//������ʾ�豸ID, 0=1�ţ�1=2��
	unsigned char Contrast;					//������ʾ�Աȶ�

	//�������豸��Ϣ
	unsigned int DEVBuzzer;					//�������豸ID��0=A���̷�������1=B���̷�����

	//�����豸��Ϣ
	unsigned int DEVButton;
	unsigned int Button;							//��ǰ����

	//Կ����Ϣ
	int KeyLock;										//Կ��״̬��0=����λ�ã�1=����λ��
	int KeyLockChg;									//Կ��״̬�Ƿ����仯

	//��ϸ��ѯʱ�Ĳ���
	unsigned int InqBillTTC;						//��ѯ���˵�TTC��
	unsigned char InqBillNozzle;					//��ѯ�˵�����0=��������0=����ǹ��
	unsigned char InqBillNext;					//0=��ѯ��TTC�˵���1=��ѯ��TTC��һ�ʣ�2=��ѯ��TTC��һ��
	unsigned char InqBillPage;					//�˵���ѯҳ��
	unsigned char InqBill[IPT_BILL_SIZE];	//�˵�

	//��ǹ�����豸��Ϣ
	int DEVGun;										//��ǹ DEV_GUNA1/DEV_GUNB1
	int GunState;									//��ǹ����״̬��0=��ǹ��1=��ǹ
	char GunStateChg;							//��ǹ����״̬�����仯��0=�ޱ仯��1=�б仯

	//���°���Ϣ
	unsigned char WorkState;				//���°�״̬0=�°ࣻ1=�ϰ�
	unsigned char EMP;							//�ϰ�Ա����
	unsigned char EMPPassword[2];		//�ϰ�Ա������
	unsigned char EMPCardID[10];			//�ϰ�Ա��������

	//����������ز���
	unsigned int PassTimer;					//�����������������ʹ�ô�ֵ���������
	unsigned char PassRandom[6];		//����������������

	//ҹ��������ز���
	unsigned char NightLock;					//0=������1=ҹ������

	//����������ز���
	unsigned char SLockTime[4];			//����������������ʱ��YYYYMMDD
	unsigned char SellLock;					//0=������������=��������

	//����������ز���
	unsigned char SetPassNew[4];		//����ʱ�����������ASCII�����ֽ�Ϊ���볤��
	unsigned char SetPassNew2[4];		//����ʱ�����������ȷ��ASCII�����ֽ�Ϊ���볤��

	//ʱ����ز���
	RTCTime Time;									//��ǰʱ�� 
	RTCTime LastTime;							//ǰһ��ʱ��

	//����������ز���
	unsigned char CardStaffLimit;			//Ա������������0=������ͣ�1=���������	

	//��ѯ/���ò�������
	unsigned char SetPage;					//һ������ҳ��ţ���ѯ/����ѡ��ѡ�����
	unsigned char SetPage2;					//�μ�����ҳ���
	unsigned char SetButtonFlag;			//������ʶ0=������������δ������1=���а���
	unsigned char SetButton[16];			//����
	unsigned char SetButtonLen;			//��������
	unsigned int SetParam;					//���ò���ʱ��ʱ�洢���õ�ֵ��ȷ������ʱ����ֵ����ʵ�ʱ���

	//������С���Ĳ�����Ԥ�ü���ʱ�İ���
	unsigned char IntegerBuffer[8];		//����
	unsigned char IntegerLen;				//��������
	unsigned char Point;							//С���� 0=�ޣ�1=��
	unsigned char DecimalBuffer[2];		//С��
	unsigned char DecimalLen;				//С������

	//������ز���
	unsigned char Speaker;					//������ѡ��(0x10=A1/0x11=A2/0x20=B1/0x21=B2/0x30=C1/0x31=C2)
	unsigned char VoiceType;					//��������0=Ů����1=����
	unsigned char VoiceVolume;				//����0~99��HEX
	unsigned char VoiceFlag;					//���ĳЩ���������������ı��
	unsigned char OilVoice[4];				//��Ʒ������ţ�ASCII����ѡ��δ����ʱ����Ʒ����"�ӿ�Э��1.1"����Ʒ�����ѯ��Ӧ��Ʒ����

	//��ӡ��ز���
	unsigned short *PrintCard;				//��ѯ/����ʱ�������Զ���ӡ�˵����Ϳ���ָ��
	unsigned char Printer;						//��ӡ��ѡ��(0x10=A1/0x11=A2/0x20=B1/0x21=B2/0x30=C1/0x31=C2)
	unsigned char PrintAuto;					//�Զ���ӡ��ʶ0=���Զ���ӡ��1=�Զ���ӡ
	unsigned char PrintUnion;					//0=��ӡһ��(���ͻ���)��1=��ӡ����(����վ�����ͻ���)
	unsigned short PrintCardUser;			//�û����Զ���ӡ�˵�����0=����ӡ��1=�Զ���ӡ
											//b0=������b1=�ӿ���b2=����b3=���ۣ�b4=���䣻b5=�ϰࣻ
											//b6=�°ࣻb7=�ǿ���b8=�ͼۻ�Ӧ��b9=�����׳���
	unsigned short PrintCardManage;	//�����Զ���ӡ�˵����ͣ���ϸ����ͬ��
	unsigned short PrintCardStaff;			//Ա�����Զ���ӡ�˵����ͣ���ϸ����ͬ��
	unsigned short PrintCardPump;		//��ÿ��Զ���ӡ�˵����ͣ���ϸ����ͬ��
	unsigned short PrintCardService;		//ά�޿��Զ���ӡ�˵����ͣ���ϸ����ͬ��

	//����Ϣ
	unsigned char BindTime[7];				//��ʱ��
	unsigned char BindMboardId[8];		//��оƬID
	unsigned char BindACTAppId[10];	//��ACT����
	unsigned char BindRIDAppId[10];	//��RID����

	//����������Ȩ������Ϣ
	unsigned char BarUserID;				//��ǰ��ǹ�û�ID����Χ>=1
	unsigned char BarOilBill[32];				//���������˵�
	unsigned char BarBillUpload;			//�Ƿ�����������˵���Ҫ�ϴ�0=�ޣ�1=��
	unsigned char CardWhite[10];			//���ÿ��Ұ�����������
	unsigned char DEVICInternal;			//���ÿ��������豸ѡ��
	unsigned char BarcodeSwitch;			//��Ʒѡ�񿪹�DEV_SWITCH_SELA1/DEV_SWITCH_SELB1
	unsigned char DEVBarcode;				//����ģ���豸BARCODE_NOZZLE_1/BARCODE_NOZZLE_2
	unsigned char Barcode[5];				//����ѹ��BCD�������F
	unsigned char BarMoney[3];				//������Ȩ���
	unsigned char InvoiceMark;				//��Ʊ��ӡ��ʶ
	unsigned char BarOilCode[2];			//������Ȩ��Ʒ
	unsigned char BarZDState;				//���������˵�״̬	0=������1=δ�ϴ�
	unsigned char CPOSFlag;					//����������̨ͨѶ��ʶ0=�޲�����1=�ѷ�������
	unsigned char CPOSOvertimes;		//����������̨ͨѶ��ʱ����
	unsigned int CPOSTimer;					//����������̨ͨѶ��ʱ��

	//������������ݣ���ip��Ϣ���ڲ�ѯ����ʱ��Ҫʹ�ù�Ҳ�����棬������ʵʱ��Ч����
	unsigned char TabletPanel;					//ƽ��������� PC_PANEL_1/PC_PANEL_2
	unsigned char TabletIP[16];					//ƽ�����IP��ַ
	unsigned char TabletMask[16];			//ƽ�������������
	unsigned char TabletGateway[16];		//ƽ���������
	unsigned char TabletFirstDNS[16];		//ƽ�������ѡDNS
	unsigned char TabletSecondDNS[16];	//ƽ����Ա���DNS
	unsigned char TabletFtpIP[16];			//ƽ��������ӵ�FTP������IP��ַ
	unsigned char TabletFtpPort[16];			//ƽ��������ӵ�FTP�������˿ں�
	unsigned char TabletTxElement;			//�������ϴ���ƽ�������Ϣ������
	unsigned char TabletGradeTag;			//���ֻ��Ƿ�������Ʒȷ�Ϲ���
	unsigned char IptOilLimitStyleTag;			//2017-02-13��Ʒ���Ʒ�ʽ����
		
	int DEVPlayButton;								//PLAY��ť�豸��
	int ValuePlayButton;								//PLAY��ť���� 0 =�޲�����1 = �а�ť����

	unsigned char PlayLongLast;				//PLAY��ťǰһ��״̬ 0 = ����״̬��1 = ̧��״̬��
	unsigned char PlayLong;						//PLAY��ť�Ƿ��а��µĲ��� 0 =�ޣ�1 = ��
	unsigned int PlayLongTimer;				//PLAY��ť���µĳ���ʱ��
	unsigned short TaState;							//״̬
	unsigned char TaStateParam[128];		//����
	int TaStateParamLength;						//��������

	//������Ϣ
	unsigned char ErrInfoBytes;				//������Ϣ����
	unsigned char ErrInfo[128];				//������Ϣ����

	//�����Ӧ����
	int DEVSensor;								//�����Ӧ����
	char SensorStateLast;					//ǰһ�������Ӧ״̬
	char SensorState;							//��ǰ�����Ӧ״̬

	//�ⲿ�����������
	unsigned char PassInputAsk;			//����ת����ȨETC������������� 0=������;1=֪ͨ�ͻ���ʼ��������;2=֪ͨ�ͻ�ȡ����������
	unsigned char AuthorizeAsk;			//����Լ��ͻ���Ȩ 0=������1=������Ȩ��2=����ȡ����Ȩ
	unsigned char AuthorizeData[64];	//��Ȩ����
	unsigned char CardDebitAsk;			//������ͻ����п�֧�� 0 = ������1 = ����֧�����̣�2 = �˳�֧�����̣�3 = ����֧����
	unsigned char CardDebitData[32];	//����֧��������; ��ˮ��(16bytes���󲹿ո�) + ���(3bytes��HEX)
	unsigned int CardDebitAmount;		//����֧���Ľ��

	unsigned char CardPresetAsk;			//������Ԥ������ 0xFF = �ޣ�1 = ��
	unsigned int CardPresetValue;			//������Ԥ�ö�
	unsigned char CardPresetMode;		//������Ԥ�÷�ʽ 0 = Ԥ��������1 = Ԥ�ý��
	unsigned char CardPresetRep;			//������Ԥ�������� 0xFF = �ޣ�0 = �ɹ���1 = ʧ�ܣ�
	
	unsigned char CardShootAsk;			//�Ƿ����˿����� 0xFF = �ޣ�1 = �У�
	unsigned char CardShootRep;			//�˿������� 0xFF = �޽����0 = �ɹ���1 = ʧ�ܣ�

	unsigned char CardAppSelectAsk;	//��Ӧ��ѡ������ 0xFF = �ޣ�0 = ������Ʊ��1 = ����Ӧ�ã�
	unsigned char CardAppSelectRep;	//��Ӧ��ѡ�������� 0xFF = �޽����0 = �ɹ��� 1 = ʧ�ܣ�

	unsigned char CardTypeSelectAsk;	//��֧����ʽѡ������ 0xFF = �ޣ�0 = �ֽ�1 = ��Ʊ��2 = ����ƾ֤��3 = ���п���4 = ����һ��5 = ������
	unsigned char CardTypeSelectRep;	//��֧����ʽѡ�������� 0xFF = �޽����0 = �ɹ��� 1 = ʧ�ܣ�

	unsigned char TMScanAsk;				//�����������0 = �޲�����1 = ��������ɨ�裻2 = �˳�����ɨ��
	unsigned char TMScanRep;				//���������������0xFF = �޽����0 = �ɹ���1= ʧ��
	unsigned char TMNumberLenght;	//����ϵͳ֪ͨ�ͻ��ⲿ���������λ�� HEX
	unsigned char TMNumber[10];			//����ϵͳ֪ͨ�ͻ��ⲿ���������, ASCII
	unsigned char TMExterInputDone;	//����ϵͳ֪ͨ�ͻ��ⲿ���������Ƿ���� 0 = δ�ꣻ1 = �������룻2 = ���

	//PCD����
	unsigned char PcdState;					//IPT��PCDͨѶ״̬0=ͨѶ�Ͽ���1=ͨѶ������ͨѶ����ʱPCD������Ч
	unsigned char PcdTxFlag;					//IPT��PCDͨѶ��ʶ0=��ռ�ã�1=�����ڲ������ѷ��ͣ�2=��ͨ�����ѷ���
	unsigned char PcdTxFrame;				//IPT��PCDͨѶ֡��
	unsigned int PcdTxTimer;					//IPT��PCDͨѶ��ʱ��ʱ��
	unsigned char PcdSaveOverTimes;	//IPT��PCDͨѶ��ʱ�������������˵��洢������
	unsigned char PcdOverTimes;			//IPT��PCDͨѶ��ʱ�����������ڹ�����
	unsigned char PcdPollOverTimes;	//IPT��PCDͨѶ��ʱ��������������ѯ������
	unsigned char BlistSrc;						//��ѯ��/������Դ0=ʹ�ú�̨��/��������1=ʹ���ͻ��ں�/������
	unsigned char PcOnline;					//ʯ����̨����״̬0=δ���ӣ�1=ͨѶ����
	unsigned char PcdErrNO;					//PCD�쳣״̬0=����������=�Ƿ�
	unsigned int BillTTC;							//������ǰ�˵�TTC
	unsigned int FuelBillTTC;					//��ǹ��ǰ�˵�TTC
	unsigned int UnloadNumber;			//����δ�ϴ��˵�����
	unsigned int FuelUnloadNumber;		//��ǹδ�ϴ��˵�����

	//���μ�������
	unsigned char YPSelect;					//�Ƿ�����Ʒȷ�ϲ��� 0 = �ޣ�1 = �У�
	unsigned char BarBillPrintRepeat;	//�����������������˵��Ƿ񻹿�ͨ����ݼ��ش�ӡ 0=�����ԣ�1=����
	unsigned char OilBillPrintRepeat;	//����IC�������˵��Ƿ񻹿�ͨ����ݼ��ش�ӡ 0=�����ԣ�1=����
	unsigned char OilBill[IPT_BILL_SIZE];//�����˵�
	unsigned char OilBillSave;					//�Ƿ����˵���Ҫ����:0=��;1=��
	unsigned char OilBillSaveStep;			//�˵��Ĵ洢����
	unsigned int OilDspTimer;					//����������ʾ�����ʱ��
	unsigned int WarnigBeepTimer;		//�����������������ʱ��

	unsigned int PriceChgTimer;				//���ݺ�̨��Ʒ���ʱ�����ʧ������һ��ʱ����ڽ�����һ�α�۳���
	unsigned char OilIcState[2];				//���Ͳ忨״̬,��"�ӿ�Э��1.1.pdf"�ļ���¼9
	unsigned char OilState;					//����״̬:0=����;1=������;2=������;3=������
	unsigned char OilTime[7];					//���ν�������ʱ��
	unsigned int PresetMode;					//Ԥ�÷�ʽ��0=���⣻1=Ԥ��������2=Ԥ�ý��
	unsigned long long PresetVolume;	//Ԥ������������λ0.01��������ʱ���ܻ��г���4�ֽڵ�������ʴ˴����ó�����
	unsigned long long PresetMoney;		//Ԥ��������λ0.01Ԫ������ʱ���ܻ��г���4�ֽڵ�������ʴ˴����ó�����
	unsigned long long SumMoney;		//���۽��
	unsigned long long SumVolume;		//��������
	unsigned int OilMoney;						//���μ��ͽ��
	unsigned int OilVolume;					//���μ�����
	unsigned int OilPrice;						//��Ʒ�۸�
	unsigned char OilRound;					//�Ƿ�����Ч��������0=�ޣ�1=��
	unsigned char JlStopNO;					//����ͣ������
	unsigned char IcValid;						//0=�޿���δ����1=�Ѷ������
	unsigned int OilEndTimer;					//�ϴμ��ͽ���ʱ����

	unsigned char ICStateFirst;				//������ʱ��״̬0=������0x01=�ҿ���0x10=TACδ��
	unsigned char DS;								//�ۿ���Դ��0=ʯ�Ϳ�������Ʊ��1=ʯ�ͻ��������֣�2=���ڿ�����Ǯ����3=���ڿ����Ӵ���
	unsigned char PayUnit;						//���㵥λѡ��0=������Ʊ��1=����
	unsigned char Payment;					//���㷽ʽ0=�ֽ�,1=��Ʊ,2=����,3=���п�,4=����1,	5=����2
	unsigned char C_TYPE;						//����b0:0=ʯ���淶����1=PBOC���ڿ�
	unsigned char UserElecFlag;				//�û���������Ƿ���ͨ��ȷ�ϼ��ͷž��� 0=δ������1=�Ѳ���

	//�������վ��ز���
	unsigned char StationName[30+1];	//����վ���ƣ����30�ֽڵ��ַ�������
	unsigned char InvoiceType;				//���μ��Ϳ�Ʊ���ͣ�01H�����ѿ���Ʊ��02H����ֵ����Ʊ��03H��ͳһ����ֵ˰��Ʊ��
																
	unsigned short PriceDiscount;			//�۸��ۿ۶�
	unsigned int OilMoneyDiscount;		//�ۿ۽����ۿۺ���ͽ��(���������ֶ��������洢)

	//�ǿ����ͼ�������͸��õ�����
	unsigned int MoneyUnself;				//���ηǿ����ͽ��������
	unsigned int VolumeUnself;				//���ηǿ������������������
	unsigned int MoneyUnselfSum;		//�ǿ������ۼƽ�������ۼƽ��
	unsigned int VolumeUnselfSum;		//�ǿ������ۼ�����������ۼ�����
	unsigned int OilStateUnself;				//�ǿ�����״̬0=���У�1=������
	unsigned char OilOverTimes;			//����������ʹ���

	//IC����Ϣ
	unsigned char DEVIC;						//IC���豸��0=1�ſ�����1=2�ſ���
	unsigned char IcOverTimes;				//IC��������ʱ����
	IcStateType IcState;						//IC��״̬
	unsigned char IcPassword[12];		//IC������(ASCII�����12λ)
	unsigned char IcPasswordLen;		//IC�����������ֽڳ���(0~12)
	unsigned char DEVPsam;					//PSAM�������� '0'~'4'��ʾ������PSAM1~PSAM4��0x00~0x01��ʾ���̵�PSAM0~PSAM1

	//����ʱ������Ϣ
	unsigned int unlockMoney;				//���ۣ����
	unsigned char unlockCTC[2];			//���ۣ�CTC
	//unsigned char unlockDS;				//���ۣ��ۿ���Դ
	unsigned char unlockGMAC[4];			//���ۣ������֤��
	unsigned char unlockPsamTID[6];	//���ۣ�PSAMӦ�ñ��
	unsigned char unlockPsamTTC[4];	//���ۣ�PSAM��TTC


	//ACT����֤�������̲���
	//ACT������
	unsigned char ACTAppId[10];						//ACT������
	//__ACT��26�ļ��ֿ��˻��������ļ�
	unsigned char ACTKeyIndex;						//��֤��Կ������(ACT������)
	unsigned char ACTUnused;							//����
	unsigned char ACTUserName[20];				//�ֿ�����
	unsigned char ACTUserID[18];						//�ֿ���֤������
	unsigned char ACTUserIDType;					//�ֿ���֤������
	//__PSAM�����
	unsigned char ACTPSAMRandom[4];			//PSAM�����
	//__ACT����������
	unsigned char ACTCiphertext[8];					//��������

	//RID����֤�������̲���
	//__RID��26�ļ��ֿ��˻��������ļ�
	unsigned char RIDKeyIndex;						//��֤��Կ������(RID������)
	unsigned char RIDCalKeyIndex;					//������Կ������( ��־MAC����)
	unsigned char RIDUserName[20];				//�ֿ�����
	unsigned char RIDUserID[18];						//�ֿ���֤������
	unsigned char RIDUserIDType;					//�ֿ���֤������
	//__PSAM�����
	unsigned char RIDPSAMRandom[4];			//PSAM�����
	//__RID����������
	unsigned char RIDCiphertext[8];					//��������
	//__RID��֤MAC
	unsigned char RIDMAC[4];				

	//IC���������ѹ��̲���
	//__PSAM��21�ļ���Ƭ������Ϣ
	unsigned char PsamId[10];							//PSAM���к�
	unsigned char PsamVersion;							//PSAM�汾��
	unsigned char PsamKeyType;						//PSAM��Կ������
	unsigned char PsamCodeVersion;				//PSAMָ��汾
	unsigned char PsamFCI;								//PSAM�������Զ���FCI����
	//__PSAM��22�ļ�MF�ն���Ϣ(SELECT  PSE)
	unsigned char PsamTermId[6];						//PSAM�ն˻����
	//__PSAM��23�ļ�Ӧ�ù�����Ϣ(SELECT  ADF)
	unsigned char PsamKeyIndex;						//PSAM������Կ������
	unsigned char PsamIssuerMark[8];				//PSAMӦ�÷�������ʶ
	unsigned char PsamRecipientsMark[8];		//PSAMӦ�ý����߱�ʶ
	unsigned char PsamAppStartTime[4];			//PSAMӦ����������
	unsigned char PsamAppEndTime[4];			//PSAMӦ����Ч��ֹ����
	//__IC��21�ļ�����Ӧ�û�������
	unsigned char IcIssuerMark[8];					//��������ʶ
	unsigned char IcAppMatk;								//Ӧ�����ͱ�ʶ
	unsigned char IcAppVersion;						//Ӧ�ð汾
	unsigned char IcAppId[10];							//Ӧ�����к�
	unsigned char IcEnableTime[4];					//Ӧ����������
	unsigned char IcInvalidTime[4];					//Ӧ����Ч��ֹ����
	unsigned char IcCodeVersion;						//ָ��汾
	unsigned char IcFile21Unused;					//21�ļ���������
	//__IC��22�ļ��ֿ��˻�������
	unsigned char IcTypeMark;							//�����ͱ�ʶ
	unsigned char IcStaffMark;							//��ϵͳְ����ʶ
	unsigned char IcUserName[20];					//�ֿ�����
	unsigned char IcUserIdeId[18];					//�ֿ���֤��(identity)����(ASCII)
	unsigned char IcUserIdeType;						//�ֿ���֤������
	//__IC��27�ļ�ET��ͨ��Ϣ����
	unsigned char IcDefaultPassword;				//�Ƿ����Ĭ������,00=ʹ��Ĭ�����룬01=ʹ���û�����
	unsigned char IcStaffId;								//Ա����(�ڲ�����Ч)
	unsigned char IcStaffPassword[2];				//Ա������(�ڲ�����Ч)
	unsigned char IcDebitUnit;							//�ۿλ(00H=Ԫ��01H=�������վCPU���ṹ�д��ֶ�)
	unsigned char IcDiscountFlag;						//�Ƿ��ۿۿ�(00H=���ۿۿ���01H=�ۿۿ������վCPU���ṹ�д��ֶ�)
	unsigned char IcAppType;								//��ƬӦ��������(01H = �û�����04H = Ա������05H = ��ÿ���21H = �û���������
																            //22H = ������������11H = �û���˾������12H = ���˿���13H = ��������
																			//14H = ���ÿ���15H = ��������16H = ������17H = ������˾������
																			//���վCPU���ṹ�д��ֶ�)
	//__IC��28�ļ�������Ϣ����
	unsigned char IcOilLimit[2];							//��Ʒ����
	unsigned char IcRegionTypeLimit;				//�޵���,��վ���ͷ�ʽ
	unsigned char IcRegionLimit[40];				//�޵���,��վ����
	unsigned char IcVolumeLimit[2];					//��ÿ�μ�����
	unsigned char IcTimesLimit;							//��ÿ����ʹ���
	unsigned char IcMoneyDayLimit[4];			//��ÿ����ͽ��
	unsigned char IcCarIdLimit[16];					//���ƺ�����(ASCII)
	//__IC����ʷ������ϸ				
	unsigned char IcRecordNumber;					//IC������ϸ��Ŀ
	IptIcRecordType IcRecord[10];						//������ϸ�����飬�ʴ洢�ĵ�һ�ʼ�Ϊ����һ�ʼ�������
	//__IC��������Ϣ
	unsigned char IcLockMark;							//״̬��:0x00=�޻�����0x01=�ѻ�����0x10=TACδ��
	unsigned char IcLockType;						//
																		//	0x00:�ϴη�����ۻ�������۵Ľ������ͱ�ʶ��
																		//	0x01:�����Ľ������ͱ�ʶ��
																		//	0x10:�ϴν�۵Ľ������ͱ�ʶ��
	unsigned char IcLockET;								//
																		//	0x00:�ϴη�����ۻ�������۵�ET��
																		//	0x01:��������ET��
																		//	0x10:�ϴν�۵�ET��
	unsigned char IcLockBalance[4];				//
																		//	0x00:�ϴη�����ۻ�������۵�ET��
																		//	0x01:��������ET��
																		//	0x10:�ϴν�۵�ET��
	unsigned char IcLockCTC[2];						//
																		//	0x00:�ϴη�����۵�ET���ѻ��������
																		//			��������۵�ET������������ţ�
																		//	0x01:��������ET�ѻ�������ţ�
																		//	0x10:�ϴν�۵�ET�ѻ�������ţ�
	unsigned char IcLockTermId[6];				//
																		//	0x00:�ϴη�����ۻ�������۵��ն˻���ţ�
																		//	0x01:ִ��GREY LOCKʱ���ն˻���ţ�
																		//	0x10:�ϴν�۵��ն˻���ţ�
	unsigned char IcLockTime[7];					//
																		//	0x00:�ϴη�����ۻ�������۵�����ʱ�䣻
																		//	0x01:ִ��GREY LOCKʱ������ʱ�䣻
																		//	0x10:�ϴν�۵�����ʱ�䣻
	unsigned char IcLockMoney[4];				//
																		//	0x00:�ϴη�����ۻ�������۵Ľ��׽�
																		//	0x01:����ʱ��MAC2��
																		//	0x10:�ϴν�۵Ľ��׽�
	unsigned char IcLockGTAC[4];					//
																		//	0x00:�ϴη�����۵�TAC��������۵�MAC3��
																		//	0x01:����ʱ��GTAC��
																		//	0x10:�ϴν�۵�TAC��
	//__IC�������Ϣ
	unsigned char IcBalance[4];							//�������Ϣ
	//__PSAM��ȫ����ʣ�����
	unsigned char PsamProofTimes;					//PSAMʣ����֤����
	//__PSAM�����ͨ����DES���������
	unsigned char DESCiphertext[8];					//DES��������
	//__IC��������ʼ����Ϣ
	unsigned char IcLockInitBalance[4];			//������ʼ������:�����
	unsigned char IcLockInitCTC[2];					//������ʼ������:�ѻ��������
	unsigned char IcLockInitOverdraw[3];		//������ʼ������:͸֧�޶�
	unsigned char IcLockInitKeysVersion;		//������ʼ������:��Կ�汾��
	unsigned char IcLockInitArithmetic;			//������ʼ������:�㷨��ʶ
	unsigned char IcLockInitRandom[4];			//������ʼ������:α�����
	//__PSAM����MAC1
	unsigned char IcPsamTTC[4];						//PSAM����:�ն˽������
	unsigned char IcPsamRandom[4];				//PSAM����:�ն������
	unsigned char IcPsamMAC1[4];					//PSAM����:MAC1
	//__IC������
	unsigned char IcGTAC[4];								//IC��������:GTAC
	unsigned char IcMAC2[4];								//IC��������:MAC2
	//__���͹���
	//__PSAM����GMAC
	unsigned char IcPsamGMAC[4];					//PSAM����:GMAC
	unsigned char IcPsamTAC[4];						//PSAM����:TAC
	//__IC����۷�������
	unsigned char IcTac[4];									//IC��۷���:TAC

	//ETC���ѹ�������,szb_fj20171120
	unsigned char etc_set_flg;						//ETC��������,szb_fj20171120
	unsigned int etc_rec_len;						//���յ������ݳ���,szb_fj20171120
	//unsigned char etc_rec_buff[ETC_DATA_MAX];		//���յ������ݻ���,szb_fj20171120
	unsigned char etc_rec_buff[600];                //fj20171120
	unsigned char EtcOilFlg;						//ETC���ͱ�־,szb_fj20171120
	//unsigned char EtcListInf[ETCCARDLEN*20];		//ETC�����б���Ϣ,szb_fj20171120
    unsigned char EtcListInf[26*20];                 //fj20171120
	//16�ֽڵĳ��ƺţ�4�ֽڵ�MAC��2�ֽڵ��������1�ֽڵ������־,szb_fj20171120
	unsigned char EtcListNum;						//������Ϣ����,szb_fj20171120
	//unsigned char EtcSelCardInf[ETCCARDLEN];		//ѡ���ĳ�����Ϣ,szb_fj20171120
	unsigned char EtcSelCardInf[26];
	unsigned char EtcTxCi;							//ETC���ʹ���,szb_fj20171120
	unsigned char EtcFreeflag;						//OBUѡ����־,szb_fj20171120
	//unsigned char etc_09_buff[ETC_CARD_MAX];		//����Ϣ����,szb_fj20171120
	unsigned char etc_09_buff[1024];
	unsigned int etc_09_len;						//����Ϣ����,szb_fj20171120
	unsigned char etc_09_num;						//����,szb_fj20171120
	unsigned char etc_card_num;						//etc����Ϣ�����,szb_fj20171120
	unsigned char etc_touming_flg;					//�Ƿ����͸�������־,szb_fj20171120
	unsigned char etc_limit_car;					//ETC�޳��ű�־,szb_fj20171120
	unsigned char etc_recnet_oil[10];				//���ϴμӵ���Ʒ������ʾ,szb_fj20171120
	unsigned char etc_now_oil[10];					//�����ڼӵ���Ʒ������ʾ,szb_fj20171120
	unsigned char EtcTxFlg;							//ETC���ͱ�־,szb_fj20171120
	unsigned char etc_update_flag;					//������Ʒ��־,szb_fj20171120
	unsigned int EtcTxTime;							//ETC����ʱ��,szb_fj20171120
	unsigned char etc_yue_dis_flag;					//�����ʾ��־,szb_fj20171120
	unsigned char EtcFreeObuflg;					//�ͷ�OBU��־,szb_fj20171120
	unsigned char EtcFreeObuCi;						//�ͷ�OBU����,szb_fj20171120
	unsigned int EtcFreeObuTime;					//�ͷ�OBUʱ��,szb_fj20171120

	unsigned char JlErr_QYL;						//�쳣ͣ��ȱһ·����,szb_fj20171120
	unsigned char JlErr_ZERO;						//�쳣ͣ�������,szb_fj20171120
	unsigned char JlErr_WMCCS;						//�쳣ͣ�������峬ʱ,szb_fj20171120
	unsigned char JlErr_QYZ;						//�쳣ͣ��ȱһ������,szb_fj20171120
	unsigned char JlErr_Freebuff[3];				//�����������,szb_fj20171120

	unsigned char JlErr_BianJia;					//��������쳣��־,szb_fj20171120
}IptParamStructType;

//��ǰIPTģ���ȡ����һЩ״̬��������
extern IptParamStructType IptParamA, IptParamB;
//205��IAP��Ӧ�ó���汾
extern unsigned char Ipt205IAP_Ver[2];
extern unsigned char Ipt205APP_Ver[2];

//PC��Ϣ
extern PcdPcInfoType IptPcInfo;

//��������
//extern int iptPidPrint(void);   //fj:û�е���,20170914,����ɾ��
//extern int iptMainUserGet(void);//fj:���ú���
//extern int iptPanelFromUserID(int userid); //fj:�޵��ú���

extern int iptPrinterRead(int nozzle);
extern int iptSpk(const IptParamStructType *iptparam, int *list, int number);
extern int iptOilVoiceIdGet(const IptParamStructType *iptparam, const char *oil_code);
extern int iptTMACCalculate(IptParamStructType *iptparam, unsigned char *TMAC, unsigned char *apdu_return, unsigned char *inbuffer, int nbytes);
extern void iptMainInterface(IptParamStructType *iptparam);
extern void iptPidSet(IptParamStructType *iptparam, unsigned short pid);

extern int iptHexVoiceIdGet(const IptParamStructType *iptparam, unsigned int data, int *voice);

extern int iptCardPayForAsk(int panel, char *inbuffer);
extern int iptCardPayFor(int panel, char *inbuffer);
extern int iptAuthETCPassInput(int panel, unsigned char iHandle);
extern int iptAuthorize(int panel, unsigned char iHandle, char *inbuffer);

extern int iptCardDebit(int panel, char *inbuffer);
extern int iptExterSuspendDo(int panel, int handle, char *ibuffer);
extern int iptExterPreset(int panel, char *inbuffer);
extern int iptExterCardAppSet(int panel, char *inbuffer);
extern int iptExterCardPaymentSet(int panel, char *inbuffer);
extern int iptExterCardShoot(int panel);
extern int iptExterScanHandle(int panel, int handle);
extern int iptExterBarcodeInput(int panel, int handle, char *inbuffer);


extern int iptMainUserSet(int user);  //fj:�ڲ�����


extern int iptPhysicalNozzleRead(int nozzle);


//extern MSG_Q_ID iptMsgIdRead(int nozzle);
extern int iptMsgIdRead(int nozzle);   //fj:20170914

extern int iptPhysicalNozzleRead(int nozzle);
extern bool iptInit(void);
extern int iptIsLianda(int nozzle); //2017-01-22��ȼ����ҿ����

void iptTask();

//������������6���쳣ͣ������
extern int iptAbnormalStopHandle(IptParamStructType *iptparam,unsigned char stopcode,unsigned int money);

#endif


