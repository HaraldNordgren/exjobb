/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains TU decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <string.h>
#include "d65_decode_tu.h"
#include "d65_transform_reconstruct.h"
#include "d65_intra_prediction.h"
#include "d65_cabac.h"

/******************************************************************************
* Definition of local variables
******************************************************************************/
static const sint32 SCAN_ORDER[3][4][64] =
{
  {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  2,  1,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  4,  1,  8,  5,  2, 12,  9,  6,  3, 13, 10,  7, 14, 11, 15,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  8,  1, 16,  9,  2, 24, 17, 10,  3, 32, 25, 18, 11,  4, 40,
    33, 26, 19, 12,  5, 48, 41, 34, 27, 20, 13,  6, 56, 49, 42, 35,
    28, 21, 14,  7, 57, 50, 43, 36, 29, 22, 15, 58, 51, 44, 37, 30,
    23, 59, 52, 45, 38, 31, 60, 53, 46, 39, 61, 54, 47, 62, 55, 63 }
  },
  {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  1,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 }
  },
  {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  2,  1,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  4,  8, 12,  1,  5,  9, 13,  2,  6, 10, 14,  3,  7, 11, 15,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

    { 0,  8, 16, 24, 32, 40, 48, 56,  1,  9, 17, 25, 33, 41, 49, 57,
    2, 10, 18, 26, 34, 42, 50, 58,  3, 11, 19, 27, 35, 43, 51, 59,
    4, 12, 20, 28, 36, 44, 52, 60,  5, 13, 21, 29, 37, 45, 53, 61,
    6, 14, 22, 30, 38, 46, 54, 62,  7, 15, 23, 31, 39, 47, 55, 63 }
  }
};

/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        DeriveNbQg
*
* Parameters:  Sessiondata_p     [in]      The decoder
*              xCurr             [in]      Current CU X
*              yCurr             [in]      Current CU Y
*              xNb               [in]      Quantization Group X
*              yNb               [in]      Quantization Group Y
*              QpPrev            [in]      Previous QP
*              QpNb              [in/out]  Qp of quantization group
*
* Returns:     -
*
* Description: Check availability of a target quantization group and derive
*              corresponding QP.
*
******************************************************************************/
uint8
DeriveNbQg(SessionData_t    *Sessiondata_p,
       const sint32      xCurr,
       const sint32      yCurr,
       const sint32      xNb,
       const sint32      yNb,
       const sint32      QpPrev,
       sint32           *QpNb)
{
  const uint32 LogMinPUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 PicWidthInPu = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 Log2CtbSize  = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const sint32 PicWidth     = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  const sint32 PicHeight    = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;

  *QpNb = QpPrev;

  if(xNb < 0 || yNb < 0 || xNb > PicWidth || yNb > PicHeight)
  {
    return 0;
  }
  else
  {
    uint32 PuAddrNb   = (yNb   >> LogMinPUSizeInPixels) * PicWidthInPu + (xNb   >> LogMinPUSizeInPixels);
    uint32 PuAddrCurr = (yCurr >> LogMinPUSizeInPixels) * PicWidthInPu + (xCurr >> LogMinPUSizeInPixels);
    if(Sessiondata_p->PredictionUnitArray[PuAddrNb].TileId != Sessiondata_p->PredictionUnitArray[PuAddrCurr].TileId)
    {
      return 0;
    }
    else
    {
      if((xNb >> Log2CtbSize << Log2CtbSize) != (xCurr >> Log2CtbSize << Log2CtbSize) ||
        (yNb >> Log2CtbSize << Log2CtbSize) != (yCurr >> Log2CtbSize << Log2CtbSize))
      {
        return 0;
      }
      else
      {
        *QpNb = Sessiondata_p->PredictionUnitArray[PuAddrNb].QpY;
        return 1;
      }
    }
  }

  return 0;
}

#if GT_RESIDUAL_PRUNING
/******************************************************************************
*
* Name:        Encode_residual_dc_luma
*
* Parameters:  Sessiondata_p     [in/out]  The decoder
*              log2TrafoSize     [in]      log size of the TU in pixels
*              scanIdx           [in]      coefficient scan index
*              transQuantBypass  [in]      transform & Quantization bypass flag
*              TransformSkipFlag [in]      transform skip flag
* Returns:     -
*
* Description: Fast encoding residual block with dc = 1, ac = 0
*
******************************************************************************/
void
Encode_residual_dc_luma(
#if MULTI_THREADED
                       ThreadData_t  *Threaddata_p,
#else
                       SessionData_t *Sessiondata_p,
#endif
                       const uint32 log2TrafoSize,
                       const uint8 scanIdx,
                       const uint8 transQuantBypass,
                       uint8 *TransformSkipFlag)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
#else
  Parser_t *Parser_p       = &Sessiondata_p->Parser;
#endif

  const uint8  transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;
  const uint8  trafoSizeIs4x4              = (log2TrafoSize == 2) ? 2 : 0;
  if(transform_skip_enabled_flag && !transQuantBypass && trafoSizeIs4x4)
  {
    EncodeTrSkipLumaFlag(Parser_p, *TransformSkipFlag);
  }

  // encode the last significant coefficients' coordinates
  EncodeLastSigCoeffPrefixXLuma(Parser_p, log2TrafoSize, 0);
  EncodeLastSigCoeffPrefixYLuma(Parser_p, log2TrafoSize, 0);

  EncodeBin(&Parser_p->SubEncoder, Parser_p->SubEncoder.CtxModels.CoeffAbsLevelG1LumaCtx + 1, 0);

  EncodeBinEP(&Parser_p->SubEncoder, 0);
}

/******************************************************************************
*
* Name:        Encode_residual_dc_chroma
*
* Parameters:  Sessiondata_p     [in/out]  The decoder
*              log2TrafoSize     [in]      log size of the TU in pixels
*              scanIdx           [in]      coefficient scan index
*              transQuantBypass  [in]      transform & Quantization bypass flag
*              TransformSkipFlag [in]      transform skip flag
* Returns:     -
*
* Description: Fast encoding residual block with dc = 1, ac = 0
*
******************************************************************************/
void
Encode_residual_dc_chroma(
#if MULTI_THREADED
                          ThreadData_t  *Threaddata_p,
#else
                          SessionData_t *Sessiondata_p,
#endif
                          const uint32 log2TrafoSize,
                          const uint8 scanIdx,
                          const uint8 transQuantBypass,
                          uint8 *TransformSkipFlag)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
#else
  Parser_t *Parser_p       = &Sessiondata_p->Parser;
#endif
  const uint8  transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;
  const uint8  trafoSizeIs4x4              = (log2TrafoSize == 2) ? 2 : 0;

  if(transform_skip_enabled_flag && !transQuantBypass && trafoSizeIs4x4)
  {
    EncodeTrSkipChromaFlag(Parser_p, *TransformSkipFlag);
  }

  // encode the last significant coefficients' coordinates
  EncodeLastSigCoeffPrefixXChroma(Parser_p, log2TrafoSize, 0);
  EncodeLastSigCoeffPrefixYChroma(Parser_p, log2TrafoSize, 0);

  EncodeBin(&Parser_p->SubEncoder, Parser_p->SubEncoder.CtxModels.CoeffAbsLevelG1ChromaCtx + 1, 0);

  EncodeBinEP(&Parser_p->SubEncoder,0);
}
#endif

#if GT_RESIDUAL_RECONSTRUCT
void
Encode_residual_luma(
#if MULTI_THREADED
                     ThreadData_t  *Threaddata_p,
#else
                     SessionData_t *Sessiondata_p,
#endif
                     uint32 Log2TrafoSize,
                     uint8 scanIndex,
                     uint8 transQuantBypass,
                     uint8 TransformSkipFlag)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
  sint16        *Coeffs_p       = Threaddata_p->ResTrQuantArray;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
  sint16   *Coeffs_p = Sessiondata_p->ResTrQuantArray;
