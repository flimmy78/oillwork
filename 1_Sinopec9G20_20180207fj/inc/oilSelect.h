#ifndef _OIL_SELECT_H_
#define _OIL_SELECT_H_

#if 0
extern int oilSelectLit(int nozzle);
extern int oilSelectShut(int nozzle);
extern int oilSelectState(int nozzle);
#endif

extern int oilNameGet(unsigned char *code, unsigned char *buffer, int maxbytes);
extern int YPLightTurnOn(int panel, int YPx);
extern int YPLightTurnOff(int panel, int YPx);


#endif

