/*
	CPC xfer.... transfer files to/from M4 board sd card, via command line, useful for ie. crossdev
	
	Duke 2016/2017

*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "http.h"
#include "parse.h"
#include "cpc.h"


#ifndef __WIN32__
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>
#endif

// mres = m4 reboot
// cres = cpc reset
// chlt = cpc pause/unpause (BUSRQ)
// run = run file on sd at startup

void dispInfo(char *exename)
{
	printf("CPC M4 xfer tool v2.0.1 - Duke 2016/2017\r\n");
	printf("%s -u ipaddr file path opt\t\t- Upload file, opt 0: no header add, 1: add ascii header\r\n", exename);
	printf("%s -d ipaddr file path opt\t\t- Download file, opt 0: leave header, 1: remove header\r\n", exename);
	printf("%s -f ipaddr file slot name\t\t- Upload rom\r\n", exename);
	printf("%s -x ipaddr path+file\t\t- Execute file on CPC\r\n", exename);
	printf("%s -y ipaddr local_file\t\t- Upload file on CPC and execute it immediatly (the sd card must contain folder '/tmp')\r\n", exename);
	printf("%s -s ipaddr\t\t\t\t- Reset CPC\r\n", exename);
	printf("%s -r ipaddr\t\t\t\t- Reboot M4\r\n", exename);
	
}
void reset(char *ip)
{	char httpReq[128];
	int sd;
	volatile int n;
	
	sd = httpConnect(ip);
	
	if ( sd > 0 )
	{	
		
		n = sprintf(httpReq, "GET /config.cgi?mres HTTP/1.0\r\nHost: %s\r\nUser-Agent: cpcxfer\r\n\r\n", ip);
		send(sd, httpReq, n, 0);
		
		while (n > 0)	// ignore response... a 200 OK check would be a good idea....
			n = recv(sd, httpReq, sizeof(httpReq),0 );
	
#ifdef __WIN32__
		closesocket(sd);
#else
		close(sd);
#endif	
		printf("M4 Reset request sent.\r\n");
	}
}
void resetCPC(char *ip)
{	char httpReq[128];
	int sd;
	volatile int n;
	
	sd = httpConnect(ip);
	
	if ( sd > 0 )
	{	
		
		n = sprintf(httpReq, "GET /config.cgi?cres HTTP/1.0\r\nHost: %s\r\nUser-Agent: cpcxfer\r\n\r\n", ip);
		send(sd, httpReq, n, 0);
		
		while (n > 0)	// ignore response... a 200 OK check would be a good idea....
			n = recv(sd, httpReq, sizeof(httpReq),0 );
	
#ifdef __WIN32__
		closesocket(sd);
#else
		close(sd);
#endif	
		printf("M4 Reset request sent.\r\n");
	}
}
void run(char *ip, char *path)
{	char httpReq[128];
	int sd;
	volatile int n;
	
	sd = httpConnect(ip);
	
	if ( sd > 0 )
	{	
		n = sprintf(httpReq, "GET /config.cgi?run=%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: cpcxfer\r\n\r\n", path, ip);
		send(sd, httpReq, n, 0);
		
		while (n > 0)	// ignore response... a 200 OK check would be a good idea....
			n = recv(sd, httpReq, sizeof(httpReq),0 );
	
#ifdef __WIN32__
		closesocket(sd);
#else
		close(sd);
#endif	
		printf("Running %s\r\n", path);
	}
}

#ifndef __WIN32__
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
#endif


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

#ifndef __WIN32__
	if (!is_regular_file(filename))
	{	printf("file %s is not a regular file\n", filename);
		fclose(fd);
		return;
	}
#endif
	
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

void uploadRom(char *filename, char *ip, int slot, char *slotname)
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
	
	buf = malloc(size);
	if ( buf == NULL )
	{
		printf("Not enough memory!\n");
		fclose(fd);
		return;
	}
	
	fread(buf, size, 1, fd);
	
	fclose(fd);
	

	ret = httpConnect(ip);
	if ( ret >= 0 )
	{	sd = ret;	// connect socket
	
		if ( httpSendRom(sd, "rom.bin", buf, size, slot, "/roms.shtml", ip, slotname) >= 0 )
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

	if ( argc < 2 )
	{	
		dispInfo(argv[0]);
		exit(0);
	}

#ifdef __WIN32__
	WSAStartup(MAKEWORD(2,2),&WsaDat);		
#endif	

	if ( !strnicmp(argv[1], "-r", 2) )	// reboot M4
	{
		reset(argv[2]);
	}
	else	if ( !strnicmp(argv[1], "-s", 2) )	// reset cpc
	{
		resetCPC(argv[2]);
	}
	else	if ( !strnicmp(argv[1], "-x", 2) && (argc>=3) )	// execute
	{
		run(argv[2], argv[3]);
	}
	else if ( !strnicmp(argv[1], "-f", 2) && (argc>=4) )	// upload (flash) rom
	{	
		uploadRom(argv[3], argv[2], atoi(argv[4]), argv[5] );
	}
	else if ( !strnicmp(argv[1], "-u", 2) && (argc>=4) )	// upload 
	{
		if ( argc < 5 )
		{
			dispInfo(argv[0]);
			exit(0);
		}
		
		if ( argc>5 )
			opt = atoi(argv[5]);
		
		upload(argv[3], argv[4], argv[2], opt);
	}
	else if ( !strnicmp(argv[1], "-y", 2) && (argc>=3) )
	{
        // Prepare the file manipulation variables
        char fullpath[256];
		char path[] = "/tmp"; // TODO Find a way to use /tmp or something like that
		char * filename = argv[3];
		char * ip = argv[2];
		int p = pathPos(filename, strlen(filename));	// strip any PC path from filename
		sprintf(fullpath, "%s/%s", path, &filename[p]);
                
		upload(filename, path, ip, 0);  // Upload the file on CPC
		run(ip, fullpath);              // Execute the file on CPC

	}
	else if ( !strnicmp(argv[1], "-d", 2) && (argc>=4) )	// download
	{	
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