#endif

  const sint32 size            = 1 << Log2TrafoSize;
  const sint32 sizeInSubblocks = size >> 2;
  const sint32 baseOffset      = (Log2TrafoSize == 3) ? ((scanIndex == SCAN_DIAGONAL) ? 9 : 15) : 21; 

  const sint32 *xorder     = Sessiondata_p->g_xScan[scanIndex][Log2TrafoSize - 1];
  const sint32 *yorder     = Sessiondata_p->g_yScan[scanIndex][Log2TrafoSize - 1];
  const sint32 *SbScan     = SCAN_ORDER[scanIndex][Log2TrafoSize - 2];

  const uint8 transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;
  const uint8 sign_data_hiding_flag       = Sessiondata_p->CurrentPictureParameterSet->SignDataHidingFlag;

  Bool   sigInSubblock[8][8];
  sint32 i, x, y, pos, n;
  sint32 sigCoeffs[16];
  sint32 sigCoeffsSize;
  sint32 lastPos  = -1;
  sint32 lastPosX = -1;
  sint32 lastPosY = -1;
  sint32 *intpoint;
  sint32 sigInt=0;
  sint32 lastSubblock;
  sint32 c1;
  sint32 subblock;
  sint32 subblockpos;
  sint32 sbY;
  sint32 sbX;
  sint32 subblockStart;
  sint32 contextCount;
  sint32 offset;
  sint32 coeff;
  sint32 numSigSubblock;
  sint32 firstGreaterOne;
  sint32 firstGreaterOnePos;
  sint32 firstSigScanPos;
  sint32 lastSigScanPos;
  sint32 subblockOffset;
  sint32 numGreaterOne;
  uint32 golombRiceParam;
  Bool   sig;
  Bool   signHidden;

  if(transform_skip_enabled_flag && !transQuantBypass && (Log2TrafoSize == 2))
  {
    EncodeTrSkipLumaFlag(Parser_p, TransformSkipFlag);
  }

  memset(sigInSubblock, 0, 64 * sizeof(sigInSubblock[0][0]));

  for (i = size*size - 1; i >= 0; i -= 16, sigInt = 0)
  {
    y = yorder[i - 15];
    x = xorder[i - 15];

    for (n = y; n < y + 4; n++)
    {
      intpoint = (sint32*)(Coeffs_p + (n * size) + x);
      sigInt |= intpoint[0];
      sigInt |= intpoint[1];
    }
    sigInSubblock[y >> 2][x >> 2]= sigInt != 0;

    if (lastPos < 0 && sigInt != 0)
    {
      for (n = i; n > i - 16; n--)
      {
        y = yorder[n];
        x = xorder[n];
        if (Coeffs_p[y * size + x] != 0)
        {
          lastPosX = x;
          lastPosY = y;
          lastPos  = n;
          break;
        }
      }
    }
  }

  if (scanIndex != SCAN_VERTICAL)
  {
    EncodeLastSigCoeffLuma(Parser_p, lastPosX, lastPosY, Log2TrafoSize);
  }
  else
  {
    EncodeLastSigCoeffLuma(Parser_p, lastPosY, lastPosX, Log2TrafoSize);
  }

  lastSubblock = lastPos >> 4;
  pos = lastPos;

  c1 = 1;
  for (subblock = lastSubblock; subblock>=0; subblock--)
  {
    subblockStart = subblock << 4;
    subblockpos   = SbScan[subblock];

    sbY = subblockpos >> (Log2TrafoSize - 2);
    sbX = subblockpos - (sbY << (Log2TrafoSize - 2));

    contextCount = 0;
    if (sbX < sizeInSubblocks - 1)
    {
      contextCount += sigInSubblock[sbY][sbX + 1];
    }
    if (sbY < sizeInSubblocks - 1)
    {
      contextCount += (sigInSubblock[sbY + 1][sbX] << 1);
    }

    if (subblock < lastSubblock && subblock > 0)
    {
      EncodeCodedSubBlockFlagLuma(Parser_p, contextCount > 0, sigInSubblock[sbY][sbX]);
      if (!sigInSubblock[sbY][sbX])
      {
        pos-=16;
        continue;
      }
    }

    offset = ((sbX + sbY) > 0) ? baseOffset + 3 : baseOffset;

    sigCoeffsSize   =  0;
    firstSigScanPos = -1;
    lastSigScanPos  = -1;

    if (pos == lastPos)
    {
      y = yorder[pos];
      x = xorder[pos];
      sigCoeffs[sigCoeffsSize++] = Coeffs_p[y * size + x];
      firstSigScanPos = pos;
      lastSigScanPos  = pos;
      pos--;
    }

    for ( ; pos > subblockStart; pos--)
    {
      y = yorder[pos];
      x = xorder[pos];
      coeff = Coeffs_p[y * size + x];

      sig = coeff!=0;

      EncodeSigCoeffFlagLuma(Parser_p, x, y, sbX, sbY, contextCount, offset, size, sig);
      if (sig)
      {
        sigCoeffs[sigCoeffsSize++] = coeff;
        if (lastSigScanPos == -1)
        {
          lastSigScanPos = pos;
        }
        firstSigScanPos = pos;
      }
    }

    if (pos == subblockStart)
    {
      y = yorder[pos];
      x = xorder[pos];
      coeff = Coeffs_p[y * size + x];

      sig = coeff!=0;

      if (sigCoeffsSize || subblock == 0)
      {
        EncodeSigCoeffFlagLuma(Parser_p,x,y, sbX, sbY, contextCount, offset, size, sig);
      }

      if(sig)
      {
        sigCoeffs[sigCoeffsSize++] = coeff;
        if (lastSigScanPos == -1)
        {
          lastSigScanPos = pos;
        }
        firstSigScanPos = pos;
      }
      pos--;
    }

    numSigSubblock = sigCoeffsSize;

    if (numSigSubblock > 0)
    {
      firstGreaterOne    = -1;
      firstGreaterOnePos = -1;
      subblockOffset     = (subblock > 0) ? 8 : 0;

      if(c1 == 0)
      {
        subblockOffset += 4;
      }
      c1 = 1;

      numGreaterOne = D65_MIN(8, numSigSubblock);

      for (n = 0; n < sigCoeffsSize && numGreaterOne > 0; n++)
      {
        numGreaterOne--;
        if (D65_ABS(sigCoeffs[n]) > 1)
        {
          EncodeCoeffAbsLevelG1Luma(Parser_p, subblockOffset + c1, 1);
          c1 = 0;
          firstGreaterOne    = sigCoeffs[n];
          firstGreaterOnePos = n;
          n++;
          break;
        }
        else
        {
          EncodeCoeffAbsLevelG1Luma(Parser_p, subblockOffset + c1, 0);
          if (c1 < 3)
          {
            c1++;
          }
        }
      }

      for( ; n < sigCoeffsSize && numGreaterOne > 0; n++)
      {
        numGreaterOne--;
        EncodeCoeffAbsLevelG1Luma(Parser_p, subblockOffset, D65_ABS(sigCoeffs[n]) > 1);
      }

      if (firstGreaterOnePos != -1)
      {
        Bool greaterTwo = D65_ABS(firstGreaterOne) > 2;
        EncodeCoeffAbsLevelG2Luma(Parser_p, subblockOffset >> 2, greaterTwo);
      }

      signHidden = sign_data_hiding_flag && !transQuantBypass && (lastSigScanPos - firstSigScanPos > 3);



      //n = 0;
      //if (!signHidden)
      //{
      //  EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      //  n++;
      //}

      for (n = 0; n < (sigCoeffsSize - 1); n++)
      {
        EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      }

      if (!signHidden)
      {
        EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      }
      else
      {
        uint8 bad = 1;
      }

      golombRiceParam = 0;
      numGreaterOne   = D65_MIN(8, numSigSubblock);

      for (n = 0; n < sigCoeffsSize; n++)
      {
        sint32 numLeft = D65_ABS(sigCoeffs[n]) - 1;
        if (numGreaterOne > 0)
        {
          numLeft--;
          numGreaterOne--;
          if (n == firstGreaterOnePos)
          {
            numLeft--;
          }
        }

        if (numLeft >= 0)
        {
          EncodeGolombRice(&Parser_p->SubEncoder,golombRiceParam, numLeft);
          if (D65_ABS(sigCoeffs[n]) > 3 * (1 << golombRiceParam))
          {
            golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
          }
        }
      }
    }
  }
}

void Encode_residual_chroma(
#if MULTI_THREADED
                            ThreadData_t  *Threaddata_p,
#else
                            SessionData_t *Sessiondata_p,
#endif
                            uint32 Log2TrafoSize,
                            uint8 scanIndex,
                            uint8 transQuantBypass,
                            uint8 TransformSkipFlag)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
  sint16        *Coeffs_p       = Threaddata_p->ResTrQuantArray;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
  sint16   *Coeffs_p = Sessiondata_p->ResTrQuantArray;
