#ifndef	_OIL_PARAM_H_
#define _OIL_PARAM_H_

//#include "yaffs22/yaffsfs.h"

/*软件版本号*/
#define _SOFTWARE_VERSION_			"V1.00.27"
/*告知平板的加油机软件版本号，应与_SOFTWARE_VERSION_同步*/
#define PC_OIL_SOFTVERSION			"\x01\x00\x27"

/*油机与平板通讯协议版本号*/
#define PC_SOFTVERSION					"\x00\x28"



/*机型定义*/
extern const char *ModelParam[];
#define MODEL_SINOPEC							0x0000	/*石化卡机联动加油机*/
#define MODEL_PRIVATE							0x0001	/*社会站加油机*/
#define MODEL_LIANDA							0x0002	/*联达系统加油机*/
#define MODEL_PARAM_MIN					MODEL_SINOPEC		/*最小值*/
#define MODEL_PARAM_MAX					MODEL_LIANDA		/*最大值*/

/*单面枪数*/
#define PANEL_NOZZLE_SINGLE			0	/*单面单枪-双枪机*/
#define PANEL_NOZZLE_DOUBLE  		1	/*单面双枪-四枪机*/

/*文件路径*/  //fj:20170905
#define FILE_SETUP						"../config/mboardFiles/OilSetup.txt"				/*参数配置文件*/
#define FILE_ERROR_INFO					"../config/mboardFiles/ErrorInfo.txt"			/*错误信息文件*/
#define FILE_RECORD						"../config/mboardFiles/Record.txt"				/*记录信息文件*/

/*计量(JL)参数在文件内的偏移，参数名最大30字节*/
#define JL0_EQUIVALENT					(0*64)			/*(A1枪)当量,4HEX*/
#define JL0_UNPULSE_TIME				(1*64)			/*(A1枪)无脉冲超时时间,4HEX*/
#define JL0_ADVANCE						(2*64)			/*(A1枪)提前量,4HEX*/
#define JL0_SHIELD							(3*64)			/*(A1枪)屏蔽量,4HEX*/
#define JL0_OVER_SHIELD				(4*64)			/*(A1枪)过冲屏蔽量,4HEX*/
#define JL0_TYPE								(5*64)			/*(A1枪)计量机型,4HEX*/
#define JL0_PRICE								(6*64)			/*(A1枪)计量单价,4HEX*/
#define JL0_VAVLE_VOLUME				(7*64)			/*(A1枪)计量不小于此油量时开启大阀,4HEX*/
#define JL0_VAVLE_STOP					(8*64)			/*(A1枪)计量无脉冲时间超过此值时关闭大阀,4HEX*/
#define JL0_ALGORITHM					(9*64)			/*(A1枪)计量加油量算法标识,4HEX*/
#define JL0_BIG_VOL_TIME					(10*64)	/*(A1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8*/
#define JL0_BIG_VOL_SPEED					(11*64)	/*(A1枪)双编码器大流量流速控制1BCD L/分10-80默认15*/


#define JL1_EQUIVALENT					(20*64)		/*(B1枪)当量,4HEX*/
#define JL1_UNPULSE_TIME				(21*64)		/*(B1枪)无脉冲超时时间,4HEX*/
#define JL1_ADVANCE						(22*64)		/*(B1枪)提前量,4HEX*/
#define JL1_SHIELD							(23*64)		/*(B1枪)屏蔽量,4HEX*/
#define JL1_OVER_SHIELD				(24*64	)		/*(B1枪)过冲屏蔽量,4HEX*/
#define JL1_TYPE								(25*64)		/*(B1枪)计量机型,4HEX*/
#define JL1_PRICE								(26*64)		/*(B1枪)计量单价,4HEX*/
#define JL1_VAVLE_VOLUME				(27*64)		/*(B1枪)计量在不小于此油量时开启大阀,4HEX*/
#define JL1_VAVLE_STOP					(28*64)		/*(B1枪)计量无脉冲时间超过此值时关闭大阀,4HEX*/
#define JL1_ALGORITHM					(29*64)		/*(B1枪)计量加油量算法标识,4HEX*/
#define JL1_BIG_VOL_TIME					(30*64)	/*(B1枪)双编码器大流量无脉冲超时时间控制1BCD 秒5-20默认8*/
#define JL1_BIG_VOL_SPEED					(31*64)	/*(B1枪)双编码器大流量流速控制1BCD L/分10-80默认15*/


