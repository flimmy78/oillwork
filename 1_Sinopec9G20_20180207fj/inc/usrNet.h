#ifndef _USR_NET_H_
#define _USR_NET_H_


extern int usrNetIpGet(char *interfaceName, char *interfaceAddress);
extern int usrNetIpSet(char *interfaceName, char *interfaceAddress);
extern int usrNetMaskGet(char *interfaceName, int *netMask);
extern int usrNetMaskSet(char *interfaceName, int netMask);
extern int usrNetHostGatewayGet(char *interfaceName, char *gatewayAddress);
extern int usrNetHostGatewaySet(char *interfaceName, char *gatewayAddress);
extern int usrNetEMACGet(char *interfaceName, char *emacAddr);
extern int usrNetEMACSet(char *interfaceName, char *emacAddr);


#endif

