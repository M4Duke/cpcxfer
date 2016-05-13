/*
	Duke 2016
	
	Quick and Dirty HTTP POST and HTTP GET implementations!
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

#include "parse.h"
#include "cpc.h"

#define BOUNDARY_ID		"-------------123123123"

int httpConnect(char *host)
{
	SOCKET sd;
	struct sockaddr_in servaddr;
	struct in_addr addr;
	char **pptr;
	int i;
	struct hostent *hptr;
	if ((hptr = gethostbyname(host)) == NULL) 
	{
		return -1;
	}

	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL) 
	{
		
		sd = socket(AF_INET, SOCK_STREAM, 0);
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(80);
		i = 0;
		while (hptr->h_addr_list[i] != 0) 
			addr.s_addr = *(u_long *) hptr->h_addr_list[i++];
    
		servaddr.sin_addr = addr;

		if ( connect(sd,(struct sockaddr *) &servaddr, sizeof(servaddr) ) >= 0  )
			return sd;
	}

	return -1;
}
int httpClose(SOCKET sd)
{
	if ( sd > 0 )
	{
#ifdef __WIN32__
		closesocket(sd);
#else
		close(sd);
#endif
		
		
		return 0;
	}

	return -1;
}

int httpSend(SOCKET sd, char *filename, unsigned char *data, int size, char *formname, char *path, char *host)
{
	int i, chunkSize, contentLen;
	char httpHeader[1024];
	char httpContent[1024];
	char httpEnd[256];
	
	contentLen = sprintf(httpEnd,  "\r\n\r\n--%s--", BOUNDARY_ID);
	contentLen += sprintf(httpContent, "--%s\r\nContent-Disposition: form-data; name=\"upfile\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n", BOUNDARY_ID, filename);
	contentLen +=size;
	// send HTTP POST header
	i = sprintf(httpHeader, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Type: multipart/form-data; boundary=%s\r\nContent-Length: %d\r\r\n\r\n", path, host, BOUNDARY_ID,  contentLen);
	send(sd, httpHeader, strlen(httpHeader), 0);
	// send content
	send(sd, httpContent, strlen(httpContent), 0);
	// actual data
	i = 0;
	while ( i < size )
	{	
		if ( (size-i) < 1460 )
			chunkSize = size-i;
		else
			chunkSize = 1460;

		send(sd, (char *)&data[i], chunkSize, 0);
		
		i+=chunkSize;
	}
	
	// send boundary end
	send(sd, httpEnd, strlen(httpEnd), 0);

	return 0;
}
int getStringValue(char *string, char *buf, int size)
{	char len[16];
	int i, j, k;
	k = strlen(string);
		
	i = 0;
	while ( i < (size-k) )
	{
		if ( !strnicmp(&buf[i], string, k) )
		{	
			i+=(k+2);
			j = 0;
			while ( i < size )
			{
				
				if ( (buf[i] >= '0') && (buf[i] <= '9') )	// skip leading spaces and other stuff
					len[j++] = buf[i];
				else
					break;	
				i++;
			}
			len[j++] = 0;
			
			return atoi(len);
		
		}
		i++;
	}
	
	return -1;	
}

int httpResponse(SOCKET sd)
{
	int n, i, ret;
	char *response;
	char *line;

	
	
	
	response = malloc(4096);
	
	memset(response, 0, 4096);

	i = 0;	
	while (i < 4096) 
	{
		n = recv(sd, &response[i], 4096-i, 0 );

		if ( n<= 0 )
			break;
		
		i+=n;
	}
	
	ret = getStringValue("HTTP/1.", response, i);
	free(response);
	return ret;
}

// send HTTP GET request and process response
int httpGet(SOCKET sd, char *host, char *url, int skipheader)
{
	char httpReq[512];
	char response[1460];
	int i, n, pos;
	memset(response, 0, sizeof(response));
	
	i = sprintf(httpReq, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: cpcxfer\r\n\r\n", url, host);
	send(sd, httpReq, i, 0);

	while (1)
	{
		n = recv(sd, &response[0], sizeof(response),0 );
		
		if ( n <= 0 )
			break;
			
		// should do more buffer rotation, reloading and checking... but it's not needed as its always in one response - atleast here :P
			
		pos = findString("200 OK", response, n);
		if ( pos > 0 )
		{	i = findString("Content-type: text/plain", &response[pos], n-pos);
			if (i > 0 )
			{
				pos+=i;
				// now find cr/lf/cr/lf and start saving...
				i = findString("\r\n\r\n", &response[pos], n-pos);
				if ( i > 0 )
				{
					
					FILE *f;
					pos+=i;
					// get filename from url
					f = fopen(&url[pathPos(url, strlen(url))], "wb");
					// rotate buffer if anything left
					memcpy(&response[0], &response[pos], n-pos);
					pos = n-pos;
					
					while (1)
					{
						n = recv(sd, &response[pos], sizeof(response)-pos,0 );
						
						n+=pos;
						if ( n==0 )
							break;
						if ( (pos != 0) && skipheader )
						{	// check if its a valid header at all (not going to remove it, if it doesnt have one!)
							_cpchead *cpcheader = (_cpchead *)response;
							
							if ( checksum16(response, 66) == cpcheader->checksum )
								fwrite(&response[0x80], n-0x80, 1, f);
							else
								fwrite(&response[0], n, 1, f);
						}
						else
							fwrite(&response[0], n, 1, f);
						
						pos = 0;
					}
					fclose(f);
					return 0;
				}
				
			}
		}
		
		
	}

	return -1;
}