/*
	Duke 2016

	Some CPC specific functions.	
*/

// decrypt "protected" basic files, reversed from CPC
int decrypt_basp(unsigned char *data, int size)
{
	int idx1, idx2, i, j;
	unsigned char xorstream1[13] = { 0xE2, 0x9D, 0xDB, 0X1A, 0x42, 0x29, 0x39, 0xC6, 0xB3, 0xC6, 0x90, 0x45, 0x8A };
	unsigned char xorstream2[11] = { 0x49, 0xB1, 0x36, 0xF0, 0x2E, 0x1E, 0x06, 0x2A, 0x28, 0x19, 0xEA };
	idx1 = 0;
	idx2 = 0;
	i = 0;
	j = 0;
	while (j < size)
	{
		if ( i == 0x80)
		{
			idx1 = 0;
			idx2 = 0;
			i = 0;
		}
		
		data[j] ^= xorstream1[idx1++];
		data[j] ^= xorstream2[idx2++];
		if (idx1 == 13)
			idx1 = 0;
			
		if (idx2 == 11)
			idx2 = 0;
		
		i++;
		j++;
		
	}	
	
	return 0;
}

// simple checksum used by amsdos headers
unsigned short checksum16(unsigned char *data, int size)
{
	int i;
	
	unsigned short checksum = 0;
	
	for (i=0; i<size; i++)
		checksum+=data[i];
	
	return checksum;
}