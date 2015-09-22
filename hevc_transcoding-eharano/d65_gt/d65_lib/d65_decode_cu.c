/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains CU/PU/TU decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include  <string.h>
#include "d65_decode_cu.h"
#include "d65_decode_pu.h"
#include "d65_decode_tu.h"
#include "d65_cabac.h"


/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        Parse_coding_unit
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      x position in pixel coordinates
*              y0              [in]      y position in pixel coordinates
*              log2CbSize      [in]      log size of the CU in pixels
*
* Returns:     -
*
* Description: Decodes a coding unit (CU)
*
******************************************************************************/
void
Parse_coding_unit(
#if MULTI_THREADED
                  ThreadData_t *Threaddata_p, 
#else
                  SessionData_t *Sessiondata_p, 
#endif
                  const uint32 x0, 
                  const uint32 y0, 
                  const uint32 log2CbSize,
                  const uint32 ctDepth) 
{
#if MULTI_THREADED
  SessionData_t    *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t         *Parser_p      = &Threaddata_p->Parser;
  QuantGroupData_t *QgData_p      = &Threaddata_p->QgData;
#else
  Parser_t         *Parser_p = &Sessiondata_p->Parser;
  QuantGroupData_t *QgData_p = &Sessiondata_p->QgData;
#endif
  const SequenceParameterSet_t *CurrSPS   = Sessiondata_p->CurrentSequenceParameterSet;

  const uint32 Log2MinCbSizeY       = CurrSPS->Log2MinCbSizeY;
  const uint32 LogMinPUSizeInPixels = CurrSPS->Log2MinUnitSize;
  const uint32 PicWidthPU           = CurrSPS->PicWidthInMinUnit;
  const uint32 AmpEnabledFlag       = CurrSPS->AmpEnabledFlag;

  const uint32 StartY               = y0 >> LogMinPUSizeInPixels;
  const uint32 StartX               = x0 >> LogMinPUSizeInPixels;
  const uint32 StartAddr            = StartY * PicWidthPU + StartX;

  const uint32 nCbS                 = 1 << log2CbSize;
  const uint32 NumberPUinThisCU     = nCbS >> LogMinPUSizeInPixels;
  const uint32 IsLeafCU             = (log2CbSize == Log2MinCbSizeY);

  PredictionUnit_t *TopLeftPU_p = Sessiondata_p->PredictionUnitArray + StartAddr;
  PredictionUnit_t *PU_p        = TopLeftPU_p;
  PredictionMode_t  PredMode    = MODE_INTRA;
  PartitionMode_t   PartMode    = PART_2Nx2N;

  uint32 TransQBypassFlag       = 0;
  uint32 SkipFlag               = 0;
  uint32 NoResiSyntaxFlag       = 0;
  uint32 IntraSplitFlag         = 0;
  uint32 InterSplitFlag         = 0;
  uint32 MaxTrafoDepth;
  uint32 y;
  uint32 x;

  if(log2CbSize < 3 || log2CbSize > 6)
  {
#if MULTI_THREADED
    BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "CU size out of range");
#else
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "CU size out of range");
#endif
  }

  if(Sessiondata_p->CurrentPictureParameterSet->transquant_bypass_enabled_flag)
  {
    TransQBypassFlag = GetCuTransquantBypassFlag(Parser_p);
  }

  if(Sessiondata_p->CurrentSliceType != MODE_INTRA)
  {
    SkipFlag = GetSkipFlag(Parser_p, TopLeftPU_p);
  }

  if(SkipFlag)
  {
    memset(Sessiondata_p->EdgeFlagsHor + StartAddr, 0x02, NumberPUinThisCU * sizeof(uint8));

    TopLeftPU_p->PartMode = PART_2Nx2N;
    for(y = 0; y < NumberPUinThisCU; y++)
    {
      Sessiondata_p->EdgeFlags[StartAddr + y * PicWidthPU] = 0x02;
      PU_p = TopLeftPU_p + y * PicWidthPU;
      for(x = 0 ; x < NumberPUinThisCU ; x++)
      {
        PU_p->CUSize = nCbS;
        PU_p->TransQuantBypass = (uint8)TransQBypassFlag;
        PU_p->skip_flag = 1;
        PU_p++->CbfLuma = 0;
      }
    }

    UpdateQp(Sessiondata_p, QgData_p, x0, y0);

#if MULTI_THREADED
    Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0, nCbS, nCbS, 0, ctDepth, PART_2Nx2N);
