#ifndef _OIL_SPK_H_
#define _OIL_SPK_H_

//采样频率
#define SAMPL_40K    		0x672		//40K
#define SAMPL_64K    		0x407		//采样率60K
#define SAMPL_80K    		0x339		//采样率80K
#define SAMPL_128K   		0x204		//采样率128K
#define SAMPL_256K			0x102		//采样率256K
#define SAMPL_512K			0x81			//采样率512K
#define SAMPL_1024K		0x40			//采样率1024K


//语音文件夹路径
//#define SPK_PATH							"/flash/voice"
#define SPK_PATH							"../config/voice"

//允许播放的语音内容总计最大字节数
#define SPK_SUM_MAX_SIZE		0x20000		//128K

//语音播放段数目
#define SPK_MAX_NUMBER			32

//扬声器枪号
#define SPK_NOZZLE_1						0					//1号扬声器
#define SPK_NOZZLE_2						1					//2号扬声器

//扬声器选择
#define SPK_DEVICE_LOCAL				0					//本地扬声器
#define SPK_DEVICE_PC					1					//平板电脑设备扬声器


#if APP_SINOPEC

//促销机语音播放列表，女声
#define SPKW_0									0x0000			//0
#define SPKW_1									0x0001			//1
#define SPKW_2									0x0002			//2
#define SPKW_3									0x0003			//3
#define SPKW_4									0x0004			//4
#define SPKW_5									0x0005			//5
#define SPKW_6									0x0006			//6
#define SPKW_7									0x0007			//7
#define SPKW_8									0x0008			//8
#define SPKW_9									0x0009			//9
#define SPKW_GASONLINE				0x0010			//汽油
#define SPKW_GAS90							0x0011			//90号汽油
#define SPKW_GAS90CLEAN				0x0012			//90号清洁汽油
#define SPKW_GAS90LEAD				0x0013			//90号无铅汽油
#define SPKW_GAS93							0x0014			//93号汽油
#define SPKW_GAS93CLEAN				0x0015			//93号清洁汽油
#define SPKW_GAS93LEAD				0x0016			//93号无铅汽油
#define SPKW_GAS93ETHANOL			0x0017			//93号乙醇汽油
#define SPKW_GAS95							0x0018			//95号汽油
#define SPKW_GAS95CLEAN				0x0019			//95号清洁汽油
#define SPKW_GAS95LEAD				0x0020			//95号无铅汽油
#define SPKW_GAS95AVIATION		0x0021			//95号航空汽油
#define SPKW_GAS97							0x0022			//97号汽油
#define SPKW_GAS97CLEAN				0x0023			//97号清洁汽油
#define SPKW_GAS97LEAD				0x0024			//97号无铅汽油
#define SPKW_GAS97ETHANOL			0x0025			//97号乙醇汽油
#define SPKW_GAS98							0x0026			//98号汽油
#define SPKW_GAS98CLEAN				0x0027			//98号清洁汽油
#define SPKW_GAS98LEAD				0x0028			//98号无铅汽油
#define SPKW_GAS120						0x0029			//120号汽油
#define SPKW_GAS200						0x0030			//200号汽油
#define SPKW_GASAVIATION			0x0031			//航空汽油
#define SPKW_GAS75AVIATION		0x0032			//75号航空汽油
#define SPKW_GAS100AVIATION		0x0033			//100号航空汽油
#define SPKW_GASCAR						0x0034			//车用汽油
#define SPKW_GASCAROTHER			0x0035			//其它车用汽油
#define SPKW_GASAVIATIONOTH		0x0036			//其它航空汽油
#define SPKW_DIESELFUEL				0x0037			//柴油
#define SPKW_DIE0							0x0038			//0号柴油
#define SPKW_DIE05MINUS				0x0039			//-5号柴油
#define SPKW_DIE10MINUS				0x0040			//-10号柴油
#define SPKW_DIE15MINUS				0x0041			//-15号柴油
#define SPKW_DIE40MINUS				0x0042			//-40号柴油
#define SPKW_DIE20MINUS				0x0043			//-20号柴油
#define SPKW_DIE30MINUS				0x0044			//-30号柴油
#define SPKW_DIE35MINUS				0x0045			//-35号柴油
#define SPKW_DIE50MINUS				0x0046			//-50号柴油
#define SPKW_DIE05PLUS					0x0047			//+5号柴油
#define SPKW_DIE10PLUS					0x0048			//+10号柴油
#define SPKW_DIE15PLUS					0x0049			//+15号柴油
#define SPKW_DIE20PLUS					0x0050			//+20号柴油
#define SPKW_DIEARMY					0x0051			//军用柴油
#define SPKW_DIE10ARMY				0x0052			//+10号军用柴油
#define SPKW_DIE20ARMY				0x0053			//+20号军用柴油
#define SPKW_DIE30ARMY				0x0054			//+30号军用柴油
#define SPKW_DIEHEAVY					0x0055			//重柴油
#define SPKW_DIE10HEAVY				0x0056			//10号重柴油
#define SPKW_DIE20HEAVY				0x0057			//20号重柴油
#define SPKW_DIELIGHTOTH				0x0058			//其它轻柴油
#define SPKW_DIEHEAVYOTH			0x0059			//其它重柴油
#define SPKW_ONE								0x0060			//个
#define SPKW_TEN								0x0061			//十
#define SPKW_HUNDRED					0x0062			//百
#define SPKW_THOUSAND					0x0063			//千
#define SPKW_TENTHOUSAND			0x0064			//万
#define SPKW_YUAN							0x0065			//元
#define SPKW_SHENG							0x0066			//升
#define SPKW_FEN								0x0067			//分
#define SPKW_POINT							0x0068			//点
#define SPKW_JIAO							0x0069			//角
#define SPKW_PASSIN						0x0070			//请输入密码
#define SPKW_PASERR						0x0071			//密码错误请重新输入
#define SPKW_CARDVALID				0x0072			//无效卡
#define SPKW_OILACK						0x0073			//泵码已回零，请确认
#define SPKW_FAULT							0x0074			//出现故障请通知工作人员
#define SPKW_OILFILL						0x0075			//此油枪加注
#define SPKW_MONEY						0x0076			//金额
#define SPKW_NUMBEROIL				0x0077			//号油品
#define SPKW_START							0x0078			//开始
#define SPKW_RETURN						0x0079			//返回
#define SPKW_CANCEL						0x0080			//取消
#define SPKW_EXACTLY						0x0081			//整
#define SPKW_PAYFOR						0x0082			//消费
#define SPKW_THISVOLUME				0x0083			//您此次的加油量是
#define SPKW_THANKSSEEYOU			0x0084			//谢谢惠顾,祝您一路平安,欢迎下次光临
#define SPKW_THANKSSEEYOU2		0x0085			//谢谢惠顾,祝您一路平安,欢迎下次光临
#define SPKW_SEEYOU						0x0086			//祝您一路平安,欢迎下次光临
#define SPKW_TOTAL							0x0087			//累计
#define SPKW_TOTALMONEY				0x0088			//累计金额
#define SPKW_ICORCODE					0x0089			//请插入IC卡或选择油品后扫描条码
#define SPKW_CODESCAN					0x0090			//请扫描条码
#define SPKW_AUTHORIZEMONEY	0x0091			//本次加油授权金额为
#define SPKW_CODEERROR				0x0092			//扫描错误，请重新扫码
#define SPKW_CODEEXPIRED			0x0093			//条码小票已过期，请到营业室处理
#define SPKW_CODEINAGINE			0x0094			//验证码错误请重新输入
#define SPKW_SELECTERR					0x0095			//油品选择错误，请选择对应的油品
#define SPKW_GUNUSING					0x0096			//此油枪正在使用，请稍候
#define SPKW_OILEND						0x0097			//本次加油已完成，请放回油枪
#define SPKW_OILBALANCE				0x0098			//本次加油尚有余额，请凭打印票据到营业室退款
#define SPKW_ICORMONEY				0x0099			//请插入IC卡或选择油品后投入纸币
#define SPKW_MONEYIN					0x0100			//请投五至十元纸币，投币结束后请按确认按钮
#define SPKW_THISMONEYIN			0x0101			//本次投币金额为
#define SPKW_OILPLEASE					0x0102			//请提枪加油或以定量方式加油
#define SPKW_GAS92							0x0103			//92号汽油
#define SPKW_GAS92CLEAN				0x0104			//92号清洁汽油
#define SPKW_GAS92LEAD				0x0105			//92号无铅汽油
#define SPKW_GAS92ETHANOL			0x0106			//92号乙醇汽油
#define SPKW_GAS89							0x0107			//89号汽油
#define SPKW_GAS89CLEAN				0x0108			//89号清洁汽油
#define SPKW_GAS89LEAD				0x0109			//89号无铅汽油
#define SPKW_GAS89ETHANOL			0x0110			//89号乙醇汽油
#define SPKW_GAS89METHANOL		0x0111			//89号甲醇汽油
#define SPKW_GAS90METHANOL		0x0112			//90号甲醇汽油
#define SPKW_GAS92METHANOL		0x0113			//92号甲醇汽油
#define SPKW_GAS93METHANOL		0x0114			//93号甲醇汽油
#define SPKW_GAS95METHANOL		0x0115			//95号甲醇汽油
#define SPKW_GAS97METHANOL		0x0116			//97号甲醇汽油
#define SPKW_GAS98METHANOL		0x0117			//98号甲醇汽油
#define SPKW_WELCOME			0x0118			//欢迎光临，请选择服务内容
#define SPKW_OILSELECT			0x0119			//请选择对应的油品
#define SPKW_GUNON				0x0120			//请提枪加油
#define SPKW_ICCARDIN			0x0121			//请插入IC卡
#define SPKW_CODEPAYFOR			0x0122			//请插入IC卡或扫描条码
#define SPKW_DIE0CAR			0x0123			//0号车用柴油
#define SPKW_ACK				0x0124			//请确认

