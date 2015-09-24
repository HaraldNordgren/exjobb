/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for intra prediction
*
******************************************************************************/
#include "d65_intra_prediction.h"


const static sint32 IntraPredAngleTable[35] = {0, 0, 32, 26, 21, 17, 13, 9, 5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32, -26, -21, -17, -13, -9, -5, -2, 0, 2, 5, 9, 13, 17, 21, 26, 32};
const static sint32 InvAngleTable[15] = {-4096, -1638, -910, -630, -482, -390, -315, -256, -315, -390, -482, -630, -910, -1638, -4096};

/******************************************************************************
*
* Name:        FilterBorderPixelsLuma
*
* Parameters:  
*              AboveRow          [in/out]  Current TU's above neibhouring reference pixels
*              LeftColumn        [in/out]  Current TU's left neibhouring reference pixels
*              IntraPredModeLuma [in]      Intra prediction direction
*              Log2TrafoSize     [in]      Log2 current TU size in pixels 
*
* Returns:     void
*
* Description: Generate filtered above and left neighouring reference pixels for 
*              current TU
*
******************************************************************************/
void
FilterBorderPixelsLuma(uint8 *AboveRow, uint8 *LeftColumn, const uint32 IntraPredModeLuma, const uint32 Log2TrafoSize, const uint8 StrongIntraSmoothing)
{
  const static uint8 IntraHorVerDistThresTable[3] = {7, 1, 0};
  const uint32 TrafoSize = 1 << Log2TrafoSize;
  uint8  IntraHorVerDistThres;
  sint32 MinDistVerHor;
  uint8  FilterFlag;
  uint8  BiIntFlag;
  uint8 FilteredAboveRow[65];
  uint8 FilteredLeftColumn[65];
  sint32 y;
  sint32 x;

  if(IntraPredModeLuma == INTRA_DC || Log2TrafoSize == 2)
  {
    FilterFlag = 0;
  }
  else
  {
    y = D65_ABS((sint32)IntraPredModeLuma - 26);
    x = D65_ABS((sint32)IntraPredModeLuma - 10);
    MinDistVerHor = D65_MIN(y, x);
    IntraHorVerDistThres = IntraHorVerDistThresTable[Log2TrafoSize - 3];
    if(MinDistVerHor > IntraHorVerDistThres)
    {
      FilterFlag = 1;
    }
    else
    {
      FilterFlag = 0;
    }
  }

  if(FilterFlag == 1)
  {
    if(StrongIntraSmoothing && (Log2TrafoSize == 5)  
      && (D65_ABS(AboveRow[0]   + AboveRow[2*TrafoSize]   - 2*AboveRow[TrafoSize]) < 8) 
      && (D65_ABS(LeftColumn[0] + LeftColumn[2*TrafoSize] - 2*LeftColumn[TrafoSize]) < 8))
    {
      BiIntFlag = 1;
      FilteredLeftColumn[64] = LeftColumn[64];
      FilteredAboveRow[64]   = AboveRow[64];
      for(y = 62; y >= 0; y--)
      {
        FilteredLeftColumn[y + 1] = (uint8)(((63-y)*LeftColumn[0] + (y + 1)*LeftColumn[64] + 32) >> 6);
      }
      FilteredLeftColumn[0] = LeftColumn[0];
      FilteredAboveRow[0]   = AboveRow[0];
      for(x = 0; x < 63; x++)
      {
        FilteredAboveRow[x+1] = (uint8)(((63-x)*AboveRow[0] + (x + 1)*AboveRow[64] + 32) >> 6); 
      }
    }
    else
    {
      BiIntFlag = 0;
      FilteredLeftColumn[2*TrafoSize] = LeftColumn[2*TrafoSize];
      FilteredAboveRow[2*TrafoSize]   = AboveRow[2*TrafoSize];
      for(y = 2*TrafoSize - 2; y >= 0; y--)
      {
        FilteredLeftColumn[y + 1] = (LeftColumn[y + 2] + 2*LeftColumn[y + 1] + LeftColumn[y] + 2) >> 2;
      }
      FilteredLeftColumn[0] = (LeftColumn[1] + 2*LeftColumn[0] + AboveRow[1] + 2) >> 2;
      for(x = 0; x <= (sint32)(2*TrafoSize - 2); x++)
      {
        FilteredAboveRow[x + 1]   = (AboveRow[x] + 2*AboveRow[x + 1] + AboveRow[x + 2] + 2) >> 2;
      }
      FilteredAboveRow[0] = FilteredLeftColumn[0];
    }
    memcpy(LeftColumn, FilteredLeftColumn,(2*TrafoSize+1)*sizeof(uint8));
    memcpy(AboveRow,   FilteredAboveRow,  (2*TrafoSize+1)*sizeof(uint8));
  }
}