#else
    Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0, nCbS, nCbS, 0, ctDepth, PART_2Nx2N);
#endif
  }
  else
  {
    if(Sessiondata_p->CurrentSliceType != MODE_INTRA)
    {
      PredMode = GetPredModeFlag(Parser_p) ? MODE_INTRA : MODE_INTER;
    }

    if(PredMode  ==  MODE_INTRA)
    {
      PartMode = IsLeafCU ? (PartitionMode_t)GetIntraPartMode(Parser_p) : PART_2Nx2N;
#if MULTI_THREADED
      Parse_intra_prediction_unit(Threaddata_p, log2CbSize, PartMode, TransQBypassFlag, NumberPUinThisCU, StartX, StartY);
#else
      Parse_intra_prediction_unit(Sessiondata_p, log2CbSize, PartMode, TransQBypassFlag, NumberPUinThisCU, StartX, StartY);
#endif
    }
    else
    {
      PartMode = (PartitionMode_t)GetInterPartMode(Parser_p, AmpEnabledFlag, Log2MinCbSizeY, IsLeafCU);
      //Set all CU info for this inter coded CU
      memset(Sessiondata_p->EdgeFlagsHor + StartAddr, 0x02, NumberPUinThisCU*sizeof(uint8));
      TopLeftPU_p->PartMode = (uint8)PartMode;
      for(y = 0; y < NumberPUinThisCU; y++)
      {
        Sessiondata_p->EdgeFlags[StartAddr + y * PicWidthPU] = 0x02;
        PU_p = TopLeftPU_p + y * PicWidthPU;
        for(x = 0; x < NumberPUinThisCU; x++)
        {
          PU_p->CUSize   = nCbS;
          PU_p->TransQuantBypass = (uint8)TransQBypassFlag;
          PU_p++->skip_flag = 0;
        }
      }
#if MULTI_THREADED
      switch(PartMode)
      {
      case PART_2Nx2N:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0, nCbS, nCbS, 0, ctDepth, PART_2Nx2N);
        break;
      case PART_2NxN:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0,               nCbS, nCbS >> 1, 0, ctDepth, PART_2NxN);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0 + (nCbS >> 1), nCbS, nCbS >> 1, 1, ctDepth, PART_2NxN);
        break;
      case PART_Nx2N:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0,               y0, nCbS >> 1, nCbS, 0, ctDepth, PART_Nx2N);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0, nCbS >> 1, nCbS, 1, ctDepth, PART_Nx2N);
        break;
      case PART_2NxnU:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0,               nCbS, nCbS >> 2,       0, ctDepth, PART_2NxnU);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0 + (nCbS >> 2), nCbS, (nCbS * 3) >> 2, 1, ctDepth, PART_2NxnU);
        break;
      case PART_2NxnD:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0,                     nCbS, (nCbS * 3) >> 2, 0, ctDepth, PART_2NxnD);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0, y0 + ((nCbS * 3) >> 2), nCbS, (nCbS >> 2),     1, ctDepth, PART_2NxnD);
        break;
      case PART_nLx2N:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0,               y0, (nCbS >> 2),     nCbS, 0, ctDepth, PART_nLx2N);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0 + (nCbS >> 2), y0, (nCbS * 3) >> 2, nCbS, 1, ctDepth, PART_nLx2N);
        break;
      case PART_nRx2N:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0,                     y0, (nCbS * 3) >> 2, nCbS, 0, ctDepth, PART_nRx2N);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0 + ((nCbS * 3) >> 2), y0, (nCbS >> 2),     nCbS, 1, ctDepth, PART_nRx2N);
        break;
      case PART_NxN:
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0,               y0,               nCbS >> 1, nCbS >> 1, 0, ctDepth, PART_NxN);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0,               nCbS >> 1, nCbS >> 1, 1, ctDepth, PART_NxN);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0,               y0 + (nCbS >> 1), nCbS >> 1, nCbS >> 1, 2, ctDepth, PART_NxN);
        Parse_prediction_unit(Threaddata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0 + (nCbS >> 1), nCbS >> 1, nCbS >> 1, 3, ctDepth, PART_NxN);
      default:
        break;
      }
