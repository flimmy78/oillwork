#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../inc/spiDev.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
void pabort(const char *s)
{
	perror(s);
	abort();
}
static const char *device = "/dev/spidev1.1";
static uint32_t CS_H = 3;
static uint32_t CS_L = 7;
static uint32_t mode = SPI_MODE_3;
static uint8_t bits = 16;
//static uint32_t speed = 300000;
static uint32_t speed = 100000;
static uint16_t delay;
static int fd;

void transfer(uint16_t const *tx, uint16_t const *rx, size_t len)
{
	int ret,i;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");
			
	/*for(i=0;i<9;i++)
	{
		printf("%d\n", *rx);
		rx++;
	}*/
}

bool initSpiDev()
{
	int ret;
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		pabort("can't open device");
		return false;
	}
	//spi mode
	
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		pabort("can't set spi mode");
		return false;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	{
		pabort("can't get spi mode");
		return false;
	}
	//bits per word
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		pabort("can't set bits per word");
		return false;
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		pabort("can't get bits per word");
		return false;
	}
	//max speed hz
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		pabort("can't set max speed hz");
		return false;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		pabort("can't get max speed hz");
		return false;
	}

	return true;
}

void CSHIGH()
{
	ioctl(fd,SPI_IOC_WR_MODE,&CS_H);
}

void CSLOW()
{
	ioctl(fd,SPI_IOC_WR_MODE,&CS_L);
}

/*
int main(int argc, char *argv[])
{
	int ret = 0;
	int fd,i;	
	uint16_t tx[9] = {0x1111,0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888,0x9999};
	uint16_t rx[20];
	int size;
	int cmd = 6;
	
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");
	//spi mode
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");
	//bits per word
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");
	//max speed hz
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");
	
  printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	
 	
	//transfer(fd, tx, rx, 9);
	
	cmd = atoi(argv[1]);
	if(cmd == 1)
	{
		ioctl(fd,IO_CS_HIGH);
	}
	else if(cmd == 0)
	{
		ioctl(fd,IO_CS_LOW);
	}
	else
	{
		
	}
		
	close(fd);
	return ret;
}*/