void
IntraPredictionLuma(SessionData_t *Sessiondata_p, const uint32 x0, const uint32 y0, const uint32 Log2TrafoSize)
{

  const uint32 Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const sint32 MinPUSizeInPixels  = 1 << Log2PUSizeInPixels;
  const uint32 PicWidthInPUs      = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 TrafoSize          = 1 << Log2TrafoSize;
  const sint32 PicWidth    = Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  const sint32 PicHeight   = Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;
  const sint32 RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
  uint8 *RecPic_p      = Sessiondata_p->CurrentRecImage->Image.Y + PADDING*RecPicWidth + PADDING;
  
  sint32 xCurrInPUAddr = x0 >> Log2PUSizeInPixels;
  sint32 yCurrInPUAddr = y0 >> Log2PUSizeInPixels;
  uint32 IntraPredMode = Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs+xCurrInPUAddr].IntraPredMode;
  sint32 xNeibor;
  sint32 yNeibor;
  sint32 xNeiborInPUAddr;
  sint32 yNeiborInPUAddr;

  uint8  BottomLeftFound = 0;
  uint8  NeiborAvailable = 0;

  uint8 AboveRow[65];
  uint8 LeftColumn[65];
  uint8 RefSample[97];

  sint32 y;
  sint32 x;
  sint32 yy;

  //Get Bottom Left Pixel's Value
  xNeibor = x0 - 1;
  if(xNeibor >= 0 && xNeibor < PicWidth)
  {
    xNeiborInPUAddr = xNeibor >> Log2PUSizeInPixels;
    for(y = (sint32)(2*TrafoSize - 1); y >= -1; y -= MinPUSizeInPixels)
    {
      yNeibor = y0 + y;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs+xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          LeftColumn[2*TrafoSize] = RecPic_p[yNeibor*RecPicWidth+xNeibor];
          BottomLeftFound = 1;
          break;
        }
      }
    }
  }
  if(BottomLeftFound == 0)
  {
    yNeibor = y0 - 1;
    yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
    if(yNeibor >= 0 && yNeibor < PicHeight)
    {
      for(x = 0; x <= (sint32)(2*TrafoSize - 1); x += MinPUSizeInPixels)
      {
        xNeibor = x0 + x;
        if(xNeibor >= 0 && xNeibor < PicWidth)
        {
          xNeiborInPUAddr = xNeibor >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs+xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            LeftColumn[2*TrafoSize] = RecPic_p[yNeibor*RecPicWidth+xNeibor];
            BottomLeftFound = 1;
            break;
          }
        }
      }
    }
  }
  //Get Left Column
  if(BottomLeftFound)
  {
    xNeibor = x0 - 1;
    if(xNeibor >= 0 && xNeibor < PicWidth)
    {
      xNeiborInPUAddr = xNeibor >> Log2PUSizeInPixels;
      
      y = 2*TrafoSize - MinPUSizeInPixels;
      yNeibor = y0 + y;
      NeiborAvailable = 0;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          NeiborAvailable = 1;
          for(yy = y; yy < y + MinPUSizeInPixels - 1; yy++)
          {
            LeftColumn[yy+1] = RecPic_p[(y0+yy)*RecPicWidth + xNeibor];
          }
        }
      }
      if(NeiborAvailable == 0)
      {
        memset(LeftColumn+y+1,LeftColumn[2*TrafoSize], (MinPUSizeInPixels-1)*sizeof(uint8));
      }

      for(y = (sint32)(2*TrafoSize - 2*MinPUSizeInPixels); y >= 0; y -= MinPUSizeInPixels) 
      {
        NeiborAvailable = 0;
        yNeibor = y0 + y;
        if(yNeibor >= 0 && yNeibor < PicHeight)
        {
          yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            NeiborAvailable = 1;
            for(yy = y; yy < y + MinPUSizeInPixels; yy++)
            {
              LeftColumn[yy+1] = RecPic_p[(y0+yy)*RecPicWidth + xNeibor];
            }
          }
        }
        if(NeiborAvailable == 0)
        {
          memset(LeftColumn+y+1,LeftColumn[y+1+MinPUSizeInPixels],MinPUSizeInPixels*sizeof(uint8));
        }
      }

      yNeibor = y0 - 1;
      NeiborAvailable = 0;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          NeiborAvailable = 1;
          LeftColumn[0] = RecPic_p[yNeibor*RecPicWidth + xNeibor];
        }
      }
      if(NeiborAvailable == 0)
      {
        LeftColumn[0] = LeftColumn[1];
      }
    }
    else
    {
      memset(LeftColumn, LeftColumn[2*TrafoSize], (2*TrafoSize)*sizeof(uint8));
    }
    //Get Above Row
    AboveRow[0] = LeftColumn[0];
    yNeibor = y0 - 1;
    if(yNeibor >= 0 && yNeibor < PicHeight)
    {
      yNeiborInPUAddr = yNeibor >> Log2PUSizeInPixels;
      for(x = 0; x <= (sint32)(2*TrafoSize - 1); x += MinPUSizeInPixels)
      {
        NeiborAvailable = 0;
        xNeibor = x0 + x;
        if(xNeibor >= 0 &&  xNeibor < PicWidth)
        {
          xNeiborInPUAddr = xNeibor >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            NeiborAvailable = 1;
            memcpy(AboveRow+x+1,RecPic_p+yNeibor*RecPicWidth+x0+x,MinPUSizeInPixels*sizeof(uint8));
          }
        }
        if(NeiborAvailable == 0)
        {
          memset(AboveRow+x+1,AboveRow[x],MinPUSizeInPixels*sizeof(uint8));
        }
      }
    }
    else
    {
      memset(AboveRow+1,AboveRow[0],(2*TrafoSize)*sizeof(uint8));
    }
  }
  else
  {
    memset(AboveRow,128,(2*TrafoSize+1)*sizeof(uint8));
    memset(LeftColumn,128,(2*TrafoSize+1)*sizeof(uint8));
  }

  //Filter Borders
  FilterBorderPixelsLuma(AboveRow, LeftColumn, IntraPredMode, Log2TrafoSize, Sessiondata_p->CurrentSequenceParameterSet->StrongIntraSmoothingFlag);

  //DO Intra Prediction
  if(IntraPredMode == INTRA_PLANAR)
  {
    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      for(x = 0; x < (sint32)TrafoSize; x++)
      {
        sint32 PredSum = (TrafoSize - 1 - x)*LeftColumn[y + 1];
        PredSum += (x + 1)*AboveRow[TrafoSize + 1];
        PredSum += (TrafoSize - 1 - y)*AboveRow[x + 1];
        PredSum += (y + 1)*LeftColumn[TrafoSize + 1];
        PredSum += TrafoSize;
        PredSum = PredSum >> (Log2TrafoSize + 1);
        RecPic_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(PredSum);
      }
    }
  }
  else if(IntraPredMode == INTRA_DC)
  {
    sint32 dcVal = 0;

    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      dcVal += AboveRow[y + 1] + LeftColumn[y + 1];
    }
    dcVal += TrafoSize;
    dcVal = dcVal >> (Log2TrafoSize + 1);
    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      for(x = 0; x < (sint32)TrafoSize; x++)
      {
        RecPic_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(dcVal);
      }
    }
    if(TrafoSize < 32)
    {
      RecPic_p[y0*RecPicWidth+x0] = (uint8)((LeftColumn[1] + 2*dcVal + AboveRow[1] + 2) >> 2);
      for(y = 1; y < (sint32)TrafoSize; y++)
      {
        RecPic_p[y0*RecPicWidth + x0 + y]   = (uint8)((AboveRow[y + 1] + 3*dcVal + 2) >> 2);
        RecPic_p[(y0 + y)*RecPicWidth + x0] = (uint8)((LeftColumn[y + 1] + 3*dcVal + 2) >> 2);
      }
    }
  }
  else
  {
    sint32 IntraPredAngle = IntraPredAngleTable[IntraPredMode];
    sint32 RefSampleExt = (IntraPredAngle << Log2TrafoSize) >> 5;
    sint32 iIdx;
    sint32 iFact;

    if(IntraPredMode >= 18)
    {
      memcpy(RefSample+TrafoSize,AboveRow,(2*TrafoSize+1)*sizeof(uint8));
      if(RefSampleExt < -1)
      {
        sint32 InvAngle = InvAngleTable[IntraPredMode - 11];
        for(x = RefSampleExt; x <= -1; x++)
        {
          RefSample[TrafoSize+x] = LeftColumn[(x*InvAngle+128) >> 8];
        }
      }
      for(y = 0; y < (sint32)TrafoSize; y++)
      {
        iIdx = ((y + 1)*IntraPredAngle) >> 5;
        iFact = ((y + 1)*IntraPredAngle) & 31;
        if(iFact != 0)
        {
          for(x = 0; x < (sint32)TrafoSize; x++)
          {
            RecPic_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSample[TrafoSize+x+iIdx+1] + iFact*RefSample[TrafoSize+x+iIdx+2] + 16)>>5);
          }
        }
        else
        {
          for(x = 0; x < (sint32)TrafoSize; x++)
          {
            RecPic_p[(y0+y)*RecPicWidth+x0+x] = RefSample[TrafoSize+x+iIdx+1];
          }
        }
      }
      if(IntraPredMode == 26 && TrafoSize < 32)
      {
        for(y = 0; y < (sint32)TrafoSize; y++)
        {
          sint32 TmpSample = AboveRow[1] + ((LeftColumn[y+1] - LeftColumn[0]) >> 1);
          RecPic_p[(y0+y)*RecPicWidth+x0] = (uint8)CLIP3(0, 255, TmpSample);
        }
      }
    }
    else
    {
      memcpy(RefSample+TrafoSize, LeftColumn, (2*TrafoSize+1)*sizeof(uint8));
      if(RefSampleExt < -1)
      {
        sint32 InvAngle = InvAngleTable[IntraPredMode - 11];

        for(x = RefSampleExt; x <= -1; x++)
        {
          RefSample[TrafoSize+x] = AboveRow[(x*InvAngle+128) >> 8];
        }
      }
      for(y = 0; y < (sint32)TrafoSize; y++)
      {
        for(x = 0; x < (sint32)TrafoSize; x++)
        {
          iIdx = ((x + 1)*IntraPredAngle) >> 5;
          iFact = ((x + 1)*IntraPredAngle) & 31;
          if(iFact != 0)
          {
            RecPic_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSample[TrafoSize+y+iIdx+1] + iFact*RefSample[TrafoSize+y+iIdx+2] + 16)>>5);
          }
          else
          {
            RecPic_p[(y0+y)*RecPicWidth+x0+x] = RefSample[TrafoSize+y+iIdx+1];
          }
        }
      }
      if(IntraPredMode == 10 && TrafoSize < 32)
      {
        for(x = 0; x < (sint32)TrafoSize; x++)
        {
          sint32 TmpSample = LeftColumn[1] + ((AboveRow[x+1] - AboveRow[0]) >> 1);
          RecPic_p[y0*RecPicWidth+x0+x] = (uint8)CLIP3(0, 255, TmpSample);
        }
      }
    }
  }
}

