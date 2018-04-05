#ifndef __IO_PIN_CONFIG_H__
#define __IO_PIN_CONFIG_H__



int READ_PE2_FLAG();
int WRITE_PE2_FLAG(int nFlag);
int GET_GPIO_VAL(int Gpx);
int SET_GPIO_HIGH(int Gpx);
int SET_GPIO_Low(int Gpx);
bool InitIoPinConfig();













#endif
