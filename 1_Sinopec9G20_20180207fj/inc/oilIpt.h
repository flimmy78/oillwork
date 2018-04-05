#ifndef _OIL_IPT_H_
#define _OIL_IPT_H_

//#include <msgQLib.h>
//#include "oilCfg.h"
//#include "oilPcd.h"
#include "oilStmTransmit.h"
//#include "oiletc.h"
//#include "oilIC.h"

//调试状态定义，调试时定义为1 ，非调试时定义为0
#define _IPT_DEBUG_							0

//参数定义
#define IPT_VERSION							"V1.00"											//IPT版本
#define IPT_COMPANY_ID					    "\x30\x33\xff\xff\xff\xff\xff\xff"	            //厂家单位ID
//#define IPT_DSP_WAIT()					taskDelay(3*ONE_SECOND)			                //显示保持时间
#define IPT_DSP_WAIT()						sleep(3)                                        //显示保持时间
#define IPT_MSG_MAX_LEN				        512					                            //IPT接收消息队列最大长度
#define IPT_MODE_IC							0						                        //卡机联动模式
#define IPT_MODE_UNSELF					    1						//非卡机联动模式
#define IPT_MODE_BARCODE				    2						//条码自助模式
#define IPT_WORK_OFFDUTY				    0						//油机下班状态
#define IPT_WORK_ONDUTY				        1						//油机上班状态
#define IPT_ROUNDING						5						//IPT计算数据舍入分界，小于此值舍入

#if 0
#if _TYPE_BIG260_
#define IPT_MONEY_MAX						10000000			//最大允许预置金额，260升大流量为10万元
#define IPT_VOLUME_MAX						10000000			//最大允许预置油量，260升大流量为10万升
#else
#define IPT_MONEY_MAX						999999				//最大允许预置金额，普通流量机型为9999.99元
#define IPT_VOLUME_MAX						999999				//最大允许预置油量，普通流量机型为9999.99升
#endif
#endif
#define IPT_TMAC_DEFUALT					"\x12\x34\x56\x78"	//默认账单TMAC
#define IPT_MONEY_MIN						100					//最小允许预置金额
#define IPT_VOLUME_MIN						100					//最小允许预置油量
#define IPT_PRICE_MAX						9999				//最大允许单价
#define IPT_PRICE_MIN						100					//最小允许单价
#define IPT_BILLUNLOAD_MAX				    2000					//本枪最多允许未上传账单数目
#define IPT_GUN_PUTUP						0						//抬枪状态
#define IPT_GUN_PUTDOWN					    1						//挂枪状态
#define IPT_PRESET_NO						0						//预置方式，任意加油
#define IPT_PRESET_VOLUME					1						//预置方式，定升数
#define IPT_PRESET_MONEY					2						//预置方式，定金额
#define IPT_PAYUNIT_MONEY				    0						//计算单位，金额
#define IPT_PAYUNIT_LOYALTY				    1						//计算单位，积分点数
#define IPT_PAYMENT_MONEY				    0						//计算方式，现金
#define IPT_PAYMENT_OILTICKET			    1						//计算方式，油票
#define IPT_PAYMENT_OILCHECK			    2						//计算方式，提油凭证
#define IPT_PAYMENT_BANK					3						//计算方式，银行卡
#define IPT_PAYMENT_OTHER1				    4						//计算方式，其它1
#define IPT_PAYMENT_OTHER2				    5						//计算方式，其它2					
#define IPT_CARDTYPE_SINO					0						//卡类型，石化规范卡
#define IPT_CARDTYPE_PBOC					1						//卡类型，PBOC金融卡
#define IPT_PHY_NOZZLE_MIN				    1						//物理枪号最小值
#define IPT_PHY_NOZZLE_MAX				    6						//物理枪号最大值
#define IPT_NIGHT_LOCK						1						//夜间锁定状态
#define IPT_NIGHT_UNLOCK					0						//非夜间锁定状态
#define IPT_SELL_LOCK						1						//销售锁定状态
#define IPT_SELL_UNLOCK						0						//非销售锁定状态
#define IPT_VOICE_TYPE_WOMAN		        0						//语音类型:女声
#define IPT_VOICE_TYPE_MAN				    1						//语音类型:男声
#define IPT_SERVEPASS_DEFAULT		        "\x99\x99"		        //默认的售后维修操作密码
#define IPT_STAPASS_DEFAULT				    "\x00\x00"		        //默认的油站操作密码
#define IPT_DSPCONTRAST_MIN			        25						//键盘显示对比度最小值
#define IPT_DSPCONTRAST_MAX			        40						//键盘显示对比度最大值
#define IPT_DSPCONTRAST_DEFAULT	            32						//键盘显示对比度默认值

//加油状态
#define IPT_OIL_IDLE						0				//空闲
#define IPT_OIL_STARTING					1				//加油启动中
#define IPT_OIL_FUELLING					2				//加油中
#define IPT_OIL_FINISHING					3				//加油结束中

//IPT与PCD间通讯分时使用标识
#define IPT_2PCD_UNUSED					0				//空闲
#define IPT_2PCD_POLL					1				//正常的轮询命令占用
#define IPT_2PCD_OILPROCESS			    2				//卡处理及加油处理过程占用
#define IPT_2PCD_ZDSAVE					3				//账单的存储过程占用
#define IPT_2PCD_INQ					4				//查询操作过程中的占用

//IPT与CPOS间通讯分时使用标识
#define IPT_2CPOS_UNUSED				0				//空闲
#define IPT_2CPOS_OILPROCESS			1				//正常加油过程占用
#define IPT_2CPOS_ZDUPLOAD			    2				//条码加油账单上传占用

//账单类型
#define IPT_BILLTYPE_NORMAL			    0				//正常
#define IPT_BILLTYPE_ESCAPE				1				//逃卡
#define IPT_BILLTYPE_ERROR				2				//错卡
#define IPT_BILLTYPE_UNLOCK			    3				//补扣
#define IPT_BILLTYPE_FINISH				4				//补充
#define IPT_BILLTYPE_WORKON			    5				//上班
#define IPT_BILLTYPE_WORKOFF			6				//下班
#define IPT_BILLTYPE_UNSELF				7				//非卡
#define IPT_BILLTYPE_OILINFO			8				//油价接收
#define IPT_BILLTYPE_REFUSE				9				//卡错拒绝

//支付终端枪号，此定义勿改动，因前期开发过程中以0/1进行操作，改动可能会造成错误
#define IPT_NOZZLE_1							0				//1号支付终端
#define IPT_NOZZLE_2							1				//2号支付终端

//IPT处理进程号
#define IPT_PID_PRETREAT					0x0000		//上电预处理
#define IPT_PID_STANDBY						0x0001		//待机界面

#define IPT_PID_OPERATE_PASS			0x0101		//查询:油机操作准入密码验证界面
#define IPT_PID_OPERATE_SELECT		    0x0102		//查询:操作选项选择
#define IPT_PID_INQUIRY					0x0103		//查询:查询选项选择
#define IPT_PID_INQUIRY_WAIT			0x0104		//查询:显示查询结果后等待按键操作返回查询选择界面
#define IPT_PID_INQ_PRN					0x0105		//查询:打印信息查询操作界面
#define IPT_PID_INQ_PRNCARD			    0x0106		//查询:自动打印卡类选择
#define IPT_PID_INQ_PRNTYPE				0x0107		//查询:自动打印账单类型
#define IPT_PID_INQ_TAXWAIT			0x0108		//查询:税控查询结果提示后等待按键操作返回查询选择界面
#define IPT_PID_INQ_TAXMSUM			0x0109		//查询:税控月累月份输入界面
#define IPT_PID_INQ_TAXDSUM			0x010a		//查询:税控日累日期输入界面
#define IPT_PID_INQ_GUNINFO			0x010b		//查询:油枪信息查询界面
#define IPT_PID_INQ_BOARDINFO		0x010c		//查询:主板信息查询界面
#define IPT_PID_INQ_ZD_SEL				0x010d		//查询:账单选择，选择本枪还是整个物理机的账单
#define IPT_PID_INQ_ZD_TTCIN			0x010e		//查询:输入查询的账单TTC
#define IPT_PID_INQ_ZD_CHECK			0x010f		//查询:账单明细查询
#define IPT_PID_INQ_ZD_INDEX			0x0110		//查询:账单明细索引
#define IPT_PID_INQ_ZD_DETAIL			0x0111		//查询:账单明细
#define IPT_PID_INQ_OILERRLOG		0x0112		//查询:查询加油异常日志
#define IPT_PID_INQ_LOCAL_NET		0x0113		//查询:本地网络信息查询项选择界面
#define IPT_PID_INQ_LOCAL_WAIT		0x0114		//查询:本地网络信息查询结果显示后等待过程
#define IPT_PID_INQ_BACKSTAGE		0x0115		//查询:后台网络通讯配置查询项选择界面
#define IPT_PID_INQ_BACKS_WAIT		0x0116		//查询:后台网络通讯配置查询结果显示后等待过程
#define IPT_PID_INQ_TABLETPC			0x0117		//查询:平板电脑配置查询项选择界面
#define IPT_PID_INQ_TABLET_WAIT	        0x0118		//查询:平板电脑配置查询结果显示后等待过程
#define IPT_PID_INQ_VERSION	        	0x0119		//查询:版本信息,szb_fj20171120