/*中石化应用部分IPT及PCD参数定义，参数名最大30字节*/
#define IPT0_DUTY_INFO					(40*64)		/*(A1枪)上/下班信息，上/下班信息1bytes+员工号1bytes+员工密码(2bytes)+员工卡号(10bytes)*/
#define IPT0_VOICE_SPEAKER			(41*64)		/*(A1枪)扬声器选择*/
#define IPT0_VOICE_TYPE					(42*64)		/*(A1枪)语音类型*/
#define IPT0_VOICE_VOLUME			(43*64)		/*(A1枪)音量*/
#define IPT0_PRINTER						(44*64)		/*(A1枪)打印机*/
#define IPT0_PRINT_AUTO				(45*64)		/*(A1枪)自动打印*/
#define IPT0_PRINT_UNION				(46*64)		/*(A1枪)打印联数*/
#define IPT0_PRN_CARD_USER		(47*64)		/*(A1枪)用户卡账单自动打印类型,2HEX*/
#define IPT0_PRN_CARD_MANAGE	(48*64)		/*(A1枪)管理卡账单自动打印类型,2HEX*/
#define IPT0_PRN_CARD_STAFF		(49*64)		/*(A1枪)员工卡账单自动打印类型,2HEX*/
#define IPT0_PRN_CARD_PUMP		(50*64)		/*(A1枪)验泵卡账单自动打印类型,2HEX*/
#define IPT0_PRN_CARD_SERVICE	(51*64)		/*(A1枪)维修卡账单自动打印类型,2HEX*/
#define IPT0_NIGHT_LOCK				(52*64)		/*(A1枪)夜间锁定*/
#define IPT0_LOGIC_NOZZLE			(53*64)		/*(A1枪)逻辑枪号*/
#define IPT0_PHYSICAL_NOZZLE		(54*64)		/*(A1枪)物理枪号*/
#define IPT0_PASSWORD					(55*64)		/*(A1枪)油站操作员密码，2BCD*/
#define IPT0_WORKMODE					(56*64	)		/*(A1枪)工作模式,1HEX*/
#define IPT0_SERVICE_PASS			(57*64)		/*(A1枪)维修密码,2BCD*/
#define IPT0_AUTHEN						(58*64)		/*(A1枪)认证方式，1HEX*/
#define IPT0_PRICE_INFO					(59*64)		/*(A1枪)油品油价信息:油品代码(2字节BCD)+油品价格(2字节HEX)*/
#define IPT0_STAFF_LIMIT				(60*64)		/*(A1枪)员工卡加油限制信息,1HEX*/
#define IPT0_BIND_TIME					(61*64)		/*(A1枪)绑定信息，时间，7BCD*/
#define IPT0_BIND_MBOARD_ID		(62*64)		/*(A1枪)绑定信息，主板号，8BCD*/
#define IPT0_BIND_ACT_APPID		(63*64)		/*(A1枪)绑定信息，ACT卡号，10BCD*/
#define IPT0_BIND_RID_APPID		(64*64)		/*(A1枪)绑定信息，RID卡号，10BCD*/
#define IPT0_OIL_VOICE					(65*64)		/*(A1枪)油品语音代码信息，即语音文件的数字前缀，4ASCII, "\x00\x00\x00\x00"表示无指定*/
#define IPT0_CONTRAST					(66*64)		/*(A1枪)键盘显示对比度，1HEX*/


#define IPT1_DUTY_INFO					(70*64)		/*(A1枪)上/下班信息，上/下班信息1bytes+员工号1bytes+员工密码(2bytes)+员工卡号(10bytes)*/
#define IPT1_VOICE_SPEAKER			(71*64)		/*(B1枪)扬声器选择*/
#define IPT1_VOICE_TYPE					(72*64)		/*(B1枪)语音类型*/
#define IPT1_VOICE_VOLUME			(73*64	)		/*(B1枪)音量*/
#define IPT1_PRINTER						(74*64	)		/*(B1枪)打印机*/
#define IPT1_PRINT_AUTO				(75*64	)		/*(B1枪)自动打印*/
#define IPT1_PRINT_UNION				(76*64	)		/*(B1枪)打印联数*/
#define IPT1_PRN_CARD_USER		(77*64	)		/*(B1枪)用户卡账单自动打印类型,2HEX*/
#define IPT1_PRN_CARD_MANAGE	(78*64	)		/*(B1枪)管理卡账单自动打印类型,2HEX*/
#define IPT1_PRN_CARD_STAFF		(79*64	)		/*(B1枪)员工卡账单自动打印类型,2HEX*/
#define IPT1_PRN_CARD_PUMP		(80*64	)		/*(B1枪)验泵卡账单自动打印类型,2HEX*/
#define IPT1_PRN_CARD_SERVICE	(81*64	)		/*(B1枪)维修卡账单自动打印类型,2HEX*/
#define IPT1_NIGHT_LOCK				(82*64	)		/*(B1枪)夜间锁定*/
#define IPT1_LOGIC_NOZZLE			(83*64	)		/*(B1枪)逻辑枪号*/
#define IPT1_PHYSICAL_NOZZLE		(84*64	)		/*(B1枪)物理枪号*/
#define IPT1_PASSWORD					(85*64	)		/*(B1枪)油站操作员密码，2BCD*/
#define IPT1_WORKMODE					(86*64	)		/*(B1枪)工作模式,1HEX*/
#define IPT1_SERVICE_PASS			(87*64)		/*(B1枪)维修密码,2BCD*/
#define IPT1_AUTHEN						(88*64)		/*(B1枪)认证方式，1HEX*/
#define IPT1_PRICE_INFO					(89*64)		/*(B1枪)油品油价信息:油品代码(2字节BCD)+油品价格(2字节HEX)*/
#define IPT1_STAFF_LIMIT				(90*64)		/*(B1枪)员工卡加油限制信息,1HEX*/
#define IPT1_BIND_TIME					(91*64)		/*(B1枪)绑定信息，时间，7BCD*/
#define IPT1_BIND_MBOARD_ID		(92*64)		/*(B1枪)绑定信息，主板号，8BCD*/
#define IPT1_BIND_ACT_APPID		(93*64)		/*(B1枪)绑定信息，ACT卡号，10BCD*/
#define IPT1_BIND_RID_APPID		(94*64)		/*(B1枪)绑定信息，RID卡号，10BCD*/
#define IPT1_OIL_VOICE					(95*64)		/*(B1枪)油品语音代码信息，即语音文件的数字前缀，4ASCII, "\x00\x00\x00\x00"表示无指定*/
#define IPT1_CONTRAST					(96*64)		/*(B1枪)键盘显示对比度，1HEX*/