#define SPKW_LIMIT_CAR          0x0126          //号枪限车号卡加油，szb_fj20171120

//促销机语音播放列表，男声
#define SPKM_0									0x0200			//0
#define SPKM_1									0x0201			//1
#define SPKM_2									0x0202			//2
#define SPKM_3									0x0203			//3
#define SPKM_4									0x0204			//4
#define SPKM_5									0x0205			//5
#define SPKM_6									0x0206			//6
#define SPKM_7									0x0207			//7
#define SPKM_8									0x0208			//8
#define SPKM_9									0x0209			//9
#define SPKM_GASONLINE				0x0210			//汽油
#define SPKM_GAS90							0x0211			//90号汽油
#define SPKM_GAS90CLEAN				0x0212			//90号清洁汽油
#define SPKM_GAS90LEAD				0x0213			//90号无铅汽油
#define SPKM_GAS93							0x0214			//93号汽油
#define SPKM_GAS93CLEAN				0x0215			//93号清洁汽油
#define SPKM_GAS93LEAD				0x0216			//93号无铅汽油
#define SPKM_GAS93ETHANOL			0x0217			//93号乙醇汽油
#define SPKM_GAS95							0x0218			//95号汽油
#define SPKM_GAS95CLEAN				0x0219			//95号清洁汽油
#define SPKM_GAS95LEAD				0x0220			//95号无铅汽油
#define SPKM_GAS95AVIATION		0x0221			//95号航空汽油
#define SPKM_GAS97							0x0222			//97号汽油
#define SPKM_GAS97CLEAN				0x0223			//97号清洁汽油
#define SPKM_GAS97LEAD				0x0224			//97号无铅汽油
#define SPKM_GAS97ETHANOL			0x0225			//97号乙醇汽油
#define SPKM_GAS98							0x0226			//98号汽油
#define SPKM_GAS98CLEAN				0x0227			//98号清洁汽油
#define SPKM_GAS98LEAD				0x0228			//98号无铅汽油
#define SPKM_GAS120						0x0229			//120号汽油
#define SPKM_GAS200						0x0230			//200号汽油
#define SPKM_GASAVIATION			0x0231			//航空汽油
#define SPKM_GAS75AVIATION		0x0232			//75号航空汽油
#define SPKM_GAS100AVIATION		0x0233			//100号航空汽油
#define SPKM_GASCAR						0x0234			//车用汽油
#define SPKM_GASCAROTHER			0x0235			//其它车用汽油
#define SPKM_GASAVIATIONOTH		0x0236			//其它航空汽油
#define SPKM_DIESELFUEL				0x0237			//柴油
#define SPKM_DIE0							0x0238			//0号柴油
#define SPKM_DIE05MINUS				0x0239			//-5号柴油
#define SPKM_DIE10MINUS				0x0240			//-10号柴油
#define SPKM_DIE15MINUS				0x0241			//-15号柴油
#define SPKM_DIE40MINUS				0x0242			//-40号柴油
#define SPKM_DIE20MINUS				0x0243			//-20号柴油
#define SPKM_DIE30MINUS				0x0244			//-30号柴油
#define SPKM_DIE35MINUS				0x0245			//-35号柴油
#define SPKM_DIE50MINUS				0x0246			//-50号柴油
#define SPKM_DIE05PLUS					0x0247			//+5号柴油
#define SPKM_DIE10PLUS					0x0248			//+10号柴油
#define SPKM_DIE15PLUS					0x0249			//+15号柴油
#define SPKM_DIE20PLUS					0x0250			//+20号柴油
#define SPKM_DIEARMY						0x0251			//军用柴油
#define SPKM_DIE10ARMY				0x0252			//+10号军用柴油
#define SPKM_DIE20ARMY				0x0253			//+20号军用柴油
#define SPKM_DIE30ARMY				0x0254			//+30号军用柴油
#define SPKM_DIEHEAVY					0x0255			//重柴油
#define SPKM_DIE10HEAVY				0x0256			//10号重柴油
#define SPKM_DIE20HEAVY				0x0257			//20号重柴油
#define SPKM_DIELIGHTOTH				0x0258			//其它轻柴油
#define SPKM_DIEHEAVYOTH			0x0259			//其它重柴油
#define SPKM_ONE								0x0260			//个
#define SPKM_TEN								0x0261			//十
#define SPKM_HUNDRED					0x0262			//百
#define SPKM_THOUSAND					0x0263			//千
#define SPKM_TENTHOUSAND			0x0264			//万
#define SPKM_YUAN							0x0265			//元
#define SPKM_SHENG							0x0266			//升
#define SPKM_FEN								0x0267			//分
#define SPKM_POINT							0x0268			//点
#define SPKM_JIAO							0x0269			//角
#define SPKM_PASSIN						0x0270			//请输入密码
#define SPKM_PASERR						0x0271			//密码错误请重新输入
#define SPKM_CARDVALID				0x0272			//无效卡
#define SPKM_OILACK						0x0273			//泵码已回零，请确认
#define SPKM_FAULT							0x0274			//出现故障请通知工作人员
#define SPKM_OILFILL						0x0275			//此油枪加注
#define SPKM_MONEY						0x0276			//金额
#define SPKM_NUMBEROIL				0x0277			//号油品
#define SPKM_START							0x0278			//开始
#define SPKM_RETURN						0x0279			//返回
#define SPKM_CANCEL						0x0280			//取消
#define SPKM_EXACTLY						0x0281			//整
#define SPKM_PAYFOR						0x0282			//消费
#define SPKM_THISVOLUME				0x0283			//您此次的加油量是
#define SPKM_THANKSSEEYOU			0x0284			//谢谢惠顾,祝您一路平安,欢迎下次光临
#define SPKM_THANKSSEEYOU2		0x0285			//谢谢惠顾,祝您一路平安,欢迎下次光临
#define SPKM_SEEYOU						0x0286			//祝您一路平安,欢迎下次光临
#define SPKM_TOTAL							0x0287			//累计
#define SPKM_TOTALMONEY				0x0288			//累计金额
#define SPKM_ICORCODE					0x0289			//请插入IC卡或选择油品后扫描条码
#define SPKM_CODESCAN					0x0290			//请扫描条码
#define SPKM_AUTHORIZEMONEY		0x0291			//本次加油授权金额为
#define SPKM_CODEERROR				0x0292			//扫描错误，请重新扫码
#define SPKM_CODEEXPIRED			0x0293			//条码小票已过期，请到营业室处理
#define SPKM_CODEINAGINE			0x0294			//验证码错误请重新输入
#define SPKM_SELECTERR					0x0295			//油品选择错误，请选择对应的油品
#define SPKM_GUNUSING					0x0296			//此油枪正在使用，请稍候
#define SPKM_OILEND						0x0297			//本次加油已完成，请放回油枪
#define SPKM_OILBALANCE				0x0298			//本次加油尚有余额，请凭打印票据到营业室退款
#define SPKM_ICORMONEY				0x0299			//请插入IC卡或选择油品后投入纸币
#define SPKM_MONEYIN					0x0300			//请投五至十元纸币，投币结束后请按确认按钮
#define SPKM_THISMONEYIN			0x0301			//本次投币金额为
#define SPKM_OILPLEASE					0x0302			//请提枪加油或以定量方式加油
#define SPKM_GAS92							0x0303			//92号汽油
#define SPKM_GAS92CLEAN				0x0304			//92号清洁汽油
#define SPKM_GAS92LEAD				0x0305			//92号无铅汽油
#define SPKM_GAS92ETHANOL			0x0306			//92号乙醇汽油
#define SPKM_GAS89							0x0307			//89号汽油
#define SPKM_GAS89CLEAN				0x0308			//89号清洁汽油
#define SPKM_GAS89LEAD				0x0309			//89号无铅汽油
#define SPKM_GAS89ETHANOL			0x0310			//89号乙醇汽油
#define SPKM_GAS89METHANOL		0x0311			//89号甲醇汽油
#define SPKM_GAS90METHANOL		0x0312			//90号甲醇汽油
#define SPKM_GAS92METHANOL		0x0313			//92号甲醇汽油
#define SPKM_GAS93METHANOL		0x0314			//93号甲醇汽油
#define SPKM_GAS95METHANOL		0x0315			//95号甲醇汽油
#define SPKM_GAS97METHANOL		0x0316			//97号甲醇汽油
#define SPKM_GAS98METHANOL		0x0317			//98号甲醇汽油
#define SPKM_INVOICE						0x0318			//如需发票，清平打印票据到营业室换取
#define SPKM_GAS90ETHANOL			0x0319			//90号乙醇汽油
#define SPKM_GAS95ETHANOL			0x0320			//95号乙醇汽油
#define SPKM_GAS97ETHANOL2		0x0321			//97号乙醇汽油
#define SPKM_GAS93ETHANOL2		0x0322			//93号乙醇汽油
#define SPKM_GAS98ETHANOL			0x0423			//98号乙醇汽油
#define SPKM_GASMETHANOL			0x0324			//甲醇汽油
#define SPKM_GASETHANOL				0x0325			//乙醇汽油
#define SPKM_WELCOME					0x0326			//欢迎光临，请选择服务内容
#define SPKM_OILSELECT					0x0327			//请选择对应的油品
#define SPKM_GUNON						0x0328			//请提枪加油
#define SPKM_ICCARDIN					0x0329			//请插入IC卡
#define SPKM_CODEPAYFOR				0x0330			//请插入IC卡或扫描条码
#define SPKM_DIE0CAR						0x0331			//0号车用柴油
#define SPKM_ACK								0x0332			//请确认