#else
      switch(PartMode)
      {
      case PART_2Nx2N:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0, nCbS, nCbS, 0, ctDepth, PART_2Nx2N);
        break;
      case PART_2NxN:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0,               nCbS, nCbS >> 1, 0, ctDepth, PART_2NxN);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0 + (nCbS >> 1), nCbS, nCbS >> 1, 1, ctDepth, PART_2NxN);
        break;
      case PART_Nx2N:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0,               y0, nCbS >> 1, nCbS, 0, ctDepth, PART_Nx2N);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0, nCbS >> 1, nCbS, 1, ctDepth, PART_Nx2N);
        break;
      case PART_2NxnU:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0,               nCbS, nCbS >> 2,       0, ctDepth, PART_2NxnU);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0 + (nCbS >> 2), nCbS, (nCbS * 3) >> 2, 1, ctDepth, PART_2NxnU);
        break;
      case PART_2NxnD:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0,                     nCbS, (nCbS * 3) >> 2, 0, ctDepth, PART_2NxnD);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0, y0 + ((nCbS * 3) >> 2), nCbS, (nCbS >> 2),     1, ctDepth, PART_2NxnD);
        break;
      case PART_nLx2N:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0,               y0, (nCbS >> 2),     nCbS, 0, ctDepth, PART_nLx2N);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0 + (nCbS >> 2), y0, (nCbS * 3) >> 2, nCbS, 1, ctDepth, PART_nLx2N);
        break;
      case PART_nRx2N:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0,                     y0, (nCbS * 3) >> 2, nCbS, 0, ctDepth, PART_nRx2N);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0 + ((nCbS * 3) >> 2), y0, (nCbS >> 2),     nCbS, 1, ctDepth, PART_nRx2N);
        break;
      case PART_NxN:
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0,               y0,               nCbS >> 1, nCbS >> 1, 0, ctDepth, PART_NxN);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0,               nCbS >> 1, nCbS >> 1, 1, ctDepth, PART_NxN);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0,               y0 + (nCbS >> 1), nCbS >> 1, nCbS >> 1, 2, ctDepth, PART_NxN);
        Parse_prediction_unit(Sessiondata_p, x0, y0, nCbS, x0 + (nCbS >> 1), y0 + (nCbS >> 1), nCbS >> 1, nCbS >> 1, 3, ctDepth, PART_NxN);
      default:
        break;
      }
#endif
    }
    UpdateQp(Sessiondata_p, QgData_p, x0, y0);

    if((PredMode !=  MODE_INTRA) && !(PartMode == PART_2Nx2N && TopLeftPU_p->merge_flag))
    {
      NoResiSyntaxFlag = GetNoResidualSyntaxFlag(Parser_p);
    }

    if(!NoResiSyntaxFlag)
    {
      if(PredMode == MODE_INTRA)
      {
        IntraSplitFlag = (PartMode == PART_NxN) ? 1 : 0;
        MaxTrafoDepth  = CurrSPS->max_transform_hierarchy_depth_intra + IntraSplitFlag;
      }
      else
      {
        MaxTrafoDepth = CurrSPS->max_transform_hierarchy_depth_inter;
        if(MaxTrafoDepth == 0)
        {
          InterSplitFlag = (PartMode == PART_2Nx2N) ? 0 : 1;
        }
      }
#if MULTI_THREADED
      Parse_transform_tree(Threaddata_p, x0, y0, x0, y0, log2CbSize, 0, 0, MaxTrafoDepth, IntraSplitFlag, InterSplitFlag, 0, PredMode);
#else
      Parse_transform_tree(Sessiondata_p, x0, y0, x0, y0, log2CbSize, 0, 0, MaxTrafoDepth, IntraSplitFlag, InterSplitFlag, 0, PredMode);
#endif
    }
    else
    {
      memset(Sessiondata_p->EdgeFlagsHor + StartAddr, 0x02, NumberPUinThisCU*sizeof(uint8));
      for(y = 0 ; y < NumberPUinThisCU ; y++)
      {
        Sessiondata_p->EdgeFlags[StartAddr + y * PicWidthPU] = 0x02;
        PU_p = TopLeftPU_p + y * PicWidthPU;
        for(x = 0 ; x < NumberPUinThisCU ; x++)
        {
          PU_p++->CbfLuma = 0;
        }
      }
    }
  }
}


