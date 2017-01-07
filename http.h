/*
	Duke 2016
	
*/
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#ifdef __WIN32__
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#endif
int httpConnect(char *host);
int httpClose(SOCKET sd);
int httpSend(SOCKET sockfd, char *filename, unsigned char *data, int size, char *formname, char *path, char *host);
int httpSendRom(SOCKET sd, char *filename, unsigned char *data, int size, int slot, char *path, char *host, char *slotname);
int httpResponse(SOCKET sockfd);
int httpGet(SOCKET sd, char *host, char *url, int skipheader);
