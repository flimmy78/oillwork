#ifndef	_OIL_PARAM_H_
#define _OIL_PARAM_H_

//#include "yaffs22/yaffsfs.h"

/*����汾��*/
#define _SOFTWARE_VERSION_			"V1.00.27"
/*��֪ƽ��ļ��ͻ�����汾�ţ�Ӧ��_SOFTWARE_VERSION_ͬ��*/
#define PC_OIL_SOFTVERSION			"\x01\x00\x27"

/*�ͻ���ƽ��ͨѶЭ��汾��*/
#define PC_SOFTVERSION					"\x00\x28"



/*���Ͷ���*/
extern const char *ModelParam[];
#define MODEL_SINOPEC							0x0000	/*ʯ�������������ͻ�*/
#define MODEL_PRIVATE							0x0001	/*���վ���ͻ�*/
#define MODEL_LIANDA							0x0002	/*����ϵͳ���ͻ�*/
#define MODEL_PARAM_MIN					MODEL_SINOPEC		/*��Сֵ*/
#define MODEL_PARAM_MAX					MODEL_LIANDA		/*���ֵ*/

/*����ǹ��*/
#define PANEL_NOZZLE_SINGLE			0	/*���浥ǹ-˫ǹ��*/
#define PANEL_NOZZLE_DOUBLE  		1	/*����˫ǹ-��ǹ��*/

/*�ļ�·��*/  //fj:20170905
#define FILE_SETUP						"../config/mboardFiles/OilSetup.txt"				/*���������ļ�*/
#define FILE_ERROR_INFO					"../config/mboardFiles/ErrorInfo.txt"			/*������Ϣ�ļ�*/
#define FILE_RECORD						"../config/mboardFiles/Record.txt"				/*��¼��Ϣ�ļ�*/

/*����(JL)�������ļ��ڵ�ƫ�ƣ����������30�ֽ�*/
#define JL0_EQUIVALENT					(0*64)			/*(A1ǹ)����,4HEX*/
#define JL0_UNPULSE_TIME				(1*64)			/*(A1ǹ)�����峬ʱʱ��,4HEX*/
#define JL0_ADVANCE						(2*64)			/*(A1ǹ)��ǰ��,4HEX*/
#define JL0_SHIELD							(3*64)			/*(A1ǹ)������,4HEX*/
#define JL0_OVER_SHIELD				(4*64)			/*(A1ǹ)����������,4HEX*/
#define JL0_TYPE								(5*64)			/*(A1ǹ)��������,4HEX*/
#define JL0_PRICE								(6*64)			/*(A1ǹ)��������,4HEX*/
#define JL0_VAVLE_VOLUME				(7*64)			/*(A1ǹ)������С�ڴ�����ʱ������,4HEX*/
#define JL0_VAVLE_STOP					(8*64)			/*(A1ǹ)����������ʱ�䳬����ֵʱ�رմ�,4HEX*/
#define JL0_ALGORITHM					(9*64)			/*(A1ǹ)�����������㷨��ʶ,4HEX*/
#define JL0_BIG_VOL_TIME					(10*64)	/*(A1ǹ)˫�����������������峬ʱʱ�����1BCD ��5-20Ĭ��8*/
#define JL0_BIG_VOL_SPEED					(11*64)	/*(A1ǹ)˫���������������ٿ���1BCD L/��10-80Ĭ��15*/


#define JL1_EQUIVALENT					(20*64)		/*(B1ǹ)����,4HEX*/
#define JL1_UNPULSE_TIME				(21*64)		/*(B1ǹ)�����峬ʱʱ��,4HEX*/
#define JL1_ADVANCE						(22*64)		/*(B1ǹ)��ǰ��,4HEX*/
#define JL1_SHIELD							(23*64)		/*(B1ǹ)������,4HEX*/
#define JL1_OVER_SHIELD				(24*64	)		/*(B1ǹ)����������,4HEX*/
#define JL1_TYPE								(25*64)		/*(B1ǹ)��������,4HEX*/
#define JL1_PRICE								(26*64)		/*(B1ǹ)��������,4HEX*/
#define JL1_VAVLE_VOLUME				(27*64)		/*(B1ǹ)�����ڲ�С�ڴ�����ʱ������,4HEX*/
#define JL1_VAVLE_STOP					(28*64)		/*(B1ǹ)����������ʱ�䳬����ֵʱ�رմ�,4HEX*/
#define JL1_ALGORITHM					(29*64)		/*(B1ǹ)�����������㷨��ʶ,4HEX*/
#define JL1_BIG_VOL_TIME					(30*64)	/*(B1ǹ)˫�����������������峬ʱʱ�����1BCD ��5-20Ĭ��8*/
#define JL1_BIG_VOL_SPEED					(31*64)	/*(B1ǹ)˫���������������ٿ���1BCD L/��10-80Ĭ��15*/