/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        Parse_sao_syntax
*
******************************************************************************/
void Parse_sao_syntax(
#if MULTI_THREADED
                      ThreadData_t  *Threaddata_p,
#else
                      SessionData_t *Sessiondata_p,
#endif
                      uint32 rx,
                      uint32 ry)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t *Parser_p = &Threaddata_p->Parser;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
#endif
  const uint32 Log2CtbSizeLuma = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const uint32 Log2MinPUSizeLuma = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 Log2CtbSizeInPU = Log2CtbSizeLuma - Log2MinPUSizeLuma;
  const uint32 RecPicWidthInPU = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 PicWidthInCtb = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint32 x0InPU = rx << Log2CtbSizeInPU;
  uint32 y0InPU = ry << Log2CtbSizeInPU;
  uint32 SaoMergeLeftFlag = 0;
  uint32 SaoMergeUpFlag = 0;
  uint32 SaoTypeIdxLuma = 0;
  uint32 SaoTypeIdxChroma = 0;
  sint32 SaoOffsetValLuma[4];
  sint32 SaoOffsetValCb[4];
  sint32 SaoOffsetValCr[4];
  sint32 SaoOffsetSignLuma[4];
  sint32 SaoOffsetSignCb[4];
  sint32 SaoOffsetSignCr[4];
  uint32 SaoTypeClassLuma;
  uint32 SaoTypeClassCb;
  uint32 SaoTypeClassCr;
  uint8 cIdx;
  uint8 i;

  if(rx > 0 && Sessiondata_p->PredictionUnitArray[y0InPU*RecPicWidthInPU+x0InPU].PredictionUnitLeft_p)
  {
    SaoMergeLeftFlag = (uint8)GetSaoMergeFlag(Parser_p);
    if(SaoMergeLeftFlag)
    {
      if(Sessiondata_p->CurrentSliceSaoLumaFlag)
      {
        SaoTypeIdxLuma = Sessiondata_p->SaoTypeIdx[0][ry*PicWidthInCtb+rx-1];
        SaoTypeClassLuma = Sessiondata_p->SaoTypeClass[0][ry*PicWidthInCtb+rx-1];
        for(i = 0; i < 4; i++)
        {
          SaoOffsetValLuma[i] = Sessiondata_p->SaoOffsetVal[0][i+1][ry*PicWidthInCtb+rx-1];
        }
      }
      if(Sessiondata_p->CurrentSliceSaoChromaFlag)
      {
        SaoTypeIdxChroma = Sessiondata_p->SaoTypeIdx[1][ry*PicWidthInCtb+rx-1];
        SaoTypeClassCb = Sessiondata_p->SaoTypeClass[1][ry*PicWidthInCtb+rx-1];
        SaoTypeClassCr = Sessiondata_p->SaoTypeClass[2][ry*PicWidthInCtb+rx-1];
        for(i = 0; i < 4; i++)
        {
          SaoOffsetValCb[i] = Sessiondata_p->SaoOffsetVal[1][i+1][ry*PicWidthInCtb+rx-1];
          SaoOffsetValCr[i] = Sessiondata_p->SaoOffsetVal[2][i+1][ry*PicWidthInCtb+rx-1];
        }
      }
    }
    //sao_merge_left_flag
  }
  if(ry > 0 && !SaoMergeLeftFlag && Sessiondata_p->PredictionUnitArray[y0InPU*RecPicWidthInPU+x0InPU].PredictionUnitUp_p)
  {
    SaoMergeUpFlag = (uint8)GetSaoMergeFlag(Parser_p);
    if(SaoMergeUpFlag)
    {
      if(Sessiondata_p->CurrentSliceSaoLumaFlag)
      {
        SaoTypeIdxLuma = Sessiondata_p->SaoTypeIdx[0][(ry-1)*PicWidthInCtb+rx];
        SaoTypeClassLuma = Sessiondata_p->SaoTypeClass[0][(ry-1)*PicWidthInCtb+rx];
        for(i = 0; i < 4; i++)
        {
          SaoOffsetValLuma[i] = Sessiondata_p->SaoOffsetVal[0][i+1][(ry-1)*PicWidthInCtb+rx];
        }
      }
      if(Sessiondata_p->CurrentSliceSaoChromaFlag)
      {
        SaoTypeIdxChroma = Sessiondata_p->SaoTypeIdx[1][(ry-1)*PicWidthInCtb+rx];
        SaoTypeClassCb = Sessiondata_p->SaoTypeClass[1][(ry-1)*PicWidthInCtb+rx];
        SaoTypeClassCr = Sessiondata_p->SaoTypeClass[2][(ry-1)*PicWidthInCtb+rx];
        for(i = 0; i < 4; i++)
        {
          SaoOffsetValCb[i] = Sessiondata_p->SaoOffsetVal[1][i+1][(ry-1)*PicWidthInCtb+rx];
          SaoOffsetValCr[i] = Sessiondata_p->SaoOffsetVal[2][i+1][(ry-1)*PicWidthInCtb+rx];
        }
      }
    }
    //sao_merge_up_flag
  }
  if(!SaoMergeLeftFlag && !SaoMergeUpFlag)
  {
    for(cIdx = 0; cIdx < 3; cIdx++)
    {
      if((Sessiondata_p->CurrentSliceSaoLumaFlag && cIdx == 0)||
        (Sessiondata_p->CurrentSliceSaoChromaFlag && cIdx > 0))
      {
        switch (cIdx)
        {
        case 0:
          SaoTypeIdxLuma = (uint8)GetSaoTypeIdx(Parser_p);
          if(SaoTypeIdxLuma != 0)
          {
            memset(SaoOffsetValLuma, 0, 4*sizeof(SaoOffsetValLuma[0]));
            for(i = 0; i < 4; i++)
            {
              SaoOffsetValLuma[i] = (sint32)GetSaoOffsetAbs(Parser_p);
            }
            memset(SaoOffsetSignLuma, 0, 4*sizeof(SaoOffsetSignLuma[0]));
            if(SaoTypeIdxLuma == 1)
            {
              for(i = 0; i < 4; i++)
              {
                if(SaoOffsetValLuma[i] != 0)
                {
                  SaoOffsetSignLuma[i] = GetSaoOffsetSign(Parser_p) ? -1 : 1;
                }
              }
              SaoTypeClassLuma = GetSaoBandPosition(Parser_p);
            }
            else
            {
              for(i = 0; i < 4; i++)
              {
                SaoOffsetSignLuma[i] = (i > 1) ? -1 : 1;
              }
              SaoTypeClassLuma = GetSaoEoClass(Parser_p);
            }
          }
          break;
        case 1:
          SaoTypeIdxChroma = (uint8)GetSaoTypeIdx(Parser_p);
          if(SaoTypeIdxChroma != 0)
          {
            memset(SaoOffsetValCb, 0, 4*sizeof(SaoOffsetValCb[0]));
            for(i = 0; i < 4; i++)
            {
              SaoOffsetValCb[i] = (sint32)GetSaoOffsetAbs(Parser_p);
            }
            memset(SaoOffsetSignCb, 0, 4*sizeof(SaoOffsetSignCb[0]));
            if(SaoTypeIdxChroma == 1)
            {
              for(i = 0; i < 4; i++)
              {
                if(SaoOffsetValCb[i] != 0)
                {
                  SaoOffsetSignCb[i] = GetSaoOffsetSign(Parser_p) ? -1 : 1;
                }
              }
              SaoTypeClassCb = GetSaoBandPosition(Parser_p);
            }
            else
            {
              for(i = 0; i < 4; i++)
              {
                SaoOffsetSignCb[i] = (i > 1) ? -1 : 1;
              }
              SaoTypeClassCb = GetSaoEoClass(Parser_p);
            }
          }
          break;
        case 2:
          if(SaoTypeIdxChroma != 0)
          {
            memset(SaoOffsetValCr, 0, 4*sizeof(SaoOffsetValCr[0]));
            for(i = 0; i < 4; i++)
            {
              SaoOffsetValCr[i] = (sint32)GetSaoOffsetAbs(Parser_p);
            }
            memset(SaoOffsetSignCr, 0, 4*sizeof(SaoOffsetSignCr[0]));
            if(SaoTypeIdxChroma == 1)
            {
              for(i = 0; i < 4; i++)
              {
                if(SaoOffsetValCr[i] != 0)
                {
                  SaoOffsetSignCr[i] = GetSaoOffsetSign(Parser_p) ? -1 : 1;
                }
              }
              SaoTypeClassCr =  GetSaoBandPosition(Parser_p);
            }
            else
            {
              for(i = 0; i < 4; i++)
              {
                SaoOffsetSignCr[i] = (i > 1) ? -1 : 1;
              }
              SaoTypeClassCr = SaoTypeClassCb;
            }
          }
          break;
        default:
          break;
        }
      }
    }
  }
  //set the sao parameters
  if(Sessiondata_p->CurrentSliceSaoLumaFlag)
  {
    Sessiondata_p->SaoTypeIdx[0][ry*PicWidthInCtb+rx] =  (uint8)SaoTypeIdxLuma;
    Sessiondata_p->SaoTypeClass[0][ry*PicWidthInCtb+rx] = (uint8)SaoTypeClassLuma;
    Sessiondata_p->SaoOffsetVal[0][0][ry*PicWidthInCtb+rx] = 0;
    for(i = 0; i < 4; i++)
    {
      if(!SaoMergeLeftFlag && !SaoMergeUpFlag)
      {
        Sessiondata_p->SaoOffsetVal[0][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValLuma[i]*SaoOffsetSignLuma[i]);
      }
      else
      {
        Sessiondata_p->SaoOffsetVal[0][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValLuma[i]);
      }
    }
  }
  if(Sessiondata_p->CurrentSliceSaoChromaFlag)
  {
    Sessiondata_p->SaoTypeIdx[1][ry*PicWidthInCtb+rx] = (uint8)SaoTypeIdxChroma;
    Sessiondata_p->SaoTypeClass[1][ry*PicWidthInCtb+rx] = (uint8)SaoTypeClassCb;
    Sessiondata_p->SaoTypeClass[2][ry*PicWidthInCtb+rx] = (uint8)SaoTypeClassCr;
    Sessiondata_p->SaoOffsetVal[1][0][ry*PicWidthInCtb+rx] = 0;
    Sessiondata_p->SaoOffsetVal[2][0][ry*PicWidthInCtb+rx] = 0;
    for(i = 0; i < 4; i++)
    {
      if(!SaoMergeLeftFlag && !SaoMergeUpFlag)
      {
        Sessiondata_p->SaoOffsetVal[1][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValCb[i]*SaoOffsetSignCb[i]);
        Sessiondata_p->SaoOffsetVal[2][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValCr[i]*SaoOffsetSignCr[i]);
      }
      else
      {
        Sessiondata_p->SaoOffsetVal[1][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValCb[i]);
        Sessiondata_p->SaoOffsetVal[2][i+1][ry*PicWidthInCtb+rx] = (sint16)(SaoOffsetValCr[i]);
      }
    }
  }
}

