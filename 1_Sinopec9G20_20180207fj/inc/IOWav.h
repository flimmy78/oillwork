#ifndef __IO_WAV_H__
#define __IO_WAV_H__





extern void SetOfControlThan(int nfd,int nNewVolume);
extern int OpenWavDev(char* pchDev);
extern void PlayWavFile(int nfd,char* pchWavName,int nVolume);








#endif

