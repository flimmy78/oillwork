#ifndef _OIL_DSP_H_
#define _OIL_DSP_H_

//显示一般数据最大长度
#define DSP_LEN_MAX			512

//显示设备
#define DSP_DEV_KEYBOARD		0			//点阵键盘
#define DSP_DEV_PCMONITOR		1			//平板电脑

//显示设备枪号
#define DSP_NOZZLE_1			0			//1号显示设备
#define DSP_NOZZLE_2			1			//2号显示设备

//键盘点阵屏显示命令宏定义
#define DSP_TEXT_INFO					101		//文本显示界面
#define DSP_CARD_STANDBY				102		//加油卡加油待机界面
#define DSP_CARD_PRETREAT				103		//加油卡预处理提示界面
#define DSP_CARD_UNLOCK_FINISH		    104		//加油卡补扣处理提示界面
#define DSP_CARD_TACCLR_FINISH			105		//加油卡补充处理提示界面
#define DSP_CARD_PASSIN					106		//加油卡密码输入界面
#define DSP_CARD_CARLIMIT				107		//限车号卡员工密码输入界面
#define DSP_CARD_BALANCE				108		//加油卡余额界面
#define DSP_CARD_PRESET					109		//加油卡加油预置界面
#define DSP_CARD_UNIT_SELECT			110		//加油卡结算单位选择界面
#define DSP_CARD_SETTLE_SELECT			111		//加油卡结算方式选择界面
#define DSP_CARD_STAFF_PASSIN			112		//加油卡加油员工密码输入界面
#define DSP_CARD_OIL_START				113		//加油卡加油启动提示界面
#define DSP_CARD_OILLING				114		//加油卡加油中界面
#define DSP_CARD_OIL_FINISH				115		//加油卡加油结束提示界面
#define DSP_CARD_OILEND					116		//加油卡加油完成界面
#define DSP_CARD_ERR_INFO				117		//加油卡加油含代码的错误提示界面
#define DSP_PASSWORD_INPUT				118		//密码输入界面
#define DSP_OPERATE_SELECT				119		//操作项选择界面
#define DSP_INQ_SELECT					120		//查询项选择界面
#define DSP_INQ_JLSUM					121		//查询计量总累界面
#define DSP_INQ_BILL_SELECT				122		//查询账单明细操作对象选择界面					
#define DSP_INQ_BILL_TTCINPUT			123		//查询账单明细TTC输入界面
#define DSP_INQ_BILL_INDEX				124		//查询账单明细索引界面
#define DSP_INQ_BILL_DATA				125		//查询账单明细原始数据界面
#define DSP_INQ_NOZZLE_INFO				126		//查询油枪信息界面
#define DSP_INQ_BOARD_INFO				127		//查询主板信息界面
#define DSP_INQ_TIME					128		//查询时间界面
#define DSP_INQ_VOICE					129		//查询语音信息界面
#define DSP_INQ_PRINT					130		//查询打印信息界面
#define DSP_INQ_PRINT_CARD			    131		//查询打印信息卡类型选择界面
#define DSP_INQ_PRINT_BILLTYPE			132		//查询打印信息账单类型选择界面
#define DSP_MONTH_INPUT					133		//月份输入界面
#define DSP_DATE_INPUT					134		//日期输入界面
#define DSP_INQ_LIMIT_INFO				135		//查询限制信息界面
#define DSP_INQ_ADVANCE_INFO			136		//查询提前量界面
#define DSP_INQ_UNPULSE_TIME			137		//查询无脉冲停机超时时间界面
#define DSP_INQ_BIND_INFO				138		//查询绑定信息界面
#define DSP_INQ_VERSION_INFO			139		//查询版本信息界面
#define DSP_SET_SELECT					140		//设置项选择界面
#define DSP_SET_PRICE					141		//设置单价界面
#define DSP_SET_TIME					142		//设置时间界面
#define DSP_SET_BACKLIT					143		//设置背光界面
#define DSP_SET_NIGHTLOCK				144		//设置夜间锁定界面
#define DSP_SET_PASSWORD_OLD			145		//设置密码，输入旧密码
#define DSP_SET_PASSWORD_NEW			146		//设置密码，输入新密码
#define DSP_SET_PASSWORD_ACK			147		//设置密码，输入新密码确认
#define DSP_SET_PHYSICAL_NOZZLE		    148		//设置物理枪号
#define DSP_SET_TAX_TIME				149		//设置税控时间
#define DSP_SET_SPEAKER					150		//设置扬声器
#define DSP_SET_VOICE_VOLUME			151		//设置语音音量
#define DSP_SET_VOICE_TYPE				152		//设置语音类型
#define DSP_SET_PRINTER					153		//设置打印机
#define DSP_SET_PRINT_AUTO				154		//设置是否自动打印
#define DSP_SET_PRINT_UNION				155		//设置打印联数
#define DSP_SET_PRINT_CARD				156		//设置自动打印卡类型选择界面
#define DSP_SET_PRINT_BILLTYPE			157		//设置自动打印账单类型界面
#define DSP_SET_ADVANCE					158		//设置提前量界面
#define DSP_SET_UNPULSE_TIME			159		//设置无脉冲超时停机时间界面
#define DSP_SET_STAFF_LIMIT				160		//设置员工卡加油限制界面
#define DSP_SET_MODE					161		//设置模式界面
#define DSP_OTHER_OPERATE_PASSSIN	    162		//其它操作密码输入界面
#define DSP_OTHER_OPERATE				163		//其它操作界面
#define DSP_INQ_JLTYPE					164		//计量机型显示界面
#define DSP_INQ_SHILED					165		//计量屏蔽量及过冲屏蔽量显示界面
#define DSP_INQ_EQUIVALENT				166		//显示计量当量界面
#define DSP_INQ_VALVE_VOLUME			167		//显示大阀打开时最低油量
#define DSP_UNSELF_STANDBY				168		//非卡机联动待机界面
#define DSP_UNSELF_PRESET				169		//非卡机联动预置界面
#define DSP_UNSELF_OILLING				170		//非卡机联动加油中界面
#define DSP_INQ_STATION_INFO1			171		//油站通用信息界面，第一部分
#define DSP_INQ_STATION_INFO2			172		//油站通用信息界面，第二部分
#define DSP_INQ_OILINFO					173		//油品油价表信息界面
#define DSP_INQ_BASELIST				174		//基础黑名单信息界面
#define DSP_INQ_ADDLIST					175		//新增黑名单信息界面
#define DSP_INQ_DELLIST					176		//新删黑名单信息界面
#define DSP_INQ_WHITELIST				177		//白名单信息界面
#define DSP_OIL_OVER_INFO				178		//过冲加油信息界面
#define DSP_OIL_OVER_STAFFIN			179		//过冲加油员工密码输入界面
#define DSP_CARD_TEST					180		//卡座电特性,协议等测试界面
#define DSP_CARD_PROTOCOL_TIME		    181		//卡座协议连续测试轮询时间设置界面

 
#define DSP_TM_SCAN						182		//请扫描或输入条码的提示界面
#define DSP_TM_BARCODE_INPUT			183		//条码输入界面
#define DSP_TM_AUTHORIZING				184		//条码授权申请中界面
#define DSP_TM_AUTHORIZE				185		//授权申请结果显示界面
#define DSP_TM_AUTHORIZE_CANCEL		    186		//条码授权撤销中
#define DSP_TM_OILSTART					187		//条码自助加油启动中
#define DSP_TM_OILLING					188		//条码自助加油中
#define DSP_TM_OILFINISH				189		//条码自助加油结束中
#define DSP_TM_OIL_FINAL				190		//条码自助加油结果显示界面

