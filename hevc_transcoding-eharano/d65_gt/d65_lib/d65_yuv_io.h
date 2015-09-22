/******************************************************************************
* © Copyright (C) Ericsson AB 2012-2014. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
******************************************************************************/

#ifndef D65_YUV_IO_H
#define D65_YUV_IO_H

#include "d65_api.h"
#include "d65_internal.h"

#if GT_RESIDUAL_RECONSTRUCT

void InitializeYUV(YUVImage_t *YUVImage_p);

uint8 ReadYUV(YUVImage_t *YUVImage_p,
              FILE* yuvFile,
              const uint32 frameNumber);

uint8 AllocateYUV(const SessionData_t *Sessiondata_p,
                  YUVImage_t *YUVImage_p,
                  const uint32 height,
                  const uint32 width);

void FreeYuv(SessionData_t *Sessiondata_p,YUVImage_t *YUVImage_p);

#endif
#endif