#if 0
//主板语音段，语音播放列表，女声
#define SPKW_0									0x0000			//0
#define SPKW_1									0x0001			//1
#define SPKW_2									0x0002			//2
#define SPKW_3									0x0003			//3
#define SPKW_4									0x0004			//4
#define SPKW_5									0x0005			//5
#define SPKW_6									0x0006			//6
#define SPKW_7									0x0007			//7
#define SPKW_8									0x0008			//8
#define SPKW_9									0x0009			//9
#define SPKW_GASONLINE				0x0010			//汽油
#define SPKW_GAS90							0x0011			//90号汽油
#define SPKW_GAS90CLEAN				0x0012			//90号清洁汽油
#define SPKW_GAS90LEAD				0x0013			//90号无铅汽油
#define SPKW_GAS93							0x0014			//93号汽油
#define SPKW_GAS93CLEAN				0x0015			//93号清洁汽油
#define SPKW_GAS93LEAD				0x0016			//93号无铅汽油
#define SPKW_GAS93ETHANOL			0x0017			//93号乙醇汽油
#define SPKW_GAS95							0x0018			//95号汽油
#define SPKW_GAS95CLEAN				0x0019			//95号清洁汽油
#define SPKW_GAS95LEAD				0x0020			//95号无铅汽油
#define SPKW_GAS97							0x0021			//97号汽油
#define SPKW_GAS97CLEAN				0x0022			//97号清洁汽油
#define SPKW_GAS97LEAD				0x0023			//97号无铅汽油
#define SPKW_GAS97ETHANOL			0x0024			//97号乙醇汽油
#define SPKW_GAS98							0x0025			//98号汽油
#define SPKW_GAS98CLEAN				0x0026			//98号清洁汽油
#define SPKW_GAS98LEAD				0x0027			//98号无铅汽油
#define SPKW_GAS120						0x0028			//120号汽油
#define SPKW_GAS200						0x0029			//200号汽油
#define SPKW_DIE0							0x0030			//0号柴油
#define SPKW_DIE05MINUS				0x0031			//-5号柴油
#define SPKW_DIE10MINUS				0x0032			//-10号柴油
#define SPKW_DIE15MINUS				0x0033			//-15号柴油
#define SPKW_DIE40MINUS				0x0034			//-40号柴油
#define SPKW_DIE20MINUS				0x0035			//-20号柴油
#define SPKW_DIE30MINUS				0x0036			//-30号柴油
#define SPKW_DIE35MINUS				0x0037			//-35号柴油
#define SPKW_DIE50MINUS				0x0038			//-50号柴油
#define SPKW_DIE05PLUS					0x0039			//+5号柴油
#define SPKW_DIE10PLUS					0x0040			//+10号柴油
#define SPKW_DIE15PLUS					0x0041			//+15号柴油
#define SPKW_DIE20PLUS					0x0042			//+20号柴油
#define SPKW_ONE								0x0043			//个
#define SPKW_TEN								0x0044			//十
#define SPKW_HUNDRED					0x0045			//百
#define SPKW_THOUSAND					0x0046			//千
#define SPKW_TENTHOUSAND			0x0047			//万
#define SPKW_YUAN							0x0048			//元
#define SPKW_SHENG							0x0049			//升
#define SPKW_FEN								0x0050			//分
#define SPKW_POINT							0x0051			//点
#define SPKW_JIAO							0x0052			//角
#define SPKW_PASSIN						0x0053			//请输入密码
#define SPKW_PASERR						0x0054			//密码错误请重新输入
#define SPKW_CARDVALID				0x0055			//无效卡
#define SPKW_OILACK						0x0056			//泵码已回零，请确认
#define SPKW_FAULT							0x0057			//出现故障请通知工作人员
#define SPKW_OILFILL						0x0058			//此油枪加注
#define SPKW_NUMBEROIL				0x0059			//号油品
#define SPKW_THISVOLUME				0x0060			//您此次的加油量是
#define SPKW_SEEYOU						0x0061			//祝您一路平安,欢迎下次光临
#define SPKW_TOTALMONEY				0x0062			//累计金额
#define SPKW_ICORCODE					0x0063			//请插入IC卡或选择油品后扫描条码
#define SPKW_CODESCAN					0x0064			//请扫描条码
#define SPKW_AUTHORIZEMONEY	0x0065			//本次加油授权金额为
#define SPKW_CODEERROR				0x0066			//扫描错误，请重新扫码
#define SPKW_CODEEXPIRED			0x0067			//条码小票已过期，请到营业室处理
#define SPKW_CODEINAGINE			0x0068			//验证码错误请重新输入
#define SPKW_SELECTERR					0x0069			//油品选择错误，请选择对应的油品
#define SPKW_GUNUSING					0x0070			//此油枪正在使用，请稍候
#define SPKW_OILEND						0x0071			//本次加油已完成，请放回油枪
#define SPKW_OILBALANCE				0x0072			//本次加油尚有余额，请凭打印票据到营业室退款
#define SPKW_ICORMONEY				0x0073			//请插入IC卡或选择油品后投入纸币
#define SPKW_DIE0CAR						0x0074			//0号车用柴油
#define SPKW_THISMONEYIN			0x0075			//本次投币金额为
#define SPKW_OILPLEASE					0x0076			//请提枪加油或以定量方式加油
//主板语音段，语音播放列表，男声
#define SPKM_0									0x0077			//0
#define SPKM_1									0x0078			//1
#define SPKM_2									0x0079			//2
#define SPKM_3									0x0080			//3
#define SPKM_4									0x0081			//4
#define SPKM_5									0x0082			//5
#define SPKM_6									0x0083			//6
#define SPKM_7									0x0084			//7
#define SPKM_8									0x0085			//8
#define SPKM_9									0x0086			//9
#define SPKM_GAS90							0x0087			//90号汽油
#define SPKM_GAS90CLEAN				0x0088			//90号清洁汽油
#define SPKM_GAS90LEAD				0x0089			//90号无铅汽油
#define SPKM_GAS93							0x0090			//93号汽油
#define SPKM_GAS93CLEAN				0x0091			//93号清洁汽油
#define SPKM_GAS93LEAD				0x0092			//93号无铅汽油
#define SPKM_GAS93ETHANOL			0x0093			//93号乙醇汽油
#define SPKM_GAS95CLEAN				0x0094			//95号清洁汽油
#define SPKM_GAS95LEAD				0x0095			//95号无铅汽油
#define SPKM_GAS97							0x0096			//97号汽油
#define SPKM_GAS97CLEAN				0x0097			//97号清洁汽油
#define SPKM_GAS97LEAD				0x0098			//97号无铅汽油
#define SPKM_GAS97ETHANOL			0x0099			//97号乙醇汽油
#define SPKM_GAS98							0x0100			//98号汽油
#define SPKM_GAS98CLEAN				0x0101			//98号清洁汽油
#define SPKM_GAS98LEAD				0x0102			//98号无铅汽油
#define SPKM_GAS120						0x0103			//120号汽油
#define SPKM_GAS200						0x0104			//200号汽油
#define SPKM_DIE0							0x0105			//0号柴油
#define SPKM_DIE05MINUS				0x0106			//-5号柴油
#define SPKM_DIE10MINUS				0x0107			//-10号柴油
#define SPKM_DIE15MINUS				0x0108			//-15号柴油
#define SPKM_DIE40MINUS				0x0109			//-40号柴油
#define SPKM_DIE20MINUS				0x0110			//-20号柴油
#define SPKM_DIE30MINUS				0x0111			//-30号柴油
#define SPKM_DIE35MINUS				0x0112			//-35号柴油
#define SPKM_DIE50MINUS				0x0113			//-50号柴油
#define SPKM_DIE05PLUS					0x0114			//+5号柴油
#define SPKM_DIE10PLUS					0x0115			//+10号柴油
#define SPKM_DIE15PLUS					0x0116			//+15号柴油
#define SPKM_DIE20PLUS					0x0117			//+20号柴油
#define SPKM_ONE								0x0118			//个
#define SPKM_TEN								0x0119			//十
#define SPKM_HUNDRED					0x0120			//百
#define SPKM_THOUSAND					0x0121			//千
#define SPKM_TENTHOUSAND			0x0122			//万
#define SPKM_YUAN							0x0123			//元
#define SPKM_SHENG							0x0124			//升
#define SPKM_FEN								0x0125			//分
#define SPKM_POINT							0x0126			//点
#define SPKM_JIAO							0x0127			//角
#define SPKM_PASSIN						0x0128			//请输入卡密码
#define SPKM_PASERR						0x0129			//密码错误请重新输入
#define SPKM_CARDVALID				0x0130			//无效卡
#define SPKM_OILACK						0x0131			//泵码已回零，请确认
#define SPKM_FAULT							0x0132			//出现故障请通知工作人员
#define SPKM_OILFILL						0x0133			//此油枪加注
#define SPKM_NUMBEROIL				0x0134			//号油品
#define SPKM_THISVOLUME				0x0135			//您此次的加油量是
#define SPKM_SEEYOU						0x0136			//祝您一路平安,欢迎下次光临
#define SPKM_TOTALMONEY				0x0137			//男声，累计金额
#define SPKM_ICORCODE					0x0138			//男声，请插入IC卡或选择油品后扫描条码
#define SPKM_CODESCAN					0x0139			//男声，请扫描条码
#define SPKM_AUTHORIZEMONEY		0x0140			//男声，本次加油授权金额为
#define SPKM_CODEERROR				0x0141			//男声，扫描错误，请重新扫码
#define SPKM_CODEEXPIRED			0x0142			//男声，条码小票已过期，请到营业室处理
#define SPKM_CODEINAGINE			0x0143			//男声，验证码错误请重新输入
#define SPKM_SELECTERR					0x0144			//男声，油品选择错误，请选择对应的油品
#define SPKM_GUNUSING					0x0145			//男声，此油枪正在使用，请稍候
#define SPKM_OILEND						0x0146			//男声，本次加油已完成，请放回油枪
#define SPKM_OILBALANCE				0x0147			//男声，本次加油尚有余额，请凭打印票据到营业室退款
#define SPKM_ICORMONEY				0x0148			//男声，请插入IC卡或选择油品后投入纸币
#define SPKM_DIE0CAR						0x0149			//男声，0号车用柴油
#define SPKM_THISMONEYIN			0x0150			//男声，本次投币金额为
#define SPKM_OILPLEASE					0x0151			//男声，请提枪加油或以定量方式加油
#define SPKM_INVOICE						0x0152			//男声，如需发票，清平打印票据到营业室换取
#define SPKW_DIESELFUEL				0x0153			//女声，柴油
#define SPKM_GASONLINE				0x0154			//男声，汽油
#define SPKM_DIESELFUEL				0x0155			//男声，柴油
#define SPKW_ICCARDIN					0x0156			//女声，请插入IC卡
#define SPKM_ICCARDIN					0x0157			//男声，请插入IC卡
#define SPKM_GAS95							0x0158			//男声，95号汽油
#define SPKW_SPACE							0x0159			//空闲
#define SPKW_GAS92							0x0160			//女声，92号汽油
#define SPKW_GAS89							0x0161			//女声，89号汽油
#define SPKM_GAS92							0x0162			//男声，92号汽油
#define SPKM_GAS89							0x0163			//男声，89号汽油
#endif