#define IPT_PID_SET							0x0151		//设置:设置选项选择界面
#define IPT_PID_SET_PRICE					0x0152		//设置:单价设置
#define IPT_PID_SET_TIME					0x0153		//设置:时间设置
#define IPT_PID_SET_SCJD					0x0154		//设置:首次检定设置
#define IPT_PID_SET_CCJD					0x0155		//设置:出厂检定设置
#define IPT_PID_SET_BACKLIT				    0x0156		//设置:背光设置
#define IPT_PID_SET_NIGHTLOCK		    0x0157		//设置:夜间锁定设置
#define IPT_PID_SET_PASS_OLD			0x0158		//设置:操作密码设置，输入旧密码
#define IPT_PID_SET_PASS_NEW			0x0159		//设置:操作密码设置，输入新密码
#define IPT_PID_SET_PASS_ACK			0x015a		//设置:操作密码设置，确认新密码
#define IPT_PID_SET_PHYNOZZLE		    0x015b		//设置:物理枪号设置
#define IPT_PID_SET_TAXTIME				0x015c		//设置:税务时间设置
#define IPT_PID_SET_SPEAKER				0x015d		//设置:语音扬声器设置
#define IPT_PID_SET_SPKVOLUME		    0x015e		//设置:语音音量设置
#define IPT_PID_SET_SPKTYPE				0x015f		//设置:语音类型设置
#define IPT_PID_SET_PRINTER				0x0160		//设置:打印机设置
#define IPT_PID_SET_PRNAUTO			    0x0161		//设置:自动打印设置
#define IPT_PID_SET_PRNUNION			0x0162		//设置:打印联数设置
#define IPT_PID_SET_PRNAUTO_IC		0x0163		//设置:自动打印账单卡类型设置
#define IPT_PID_SET_PRNAUTO_ZD		0x0164		//设置:自动打印账单类型设置
#define IPT_PID_SET_ADVANCE			0x0165		//设置:计量提前量设置
#define IPT_PID_SET_UNPULSE_TIME	0x0166		//设置:计量无脉冲超时停机时间设置	
#define IPT_PID_SET_STAFF_LIMIT		0x0167		//设置:员工卡是否允许加油设置
#define IPT_PID_SET_MODE				0x0168		//设置:模式设置
#define IPT_PID_SET_TAX_WAIT			0x0169		//设置:税控设置结果等待处理界面
#define IPT_PID_SET_OILVOICE_SEL	    0x016a		//设置:油品语音大项选择界面
#define IPT_PID_SET_OILVOICE			0x016b		//设置:油品语音选择界面
#define IPT_PID_SET_NOZZLE_NUM		    0x016c		//设置:条码单面枪数设置界面
#define IPT_PID_SET_BARBRAND			0x016d		//设置:条码扫描模块设置界面
#define IPT_PID_SET_CONNECT_TYPE	    0x016e		//设置:石化后台联网方式设置界面
#define IPT_PID_SET_LOCAL_NET			0x016f		//设置:本地网络信息设置界面
#define IPT_PID_SET_LOCAL_IP			0x0170		//设置:本地网络IP地址设置界面
#define IPT_PID_SET_LOCAL_MASK		0x0171		//设置:本地网络子网掩码设置界面
#define IPT_PID_SET_LOCAL_GATE		0x0172		//设置:本地网络默认网关设置界面
#define IPT_PID_SET_LOCAL_UNUSE	    0x0173		//设置:本地网络设置界面-备用
#define IPT_PID_SET_BACKSTAGE		0x0174		//设置:石化后台网络通讯设置界面
#define IPT_PID_SET_BACK_CONNECT	0x0175		//设置:石化后台网络通讯方式设置界面
#define IPT_PID_SET_BACK_IP			0x0176		//设置:石化后台网络通讯IP地址设置界面
#define IPT_PID_SET_BACK_PORT		0x0177		//设置:石化后台网络通讯端口号设置界面
#define IPT_PID_SET_BACK_LOCAL_PORT	0x0178		//设置:石化后台网络通讯本地服务器端口号设置界面
#define IPT_PID_SET_BACK_UNUSE2	    0x0178		//设置:石化后台网络通讯设置界面-备用2
#define IPT_PID_SET_PC_INFO		    0x0179		//设置:平板电脑信息设置界面
#define IPT_PID_SET_PC_IP			0x017a		//设置:平板电脑IP地址设置界面
#define IPT_PID_SET_PC_MASK			0x017b		//设置:平板电脑子网掩码设置界面
#define IPT_PID_SET_PC_GATEWAY		0x017c		//设置:平板电脑默认网关设置界面
#define IPT_PID_SET_PC_DNS1			0x017d		//设置:平板电脑首选DNS设置界面
#define IPT_PID_SET_PC_DNS2			0x017e		//设置:平板电脑备用DNS设置界面
#define IPT_PID_SET_PC_FTP_IP		0x017f		//设置:平板电脑FTP服务器地址设置界面
#define IPT_PID_SET_PC_FTP_PORT	    0x0180		//设置:平板电脑FTP服务器端口设置界面
#define IPT_PID_SET_PC_SERVER		0x0181		//设置:平板电脑的后台服务器IP地址
#define IPT_PID_SET_PC_VOLUME		0x0182		//设置:平板电脑音量设置
#define IPT_PID_SET_PC_TELE_IP		0x0183		//设置:平板电脑语音对讲后台IP地址设置界面
#define IPT_PID_SET_PROMOTION		0x0184		//设置:设置促销功能是否启用
#define IPT_PID_SET_CONTRAST		0x0185		//设置:设置键盘显示对比度界面
#define IPT_PID_SET_GRADE_FUN		0x0186		//设置:设置油品确认功能是否启用
#define IPT_PID_SET_ETC_FUN			0x0187		//设置:ETC功能,szb_fj20171120

#define IPT_PID_OTH_PASS			0x0191		//操作:其它操作密码验证过程
#define IPT_PID_OTHER				0x0192		//操作:其它操作界面
#define IPT_PID_OTHER_KEYLOAD		0x0193		//操作:密钥下载过程
#define IPT_PID_OTHER_MODEL			0x0194		//操作:机型设置过程
#define IPT_PID_OTHER_SUMWRITE	    0x0195		//操作:累计数修改过程
#define IPT_PID_OTHER_WAIT		    0x0196		//操作:其它操作显示结果等待操作过程

#define IPT_PID_IC_STANDBY			0x0201		//加油卡:待机处理进程
#define IPT_PID_IC_PRETREAT			0x0202		//加油卡:IC卡插入预处理，读卡信息
#define IPT_PID_PSAM_PRETREAT		0x0203		//加油卡:PSAM预处理及PSAM合法性检查
#define IPT_PID_IC_CHECK			0x0204		//加油卡:PSAM卡及IC卡合法性检查
#define IPT_PID_IC_PIN_INPUT		0x0205		//加油卡:IC卡密码输入
#define IPT_PID_IC_PIN_CHECK		0x0206		//加油卡:IC卡密码验证
#define IPT_PID_IC_STAF_PASSIN		0x0207		//加油卡:限车号卡员工密码验证
#define IPT_PID_IC_NOTES			0x0208		//加油卡:IC卡历史交易记录查询
#define IPT_PID_IC_NOTES_CHECK		0x0209		//加油卡:IC卡历史交易记录合法性检查
#define IPT_PID_IC_BL_CHECK			0x020a		//加油卡:IC卡黑/白名单检查
#define IPT_PID_IC_LOCK_INFO		0x020b		//加油卡:IC卡灰锁交易记录查询
#define IPT_PID_IC_LOCKRECORD		0x020c		//加油卡:查询灰卡交易记录
#define IPT_PID_IC_BAL_READ			0x020d		//加油卡:IC卡余额读取
#define IPT_PID_IC_USER_ACK			0x020e		//加油卡:用户卡加油按确认键释放静电
#define IPT_PID_IC_BALANCE			0x020f		//加油卡:IC卡余额显示界面
#define IPT_PID_IC_PAY_UNIT			0x0210		//加油卡:结算单位选择
#define IPT_PID_IC_PAY_MODE			0x0211		//加油卡:结算方式选择
#define IPT_PID_IC_LOGIN			0x0212		//加油卡:上班登陆操作界面
#define IPT_PID_IC_LOG_PASSIN		0x0213		//加油卡:上班登陆员工卡密码输入界面
#define IPT_PID_IC_LOGOUT			0x0214		//加油卡:下班确认界面
#define IPT_PID_IC_LOGOUT_PASSIN	0x0215		//加油卡:下班密码输入界面
#define IPT_PID_IC_OIL_ACK			0x0216		//加油卡:油品确认界面
#define IPT_PID_IC_OILCHECK			0x0217		//加油卡:加油启动数据合法性检测
#define IPT_PID_IC_OIL_AUTHEN		0x0218		//加油卡:IC卡加油启动认证
#define IPT_PID_IC_LOCK_INIT		0x0219		//加油卡:灰锁初始化
#define IPT_PID_IC_MAC1				0x021a		//加油卡:PSAM计算MAC1
#define IPT_PID_IC_LOCK				0x021b		//加油卡:IC卡灰锁
#define IPT_PID_IC_LOCK_GREYINFO	0x021c		//加油卡:IC卡灰锁超时后读取灰锁状态
#define IPT_PID_IC_ZD_ESCAPE		0x021d		//加油卡:加油过程产生逃卡账单
#define IPT_PID_IC_MAC2					0x021e		//加油卡:PSAM验证MAC2
#define IPT_PID_IC_OILSTART				0x021f		//加油卡:IC卡开始加油
#define IPT_PID_IC_OILLING				0x0220		//加油卡:IC卡加油中
#define IPT_PID_IC_OILFINISH			0x0221		//加油卡:IC卡结束加油
#define IPT_PID_IC_PSAM_GMAC			0x0222		//加油卡:PSAM产生GMAC
#define IPT_PID_IC_UNLOCK				0x0223		//加油卡:IC卡jie扣
#define IPT_PID_IC_TAC_CLEAR			0x0224		//加油卡:IC卡TAC清除
#define IPT_PID_IC_TTC_GET				0x0225		//加油卡:获取PCD分配的TTC
#define IPT_PID_IC_TMAC					0x0226		//加油卡:计算账单TMAC
#define IPT_PID_IC_ZD_SAVE				0x0227		//加油卡:加油数据上传
#define IPT_PID_IC_LASTSTEP				0x0228		//加油卡:加油结束的处理
#define IPT_PID_IC_ENDACK				0x0229		//加油卡:加油结束等待操作界面
#define IPT_PID_IC_OIL_OVERINFO	        0x022a		//加油卡:加油过冲信息界面
#define IPT_PID_IC_ESCAPE_ERR			0x022b		//加油卡:逃卡报警界面				
#define IPT_PID_IC_OIL_OVERSTAFF	    0x022c		//加油卡:加油过冲员工密码输入界面
#define IPT_PID_ACT_AUTHEN				0x022d		//加油卡:ACT认证处理过程
#define IPT_PID_RID_AUTHEN				0x022e		//加油卡:RID认证处理过程
#define IPT_PID_IC_DISCOUNT_ASK	    0x022f		//加油卡:(联达系统)向后台申请本次加油需要的数据
#define IPT_PID_IC_DISCOUNT_KEEP	0x0230		//加油卡:向后台申请折扣额失败，人工确认后继续以非折扣价格加油
#define IPT_PID_IC_DEBIT_START		0x0231		//加油卡:进入加油卡支付流程
#define IPT_PID_IC_DEBIT_DONE		0x0232		//加油卡:加油卡支付确认过程

