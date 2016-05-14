/*
	CPC xfer.... transfer files to/from M4 board sd card, via command line, useful for ie. crossdev
	
	Duke 2016.

*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "http.h"
#include "parse.h"
#include "cpc.h"


void dispInfo(char *exename)
{
	printf("CPC M4 xfer tool v1.0.0 - Duke 2016\r\n");
	printf("%s -u ipaddr file path opt\t\t- Upload file, opt 0: no header add, 1: add ascii header\r\n", exename);
	printf("%s -d ipaddr file path opt\t\t- Download file, opt 0: leave header, 1: remove header\r\n", exename);

}

void upload(char *filename, char *path, char *ip, int opt)
{
	int ret, size, p, n, k, i, j;
	char fullpath[256];	// don't exceed this, the cpc wouldn't be able |cd it anyway 
	FILE *fd;
	
	SOCKET sd;
	
	unsigned char *buf;
	fd = fopen(filename, "rb");
	if ( fd == NULL )
	{	printf("file %s not found\n", filename);
		return;
	}
	
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	
	buf = malloc(size+0x80);
	if ( buf == NULL )
	{
		printf("Not enough memory!\n");
		fclose(fd);
		return;
	}
	
	if ( opt != 0 )
	{	_cpchead *cpcheader = (_cpchead *)buf;
		memset(cpcheader, 0, 0x80);
		
		// set up cleaned filename and extionsion
		formatfn(cpcheader->filename, filename);
		getExtension(cpcheader->extension, filename);
		
		// add more file types here... and more paramters....
		
		switch (opt)
		{
			default:		// just ascii for now
			cpcheader->addr = 0x172;	// protext uses this
			cpcheader->type = 10; 	
			cpcheader->size = size;
			cpcheader->size2 = size;
			cpcheader->checksum = checksum16(buf, 66);
			break;
			
		}
		
		fread(&buf[0x80], size, 1, fd);
		size+=0x80;
	}
	else
		fread(buf, size, 1, fd);
	
	fclose(fd);
	

	ret = httpConnect(ip);
	if ( ret >= 0 )
	{	sd = ret;	// connect socket
	
		p = pathPos(filename, strlen(filename));	// strip any PC path from filename
		sprintf(fullpath, "%s/%s", path, &filename[p]);
		
		k = strlen(fullpath);
		n = 0; 
		while ( n < k ) // remove leading / from path
		{
			if ( fullpath[n] != '/' )
				break;
			n++;
		}
		
		// remove duplicate /'s
	
		k = strlen(fullpath);
		j = 0;
		for (i=n; i < (k+1); i++)
		{
			if ( (fullpath[i] == '/') && (fullpath[i+1] == '/') )
				i++;
			
			fullpath[j++] = fullpath[i];
		}
		
		if ( httpSend(sd, fullpath, buf, size, "upfile", "/upload.html", ip) >= 0 )
			ret = httpResponse(sd);
		
		httpClose(sd);
		if ( ret == 200 )
			printf("Upload OK!\r\n");
		else
			printf("Upload error code. %i\r\n", ret);
	}
	else
		printf("Connect to %s failed\n", ip);

	free(buf);
}

void download(char *filename, char *path, char *ip, int opt)
{
	SOCKET sd;
	int ret, k, i, j;
	char fullpath[256];
	sprintf(fullpath, "sd/%s/%s", path, filename);

	// remove duplicate /'s
	
	k = strlen(fullpath);
	j = 0;
	for (i=0; i < (k+1); i++)
	{
		if ( (fullpath[i] == '/') && (fullpath[i+1] == '/') )
			i++;
		
		fullpath[j++] = fullpath[i];
	}
	ret = httpConnect(ip);
	
	sd = ret;
	if ( ret >= 0 )
	{	
		if ( httpGet(sd, ip, fullpath, opt) == 0)
			printf("Downloaded succesfully.\r\n");
		else
			printf("Download failed.\r\n");
	}
	else
		printf("Connect failed, wrong ip?\r\n");
}

int main(int argc, char *argv[])
{	int opt = 0;
	
#ifdef __WIN32__
	WSADATA WsaDat;
#endif

	if ( argc < 4 )
	{	
		dispInfo(argv[0]);
		exit(0);
	}
	
	// upload
	if ( !strnicmp(argv[1], "-u", 2) )
	{
		
#ifdef __WIN32__
		WSAStartup(MAKEWORD(2,2),&WsaDat);		
#endif
		if ( argc < 5 )
		{
			dispInfo(argv[0]);
			exit(0);
		}
		
		if ( argc>5 )
			opt = atoi(argv[5]);
		
		upload(argv[3], argv[4], argv[2], opt);
	}
	else if ( !strnicmp(argv[1], "-d", 2) )	// download
	{	
#ifdef __WIN32__
		WSAStartup(MAKEWORD(2,2),&WsaDat);		
#endif
		
		if ( argc>5 )
			opt = atoi(argv[5]);

		download(argv[3], argv[4], argv[2], opt);
	}
	else
	{	
		dispInfo(argv[0]);
		exit(0);
	}
	

#ifdef __WIN32__	
	WSACleanup();
#endif
	
	return 0;
}

