#ifndef _OIL_SPK_H_
#define _OIL_SPK_H_

//����Ƶ��
#define SAMPL_40K    		0x672		//40K
#define SAMPL_64K    		0x407		//������60K
#define SAMPL_80K    		0x339		//������80K
#define SAMPL_128K   		0x204		//������128K
#define SAMPL_256K			0x102		//������256K
#define SAMPL_512K			0x81			//������512K
#define SAMPL_1024K		0x40			//������1024K


//�����ļ���·��
//#define SPK_PATH							"/flash/voice"
#define SPK_PATH							"../config/voice"

//�����ŵ����������ܼ�����ֽ���
#define SPK_SUM_MAX_SIZE		0x20000		//128K

//�������Ŷ���Ŀ
#define SPK_MAX_NUMBER			32

//������ǹ��
#define SPK_NOZZLE_1						0					//1��������
#define SPK_NOZZLE_2						1					//2��������

//������ѡ��
#define SPK_DEVICE_LOCAL				0					//����������
#define SPK_DEVICE_PC					1					//ƽ������豸������


#if APP_SINOPEC

//���������������б�Ů��
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
#define SPKW_GASONLINE				0x0010			//����
#define SPKW_GAS90							0x0011			//90������
#define SPKW_GAS90CLEAN				0x0012			//90���������
#define SPKW_GAS90LEAD				0x0013			//90����Ǧ����
#define SPKW_GAS93							0x0014			//93������
#define SPKW_GAS93CLEAN				0x0015			//93���������
#define SPKW_GAS93LEAD				0x0016			//93����Ǧ����
#define SPKW_GAS93ETHANOL			0x0017			//93���Ҵ�����
#define SPKW_GAS95							0x0018			//95������
#define SPKW_GAS95CLEAN				0x0019			//95���������
#define SPKW_GAS95LEAD				0x0020			//95����Ǧ����
#define SPKW_GAS95AVIATION		0x0021			//95�ź�������
#define SPKW_GAS97							0x0022			//97������
#define SPKW_GAS97CLEAN				0x0023			//97���������
#define SPKW_GAS97LEAD				0x0024			//97����Ǧ����
#define SPKW_GAS97ETHANOL			0x0025			//97���Ҵ�����
#define SPKW_GAS98							0x0026			//98������
#define SPKW_GAS98CLEAN				0x0027			//98���������
#define SPKW_GAS98LEAD				0x0028			//98����Ǧ����
#define SPKW_GAS120						0x0029			//120������
#define SPKW_GAS200						0x0030			//200������
#define SPKW_GASAVIATION			0x0031			//��������
#define SPKW_GAS75AVIATION		0x0032			//75�ź�������
#define SPKW_GAS100AVIATION		0x0033			//100�ź�������
#define SPKW_GASCAR						0x0034			//��������
#define SPKW_GASCAROTHER			0x0035			//������������
#define SPKW_GASAVIATIONOTH		0x0036			//������������
#define SPKW_DIESELFUEL				0x0037			//����
#define SPKW_DIE0							0x0038			//0�Ų���
#define SPKW_DIE05MINUS				0x0039			//-5�Ų���
#define SPKW_DIE10MINUS				0x0040			//-10�Ų���
#define SPKW_DIE15MINUS				0x0041			//-15�Ų���
#define SPKW_DIE40MINUS				0x0042			//-40�Ų���
#define SPKW_DIE20MINUS				0x0043			//-20�Ų���
#define SPKW_DIE30MINUS				0x0044			//-30�Ų���
#define SPKW_DIE35MINUS				0x0045			//-35�Ų���
#define SPKW_DIE50MINUS				0x0046			//-50�Ų���
#define SPKW_DIE05PLUS					0x0047			//+5�Ų���
#define SPKW_DIE10PLUS					0x0048			//+10�Ų���
#define SPKW_DIE15PLUS					0x0049			//+15�Ų���
#define SPKW_DIE20PLUS					0x0050			//+20�Ų���
#define SPKW_DIEARMY					0x0051			//���ò���
#define SPKW_DIE10ARMY				0x0052			//+10�ž��ò���
#define SPKW_DIE20ARMY				0x0053			//+20�ž��ò���
#define SPKW_DIE30ARMY				0x0054			//+30�ž��ò���
#define SPKW_DIEHEAVY					0x0055			//�ز���
#define SPKW_DIE10HEAVY				0x0056			//10���ز���
#define SPKW_DIE20HEAVY				0x0057			//20���ز���
#define SPKW_DIELIGHTOTH				0x0058			//���������
#define SPKW_DIEHEAVYOTH			0x0059			//�����ز���
#define SPKW_ONE								0x0060			//��
#define SPKW_TEN								0x0061			//ʮ
#define SPKW_HUNDRED					0x0062			//��
#define SPKW_THOUSAND					0x0063			//ǧ
#define SPKW_TENTHOUSAND			0x0064			//��
#define SPKW_YUAN							0x0065			//Ԫ
#define SPKW_SHENG							0x0066			//��
#define SPKW_FEN								0x0067			//��
#define SPKW_POINT							0x0068			//��
#define SPKW_JIAO							0x0069			//��
#define SPKW_PASSIN						0x0070			//����������
#define SPKW_PASERR						0x0071			//�����������������
#define SPKW_CARDVALID				0x0072			//��Ч��
#define SPKW_OILACK						0x0073			//�����ѻ��㣬��ȷ��
#define SPKW_FAULT							0x0074			//���ֹ�����֪ͨ������Ա
#define SPKW_OILFILL						0x0075			//����ǹ��ע
#define SPKW_MONEY						0x0076			//���
#define SPKW_NUMBEROIL				0x0077			//����Ʒ
#define SPKW_START							0x0078			//��ʼ
#define SPKW_RETURN						0x0079			//����
#define SPKW_CANCEL						0x0080			//ȡ��
#define SPKW_EXACTLY						0x0081			//��
#define SPKW_PAYFOR						0x0082			//����
#define SPKW_THISVOLUME				0x0083			//���˴εļ�������
#define SPKW_THANKSSEEYOU			0x0084			//лл�ݹ�,ף��һ·ƽ��,��ӭ�´ι���
#define SPKW_THANKSSEEYOU2		0x0085			//лл�ݹ�,ף��һ·ƽ��,��ӭ�´ι���
#define SPKW_SEEYOU						0x0086			//ף��һ·ƽ��,��ӭ�´ι���
#define SPKW_TOTAL							0x0087			//�ۼ�
#define SPKW_TOTALMONEY				0x0088			//�ۼƽ��
#define SPKW_ICORCODE					0x0089			//�����IC����ѡ����Ʒ��ɨ������
#define SPKW_CODESCAN					0x0090			//��ɨ������
#define SPKW_AUTHORIZEMONEY	0x0091			//���μ�����Ȩ���Ϊ
#define SPKW_CODEERROR				0x0092			//ɨ�����������ɨ��
#define SPKW_CODEEXPIRED			0x0093			//����СƱ�ѹ��ڣ��뵽Ӫҵ�Ҵ���
#define SPKW_CODEINAGINE			0x0094			//��֤���������������
#define SPKW_SELECTERR					0x0095			//��Ʒѡ�������ѡ���Ӧ����Ʒ
#define SPKW_GUNUSING					0x0096			//����ǹ����ʹ�ã����Ժ�
#define SPKW_OILEND						0x0097			//���μ�������ɣ���Ż���ǹ
#define SPKW_OILBALANCE				0x0098			//���μ�����������ƾ��ӡƱ�ݵ�Ӫҵ���˿�
#define SPKW_ICORMONEY				0x0099			//�����IC����ѡ����Ʒ��Ͷ��ֽ��
#define SPKW_MONEYIN					0x0100			//��Ͷ����ʮԪֽ�ң�Ͷ�ҽ������밴ȷ�ϰ�ť
#define SPKW_THISMONEYIN			0x0101			//����Ͷ�ҽ��Ϊ
#define SPKW_OILPLEASE					0x0102			//����ǹ���ͻ��Զ�����ʽ����
#define SPKW_GAS92							0x0103			//92������
#define SPKW_GAS92CLEAN				0x0104			//92���������
#define SPKW_GAS92LEAD				0x0105			//92����Ǧ����
#define SPKW_GAS92ETHANOL			0x0106			//92���Ҵ�����
#define SPKW_GAS89							0x0107			//89������
#define SPKW_GAS89CLEAN				0x0108			//89���������
#define SPKW_GAS89LEAD				0x0109			//89����Ǧ����
#define SPKW_GAS89ETHANOL			0x0110			//89���Ҵ�����
#define SPKW_GAS89METHANOL		0x0111			//89�ż״�����
#define SPKW_GAS90METHANOL		0x0112			//90�ż״�����
#define SPKW_GAS92METHANOL		0x0113			//92�ż״�����
#define SPKW_GAS93METHANOL		0x0114			//93�ż״�����
#define SPKW_GAS95METHANOL		0x0115			//95�ż״�����
#define SPKW_GAS97METHANOL		0x0116			//97�ż״�����
#define SPKW_GAS98METHANOL		0x0117			//98�ż״�����
#define SPKW_WELCOME			0x0118			//��ӭ���٣���ѡ���������
#define SPKW_OILSELECT			0x0119			//��ѡ���Ӧ����Ʒ
#define SPKW_GUNON				0x0120			//����ǹ����
#define SPKW_ICCARDIN			0x0121			//�����IC��
#define SPKW_CODEPAYFOR			0x0122			//�����IC����ɨ������
#define SPKW_DIE0CAR			0x0123			//0�ų��ò���
#define SPKW_ACK				0x0124			//��ȷ��

#define SPKW_LIMIT_CAR          0x0126          //��ǹ�޳��ſ����ͣ�szb_fj20171120

//���������������б�����
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
#define SPKM_GASONLINE				0x0210			//����
#define SPKM_GAS90							0x0211			//90������
#define SPKM_GAS90CLEAN				0x0212			//90���������
#define SPKM_GAS90LEAD				0x0213			//90����Ǧ����
#define SPKM_GAS93							0x0214			//93������
#define SPKM_GAS93CLEAN				0x0215			//93���������
#define SPKM_GAS93LEAD				0x0216			//93����Ǧ����
#define SPKM_GAS93ETHANOL			0x0217			//93���Ҵ�����
#define SPKM_GAS95							0x0218			//95������
#define SPKM_GAS95CLEAN				0x0219			//95���������
#define SPKM_GAS95LEAD				0x0220			//95����Ǧ����
#define SPKM_GAS95AVIATION		0x0221			//95�ź�������
#define SPKM_GAS97							0x0222			//97������
#define SPKM_GAS97CLEAN				0x0223			//97���������
#define SPKM_GAS97LEAD				0x0224			//97����Ǧ����
#define SPKM_GAS97ETHANOL			0x0225			//97���Ҵ�����
#define SPKM_GAS98							0x0226			//98������
#define SPKM_GAS98CLEAN				0x0227			//98���������
#define SPKM_GAS98LEAD				0x0228			//98����Ǧ����
#define SPKM_GAS120						0x0229			//120������
#define SPKM_GAS200						0x0230			//200������
#define SPKM_GASAVIATION			0x0231			//��������
#define SPKM_GAS75AVIATION		0x0232			//75�ź�������
#define SPKM_GAS100AVIATION		0x0233			//100�ź�������
#define SPKM_GASCAR						0x0234			//��������
#define SPKM_GASCAROTHER			0x0235			//������������
#define SPKM_GASAVIATIONOTH		0x0236			//������������
#define SPKM_DIESELFUEL				0x0237			//����
#define SPKM_DIE0							0x0238			//0�Ų���
#define SPKM_DIE05MINUS				0x0239			//-5�Ų���
#define SPKM_DIE10MINUS				0x0240			//-10�Ų���
#define SPKM_DIE15MINUS				0x0241			//-15�Ų���
#define SPKM_DIE40MINUS				0x0242			//-40�Ų���
#define SPKM_DIE20MINUS				0x0243			//-20�Ų���
#define SPKM_DIE30MINUS				0x0244			//-30�Ų���
#define SPKM_DIE35MINUS				0x0245			//-35�Ų���
#define SPKM_DIE50MINUS				0x0246			//-50�Ų���
#define SPKM_DIE05PLUS					0x0247			//+5�Ų���
#define SPKM_DIE10PLUS					0x0248			//+10�Ų���
#define SPKM_DIE15PLUS					0x0249			//+15�Ų���
#define SPKM_DIE20PLUS					0x0250			//+20�Ų���
#define SPKM_DIEARMY						0x0251			//���ò���
#define SPKM_DIE10ARMY				0x0252			//+10�ž��ò���
#define SPKM_DIE20ARMY				0x0253			//+20�ž��ò���
#define SPKM_DIE30ARMY				0x0254			//+30�ž��ò���
#define SPKM_DIEHEAVY					0x0255			//�ز���
#define SPKM_DIE10HEAVY				0x0256			//10���ز���
#define SPKM_DIE20HEAVY				0x0257			//20���ز���
#define SPKM_DIELIGHTOTH				0x0258			//���������
#define SPKM_DIEHEAVYOTH			0x0259			//�����ز���
#define SPKM_ONE								0x0260			//��
#define SPKM_TEN								0x0261			//ʮ
#define SPKM_HUNDRED					0x0262			//��
#define SPKM_THOUSAND					0x0263			//ǧ
#define SPKM_TENTHOUSAND			0x0264			//��
#define SPKM_YUAN							0x0265			//Ԫ
#define SPKM_SHENG							0x0266			//��
#define SPKM_FEN								0x0267			//��
#define SPKM_POINT							0x0268			//��
#define SPKM_JIAO							0x0269			//��
#define SPKM_PASSIN						0x0270			//����������
#define SPKM_PASERR						0x0271			//�����������������
#define SPKM_CARDVALID				0x0272			//��Ч��
#define SPKM_OILACK						0x0273			//�����ѻ��㣬��ȷ��
#define SPKM_FAULT							0x0274			//���ֹ�����֪ͨ������Ա
#define SPKM_OILFILL						0x0275			//����ǹ��ע
#define SPKM_MONEY						0x0276			//���
#define SPKM_NUMBEROIL				0x0277			//����Ʒ
#define SPKM_START							0x0278			//��ʼ
#define SPKM_RETURN						0x0279			//����
#define SPKM_CANCEL						0x0280			//ȡ��
#define SPKM_EXACTLY						0x0281			//��
#define SPKM_PAYFOR						0x0282			//����
#define SPKM_THISVOLUME				0x0283			//���˴εļ�������
#define SPKM_THANKSSEEYOU			0x0284			//лл�ݹ�,ף��һ·ƽ��,��ӭ�´ι���
#define SPKM_THANKSSEEYOU2		0x0285			//лл�ݹ�,ף��һ·ƽ��,��ӭ�´ι���
#define SPKM_SEEYOU						0x0286			//ף��һ·ƽ��,��ӭ�´ι���
#define SPKM_TOTAL							0x0287			//�ۼ�
#define SPKM_TOTALMONEY				0x0288			//�ۼƽ��
#define SPKM_ICORCODE					0x0289			//�����IC����ѡ����Ʒ��ɨ������
#define SPKM_CODESCAN					0x0290			//��ɨ������
#define SPKM_AUTHORIZEMONEY		0x0291			//���μ�����Ȩ���Ϊ
#define SPKM_CODEERROR				0x0292			//ɨ�����������ɨ��
#define SPKM_CODEEXPIRED			0x0293			//����СƱ�ѹ��ڣ��뵽Ӫҵ�Ҵ���
#define SPKM_CODEINAGINE			0x0294			//��֤���������������
#define SPKM_SELECTERR					0x0295			//��Ʒѡ�������ѡ���Ӧ����Ʒ
#define SPKM_GUNUSING					0x0296			//����ǹ����ʹ�ã����Ժ�
#define SPKM_OILEND						0x0297			//���μ�������ɣ���Ż���ǹ
#define SPKM_OILBALANCE				0x0298			//���μ�����������ƾ��ӡƱ�ݵ�Ӫҵ���˿�
#define SPKM_ICORMONEY				0x0299			//�����IC����ѡ����Ʒ��Ͷ��ֽ��
#define SPKM_MONEYIN					0x0300			//��Ͷ����ʮԪֽ�ң�Ͷ�ҽ������밴ȷ�ϰ�ť
#define SPKM_THISMONEYIN			0x0301			//����Ͷ�ҽ��Ϊ
#define SPKM_OILPLEASE					0x0302			//����ǹ���ͻ��Զ�����ʽ����
#define SPKM_GAS92							0x0303			//92������
#define SPKM_GAS92CLEAN				0x0304			//92���������
#define SPKM_GAS92LEAD				0x0305			//92����Ǧ����
#define SPKM_GAS92ETHANOL			0x0306			//92���Ҵ�����
#define SPKM_GAS89							0x0307			//89������
#define SPKM_GAS89CLEAN				0x0308			//89���������
#define SPKM_GAS89LEAD				0x0309			//89����Ǧ����
#define SPKM_GAS89ETHANOL			0x0310			//89���Ҵ�����
#define SPKM_GAS89METHANOL		0x0311			//89�ż״�����
#define SPKM_GAS90METHANOL		0x0312			//90�ż״�����
#define SPKM_GAS92METHANOL		0x0313			//92�ż״�����
#define SPKM_GAS93METHANOL		0x0314			//93�ż״�����
#define SPKM_GAS95METHANOL		0x0315			//95�ż״�����
#define SPKM_GAS97METHANOL		0x0316			//97�ż״�����
#define SPKM_GAS98METHANOL		0x0317			//98�ż״�����
#define SPKM_INVOICE						0x0318			//���跢Ʊ����ƽ��ӡƱ�ݵ�Ӫҵ�һ�ȡ
#define SPKM_GAS90ETHANOL			0x0319			//90���Ҵ�����
#define SPKM_GAS95ETHANOL			0x0320			//95���Ҵ�����
#define SPKM_GAS97ETHANOL2		0x0321			//97���Ҵ�����
#define SPKM_GAS93ETHANOL2		0x0322			//93���Ҵ�����
#define SPKM_GAS98ETHANOL			0x0423			//98���Ҵ�����
#define SPKM_GASMETHANOL			0x0324			//�״�����
#define SPKM_GASETHANOL				0x0325			//�Ҵ�����
#define SPKM_WELCOME					0x0326			//��ӭ���٣���ѡ���������
#define SPKM_OILSELECT					0x0327			//��ѡ���Ӧ����Ʒ
#define SPKM_GUNON						0x0328			//����ǹ����
#define SPKM_ICCARDIN					0x0329			//�����IC��
#define SPKM_CODEPAYFOR				0x0330			//�����IC����ɨ������
#define SPKM_DIE0CAR						0x0331			//0�ų��ò���
#define SPKM_ACK								0x0332			//��ȷ��





#if 0
//���������Σ����������б�Ů��
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
#define SPKW_GASONLINE				0x0010			//����
#define SPKW_GAS90							0x0011			//90������
#define SPKW_GAS90CLEAN				0x0012			//90���������
#define SPKW_GAS90LEAD				0x0013			//90����Ǧ����
#define SPKW_GAS93							0x0014			//93������
#define SPKW_GAS93CLEAN				0x0015			//93���������
#define SPKW_GAS93LEAD				0x0016			//93����Ǧ����
#define SPKW_GAS93ETHANOL			0x0017			//93���Ҵ�����
#define SPKW_GAS95							0x0018			//95������
#define SPKW_GAS95CLEAN				0x0019			//95���������
#define SPKW_GAS95LEAD				0x0020			//95����Ǧ����
#define SPKW_GAS97							0x0021			//97������
#define SPKW_GAS97CLEAN				0x0022			//97���������
#define SPKW_GAS97LEAD				0x0023			//97����Ǧ����
#define SPKW_GAS97ETHANOL			0x0024			//97���Ҵ�����
#define SPKW_GAS98							0x0025			//98������
#define SPKW_GAS98CLEAN				0x0026			//98���������
#define SPKW_GAS98LEAD				0x0027			//98����Ǧ����
#define SPKW_GAS120						0x0028			//120������
#define SPKW_GAS200						0x0029			//200������
#define SPKW_DIE0							0x0030			//0�Ų���
#define SPKW_DIE05MINUS				0x0031			//-5�Ų���
#define SPKW_DIE10MINUS				0x0032			//-10�Ų���
#define SPKW_DIE15MINUS				0x0033			//-15�Ų���
#define SPKW_DIE40MINUS				0x0034			//-40�Ų���
#define SPKW_DIE20MINUS				0x0035			//-20�Ų���
#define SPKW_DIE30MINUS				0x0036			//-30�Ų���
#define SPKW_DIE35MINUS				0x0037			//-35�Ų���
#define SPKW_DIE50MINUS				0x0038			//-50�Ų���
#define SPKW_DIE05PLUS					0x0039			//+5�Ų���
#define SPKW_DIE10PLUS					0x0040			//+10�Ų���
#define SPKW_DIE15PLUS					0x0041			//+15�Ų���
#define SPKW_DIE20PLUS					0x0042			//+20�Ų���
#define SPKW_ONE								0x0043			//��
#define SPKW_TEN								0x0044			//ʮ
#define SPKW_HUNDRED					0x0045			//��
#define SPKW_THOUSAND					0x0046			//ǧ
#define SPKW_TENTHOUSAND			0x0047			//��
#define SPKW_YUAN							0x0048			//Ԫ
#define SPKW_SHENG							0x0049			//��
#define SPKW_FEN								0x0050			//��
#define SPKW_POINT							0x0051			//��
#define SPKW_JIAO							0x0052			//��
#define SPKW_PASSIN						0x0053			//����������
#define SPKW_PASERR						0x0054			//�����������������
#define SPKW_CARDVALID				0x0055			//��Ч��
#define SPKW_OILACK						0x0056			//�����ѻ��㣬��ȷ��
#define SPKW_FAULT							0x0057			//���ֹ�����֪ͨ������Ա
#define SPKW_OILFILL						0x0058			//����ǹ��ע
#define SPKW_NUMBEROIL				0x0059			//����Ʒ
#define SPKW_THISVOLUME				0x0060			//���˴εļ�������
#define SPKW_SEEYOU						0x0061			//ף��һ·ƽ��,��ӭ�´ι���
#define SPKW_TOTALMONEY				0x0062			//�ۼƽ��
#define SPKW_ICORCODE					0x0063			//�����IC����ѡ����Ʒ��ɨ������
#define SPKW_CODESCAN					0x0064			//��ɨ������
#define SPKW_AUTHORIZEMONEY	0x0065			//���μ�����Ȩ���Ϊ
#define SPKW_CODEERROR				0x0066			//ɨ�����������ɨ��
#define SPKW_CODEEXPIRED			0x0067			//����СƱ�ѹ��ڣ��뵽Ӫҵ�Ҵ���
#define SPKW_CODEINAGINE			0x0068			//��֤���������������
#define SPKW_SELECTERR					0x0069			//��Ʒѡ�������ѡ���Ӧ����Ʒ
#define SPKW_GUNUSING					0x0070			//����ǹ����ʹ�ã����Ժ�
#define SPKW_OILEND						0x0071			//���μ�������ɣ���Ż���ǹ
#define SPKW_OILBALANCE				0x0072			//���μ�����������ƾ��ӡƱ�ݵ�Ӫҵ���˿�
#define SPKW_ICORMONEY				0x0073			//�����IC����ѡ����Ʒ��Ͷ��ֽ��
#define SPKW_DIE0CAR						0x0074			//0�ų��ò���
#define SPKW_THISMONEYIN			0x0075			//����Ͷ�ҽ��Ϊ
#define SPKW_OILPLEASE					0x0076			//����ǹ���ͻ��Զ�����ʽ����
//���������Σ����������б�����
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
#define SPKM_GAS90							0x0087			//90������
#define SPKM_GAS90CLEAN				0x0088			//90���������
#define SPKM_GAS90LEAD				0x0089			//90����Ǧ����
#define SPKM_GAS93							0x0090			//93������
#define SPKM_GAS93CLEAN				0x0091			//93���������
#define SPKM_GAS93LEAD				0x0092			//93����Ǧ����
#define SPKM_GAS93ETHANOL			0x0093			//93���Ҵ�����
#define SPKM_GAS95CLEAN				0x0094			//95���������
#define SPKM_GAS95LEAD				0x0095			//95����Ǧ����
#define SPKM_GAS97							0x0096			//97������
#define SPKM_GAS97CLEAN				0x0097			//97���������
#define SPKM_GAS97LEAD				0x0098			//97����Ǧ����
#define SPKM_GAS97ETHANOL			0x0099			//97���Ҵ�����
#define SPKM_GAS98							0x0100			//98������
#define SPKM_GAS98CLEAN				0x0101			//98���������
#define SPKM_GAS98LEAD				0x0102			//98����Ǧ����
#define SPKM_GAS120						0x0103			//120������
#define SPKM_GAS200						0x0104			//200������
#define SPKM_DIE0							0x0105			//0�Ų���
#define SPKM_DIE05MINUS				0x0106			//-5�Ų���
#define SPKM_DIE10MINUS				0x0107			//-10�Ų���
#define SPKM_DIE15MINUS				0x0108			//-15�Ų���
#define SPKM_DIE40MINUS				0x0109			//-40�Ų���
#define SPKM_DIE20MINUS				0x0110			//-20�Ų���
#define SPKM_DIE30MINUS				0x0111			//-30�Ų���
#define SPKM_DIE35MINUS				0x0112			//-35�Ų���
#define SPKM_DIE50MINUS				0x0113			//-50�Ų���
#define SPKM_DIE05PLUS					0x0114			//+5�Ų���
#define SPKM_DIE10PLUS					0x0115			//+10�Ų���
#define SPKM_DIE15PLUS					0x0116			//+15�Ų���
#define SPKM_DIE20PLUS					0x0117			//+20�Ų���
#define SPKM_ONE								0x0118			//��
#define SPKM_TEN								0x0119			//ʮ
#define SPKM_HUNDRED					0x0120			//��
#define SPKM_THOUSAND					0x0121			//ǧ
#define SPKM_TENTHOUSAND			0x0122			//��
#define SPKM_YUAN							0x0123			//Ԫ
#define SPKM_SHENG							0x0124			//��
#define SPKM_FEN								0x0125			//��
#define SPKM_POINT							0x0126			//��
#define SPKM_JIAO							0x0127			//��
#define SPKM_PASSIN						0x0128			//�����뿨����
#define SPKM_PASERR						0x0129			//�����������������
#define SPKM_CARDVALID				0x0130			//��Ч��
#define SPKM_OILACK						0x0131			//�����ѻ��㣬��ȷ��
#define SPKM_FAULT							0x0132			//���ֹ�����֪ͨ������Ա
#define SPKM_OILFILL						0x0133			//����ǹ��ע
#define SPKM_NUMBEROIL				0x0134			//����Ʒ
#define SPKM_THISVOLUME				0x0135			//���˴εļ�������
#define SPKM_SEEYOU						0x0136			//ף��һ·ƽ��,��ӭ�´ι���
#define SPKM_TOTALMONEY				0x0137			//�������ۼƽ��
#define SPKM_ICORCODE					0x0138			//�����������IC����ѡ����Ʒ��ɨ������
#define SPKM_CODESCAN					0x0139			//��������ɨ������
#define SPKM_AUTHORIZEMONEY		0x0140			//���������μ�����Ȩ���Ϊ
#define SPKM_CODEERROR				0x0141			//������ɨ�����������ɨ��
#define SPKM_CODEEXPIRED			0x0142			//����������СƱ�ѹ��ڣ��뵽Ӫҵ�Ҵ���
#define SPKM_CODEINAGINE			0x0143			//��������֤���������������
#define SPKM_SELECTERR					0x0144			//��������Ʒѡ�������ѡ���Ӧ����Ʒ
#define SPKM_GUNUSING					0x0145			//����������ǹ����ʹ�ã����Ժ�
#define SPKM_OILEND						0x0146			//���������μ�������ɣ���Ż���ǹ
#define SPKM_OILBALANCE				0x0147			//���������μ�����������ƾ��ӡƱ�ݵ�Ӫҵ���˿�
#define SPKM_ICORMONEY				0x0148			//�����������IC����ѡ����Ʒ��Ͷ��ֽ��
#define SPKM_DIE0CAR						0x0149			//������0�ų��ò���
#define SPKM_THISMONEYIN			0x0150			//����������Ͷ�ҽ��Ϊ
#define SPKM_OILPLEASE					0x0151			//����������ǹ���ͻ��Զ�����ʽ����
#define SPKM_INVOICE						0x0152			//���������跢Ʊ����ƽ��ӡƱ�ݵ�Ӫҵ�һ�ȡ
#define SPKW_DIESELFUEL				0x0153			//Ů��������
#define SPKM_GASONLINE				0x0154			//����������
#define SPKM_DIESELFUEL				0x0155			//����������
#define SPKW_ICCARDIN					0x0156			//Ů���������IC��
#define SPKM_ICCARDIN					0x0157			//�����������IC��
#define SPKM_GAS95							0x0158			//������95������
#define SPKW_SPACE							0x0159			//����
#define SPKW_GAS92							0x0160			//Ů����92������
#define SPKW_GAS89							0x0161			//Ů����89������
#define SPKM_GAS92							0x0162			//������92������
#define SPKM_GAS89							0x0163			//������89������
#endif