#define IPT_PID_UNSELF_PRESET			0x0301		//非卡机联动加油预置过程
#define IPT_PID_UNSELF_CHECK			0x0302		//非卡机联动加油启动检查过程
#define IPT_PID_UNSELF_START			0x0303		//非卡机联动加油启动过程
#define IPT_PID_UNSELF_OILLING		    0x0304		//非卡机联动加油中
#define IPT_PID_UNSELF_FINISH			0x0305		//非卡机联动加油结束

//#define IPT_PID_TM_BAR_SCAN			0x0401		//条码自助:等待界面扫描条码
//#define IPT_PID_TM_BAR_INPUT			0x0402		//条码自助:条码输入界面
#define IPT_PID_TM_PRETREAT			0x0401		//条码自助:条码扫描前的预处理工作
#define IPT_PID_TM_SCAN				0x0402		//条码自助:条码扫描及输入界面
#define IPT_PID_TM_IC_PRETREAT		0x0403		//条码自助:IC卡预处理
#define IPT_PID_TM_PSAM_PRE			0x0404		//条码自助:PSAM预处理
#define IPT_PID_TM_CHECK				0x0405		//条码自助:PSAM卡及IC卡合法性检查
#define IPT_PID_TM_PIN_INPUT			0x0406		//条码自助:IC卡密码输入
#define IPT_PID_TM_PIN_CHECK			0x0407		//条码自助:IC卡密码验证
#define IPT_PID_TM_BL_CHECK			    0x0408		//条码自助:IC卡黑/白名单检查
#define IPT_PID_TM_LOCK_INFO			0x0409		//条码自助:IC卡灰锁交易记录查询
#define IPT_PID_TM_LOCKRECORD		0x040a		//条码自助:查询灰卡交易记录
#define IPT_PID_TM_BAL_READ			0x040b		//条码自助:IC卡余额读取
#define IPT_PID_TM_CODE_CHECK		0x040c		//条码自助:加油机向PC机查询验证码
#define IPT_PID_TM_CODE_ACK			0x040d		//条码自助:加油机确认PC机的查询结果
#define IPT_PID_TM_AUTHORIZED		0x040e		//条码自助:授权成功界面
#define IPT_PID_TM_PAY_MODE			0x040f		//条码自助:支付方式选择界面
#define IPT_PID_TM_OIL_ACK				0x0410		//条码自助:油品确认界面
#define IPT_PID_TM_OILCHECK				0x0411		//条码自助:加油启动数据合法性检测
#define IPT_PID_TM_OIL_AUTHEN		    0x0412		//条码自助:IC卡加油启动认证
#define IPT_PID_TM_LOCK_INIT			0x0413		//条码自助:灰锁初始化
#define IPT_PID_TM_MAC1						0x0414		//条码自助:PSAM计算MAC1
#define IPT_PID_TM_LOCK						0x0415		//条码自助:IC卡灰锁
#define IPT_PID_TM_MAC2						0x0416		//条码自助:PSAM验证MAC2
#define IPT_PID_TM_OILSTART				0x0417		//条码自助:IC卡开始加油
#define IPT_PID_TM_OILLING				0x0418		//条码自助:IC卡加油中
#define IPT_PID_TM_OILFINISH			0x0419		//条码自助:IC卡结束加油
#define IPT_PID_TM_PSAM_GMAC		    0x041a		//条码自助:PSAM产生GMAC
#define IPT_PID_TM_UNLOCK				0x041b		//条码自助:IC卡jie扣
#define IPT_PID_TM_TAC_CLEAR			0x041c		//条码自助:IC卡TAC清除
#define IPT_PID_TM_TTC_GET				0x041d		//条码自助:获取PCD分配的TTC
#define IPT_PID_TM_TMAC					0x041e		//条码自助:计算账单TMAC
#define IPT_PID_TM_ZD_SAVE				0x041f		//条码自助:加油数据保存
#define IPT_PID_TM_ZD_UPLOAD			0x0420		//条码自助:加油机主动上传授权加油结果信息
#define IPT_PID_TM_OIL_END				0x0421		//条码自助:加油结束处理
#define IPT_PID_TM_OIL_ENDWAIT		    0x0422		//条码自助:加油结束等待处理

#define IPT_PID_AUTH_PRETREAT		0x0501		//授权加油预处理
#define IPT_PID_AUTH_BALANCE		0x0502		//授权余额处理
#define IPT_PID_AUTH_OIL_CHECK		0x0503		//授权加油启动前数据自检过程
#define IPT_PID_AUTH_OIL_START		0x0504		//授权加油启动过程
#define IPT_PID_AUTH_OILLING		0x0505		//授权加油中
#define IPT_PID_AUTH_OIL_FINISH	    0x0506		//授权加油结束过程
#define IPT_PID_AUTH_DEBIT_APPLY	0x0507		//授权加油申请扣款过程
#define IPT_PID_AUTH_BILL_SAVE		0x0508		//授权加油账单存储过程
#define IPT_PID_AUTH_START_ERR		0x0550		//授权加油启动失败
#define IPT_PID_AUTH_PASS_INPUT	    0x0551		//授权加油ETC卡密码输入界面


#define IPT_PID_ERR_OILOVER			0x0601		//加油机过冲锁机等待处理界面
#define IPT_PID_ERRINFO_TMAC		0x0602		//内部出错信息计算TMAC过程
#define IPT_PID_ERRINFO_UPLOAD		0x0603		//内部出错信息上传过程
#define IPT_PID_DEBUG_CARD_TEST	    0x0604		//检测卡座过程
#define IPT_PID_DEBUG_TIME			0x0605		//检测卡座协议连续测试轮询时间设置过程
#define IPT_PID_ERR_QUEYILU			0x0606		//缺一路脉冲停机,szb_fj20171120
#define IPT_PID_ERR_LINGJIAYOU		0x0607		//零加油停机,szb_fj20171120
#define IPT_PID_ERR_WUPULSE			0x0608		//无脉冲超时停机,szb_fj20171120
#define IPT_PID_ERR_QUEYIZU			0x0609		//缺一组脉冲停机,szb_fj20171120

#define IPT_PID_BANK_PIN_INPUT		0x0701		//银行卡加油:PIN输入界面
#define IPT_PID_BANK_PRESET			0x0702		//银行卡加油:预置界面
#define IPT_PID_BANK_AUTH_REGUST	0x0703		//银行卡加油:预授权申请
#define IPT_PID_BANK_AUTH_RESULT	0x0704		//银行卡加油:预授权申请成功等待加油启动
#define IPT_PID_BANK_OIL_START		0x0705		//银行卡加油:加油启动中
#define IPT_PID_BANK_OILLING		0x0706		//银行卡加油:加油中
#define IPT_PID_BANK_OIL_FINISH		0x0707		//银行卡加油:加油结束中
#define IPT_PID_BANK_AUTH_CANCEL	0x0708		//银行卡加油:预授权撤销
#define IPT_PID_BANK_AUTH_ACK		0x0709		//银行卡加油:预授权确认扣款
#define IPT_PID_BANK_END_WAIT		0x070A		//银行卡加油:交易结束后等待操作


