#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <termios.h>  
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define FALSE 0
#define TRUE  1

int speed_arr[] = { 
	B3000000, B921600, B460800, B230400, B115200, B57600, B38400, B19200, 
	B9600, B4800, B2400, B1200, B300, 
};

int name_arr[] = {
	3000000, 921600, 460800, 230400, 115200, 57600, 38400,  19200,  
	9600,  4800,  2400,  1200,  300,  
};

int fd[2];    //两个文件描述符
pthread_t threads[10];//线程标识符
char buff[101];
char buff2[101];
static struct termios newtios,oldtios; 
static int saved_portfd=-1;   

static void reset_tty_atexit(void)
{
	if(saved_portfd != -1)
	{
		tcsetattr(saved_portfd,TCSANOW,&oldtios);
	}                      
}

/*clean up signal handler */
static void reset_tty_handler(int signal)
{
	if(saved_portfd != -1)
	{
		tcsetattr(saved_portfd,TCSANOW,&oldtios);//设置终端参数
	}
	_exit(EXIT_FAILURE);
}

/*
int open_port(const char *portname)
{
	struct sigaction sa;
	int portfd;
	printf("opening serial port:%s\n",portname);
	
	if((portfd=open(portname,O_RDWR | O_NOCTTY)) < 0 )
	{
   		printf("open serial port %s fail \n ",portname);
   		return portfd;
	}	
	tcgetattr(portfd,&newtios);  
	memcpy(&oldtios,&newtios,sizeof newtios);	
	cfmakeraw(&newtios);  
	newtios.c_iflag |=IGNPAR; 
	newtios.c_oflag &= ~(OPOST | ONLCR | OLCUC | OCRNL | ONOCR | ONLRET | OFILL); 
	newtios.c_cflag = CS8 | CLOCAL | CREAD;
	
//	newtios.c_cc[VMIN]=1;
//	newtios.c_cc[VTIME]=0; 	

	newtios.c_cc[VTIME] = 20; //2 seconds
	newtios.c_cc[VMIN] = 0;
//	newtios.c_cc[VTIME] = 150; //15 seconds
//	newtios.c_cc[VMIN] = 0;
	
	cfsetospeed(&newtios,B115200); 
	cfsetispeed(&newtios,B115200); 
	//cfsetospeed(&newtios,B9600); 
	//cfsetispeed(&newtios,B9600); 
	atexit(reset_tty_atexit);
	memset(&sa,0,sizeof sa);
	sa.sa_handler = reset_tty_handler;
	sigaction(SIGHUP,&sa,NULL);
	sigaction(SIGINT,&sa,NULL);
	sigaction(SIGPIPE,&sa,NULL);
	sigaction(SIGTERM,&sa,NULL);
	//apply modified termios 
	saved_portfd=portfd;
	tcflush(portfd,TCIFLUSH);
	tcsetattr(portfd,TCSADRAIN,&newtios);
	               
	return portfd;
}*/


int open_port(const char *portname,int speed)
{
	struct sigaction sa;
	int portfd;
	printf("opening serial port:%s\n",portname);

	printf("speed = %d\n",speed);
	
	if((portfd=open(portname,O_RDWR | O_NOCTTY)) < 0 )
	{
   		printf("open serial port %s fail \n ",portname);
   		return portfd;
	}	
	tcgetattr(portfd,&newtios);  
	memcpy(&oldtios,&newtios,sizeof newtios);	
	cfmakeraw(&newtios);  
	newtios.c_iflag |=IGNPAR; 
	newtios.c_oflag &= ~(OPOST | ONLCR | OLCUC | OCRNL | ONOCR | ONLRET | OFILL); 
	newtios.c_cflag = CS8 | CLOCAL | CREAD;
	
//	newtios.c_cc[VMIN]=1;
//	newtios.c_cc[VTIME]=0; 	

	newtios.c_cc[VTIME] = 30; //500ms
	newtios.c_cc[VMIN] = 0;
//	newtios.c_cc[VTIME] = 150; //15 seconds
//	newtios.c_cc[VMIN] = 0;

	int i;
	for (i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{
		if (speed == name_arr[i])
		{
	        cfsetospeed(&newtios, speed_arr[i]);
			cfsetispeed(&newtios, speed_arr[i]);	
			break;
		}
	}

	//cfsetospeed(&newtios,B9600); 
	//cfsetispeed(&newtios,B9600); 
	atexit(reset_tty_atexit);
	memset(&sa,0,sizeof sa);
	sa.sa_handler = reset_tty_handler;
	sigaction(SIGHUP,&sa,NULL);
	sigaction(SIGINT,&sa,NULL);
	sigaction(SIGPIPE,&sa,NULL);
	sigaction(SIGTERM,&sa,NULL);
	//apply modified termios 
	saved_portfd=portfd;
	tcflush(portfd,TCIFLUSH);
	tcsetattr(portfd,TCSADRAIN,&newtios);

	return portfd;
}

void * process1(void* arg)
{
	int portfd = (int) arg;	
	int rev1, rev2;	
	rev1 =0;
	rev2 =0;
	while(rev2 < 100)
 	{
		rev1 = write(portfd,(buff + rev2),100);
		rev2 += rev1;
 	}
	printf("\n uart1 send %d byts\n", rev2);	
}	

/*
int main(int argc, char **argv)
{
	char *dev[10]={"/dev/ttyS2", "/dev/ttyS3"};
	unsigned int i;
	printf("\n demo uart1/uart2 external loop back function \n");
	for(i = 0; i < 100; i++)
	{
		buff[i] = (i & 0xff);
	}
	
	
	if((fd[0] = open_port(dev[0]))<0)
   		return -1;
		
	pthread_create(&threads[0], NULL, process1, (void*)(fd[0]));	
	pthread_join(threads[0], NULL);	
  	return 0;
}*/