/******************************************************************************
*
* Name:        IntraPredictionChroma
*
******************************************************************************/
void
IntraPredictionChroma(SessionData_t *Sessiondata_p, const uint32 x0, const uint32 y0, const uint32 Log2TrafoSize)
{
  const uint32 Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const sint32 MinPUCSizeInPixels = 1 << (Log2PUSizeInPixels - 1);
  const uint32 PicWidthInPUs      = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 TrafoSize          = 1 << Log2TrafoSize;
  const sint32 PicWidth    = Sessiondata_p->CurrentSequenceParameterSet->PictureWidth >> 1;
  const sint32 PicHeight   = Sessiondata_p->CurrentSequenceParameterSet->PictureHeight >> 1;
  const sint32 RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
  uint8 *RecPicCb_p      = Sessiondata_p->CurrentRecImage->Image.Cb + (PADDING>>1)*RecPicWidth + (PADDING>>1);
  uint8 *RecPicCr_p      = Sessiondata_p->CurrentRecImage->Image.Cr + (PADDING>>1)*RecPicWidth + (PADDING>>1);

  sint32 xCurrInPUAddr = (x0 << 1) >> Log2PUSizeInPixels;
  sint32 yCurrInPUAddr = (y0 << 1) >> Log2PUSizeInPixels;
  uint32 IntraPredMode = Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs+xCurrInPUAddr].intraChromaPredMode;
  sint32 xNeibor;
  sint32 yNeibor;
  sint32 xNeiborInPUAddr;
  sint32 yNeiborInPUAddr;

  uint8  BottomLeftFound = 0;
  uint8  NeiborAvailable = 0;

  uint8 AboveRowCb[33]; 
  uint8 LeftColumnCb[33];
  uint8 AboveRowCr[33];
  uint8 LeftColumnCr[33];
  uint8 RefSampleCb[49];
  uint8 RefSampleCr[49];

  sint32 y;
  sint32 x;
  sint32 yy;

  //Get Border Pixels
  xNeibor = x0 - 1;
  if(xNeibor >= 0 && xNeibor < PicWidth)
  {
    xNeiborInPUAddr = (xNeibor << 1) >> Log2PUSizeInPixels;
    for(y = (sint32)(2*TrafoSize - 1); y >= -1; y -= MinPUCSizeInPixels)
    {
      yNeibor = y0 + y;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs+xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs+xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          LeftColumnCb[2*TrafoSize] = RecPicCb_p[yNeibor*RecPicWidth+xNeibor];
          LeftColumnCr[2*TrafoSize] = RecPicCr_p[yNeibor*RecPicWidth+xNeibor];
          BottomLeftFound = 1;
          break;
        }
      }
    }
  }
  if(BottomLeftFound == 0)
  {
    yNeibor = y0 - 1;
    if(yNeibor > 0 && yNeibor < PicHeight)
    {
      yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
      for(x = 0; x <= (sint32)(2*TrafoSize - 1); x += MinPUCSizeInPixels)
      {
        xNeibor = x0 + x;
        if(xNeibor >= 0 && xNeibor < PicWidth)
        {
          xNeiborInPUAddr = (xNeibor << 1) >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            LeftColumnCb[2*TrafoSize] = RecPicCb_p[yNeibor*RecPicWidth+xNeibor];
            LeftColumnCr[2*TrafoSize] = RecPicCr_p[yNeibor*RecPicWidth+xNeibor];
            BottomLeftFound = 1;
            break;
          }
        }
      }
    }
  }

  //Get Left Column
  if(BottomLeftFound)
  {
    xNeibor = x0 - 1;
    if(xNeibor >= 0 && xNeibor < PicWidth)
    {
      xNeiborInPUAddr = (xNeibor << 1) >> Log2PUSizeInPixels;

      y = 2*TrafoSize - MinPUCSizeInPixels;
      yNeibor = y0 + y;
      NeiborAvailable = 0;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          NeiborAvailable = 1;
          for(yy = y; yy < y + MinPUCSizeInPixels - 1; yy++)
          {
            LeftColumnCb[yy+1] = RecPicCb_p[(y0+yy)*RecPicWidth+xNeibor];
            LeftColumnCr[yy+1] = RecPicCr_p[(y0+yy)*RecPicWidth+xNeibor];
          }
        }
      }
      if(NeiborAvailable == 0)
      {
        memset(LeftColumnCb+y+1,LeftColumnCb[2*TrafoSize],(MinPUCSizeInPixels-1)*sizeof(uint8));
        memset(LeftColumnCr+y+1,LeftColumnCr[2*TrafoSize],(MinPUCSizeInPixels-1)*sizeof(uint8));
      }

      for(y = (sint32)(2*TrafoSize - 2*MinPUCSizeInPixels); y >= 0; y -= MinPUCSizeInPixels)
      {
        NeiborAvailable = 0;
        yNeibor = y0 + y;
        if(yNeibor >= 0 && yNeibor < PicHeight)
        {
          yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            NeiborAvailable = 1;
            for(yy = y; yy < y + MinPUCSizeInPixels; yy++)
            {
              LeftColumnCb[yy+1] = RecPicCb_p[(y0+yy)*RecPicWidth+xNeibor];
              LeftColumnCr[yy+1] = RecPicCr_p[(y0+yy)*RecPicWidth+xNeibor];
            }
          }
        }
        if(NeiborAvailable == 0)
        {
          memset(LeftColumnCb+y+1,LeftColumnCb[y+1+MinPUCSizeInPixels],MinPUCSizeInPixels*sizeof(uint8));
          memset(LeftColumnCr+y+1,LeftColumnCr[y+1+MinPUCSizeInPixels],MinPUCSizeInPixels*sizeof(uint8));
        }
      }

      yNeibor = y0 - 1;
      NeiborAvailable = 0;
      if(yNeibor >= 0 && yNeibor < PicHeight)
      {
        yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
        if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
          && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
        {
          NeiborAvailable = 1;
          LeftColumnCb[0] = RecPicCb_p[yNeibor*RecPicWidth+xNeibor];
          LeftColumnCr[0] = RecPicCr_p[yNeibor*RecPicWidth+xNeibor];
        }
      }
      if(NeiborAvailable == 0)
      {
        LeftColumnCb[0] = LeftColumnCb[1];
        LeftColumnCr[0] = LeftColumnCr[1];
      }
    }
    else
    {
      memset(LeftColumnCb, LeftColumnCb[2*TrafoSize], 2*TrafoSize*sizeof(uint8));
      memset(LeftColumnCr, LeftColumnCr[2*TrafoSize], 2*TrafoSize*sizeof(uint8));
    }

    AboveRowCb[0] = LeftColumnCb[0];
    AboveRowCr[0] = LeftColumnCr[0];

    yNeibor = y0 - 1;
    if(yNeibor >= 0 && yNeibor < PicHeight)
    {
      yNeiborInPUAddr = (yNeibor << 1) >> Log2PUSizeInPixels;
      for(x = 0; x <= (sint32)(2*TrafoSize - 1); x += MinPUCSizeInPixels)
      {
        NeiborAvailable = 0;
        xNeibor = x0 + x;
        if(xNeibor >= 0 && xNeibor < PicWidth)
        {
          xNeiborInPUAddr = (xNeibor << 1) >> Log2PUSizeInPixels;
          if((Sessiondata_p->PredictionUnitArray[yCurrInPUAddr*PicWidthInPUs + xCurrInPUAddr].TileId == Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].TileId)
            && Sessiondata_p->PredictionUnitArray[yNeiborInPUAddr*PicWidthInPUs + xNeiborInPUAddr].PredMode != NOT_DECODED)
          {
            NeiborAvailable = 1;
            memcpy(AboveRowCb+x+1, RecPicCb_p+yNeibor*RecPicWidth+x0+x, MinPUCSizeInPixels*sizeof(uint8));
            memcpy(AboveRowCr+x+1, RecPicCr_p+yNeibor*RecPicWidth+x0+x, MinPUCSizeInPixels*sizeof(uint8));
          }
        }
        if(NeiborAvailable == 0)
        {
          memset(AboveRowCb+x+1, AboveRowCb[x], MinPUCSizeInPixels*sizeof(uint8));
          memset(AboveRowCr+x+1, AboveRowCr[x], MinPUCSizeInPixels*sizeof(uint8));
        }
      }
    }
    else
    {
      memset(AboveRowCb + 1, AboveRowCb[0], 2*TrafoSize*sizeof(uint8));
      memset(AboveRowCr + 1, AboveRowCr[0], 2*TrafoSize*sizeof(uint8));
    }
  }
  else
  {
    memset(AboveRowCb, 128, (2*TrafoSize+1)*sizeof(uint8));
    memset(AboveRowCr, 128, (2*TrafoSize+1)*sizeof(uint8));
    memset(LeftColumnCb, 128, (2*TrafoSize+1)*sizeof(uint8));
    memset(LeftColumnCr, 128, (2*TrafoSize+1)*sizeof(uint8));
  }
  //
  if(IntraPredMode == INTRA_PLANAR)
  {
    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      for(x = 0; x < (sint32)TrafoSize; x++)
      {
        //Cb 
        sint32 PredSum = (TrafoSize - 1 - x)*LeftColumnCb[y + 1];
        PredSum += (x + 1)*AboveRowCb[TrafoSize + 1];
        PredSum += (TrafoSize - 1 - y)*AboveRowCb[x + 1];
        PredSum += (y + 1)*LeftColumnCb[TrafoSize + 1];
        PredSum += TrafoSize;
        PredSum = PredSum >> (Log2TrafoSize + 1);
        RecPicCb_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(PredSum);
        //Cr
        PredSum = (TrafoSize - 1 - x)*LeftColumnCr[y + 1];
        PredSum += (x + 1)*AboveRowCr[TrafoSize + 1];
        PredSum += (TrafoSize - 1 - y)*AboveRowCr[x + 1];
        PredSum += (y + 1)*LeftColumnCr[TrafoSize + 1];
        PredSum += TrafoSize;
        PredSum = PredSum >> (Log2TrafoSize + 1);
        RecPicCr_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(PredSum);
      }
    }
  }
  else if(IntraPredMode == INTRA_DC)
  {
    sint32 dcValCb = 0;
    sint32 dcValCr = 0;
    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      dcValCb += AboveRowCb[y + 1] + LeftColumnCb[y + 1];
      dcValCr += AboveRowCr[y + 1] + LeftColumnCr[y + 1];
    }
    dcValCb += TrafoSize;
    dcValCb = dcValCb >> (Log2TrafoSize + 1);
    dcValCr += TrafoSize;
    dcValCr = dcValCr >> (Log2TrafoSize + 1);
    for(y = 0; y < (sint32)TrafoSize; y++)
    {
      for(x = 0; x < (sint32)TrafoSize; x++)
      {
        RecPicCb_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(dcValCb);
        RecPicCr_p[(y0 + y)*RecPicWidth + (x0 + x)] = (uint8)(dcValCr);
      }
    }
  }
  else
  {
    sint32 IntraPredAngle = IntraPredAngleTable[IntraPredMode];
    sint32 InvAngle = InvAngleTable[IntraPredMode - 11];
    sint32 RefSampleExt = (IntraPredAngle << Log2TrafoSize) >> 5;
    sint32 iIdx;
    sint32 iFact;

    if(IntraPredMode >= 18)
    {
      memcpy(RefSampleCb+TrafoSize,AboveRowCb,(2*TrafoSize+1)*sizeof(uint8));
      memcpy(RefSampleCr+TrafoSize,AboveRowCr,(2*TrafoSize+1)*sizeof(uint8));
      if(RefSampleExt < -1)
      {
        for(x = RefSampleExt; x <= -1; x++)
        {
          RefSampleCb[TrafoSize+x] = LeftColumnCb[(x*InvAngle+128) >> 8];
          RefSampleCr[TrafoSize+x] = LeftColumnCr[(x*InvAngle+128) >> 8];
        }
      }
      for(y = 0; y < (sint32)TrafoSize; y++)
      {
        iIdx = ((y + 1)*IntraPredAngle) >> 5;
        iFact = ((y + 1)*IntraPredAngle) & 31;
        if(iFact != 0)
        {
          for(x = 0; x < (sint32)TrafoSize; x++)
          {
            RecPicCb_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSampleCb[TrafoSize+x+iIdx+1] + iFact*RefSampleCb[TrafoSize+x+iIdx+2] + 16)>>5);
            RecPicCr_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSampleCr[TrafoSize+x+iIdx+1] + iFact*RefSampleCr[TrafoSize+x+iIdx+2] + 16)>>5);
          }
        }
        else
        {
          for(x = 0; x < (sint32)TrafoSize; x++)
          {
            RecPicCb_p[(y0+y)*RecPicWidth+x0+x] = RefSampleCb[TrafoSize+x+iIdx+1];
            RecPicCr_p[(y0+y)*RecPicWidth+x0+x] = RefSampleCr[TrafoSize+x+iIdx+1];
          }
        }
      }
    }
    else
    {
      memcpy(RefSampleCb+TrafoSize, LeftColumnCb, (2*TrafoSize+1)*sizeof(uint8));
      memcpy(RefSampleCr+TrafoSize, LeftColumnCr, (2*TrafoSize+1)*sizeof(uint8));
      if(RefSampleExt < -1)
      {
        for(x = RefSampleExt; x <= -1; x++)
        {
          RefSampleCb[TrafoSize+x] = AboveRowCb[(x*InvAngle+128) >> 8];
          RefSampleCr[TrafoSize+x] = AboveRowCr[(x*InvAngle+128) >> 8];
        }
      }
      for(y = 0; y < (sint32)TrafoSize; y++)
      {
        for(x = 0; x < (sint32)TrafoSize; x++)
        {
          iIdx = ((x + 1)*IntraPredAngle) >> 5;
          iFact = ((x + 1)*IntraPredAngle) & 31;
          if(iFact != 0)
          {
            RecPicCb_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSampleCb[TrafoSize+y+iIdx+1] + iFact*RefSampleCb[TrafoSize+y+iIdx+2] + 16)>>5);
            RecPicCr_p[(y0+y)*RecPicWidth+x0+x] = (uint8)(((32-iFact)*RefSampleCr[TrafoSize+y+iIdx+1] + iFact*RefSampleCr[TrafoSize+y+iIdx+2] + 16)>>5);
          }
          else
          {
            RecPicCb_p[(y0+y)*RecPicWidth+x0+x] = RefSampleCb[TrafoSize+y+iIdx+1];
            RecPicCr_p[(y0+y)*RecPicWidth+x0+x] = RefSampleCr[TrafoSize+y+iIdx+1];
          }
        }
      }
    }
  }
}