#endif //#if APP_SINOPEC



#if APP_CNPC

//���������б�Ů��
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
#define SPKW_TEN								0x0010			//ʮ
#define SPKW_HUNDRED					        0x0011			//��
#define SPKW_THOUSAND					        0x0012			//ǧ
#define SPKW_TENTHOUSAND			            0x0013			//��
#define SPKW_YUAN							    0x0014			//Ԫ
#define SPKW_SHENG							    0x0015			//��
#define SPKW_POINT							    0x0016			//��
#define SPKW_PLUS							    0x0017			//��

#endif //#if APP_CNPC




//�������ݲ��Žڵ�
typedef struct
{
	unsigned char *Buffer;					//��������
	unsigned int Length;						//�������ݳ���
	unsigned int Pointer;						//�������ݲ����±�

}SPKPlayDataStruct;


//�����������ݽṹ
typedef struct
{
	//��������
	char Speaker;								//������ѡ��0=������������1=�ⲿ��չ�豸������
	int iFdCom;									//�����ⲿ��չ�豸�������Ĵ���
	int tIdListSpk;								//���������б�������

	//���������������ݣ����Ǳ��β�����Ҫ
	unsigned char VoiceListPointer;									//��ǰ���������±�
	unsigned char VoiceListLength;									//���������б�����������Ŀ0~SPK_MAX_NUMBER
	unsigned int VoiceList[SPK_MAX_NUMBER];					//Ҫ���ŵ����������к�2bytesΪһ��

	//�����������ݣ�������TC�ж��в��ŵ���������
	unsigned int SampRate;												//������
	unsigned char Volume;													//����


	SPKPlayDataStruct SpkData[SPK_MAX_NUMBER];		//����������
	unsigned char SpkDataLength;									//��������Ŀ
	unsigned char SpkDataPointer;									//�����ε�ǰ�±�ָ��
}SpkStructType;


extern int spkNextFileNameGet(const char *param, char *nextname, int maxbytes, char *thisname, int nextflag, int type);
extern int spkFileNameGet(const char *param, char *name, int maxbytes);
extern int spkDeviceSet(int speaker, int device);
extern int spkPlay(int speaker, int volume, int *voicelist, int voicenumber);
extern bool spkDevInit(void);

int tSpeaker(int speaker);


#endif



