#include "../inc/main.h"


void td_GasPoll()
{
	char rxbuffer[32] = {0};
	int nLen = 0;
	int i = 0;
	FOREVER
	{
		i++;
		comWrite(COM1,"0123456789",10);
		printf("[%s][%d][%x]Send Over.\n", __FUNCTION__, __LINE__, i);
		sleep(1);
	}
}