/*��ʯ��Ӧ�ò���IPT��PCD�������壬���������30�ֽ�*/
#define IPT0_DUTY_INFO					(40*64)		/*(A1ǹ)��/�°���Ϣ����/�°���Ϣ1bytes+Ա����1bytes+Ա������(2bytes)+Ա������(10bytes)*/
#define IPT0_VOICE_SPEAKER			(41*64)		/*(A1ǹ)������ѡ��*/
#define IPT0_VOICE_TYPE					(42*64)		/*(A1ǹ)��������*/
#define IPT0_VOICE_VOLUME			(43*64)		/*(A1ǹ)����*/
#define IPT0_PRINTER						(44*64)		/*(A1ǹ)��ӡ��*/
#define IPT0_PRINT_AUTO				(45*64)		/*(A1ǹ)�Զ���ӡ*/
#define IPT0_PRINT_UNION				(46*64)		/*(A1ǹ)��ӡ����*/
#define IPT0_PRN_CARD_USER		(47*64)		/*(A1ǹ)�û����˵��Զ���ӡ����,2HEX*/
#define IPT0_PRN_CARD_MANAGE	(48*64)		/*(A1ǹ)�����˵��Զ���ӡ����,2HEX*/
#define IPT0_PRN_CARD_STAFF		(49*64)		/*(A1ǹ)Ա�����˵��Զ���ӡ����,2HEX*/
#define IPT0_PRN_CARD_PUMP		(50*64)		/*(A1ǹ)��ÿ��˵��Զ���ӡ����,2HEX*/
#define IPT0_PRN_CARD_SERVICE	(51*64)		/*(A1ǹ)ά�޿��˵��Զ���ӡ����,2HEX*/
#define IPT0_NIGHT_LOCK				(52*64)		/*(A1ǹ)ҹ������*/
#define IPT0_LOGIC_NOZZLE			(53*64)		/*(A1ǹ)�߼�ǹ��*/
#define IPT0_PHYSICAL_NOZZLE		(54*64)		/*(A1ǹ)����ǹ��*/
#define IPT0_PASSWORD					(55*64)		/*(A1ǹ)��վ����Ա���룬2BCD*/
#define IPT0_WORKMODE					(56*64	)		/*(A1ǹ)����ģʽ,1HEX*/
#define IPT0_SERVICE_PASS			(57*64)		/*(A1ǹ)ά������,2BCD*/
#define IPT0_AUTHEN						(58*64)		/*(A1ǹ)��֤��ʽ��1HEX*/
#define IPT0_PRICE_INFO					(59*64)		/*(A1ǹ)��Ʒ�ͼ���Ϣ:��Ʒ����(2�ֽ�BCD)+��Ʒ�۸�(2�ֽ�HEX)*/
#define IPT0_STAFF_LIMIT				(60*64)		/*(A1ǹ)Ա��������������Ϣ,1HEX*/
#define IPT0_BIND_TIME					(61*64)		/*(A1ǹ)����Ϣ��ʱ�䣬7BCD*/
#define IPT0_BIND_MBOARD_ID		(62*64)		/*(A1ǹ)����Ϣ������ţ�8BCD*/
#define IPT0_BIND_ACT_APPID		(63*64)		/*(A1ǹ)����Ϣ��ACT���ţ�10BCD*/
#define IPT0_BIND_RID_APPID		(64*64)		/*(A1ǹ)����Ϣ��RID���ţ�10BCD*/
#define IPT0_OIL_VOICE					(65*64)		/*(A1ǹ)��Ʒ����������Ϣ���������ļ�������ǰ׺��4ASCII, "\x00\x00\x00\x00"��ʾ��ָ��*/
#define IPT0_CONTRAST					(66*64)		/*(A1ǹ)������ʾ�Աȶȣ�1HEX*/