//油机状态
#define IPT_STATE_IDLE					0x0000		//空闲
#define IPT_STATE_CARD_PRETREAT			0x0001		//加油卡预处理
#define IPT_STATE_CARD_PASSIN			0x0002		//加油卡密码输入
#define IPT_STATE_CARD_PLATE			0x0003		//加油卡限车号
#define IPT_STATE_CARD_REUNLOCK			0x0004		//加油卡补扣
#define IPT_STATE_CARD_BALANCE			0x0005		//加油卡余额、预置
#define IPT_STATE_CARD_APP_SEL			0x0006		//加油卡应用选择
#define IPT_STATE_CARD_TYPE_SEL			0x0007		//加油卡支付方式选择
#define IPT_STATE_CARD_OILSTART			0x0008		//加油卡加油启动
#define IPT_STATE_CARD_OILLING			0x0009		//加油卡加油中
#define IPT_STATE_CARD_OILFINISHING	    0x000A		//加油卡加油结束处理中

#define IPT_STATE_UNSELF_PRESET			0x000C		//非卡预置界面
#define IPT_STATE_UNSELF_OILSTART		0x000D		//非卡加油启动中
#define IPT_STATE_UNSELF_OILLING		0x000E		//非卡加油中
#define IPT_STATE_UNSELF_OILEND			0x000F		//非卡加油结果
#define IPT_STATE_ERROR_INFO			0x0010		//错误信息提示
#define IPT_STATE_TM_SCAN					0x0013		//条码，请扫描条码或在键盘上输入条码
#define IPT_STATE_TM_APPLY_FOR				0x0014		//条码，条码授权申请中
#define IPT_STATE_TM_APPLY_OK				0x0015		//条码，授权申请成功界面
#define IPT_STATE_TM_CANCELING				0x0016		//条码，授权取消中
#define IPT_STATE_TM_OIL_START				0x0017		//条码，加油启动中
#define IPT_STATE_TM_OILLING				0x0018		//条码，加油中
#define IPT_STATE_TM_FINISHING				0x0019		//条码，加油结束中
#define IPT_STATE_TM_OIL_END				0x0020		//条码，加油结果
#define IPT_STATE_CARD_OILEND				0x0021		//加油卡加油结束结果
#define IPT_STATE_AUTH_BALANCE				0x0022		//授权加油，油机获得授权
#define IPT_STATE_AUTH_CANCEL				0x0023		//授权加油，油机取消授权
#define IPT_STATE_AUTH_OILSTART			    0x0024		//授权加油，油机加油启动中
#define IPT_STATE_AUTH_OILLING				0x0025		//授权加油，油机加油中
#define IPT_STATE_AUTH_OILFINISH			0x0026		//授权加油，油机加油结束中
#define IPT_STATE_AUTH_OIL_DATA			    0x0027		//授权加油，油机加油结果





//IPT与PCD通讯命令字
#define IPT_CMD_POLL						0x01			//普通查询命令
#define IPT_CMD_FORTTC						0x02			//IPT申请TTC
#define IPT_CMD_ZDSAVE						0x03			//IPT申请保存账单
#define IPT_CMD_LIST						0x04			//IPT查询黑/白名单
#define IPT_CMD_GREYINFO					0x05			//IPT查询灰锁记录
#define IPT_CMD_PRINTE						0x06			//IPT打印数据
#define IPT_CMD_SPK							0x07			//IPT语音数据
#define IPT_CMD_ZD_CHECK					0x08			//IPT查询本地账单
#define IPT_CMD_ID_SET						0x09			//IPT设置PCD主板ID
#define IPT_CMD_BARCODE					0x0a			//IPT条码数据转发
#define IPT_CMD_FOR_TMAC				0x0b			//PCD申请IPT计算TMAC
#define IPT_CMD_ERRINFO_UPLOAD	0x0c			//IPT通过PCD上传内部出错信息到后台
#define IPT_CMD_DISCOUNT_ASK		0x0d			//IPT通过PCD向后台申请折扣额

//账单表内偏移定义
#define IPT_BILL_SIZE									128			//账单长度
#define IPT_OFFSET_TTC									(0)			//POS_TTC	4bytes
#define IPT_OFFSET_T_TYPE							(4)			//交易类型1byte
#define IPT_OFFSET_TIME								(5)			//交易日期及时间7bytes卡应用
#define IPT_OFFSET_ASN								(12)			//卡应用号10bytes
#define IPT_OFFSET_BALANCE						(22)			//余额4bytes
#define IPT_OFFSET_AMN								(26)			//数额3bytes
#define IPT_OFFSET_CTC									(29)			//卡交易序号2bytes
#define IPT_OFFSET_TAC									(31)			//电子签名4bytes
#define IPT_OFFSET_GMAC								(35)			//解灰认证码4bytes
#define IPT_OFFSET_PSAM_TAC						(39)			//PSAM灰锁签名4bytes
#define IPT_OFFSET_PSAM_ASN					(43)			//PSAM应用号10bytes
#define IPT_OFFSET_TID									(53)			//PSAM编号6bytes
#define IPT_OFFSET_PSAM_TTC						(59)			//PSAM终端交易序号4bytes
#define IPT_OFFSET_DS									(63)			//扣款来源1byte
#define IPT_OFFSET_UNIT								(64)			//结算单位/方式1byte
#define IPT_OFFSET_C_TYPE							(65)			//卡类1byte
#define IPT_OFFSET_VER								(66)			//卡版本1byte	b7~b4:卡密钥索引号；b3~b0:卡密钥版本号
#define IPT_OFFSET_NZN								(67)			//枪号1byte
#define IPT_OFFSET_G_CODE							(68)			//油品代码2bytes
#define IPT_OFFSET_VOL								(70)			//升数3bytes
#define IPT_OFFSET_PRC								(73)			//成交价格2bytes
#define IPT_OFFSET_EMP								(75)			//员工号1byte
#define IPT_OFFSET_V_TOT							(76)			//升累计4bytes
#define IPT_OFFSET_RFU								(80)			//备用部分11bytes
#define IPT_OFFSET_MONEY_DISCOUNT		(80)			//(仅联达系统)折扣后金额，3HEX
#define IPT_OFFSET_DISCOUNT						(83)			//(仅联达系统)折扣额，2HEX
#define IPT_OFFSET_STATE							(85)			//(授权加油)账单状态 1byte 0=正常；1=ETC卡扣款失败
#define IPT_OFFSET_T_MAC							(91)			//终端数据认证码4bytes........................................
#define IPT_OFFSET_PHYGUN							(95)			//物理枪号1byte
#define IPT_OFFSET_STOPNO							(96)			//计量停机代码1byte
#define IPT_OFFSET_BEFOR_BAL					(97)			//扣前余额4bytes
#define IPT_OFFSET_ZD_STATE						(101)		//账单状态0=正常；1=未完成
#define IPT_OFFSET_JLNOZZLE						(102)		//计量枪号1byte
																							//备用，103~126
#define IPT_OFFSET_INVOICE_TYPE				(103)		//(仅联达系统)开票类型1HEX
#define IPT_OFFSET_ZDXOR							(127)		//账单异或校验,1byte
#define IPT_OFFSET_ZDBACKUP						(128)		//128~255账单备份，总长128字节

//非卡/过冲账单数据表内偏移
#define IPT_UNSELF_BILL_SIZE					32				//非卡/过冲账单数据表内偏移
#define IPT_UNSELF_OFF_MONEY				0					//3HEX	本次加油金额；(偏移0)
#define IPT_UNSELF_OFF_VOLUME				3				//3HEX	本次加油油量；(偏移3)
#define IPT_UNSELF_OFF_STATE					6				//1byte	非卡加油状态；(偏移6)
#define IPT_UNSELF_OFF_SUMMONEY			7				//3HEX	本次累计金额；(偏移7)
#define IPT_UNSELF_OFF_SUMVOLUME		10				//3HEX	本次累计油量；(偏移10)
#define IPT_UNSELF_OFF_OVERTIMES		13					//1HEX	过冲次数；(偏移13)
#define IPT_UNSELF_OFF_CHECK					30				//2bytes	CRC校验，计算前30bytes；(偏移30)


//条码自助账单数据表内偏移
#define IPT_BAR_BILL_SIZE							32				//条码账单大小
#define IPT_BAR_OFF_NOZZLE						0				//枪号1byte
#define IPT_BAR_OFF_TTC							1				//POS_TTC	4bytes
#define IPT_BAR_OFF_TIME							5				//日期时间7bytes
#define IPT_BAR_OFF_MONEY						12				//加油金额3bytes
#define IPT_BAR_OFF_VOLUME					15				//加油升数3bytes
#define IPT_BAR_OFF_AUTH_MONEY			18				//授权金额3bytes
#define IPT_BAR_OFF_AUTH_CODE				21				//授权验证码5bytes
#define IPT_BAR_OFF_PRICE						26				//单价2bytes
#define IPT_BAR_OFF_OIL_CODE					28				//油品代码2bytes
#define IPT_BAR_OFF_STATE						30				//账单状态1byte，0=已上传；1=未上传
#define IPT_BAR_OFF_CHECK						31				//校验码1byte