#endif //#if APP_SINOPEC



#if APP_CNPC

//语音播放列表，女声
#define SPKW_0									0x0000			//0
#define SPKW_1									0x0001			//1
#define SPKW_2									0x0002			//2
#define SPKW_3									0x0003			//3
#define SPKW_4									0x0004			//4
#define SPKW_5									0x0005			//5
#define SPKW_6									0x0006			//6
#define SPKW_7									0x0007			//7
#define SPKW_8									0x0008			//8
#define SPKW_9									0x0009			//9
#define SPKW_TEN								0x0010			//十
#define SPKW_HUNDRED					        0x0011			//百
#define SPKW_THOUSAND					        0x0012			//千
#define SPKW_TENTHOUSAND			            0x0013			//万
#define SPKW_YUAN							    0x0014			//元
#define SPKW_SHENG							    0x0015			//升
#define SPKW_POINT							    0x0016			//点
#define SPKW_PLUS							    0x0017			//加

#endif //#if APP_CNPC




//语音内容播放节点
typedef struct
{
	unsigned char *Buffer;					//语音数据
	unsigned int Length;						//语音数据长度
	unsigned int Pointer;						//语音数据播放下标

}SPKPlayDataStruct;


//语音播放数据结构
typedef struct
{
	//基本数据
	char Speaker;								//扬声器选择；0=本地扬声器；1=外部扩展设备扬声器
	int iFdCom;									//连接外部扩展设备扬声器的串口
	int tIdListSpk;								//语音播放列表播放任务

	//语音段序号相关数据，这是本次播放需要
	unsigned char VoiceListPointer;									//当前播放语音下标
	unsigned char VoiceListLength;									//语音播放列表内语音段数目0~SPK_MAX_NUMBER
	unsigned int VoiceList[SPK_MAX_NUMBER];					//要播放的语音段序列号2bytes为一段

	//语音播放数据，这是在TC中断中播放的语音数据
	unsigned int SampRate;												//采样率
	unsigned char Volume;													//音量


	SPKPlayDataStruct SpkData[SPK_MAX_NUMBER];		//语音段数据
	unsigned char SpkDataLength;									//语音段数目
	unsigned char SpkDataPointer;									//语音段当前下标指针
}SpkStructType;


extern int spkNextFileNameGet(const char *param, char *nextname, int maxbytes, char *thisname, int nextflag, int type);
extern int spkFileNameGet(const char *param, char *name, int maxbytes);
extern int spkDeviceSet(int speaker, int device);
extern int spkPlay(int speaker, int volume, int *voicelist, int voicenumber);
extern bool spkDevInit(void);

int tSpeaker(int speaker);


#endif



