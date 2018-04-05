#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h> 
#include<unistd.h> 
#include <sys/ioctl.h>
#include "../inc/IOWav.h"

#define SAMPL_40K    	625
//#define SAMPL_200K    	125
#define SAMPL_200K  625

//#define SAMPL_100K      250
//#define SAMPL_80K       312

//#define SAMPL_200K    312

//#define SAMPL_200K 49
//#define GPIO2PWM	1
//#define PWM2GPIO	0
unsigned int len = 0;
int g_nOldVolume = 0;

// define Wave format structure
typedef struct tWAVEFORMATEX
{
    short wFormatTag;         /* format type */
    short nChannels;          /* number of channels (i.e. mono, stereo...) */
    unsigned int nSamplesPerSec;  /* sample rate */
    unsigned int nAvgBytesPerSec; /* for buffer estimation */
    short nBlockAlign;        /* block size of data */
    short wBitsPerSample;     /* number of bits per sample of mono data */
    short cbSize;             /* the count in bytes of the size of */
                              /* extra information (after cbSize) */
} WAVEFORMATEX, *PWAVEFORMATEX;

// read wave file
unsigned char* wavread(char *fname, WAVEFORMATEX *wf,int* pnDataLen)
{
  FILE* fp;
  char str[32];
  char *speech;
  unsigned int subchunk1size; // head size
  unsigned int subchunk2size; // speech data size

  // check format type
  fp = fopen(fname,"r");
  if(!fp)
  {
      fprintf(stderr,"Can not open the wave file: %s.\n",fname);
      return NULL;
  }
  fseek(fp, 8, SEEK_SET);
  fread(str, sizeof(char), 7, fp);
  str[7] = '\0';
  if(strcmp(str,"WAVEfmt"))
  {
      fprintf(stderr,"The file is not in WAVE format!\n");
      return NULL;
  }
  
  // read format header
  fseek(fp, 16, SEEK_SET);
  fread((unsigned int*)(&subchunk1size),4,1,fp);
  fseek(fp, 20, SEEK_SET);
  fread(wf, subchunk1size, 1, fp);
  
  // read wave data
  fseek(fp, 20+subchunk1size, SEEK_SET);
  fread(str, 1, 4, fp);
  str[4] = '\0';
  if(strcmp(str,"data"))
  {
      fprintf(stderr,"Locating data start point failed!\n");
      return NULL;
  }
  fseek(fp, 20+subchunk1size+4, SEEK_SET);
  fread((unsigned int*)(&subchunk2size), 4, 1, fp);
  speech = (char*)malloc(sizeof(char)*subchunk2size);
  if(!speech)
  {
      fprintf(stderr, "Memory alloc failed!\n");
      return NULL;
  }
  fseek(fp, 20+subchunk1size+8, SEEK_SET);
  fread(speech, 1, subchunk2size, fp);
	//len = subchunk2size;
	*pnDataLen = subchunk2size;
  fclose(fp);
  return speech;
}

void SetOfControlThan(int nfd,int nNewVolume)
{
    float number = 0;
	int zbuf;
	unsigned int samprate = SAMPL_200K;
	//number = (100*(0x80 * 1.0))/255;
	//zbuf = samprate-(((100-(int)number)*samprate)/(((nNewVolume+10)/10) * 100));
	zbuf = 313;

	unsigned short ushCmd = 0x55aa;
	//ioctl(nfd,2,&zbuf);
	ioctl(nfd,ushCmd,&zbuf);
	//printf("******ioctl******,nfd = %d,zbuf = %d \n",nfd,zbuf);
}

int OpenWavDev(char* pchDev)
{
	int nfd = -1;
	if(pchDev != NULL)
	{
        nfd = open(pchDev,O_RDWR);
        if(nfd < 0)
		{
		    printf("open %s fail !!!\n",pchDev);
			return -1;
		}
	}

	//if(nVolume != g_nOldVolume)
	//{
	//	printf("nVolume = %d,g_nOldVolume = %d\n",nVolume,g_nOldVolume);
	//	SetOfControlThan(nfd,nVolume);
		//g_nOldVolume = nVolume;
	//}

	return nfd;
}

void PlayWavFile(int nfd,char* pchWavName,int nVolume)
{
	WAVEFORMATEX wf;
    unsigned char *data;
	unsigned char *dataTemp = NULL;
	int zbuf[50000];
	float number = 0;
	//int   volume = 1;//语音音量,0~99
	int volume = nVolume;
	//printf("nVolume = %d\n",nVolume);
	unsigned int samprate = SAMPL_200K;
	unsigned int i;

	int nDataLen = 0;

	//if(nVolume != g_nOldVolume)
	//{
	//	printf("------nVolume = %d,g_nOldVolume = %d\n",nVolume,g_nOldVolume);
	//	SetOfControlThan(nfd,nVolume);
	//	g_nOldVolume = nVolume;
	//}
	
	data = wavread(pchWavName, &wf,&nDataLen);

	if(data == NULL)
	{
		return;
	}

	dataTemp = data;

	//for(i=0;i<len;i++)
	for(i = 0; i < nDataLen; i++)
	{
		//number = (100*((*data) * 1.0))/255;			
        //zbuf[i] = samprate-(((100-(int)number)*samprate)/(((volume+10)/10)*100));
		
		number = 1.0 * (0x80 - (*data-0x80)*(100-volume)/100.0)/256;
		zbuf[i] = (int)(samprate - samprate*number + 0.5);
		
		data++;
	}

//	printf("wav test -------------------\n");
//	PrintH(256,zbuf);

	//ioctl(nfd,1,0);
	//write(nfd,zbuf,len);
	write(nfd,zbuf,nDataLen);
	//ioctl(nfd,0,0);
	
	if(dataTemp != NULL)  //20171201
	{
		free(dataTemp);
	}
}


/*
int main(int argc,char *argv[])
{
	WAVEFORMATEX wf;
	char fname[] = "/test.wav";
  unsigned char *data;
	unsigned char zbuf[50000];
	float number = 0;
	int volume = 1;//语音音量,0~99
	unsigned int samprate = SAMPL_200K;
	int fd;	
	unsigned int i;
	fd = open("/dev/972-pwm0",O_RDWR);
	if(fd < 0)
		printf("open /dev/972-pwm0 fail !!!\n");
		
  data = wavread(fname, &wf);
  for(i=0;i<len;i++)
	{
		number = (100*((*data) * 1.0))/255;
		zbuf[i] = samprate-(((100-(int)number)*samprate)/(volume*100));
		data++;
	}
	  
	write(fd,zbuf,len);	

	close(fd);
}*/