/*公共参数部分，参数名最大30字节*/
#define PRM_MBOARD_ID					(101*64)		/*主板号 1HEX*/
#define PRM_SELL_LOCK					(102*64)		/*销售锁定*/
#define PRM_BACKLIT						(103*64)		/*背光控制*/
#define PRM_IP_ADDR						(104*64)		/*本地IP信息*/
#define PRM_EPS_ADDR					(105*64)		/*EPS网络信息*/
#define PRM_BANK_ADDR					(106*64)		/*银行网络信息*/
#define PRM_PRC_INFO_1					(107*64)		/*油品信息1*/
#define PRM_PRC_INFO_2					(108*64)		/*油品信息2*/
#define PRM_PRC_INFO_3					(109*64)		/*油品信息3*/
#define PRM_PRC_INFO_4					(110*64)		/*油品信息4*/
#define PRM_PRC_INFO_5					(111*64)		/*油品信息5*/
#define PRM_PRC_INFO_6					(112*64)		/*油品信息6*/
#define PRM_NOZZLE_INFO_1			(113*64)		/*油枪信息1*/
#define PRM_NOZZLE_INFO_2			(114*64)		/*油枪信息2*/
#define PRM_NOZZLE_INFO_3			(115*64)		/*油枪信息3*/
#define PRM_NOZZLE_INFO_4			(116*64)		/*油枪信息4*/
#define PRM_NOZZLE_INFO_5			(117*64)		/*油枪信息5*/
#define PRM_NOZZLE_INFO_6			(118*64)		/*油枪信息6*/
#define PRM_VOLUME_SPEAKER0		  (119*64)		/*语音通道A音量*/
#define PRM_VOLUME_SPEAKER1		  (120*64)		/*语音通道B音量*/
#define PRM_SELL_LOCK_TIME		  (121*64)		/*销售锁定时间，4BCD(YYYYMMDD)+是否已锁定1byte*/
#define PRM_NOZZLE_NUMBER		  (122*64)		/*单面枪数 4HEX，0=单面单枪；1=单面双枪*/
#define PRM_BARCODE_BRAND_A		  (123*64)		/*A面条码扫描模块品牌，1ASCII*/
#define PRM_BARCODE_BRAND_B		  (124*64)		/*B面条码扫描模块品牌，1ASCII*/
#define PRM_MODEL				  (125*64)		/*机型 4HEX*/
#define PRM_PROMOTION			  (126*64)		/*是否启用促销功能 4HEX 0 =不启用促销；1 = 启用促销功能*/
#define PRM_SINOPEC_CONNECT		  (127*64)		/*卡机联动后台连接方式ASCII '0'=电流环串口；'1'=RJ45网口；*/
#define PRM_SINOPEC_ADDRESS		  (128*64)		/*卡机联动后台服务器地址，IP地址(4HEX) + 端口号(2HEX)*/
#define PRM_SINOPEC_LOCAL_PORT	  (129*64)	/*卡机联动后台通讯本地服务器端口号2HEX*/
#define PRM_YuLe_Grade_OK		  (130*64)	/*卡机联动娱乐机油品确认功能是否启用1HEX*/
#define PRM_OilLimit_Style_Set	  (131*64)	/*2017-02-13油品限制方式设置1HEX*/
#define PRM_ETC_FUN_SET			  (132*64)	/*ETC功能设置1hex*/

//fj:20171214增加
#define PRM_SINOPEC_LOCAL_IP      (133*64)  //卡机联动后台本地IP
#define PRM_SINOPEC_LOCAL_MASK    (134*64)  //卡机联动后台本地子网掩码
#define PRM_SINOPEC_LOCAL_GATEWAY (135*64)  //卡机联动后台本地网关
#define PRM_SINOPEC_LOCAL_MAC     (136*64)  //卡机联动后台网卡MAC地址


/*外部函数声明*/
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