//铁电存储地址，每条枪512bytes
#define IPT_FM_DATALEN						512								//供IPT使用的铁电大小为,单枪数据长度
#define IPT_FM_ZD									0									//账单起始地址
#define IPT_FM_ZDBACKUP					(IPT_FM_ZD+128)			//128~255账单备份，总长128字节

#define IPT_FM_ZD_UNSELF					256								//256~287，当次非卡加油数据,共32bytes
																			//3HEX	本次加油金额；(偏移0)
																			//3HEX	本次加油油量；(偏移3)
																			//1byte	非卡加油状态；(偏移6)
																			//3HEX	本次累计金额；(偏移7)
																			//3HEX	本次累计油量；(偏移10)
																			//1HEX	过冲次数；(偏移13)
																			//2bytes	CRC校验，计算前30bytes；(偏移30)
																									
#define IPT_FM_ZD_UNSELF2				288								//288~非卡加油数据备份,共32bytes																		

#define IPT_FM_ZD_TM							320								//条码自助加油数据,共32bytes
																				//(偏移0)	1byte枪号；
																				//(偏移1)	4bytes POS_TTC；
																				//(偏移5)	7bytes日期时间；
																				//(偏移12)	3bytes加油金额；
																				//(偏移15)	3bytes加油升数；
																				//(偏移18)	3bytes授权金额；
																				//(偏移21)	5bytes授权验证码；
																				//(偏移26)	2bytes单价；
																				//(偏移28)	2bytes油品代码；
																				//(偏移30)	1byte账单状态0=已上传；1=未上传；
																				//(偏移31)	1byte异或校验码
																									
#define IPT_FM_ZD_TM2						352								//条码自助加油数据备份,共32bytes

#define IPT_FM_OIL_ATUO					384								//是否自动加油

//2017-02-13联达系统直接拔主板的电数据备份
#define IPT_LDPowerData					385	//联达系统直接主板拔电4字节(1字节折扣单位1字节是否折扣卡2字节折扣单价)

#define FM_ETC_FREE_FLG_A				389//ETC释放标志保存1字节+4字节的MAC号,szb_fj20171120
#define FM_ETC_FREE_FLG_B				394//ETC释放标志保存1字节+4字节的MAC号,szb_fj20171120
#define IPT_FM_BILL_FLAG				399//1字节存储iptparam->OilBillSave标志,szb_fj20171120
#define FM_ERR_QUEYILU_PULSE			400//1字节连续缺一路脉冲超过6次锁机,szb_fj20171120
#define FM_ERR_LING_JIAYOU				401//1字节连续零加油超过6次锁机,szb_fj20171120
#define FM_ERR_WU_PULSE					402//1字节连续无脉冲超时超过6次锁机,szb_fj20171120
#define FM_ERR_QUEYIZU_PULSE			403//1字节连续缺一组脉冲超过6次锁机,szb_fj20171120
#define FM_ERR_BIAN_PRICE				404//1字节变价失败标志,szb_fj20171120

//中燃联达卡类型定义
#define IPT_LIANDA_APPTYPE_JIZHANG		0x12			//记账卡
#define IPT_LIANDA_APPTYPE_DAICHU		0x13			//代储卡
#define IPT_LIANDA_APPTYPE_ZIYONG		0x14			//自用卡
#define IPT_LIANDA_APPTYPE_DIAOBO		0x15			//调拨卡
#define IPT_LIANDA_APPTYPE_GUOBIAO	    0x16			//过表卡
#define IPT_LIANDA_APPTYPE_DAICHUSIJI	0x17		//待储卡司机卡

//2017-02-13油品限制方式设置
//#define IPT_SET_OILLIMIT_ZSH			0x30			//中石化标准
//#define IPT_SET_OILLIMIT_JR			0x31			//兼容中石化标准
#define IPT_SET_OILLIMIT_F				0x30			//F标准
#define IPT_SET_OILLIMIT_0				0x31			//0标准
#define IPT_SET_OILLIMIT_0F				0x32			//0F标准

//IC交易明细
typedef struct
{
	unsigned char TTC[2];						//ET联机或脱机交易序号
	unsigned char Limit[3];						//透支限额
	unsigned char Money[4];					//交易金额
	unsigned char Type;							//交易类型标识
	unsigned char TermID[6];				//终端机编号
	unsigned char Time[7];						//交易时间
}IptIcRecordType;

																									

