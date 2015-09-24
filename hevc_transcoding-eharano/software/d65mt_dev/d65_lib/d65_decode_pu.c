/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains PU decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <string.h>
#include "d65_decode_pu.h"
#include "d65_inter_prediction.h"
#include "d65_cabac.h"


/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        ParseMvdCoding
*
* Parameters:  Parser_p   [in/out]  The parser
*
* Returns:     Motion Vector Difference
*
* Description: Decodes motion vector difference for a PU
*
******************************************************************************/
Vector_t
ParseMvdCoding(Parser_t *Parser_p)
{
  Vector_t MotionVectorDiff = {0, 0};
  uint16 AbsMv_x;
  uint16 AbsMv_y;
  uint16 abs_mvd_greater0_flag_x;
  uint16 abs_mvd_greater0_flag_y;
  uint16 abs_mvd_greater1_flag_x;
  uint16 abs_mvd_greater1_flag_y;
  uint16 abs_mvd_minus2_x;
  uint16 abs_mvd_minus2_y;
  uint16 mvd_sign_flag_x;
  uint16 mvd_sign_flag_y;

  AbsMv_x = 0;
  abs_mvd_greater0_flag_x = (uint16)GetAbsMvdGreater0Flag(Parser_p);
  AbsMv_x += abs_mvd_greater0_flag_x;

  AbsMv_y = 0;
  abs_mvd_greater0_flag_y = (uint16)GetAbsMvdGreater0Flag(Parser_p);
  AbsMv_y += abs_mvd_greater0_flag_y;

  if(abs_mvd_greater0_flag_x)
  {
    abs_mvd_greater1_flag_x = (uint16)GetAbsMvdGreater1Flag(Parser_p);
    AbsMv_x += abs_mvd_greater1_flag_x;
  }

  if(abs_mvd_greater0_flag_y)
  {
    abs_mvd_greater1_flag_y = (uint16)GetAbsMvdGreater1Flag(Parser_p);
    AbsMv_y += abs_mvd_greater1_flag_y;
  }

  if(abs_mvd_greater0_flag_x)
  {
    if(abs_mvd_greater1_flag_x)
    {
      abs_mvd_minus2_x = (uint16)GetAbsMvdMinus2(Parser_p);
      AbsMv_x += abs_mvd_minus2_x;
    }
    mvd_sign_flag_x = 0;
    mvd_sign_flag_x = (uint16)GetMvdSignFlag(Parser_p);
    MotionVectorDiff.x = AbsMv_x * (1 - 2 * mvd_sign_flag_x);
  }

  if(abs_mvd_greater0_flag_y)
  {
    if(abs_mvd_greater1_flag_y)
    {
      abs_mvd_minus2_y = (uint16)GetAbsMvdMinus2(Parser_p);
      AbsMv_y += abs_mvd_minus2_y;
    }
    mvd_sign_flag_y = 0;
    mvd_sign_flag_y = (uint16)GetMvdSignFlag(Parser_p);
    MotionVectorDiff.y = AbsMv_y *(1 - 2 * mvd_sign_flag_y);
  }

  return MotionVectorDiff;
}


/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        Parse_prediction_unit
*
******************************************************************************/
void
Parse_prediction_unit(
#if MULTI_THREADED
                      ThreadData_t *Threaddata_p,
#else
                      SessionData_t *Sessiondata_p,
#endif
                      const uint32 xC,
                      const uint32 yC,
                      const uint32 nCbS,
                      const uint32 x0,
                      const uint32 y0,
                      const uint32 nPbW,
                      const uint32 nPbH,
                      const uint8 PartIdx,
                      const uint32 ctDepth,
                      const PartitionMode_t PartMode)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p      = &Threaddata_p->Parser;
#else
  Parser_t      *Parser_p      = &Sessiondata_p->Parser;
