// converts a file to C++ code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc,const char **argv)
{
	int ret = 1; // failed..

	if ( argc == 2 )
	{
		char scratch[512];
		strcpy(scratch,argv[1]);
		FILE *fph = fopen(scratch,"rb");
		if ( fph )
		{
			fseek(fph,0L,SEEK_END);
			size_t len = ftell(fph);
			if ( len )
			{
				unsigned char *data = new unsigned char[len];
				fseek(fph,0L,SEEK_SET);
				size_t l = fread(data,len,1,fph);
				if ( l )
				{
					// ok..read to convert into code..
					char *dot = strchr(scratch,'.');
					if ( dot )
					{
						*dot = 0;
						char fname1[512];
						char fname2[512];
						sprintf(fname1,"%s.h", scratch );
						sprintf(fname2,"%s.cpp", scratch );
						char uscratch[512];
						strcpy(uscratch,scratch);
						strupr(uscratch);

						FILE *fp1 = fopen(fname1,"wb");
						FILE *fp2 = fopen(fname2,"wb");
						if ( fp1 && fp2 )
						{
							fprintf(fp1,"#ifndef %s_H\r\n", uscratch );
							fprintf(fp1,"#define %s_H\r\n", uscratch );
							fprintf(fp1,"\r\n");
							fprintf(fp1,"extern unsigned char g_%s[%d];\r\n", scratch, len );
							fprintf(fp1,"\r\n");
							fprintf(fp1,"#endif\r\n");
							fclose(fp1);

							fprintf(fp2,"#include \"%s\"\r\n", fname1 );
							fprintf(fp2,"\r\n");

							fprintf(fp2,"unsigned char g_%s[%d] = {\r\n", scratch, len );
				  		fprintf(fp2,"  ");
						  for (unsigned int i=0; i<len; i++)
						  {
						  	fprintf(fp2,"0x%02X", data[i] );
								if ( i < (len-1) )
									fprintf(fp2,",");
						  	if ( ((i+1)&31) == 0 )
						  	{
						  		fprintf(fp2,"\r\n");
						  		fprintf(fp2,"  ");
						  	}
						  }
							fprintf(fp2,"\r\n");

							fprintf(fp2,"};\r\n");

							fclose(fp2);
							printf("Created header file '%s'\r\n", fname1 );
							printf("Created source file '%s'\r\n", fname2 );
							ret = 0;
						}
						else
						{
							if ( fp1 )
								fclose(fp1);
							else
								printf("Failed to open file %s for write access.\r\n", fname1 );
							if ( fp2 )
								fclose(fp2);
							else
								printf("Failed to open file %s for write access.\r\n", fname2 );
						}
					}
					else
					{
						printf("Expected filename extension on input file.\r\n");
					}
				}
				else
				{
					printf("Failed to read entire file.\r\n");
				}
			}
			else
			{
				printf("Nothing to convert, empty file.\r\n");
			}
			fclose(fph);
		}
		else
		{
			printf("Failed to open input file '%s' for read access.\r\n", argv[1]);
		}
	}
	else
	{
		printf("Usage: FileCode <fileName>\r\n");
	}
	return ret;
}