#define IPT1_DUTY_INFO					(70*64)		/*(A1ǹ)��/�°���Ϣ����/�°���Ϣ1bytes+Ա����1bytes+Ա������(2bytes)+Ա������(10bytes)*/
#define IPT1_VOICE_SPEAKER			(71*64)		/*(B1ǹ)������ѡ��*/
#define IPT1_VOICE_TYPE					(72*64)		/*(B1ǹ)��������*/
#define IPT1_VOICE_VOLUME			(73*64	)		/*(B1ǹ)����*/
#define IPT1_PRINTER						(74*64	)		/*(B1ǹ)��ӡ��*/
#define IPT1_PRINT_AUTO				(75*64	)		/*(B1ǹ)�Զ���ӡ*/
#define IPT1_PRINT_UNION				(76*64	)		/*(B1ǹ)��ӡ����*/
#define IPT1_PRN_CARD_USER		(77*64	)		/*(B1ǹ)�û����˵��Զ���ӡ����,2HEX*/
#define IPT1_PRN_CARD_MANAGE	(78*64	)		/*(B1ǹ)�����˵��Զ���ӡ����,2HEX*/
#define IPT1_PRN_CARD_STAFF		(79*64	)		/*(B1ǹ)Ա�����˵��Զ���ӡ����,2HEX*/
#define IPT1_PRN_CARD_PUMP		(80*64	)		/*(B1ǹ)��ÿ��˵��Զ���ӡ����,2HEX*/
#define IPT1_PRN_CARD_SERVICE	(81*64	)		/*(B1ǹ)ά�޿��˵��Զ���ӡ����,2HEX*/
#define IPT1_NIGHT_LOCK				(82*64	)		/*(B1ǹ)ҹ������*/
#define IPT1_LOGIC_NOZZLE			(83*64	)		/*(B1ǹ)�߼�ǹ��*/
#define IPT1_PHYSICAL_NOZZLE		(84*64	)		/*(B1ǹ)����ǹ��*/
#define IPT1_PASSWORD					(85*64	)		/*(B1ǹ)��վ����Ա���룬2BCD*/
#define IPT1_WORKMODE					(86*64	)		/*(B1ǹ)����ģʽ,1HEX*/
#define IPT1_SERVICE_PASS			(87*64)		/*(B1ǹ)ά������,2BCD*/
#define IPT1_AUTHEN						(88*64)		/*(B1ǹ)��֤��ʽ��1HEX*/
#define IPT1_PRICE_INFO					(89*64)		/*(B1ǹ)��Ʒ�ͼ���Ϣ:��Ʒ����(2�ֽ�BCD)+��Ʒ�۸�(2�ֽ�HEX)*/
#define IPT1_STAFF_LIMIT				(90*64)		/*(B1ǹ)Ա��������������Ϣ,1HEX*/
#define IPT1_BIND_TIME					(91*64)		/*(B1ǹ)����Ϣ��ʱ�䣬7BCD*/
#define IPT1_BIND_MBOARD_ID		(92*64)		/*(B1ǹ)����Ϣ������ţ�8BCD*/
#define IPT1_BIND_ACT_APPID		(93*64)		/*(B1ǹ)����Ϣ��ACT���ţ�10BCD*/
#define IPT1_BIND_RID_APPID		(94*64)		/*(B1ǹ)����Ϣ��RID���ţ�10BCD*/
#define IPT1_OIL_VOICE					(95*64)		/*(B1ǹ)��Ʒ����������Ϣ���������ļ�������ǰ׺��4ASCII, "\x00\x00\x00\x00"��ʾ��ָ��*/
#define IPT1_CONTRAST					(96*64)		/*(B1ǹ)������ʾ�Աȶȣ�1HEX*/