#endif
  const uint32 PicWidthInPU          = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 Log2MinPUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 StartPU               = (y0 >> Log2MinPUSizeInPixels) * PicWidthInPU + (x0 >> Log2MinPUSizeInPixels);
  const uint32 PUWidth               = nPbW >> Log2MinPUSizeInPixels;
  const uint32 PUHeight              = nPbH >> Log2MinPUSizeInPixels;
  const uint8  MvdL1Zero             = Sessiondata_p->CurrentMvdL1ZeroFlag;
  const uint8  ParMgCU               = (nCbS == 8) && (Sessiondata_p->CurrentPictureParameterSet->log2_parallel_merge_level > 2);

  PredictionUnit_t   *PU_p               =  Sessiondata_p->PredictionUnitArray + StartPU;
  ReferencePicture_t *ReferencePicture_p = NULL;

  Vector_t MotionVectorL0 = { 0, 0, -1 };
  Vector_t MotionVectorL1 = { 0, 0, -1 };
  Vector_t MvdL0          = { 0, 0, -1 };
  Vector_t MvdL1          = { 0, 0, -1 };

  sint8 ref_idx_l0 = -1;
  sint8 ref_idx_l1 = -1;
  uint8 merge_idx  =  0;
  uint8 merge_flag =  0;
  uint8 inter_pred_idc;
  uint8 mvp_l0_flag;
  uint8 mvp_l1_flag;
  uint32 y;
  uint32 x;

  if(PU_p->skip_flag)
  {
    /* MODE_SKIP */
    if(Sessiondata_p->CurrentMaxNumMergeCand > 1)
    {
      merge_idx = GetMergeIdx(Parser_p, Sessiondata_p->CurrentMaxNumMergeCand);
      PU_p->merge_idx = merge_idx;
      if (ParMgCU)
      {
        GetMergedMotion[Sessiondata_p->CurrentSliceType](Sessiondata_p, xC, yC, nCbS, nCbS, merge_idx, 0, PART_2Nx2N, &MotionVectorL0, &MotionVectorL1);
      }
      else
      {
        GetMergedMotion[Sessiondata_p->CurrentSliceType](Sessiondata_p, x0, y0, nPbW, nPbH, merge_idx, PartIdx, PartMode, &MotionVectorL0, &MotionVectorL1);
      }
      if(MotionVectorL1.ref_idx != -1 && MotionVectorL0.ref_idx != -1 && nPbW + nPbH == 12)
      {
        MotionVectorL1.ref_idx = -1;
        MotionVectorL1.x = 0;
        MotionVectorL1.y = 0;
      }
    }
  }
  else
  {
    /* MODE_INTER */
    merge_flag = GetMergeFlag(Parser_p);
    PU_p->merge_flag = merge_flag;
    if(merge_flag) 
    {
      if(Sessiondata_p->CurrentMaxNumMergeCand > 1)
      {
        merge_idx = GetMergeIdx(Parser_p, Sessiondata_p->CurrentMaxNumMergeCand);
        PU_p->merge_idx = merge_idx;
        if (ParMgCU)
        {
          GetMergedMotion[Sessiondata_p->CurrentSliceType](Sessiondata_p, xC, yC, nCbS, nCbS, merge_idx, 0, PART_2Nx2N, &MotionVectorL0, &MotionVectorL1);
        }
        else
        {
          GetMergedMotion[Sessiondata_p->CurrentSliceType](Sessiondata_p, x0, y0, nPbW, nPbH, merge_idx, PartIdx, PartMode, &MotionVectorL0, &MotionVectorL1);
        }
        if(MotionVectorL1.ref_idx != -1 && MotionVectorL0.ref_idx != -1 && nPbW + nPbH == 12)
        {
          MotionVectorL1.ref_idx = -1;
          MotionVectorL1.x = 0;
          MotionVectorL1.y = 0;
        }
      }
    }
    else
    {
      if(Sessiondata_p->CurrentSliceType == MODE_B)
      {
        inter_pred_idc = ((nPbW + nPbH) != 12) ? GetInterPredIdc(Parser_p, ctDepth) : GetMinInterPredIdc(Parser_p);
      }
      else
      {
        inter_pred_idc = PRED_L0;
      }
      PU_p->inter_pred_idc = inter_pred_idc;

      if(inter_pred_idc != PRED_L1)
      {
        ref_idx_l0 = 0;
        mvp_l0_flag = 0;

        if(Sessiondata_p->Current_num_ref_idx_l0_active_minus1 > 0)
        {
          ref_idx_l0 = (sint8)GetRefIdx(Parser_p, Sessiondata_p->Current_num_ref_idx_l0_active_minus1); 
        }

        ReferencePicture_p = Sessiondata_p->RefPicList0[ref_idx_l0];
        // Check whether ref_idx_l0 points at a picture that is unavailable or
        // marked as unused_for_reference.
        if(ReferencePicture_p == NULL || ReferencePicture_p->PictureMarking == UNUSED_FOR_REFERENCE)
        {
          // If we detect a bad ref_idx_l0 we should use the existing short-term picture
          // with closest poc instead. If no picture is found we use PreviousImage[0].
          // We assume here that pictures with close POCs look similar.

          // Currently not tested so for now we throw a bit-error.
          // See how this is done in d64, function DecodeRefIdxAndMVs()
#if MULTI_THREADED
          BIT_ERROR(Threaddata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Referenced picture marked unused for reference");
#else
          BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Referenced picture marked unused for reference");
#endif
        }

        MvdL0 = ParseMvdCoding(Parser_p);
        mvp_l0_flag = (uint8)GetMvpLxFlag(Parser_p);
        MotionVectorL0 = GetMvpCandidateLX(Sessiondata_p, x0, y0, nPbW, nPbH, mvp_l0_flag, ref_idx_l0, 0);
        MotionVectorL0.x += MvdL0.x;
        MotionVectorL0.y += MvdL0.y;
        MotionVectorL0.ref_idx = ref_idx_l0;
      }

      if(inter_pred_idc != PRED_L0)
      {
        ref_idx_l1=0;
        mvp_l1_flag=0;
        if( Sessiondata_p->Current_num_ref_idx_l1_active_minus1 > 0)
        {
          ref_idx_l1= (sint8)GetRefIdx(Parser_p, Sessiondata_p->Current_num_ref_idx_l1_active_minus1);
        }
        ReferencePicture_p = Sessiondata_p->RefPicList0[ref_idx_l1];
        // Check whether ref_idx_l1 points at a picture that is unavailable or
        // marked as unused_for_reference.
        if(ReferencePicture_p == NULL || ReferencePicture_p->PictureMarking == UNUSED_FOR_REFERENCE)
        {
#if MULTI_THREADED
          BIT_ERROR(Threaddata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Referenced picture marked unused for reference");
#else
          BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Referenced picture marked unused for reference");
#endif        
        }

        if(!(MvdL1Zero && inter_pred_idc == PRED_BI)) 
        {
          MvdL1 = ParseMvdCoding(Parser_p);
        }
        mvp_l1_flag = (uint8)GetMvpLxFlag(Parser_p);
        MotionVectorL1 = GetMvpCandidateLX(Sessiondata_p, x0, y0, nPbW, nPbH, mvp_l1_flag, ref_idx_l1, 1);
        MotionVectorL1.x += MvdL1.x;
        MotionVectorL1.y += MvdL1.y;
        MotionVectorL1.ref_idx = ref_idx_l1;
      }
    }
  }

  if((PartIdx == 1) && (PartMode == PART_2NxN || PartMode == PART_2NxnD || PartMode == PART_2NxnU) ||
    ((PartIdx >= 2) && (PartMode == PART_NxN)))
  {
    memset(Sessiondata_p->EdgeFlagsHor + StartPU, 1, PUWidth*sizeof(uint8));
  }

  if((PartIdx == 1) && (PartMode == PART_Nx2N || PartMode == PART_nLx2N || PartMode == PART_nRx2N) ||
    ((PartIdx % 2) && (PartMode == PART_NxN)))
  {
    for(y = 0; y < PUHeight; y++)
    {
      Sessiondata_p->EdgeFlags[StartPU + y*PicWidthInPU] = 1;
    }
  }

  for(y = 0; y < PUHeight; y++)
  {
    for(x = 0; x < PUWidth; x++)
    {
      PU_p[y*PicWidthInPU+x].MotionvectorL0 = MotionVectorL0;
      PU_p[y*PicWidthInPU+x].MotionvectorL1 = MotionVectorL1;
      PU_p[y*PicWidthInPU+x].PredMode = MODE_INTER;
    }
  }

#if MULTI_THREADED
  if((MotionVectorL0.ref_idx != -1) && (MotionVectorL1.ref_idx != -1))
  {
    /* Biprediction */
    PerformBiprediction(Threaddata_p, x0, y0, nPbW, nPbH, MotionVectorL0, MotionVectorL1);
  }
  else
  {
    /* Uniprediction */
    if(MotionVectorL0.ref_idx != -1)
    {
      /* Using reference list 0 */
      PerformUniprediction(Threaddata_p, x0, y0, nPbW, nPbH, Sessiondata_p->RefPicList0, MotionVectorL0);
    }
    else
    {
      /* Using reference list 1 */
      PerformUniprediction(Threaddata_p, x0, y0, nPbW, nPbH, Sessiondata_p->RefPicList1, MotionVectorL1);
    }
  }
#else
  if((MotionVectorL0.ref_idx != -1) && (MotionVectorL1.ref_idx != -1))
  {
    /* Biprediction */
    PerformBiprediction(Sessiondata_p, x0, y0, nPbW, nPbH, MotionVectorL0, MotionVectorL1);
  }
  else
  {
    /* Uniprediction */
    if(MotionVectorL0.ref_idx != -1)
    {
      /* Using reference list 0 */
      PerformUniprediction(Sessiondata_p, x0, y0, nPbW, nPbH, Sessiondata_p->RefPicList0, MotionVectorL0);
    }
    else
    {
      /* Using reference list 1 */
      PerformUniprediction(Sessiondata_p, x0, y0, nPbW, nPbH, Sessiondata_p->RefPicList1, MotionVectorL1);
    }
  }
#endif
}


/******************************************************************************
*
* Name:        Parse_intra_prediction_unit
*
******************************************************************************/
void Parse_intra_prediction_unit(
#if MULTI_THREADED
                                 ThreadData_t *Threaddata_p,
#else
                                 SessionData_t *Sessiondata_p,
#endif
                                 const uint32 log2CbSize,
                                 const uint32 PartMode,
                                 const uint32 cu_transquant_bypass_flag,
                                 const uint32 NumberPUinThisCU,
                                 const uint32 StartX,
                                 const uint32 StartY)
{
#if MULTI_THREADED
  SessionData_t* Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t *Parser_p = &Threaddata_p->Parser;
#else
  Parser_t *Parser_p = &Sessiondata_p->Parser;
#endif
  const uint32 Log2CtbSizeY         = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const uint32 Log2MinCbSizeY = Sessiondata_p->CurrentSequenceParameterSet->Log2MinCbSizeY;
  const uint32 LogMinPUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 PicWidthPU  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 nCbS = 1 << log2CbSize;
  uint32 IntraPartNum;
  
  uint32 prev_intra_luma_pred_flag[4];
  uint8 candModeList[3];
  uint8 candIntraPredModeA;
  uint8 candIntraPredModeB;
  uint8 IntraPredMode[4];
  uint8 IntraPredModeC;

  uint32 y;
  uint32 x;
  uint32 i;
  uint32 yP;
  uint32 xP;
  uint32 yBB;

  PredictionUnit_t *PU_p = Sessiondata_p->PredictionUnitArray + StartY*PicWidthPU + StartX;

  //if( PartMode  ==  PART_2Nx2N && pcm_enabled_flag &&
  //  log2CbSize >= Log2MinIpcmCbSizeY &&
  //  log2CbSize <= Log2MaxIpcmCbSizeY )
  //  pcm_flag[ x0 ][ y0 ]
  //if( pcm_flag[ x0 ][ y0 ] ) {
  //  while( !byte_aligned( ) )
  //    pcm_alignment_zero_bit
  //    pcm_sample( x0, y0, log2CbSize )
  //} else 
  //{
  IntraPartNum = (PartMode  ==  PART_NxN) ? 4 : 1;
  for(i = 0 ; i < IntraPartNum; i++)
  {
    prev_intra_luma_pred_flag[i] = GetPrevIntraLumaPredFlag(Parser_p);
  }
  for(i = 0; i < IntraPartNum; i++)
  {
    yP = StartY +(i/2)*(NumberPUinThisCU/2);
    xP = StartX +(i%2)*(NumberPUinThisCU/2);

    //Get Left PU's intra prediction mode
    if(i%2)
    {
      candIntraPredModeA = IntraPredMode[i-1];
    }
    else
    {
      candIntraPredModeA = INTRA_DC;
      if(Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitLeft_p)
      {
        if(Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitLeft_p->PredMode == MODE_INTRA)
        {
          candIntraPredModeA = Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitLeft_p->IntraPredMode;
        }
      }
    }
    //Get up PU's intra prediction mode
    if(i/2)
    {
      candIntraPredModeB = IntraPredMode[i-2];
    }
    else
    {
      candIntraPredModeB = INTRA_DC;
      if(Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitUp_p)
      {
        yBB = yP << LogMinPUSizeInPixels;
        if(Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitUp_p->PredMode == MODE_INTRA && (yBB - 1) >= ((yBB >> Log2CtbSizeY)<<Log2CtbSizeY))
        {
          candIntraPredModeB = Sessiondata_p->PredictionUnitArray[yP * PicWidthPU + xP].PredictionUnitUp_p->IntraPredMode;
        }
      }
    }
    //Generate intra prediction mode candidate list
    if(candIntraPredModeA == candIntraPredModeB)
    {
      if(candIntraPredModeA < 2)
      {
        candModeList[0] = INTRA_PLANAR;
        candModeList[1] = INTRA_DC;
        candModeList[2] = INTRA_VER;
      }
      else
      {
        candModeList[0] = candIntraPredModeA;
        candModeList[1] = 2 + ((candIntraPredModeA + 29) % 32);
        candModeList[2] = 2 + ((candIntraPredModeA - 2 + 1 ) % 32);
      }
    }
    else
    {
      candModeList[0] = candIntraPredModeA;
      candModeList[1] = candIntraPredModeB;
      if(candModeList[0] != INTRA_PLANAR && candModeList[1] != INTRA_PLANAR)
      {
        candModeList[2] = INTRA_PLANAR;
      }
      else if(candModeList[0] != INTRA_DC && candModeList[1] != INTRA_DC)
      {
        candModeList[2] = INTRA_DC;
      }
      else
      {
        candModeList[2]= INTRA_VER;
      }
    }

    if(prev_intra_luma_pred_flag[i])
    {
      uint32 mpm_idx = GetMpmIdx(Parser_p);
      IntraPredMode[i] = candModeList[mpm_idx];
    }
    else
    {
      uint8 rem_intra_luma_pred_mode = (uint8)GetRemIntraLumaPredMode(Parser_p);
      //ugly code now...candidate list reordering...
      uint8 temp;
      if(candModeList[0] > candModeList[1])
      {
        temp = candModeList[1];
        candModeList[1] = candModeList[0];
        candModeList[0] = temp;
      }
      if(candModeList[0] > candModeList[2])
      {
        temp = candModeList[2];
        candModeList[2] = candModeList[0];
        candModeList[0] = temp;
      }
      if(candModeList[1] > candModeList[2])
      {
        temp = candModeList[2];
        candModeList[2] = candModeList[1];
        candModeList[1] = temp;
      }
      IntraPredMode[i] = rem_intra_luma_pred_mode;
      for(temp = 0; temp < 3; temp++)
      {
        if(IntraPredMode[i] >= candModeList[temp])
        {
          IntraPredMode[i]++;
        }
      }
    }
  }
  if(IntraPartNum == 1)
  {
    memset(IntraPredMode, IntraPredMode[0], 4 * sizeof(uint8));
  }
  //printf("\nx = %d, y = %d IntraPredMode = %d", x0, y0, IntraPredMode[0]);
  switch(GetIntraChromaPredMode(Parser_p))
  {
  case 0:
    if(IntraPredMode[0] == 0)
    {
      IntraPredModeC = 34;
    }
    else
    {
      IntraPredModeC = 0;
    }
    break;
  case 1:
    if(IntraPredMode[0] == 26)
    {
      IntraPredModeC = 34;
    }
    else
    {
      IntraPredModeC = 26;
    }
    break;
  case 2:
    if(IntraPredMode[0] == 10)
    {
      IntraPredModeC = 34;
    }
    else
    {
      IntraPredModeC = 10;
    }
    break;
  case 3:
    if(IntraPredMode[0] == 1)
    {
      IntraPredModeC = 34;
    }
    else
    {
      IntraPredModeC = 1;
    }
    break;
  case 4:
    IntraPredModeC = (uint8)IntraPredMode[0]; // Look at table 8-2 to derive IntraChomaPredMode from IntraPredMode and intra_chroma_pred_mode
    break;
  default:
    break;
  }
  //Set all CU info for this intra coded CU
  PU_p[0].PartMode = (uint8)PartMode;
  for(y = 0; y < NumberPUinThisCU; y++)
  {
    uint8 LowPart = (y >= NumberPUinThisCU/2);
    for(x = 0; x < NumberPUinThisCU; x++)
    {
      PU_p[y*PicWidthPU+x].CUSize   = nCbS;
      PU_p[y*PicWidthPU+x].TransQuantBypass = (uint8)cu_transquant_bypass_flag;
      PU_p[y*PicWidthPU+x].skip_flag = 0;
      PU_p[y*PicWidthPU+x].IntraPredMode = IntraPredMode[(LowPart << 1) + (x >= NumberPUinThisCU/2)];
      PU_p[y*PicWidthPU+x].intraChromaPredMode = IntraPredModeC;
    }
  }
}
