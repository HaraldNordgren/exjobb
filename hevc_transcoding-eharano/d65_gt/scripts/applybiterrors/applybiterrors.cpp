// applybiterrors.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>           /* Definition of atoi          */
#include <stdio.h>

int main(int argc, char* argv[])
{
  if(argc != 6) {
    printf("Usage: %s <infile> <outfile> <errorfile> <errorfileoffset> <infileoffset>\n", argv[0]);
    return 1;
  }
   
  FILE *infile = fopen(argv[1], "rb");
  if(infile == NULL) {
    printf("Couldn't open file: %s\n", argv[1]);
    return 1;
  }

  FILE *outfile = fopen(argv[2], "wb");
  if(outfile == NULL) {
    printf("Couldn't open file: %s\n", argv[2]);
    return 1;
  }

  FILE *errorfile = fopen(argv[3], "rb");
  if(errorfile == NULL) {
    printf("Couldn't open file: %s\n", argv[3]);
    return 1;
  }

  fseek (errorfile, 0, SEEK_END);
  int size = ftell(errorfile);
  fseek (errorfile, 0, SEEK_SET);

  unsigned char *errorbuffer = (unsigned char *)malloc(size);
  fread(errorbuffer, 1, size, errorfile);

  int errorfileoffset = atoi(argv[4]);
  if(errorfileoffset < 0 || errorfileoffset >= size)
  {
    printf("Bad offset for the error file %s\n", errorfileoffset);
  }

  fseek (infile, 0, SEEK_END);
  int size_in = ftell(infile);
  fseek (infile, 0, SEEK_SET);

  int infileoffset = atoi(argv[5]);
  if(infileoffset < 0 || infileoffset >= size_in-2)
  {
    printf("Bad offset for the input file %s\n", infileoffset);
  }


  int pos = errorfileoffset;
  while(true)
  {
    if(pos >= size)
    {
      pos = 0;
    }
    int ch = getc(infile);
    if(ch==EOF)
    {
      break;
    }

    if(pos > errorfileoffset+infileoffset)
    {
      ch ^= errorbuffer[pos];
    }
    pos++;
    putc(ch, outfile);
  }

  fclose(infile);
  fclose(outfile);
  fclose(errorfile);

  return 0;
}