#define DSP_INQ_UNPULSE_OVERTIME	    191		//无脉冲超时关大阀时间界面
#define DSP_INQ_ISAUTHEN				192		//加油是否需要DES认证的界面
#define DSP_INQ_OILVOICE				193		//油品语音查询界面
#define DSP_SET_OILVOICE_SELECT		    194		//油品语音大项选择界面
#define DSP_SET_OILVOICE				195		//油品语音选择界面
#define DSP_INQ_OILERRLOG				196		//查询错误日志界面
#define DSP_INQ_BARGUNNUMBER			197		//条码枪数查询界面	
#define DSP_INQ_BARBRAND				198		//条码品牌查询界面
#define DSP_SET_NOZZLE_NUMBER			199		//条码枪数设置界面
#define DSP_SET_BARBRAND				200		//条码品牌设置界面
#define DSP_LOCAL_NETINFO				201		//本地网络信息项选择界面
#define DSP_LOCAL_IP					202		//本地IP地址界面
#define DSP_LOCAL_MASK					203		//本地掩码界面
#define DSP_LOCAL_GATEWAY			    204		//本地默认网关界面
#define DSP_LOCAL_UNUSED				205		//备用
#define DSP_BACKSTAGE_INFO				206		//石化后台配置信息
#define DSP_BACKSTAGE_IP				207		//石化后台IP地址
#define DSP_BACKSTAGE_PORT				208		//石化后台通讯后台服务器端口号信息
#define DSP_BACKSTAGE_LOCAL_PORT	    209		//石化后台通讯本地服务器端口号信息
#define DSP_BACKSTAGE_UNUSED2			210		//备用
#define DSP_TABLETPC_INFO				211		//平板电脑网络信息项选择界面
#define DSP_TABLETPC_IP					212		//平板电脑IP信息界面
#define DSP_TABLETPC_MASK				213		//平板电脑掩码信息界面
#define DSP_TABLETPC_GATEWAY			214		//平板电脑网关信息界面
#define DSP_TABLETPC_FIRSTDNS			215		//平板电脑首选DNS信息界面
#define DSP_TABLETPC_SECONDDNS		    216		//平板电脑备用DNS信息界面
#define DSP_TABLETPC_FTP_IP				217		//平板电脑FTP地址信息界面
#define DSP_TABLETPC_FTP_PORT			218		//平板电脑FTP端口号信息界面
#define DSP_TABLETPC_SERVERIP			219		//平板电脑连接的后台IP地址信息界面
#define DSP_TABLETPC_VOLUME				220		//平板电脑音量信息界面
#define DSP_TABLETPC_TELE_IP			221		//平板电脑语音对讲后台IP地址信息界面
#define DSP_TABLETPC_UNUSED2			222		//备用
#define DSP_CONNECT_TYPE_SET			223		//后台连接方式设置界面
#define DSP_CONNECT_TYPE_DSP			224		//后台连接方式查询界面
#define DSP_CONTRAST					225		//显示对比度
#define DSP_DICOUNT_FAIL_KEEP			226		//申请折扣失败后人工确认界面
#define DSP_MODEL						227		//机型操作界面
#define DSP_AUTH_BALANCE				228		//授权加油余额界面
#define DSP_CARD_DEBIT					229		//非油消费扣款界面
#define DSP_PROMOTION					230		//是否启用促销界面
#define DSP_TM_SCAN_AND_INPUT			231		//条码扫描及输入界面
#define DSP_BANK_PRESET					232		//银行卡预置界面
#define DSP_BANK_PIN_INPUT				233		//银行卡密码输入界面
#define DSP_BANK_AUTH_REGUEST			234		//银行卡预授权
#define DSP_BANK_AUTH_RESULT			235		//银行卡预授权成功界面
#define DSP_BANK_OILLING				236		//银行卡加油中界面
#define DSP_YuLe_Grade_Fun				237		//是否启用油品确认功能
#define DSP_ETC_FUN_COUR                238     //ETC功能设置界面,szb_fj20171120
#define DSP_ETC_PIN_INPUT               239     //ETC验证码输入,szb_fj20171120
#define DSP_CAR_NOYIZHI                 240     //ETC车牌号不一致,szb_fj20171120
#define DSP_ETC_OILCR                   241     //ETC油品不一致确认,szb_fj20171120

//显示空闲时间计数器
extern unsigned int dspTimerA, dspTimerB;

//播放视频
extern int dspVideo(int nozzle);
//显示界面接口函数
extern int dsp(int nozzle, int id, unsigned char *buffer, int nbytes);
//显示模块功能初始化
extern int dspInit(void);

//void tdspRecive(int nozzle);
//void tdsp(int nozzle);
void tdspRecive(void* pNozzleType);
void tdsp(void* pNozzleType);

#endif

