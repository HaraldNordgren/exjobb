
#define SIMD 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

# define fseek64   _fseeki64
# define ftell64   _ftelli64

int main(int argc, char *argv[])
{

  int width_org,  width_org_c;
  int height_org, height_org_c;
  int size_org,   size_org_c;

  unsigned char *Y1, *U1, *V1;

  FILE * infile;
  FILE * outfile;

  int i;
  int Frames = 10000;
  int TotalFrames = 0;
  int StartFrame  = 0;
  int FrameSkip   = 0;

  if (argc < 5)
  {
    printf("\nIncorrect number of arguments!!!\n\n");
    printf("Syntax: \n");
    printf("%s <width> <height> <input> <output> [frames_to_process] [skip_frames] [start_frame]\n\n", argv[0]);
    printf("Examples: \n");
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv \n", argv[0]);
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv 100 0 0\n", argv[0]);
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv 100 0 10\n\n", argv[0]);
    return -1;
  }

  width_org  = atoi  (argv[1]);
  height_org = atoi  (argv[2]);
  infile     = fopen (argv[3], "rb");
  outfile    = fopen (argv[4], "wb");

  for (i = 5; i < argc; i++)
  {
    switch(i)
    {
    case 5:
      Frames = atoi(argv[5]);
      break;
    case 6:
      FrameSkip = atoi(argv[6]);
      break;
    case 7:
      StartFrame = atoi(argv[7]);
      break;
    default:
      printf("Too many input arguments");
      break;
    }
  }

  if (width_org < 4 || height_org < 4 || infile == NULL || outfile == NULL ||
      Frames < 1 || FrameSkip < 0 || StartFrame < 0)
  {
    printf("input error\n");
    return -1;
  }

  fseek64 (infile, 0, SEEK_END);

  TotalFrames = (int) ((ftell64(infile) * 2) / (width_org * height_org * 3));

  if (StartFrame >= TotalFrames)
  {
    printf("StartFrame value too large (%d)\n", StartFrame);
    return -1;
  }

  TotalFrames -= StartFrame;
  TotalFrames  = ((Frames - 1) * (1 + FrameSkip) + 1) > TotalFrames ? TotalFrames : ((Frames - 1) * (1 + FrameSkip) + 1);

  fseek64(infile, 0, SEEK_SET);

  size_org         = height_org   * width_org;

  width_org_c      = width_org  >> 1;
  height_org_c     = height_org >> 1;
  size_org_c       = size_org   >> 2;

  //printf("\n=============================================================\n");
  //printf("\n Input  = %s", argv[3]);
  //printf("\n Output = %s\n", argv[4]);
  //printf("\n Rescaling input from (%d,%d) to (%d,%d) resolution\n", width_org, height_org, width_scaled, height_scaled);
  //printf("\n=============================================================\n\n");


  // Memory allocation for Source YUV

  Y1 = (unsigned char*)malloc((height_org) * (width_org) * sizeof(unsigned char));
  U1 = (unsigned char*)malloc((height_org_c) * (width_org_c) * sizeof(unsigned char));
  V1 = (unsigned char*)malloc((height_org_c) * (width_org_c) * sizeof(unsigned char));

  if (StartFrame != 0)
  {
    fseek64(infile, (size_org * StartFrame * 3) >> 1, SEEK_SET);
  }

  for (i = 0; i < TotalFrames; i++)
  {
    //fprintf(stdout,"Rescaling frame %d\r", i);
    //fflush(stdout);

    // read Y
    fread(Y1, sizeof(unsigned char), size_org, infile);
   

    // read U
    fread(U1, sizeof(unsigned char), size_org_c, infile);

    // read V
    fread(V1, sizeof(unsigned char), size_org_c, infile);

    // write scaled YUV
    fwrite(Y1, sizeof(unsigned char), size_org,   outfile);
    fwrite(U1, sizeof(unsigned char), size_org_c, outfile);
    fwrite(V1, sizeof(unsigned char), size_org_c, outfile);

    if (FrameSkip != 0)
    {
      if ((i + FrameSkip) < TotalFrames)
      {
        fseek64(infile, (size_org * FrameSkip * 3) / 2, SEEK_CUR);
        i += FrameSkip;
      }
      else
      {
        break;
      }
    }
  }

  //printf("\nEnd of rescaling process.\n");

  free(Y1);
  free(U1);
  free(V1);

 

  fclose(infile);
  fclose(outfile);

  return 0;
}