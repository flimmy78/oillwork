#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h> 
#include<unistd.h> 
#include<sys/ioctl.h>
#include<stdbool.h>

#include "../inc/IOPinConfig.h"

#define GPA11   0x01 //���ͻ�������
#define GPB4    0x02 //���ڸ�λ�ź�
#define GPB5    0x03 //Ӳ���汾ʶ��
#define GPD9    0x04
#define GPD10   0x05
#define GPE2    0x06 //����205оƬ���ж�����
#define GPE10   0x07 //���İ�STM�ĸ�λ����
#define GPE11   0x08 //���İ�STM��ι���ź�
#define GPG0    0x09
#define GPG1    0x0a
#define GPG5    0x0b //���ͻ�������ʾ
#define GPG10   0x0c
#define GPG13   0x0d
#define GPG14   0x0e
#define GPH2    0x0f
#define GPH3    0x10

#define my_MAGIC  'K'
#define GPIO_IN_MODE      _IO(my_MAGIC,0) 
#define GPIO_OUT_MODE     _IO(my_MAGIC,1) 
#define GPIO_SET_HIGH     _IO(my_MAGIC,2) 
#define GPIO_SET_LOW      _IO(my_MAGIC,3) 
#define GPIO_GET_VAL      _IO(my_MAGIC,4)

static int g_nfd_io = -1;

int READ_PE2_FLAG()//�ú��������ڶ� PE2 �ı�־λ
{
	int flag = 0;
	ioctl(g_nfd_io,GPIO_GET_VAL,GPE2);
	read(g_nfd_io,&flag,4);
	return flag;
}

int WRITE_PE2_FLAG(int nFlag)
{
	write(g_nfd_io,&nFlag,4);//д PE2 �ı�־λ
}

int GET_GPIO_VAL(int GPx)//�����ŵĵ�ƽֵ
{
	int gpio_val = 0;
	ioctl(g_nfd_io,GPIO_GET_VAL,GPx);
	read(g_nfd_io,&gpio_val,4);
	return gpio_val;
}

int SET_GPIO_HIGH(int GPx)
{
	ioctl(g_nfd_io,GPIO_OUT_MODE,GPx);
	ioctl(g_nfd_io,GPIO_SET_HIGH,GPx);
}

int SET_GPIO_LOW(int GPx)
{
	ioctl(g_nfd_io,GPIO_OUT_MODE,GPx);
	ioctl(g_nfd_io,GPIO_SET_LOW,GPx);
}

bool InitIoPinConfig()
{
	int cmd = 60;
	int PE2_flag;
	int GPIO_val;
	g_nfd_io = open("/dev/NUC970_PE2",O_RDWR);
	if(g_nfd_io<0)
	{
		printf("open fail\n");
		return false;
	}

    return true;

/*	cmd = atoi(argv[1]);
	if(cmd == 1)
	{				
		PE2_flag = RD_PE2_FLAG(fd);//�� PE2 �ı�־λ
		printf("app read PE2_flag is %d\n",PE2_flag);
	}
	else if(cmd == 2)
	{	
		PE2_flag = 0;	
		write(fd,&PE2_flag,4);//д PE2 �ı�־λ
	}
	else if(cmd == 3)
	{	
		ioctl(fd,GPIO_IN_MODE,GPA11);//����Ϊ����ģʽ
	}
	else if(cmd == 4)
	{	
		ioctl(fd,GPIO_OUT_MODE,GPA11);//Ĭ������ߵ�ƽ		
	}
	else if(cmd == 5)
	{	
		GPIO_val = GET_GPIO_VAL(fd,GPA11);//�� PA11 �ĵ�ƽֵ
		printf("app read PA11 is %d\n",GPIO_val);
	}
	else if(cmd == 6)
	{	
		ioctl(fd,GPIO_SET_LOW,GPA11);//����Ϊ�͵�ƽ
	}
	else if(cmd == 7)
	{	
		ioctl(fd,GPIO_SET_HIGH,GPA11);//����Ϊ�ߵ�ƽ
	}
	close(fd);*/
}

/*

int main(int argc,char *argv[])
{
	int fd;
	int cmd = 60;
	int PE2_flag;
	int GPIO_val;
	fd = open("/dev/NUC970_PE2",O_RDWR);
	if(fd<0)
		printf("open fail\n");

	cmd = atoi(argv[1]);
	if(cmd == 1)
	{				
		PE2_flag = RD_PE2_FLAG(fd);//�� PE2 �ı�־λ
		printf("app read PE2_flag is %d\n",PE2_flag);
	}
	else if(cmd == 2)
	{	
		PE2_flag = 0;	
		write(fd,&PE2_flag,4);//д PE2 �ı�־λ
	}
	else if(cmd == 3)
	{	
		ioctl(fd,GPIO_IN_MODE,GPA11);//����Ϊ����ģʽ
	}
	else if(cmd == 4)
	{	
		ioctl(fd,GPIO_OUT_MODE,GPA11);//Ĭ������ߵ�ƽ		
	}
	else if(cmd == 5)
	{	
		GPIO_val = GET_GPIO_VAL(fd,GPA11);//�� PA11 �ĵ�ƽֵ
		printf("app read PA11 is %d\n",GPIO_val);
	}
	else if(cmd == 6)
	{	
		ioctl(fd,GPIO_SET_LOW,GPA11);//����Ϊ�͵�ƽ
	}
	else if(cmd == 7)
	{	
		ioctl(fd,GPIO_SET_HIGH,GPA11);//����Ϊ�ߵ�ƽ
	}
	close(fd);
}*/






