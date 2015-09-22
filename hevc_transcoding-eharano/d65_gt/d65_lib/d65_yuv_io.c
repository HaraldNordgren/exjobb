/******************************************************************************
* © Copyright (C) Ericsson AB 2012-2014. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
******************************************************************************/

#include "d65_yuv_io.h"
#include <stdio.h>

#if GT_RESIDUAL_RECONSTRUCT

void InitializeYUV(YUVImage_t *YUVImage_p)
{
  YUVImage_p->UserPictureValue = 0;
  YUVImage_p->PictureType      = 0;

  YUVImage_p->Y_Height_image   = 0;
  YUVImage_p->Y_Width_image    = 0;
  YUVImage_p->C_Height_image   = 0;
  YUVImage_p->C_Width_image    = 0;

  YUVImage_p->Cb = NULL;
  YUVImage_p->Cr = NULL;
  YUVImage_p->Y  = NULL;
}

uint8 ReadYUV(YUVImage_t *YUVImage_p,
              FILE* yuvFile,
              const uint32 frameNumber)
{
  if (yuvFile && !feof(yuvFile))
  {
    uint32 sizeLuma   = YUVImage_p->Y_Height_image * YUVImage_p->Y_Width_image;
    uint32 sizeChroma = sizeLuma >> 2;

    fseek(yuvFile, frameNumber * 3 * (sizeLuma >> 1), SEEK_SET);

    fread(YUVImage_p->Y,  sizeof(unsigned char), sizeLuma,   yuvFile);
    fread(YUVImage_p->Cb, sizeof(unsigned char), sizeChroma, yuvFile);
    fread(YUVImage_p->Cr, sizeof(unsigned char), sizeChroma, yuvFile);

    return 0;
  }
  return 1;
}

uint8 AllocateYUV(const SessionData_t *Sessiondata_p,
                  YUVImage_t *YUVImage_p,
                  const uint32 lumaHeight,
                  const uint32 lumaWidth)
{
  YUVImage_p->Y_Height_image = lumaHeight;
  YUVImage_p->Y_Width_image  = lumaWidth;

  YUVImage_p->C_Height_image = YUVImage_p->Y_Height_image >> 1;
  YUVImage_p->C_Width_image  = YUVImage_p->Y_Width_image  >> 1;

  YUVImage_p->Y  = (uint8*) Sessiondata_p->Malloc_Function_p(YUVImage_p->Y_Width_image * YUVImage_p->Y_Height_image, D65_IMAGE_DATA);
  if(YUVImage_p->Y == NULL)
  {
    return 1;
  }
  YUVImage_p->Cb  = (uint8*) Sessiondata_p->Malloc_Function_p(YUVImage_p->C_Width_image * YUVImage_p->C_Height_image, D65_IMAGE_DATA);
  if(YUVImage_p->Cb == NULL)
  {
    return 1;
  }

  YUVImage_p->Cr  = (uint8*) Sessiondata_p->Malloc_Function_p(YUVImage_p->C_Width_image * YUVImage_p->C_Height_image, D65_IMAGE_DATA);
  if(YUVImage_p->Cr == NULL)
  {
    return 1;
  }

  return 0;
}

void FreeYuv(SessionData_t *Sessiondata_p,YUVImage_t *YUVImage_p)
{
  if(YUVImage_p->Y != NULL)
  {
    Sessiondata_p->Free_Function_p(YUVImage_p->Y, D65_IMAGE_DATA);
    YUVImage_p->Y = NULL;
  }
  if(YUVImage_p->Cb != NULL)
  {
    Sessiondata_p->Free_Function_p(YUVImage_p->Cb, D65_IMAGE_DATA);
    YUVImage_p->Cb = NULL;
  }
  if(YUVImage_p->Cr != NULL)
  {
    Sessiondata_p->Free_Function_p(YUVImage_p->Cr, D65_IMAGE_DATA);
    YUVImage_p->Cr = NULL;
  }

  YUVImage_p->UserPictureValue = 0;
  YUVImage_p->PictureType      = 0;

  YUVImage_p->Y_Height_image   = 0;
  YUVImage_p->Y_Width_image    = 0;
  YUVImage_p->C_Height_image   = 0;
  YUVImage_p->C_Width_image    = 0;
}
#endif