#endif

  const sint32 *SbScan = SCAN_ORDER[scanIndex][Log2TrafoSize - 2];
  const sint32 *xorder = Sessiondata_p->g_xScan[scanIndex][Log2TrafoSize-1];
  const sint32 *yorder = Sessiondata_p->g_yScan[scanIndex][Log2TrafoSize-1];

  const sint32 size = 1 << Log2TrafoSize;
  const sint32 sizeInSubblocks = size >> 2;
  const sint32 offset = (size  == 8) ? ((scanIndex == SCAN_DIAGONAL) ? 9 : 15) : 12;

  const uint8  transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;
  const uint8  sign_data_hiding_flag       = Sessiondata_p->CurrentPictureParameterSet->SignDataHidingFlag;

  Bool   sigInSubblock[4][4];

  sint32 lastPos  = -1;
  sint32 lastPosX = -1;
  sint32 lastPosY = -1;

  sint32 subblock;
  sint32 i, x, y, pos, n;
  sint32 lastSubblock;
  sint32 sigCoeffs[16];
  sint32 sigCoeffsSize;
  sint32 c1;
  sint32 subblockStart;
  sint32 subblockpos;
  sint32 contextCount;
  sint32 inx, iny;
  sint32 numSigSubblock;
  sint32 contextOffset;
  sint32 numGreaterOne;
  sint32 firstGreaterOne;
  sint32 firstGreaterOnePos;
  sint32 firstSigScanPos;
  sint32 lastSigScanPos;
  uint32 golombRiceParam;
  Bool   greaterTwo;
  Bool   sig;
  Bool   signHidden;

  if(transform_skip_enabled_flag && !transQuantBypass && (Log2TrafoSize == 2))
  {
    EncodeTrSkipChromaFlag(Parser_p, TransformSkipFlag);
  }

  memset(sigInSubblock[0], 0, 16 * sizeof(sigInSubblock[0][0]));

  for (i = size*size - 1; i >= 0; i--)
  {
    y = yorder[i];
    x = xorder[i];
    if (Coeffs_p[y * size + x] != 0)
    {
      sigInSubblock[y/4][x/4] = 1;
      lastPosX = x;
      lastPosY = y;
      lastPos = i;
      break;
    }
  }

  for (; i >= 0; i--)
  {
    y = yorder[i];
    x = xorder[i];
    sigInSubblock[y/4][x/4] |= Coeffs_p[y * size + x] != 0;
  }

  if (scanIndex != SCAN_VERTICAL)
  {
    EncodeLastSigCoeffChroma(Parser_p, lastPosX, lastPosY, Log2TrafoSize);
  }
  else
  { 
    EncodeLastSigCoeffChroma(Parser_p, lastPosY, lastPosX, Log2TrafoSize);
  }

  lastSubblock = lastPos >> 4;
  pos = lastPos;

  c1 = 1;

  for (subblock = lastSubblock; subblock>=0; subblock--)
  {
    subblockStart = subblock<<4;
    subblockpos   = SbScan[subblock];


    y = subblockpos / sizeInSubblocks;
    x = subblockpos % sizeInSubblocks;

    contextCount = 0;
    if (x < sizeInSubblocks - 1)
    {
      contextCount += sigInSubblock[y][x + 1];
    }
    if (y < sizeInSubblocks - 1){
      contextCount += (sigInSubblock[y + 1][x] << 1);
    }

    //encode significant_coeff_group_flag
    if (subblock < lastSubblock && subblock > 0)
    {
      EncodeCodedSubBlockFlagChroma(Parser_p, contextCount > 0, sigInSubblock[y][x]);
      if (!sigInSubblock[y][x])
      {
        pos-=16;
        continue;
      }
    }

    sigCoeffsSize   =  0;
    firstSigScanPos = -1;
    lastSigScanPos  = -1;

    //encode significant_coeff_flag
    for ( ; pos >= subblockStart; pos--)
    {
      iny = yorder[pos];
      inx = xorder[pos];
      sig = Coeffs_p[iny * size + inx] != 0;

      if (pos != lastPos && (pos > subblockStart || sigCoeffsSize || subblock == 0))
      {
        EncodeSigCoeffFlagChroma(Parser_p, inx, iny, size, x, y, contextCount, offset, sig); 
      }
      if(sig) 
      {
        sigCoeffs[sigCoeffsSize++] = Coeffs_p[iny * size + inx];
        if (lastSigScanPos == -1)
        {
          lastSigScanPos = pos;
        }
        firstSigScanPos = pos;
      }
    }

    if (sigCoeffsSize > 0)
    {
      numSigSubblock = sigCoeffsSize;
      contextOffset = (c1 == 0) ? 4 : 0;
      c1 = 1;

      //encode abs_level_greater_one_flag
      numGreaterOne = D65_MIN(8, numSigSubblock);

      firstGreaterOne    = -1;
      firstGreaterOnePos = -1;

      for (n =  0; n < sigCoeffsSize && numGreaterOne > 0; n++)
      {
        numGreaterOne--;
        if (D65_ABS(sigCoeffs[n]) > 1)
        {
          EncodeCoeffAbsLevelG1Chroma(Parser_p, contextOffset + c1, 1);
          c1 = 0;
          firstGreaterOne    = sigCoeffs[n];
          firstGreaterOnePos = n;
          n++;
          break;
        }
        else
        {
          EncodeCoeffAbsLevelG1Chroma(Parser_p, contextOffset + c1, 0);
          if (c1 < 3)
          {
            c1++;
          }
        }
      }

      //continuation for above loop after firstGreaterOnePos has been determined, because it is much simpler then
      for( ; n < sigCoeffsSize && numGreaterOne > 0; n++)
      {
        numGreaterOne--;
        EncodeCoeffAbsLevelG1Chroma(Parser_p,contextOffset, D65_ABS(sigCoeffs[n]) > 1);
      }

      //encode greater2 for first greater1 coeff
      if (firstGreaterOnePos != -1)
      {
        greaterTwo = D65_ABS(firstGreaterOne) > 2;
        EncodeCoeffAbsLevelG2Chroma(Parser_p, contextOffset >> 2,greaterTwo);
      }

      signHidden = sign_data_hiding_flag && !transQuantBypass && (lastSigScanPos - firstSigScanPos > 3);

      //encode signs
      //n = 0;
      //if (!signHidden)
      //{
      //  EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      //  n++;
      //}
      for (n = 0 ; n < (sigCoeffsSize - 1); n++)
      {
        EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      }
      if (!signHidden)
      {
        EncodeBinEP(&Parser_p->SubEncoder, sigCoeffs[n] < 0);
      }

      //encode remaining bits
      numGreaterOne = D65_MIN(8, numSigSubblock);
      golombRiceParam =0;
      for (n = 0; n < sigCoeffsSize; n++)
      {
        sint32 numLeft = D65_ABS(sigCoeffs[n]) - 1;
        if (numGreaterOne > 0)
        {
          numLeft--;
          numGreaterOne--;
          if (n == firstGreaterOnePos)
          {
            numLeft--;
          }
        }
        if(numLeft>=0)
        {
          EncodeGolombRice(&Parser_p->SubEncoder, golombRiceParam, numLeft);
          if (D65_ABS(sigCoeffs[n]) > 3 * (1<<golombRiceParam))
          {
            golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
          }
        }
      }
    }
  }
}

#endif