/*�����������֣����������30�ֽ�*/
#define PRM_MBOARD_ID					(101*64)		/*����� 1HEX*/
#define PRM_SELL_LOCK					(102*64)		/*��������*/
#define PRM_BACKLIT						(103*64)		/*�������*/
#define PRM_IP_ADDR						(104*64)		/*����IP��Ϣ*/
#define PRM_EPS_ADDR					(105*64)		/*EPS������Ϣ*/
#define PRM_BANK_ADDR					(106*64)		/*����������Ϣ*/
#define PRM_PRC_INFO_1					(107*64)		/*��Ʒ��Ϣ1*/
#define PRM_PRC_INFO_2					(108*64)		/*��Ʒ��Ϣ2*/
#define PRM_PRC_INFO_3					(109*64)		/*��Ʒ��Ϣ3*/
#define PRM_PRC_INFO_4					(110*64)		/*��Ʒ��Ϣ4*/
#define PRM_PRC_INFO_5					(111*64)		/*��Ʒ��Ϣ5*/
#define PRM_PRC_INFO_6					(112*64)		/*��Ʒ��Ϣ6*/
#define PRM_NOZZLE_INFO_1			(113*64)		/*��ǹ��Ϣ1*/
#define PRM_NOZZLE_INFO_2			(114*64)		/*��ǹ��Ϣ2*/
#define PRM_NOZZLE_INFO_3			(115*64)		/*��ǹ��Ϣ3*/
#define PRM_NOZZLE_INFO_4			(116*64)		/*��ǹ��Ϣ4*/
#define PRM_NOZZLE_INFO_5			(117*64)		/*��ǹ��Ϣ5*/
#define PRM_NOZZLE_INFO_6			(118*64)		/*��ǹ��Ϣ6*/
#define PRM_VOLUME_SPEAKER0		  (119*64)		/*����ͨ��A����*/
#define PRM_VOLUME_SPEAKER1		  (120*64)		/*����ͨ��B����*/
#define PRM_SELL_LOCK_TIME		  (121*64)		/*��������ʱ�䣬4BCD(YYYYMMDD)+�Ƿ�������1byte*/
#define PRM_NOZZLE_NUMBER		  (122*64)		/*����ǹ�� 4HEX��0=���浥ǹ��1=����˫ǹ*/
#define PRM_BARCODE_BRAND_A		  (123*64)		/*A������ɨ��ģ��Ʒ�ƣ�1ASCII*/
#define PRM_BARCODE_BRAND_B		  (124*64)		/*B������ɨ��ģ��Ʒ�ƣ�1ASCII*/
#define PRM_MODEL				  (125*64)		/*���� 4HEX*/
#define PRM_PROMOTION			  (126*64)		/*�Ƿ����ô������� 4HEX 0 =�����ô�����1 = ���ô�������*/
#define PRM_SINOPEC_CONNECT		  (127*64)		/*����������̨���ӷ�ʽASCII '0'=���������ڣ�'1'=RJ45���ڣ�*/
#define PRM_SINOPEC_ADDRESS		  (128*64)		/*����������̨��������ַ��IP��ַ(4HEX) + �˿ں�(2HEX)*/
#define PRM_SINOPEC_LOCAL_PORT	  (129*64)	/*����������̨ͨѶ���ط������˿ں�2HEX*/
#define PRM_YuLe_Grade_OK		  (130*64)	/*�����������ֻ���Ʒȷ�Ϲ����Ƿ�����1HEX*/
#define PRM_OilLimit_Style_Set	  (131*64)	/*2017-02-13��Ʒ���Ʒ�ʽ����1HEX*/
#define PRM_ETC_FUN_SET			  (132*64)	/*ETC��������1hex*/

//fj:20171214����
#define PRM_SINOPEC_LOCAL_IP      (133*64)  //����������̨����IP
#define PRM_SINOPEC_LOCAL_MASK    (134*64)  //����������̨������������
#define PRM_SINOPEC_LOCAL_GATEWAY (135*64)  //����������̨��������
#define PRM_SINOPEC_LOCAL_MAC     (136*64)  //����������̨����MAC��ַ


/*�ⲿ��������*/
extern int paramSetupWrite(off_t offset, const unsigned char *buffer, int nbytes);
extern int paramSetupRead(off_t offset, unsigned char *buffer, int maxbytes);

extern int paramModelSet(int model);
extern int paramModelGet(void);
extern int paramPanelNozzleSet(int panel_nozzle);
extern int paramPanelNozzleGet(void);
extern int paramPromotionSet(int promoton);
extern int paramPromotionGet(void);

extern int paramBSPVersionGet(char *outbuffer, int maxbytes);

extern int paramSetupClr(void);
extern bool paramSetupInit(void);


#endif