/******************************************************************************
*
* Name:        Parse_coding_quadtree
*
******************************************************************************/
void
Parse_coding_quadtree(
#if MULTI_THREADED
                      ThreadData_t *Threaddata_p,
#else
                      SessionData_t *Sessiondata_p,
#endif
                      const uint32 x0,
                      const uint32 y0,
                      const uint32 log2CbSize,
                      const uint32 ctDepth)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t *Parser_p = &Threaddata_p->Parser;
  QuantGroupData_t *QgData_p = &Threaddata_p->QgData;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
  QuantGroupData_t *QgData_p = &Sessiondata_p->QgData;
#endif
  uint32 pic_height_in_luma_samples = Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;
  uint32 pic_width_in_luma_samples  = Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  uint32 Log2MinCbSizeY             = Sessiondata_p->CurrentSequenceParameterSet->Log2MinCbSizeY;
  uint32 x1;
  uint32 y1;
  uint32 split_cu_flag;

  split_cu_flag = (log2CbSize > Log2MinCbSizeY) ? 1 : 0;

  if(x0 + ( 1 << log2CbSize ) <= pic_width_in_luma_samples  &&
     y0 + ( 1 << log2CbSize ) <= pic_height_in_luma_samples &&
    log2CbSize > Log2MinCbSizeY)
  {
    const uint32 LogMinPUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
    const uint32 PicWidthPU  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
    uint32 StartY = y0 >> LogMinPUSizeInPixels;
    uint32 StartX = x0 >> LogMinPUSizeInPixels;
    split_cu_flag = GetSplitCuFlag(Parser_p, Sessiondata_p->PredictionUnitArray + StartY * PicWidthPU + StartX, 1 << log2CbSize);
  }

  if(Sessiondata_p->CurrentPictureParameterSet->CuQpDeltaEnabledFlag &&
     log2CbSize >= Sessiondata_p->Log2MinCuQpDeltaSize)
  {
    QgData_p->IsCuQpDataCoded = 0;
    QgData_p->CuQpDeltaVal    = 0;
  }

  if(split_cu_flag)
  {
    x1 = x0 + ( ( 1 << log2CbSize ) >> 1 );
    y1 = y0 + ( ( 1 << log2CbSize ) >> 1 );

#if MULTI_THREADED
    Parse_coding_quadtree(Threaddata_p, x0, y0, log2CbSize - 1, ctDepth + 1);
    if( x1 < pic_width_in_luma_samples)
    {
      Parse_coding_quadtree(Threaddata_p, x1, y0, log2CbSize - 1, ctDepth + 1);
    }
    if( y1 < pic_height_in_luma_samples)
    {
      Parse_coding_quadtree(Threaddata_p, x0, y1, log2CbSize - 1, ctDepth + 1);
    }
    if( x1 < pic_width_in_luma_samples && y1 < pic_height_in_luma_samples)
    {
      Parse_coding_quadtree(Threaddata_p, x1, y1, log2CbSize - 1, ctDepth + 1);
    }
  }
  else
  {
    Parse_coding_unit(Threaddata_p, x0, y0, log2CbSize, ctDepth);
  }
#else
    Parse_coding_quadtree(Sessiondata_p, x0, y0, log2CbSize - 1, ctDepth + 1);

    if( x1 < pic_width_in_luma_samples)
    {
      Parse_coding_quadtree(Sessiondata_p, x1, y0, log2CbSize - 1, ctDepth + 1);
    }
    if( y1 < pic_height_in_luma_samples)
    {
      Parse_coding_quadtree(Sessiondata_p, x0, y1, log2CbSize - 1, ctDepth + 1);
    }
    if( x1 < pic_width_in_luma_samples && y1 < pic_height_in_luma_samples)
    {
      Parse_coding_quadtree(Sessiondata_p, x1, y1, log2CbSize - 1, ctDepth + 1);
    }
  }
  else
  {
    Parse_coding_unit(Sessiondata_p, x0, y0, log2CbSize, ctDepth);
  }
#endif
}
