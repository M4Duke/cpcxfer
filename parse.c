/*
	Duke 2016
	
	Parsing helpers.
*/
#include "string.h"

int pathPos(char *filename, int len)
{
	int i;
	for (i=len; i > 0; i--)
	{
		if ( (filename[i] =='/') || (filename[i] =='\\') )
		{	i++;	
			break;
		}
	}
	return i;
}

int formatfn(char *sfn, char *filename)
{
	int i, n, k, spos;
	
	k = strlen(filename);
	
	
	// skip any path that may be leading the filename
	spos = pathPos(filename, k);
	
	for ( i=spos; i < k; i++ )
	{
		if ( (filename[i] >= 'a') && (filename[i] <= 'z') )
			filename[i] &= 0xDF;	// to uppercase
	}
	
	//strip spaces
	n = 0;
	for ( i=spos; i < k; i++ )
	{
		if ( filename[i] ==  '.' )
			break;
			
		if ( filename[i] != ' ' )
			sfn[n++] = filename[i];
		
		if (n == 8)
			break;
		
	}

	while ( n < 8 )
		sfn[n++] = ' ';	// pad with spaces

	return n;
}

void getExtension(char *ext, char *filename)
{
	int i, k, n;
	k = strlen(filename);
	n = 0;
	for (i=k; i > 0; i--)
	{
		if ( filename[i] == '.' )
		{	i++;
			
			while ( i < k )
			{	ext[n++] = filename[i++];
				if ( n == 3 )
					break;
			}
			
			break; 
		}	
	}
	while (n<3)
		ext[n++] = ' '; // pad with spaces

}

int findString(char *fstr, char *str, int size)
{
	int i,n,k;
	
	
	n = strlen(fstr);
	i = 0;
	
	while ( i<=(size-n) )
	{
		
		if ( !strnicmp(&str[i], fstr, n) )
		{	
			i+=n;
			
			return i;
		}
		
		i++;
	}
	return -1;
}