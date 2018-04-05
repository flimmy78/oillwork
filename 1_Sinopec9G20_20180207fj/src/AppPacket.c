#include "../inc/main.h"



void InitParam()
{
	while(1)
	{
		printf("test \n");
		//break;
		//sleep(10);
	}
}

void InitTestParentTd()
{
	while(1)
	{
		printf("parent thread\n");
		//sleep(16);
	}
}

void InitTestChildTd()
{
	while(1)
	{
		printf("child thread\n");
		//sleep(22);
	}
}
