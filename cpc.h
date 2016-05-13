/*
	Duke 2016
	
*/

#ifndef _CPC_H_
#define _CPC_H_

#pragma pack(1)

typedef struct 
{
	unsigned char user;			// 0
	char filename[8];			// 1
	char extension[3];			// 9
	unsigned char notused[6];	// 12
	unsigned char type;			// 18
	unsigned char notused1[2];	// 19
	unsigned short addr;		// 21
	unsigned char  pad;			// 23
	unsigned short size;		// 24
	unsigned short exec;		// 26
	unsigned char notused2[36];	// 28
	unsigned short size2;		// 64
	unsigned char  pad2;		// 66
	unsigned short checksum;		// 67
	unsigned short notused3[59];	// 69
	
} __attribute__((packed)) _cpchead;

int decrypt_basp(unsigned char *data, int size);

#endif