/******************************************************************************
*
* Name:        Parse_residual_coding_luma
*
* Parameters:  Sessiondata_p     [in/out]  The decoder
*              log2TrafoSize     [in]      log size of the TU in pixels
*              scanIdx           [in]      coefficient scan index
*              transQuantBypass  [in]      transform & Quantization bypass flag
*              TransformSkipFlag [in]      transform skip flag
* Returns:     -
*
* Description: Decodes luma residuals of a TU
*
******************************************************************************/
void
Parse_residual_coding_luma(
#if MULTI_THREADED
               ThreadData_t  *Threaddata_p,
#else
               SessionData_t *Sessiondata_p,
#endif
               const uint32 log2TrafoSize,
               const uint8 scanIdx,
               const uint8 transQuantBypass,
               uint8 *TransformSkipFlag
               )
{
  const static uint32 MinInGroup[ 10 ] = { 0, 1, 2, 3, 4, 6, 8, 12, 16, 24 };

#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
  sint16        *TransformArray = Threaddata_p->TransformArray;
#else
  Parser_t *Parser_p       = &Sessiondata_p->Parser;
  sint16   *TransformArray = Sessiondata_p->TransformArray;
#endif
  const uint32 log2trafoM2    = log2TrafoSize - 2;
  const uint32 subblocksize   = 1 << log2trafoM2;
  const uint8  trafoSizeIs4x4 = (log2TrafoSize == 2) ? 2 : 0;
  const uint8  BaseSigCoeffContextOffset   = (log2TrafoSize == 3) ? ((scanIdx == 0) ? 9 : 15) : 21;
  const uint8  sign_data_hiding_flag       = Sessiondata_p->CurrentPictureParameterSet->SignDataHidingFlag;
  const uint8  transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;

  const sint32 *bigscan = SCAN_ORDER[scanIdx][log2trafoM2];
  const sint32 *scan    = SCAN_ORDER[scanIdx][2];
  sint16 *baseTr;

  uint8 coded_sub_block_flag[64];
  uint8 coeff_sign_flag[16];
  uint8 inferSbDcSigCoeffFlag;
  uint8 sigCoeffFlag;
  uint8 coeffAbsLevelG1Flag;
  uint8 coeffAbsLevelG2Flag;
  uint8 signHidden;
  uint8 lastScanPos;
  uint8 SubFlagAndSigCoeffContextOffset;
  uint8 SigCoeffContextOffset;
  uint8 sigCoeffContextFlag;
  uint8 golombRiceParam;
  uint8 ctxSet;
  uint8 c1 = 1;
  uint8 numNonZeroCount;
  uint8 numNonZeroM1MN;
  uint8 numSigCoeff;
  uint8 numGreater1Flag;
  sint8 n;

  uint32 AbsTransCoeffLevel[16];
  uint32 sigCoeffPos[16];
  uint32 sigCoeffScanPos[16];
  uint32 coeffAbsLevelRemain;
  uint32 LastSignificantCoeffX;
  uint32 LastSignificantCoeffY;
  uint32 LastSignificantCoeffXprefix;
  uint32 LastSignificantCoeffYprefix;
  sint32 subblockScanPos;
  sint32 innerBlockScanPos;
  sint32 lastSubBlock;
  uint32 firstSigScanPos;
  uint32 lastSigScanPos;
  sint32 lastGreater1ScanPos;
  uint32 sumAbsLevel;
  uint32 baseLevel;
  uint32 xS;
  uint32 yS;
  uint32 pos;
  uint32 bigpos;
  sint32 i;
  sint32 TransCoeffLevel;

  memset(coded_sub_block_flag, 0, 64 * sizeof(coded_sub_block_flag[0]));

  if(transform_skip_enabled_flag && !transQuantBypass && trafoSizeIs4x4)
  {
    *TransformSkipFlag = GetTrSkipLumaFlag(Parser_p);
  }
  else
  {
    *TransformSkipFlag = 0;
  }

  LastSignificantCoeffXprefix = GetLastSigCoeffPrefixXLumaArray[log2trafoM2](Parser_p);
  LastSignificantCoeffYprefix = GetLastSigCoeffPrefixYLumaArray[log2trafoM2](Parser_p);

  if(LastSignificantCoeffXprefix > 3)
  {
    LastSignificantCoeffX  = MinInGroup[LastSignificantCoeffXprefix];
    LastSignificantCoeffX += GetLastSigCoeffSuffix(Parser_p, LastSignificantCoeffXprefix);
  }
  else
  {
    LastSignificantCoeffX = LastSignificantCoeffXprefix;
  }

  if(LastSignificantCoeffYprefix > 3)
  {
    LastSignificantCoeffY  = MinInGroup[LastSignificantCoeffYprefix];
    LastSignificantCoeffY += GetLastSigCoeffSuffix(Parser_p, LastSignificantCoeffYprefix);
  }
  else
  {
    LastSignificantCoeffY = LastSignificantCoeffYprefix;
  }

  if(scanIdx == 2)
  {
    uint32 LastSignificantCoeffTmp = LastSignificantCoeffY;
    LastSignificantCoeffY = LastSignificantCoeffX;
    LastSignificantCoeffX = LastSignificantCoeffTmp;
  }

  subblockScanPos   = ((LastSignificantCoeffY >> 2) << log2trafoM2) + (LastSignificantCoeffX >> 2);
  innerBlockScanPos = ((LastSignificantCoeffY & 0x03) << 2) + (LastSignificantCoeffX & 0x03);

  lastSubBlock = subblocksize * subblocksize;
  while(bigscan[--lastSubBlock] != subblockScanPos) { }

  lastScanPos = 16;
  while(scan[--lastScanPos] != innerBlockScanPos) { }

  for(i = lastSubBlock, sigCoeffContextFlag = trafoSizeIs4x4; i >= 0; i--, sigCoeffContextFlag = trafoSizeIs4x4) 
  {
    bigpos  = (uint32)bigscan[i];

    yS      = bigpos >> (log2trafoM2);
    xS      = bigpos - (yS << (log2trafoM2));

    baseTr = TransformArray + (yS << (log2TrafoSize + 2)) + (xS << 2);

    SubFlagAndSigCoeffContextOffset = 0;
    if(xS < (subblocksize - 1))
    {
      SubFlagAndSigCoeffContextOffset += coded_sub_block_flag[bigpos + 1];
    }
    if(yS < (subblocksize- 1))
    {
      SubFlagAndSigCoeffContextOffset += (coded_sub_block_flag[bigpos + subblocksize] << 1);
    }

    SigCoeffContextOffset = bigpos ? BaseSigCoeffContextOffset + 3: BaseSigCoeffContextOffset;

    if(i < lastSubBlock && i > 0) 
    {
      coded_sub_block_flag[bigpos] = GetCodedSubBlockFlagLuma(Parser_p,(SubFlagAndSigCoeffContextOffset > 0));
      inferSbDcSigCoeffFlag        = 1;
    }
    else
    {
      coded_sub_block_flag[bigpos] = 1;
      inferSbDcSigCoeffFlag        = 0;
    }

    numNonZeroCount = 0;

    if(i == lastSubBlock)
    {
      pos = scan[lastScanPos];
      sigCoeffPos[numNonZeroCount] = pos;
      sigCoeffScanPos[numNonZeroCount]  = lastScanPos;
      AbsTransCoeffLevel[numNonZeroCount] = 1;
      numNonZeroCount++;

      for(n = lastScanPos - 1; n > 0; n--)
      {
        pos = scan[n];
        sigCoeffFlag = GetSigCoeffFlagLuma(Parser_p, sigCoeffContextFlag, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
        if(sigCoeffFlag)
        {
          sigCoeffPos[numNonZeroCount] = pos;
          sigCoeffScanPos[numNonZeroCount]  = n;
          AbsTransCoeffLevel[numNonZeroCount] = 1;
          numNonZeroCount++;
        }
      }
      if(n == 0)
      {
        pos = scan[0];
        sigCoeffContextFlag += (i == 0) ? 1 : 0;
        sigCoeffFlag = GetSigCoeffFlagLuma(Parser_p, sigCoeffContextFlag, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
        if(sigCoeffFlag)
        {
          sigCoeffPos[numNonZeroCount] = pos;
          sigCoeffScanPos[numNonZeroCount]  = 0;
          AbsTransCoeffLevel[numNonZeroCount] = 1;
          numNonZeroCount++;
        }
      }
    } 
    else
    {
      if(coded_sub_block_flag[bigpos])
      {
        //handle positions 15 through 1
        for(n = 15; n > 0; n--)
        {
          pos = scan[n];
          sigCoeffFlag = GetSigCoeffFlagLuma(Parser_p, sigCoeffContextFlag, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
          if(sigCoeffFlag)
          {
            inferSbDcSigCoeffFlag=0;
            sigCoeffPos[numNonZeroCount] = pos;
            sigCoeffScanPos[numNonZeroCount]  = n;
            AbsTransCoeffLevel[numNonZeroCount] = 1;
            numNonZeroCount++;
          }
        }
        //handle n==0 which sometimes infers value
        pos=scan[0];
        if(!inferSbDcSigCoeffFlag)
        {
          sigCoeffContextFlag += (i == 0) ? 1 : 0;
          sigCoeffFlag = GetSigCoeffFlagLuma(Parser_p, sigCoeffContextFlag, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
          if(sigCoeffFlag)
          {
            sigCoeffPos[numNonZeroCount] = pos;
            sigCoeffScanPos[numNonZeroCount]  = 0;
            AbsTransCoeffLevel[numNonZeroCount] = 1;
            numNonZeroCount++;
          }
        }
        else
        {
          sigCoeffPos[numNonZeroCount] = pos;
          sigCoeffScanPos[numNonZeroCount]  = 0;
          AbsTransCoeffLevel[numNonZeroCount] = 1;
          numNonZeroCount++;
        }
      }
    }

    if(numNonZeroCount)
    {
      firstSigScanPos     = sigCoeffScanPos[numNonZeroCount-1];
      lastSigScanPos      = sigCoeffScanPos[0];
      numGreater1Flag     = 0;
      lastGreater1ScanPos = -1;

      ctxSet = (i > 0) ? 2 : 0;
      if(c1 == 0)
      {
        ctxSet++;
      }
      c1 = 1;
      for(n = numNonZeroCount - 1; (numGreater1Flag < 8) && n >= 0; n--)
      {
        coeffAbsLevelG1Flag = GetCoeffAbsLevelGreater1FlagLuma(Parser_p, 4*ctxSet + c1);
        numGreater1Flag++;
        AbsTransCoeffLevel[numNonZeroCount-1-n] += coeffAbsLevelG1Flag;
        if(coeffAbsLevelG1Flag)
        {
          c1 = 0;
          lastGreater1ScanPos = n;
          n--;
          break;
        }
        else if((c1 < 3) && (c1 > 0))
        {
          c1++;
        }
      }

      for( ; (numGreater1Flag < 8) && n >= 0; n--)
      {
        coeffAbsLevelG1Flag = GetCoeffAbsLevelGreater1FlagLuma(Parser_p, ctxSet << 2);
        numGreater1Flag++;
        AbsTransCoeffLevel[numNonZeroCount-1-n] += coeffAbsLevelG1Flag;
      }
      signHidden = (lastSigScanPos - firstSigScanPos > 3) && !transQuantBypass && sign_data_hiding_flag;
      coeffAbsLevelG2Flag = 0;
      if( lastGreater1ScanPos != -1)
      {
        coeffAbsLevelG2Flag = GetCoeffAbsLevelGreater2FlagLuma(Parser_p, ctxSet);
        AbsTransCoeffLevel[numNonZeroCount-1-lastGreater1ScanPos] += coeffAbsLevelG2Flag;
      }

      for(n = numNonZeroCount-1; n > 0; n--)
      {
        coeff_sign_flag[numNonZeroCount - 1 - n] = GetCoeffSignFlag(Parser_p);
      }
      coeff_sign_flag[numNonZeroCount - 1] = signHidden ? 0 : GetCoeffSignFlag(Parser_p);


      numSigCoeff = 0;
      sumAbsLevel = 0;
      golombRiceParam = 0;

      for(n = numNonZeroCount - 1; n > 0; n--)
      {
        numNonZeroM1MN = numNonZeroCount - 1 - n;
        baseLevel = ((numSigCoeff < 8) ? ((n == lastGreater1ScanPos) ? 3 : 2 ) : 1 );
        if( AbsTransCoeffLevel[numNonZeroM1MN] ==  baseLevel)
        {
          coeffAbsLevelRemain = GetCoeffAbsLevelRemaining(Parser_p, golombRiceParam);
          AbsTransCoeffLevel[numNonZeroM1MN] += coeffAbsLevelRemain;
          if(AbsTransCoeffLevel[numNonZeroM1MN] > (uint32)(3 *(1 << golombRiceParam)))
          {
            golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
          }
        }
        TransCoeffLevel = AbsTransCoeffLevel[numNonZeroM1MN] * (1 - (coeff_sign_flag[numNonZeroM1MN] << 1));

        // Store transform coefficients
        pos = sigCoeffPos[numNonZeroM1MN];
        baseTr[((pos >> 2) << log2TrafoSize) + (pos & 0x03)] = (sint16)TransCoeffLevel;
        numSigCoeff++;
      }

      numNonZeroM1MN = numNonZeroCount - 1;
      baseLevel = ((numSigCoeff < 8) ? ((n == lastGreater1ScanPos) ? 3 : 2 ) : 1 );
      if( AbsTransCoeffLevel[numNonZeroM1MN] ==  baseLevel)
      {
        coeffAbsLevelRemain = GetCoeffAbsLevelRemaining(Parser_p, golombRiceParam);
        AbsTransCoeffLevel[numNonZeroM1MN] += coeffAbsLevelRemain;
        if(AbsTransCoeffLevel[numNonZeroM1MN] > (uint32)(3 *(1 << golombRiceParam)))
        {
          golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
        }
      }
      TransCoeffLevel = AbsTransCoeffLevel[numNonZeroM1MN] * (1 - (coeff_sign_flag[numNonZeroM1MN] << 1));
      if(signHidden)
      {
        for(n = numNonZeroCount - 1; n >= 0; n--)
        {
          sumAbsLevel += AbsTransCoeffLevel[numNonZeroM1MN-n];
        }
        if((sumAbsLevel % 2) == 1)
        {
          TransCoeffLevel = -1*TransCoeffLevel;
        }
      }

      pos = sigCoeffPos[numNonZeroM1MN];
      baseTr[((pos >> 2) << log2TrafoSize) + (pos & 0x03)] = (sint16)TransCoeffLevel;
    }
  }
}


/******************************************************************************
*
* Name:        Parse_residual_coding_chroma
*
* Parameters:  Sessiondata_p     [in/out]  The decoder
*              log2TrafoSize     [in]      log size of the TU in pixels
*              scanIdx           [in]      coefficient scan index
*              transQuantBypass  [in]      transform & Quantization bypass flag
*              TransformSkipFlag [in]      transform skip flag
* Returns:     -
*
* Description: Decodes chroma residuals of a TU
*
******************************************************************************/
void
Parse_residual_coding_chroma(
#if MULTI_THREADED
               ThreadData_t  *Threaddata_p,
#else
               SessionData_t *Sessiondata_p,
#endif
               const uint32 log2TrafoSize,
               const uint8 scanIdx,
               const uint8 transQuantBypass,
               uint8 *TransformSkipFlag
               )
{
  const static uint32 MinInGroup[ 10 ] = {0, 1, 2, 3, 4, 6, 8, 12, 16, 24};

#if MULTI_THREADED
  SessionData_t *Sessiondata_p  = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
  sint16        *TransformArray = Threaddata_p->TransformArray;
#else
  Parser_t *Parser_p       = &Sessiondata_p->Parser;
  sint16   *TransformArray = Sessiondata_p->TransformArray;
#endif
  const uint32 log2TrafoSizeM2 = log2TrafoSize - 2;
  const uint32 subblocksize = 1 << log2TrafoSizeM2;
  const sint32 *bigscan = SCAN_ORDER[scanIdx][log2TrafoSizeM2];
  const sint32 *scan    = SCAN_ORDER[scanIdx][2];
  const uint8 SigCoeffContextOffset = (log2TrafoSize == 3) ? 9 : 12;
  const uint8 sign_data_hiding_flag       = Sessiondata_p->CurrentPictureParameterSet->SignDataHidingFlag;
  const uint8 transform_skip_enabled_flag = Sessiondata_p->CurrentPictureParameterSet->TransformSkipFlag;

  uint32 LastSignificantCoeffX;
  uint32 LastSignificantCoeffY;
  uint32 LastSignificantCoeffXprefix;
  uint32 LastSignificantCoeffYprefix;
  sint32 lastSubBlock;
  sint32 subblockScanPos;
  sint32 innerBlockScanPos;
  uint32 firstSigScanPos;
  uint32 lastSigScanPos;
  sint32 lastGreater1ScanPos;
  uint32 sigCoeffPos[16];
  uint32 sigCoeffScanPos[16];
  uint32 AbsTransCoeffLevel[16];
  uint32 coeffAbsLevelRemain;
  sint32 TransCoeffLevel;
  uint32 sumAbsLevel;
  uint32 baseLevel;
  uint32 xS;
  uint32 yS;
  uint32 pos;
  uint32 bigpos;
  sint32 i;
  sint16 *baseTr;

  uint8 coded_sub_block_flag[16];
  uint8 coeff_sign_flag[16];
  uint8 inferSbDcSigCoeffFlag;
  uint8 sigCoeffFlag;
  uint8 coeffAbsLevelG1Flag;
  uint8 coeffAbsLevelG2Flag;
  uint8 numGreater1Flag;
  uint8 lastScanPos;
  uint8 SubFlagAndSigCoeffContextOffset;
  uint8 c1 = 1;
  uint8 ctxSet;
  uint8 golombRiceParam;
  uint8 signHidden;
  uint8 numSigCoeff;
  uint8 numNonZeroCount;
  sint8 n;

  memset(coded_sub_block_flag, 0, 16 * sizeof(coded_sub_block_flag[0]));

  if(transform_skip_enabled_flag && !transQuantBypass && (log2TrafoSize == 2))
  {
    *TransformSkipFlag = GetTrSkipChromaFlag(Parser_p);
  }
  else
  {
    *TransformSkipFlag = 0;
  }

  LastSignificantCoeffXprefix = GetLastSigCoeffPrefixXChroma(Parser_p, log2TrafoSize);
  LastSignificantCoeffYprefix = GetLastSigCoeffPrefixYChroma(Parser_p, log2TrafoSize);

  if(LastSignificantCoeffXprefix > 3)
  {
    LastSignificantCoeffX  = MinInGroup[LastSignificantCoeffXprefix];
    LastSignificantCoeffX += GetLastSigCoeffSuffix(Parser_p, LastSignificantCoeffXprefix);
  }
  else
  {
    LastSignificantCoeffX = LastSignificantCoeffXprefix;
  }

  if(LastSignificantCoeffYprefix > 3)
  {
    LastSignificantCoeffY  = MinInGroup[LastSignificantCoeffYprefix];
    LastSignificantCoeffY += GetLastSigCoeffSuffix(Parser_p, LastSignificantCoeffYprefix);
  }
  else
  {
    LastSignificantCoeffY = LastSignificantCoeffYprefix;
  }

  if(scanIdx == 2)
  {
    uint32 LastSignificantCoeffTmp = LastSignificantCoeffY;
    LastSignificantCoeffY = LastSignificantCoeffX;
    LastSignificantCoeffX = LastSignificantCoeffTmp;
  }

  subblockScanPos   = ((LastSignificantCoeffY >> 2) << (log2TrafoSize - 2)) + (LastSignificantCoeffX >> 2);
  innerBlockScanPos = ((LastSignificantCoeffY & 0x03) << 2) + (LastSignificantCoeffX & 0x03);

  lastSubBlock = subblocksize*subblocksize;
  while(bigscan[--lastSubBlock] != subblockScanPos) { }

  lastScanPos = 16;
  while(scan[--lastScanPos] != innerBlockScanPos) { }

  for(i = lastSubBlock; i >= 0; i--) 
  {
    bigpos = bigscan[i];
    yS  = bigpos >> log2TrafoSizeM2;
    xS  = bigpos - (yS << log2TrafoSizeM2);
    baseTr = TransformArray + (yS << (log2TrafoSize + 2)) + (xS << 2);

    SubFlagAndSigCoeffContextOffset = 0;

    if(xS < (subblocksize-1))
    {
      SubFlagAndSigCoeffContextOffset += coded_sub_block_flag[bigpos + 1];
    }
    if(yS < (subblocksize- 1))
    {
      SubFlagAndSigCoeffContextOffset += (coded_sub_block_flag[bigpos + subblocksize] << 1);
    }

    if(i < lastSubBlock && i > 0) 
    {
      coded_sub_block_flag[bigpos] = GetCodedSubBlockFlagChroma(Parser_p,(SubFlagAndSigCoeffContextOffset > 0));
      inferSbDcSigCoeffFlag = 1;
    }
    else
    {
      coded_sub_block_flag[bigpos] = 1;
      inferSbDcSigCoeffFlag        = 0;
    }

    numNonZeroCount = 0;

    if(i == lastSubBlock)
    {
      pos = scan[lastScanPos];
      sigCoeffPos[numNonZeroCount]        = pos;
      sigCoeffScanPos[numNonZeroCount]    = lastScanPos;
      AbsTransCoeffLevel[numNonZeroCount] = 1;
      numNonZeroCount++;

      for(n = lastScanPos-1; n >= 0; n--)
      {
        pos = scan[n];
        sigCoeffFlag = GetSigCoeffFlagChroma(Parser_p, log2TrafoSize, i, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
        if(sigCoeffFlag)
        {
          sigCoeffPos[numNonZeroCount]        = pos;
          sigCoeffScanPos[numNonZeroCount]    = n;
          AbsTransCoeffLevel[numNonZeroCount] = 1;
          numNonZeroCount++;
        }
      }
    }
    else
    {
      if(coded_sub_block_flag[bigpos])
      {
        //handle positions 15 through 1
        for(n = 15; n > 0; n--)
        {
          pos = scan[n];
          sigCoeffFlag = GetSigCoeffFlagChroma(Parser_p,log2TrafoSize, i, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
          if(sigCoeffFlag)
          {
            inferSbDcSigCoeffFlag=0;
            sigCoeffPos[numNonZeroCount]        = pos;
            sigCoeffScanPos[numNonZeroCount]    = n;
            AbsTransCoeffLevel[numNonZeroCount] = 1;
            numNonZeroCount++;
          }
        }
        //handle n==0 which sometimes infers value
        pos=scan[0];
        if(!inferSbDcSigCoeffFlag)
        {
          sigCoeffFlag = GetSigCoeffFlagChroma(Parser_p,log2TrafoSize, i, pos, SubFlagAndSigCoeffContextOffset, SigCoeffContextOffset);
          if(sigCoeffFlag)
          {
            sigCoeffPos[numNonZeroCount]        = pos;
            sigCoeffScanPos[numNonZeroCount]    = 0;
            AbsTransCoeffLevel[numNonZeroCount] = 1;
            numNonZeroCount++;
          }
        }
        else
        {
          sigCoeffPos[numNonZeroCount]        = pos;
          sigCoeffScanPos[numNonZeroCount]    = 0;
          AbsTransCoeffLevel[numNonZeroCount] = 1;
          numNonZeroCount++;
        }
      }
    }

    if(numNonZeroCount)
    {
      firstSigScanPos     = sigCoeffScanPos[numNonZeroCount-1];
      lastSigScanPos      = sigCoeffScanPos[0];
      numGreater1Flag     = 0;
      lastGreater1ScanPos = -1;
      ctxSet = 0;
      if(c1 == 0)
      {
        ctxSet++;
      }
      c1 = 1;
      for(n = numNonZeroCount - 1; (numGreater1Flag < 8) && n >= 0; n--)
      {
        coeffAbsLevelG1Flag = GetCoeffAbsLevelGreater1FlagChroma(Parser_p, 4 * ctxSet + c1);
        numGreater1Flag++;
        AbsTransCoeffLevel[numNonZeroCount-1-n] += coeffAbsLevelG1Flag;
        if(coeffAbsLevelG1Flag)
        {
          c1 = 0;
          lastGreater1ScanPos = n;
          n--;
          break;
        }
        else if((c1 < 3) && (c1 > 0))
        {
          c1++;
        }
      }

      for( ; (numGreater1Flag < 8) && n >= 0; n--)
      {
        coeffAbsLevelG1Flag = GetCoeffAbsLevelGreater1FlagChroma(Parser_p, 4 * ctxSet);
        numGreater1Flag++;
        AbsTransCoeffLevel[numNonZeroCount-1-n] += coeffAbsLevelG1Flag;
      }
      signHidden = sign_data_hiding_flag && !transQuantBypass && (lastSigScanPos - firstSigScanPos > 3);
      coeffAbsLevelG2Flag = 0;
      if( lastGreater1ScanPos != -1)
      {
        coeffAbsLevelG2Flag = GetCoeffAbsLevelGreater2FlagChroma(Parser_p, ctxSet);
        AbsTransCoeffLevel[numNonZeroCount-1-lastGreater1ScanPos] += coeffAbsLevelG2Flag;
      }

      for(n = numNonZeroCount-1; n > 0; n--)
      {
        coeff_sign_flag[numNonZeroCount-1-n] = GetCoeffSignFlag(Parser_p);
      }
      if(!signHidden)
      {
        coeff_sign_flag[numNonZeroCount-1-n] = GetCoeffSignFlag(Parser_p);
      }
      else
      {
        coeff_sign_flag[numNonZeroCount-1] = 0;
      }

      numSigCoeff = 0;
      sumAbsLevel = 0;
      golombRiceParam = 0;
      for(n = numNonZeroCount-1; n > 0; n--)
      {
        baseLevel = ((numSigCoeff < 8) ? ((n == lastGreater1ScanPos) ? 3 : 2 ) : 1 );
        if( AbsTransCoeffLevel[numNonZeroCount-1-n] ==  baseLevel)
        {
          coeffAbsLevelRemain = GetCoeffAbsLevelRemaining(Parser_p, golombRiceParam);
          AbsTransCoeffLevel[numNonZeroCount-1-n] += coeffAbsLevelRemain;
          if(AbsTransCoeffLevel[numNonZeroCount-1-n] > (uint32)(3 *(1 << golombRiceParam)))
          {
            golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
          }
        }
        TransCoeffLevel = AbsTransCoeffLevel[numNonZeroCount-1-n] * (1 - 2 * coeff_sign_flag[numNonZeroCount-1-n]);

        // Store transform coefficients
        pos = sigCoeffPos[numNonZeroCount-1-n];
        baseTr[((pos >> 2) << log2TrafoSize) + (pos & 0x03)] = (sint16)TransCoeffLevel;
        numSigCoeff++;
      }

      baseLevel = ((numSigCoeff < 8) ? ((0 == lastGreater1ScanPos) ? 3 : 2 ) : 1 );
      if( AbsTransCoeffLevel[numNonZeroCount-1] == baseLevel)
      {
        coeffAbsLevelRemain = GetCoeffAbsLevelRemaining(Parser_p, golombRiceParam);
        AbsTransCoeffLevel[numNonZeroCount-1] += coeffAbsLevelRemain;
        if(AbsTransCoeffLevel[numNonZeroCount-1] > (uint32)(3 *(1 << golombRiceParam)))
        {
          golombRiceParam = D65_MIN(golombRiceParam + 1, 4);
        }
      }
      TransCoeffLevel = AbsTransCoeffLevel[numNonZeroCount-1] * (1 - 2 * coeff_sign_flag[numNonZeroCount-1]);
      if(signHidden)
      {
        for(n = numNonZeroCount-1; n >= 0; n--)
        {
          sumAbsLevel += AbsTransCoeffLevel[numNonZeroCount-1-n];
        }
        if((sumAbsLevel % 2) == 1)
        {
          TransCoeffLevel = -1*TransCoeffLevel;
        }
      }
      // Store transform coefficients
      pos = sigCoeffPos[numNonZeroCount-1];
      baseTr[((pos >> 2) << log2TrafoSize) + (pos & 0x03)] = (sint16)TransCoeffLevel;
      numSigCoeff++;
    }
  }
}


void writeChromaComponent(SessionData_t *Sessiondata_p, YUVChannel channel, uint32 log2TrafoSize, uint32 ScanIdx, uint32 x0, uint32 y0, uint32 TopLeftAddr)
{
  /*
  calculateDifference(&Sessiondata_p->sourceYUV, &Sessiondata_p->CurrentRecImage->Image, Sessiondata_p->difference, x0, y0, 1<<log2TrafoSize, channel);
  partialTransform(Sessiondata_p->difference,Sessiondata_p->coefficients,log2TrafoSize);
  performQuantization(Sessiondata_p->difference,Sessiondata_p->coefficients,log2TrafoSize,Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpPrimeY,Sessiondata_p->CurrentSliceType);

  writeChroma(Sessiondata_p, Sessiondata_p->coefficients,log2TrafoSize,ScanIdx);
  performDequantization((sint16*)Sessiondata_p->reconstructedResiduals,log2TrafoSize,Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpPrimeY);
  partialInverse(Sessiondata_p->coefficients,Sessiondata_p->reconstructedResiduals,1<<log2TrafoSize);
  */
}

/******************************************************************************
*
* Name:        Parse_transform_unit
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      x position of TU in pixel coordinates
*              y0              [in]      y position of TU in pixel coordinates
*              log2TrafoSize   [in]      log size of the TU in pixels
*              blkIdx          [in]      block index
*              cbf             [in]      coded block flag
* Returns:     -
*
* Description: Decodes a transform unit (TU) 
*
******************************************************************************/
void
Parse_transform_unit(
#if MULTI_THREADED
           ThreadData_t *Threaddata_p,
#else
           SessionData_t *Sessiondata_p,
#endif
           const uint32 x0,
           const uint32 y0,
           const uint32 xBase,
           const uint32 yBase,
           const uint32 log2TrafoSize,
           const uint32 blkIdx,
           const uint32 cbf)
{
  static const sint32 CHROMA_QP_FROM_LUMA_QP[70] = 
  {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 
    36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63
  };

#if MULTI_THREADED
  SessionData_t *Sessiondata_p    = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p       = &Threaddata_p->Parser;
  sint16        *TransformArray_p = Threaddata_p->TransformArray;
  QuantGroupData_t *QgData_p      = &Threaddata_p->QgData;
#if GT_RESIDUAL_RECONSTRUCT
  sint16        *ResTrQuant_p      = Threaddata_p->ResTrQuantArray;
#endif
#else
  sint16        *TransformArray_p = Sessiondata_p->TransformArray;
  QuantGroupData_t *QgData_p      = &Sessiondata_p->QgData;
  Parser_t         *Parser_p      = &Sessiondata_p->Parser;
#if GT_RESIDUAL_RECONSTRUCT
  sint16        *ResTrQuant_p      = Sessiondata_p->ResTrQuantArray;
#endif
#endif

  const uint32 PicWidthInPU       = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const sint32 TrafoSize          = 1 << log2TrafoSize;
  const uint32 TrafoSizeInPU      = TrafoSize >> Log2PUSizeInPixels;
  const uint32 TopLeftAddr        = (y0 >> Log2PUSizeInPixels) * PicWidthInPU + (x0 >> Log2PUSizeInPixels);
  const uint8  TransQuantBypass   = Sessiondata_p->PredictionUnitArray[TopLeftAddr].TransQuantBypass;
  sint32 width;
  uint32 y;
  uint32 x;
  uint8 *ptr;
  uint8  PredMode = Sessiondata_p->PredictionUnitArray[TopLeftAddr].PredMode;
  uint8  TransformSkipFlag;
  uint8  IntraPredMode;
  uint8  DST = 0;
  uint8  ScanIdx;
  sint32 CuQpDeltaVal = 0;

#if MULTI_THREADED
  if(log2TrafoSize > 5 || log2TrafoSize < 2)
  {
    BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "TU size exceeds allowed range");
  }
#else
  if(log2TrafoSize > 5 || log2TrafoSize < 2)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "TU size exceeds allowed range");
  }
#endif

  //set transform unit size and Cbf luma flag
  memset(Sessiondata_p->EdgeFlagsHor + TopLeftAddr, 0x02, TrafoSizeInPU * sizeof(uint8));
  for(y = 0; y < TrafoSizeInPU; y++)
  {
    Sessiondata_p->EdgeFlags[TopLeftAddr + y*PicWidthInPU] = 0x02;
    for(x = 0; x < TrafoSizeInPU; x++)
    {
      Sessiondata_p->PredictionUnitArray[TopLeftAddr + y*PicWidthInPU + x].CbfLuma = ((cbf & 0x04) > 0);

    }
  }

  if(PredMode == NOT_DECODED)
  {
    PredMode = MODE_INTRA;
    // Set predmode
    for(y = 0; y < TrafoSizeInPU; y++)
    {
      for(x = 0; x < TrafoSizeInPU; x++)
      {
        Sessiondata_p->PredictionUnitArray[TopLeftAddr + y*PicWidthInPU + x].PredMode = MODE_INTRA;
      }
    }

    IntraPredictionLuma(Sessiondata_p, x0, y0, log2TrafoSize);
    if(log2TrafoSize > 2)
    {
      IntraPredictionChroma(Sessiondata_p, x0 >> 1, y0 >> 1, log2TrafoSize - 1);
    }
    else
    {
      DST = 1;
      if(blkIdx == 0)
      {
        IntraPredictionChroma(Sessiondata_p, x0 >> 1, y0 >> 1, 2);
      }
    }
  }

  if(cbf & 0x07)
  {
    if(Sessiondata_p->CurrentPictureParameterSet->CuQpDeltaEnabledFlag && !QgData_p->IsCuQpDataCoded)
    {
      uint32 CuQpDeltaAbs = GetCuQpDeltaAbs(Parser_p);
      if(CuQpDeltaAbs)
      {
        CuQpDeltaVal = GetCuQpDeltaSignFlag(Parser_p) ? (-1) * (sint32)CuQpDeltaAbs : (sint32)CuQpDeltaAbs;
      }
      QgData_p->IsCuQpDataCoded = 1;
      QgData_p->CuQpDeltaVal    = CuQpDeltaVal;
      UpdateQp(Sessiondata_p, QgData_p, xBase, yBase);
    }
  }

  if(cbf & 0x04)
  {
    // Set TransformArray to 0
    memset(TransformArray_p, 0, sizeof(sint16)*(TrafoSize)*(TrafoSize));
    ScanIdx = 0;
    if(PredMode == MODE_INTRA && log2TrafoSize < 4)
    {
      IntraPredMode = Sessiondata_p->PredictionUnitArray[TopLeftAddr].IntraPredMode;
      if(IntraPredMode > 5 && IntraPredMode < 15)
      {
        ScanIdx = 2;
      }
      else if(IntraPredMode > 21 && IntraPredMode < 31)
      {
        ScanIdx = 1;
      }
    }
    // Parse Luma coefficients
#if MULTI_THREADED
    Parse_residual_coding_luma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
    Parse_residual_coding_luma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif

#if GT_RESIDUAL_PRUNING
#if MULTI_THREADED
    Encode_residual_dc_luma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
    Encode_residual_dc_luma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif
#endif

#if GT_RESIDUAL_RECONSTRUCT
    if (Parser_p->SubEncoder.OutputMode == 1)
    {
      uint32 oWidth = Sessiondata_p->SourceYUV.Y_Width_image;
      uint32 pWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;

      uint8 *pSrc   = Sessiondata_p->SourceYUV.Y + y0 * oWidth + x0;
      uint8 *pPre   = Sessiondata_p->CurrentRecImage->Image.Y + (y0 + PADDING) * pWidth + (x0 + PADDING);

      CalculateResiduals(pSrc, pPre, ResTrQuant_p, oWidth, pWidth, TrafoSize);
      PerformTransform(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, TransformSkipFlag, DST);
      performQuantization(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpPrimeY, Sessiondata_p->CurrentSliceType);
#if MULTI_THREADED
      Encode_residual_luma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#else
      Encode_residual_luma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#endif
      memcpy(TransformArray_p, ResTrQuant_p, sizeof(sint16)*(TrafoSize)*(TrafoSize));
    }
#endif

    width = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    ptr   = &(Sessiondata_p->CurrentRecImage->Image.Y[(y0+PADDING)*width + x0 + PADDING]);
    if(!TransQuantBypass)
    {
      // Dequantize luma block
      performDequantization(TransformArray_p, log2TrafoSize, Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpPrimeY);
      // Transform luma block
      performInverseTransform(TransformArray_p, TrafoSize, TransformSkipFlag, DST);
    }
    performReconstruction(TransformArray_p, ptr, TrafoSize, width);
  }

  if(cbf & 0x03)
  {
    ScanIdx = 0;
    if(PredMode == MODE_INTRA && log2TrafoSize <= 3)
    {
      IntraPredMode = Sessiondata_p->PredictionUnitArray[TopLeftAddr].intraChromaPredMode;
      if(IntraPredMode > 5 && IntraPredMode < 15)
      {
        ScanIdx = 2;
      }
      else if(IntraPredMode > 21 && IntraPredMode < 31)
      {
        ScanIdx = 1;
      }
    }
  }

  if(log2TrafoSize > 2)
  {
    if(cbf & 0x02)
    {
      // Set TransformArray to 0
      memset(TransformArray_p, 0, sizeof(sint16)*(TrafoSize >> 1)*(TrafoSize >> 1));
      // Parse Cb coefficients
#if MULTI_THREADED
      Parse_residual_coding_chroma(Threaddata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Parse_residual_coding_chroma(Sessiondata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif

#if GT_RESIDUAL_PRUNING
#if MULTI_THREADED
      Encode_residual_dc_chroma(Threaddata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Encode_residual_dc_chroma(Sessiondata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif
#endif

#if GT_RESIDUAL_RECONSTRUCT
    if (Parser_p->SubEncoder.OutputMode == 1)
    {
      uint32 oWidth = Sessiondata_p->SourceYUV.C_Width_image;
      uint32 pWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;

      uint8 *pSrc   = Sessiondata_p->SourceYUV.Cb + oWidth * (y0 >> 1) + (x0 >> 1);
      uint8 *pPre   = Sessiondata_p->CurrentRecImage->Image.Cb + pWidth * ((y0 + PADDING) >> 1) + ((x0 + PADDING) >> 1);

      CalculateResiduals(pSrc, pPre, ResTrQuant_p, oWidth, pWidth, (TrafoSize >> 1));
      PerformTransform(ResTrQuant_p, ResTrQuant_p, log2TrafoSize - 1, TransformSkipFlag, 0);
      performQuantization(ResTrQuant_p, ResTrQuant_p, log2TrafoSize - 1, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY], Sessiondata_p->CurrentSliceType);
#if MULTI_THREADED
      Encode_residual_chroma(Threaddata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, TransformSkipFlag);
#else
      Encode_residual_chroma(Sessiondata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, TransformSkipFlag);
#endif
      memcpy(TransformArray_p, ResTrQuant_p, sizeof(sint16)*(TrafoSize >> 1)*(TrafoSize >> 1));
    }
#endif

      if(!TransQuantBypass)
      {

        // Dequantize Cb block
        performDequantization(TransformArray_p, log2TrafoSize - 1, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY]);
        // Transform Cb block
        performInverseTransform(TransformArray_p, TrafoSize >> 1, TransformSkipFlag, 0);
      }
      width = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
      ptr   = &(Sessiondata_p->CurrentRecImage->Image.Cb[((y0 + PADDING) >>1) * width + ((x0 + PADDING) >> 1)]);
      performReconstruction(TransformArray_p, ptr, TrafoSize >> 1, width);
    }

    if(cbf & 0x01)
    {
      // Set TransformArray to 0
      memset(TransformArray_p, 0, sizeof(sint16)*(TrafoSize >> 1)*(TrafoSize >> 1));
      // Parse Cr coefficients
#if MULTI_THREADED
      Parse_residual_coding_chroma(Threaddata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Parse_residual_coding_chroma(Sessiondata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif

#if GT_RESIDUAL_PRUNING
#if MULTI_THREADED
      Encode_residual_dc_chroma(Threaddata_p, log2TrafoSize - 1,ScanIdx,TransQuantBypass,&TransformSkipFlag);
#else
      Encode_residual_dc_chroma(Sessiondata_p,log2TrafoSize - 1,ScanIdx,TransQuantBypass,&TransformSkipFlag);
#endif
#endif

#if GT_RESIDUAL_RECONSTRUCT
    if (Parser_p->SubEncoder.OutputMode == 1)
    {
      uint32 oWidth = Sessiondata_p->SourceYUV.C_Width_image;
      uint32 pWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;

      uint8 *pSrc   = Sessiondata_p->SourceYUV.Cr + oWidth * (y0 >> 1) + (x0 >> 1);
      uint8 *pPre   = Sessiondata_p->CurrentRecImage->Image.Cr + pWidth * ((y0 + PADDING) >> 1) + ((x0 + PADDING) >> 1);

      CalculateResiduals(pSrc, pPre, ResTrQuant_p, oWidth, pWidth, (TrafoSize >> 1));
      PerformTransform(ResTrQuant_p, ResTrQuant_p, log2TrafoSize - 1, TransformSkipFlag, 0);
      performQuantization(ResTrQuant_p, ResTrQuant_p, log2TrafoSize - 1, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY], Sessiondata_p->CurrentSliceType);
#if MULTI_THREADED
      Encode_residual_chroma(Threaddata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, TransformSkipFlag);
#else
      Encode_residual_chroma(Sessiondata_p, log2TrafoSize - 1, ScanIdx, TransQuantBypass, TransformSkipFlag);
#endif
      memcpy(TransformArray_p, ResTrQuant_p, sizeof(sint16)*(TrafoSize >> 1)*(TrafoSize >> 1));
    }
#endif

      if(!TransQuantBypass)
      {
        // Dequantize Cr block
        performDequantization(TransformArray_p, log2TrafoSize - 1, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY]);
        // Transform Cr block
        performInverseTransform(TransformArray_p, TrafoSize >> 1, TransformSkipFlag, 0);
      }
      width = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
      ptr   = &(Sessiondata_p->CurrentRecImage->Image.Cr[((y0 + PADDING) >> 1)*width + ((x0 + PADDING) >> 1)]);
      performReconstruction(TransformArray_p, ptr, TrafoSize >> 1, width);
    }
  }
  else if(blkIdx == 3)
  {
    if(cbf & 0x02)
    {
      // Set TransformArray to 0
      memset(TransformArray_p, 0, sizeof(sint16)*(TrafoSize)*(TrafoSize));
      // Parse transform coefficients
#if MULTI_THREADED
      Parse_residual_coding_chroma(Threaddata_p, log2TrafoSize,ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Parse_residual_coding_chroma(Sessiondata_p, log2TrafoSize,ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif

#if GT_RESIDUAL_PRUNING
#if MULTI_THREADED
      Encode_residual_dc_chroma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Encode_residual_dc_chroma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif
#endif

#if GT_RESIDUAL_RECONSTRUCT
    if (Parser_p->SubEncoder.OutputMode == 1)
    {
      uint32 oWidth = Sessiondata_p->SourceYUV.C_Width_image;
      uint32 pWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;

      uint8 *pSrc   = Sessiondata_p->SourceYUV.Cb + oWidth * ((y0 - TrafoSize) >> 1) + ((x0 - TrafoSize) >> 1);
      uint8 *pPre   = Sessiondata_p->CurrentRecImage->Image.Cb + pWidth * ((y0 - TrafoSize + PADDING) >> 1) + ((x0 - TrafoSize + PADDING) >> 1);

      CalculateResiduals(pSrc, pPre, ResTrQuant_p, oWidth, pWidth, TrafoSize);
      PerformTransform(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, TransformSkipFlag, 0);
      performQuantization(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY], Sessiondata_p->CurrentSliceType);
#if MULTI_THREADED
      Encode_residual_chroma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#else
      Encode_residual_chroma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#endif
      memcpy(TransformArray_p, ResTrQuant_p, sizeof(sint16)*TrafoSize*TrafoSize);
    }
#endif


      if(!TransQuantBypass)
      {
        // Dequantize Cb block
        performDequantization(TransformArray_p, log2TrafoSize, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY]);
        // Transform Cb block
        performInverseTransform(TransformArray_p, TrafoSize, TransformSkipFlag, 0);
      }
      width = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
      ptr   = &(Sessiondata_p->CurrentRecImage->Image.Cb[((y0 - TrafoSize + PADDING) >> 1)*width + ((x0 - TrafoSize + PADDING) >> 1)]);
      performReconstruction(TransformArray_p, ptr, TrafoSize, width);
    }

    if(cbf & 0x01)
    {
      // Set TransformArray to 0
      memset(TransformArray_p, 0, sizeof(sint16)*(TrafoSize)*(TrafoSize));

      // Parse transform coefficients
#if MULTI_THREADED
      Parse_residual_coding_chroma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Parse_residual_coding_chroma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif

#if GT_RESIDUAL_PRUNING
#if MULTI_THREADED
      Encode_residual_dc_chroma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#else
      Encode_residual_dc_chroma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, &TransformSkipFlag);
#endif
#endif

#if GT_RESIDUAL_RECONSTRUCT
    if (Parser_p->SubEncoder.OutputMode == 1)
    {
      uint32 oWidth = Sessiondata_p->SourceYUV.C_Width_image;
      uint32 pWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;

      uint8 *pSrc   = Sessiondata_p->SourceYUV.Cr + oWidth * ((y0 - TrafoSize) >> 1) + ((x0 - TrafoSize) >> 1);
      uint8 *pPre   = Sessiondata_p->CurrentRecImage->Image.Cr + pWidth * ((y0 - TrafoSize + PADDING) >> 1) + ((x0 - TrafoSize + PADDING) >> 1);

      CalculateResiduals(pSrc, pPre, ResTrQuant_p, oWidth, pWidth, TrafoSize);
      PerformTransform(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, TransformSkipFlag, 0);
      performQuantization(ResTrQuant_p, ResTrQuant_p, log2TrafoSize, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY], Sessiondata_p->CurrentSliceType);
#if MULTI_THREADED
      Encode_residual_chroma(Threaddata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#else
      Encode_residual_chroma(Sessiondata_p, log2TrafoSize, ScanIdx, TransQuantBypass, TransformSkipFlag);
#endif
      memcpy(TransformArray_p, ResTrQuant_p, sizeof(sint16)*TrafoSize*TrafoSize);
    }
#endif

      if(!TransQuantBypass)
      {
        // Dequantize Cr block
        performDequantization(TransformArray_p, log2TrafoSize, CHROMA_QP_FROM_LUMA_QP[Sessiondata_p->PredictionUnitArray[TopLeftAddr].QpY]);
        // Transform Cr block
        performInverseTransform(TransformArray_p, TrafoSize, TransformSkipFlag, 0);
      }
      width = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
      ptr   = &(Sessiondata_p->CurrentRecImage->Image.Cr[((y0 - TrafoSize + PADDING) >> 1)*width + ((x0 - TrafoSize + PADDING) >> 1)]);
      performReconstruction(TransformArray_p, ptr, TrafoSize, width);
    }
  }
}


/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        UpdateQp
*
******************************************************************************/
void
UpdateQp(SessionData_t    *Sessiondata_p,
     QuantGroupData_t *QgData_p,
     const uint32      xBase,
     const uint32      yBase)
{
  const uint32 Log2CtbSize = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const uint32 LogMinPUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 PicWidthInPu = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 PuXCurr = xBase >> LogMinPUSizeInPixels;
  const uint32 PuYCurr = yBase >> LogMinPUSizeInPixels;
  const uint32 PuCurr = PuYCurr * PicWidthInPu + PuXCurr;
  PredictionUnit_t *TopLeftPU_p = &Sessiondata_p->PredictionUnitArray[PuCurr];
  uint32 CuSizeInPuCurr = TopLeftPU_p->CUSize >> LogMinPUSizeInPixels;
  sint32 xQg = (sint32)(xBase - (xBase & ((1 << Sessiondata_p->Log2MinCuQpDeltaSize) - 1)));
  sint32 yQg = (sint32)(yBase - (yBase & ((1 << Sessiondata_p->Log2MinCuQpDeltaSize) - 1)));
  sint32 QpA;
  sint32 QpB;
  uint8  firstQg = 0;
  sint32 QpPrev;
  sint32 QpPred;
  sint32 QpY;
  sint32 QpPrimeY;
  uint32 x;
  uint32 y;
  PredictionUnit_t *Pu_p;

  if(xQg != QgData_p->QgX || yQg != QgData_p->QgY)
  {
    QgData_p->QgX     = xQg;
    QgData_p->QgY     = yQg;
    QgData_p->PrevQpY = QgData_p->CurrQpY;
  }

  if(Sessiondata_p->CurrentPictureParameterSet->TilesEnabledFlag == 1)
  {
    uint32 tileX;
    uint32 tileY;
    uint32 maxTilesX = Sessiondata_p->CurrentPictureParameterSet->num_tile_columns_minus1 + 1;
    uint32 maxTilesY = Sessiondata_p->CurrentPictureParameterSet->num_tile_rows_minus1    + 1;
    for(tileY = 0; tileY < maxTilesY; tileY++)
    {
      for(tileX = 0; tileX < maxTilesX; tileX++)
      {
        if((xQg == (sint32)(Sessiondata_p->colBd[tileX] << Log2CtbSize)) & 
          (yQg == (sint32)(Sessiondata_p->rowBd[tileY] << Log2CtbSize)))
        {
          firstQg = 1;
          break;
        }
      }
    }
  }
  else
  {
    firstQg = (xQg == 0) & (yQg == 0);
  }

  QpPrev = firstQg ? Sessiondata_p->CurrentQuant : QgData_p->PrevQpY;
  DeriveNbQg(Sessiondata_p, (sint32)xBase, (sint32)yBase, xQg - 1, yQg    , QpPrev, &QpA);
  DeriveNbQg(Sessiondata_p, (sint32)xBase, (sint32)yBase, xQg    , yQg - 1, QpPrev, &QpB);
  QpPred = (QpA + QpB + 1) >> 1;
  QpY    = (QpPred + QgData_p->CuQpDeltaVal + 52) % 52;
  QpPrimeY = QpY; // assuming bit depth is 8

  // Set current CU's QpY and QpPrimeY
  for(y = 0; y < CuSizeInPuCurr; y++)
  {
    Pu_p = TopLeftPU_p + y * PicWidthInPu;
    for(x = 0 ; x < CuSizeInPuCurr ; x++)
    {
      Pu_p->QpY        = QpY;
      Pu_p++->QpPrimeY = QpPrimeY;
    }
  }
  QgData_p->CurrQpY = QpY;
}

/******************************************************************************
*
* Name:        Parse_transform_tree
*
******************************************************************************/
void
Parse_transform_tree(
#if MULTI_THREADED
           ThreadData_t *Threaddata_p,
#else
           SessionData_t *Sessiondata_p,
#endif
           const uint32 x0,
           const uint32 y0,
           const uint32 xBase,
           const uint32 yBase,
           const uint32 log2TrafoSize,
           const uint32 trafoDepth, 
           const uint32 blkIdx,
           const uint32 MaxTrafoDepth,
           const uint32 IntraSplitFlag,
           const uint32 InterSplitFlag,
           const uint8 CbfChroma,
           const uint32 PredMode)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*) Threaddata_p->Sessiondata_p;
  Parser_t *Parser_p = &Threaddata_p->Parser;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
#endif
  const uint32 Log2MaxTrafoSize = Sessiondata_p->CurrentSequenceParameterSet->Log2MaxTrafoSize;
  const uint32 Log2MinTrafoSize = Sessiondata_p->CurrentSequenceParameterSet->Log2MinTrafoSize;
  uint32 split_transform_flag = 0;
  uint8 cbf_cb = 0;
  uint8 cbf_cr = 0;
  uint8 cbf_chroma = 0;
  uint8 cbf_luma;
  uint32 y1;
  uint32 x1;

  if((log2TrafoSize <= Log2MaxTrafoSize)&&(log2TrafoSize > Log2MinTrafoSize)&& (trafoDepth < MaxTrafoDepth)&& !(IntraSplitFlag && (trafoDepth == 0)) )
  {
    split_transform_flag = GetSplitTransformFlag(Parser_p, log2TrafoSize);
  }
  // Otherwise infer split_transform_flag
  else if(log2TrafoSize > Log2MaxTrafoSize)
  {
    split_transform_flag = 1;
  }
  else if(IntraSplitFlag == 1 && trafoDepth == 0)
  {
    split_transform_flag = 1;
  }
  else if(InterSplitFlag == 1)
  {
    split_transform_flag = 1;
  }
  // Add cases for interSplitFlag and IntraSplitFlag

  if(log2TrafoSize > 2 )
  {
    if(trafoDepth == 0 || (CbfChroma & 0x02))
    {
      cbf_cb = (uint8)GetCbfChroma(Parser_p, trafoDepth);
    }
    if(trafoDepth == 0 || (CbfChroma & 0x01))
    {
      cbf_cr = (uint8)GetCbfChroma(Parser_p, trafoDepth);
    }
  }
  else if(trafoDepth > 0)
  {
    cbf_cb = (CbfChroma & 0x02) > 0;
    cbf_cr = (CbfChroma & 0x01) > 0;
  }
  cbf_chroma = (cbf_cb << 1)|cbf_cr;
  if(split_transform_flag)
  {
    x1 = x0 + ((1 << log2TrafoSize) >> 1);
    y1 = y0 + ((1 << log2TrafoSize) >> 1);
#if MULTI_THREADED
    Parse_transform_tree(Threaddata_p, x0, y0, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 0, MaxTrafoDepth, IntraSplitFlag, 0, cbf_chroma, PredMode);
    Parse_transform_tree(Threaddata_p, x1, y0, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 1, MaxTrafoDepth, IntraSplitFlag, 0, cbf_chroma, PredMode);
    Parse_transform_tree(Threaddata_p, x0, y1, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 2, MaxTrafoDepth, IntraSplitFlag, 0, cbf_chroma, PredMode);
    Parse_transform_tree(Threaddata_p, x1, y1, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 3, MaxTrafoDepth, IntraSplitFlag, 0, cbf_chroma, PredMode);
#else
    Parse_transform_tree(Sessiondata_p, x0, y0, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 0, MaxTrafoDepth, IntraSplitFlag, 0,cbf_chroma, PredMode);
    Parse_transform_tree(Sessiondata_p, x1, y0, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 1, MaxTrafoDepth, IntraSplitFlag, 0,cbf_chroma, PredMode);
    Parse_transform_tree(Sessiondata_p, x0, y1, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 2, MaxTrafoDepth, IntraSplitFlag, 0,cbf_chroma, PredMode);
    Parse_transform_tree(Sessiondata_p, x1, y1, xBase, yBase, log2TrafoSize - 1, trafoDepth + 1, 3, MaxTrafoDepth, IntraSplitFlag, 0,cbf_chroma, PredMode);
#endif
  }
  else
  {
    uint8 cbf = 0;
    cbf_luma = 1;
    if(PredMode == MODE_INTRA || trafoDepth != 0 || (cbf_chroma & 0x3))
    {
      cbf_luma = (uint8)GetCbfLuma(Parser_p, trafoDepth);
    }
    cbf = (cbf_luma << 2) | (cbf_cb << 1) | cbf_cr;
#if MULTI_THREADED
    Parse_transform_unit (Threaddata_p, x0, y0, xBase, yBase, log2TrafoSize, blkIdx, cbf);
#else
    Parse_transform_unit (Sessiondata_p, x0, y0, xBase, yBase, log2TrafoSize, blkIdx, cbf);
#endif
  }
}