/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for SAO
*
******************************************************************************/
#include "d65_sample_adaptive_offset.h"

/******************************************************************************
* Definition of local variables
******************************************************************************/

static const uint8 edgeIdxTable[5] = { 1, 2, 0, 3, 4};

static const sint8 Sign3Table[512] = 
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 
};

/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        GetBandIdx
*
* Parameters:  Sessiondata_p [in/out] The decoder
*
* Returns:     -
*
* Description: Get band index for a Ctb 
*
******************************************************************************/
void
GetBandIdx(SessionData_t *Sessiondata_p,
              const uint32 rx,
              const uint32 ry,
              const uint32 cIdx)
{
  uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint8  CurrSaoTypeClass =Sessiondata_p->SaoTypeClass[cIdx][ry*PicWidthInCtb+rx];
  uint32 RecPicWidth;
  uint32 log2CtbSize;
  sint32 xC;
  sint32 yC;
  sint32 EndX;
  sint32 EndY;
  sint32 nS;
  uint32 BandIdx;
  sint32 x;
  sint32 y;
  uint8 *RecCtb_p;
  uint32 SaoEdgeBlockWidth;
  uint8 BandTable[32];
  uint8 k;

  memset(BandTable, 0, 32*sizeof(BandTable[0]));
  for(k = 0; k < 4; k++)
  {
    BandTable[(k+CurrSaoTypeClass)&31] = k+1;
  }

  switch (cIdx)
  {
  case 0:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Y + PADDING*RecPicWidth + PADDING;
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
    break;
  case 1:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cb + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  case 2:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cr + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  default:
    break;
  }

  SaoEdgeBlockWidth = PicWidthInCtb << log2CtbSize;
  xC = rx << log2CtbSize;
  yC = ry << log2CtbSize;
  nS = 1 << log2CtbSize;

  EndX = xC + nS;
  EndY = yC + nS;

  for(y = yC; y < EndY; y++)
  {
    for(x = xC; x < EndX; x++)
    {
      BandIdx = BandTable[RecCtb_p[y*RecPicWidth + x] >> 3];
      Sessiondata_p->SaoOffsetToUse[y*SaoEdgeBlockWidth + x] = Sessiondata_p->SaoOffsetVal[cIdx][BandIdx][ry*PicWidthInCtb+rx];
    }
  }

}
/******************************************************************************
*
* Name:        GetSaoEdgeIdx
*
* Parameters:  Sessiondata_p [in/out] The decoder
*
* Returns:     -
*
* Description: Get edge index for a Ctb 
*
******************************************************************************/
void
GetSaoEdgeIdx(SessionData_t *Sessiondata_p,
              const uint32 rx,
              const uint32 ry,
              const uint32 cIdx)
{
  uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint32 CtbRasterAddr = ry*PicWidthInCtb+rx;
  uint8  CurrSaoTypeClass =Sessiondata_p->SaoTypeClass[cIdx][CtbRasterAddr];
  uint32 log2CtbSize;
  uint32 RecPicWidth;
  sint32 xC;
  sint32 yC;
  sint32 StartX;
  sint32 StartY;
  sint32 EndX;
  sint32 EndY;
  uint32 PicWidthModCtbSize;
  uint32 PicHeightModCtbSize;

  uint32 edgeIdx;
  sint32 x;
  sint32 y;
  sint16 CurrPixel;
  sint16 DiffPixel0;
  sint16 DiffPixel1;
  uint8 *RecCtb_p;
  sint16 *Offset_p;
  sint32 nS;
  uint32 SaoEdgeBlockWidth;
  const sint8 *SignLut = Sign3Table + 255;
  sint16 LocalOffset[5];

  LocalOffset[0] = Sessiondata_p->SaoOffsetVal[cIdx][1][CtbRasterAddr];
  LocalOffset[1] = Sessiondata_p->SaoOffsetVal[cIdx][2][CtbRasterAddr];
  LocalOffset[2] = Sessiondata_p->SaoOffsetVal[cIdx][0][CtbRasterAddr];
  LocalOffset[3] = Sessiondata_p->SaoOffsetVal[cIdx][3][CtbRasterAddr];
  LocalOffset[4] = Sessiondata_p->SaoOffsetVal[cIdx][4][CtbRasterAddr];

  switch (cIdx)
  {
  case 0:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Y + PADDING*RecPicWidth + PADDING;
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
    PicWidthModCtbSize = Sessiondata_p->PicWidthModCtbSize;
    PicHeightModCtbSize = Sessiondata_p->PicHeightModCtbSize;
    break;
  case 1:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cb + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    PicWidthModCtbSize = Sessiondata_p->PicWidthModCtbSize >> 1;
    PicHeightModCtbSize = Sessiondata_p->PicHeightModCtbSize >> 1;
    break;
  case 2:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cr + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    PicWidthModCtbSize = Sessiondata_p->PicWidthModCtbSize >> 1;
    PicHeightModCtbSize = Sessiondata_p->PicHeightModCtbSize >> 1;
    break;
  default:
    break;
  }

  SaoEdgeBlockWidth = PicWidthInCtb << log2CtbSize;
  xC = rx << log2CtbSize;
  yC = ry << log2CtbSize;
  nS = 1 << log2CtbSize;

  RecCtb_p += yC*RecPicWidth + xC;
  Offset_p  = Sessiondata_p->SaoOffsetToUse + yC*SaoEdgeBlockWidth + xC;

  StartX = 0;
  StartY = 0;
  EndX = nS;
  EndY = nS;

  switch (CurrSaoTypeClass)
  {
  case 0:
    if(rx == 0)
    {
      StartX++;
    }
    if(rx == Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY - 1)
    {
      EndX = (nS + PicWidthModCtbSize - 1) % nS;
    }
    for(y = StartY; y < EndY; y++)
    {
      DiffPixel0 = SignLut[RecCtb_p[StartX - 1] - RecCtb_p[StartX]];
      for(x = StartX; x < EndX; x++)
      {
        edgeIdx = 2 - DiffPixel0;
        DiffPixel0 = SignLut[RecCtb_p[x] - RecCtb_p[x+1]];
        edgeIdx += DiffPixel0;
        Offset_p[x] = LocalOffset[edgeIdx];
      }
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    break;
  case 1:
    if(ry == 0)
    {
      StartY++;
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    if(ry == Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY - 1)
    {
      EndY = (nS + PicHeightModCtbSize - 1) % nS;
    }
    for(y = StartY; y < EndY; y++)
    {
      for(x = StartX; x < EndX; x++)
      {
        CurrPixel = (sint16)RecCtb_p[x];
        DiffPixel0 = CurrPixel - (sint16)RecCtb_p[x + RecPicWidth];
        DiffPixel1 = CurrPixel - (sint16)RecCtb_p[x - RecPicWidth];
        edgeIdx = 2 + SignLut[DiffPixel0] + SignLut[DiffPixel1];
        Offset_p[x] = LocalOffset[edgeIdx];
      }
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    break;
  case 2:
    if(rx == 0)
    {
      StartX++;
    }
    if(rx == Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY - 1)
    {
      EndX = (nS + PicWidthModCtbSize - 1) % nS;
    }
    if(ry == 0)
    {
      StartY++;
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    if(ry == Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY - 1)
    {
      EndY = (nS + PicHeightModCtbSize - 1) % nS;
    }
    for(y = StartY; y < EndY; y++)
    {
      for(x = StartX; x < EndX; x++)
      {
        CurrPixel = (sint16)RecCtb_p[x];
        DiffPixel0 = CurrPixel - (sint16)RecCtb_p[x + 1 + RecPicWidth];
        DiffPixel1 = CurrPixel - (sint16)RecCtb_p[x - 1 - RecPicWidth];
        edgeIdx = 2 + SignLut[DiffPixel0] + SignLut[DiffPixel1];
        Offset_p[x] = LocalOffset[edgeIdx];
      }
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    break;
  case 3:
    if(rx == 0)
    {
      StartX++;
    }
    if(rx == Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY - 1)
    {
      EndX = (nS + PicWidthModCtbSize - 1) % nS;
    }
    if(ry == 0)
    {
      StartY++;
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    if(ry == Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY - 1)
    {
      EndY = (nS + PicHeightModCtbSize - 1) % nS;
    }
    for(y = StartY; y < EndY; y++)
    {
      for(x = StartX; x < EndX; x++)
      {
        CurrPixel = (sint16)RecCtb_p[x];
        DiffPixel0 = CurrPixel - (sint16)RecCtb_p[x - 1 + RecPicWidth];
        DiffPixel1 = CurrPixel - (sint16)RecCtb_p[x + 1 - RecPicWidth];
        edgeIdx = 2 + SignLut[DiffPixel0] + SignLut[DiffPixel1];
        Offset_p[x] = LocalOffset[edgeIdx];
      }
      RecCtb_p += RecPicWidth;
      Offset_p += SaoEdgeBlockWidth;
    }
    break;
  default:
    break;
  }
}

/******************************************************************************
*
* Name:        BandOffset
*
* Parameters:  Sessiondata_p [in/out] The decoder
*              rx        [in]           x coordinate in CTUs
*              ry        [in]           y coordinate in CTUs
*              log2CtbSize[in]          Log2 Ctb size
*              cIdx       [in]          Color component index;
*
* Returns:     -
*
* Description: Perform Band Offset for a Ctb 
*
******************************************************************************/
void
BandOffset(SessionData_t *Sessiondata_p,
          const uint32 rx,
          const uint32 ry,
          const uint32 cIdx)
{
  uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint8  CurrSaoTypeClass =Sessiondata_p->SaoTypeClass[cIdx][ry*PicWidthInCtb+rx];
  uint32 RecPicWidth;
  uint32 log2CtbSize;
  sint32 xC;
  sint32 yC;
  sint32 EndX;
  sint32 EndY;
  sint32 nS;
  sint32 x;
  sint32 y;
  sint16 SaoOffsetVal;
  uint8 *RecCtb_p;
  uint32 SaoEdgeBlockWidth;

  switch (cIdx)
  {
  case 0:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Y + PADDING*RecPicWidth + PADDING;
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
    break;
  case 1:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cb + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  case 2:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cr + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  default:
    break;
  }

  SaoEdgeBlockWidth = PicWidthInCtb << log2CtbSize;
  xC = rx << log2CtbSize;
  yC = ry << log2CtbSize;
  nS = 1 << log2CtbSize;


  EndX = xC + nS;
  EndY = yC + nS;

  for(y = yC; y < EndY; y++)
  {
    for(x = xC; x < EndX; x++)
    {
      SaoOffsetVal = Sessiondata_p->SaoOffsetToUse[y*SaoEdgeBlockWidth+x];
      RecCtb_p[y*RecPicWidth + x] = (uint8)CLIP3(0, 255, RecCtb_p[y*RecPicWidth + x] + SaoOffsetVal);
    }
  }
}

/******************************************************************************
*
* Name:        EdgeOffset
*
* Parameters:  Sessiondata_p [in/out] The decoder
*              rx        [in]           x coordinate in CTUs
*              ry        [in]           y coordinate in CTUs
*              log2CtbSize[in]          Log2 Ctb size
*              cIdx       [in]          Color component index;
*
* Returns:     -
*
* Description: Perform Edge Offset for a Ctb 
*
******************************************************************************/
void
EdgeOffset(SessionData_t *Sessiondata_p,
          const uint32 rx,
          const uint32 ry,
          const uint32 cIdx)
{
  uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint8  CurrSaoTypeClass =Sessiondata_p->SaoTypeClass[cIdx][ry*PicWidthInCtb+rx];
  uint32 RecPicWidth;
  uint32 log2CtbSize;
  sint32 xC;
  sint32 yC;
  sint32 nS;
  sint32 x;
  sint32 y;
  uint8 *RecCtb_p;
  uint32 SaoEdgeBlockWidth;
  sint16 *SaoOffset_p;

  switch (cIdx)
  {
  case 0:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Y + PADDING*RecPicWidth + PADDING;
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
    break;
  case 1:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cb + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  case 2:
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    RecCtb_p = Sessiondata_p->CurrentRecImage->Image.Cr + (PADDING >> 1)*RecPicWidth + (PADDING >> 1);
    log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - 1;
    break;
  default:
    break;
  }

  SaoEdgeBlockWidth = PicWidthInCtb << log2CtbSize;
  xC = rx << log2CtbSize;
  yC = ry << log2CtbSize;
  nS = 1  << log2CtbSize;

  RecCtb_p += yC*RecPicWidth + xC;
  SaoOffset_p = Sessiondata_p->SaoOffsetToUse + yC*SaoEdgeBlockWidth + xC;

  for(y = 0; y < nS; y++)
  {
#if SIMD
    for(x = 0; x < nS; x += 8)
    {
      __m128i Rec_p = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(RecCtb_p + x)),_mm_setzero_si128());
      __m128i Offset_p = _mm_loadu_si128((__m128i*)(SaoOffset_p + x));
      __m128i ModifiedRec_p = _mm_add_epi16(Rec_p, Offset_p);
      _mm_storel_epi64((__m128i*)(RecCtb_p + x),_mm_packus_epi16(ModifiedRec_p,ModifiedRec_p));
    }
    RecCtb_p += RecPicWidth;
    SaoOffset_p += SaoEdgeBlockWidth;
#else
    for(x = 0; x < nS; x++)
    {
      RecCtb_p[x] = CLIP3(0, 255, RecCtb_p[x] + SaoOffset_p[x]);
    }
    RecCtb_p += RecPicWidth;
    SaoOffset_p += SaoEdgeBlockWidth;
#endif
  }
}

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        SaoProcess 
*
******************************************************************************/

void
SaoProcess(SessionData_t *Sessiondata_p)
{
  const uint8 SPSSaoOnFlag = Sessiondata_p->CurrentSequenceParameterSet->SampleAdaptiveOffsetEnabledFlag;
  const uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  const uint32 PicSizeInCtb = (Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY) * PicWidthInCtb;
  uint32 Log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  sint32 CtuCoordinateY;
  sint32 CtuCoordinateX;
  sint32 CtuEndX;
  sint32 CtuEndY;
  uint8 CurrSaoTypeIdx;
  uint32 PicWidth = Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  uint32 PicHeight = Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;
  uint8 i;

  if(SPSSaoOnFlag)
  {
    CtuEndY  = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
    CtuEndX  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;

    if(Sessiondata_p->CurrentSliceSaoLumaFlag == 1)
    {
      memset(Sessiondata_p->SaoOffsetToUse, 0, PicSizeInCtb*(1 << Log2CtbSize)*(1 << Log2CtbSize)*sizeof(sint16));

      for(CtuCoordinateY = 0 ; CtuCoordinateY < CtuEndY ; CtuCoordinateY++)
      {
        for(CtuCoordinateX = 0 ; CtuCoordinateX < CtuEndX ; CtuCoordinateX++)
        {
          CurrSaoTypeIdx = Sessiondata_p->SaoTypeIdx[0][CtuCoordinateY*PicWidthInCtb+CtuCoordinateX];
          switch (CurrSaoTypeIdx)
          {
          case 0:
            break;
          case 1:
            GetBandIdx(Sessiondata_p, CtuCoordinateX, CtuCoordinateY, 0);
            break;
          case 2:
            GetSaoEdgeIdx(Sessiondata_p,  CtuCoordinateX, CtuCoordinateY, 0);
            break;
          default:
            break;
          }
        }
      }

      for(CtuCoordinateY = 0 ; CtuCoordinateY < CtuEndY ; CtuCoordinateY++)
      {
        for(CtuCoordinateX = 0 ; CtuCoordinateX < CtuEndX ; CtuCoordinateX++)
        {
          CurrSaoTypeIdx = Sessiondata_p->SaoTypeIdx[0][CtuCoordinateY*PicWidthInCtb+CtuCoordinateX];
          switch (CurrSaoTypeIdx)
          {
          case 0:
            break;
          case 1:
            BandOffset(Sessiondata_p, CtuCoordinateX, CtuCoordinateY, 0);
            break;
          case 2:
            EdgeOffset(Sessiondata_p, CtuCoordinateX, CtuCoordinateY, 0);
            break;
          default:
            break;
          }
        }
      }
    }

    if(Sessiondata_p->CurrentSliceSaoChromaFlag == 1)
    {
      Log2CtbSize -= 1;
      for(i = 1; i < 3; i++)
      {
        memset(Sessiondata_p->SaoOffsetToUse, 0, PicSizeInCtb*(1 << Log2CtbSize)*(1 << Log2CtbSize)*sizeof(sint16));

        for(CtuCoordinateY = 0 ; CtuCoordinateY < CtuEndY ; CtuCoordinateY++)
        {
          for(CtuCoordinateX = 0 ; CtuCoordinateX < CtuEndX ; CtuCoordinateX++)
          {
            CurrSaoTypeIdx = Sessiondata_p->SaoTypeIdx[1][CtuCoordinateY*PicWidthInCtb+CtuCoordinateX];
            switch (CurrSaoTypeIdx)
            {
            case 0:
              break;
            case 1:
              GetBandIdx(Sessiondata_p, CtuCoordinateX, CtuCoordinateY,i);
              break;
            case 2:
              GetSaoEdgeIdx(Sessiondata_p,  CtuCoordinateX, CtuCoordinateY, i);
              break;
            default:
              break;
            }
          }
        }

        for(CtuCoordinateY = 0 ; CtuCoordinateY < CtuEndY ; CtuCoordinateY++)
        {
          for(CtuCoordinateX = 0 ; CtuCoordinateX < CtuEndX ; CtuCoordinateX++)
          {
            CurrSaoTypeIdx = Sessiondata_p->SaoTypeIdx[1][CtuCoordinateY*PicWidthInCtb+CtuCoordinateX];
            switch (CurrSaoTypeIdx)
            {
            case 0:
              break;
            case 1:
              BandOffset(Sessiondata_p, CtuCoordinateX, CtuCoordinateY,i);
              break;
            case 2:
              EdgeOffset(Sessiondata_p, CtuCoordinateX, CtuCoordinateY,i);
              break;
            default:
              break;
            }
          }
        }
      }
    }
  }
}