//支付终端(IPT)操作及加油数据
typedef struct
{
	//测试所需信息
#if _IPT_DEBUG_
	unsigned int testOilTimes;
#endif

	//基本信息
	unsigned char Id;								//支付终端号(即面板号), 0=1号枪(IPT_NOZZLE_1);1=2号枪(IPT_NOZZLE_2)
	unsigned char PhysicalNozzle;			//物理枪号，一个通讯终端内部的排序枪号，可设置范围1~6
	
	//MSG_Q_ID MsgIdRx;						//接收消息队列ID
	int MsgIdRx;                            //接收和发送消息的ID

	int tId;												//支付终端处理任务ID
	unsigned int ProcessId;					//处理进程号
	unsigned char Step;							//处理步骤号，某一进程号的分支序号
	unsigned int NousedTimer;				//此计数器累计支付终端无任何操作的时间长度
	unsigned int FMAddrBase;				//铁电存储数据基础地址
	unsigned int UserID;							//用户ID，>0

	//检测电源状态的时间间隔
	unsigned int PowerStateTimer;

	//油枪信息
	unsigned char NozzleNumber;			//逻辑油枪数目
	unsigned char LogicNozzle;				//逻辑机号，即油站大排序枪号,0表示非法
	unsigned char OilVersion;					//后台下发的油品油价表版本
	unsigned char OilPriceBySNPC[2];	//后台下发的价格 HEX
	unsigned char OilCode[2];					//油品代码
	unsigned char OilName[32];				//油品名称
	unsigned char JlNozzle;						//计量枪号，0=1号枪，1=2号枪

	//感应开关信息
	unsigned char DEVPIR;						//感应开关设备选择0=A键盘；1=B键盘

	//是否暂停使用 0 = 否；1 = 是
	unsigned char IsSuspend;

	//加油是否需要认证 ;0=需要DES认证；1=不需要DES认证
	unsigned char DESAuthen;			

	//IPT模式:0=卡机联动，1=非卡机联动，2=条码自助模式
	unsigned char Mode;		

	//操作密码
	unsigned char Password[2];				//油站操作密码2BCD
	unsigned char ServicePass[2];			//售后操作密码2BCD
	unsigned char TempBuffer[74];		//操作中所使用的临时缓存

	//显示设备信息
	unsigned int DEVDsp;						//键盘显示设备ID, 0=1号；1=2号
	unsigned char Contrast;					//键盘显示对比度

	//蜂鸣器设备信息
	unsigned int DEVBuzzer;					//蜂鸣器设备ID，0=A键盘蜂鸣器；1=B键盘蜂鸣器

	//按键设备信息
	unsigned int DEVButton;
	unsigned int Button;							//当前按键

	//钥匙信息
	int KeyLock;										//钥匙状态；0=加油位置；1=设置位置
	int KeyLockChg;									//钥匙状态是否发生变化

	//明细查询时的参数
	unsigned int InqBillTTC;						//查询的账单TTC号
	unsigned char InqBillNozzle;					//查询账单对象；0=整机；非0=物理枪号
	unsigned char InqBillNext;					//0=查询该TTC账单；1=查询该TTC上一笔；2=查询该TTC下一笔
	unsigned char InqBillPage;					//账单查询页码
	unsigned char InqBill[IPT_BILL_SIZE];	//账单

	//油枪开关设备信息
	int DEVGun;										//油枪 DEV_GUNA1/DEV_GUNB1
	int GunState;									//油枪开关状态；0=提枪；1=挂枪
	char GunStateChg;							//油枪开关状态发生变化；0=无变化；1=有变化

	//上下班信息
	unsigned char WorkState;				//上下班状态0=下班；1=上班
	unsigned char EMP;							//上班员工号
	unsigned char EMPPassword[2];		//上班员工密码
	unsigned char EMPCardID[10];			//上班员工卡卡号

	//其它操作相关参数
	unsigned int PassTimer;					//密码随机数计算器，使用此值计算随机数
	unsigned char PassRandom[6];		//密码计算明码随机数

	//夜间锁定相关参数
	unsigned char NightLock;					//0=正常；1=夜间锁定

	//销售锁定相关参数
	unsigned char SLockTime[4];			//启动销售是锁定的时间YYYYMMDD
	unsigned char SellLock;					//0=无锁定；其它=销售锁定

	//操作密码相关参数
	unsigned char SetPassNew[4];		//设置时输入的新密码ASCII，首字节为密码长度
	unsigned char SetPassNew2[4];		//设置时输入的新密码确认ASCII，首字节为密码长度

	//时间相关参数
	RTCTime Time;									//当前时间 
	RTCTime LastTime;							//前一刻时间

	//加油限制相关参数
	unsigned char CardStaffLimit;			//员工卡加油限制0=允许加油；1=不允许加油	

	//查询/设置操作参数
	unsigned char SetPage;					//一级操作页面号，查询/设置选项选择界面
	unsigned char SetPage2;					//次级操作页面号
	unsigned char SetButtonFlag;			//按键标识0=进入操作界面后未按键；1=已有按键
	unsigned char SetButton[16];			//按键
	unsigned char SetButtonLen;			//按键长度
	unsigned int SetParam;					//设置操作时临时存储设置的值，确认设置时将此值赋予实际变量

	//设置有小数的操作或预置加油时的按键
	unsigned char IntegerBuffer[8];		//整数
	unsigned char IntegerLen;				//整数个数
	unsigned char Point;							//小数点 0=无，1=有
	unsigned char DecimalBuffer[2];		//小数
	unsigned char DecimalLen;				//小数个数

	//语音相关参数
	unsigned char Speaker;					//扬声器选择(0x10=A1/0x11=A2/0x20=B1/0x21=B2/0x30=C1/0x31=C2)
	unsigned char VoiceType;					//语音类型0=女声；1=男声
	unsigned char VoiceVolume;				//音量0~99，HEX
	unsigned char VoiceFlag;					//标记某些操作中语音播报的标记
	unsigned char OilVoice[4];				//油品语音编号，ASCII，此选项未配置时则油品根据"接口协议1.1"中油品代码查询对应油品语音

	//打印相关参数
	unsigned short *PrintCard;				//查询/设置时操作的自动打印账单类型卡类指针
	unsigned char Printer;						//打印机选择(0x10=A1/0x11=A2/0x20=B1/0x21=B2/0x30=C1/0x31=C2)
	unsigned char PrintAuto;					//自动打印标识0=不自动打印，1=自动打印
	unsigned char PrintUnion;					//0=打印一联(即客户联)；1=打印两联(加油站联及客户联)
	unsigned short PrintCardUser;			//用户卡自动打印账单类型0=不打印；1=自动打印
											//b0=正常；b1=逃卡；b2=错卡；b3=补扣；b4=补充；b5=上班；
											//b6=下班；b7=非卡；b8=油价回应；b9=卡交易出错
	unsigned short PrintCardManage;	//管理卡自动打印账单类型，详细设置同上
	unsigned short PrintCardStaff;			//员工卡自动打印账单类型，详细设置同上
	unsigned short PrintCardPump;		//验泵卡自动打印账单类型，详细设置同上
	unsigned short PrintCardService;		//维修卡自动打印账单类型，详细设置同上

	//绑定信息
	unsigned char BindTime[7];				//绑定时间
	unsigned char BindMboardId[8];		//绑定芯片ID
	unsigned char BindACTAppId[10];	//绑定ACT卡号
	unsigned char BindRIDAppId[10];	//绑定RID卡号

	//条码自助授权加油信息
	unsigned char BarUserID;				//当前油枪用户ID，范围>=1
	unsigned char BarOilBill[32];				//条码自助账单
	unsigned char BarBillUpload;			//是否有条码加油账单需要上传0=无；1=有
	unsigned char CardWhite[10];			//内置卡且白名单卡卡号
	unsigned char DEVICInternal;			//内置卡所连接设备选择
	unsigned char BarcodeSwitch;			//油品选择开关DEV_SWITCH_SELA1/DEV_SWITCH_SELB1
	unsigned char DEVBarcode;				//条码模块设备BARCODE_NOZZLE_1/BARCODE_NOZZLE_2
	unsigned char Barcode[5];				//条码压缩BCD，不足后补F
	unsigned char BarMoney[3];				//条码授权金额
	unsigned char InvoiceMark;				//发票打印标识
	unsigned char BarOilCode[2];			//条码授权油品
	unsigned char BarZDState;				//条码自助账单状态	0=正常；1=未上传
	unsigned char CPOSFlag;					//条码自助后台通讯标识0=无操作，1=已发送数据
	unsigned char CPOSOvertimes;		//条码自助后台通讯超时次数
	unsigned int CPOSTimer;					//条码自助后台通讯定时器

	//促销机相关数据，因ip信息等在查询设置时需要使用故也做保存，但并非实时有效数据
	unsigned char TabletPanel;					//平板电脑面板号 PC_PANEL_1/PC_PANEL_2
	unsigned char TabletIP[16];					//平板电脑IP地址
	unsigned char TabletMask[16];			//平板电脑子网掩码
	unsigned char TabletGateway[16];		//平板电脑网关
	unsigned char TabletFirstDNS[16];		//平板电脑首选DNS
	unsigned char TabletSecondDNS[16];	//平板电脑备用DNS
	unsigned char TabletFtpIP[16];			//平板电脑连接的FTP服务器IP地址
	unsigned char TabletFtpPort[16];			//平板电脑连接的FTP服务器端口号
	unsigned char TabletTxElement;			//加油中上传给平板电脑信息的内容
	unsigned char TabletGradeTag;			//娱乐机是否启用油品确认功能
	unsigned char IptOilLimitStyleTag;			//2017-02-13油品限制方式设置
		
	int DEVPlayButton;								//PLAY按钮设备号
	int ValuePlayButton;								//PLAY按钮动作 0 =无操作；1 = 有按钮操作

	unsigned char PlayLongLast;				//PLAY按钮前一刻状态 0 = 按下状态；1 = 抬起状态；
	unsigned char PlayLong;						//PLAY按钮是否有按下的操作 0 =无；1 = 有
	unsigned int PlayLongTimer;				//PLAY按钮按下的持续时间
	unsigned short TaState;							//状态
	unsigned char TaStateParam[128];		//参数
	int TaStateParamLength;						//参数长度

	//报警信息
	unsigned char ErrInfoBytes;				//报警信息长度
	unsigned char ErrInfo[128];				//报警信息数据

	//人体感应数据
	int DEVSensor;								//人体感应数据
	char SensorStateLast;					//前一刻人体感应状态
	char SensorState;							//当前人体感应状态

	//外部请求相关数据
	unsigned char PassInputAsk;			//请求转入授权ETC卡密码输入界面 0=无请求;1=通知油机开始输入密码;2=通知油机取消输入密码
	unsigned char AuthorizeAsk;			//请求对加油机授权 0=无请求；1=请求授权；2=请求取消授权
	unsigned char AuthorizeData[64];	//授权数据
	unsigned char CardDebitAsk;			//请求加油机进行卡支付 0 = 无请求；1 = 进入支付流程；2 = 退出支付流程；3 = 请求卡支付；
	unsigned char CardDebitData[32];	//请求卡支付的数据; 流水号(16bytes、后补空格) + 金额(3bytes、HEX)
	unsigned int CardDebitAmount;		//请求卡支付的金额

	unsigned char CardPresetAsk;			//卡加油预置请求 0xFF = 无；1 = 有
	unsigned int CardPresetValue;			//卡加油预置额
	unsigned char CardPresetMode;		//卡加油预置方式 0 = 预置升数；1 = 预置金额
	unsigned char CardPresetRep;			//卡加油预置请求结果 0xFF = 无；0 = 成功；1 = 失败；
	
	unsigned char CardShootAsk;			//是否有退卡请求 0xFF = 无；1 = 有；
	unsigned char CardShootRep;			//退卡请求结果 0xFF = 无结果；0 = 成功；1 = 失败；

	unsigned char CardAppSelectAsk;	//卡应用选择请求 0xFF = 无；0 = 电子油票；1 = 积分应用；
	unsigned char CardAppSelectRep;	//卡应用选择请求结果 0xFF = 无结果；0 = 成功； 1 = 失败；

	unsigned char CardTypeSelectAsk;	//卡支付方式选择请求 0xFF = 无；0 = 现金；1 = 油票；2 = 提油凭证；3 = 银行卡；4 = 其它一；5 = 其它二
	unsigned char CardTypeSelectRep;	//卡支付方式选择请求结果 0xFF = 无结果；0 = 成功； 1 = 失败；

	unsigned char TMScanAsk;				//条码操作请求，0 = 无操作；1 = 启动条码扫描；2 = 退出条码扫描
	unsigned char TMScanRep;				//条码操作请求结果，0xFF = 无结果；0 = 成功；1= 失败
	unsigned char TMNumberLenght;	//促销系统通知油机外部输入的条码位数 HEX
	unsigned char TMNumber[10];			//促销系统通知油机外部输入的条码, ASCII
	unsigned char TMExterInputDone;	//促销系统通知油机外部输入条码是否完毕 0 = 未完；1 = 正在输入；2 = 完毕

	//PCD参数
	unsigned char PcdState;					//IPT与PCD通讯状态0=通讯断开；1=通讯正常，通讯正常时PCD数据有效
	unsigned char PcdTxFlag;					//IPT及PCD通讯标识0=无占用；1=流程内部命令已发送；2=普通命令已发送
	unsigned char PcdTxFrame;				//IPT与PCD通讯帧号
	unsigned int PcdTxTimer;					//IPT与PCD通讯超时定时器
	unsigned char PcdSaveOverTimes;	//IPT与PCD通讯超时次数，适用于账单存储过程中
	unsigned char PcdOverTimes;			//IPT与PCD通讯超时次数，适用于过程中
	unsigned char PcdPollOverTimes;	//IPT与PCD通讯超时次数，适用于轮询过程中
	unsigned char BlistSrc;						//查询黑/白名单源0=使用后台黑/白名单；1=使用油机内黑/白名单
	unsigned char PcOnline;					//石化后台联网状态0=未连接，1=通讯正常
	unsigned char PcdErrNO;					//PCD异常状态0=正常；其它=非法
	unsigned int BillTTC;							//整机当前账单TTC
	unsigned int FuelBillTTC;					//本枪当前账单TTC
	unsigned int UnloadNumber;			//整机未上传账单数量
	unsigned int FuelUnloadNumber;		//本枪未上传账单数量

	//当次加油数据
	unsigned char YPSelect;					//是否有油品确认操作 0 = 无；1 = 有；
	unsigned char BarBillPrintRepeat;	//本次条码自助加油账单是否还可通过快捷键重打印 0=不可以；1=可以
	unsigned char OilBillPrintRepeat;	//本次IC卡加油账单是否还可通过快捷键重打印 0=不可以；1=可以
	unsigned char OilBill[IPT_BILL_SIZE];//加油账单
	unsigned char OilBillSave;					//是否有账单需要保存:0=无;1=有
	unsigned char OilBillSaveStep;			//账单的存储步骤
	unsigned int OilDspTimer;					//加油数据显示间隔定时器
	unsigned int WarnigBeepTimer;		//出错报警音报警间隔定时器

	unsigned int PriceChgTimer;				//根据后台油品变价时，如果失败则间隔一段时间后在进行下一次变价尝试
	unsigned char OilIcState[2];				//加油插卡状态,见"接口协议1.1.pdf"文件附录9
	unsigned char OilState;					//加油状态:0=空闲;1=启动中;2=加油中;3=结束中
	unsigned char OilTime[7];					//当次交易日期时间
	unsigned int PresetMode;					//预置方式；0=任意；1=预置油量；2=预置金额
	unsigned long long PresetVolume;	//预置量油量，单位0.01升，计算时可能会有超过4字节的情况，故此处采用长整形
	unsigned long long PresetMoney;		//预置量金额，单位0.01元，计算时可能会有超过4字节的情况，故此处采用长整形
	unsigned long long SumMoney;		//总累金额
	unsigned long long SumVolume;		//总累油量
	unsigned int OilMoney;						//当次加油金额
	unsigned int OilVolume;					//当次加油量
	unsigned int OilPrice;						//油品价格
	unsigned char OilRound;					//是否有有效凑整操作0=无；1=有
	unsigned char JlStopNO;					//计量停机代码
	unsigned char IcValid;						//0=无卡或卡未处理，1=已读出余额
	unsigned int OilEndTimer;					//上次加油结束时间间隔

	unsigned char ICStateFirst;				//卡插入时的状态0=正常；0x01=灰卡；0x10=TAC未清
	unsigned char DS;								//扣款来源，0=石油卡电子油票；1=石油积分区积分；2=金融卡电子钱包；3=金融卡电子存折
	unsigned char PayUnit;						//结算单位选择0=电子油票，1=积分
	unsigned char Payment;					//结算方式0=现金,1=油票,2=记账,3=银行卡,4=其它1,	5=其它2
	unsigned char C_TYPE;						//卡类b0:0=石化规范卡；1=PBOC金融卡
	unsigned char UserElecFlag;				//用户卡插入后是否已通过确认键释放静电 0=未操作；1=已操作

	//联达加油站相关参数
	unsigned char StationName[30+1];	//加油站名称，最多30字节的字符串数据
	unsigned char InvoiceType;				//当次加油开票类型，01H：消费开发票；02H：充值开发票；03H：统一开增值税发票；
																
	unsigned short PriceDiscount;			//价格折扣额
	unsigned int OilMoneyDiscount;		//折扣金额，即折扣后加油金额(代储卡此字段以升数存储)

	//非卡加油及过冲加油复用的数据
	unsigned int MoneyUnself;				//本次非卡加油金额或过冲金额
	unsigned int VolumeUnself;				//本次非卡加油升数或过冲升数
	unsigned int MoneyUnselfSum;		//非卡加油累计金额或过冲累计金额
	unsigned int VolumeUnselfSum;		//非卡加油累计油量或过冲累计油量
	unsigned int OilStateUnself;				//非卡加油状态0=空闲；1=加油中
	unsigned char OilOverTimes;			//连续过冲加油次数

	//IC卡信息
	unsigned char DEVIC;						//IC卡设备号0=1号卡座；1=2号卡座
	unsigned char IcOverTimes;				//IC卡操作超时次数
	IcStateType IcState;						//IC卡状态
	unsigned char IcPassword[12];		//IC卡密码(ASCII，最大12位)
	unsigned char IcPasswordLen;		//IC卡密码数据字节长度(0~12)
	unsigned char DEVPsam;					//PSAM卡卡座号 '0'~'4'表示卡座的PSAM1~PSAM4；0x00~0x01表示键盘的PSAM0~PSAM1

	//补扣时加油信息
	unsigned int unlockMoney;				//补扣，金额
	unsigned char unlockCTC[2];			//补扣，CTC
	//unsigned char unlockDS;				//补扣，扣款来源
	unsigned char unlockGMAC[4];			//补扣，解灰认证码
	unsigned char unlockPsamTID[6];	//补扣，PSAM应用编号
	unsigned char unlockPsamTTC[4];	//补扣，PSAM的TTC


	//ACT卡认证操作过程参数
	//ACT卡卡号
	unsigned char ACTAppId[10];						//ACT卡卡号
	//__ACT卡26文件持卡人基本数据文件
	unsigned char ACTKeyIndex;						//认证密钥索引号(ACT索引号)
	unsigned char ACTUnused;							//保留
	unsigned char ACTUserName[20];				//持卡人名
	unsigned char ACTUserID[18];						//持卡人证件号码
	unsigned char ACTUserIDType;					//持卡人证件类型
	//__PSAM随机数
	unsigned char ACTPSAMRandom[4];			//PSAM随机数
	//__ACT卡密文数据
	unsigned char ACTCiphertext[8];					//密文数据

	//RID卡认证操作过程参数
	//__RID卡26文件持卡人基本数据文件
	unsigned char RIDKeyIndex;						//认证密钥索引号(RID索引号)
	unsigned char RIDCalKeyIndex;					//计算密钥索引号( 日志MAC计算)
	unsigned char RIDUserName[20];				//持卡人名
	unsigned char RIDUserID[18];						//持卡人证件号码
	unsigned char RIDUserIDType;					//持卡人证件类型
	//__PSAM随机数
	unsigned char RIDPSAMRandom[4];			//PSAM随机数
	//__RID卡密文数据
	unsigned char RIDCiphertext[8];					//密文数据
	//__RID认证MAC
	unsigned char RIDMAC[4];				

	//IC卡加油消费过程参数
	//__PSAM卡21文件卡片公用信息
	unsigned char PsamId[10];							//PSAM序列号
	unsigned char PsamVersion;							//PSAM版本号
	unsigned char PsamKeyType;						//PSAM密钥卡类型
	unsigned char PsamCodeVersion;				//PSAM指令集版本
	unsigned char PsamFCI;								//PSAM发卡方自定义FCI数据
	//__PSAM卡22文件MF终端信息(SELECT  PSE)
	unsigned char PsamTermId[6];						//PSAM终端机编号
	//__PSAM卡23文件应用公共信息(SELECT  ADF)
	unsigned char PsamKeyIndex;						//PSAM消费密钥索引号
	unsigned char PsamIssuerMark[8];				//PSAM应用发卡方标识
	unsigned char PsamRecipientsMark[8];		//PSAM应用接收者标识
	unsigned char PsamAppStartTime[4];			//PSAM应用启用日期
	unsigned char PsamAppEndTime[4];			//PSAM应用有效截止日期
	//__IC卡21文件公共应用基本数据
	unsigned char IcIssuerMark[8];					//发卡方标识
	unsigned char IcAppMatk;								//应用类型标识
	unsigned char IcAppVersion;						//应用版本
	unsigned char IcAppId[10];							//应用序列号
	unsigned char IcEnableTime[4];					//应用启用日期
	unsigned char IcInvalidTime[4];					//应用有效截止日期
	unsigned char IcCodeVersion;						//指令集版本
	unsigned char IcFile21Unused;					//21文件备用区域
	//__IC卡22文件持卡人基本数据
	unsigned char IcTypeMark;							//卡类型标识
	unsigned char IcStaffMark;							//本系统职工标识
	unsigned char IcUserName[20];					//持卡人名
	unsigned char IcUserIdeId[18];					//持卡人证件(identity)号码(ASCII)
	unsigned char IcUserIdeType;						//持卡人证件类型
	//__IC卡27文件ET普通信息数据
	unsigned char IcDefaultPassword;				//是否采用默认密码,00=使用默认密码，01=使用用户密码
	unsigned char IcStaffId;								//员工号(内部卡有效)
	unsigned char IcStaffPassword[2];				//员工密码(内部卡有效)
	unsigned char IcDebitUnit;							//扣款单位(00H=元；01H=升；社会站CPU卡结构有此字段)
	unsigned char IcDiscountFlag;						//是否折扣卡(00H=非折扣卡；01H=折扣卡；社会站CPU卡结构有此字段)
	unsigned char IcAppType;								//卡片应用子类型(01H = 用户卡；04H = 员工卡；05H = 验泵卡；21H = 用户卡主卡；
																            //22H = 代储卡主卡；11H = 用户卡司机卡；12H = 记账卡；13H = 代储卡；
																			//14H = 自用卡；15H = 调拨卡；16H = 过表卡；17H = 代储卡司机卡；
																			//社会站CPU卡结构有此字段)
	//__IC卡28文件敏感信息数据
	unsigned char IcOilLimit[2];							//油品限制
	unsigned char IcRegionTypeLimit;				//限地区,油站加油方式
	unsigned char IcRegionLimit[40];				//限地区,油站加油
	unsigned char IcVolumeLimit[2];					//限每次加油量
	unsigned char IcTimesLimit;							//限每天加油次数
	unsigned char IcMoneyDayLimit[4];			//限每天加油金额
	unsigned char IcCarIdLimit[16];					//车牌号限制(ASCII)
	//__IC卡历史交易明细				
	unsigned char IcRecordNumber;					//IC交易明细条目
	IptIcRecordType IcRecord[10];						//交易明细，倒查，故存储的第一笔即为最晚一笔加油数据
	//__IC卡灰锁信息
	unsigned char IcLockMark;							//状态字:0x00=无灰锁；0x01=已灰锁；0x10=TAC未读
	unsigned char IcLockType;						//
																		//	0x00:上次发生解扣或联机解扣的交易类型标识；
																		//	0x01:灰锁的交易类型标识；
																		//	0x10:上次解扣的交易类型标识；
	unsigned char IcLockET;								//
																		//	0x00:上次发生解扣或联机解扣的ET；
																		//	0x01:被灰锁的ET；
																		//	0x10:上次解扣的ET；
	unsigned char IcLockBalance[4];				//
																		//	0x00:上次发生解扣或联机解扣的ET余额；
																		//	0x01:被灰锁的ET余额；
																		//	0x10:上次解扣的ET余额；
	unsigned char IcLockCTC[2];						//
																		//	0x00:上次发生解扣的ET的脱机交易序号
																		//			或联机解扣的ET的联机交易序号；
																		//	0x01:被灰锁的ET脱机交易序号；
																		//	0x10:上次解扣的ET脱机交易序号；
	unsigned char IcLockTermId[6];				//
																		//	0x00:上次发生解扣或联机解扣的终端机编号；
																		//	0x01:执行GREY LOCK时的终端机编号；
																		//	0x10:上次解扣的终端机编号；
	unsigned char IcLockTime[7];					//
																		//	0x00:上次发生解扣或联机解扣的日期时间；
																		//	0x01:执行GREY LOCK时的日期时间；
																		//	0x10:上次解扣的日期时间；
	unsigned char IcLockMoney[4];				//
																		//	0x00:上次发生解扣或联机解扣的交易金额；
																		//	0x01:灰锁时的MAC2；
																		//	0x10:上次解扣的交易金额；
	unsigned char IcLockGTAC[4];					//
																		//	0x00:上次发生解扣的TAC或联机解扣的MAC3；
																		//	0x01:灰锁时的GTAC；
																		//	0x10:上次解扣的TAC；
	//__IC卡余额信息
	unsigned char IcBalance[4];							//卡余额信息
	//__PSAM安全提升剩余次数
	unsigned char PsamProofTimes;					//PSAM剩余认证次数
	//__PSAM随机数通过的DES计算的密文
	unsigned char DESCiphertext[8];					//DES计算密文
	//__IC卡灰锁初始化信息
	unsigned char IcLockInitBalance[4];			//灰锁初始化返回:卡余额
	unsigned char IcLockInitCTC[2];					//灰锁初始化返回:脱机交易序号
	unsigned char IcLockInitOverdraw[3];		//灰锁初始化返回:透支限额
	unsigned char IcLockInitKeysVersion;		//灰锁初始化返回:密钥版本号
	unsigned char IcLockInitArithmetic;			//灰锁初始化返回:算法标识
	unsigned char IcLockInitRandom[4];			//灰锁初始化返回:伪随机数
	//__PSAM计算MAC1
	unsigned char IcPsamTTC[4];						//PSAM返回:终端交易序号
	unsigned char IcPsamRandom[4];				//PSAM返回:终端随机数
	unsigned char IcPsamMAC1[4];					//PSAM返回:MAC1
	//__IC卡灰锁
	unsigned char IcGTAC[4];								//IC灰锁返回:GTAC
	unsigned char IcMAC2[4];								//IC灰锁返回:MAC2
	//__加油过程
	//__PSAM计算GMAC
	unsigned char IcPsamGMAC[4];					//PSAM返回:GMAC
	unsigned char IcPsamTAC[4];						//PSAM返回:TAC
	//__IC卡解扣返回数据
	unsigned char IcTac[4];									//IC解扣返回:TAC

	//ETC消费功能数据,szb_fj20171120
	unsigned char etc_set_flg;						//ETC功能设置,szb_fj20171120
	unsigned int etc_rec_len;						//接收到的数据长度,szb_fj20171120
	//unsigned char etc_rec_buff[ETC_DATA_MAX];		//接收到的数据缓存,szb_fj20171120
	unsigned char etc_rec_buff[600];                //fj20171120
	unsigned char EtcOilFlg;						//ETC加油标志,szb_fj20171120
	//unsigned char EtcListInf[ETCCARDLEN*20];		//ETC车辆列表信息,szb_fj20171120
    unsigned char EtcListInf[26*20];                 //fj20171120
	//16字节的车牌号，4字节的MAC，2字节的随机数，1字节的密码标志,szb_fj20171120
	unsigned char EtcListNum;						//车辆信息个数,szb_fj20171120
	//unsigned char EtcSelCardInf[ETCCARDLEN];		//选定的车号信息,szb_fj20171120
	unsigned char EtcSelCardInf[26];
	unsigned char EtcTxCi;							//ETC发送次数,szb_fj20171120
	unsigned char EtcFreeflag;						//OBU选定标志,szb_fj20171120
	//unsigned char etc_09_buff[ETC_CARD_MAX];		//卡信息缓存,szb_fj20171120
	unsigned char etc_09_buff[1024];
	unsigned int etc_09_len;						//卡信息长度,szb_fj20171120
	unsigned char etc_09_num;						//包号,szb_fj20171120
	unsigned char etc_card_num;						//etc卡信息命令号,szb_fj20171120
	unsigned char etc_touming_flg;					//是否采用透明传输标志,szb_fj20171120
	unsigned char etc_limit_car;					//ETC限车号标志,szb_fj20171120
	unsigned char etc_recnet_oil[10];				//卡上次加的油品名称显示,szb_fj20171120
	unsigned char etc_now_oil[10];					//卡现在加的油品名称显示,szb_fj20171120
	unsigned char EtcTxFlg;							//ETC发送标志,szb_fj20171120
	unsigned char etc_update_flag;					//更新油品标志,szb_fj20171120
	unsigned int EtcTxTime;							//ETC发送时间,szb_fj20171120
	unsigned char etc_yue_dis_flag;					//余额显示标志,szb_fj20171120
	unsigned char EtcFreeObuflg;					//释放OBU标志,szb_fj20171120
	unsigned char EtcFreeObuCi;						//释放OBU次数,szb_fj20171120
	unsigned int EtcFreeObuTime;					//释放OBU时间,szb_fj20171120

	unsigned char JlErr_QYL;						//异常停机缺一路脉冲,szb_fj20171120
	unsigned char JlErr_ZERO;						//异常停机零加油,szb_fj20171120
	unsigned char JlErr_WMCCS;						//异常停机无脉冲超时,szb_fj20171120
	unsigned char JlErr_QYZ;						//异常停机缺一组脉冲,szb_fj20171120
	unsigned char JlErr_Freebuff[3];				//解除锁定缓存,szb_fj20171120

	unsigned char JlErr_BianJia;					//计量变价异常标志,szb_fj20171120
}IptParamStructType;

//当前IPT模块读取到的一些状态，参数等
extern IptParamStructType IptParamA, IptParamB;
//205的IAP与应用程序版本
extern unsigned char Ipt205IAP_Ver[2];
extern unsigned char Ipt205APP_Ver[2];

//PC信息
extern PcdPcInfoType IptPcInfo;

//函数声明
//extern int iptPidPrint(void);   //fj:没有调用,20170914,都已删除
//extern int iptMainUserGet(void);//fj:无用函数
//extern int iptPanelFromUserID(int userid); //fj:无调用函数

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


extern int iptMainUserSet(int user);  //fj:内部函数


extern int iptPhysicalNozzleRead(int nozzle);


//extern MSG_Q_ID iptMsgIdRead(int nozzle);
extern int iptMsgIdRead(int nozzle);   //fj:20170914

extern int iptPhysicalNozzleRead(int nozzle);
extern bool iptInit(void);
extern int iptIsLianda(int nozzle); //2017-01-22中燃联达灰卡解扣

void iptTask();

//计量连续超过6次异常停机处理
extern int iptAbnormalStopHandle(IptParamStructType *iptparam,unsigned char stopcode,unsigned int money);

#endif


