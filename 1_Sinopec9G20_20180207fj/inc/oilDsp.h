#ifndef _OIL_DSP_H_
#define _OIL_DSP_H_

//��ʾһ��������󳤶�
#define DSP_LEN_MAX			512

//��ʾ�豸
#define DSP_DEV_KEYBOARD		0			//�������
#define DSP_DEV_PCMONITOR		1			//ƽ�����

//��ʾ�豸ǹ��
#define DSP_NOZZLE_1			0			//1����ʾ�豸
#define DSP_NOZZLE_2			1			//2����ʾ�豸

//���̵�������ʾ����궨��
#define DSP_TEXT_INFO					101		//�ı���ʾ����
#define DSP_CARD_STANDBY				102		//���Ϳ����ʹ�������
#define DSP_CARD_PRETREAT				103		//���Ϳ�Ԥ������ʾ����
#define DSP_CARD_UNLOCK_FINISH		    104		//���Ϳ����۴�����ʾ����
#define DSP_CARD_TACCLR_FINISH			105		//���Ϳ����䴦����ʾ����
#define DSP_CARD_PASSIN					106		//���Ϳ������������
#define DSP_CARD_CARLIMIT				107		//�޳��ſ�Ա�������������
#define DSP_CARD_BALANCE				108		//���Ϳ�������
#define DSP_CARD_PRESET					109		//���Ϳ�����Ԥ�ý���
#define DSP_CARD_UNIT_SELECT			110		//���Ϳ����㵥λѡ�����
#define DSP_CARD_SETTLE_SELECT			111		//���Ϳ����㷽ʽѡ�����
#define DSP_CARD_STAFF_PASSIN			112		//���Ϳ�����Ա�������������
#define DSP_CARD_OIL_START				113		//���Ϳ�����������ʾ����
#define DSP_CARD_OILLING				114		//���Ϳ������н���
#define DSP_CARD_OIL_FINISH				115		//���Ϳ����ͽ�����ʾ����
#define DSP_CARD_OILEND					116		//���Ϳ�������ɽ���
#define DSP_CARD_ERR_INFO				117		//���Ϳ����ͺ�����Ĵ�����ʾ����
#define DSP_PASSWORD_INPUT				118		//�����������
#define DSP_OPERATE_SELECT				119		//������ѡ�����
#define DSP_INQ_SELECT					120		//��ѯ��ѡ�����
#define DSP_INQ_JLSUM					121		//��ѯ�������۽���
#define DSP_INQ_BILL_SELECT				122		//��ѯ�˵���ϸ��������ѡ�����					
#define DSP_INQ_BILL_TTCINPUT			123		//��ѯ�˵���ϸTTC�������
#define DSP_INQ_BILL_INDEX				124		//��ѯ�˵���ϸ��������
#define DSP_INQ_BILL_DATA				125		//��ѯ�˵���ϸԭʼ���ݽ���
#define DSP_INQ_NOZZLE_INFO				126		//��ѯ��ǹ��Ϣ����
#define DSP_INQ_BOARD_INFO				127		//��ѯ������Ϣ����
#define DSP_INQ_TIME					128		//��ѯʱ�����
#define DSP_INQ_VOICE					129		//��ѯ������Ϣ����
#define DSP_INQ_PRINT					130		//��ѯ��ӡ��Ϣ����
#define DSP_INQ_PRINT_CARD			    131		//��ѯ��ӡ��Ϣ������ѡ�����
#define DSP_INQ_PRINT_BILLTYPE			132		//��ѯ��ӡ��Ϣ�˵�����ѡ�����
#define DSP_MONTH_INPUT					133		//�·��������
#define DSP_DATE_INPUT					134		//�����������
#define DSP_INQ_LIMIT_INFO				135		//��ѯ������Ϣ����
#define DSP_INQ_ADVANCE_INFO			136		//��ѯ��ǰ������
#define DSP_INQ_UNPULSE_TIME			137		//��ѯ������ͣ����ʱʱ�����
#define DSP_INQ_BIND_INFO				138		//��ѯ����Ϣ����
#define DSP_INQ_VERSION_INFO			139		//��ѯ�汾��Ϣ����
#define DSP_SET_SELECT					140		//������ѡ�����
#define DSP_SET_PRICE					141		//���õ��۽���
#define DSP_SET_TIME					142		//����ʱ�����
#define DSP_SET_BACKLIT					143		//���ñ������
#define DSP_SET_NIGHTLOCK				144		//����ҹ����������
#define DSP_SET_PASSWORD_OLD			145		//�������룬���������
#define DSP_SET_PASSWORD_NEW			146		//�������룬����������
#define DSP_SET_PASSWORD_ACK			147		//�������룬����������ȷ��
#define DSP_SET_PHYSICAL_NOZZLE		    148		//��������ǹ��
#define DSP_SET_TAX_TIME				149		//����˰��ʱ��
#define DSP_SET_SPEAKER					150		//����������
#define DSP_SET_VOICE_VOLUME			151		//������������
#define DSP_SET_VOICE_TYPE				152		//������������
#define DSP_SET_PRINTER					153		//���ô�ӡ��
#define DSP_SET_PRINT_AUTO				154		//�����Ƿ��Զ���ӡ
#define DSP_SET_PRINT_UNION				155		//���ô�ӡ����
#define DSP_SET_PRINT_CARD				156		//�����Զ���ӡ������ѡ�����
#define DSP_SET_PRINT_BILLTYPE			157		//�����Զ���ӡ�˵����ͽ���
#define DSP_SET_ADVANCE					158		//������ǰ������
#define DSP_SET_UNPULSE_TIME			159		//���������峬ʱͣ��ʱ�����
#define DSP_SET_STAFF_LIMIT				160		//����Ա�����������ƽ���
#define DSP_SET_MODE					161		//����ģʽ����
#define DSP_OTHER_OPERATE_PASSSIN	    162		//�������������������
#define DSP_OTHER_OPERATE				163		//������������
#define DSP_INQ_JLTYPE					164		//����������ʾ����
#define DSP_INQ_SHILED					165		//������������������������ʾ����
#define DSP_INQ_EQUIVALENT				166		//��ʾ������������
#define DSP_INQ_VALVE_VOLUME			167		//��ʾ�󷧴�ʱ�������
#define DSP_UNSELF_STANDBY				168		//�ǿ���������������
#define DSP_UNSELF_PRESET				169		//�ǿ�������Ԥ�ý���
#define DSP_UNSELF_OILLING				170		//�ǿ������������н���
#define DSP_INQ_STATION_INFO1			171		//��վͨ����Ϣ���棬��һ����
#define DSP_INQ_STATION_INFO2			172		//��վͨ����Ϣ���棬�ڶ�����
#define DSP_INQ_OILINFO					173		//��Ʒ�ͼ۱���Ϣ����
#define DSP_INQ_BASELIST				174		//������������Ϣ����
#define DSP_INQ_ADDLIST					175		//������������Ϣ����
#define DSP_INQ_DELLIST					176		//��ɾ��������Ϣ����
#define DSP_INQ_WHITELIST				177		//��������Ϣ����
#define DSP_OIL_OVER_INFO				178		//���������Ϣ����
#define DSP_OIL_OVER_STAFFIN			179		//�������Ա�������������
#define DSP_CARD_TEST					180		//����������,Э��Ȳ��Խ���
#define DSP_CARD_PROTOCOL_TIME		    181		//����Э������������ѯʱ�����ý���

 
#define DSP_TM_SCAN						182		//��ɨ��������������ʾ����
#define DSP_TM_BARCODE_INPUT			183		//�����������
#define DSP_TM_AUTHORIZING				184		//������Ȩ�����н���
#define DSP_TM_AUTHORIZE				185		//��Ȩ��������ʾ����
#define DSP_TM_AUTHORIZE_CANCEL		    186		//������Ȩ������
#define DSP_TM_OILSTART					187		//������������������
#define DSP_TM_OILLING					188		//��������������
#define DSP_TM_OILFINISH				189		//�����������ͽ�����
#define DSP_TM_OIL_FINAL				190		//�����������ͽ����ʾ����

#define DSP_INQ_UNPULSE_OVERTIME	    191		//�����峬ʱ�ش�ʱ�����
#define DSP_INQ_ISAUTHEN				192		//�����Ƿ���ҪDES��֤�Ľ���
#define DSP_INQ_OILVOICE				193		//��Ʒ������ѯ����
#define DSP_SET_OILVOICE_SELECT		    194		//��Ʒ��������ѡ�����
#define DSP_SET_OILVOICE				195		//��Ʒ����ѡ�����
#define DSP_INQ_OILERRLOG				196		//��ѯ������־����
#define DSP_INQ_BARGUNNUMBER			197		//����ǹ����ѯ����	
#define DSP_INQ_BARBRAND				198		//����Ʒ�Ʋ�ѯ����
#define DSP_SET_NOZZLE_NUMBER			199		//����ǹ�����ý���
#define DSP_SET_BARBRAND				200		//����Ʒ�����ý���
#define DSP_LOCAL_NETINFO				201		//����������Ϣ��ѡ�����
#define DSP_LOCAL_IP					202		//����IP��ַ����
#define DSP_LOCAL_MASK					203		//�����������
#define DSP_LOCAL_GATEWAY			    204		//����Ĭ�����ؽ���
#define DSP_LOCAL_UNUSED				205		//����
#define DSP_BACKSTAGE_INFO				206		//ʯ����̨������Ϣ
#define DSP_BACKSTAGE_IP				207		//ʯ����̨IP��ַ
#define DSP_BACKSTAGE_PORT				208		//ʯ����̨ͨѶ��̨�������˿ں���Ϣ
#define DSP_BACKSTAGE_LOCAL_PORT	    209		//ʯ����̨ͨѶ���ط������˿ں���Ϣ
#define DSP_BACKSTAGE_UNUSED2			210		//����
#define DSP_TABLETPC_INFO				211		//ƽ�����������Ϣ��ѡ�����
#define DSP_TABLETPC_IP					212		//ƽ�����IP��Ϣ����
#define DSP_TABLETPC_MASK				213		//ƽ�����������Ϣ����
#define DSP_TABLETPC_GATEWAY			214		//ƽ�����������Ϣ����
#define DSP_TABLETPC_FIRSTDNS			215		//ƽ�������ѡDNS��Ϣ����
#define DSP_TABLETPC_SECONDDNS		    216		//ƽ����Ա���DNS��Ϣ����
#define DSP_TABLETPC_FTP_IP				217		//ƽ�����FTP��ַ��Ϣ����
#define DSP_TABLETPC_FTP_PORT			218		//ƽ�����FTP�˿ں���Ϣ����
#define DSP_TABLETPC_SERVERIP			219		//ƽ��������ӵĺ�̨IP��ַ��Ϣ����
#define DSP_TABLETPC_VOLUME				220		//ƽ�����������Ϣ����
#define DSP_TABLETPC_TELE_IP			221		//ƽ����������Խ���̨IP��ַ��Ϣ����
#define DSP_TABLETPC_UNUSED2			222		//����
#define DSP_CONNECT_TYPE_SET			223		//��̨���ӷ�ʽ���ý���
#define DSP_CONNECT_TYPE_DSP			224		//��̨���ӷ�ʽ��ѯ����
#define DSP_CONTRAST					225		//��ʾ�Աȶ�
#define DSP_DICOUNT_FAIL_KEEP			226		//�����ۿ�ʧ�ܺ��˹�ȷ�Ͻ���
#define DSP_MODEL						227		//���Ͳ�������
#define DSP_AUTH_BALANCE				228		//��Ȩ����������
#define DSP_CARD_DEBIT					229		//�������ѿۿ����
#define DSP_PROMOTION					230		//�Ƿ����ô�������
#define DSP_TM_SCAN_AND_INPUT			231		//����ɨ�輰�������
#define DSP_BANK_PRESET					232		//���п�Ԥ�ý���
#define DSP_BANK_PIN_INPUT				233		//���п������������
#define DSP_BANK_AUTH_REGUEST			234		//���п�Ԥ��Ȩ
#define DSP_BANK_AUTH_RESULT			235		//���п�Ԥ��Ȩ�ɹ�����
#define DSP_BANK_OILLING				236		//���п������н���
#define DSP_YuLe_Grade_Fun				237		//�Ƿ�������Ʒȷ�Ϲ���
#define DSP_ETC_FUN_COUR                238     //ETC�������ý���,szb_fj20171120
#define DSP_ETC_PIN_INPUT               239     //ETC��֤������,szb_fj20171120
#define DSP_CAR_NOYIZHI                 240     //ETC���ƺŲ�һ��,szb_fj20171120
#define DSP_ETC_OILCR                   241     //ETC��Ʒ��һ��ȷ��,szb_fj20171120

//��ʾ����ʱ�������
extern unsigned int dspTimerA, dspTimerB;

//������Ƶ
extern int dspVideo(int nozzle);
//��ʾ����ӿں���
extern int dsp(int nozzle, int id, unsigned char *buffer, int nbytes);
//��ʾģ�鹦�ܳ�ʼ��
extern int dspInit(void);

//void tdspRecive(int nozzle);
//void tdsp(int nozzle);
void tdspRecive(void* pNozzleType);
void tdsp(void* pNozzleType);

#endif

