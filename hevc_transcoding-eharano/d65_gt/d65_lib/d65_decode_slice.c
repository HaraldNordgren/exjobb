/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains slice decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <string.h>
#include "d65_decode_slice.h"
#include "d65_context.h"
#include "d65_getbits.h"
#include "d65_cabac.h"
#include "d65_init.h"
#include "d65_dpb.h"
#include "d65_decode_cu.h"
#include "d65_putbits.h"

#if MULTI_THREADED
#include "pthread.h"
#endif

/******************************************************************************
* Definition of local variables
******************************************************************************/

const static PredictionMode_t CabacInitSliceLut[3] = {MODE_INTER, MODE_B, MODE_INTRA};

/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        ComputeTileBoundaries
*
* Parameters:  Sessiondata_p [in]      The decoder
*              CurrPicSet    [in]      Current PPS
*              NewSPSFlag    [in]      New SPS enabled flag
*
* Returns:     uint8 - flag for signaling tiling has been changed
*
* Description: Check & Update tiling structure
*
******************************************************************************/
uint8 ComputeTileBoundaries(SessionData_t *Sessiondata_p,
                            const PictureParameterSet_t *CurrPicSet,
                            const uint8 NewSPSFlag)
{
  const PictureParameterSet_t *PrevPicSet = Sessiondata_p->CurrentPictureParameterSet;
  const uint32 PicHeightInCtbsY = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
  const uint32 PicWidthInCtbsY  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  const uint32 Log2CtbSizeY     = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;

  const uint32 num_tile_columns_minus1 = CurrPicSet->num_tile_columns_minus1;
  const uint32 num_tile_rows_minus1    = CurrPicSet->num_tile_rows_minus1;
  const uint32 *column_width           = CurrPicSet->column_width;
  const uint32 *row_height             = CurrPicSet->row_height;
  const uint8  uniform_spacing         = CurrPicSet->uniform_spacing_flag;
  sint32 colWidth[256];
  sint32 rowHeight[256];
  uint32 colBd[257];
  uint32 rowBd[257];
  uint32 i;
  uint32 j;
  uint8 tilingChanged = 0;

  if(PrevPicSet)
  {
    tilingChanged = NewSPSFlag ||
                    PrevPicSet->num_tile_columns_minus1      != num_tile_columns_minus1 ||
                    PrevPicSet->num_tile_rows_minus1         != num_tile_rows_minus1 ||
                    PrevPicSet->uniform_spacing_flag         != uniform_spacing ||
                    Sessiondata_p->CurrentTileColumnMinus1   != num_tile_columns_minus1 ||
                    Sessiondata_p->CurrentTileRowMinus1      != num_tile_rows_minus1 ||
                    Sessiondata_p->CurrentUniformSpacingFlag != uniform_spacing;
  }
  else
  {
    tilingChanged = 1;
  }

  if(tilingChanged && CurrPicSet->TilesEnabledFlag)
  {
    // Now we have all the information to decode the column width and row height for the tiles
    if(uniform_spacing)
    {
      for( i = 0; i <= num_tile_columns_minus1; i++ )
      {
        colWidth[i] = (((i + 1) * PicWidthInCtbsY) / (num_tile_columns_minus1 + 1 )) - (( i * PicWidthInCtbsY ) / (num_tile_columns_minus1 + 1));
      }

      for(j = 0; j <= num_tile_rows_minus1; j++)
      {
        rowHeight[j] = (((j+1) * PicHeightInCtbsY) / (num_tile_rows_minus1 + 1)) - ((j * PicHeightInCtbsY) / (num_tile_rows_minus1 + 1));
      }
    }
    else 
    {
      colWidth[num_tile_columns_minus1] = PicWidthInCtbsY;
      for(i = 0; i < num_tile_columns_minus1; i++) 
      {
        colWidth[i]                        = column_width[i];
        colWidth[num_tile_columns_minus1] -= colWidth[i];
      }

      rowHeight[num_tile_rows_minus1] = PicHeightInCtbsY;
      for(j = 0; j < num_tile_rows_minus1; j++)
      {
        rowHeight[j]                     = row_height[j];
        rowHeight[num_tile_rows_minus1] -= rowHeight[j];
      }
    }

    // Do tile size check

    for(i = 0; i < num_tile_columns_minus1; i++)
    {
      if((colWidth[i] << Log2CtbSizeY) < 256)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "ColumnWidthInLumaSamples shall be greater than or equal to 256");
      }
    }

    for(i = 0; i < num_tile_rows_minus1; i++)
    {
      if((rowHeight[i] << Log2CtbSizeY) < 64)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "RowHeightInLumaSamples shall be greater than or equal to 64");
      }
    }

    // Finally, find column boundaries for the tiles.
    for(colBd[0] = 0, i = 0; i <= num_tile_columns_minus1; i++)
    {
      colBd[i+1] = colBd[i] + colWidth[i];
    }

    for(i = 0; i < num_tile_columns_minus1+2; i++)
    {
      Sessiondata_p->colBd[i] = colBd[i];
    }

    // Finally, find row boundaries for the tiles.
    for(rowBd[0]=0, j=0; j <= num_tile_rows_minus1; j++)
    {
      rowBd[j+1] = rowBd[j] + rowHeight[j];
    }

    for(j = 0; j < num_tile_rows_minus1+2; j++)
    {
      Sessiondata_p->rowBd[j] = rowBd[j];
    }
  }

  Sessiondata_p->CurrentTileColumnMinus1   = num_tile_columns_minus1;
  Sessiondata_p->CurrentTileRowMinus1      = num_tile_rows_minus1;
  Sessiondata_p->CurrentUniformSpacingFlag = uniform_spacing;

  return tilingChanged;
}

/******************************************************************************
*
* Name:        ReallocationNeeded
*
* Parameters:  Sessiondata_p [in]      The decoder
*              NewSPS        [in]      The new activated SPS
*
* Returns:     uint8 - flag for signaling reallocation is needed
*
* Description: Check if we need to reallocate memory because of the new SPS
*
******************************************************************************/
uint8 
ReallocationNeeded(const SessionData_t          *Sessiondata_p,
                   const SequenceParameterSet_t *NewSPS)
{
  SequenceParameterSet_t *CurrSPS = Sessiondata_p->CurrentSequenceParameterSet;

  if(CurrSPS == NULL)
  {
    return 1;
  }

  if(NewSPS->PictureHeight != Sessiondata_p->AllocatedImageHeight ||
     NewSPS->PictureWidth  != Sessiondata_p->AllocatedImageWidth  ||
     NewSPS->DpbSize       != Sessiondata_p->AllocatedDpbSize)
  {
    return 1;
  }

  if(CurrSPS->Log2CtbSizeY     != NewSPS->Log2CtbSizeY   ||
     CurrSPS->Log2MinCbSizeY   != NewSPS->Log2MinCbSizeY ||
     CurrSPS->Log2MaxTrafoSize != NewSPS->Log2MaxTrafoSize)
  {
    return 1;
  }

  return 0;
}

/******************************************************************************
*
* Name:        ActivatePPS
*
* Parameters:  Sessiondata_p           [in]      The decoder
*              RapPicFlag              [in]      RAP picture flag
*              NAL_TYPE                [in]      Current NAL unit type
*              NoOutputOfPriorPicsFlag [in]      No output of prior pics flag
*              CurrPPS                 [in]      Current PPS (to be activated)
*
* Returns:    -
*
* Description: PPS activation process
*
******************************************************************************/
void
ActivatePPS(SessionData_t          *Sessiondata_p,
            const uint8             RapPicFlag,
            const NAL_TYPE          NALType,
            const uint32            NoOutputOfPriorPicsFlag,
            PictureParameterSet_t  *CurrPPS)
{
  SequenceParameterSet_t *NewSequenceParameterSet = Sessiondata_p->SequenceParameterSetArray[CurrPPS->SequenceParameterSetID];
  uint32 log2CtbSize;
  sint32 i;
  uint8  ResetDPBFlag  = 0;
  uint8  Reallocation  = 0;
  uint8  TilingChanged = 0;
  uint8  NewSPS        = 0;

  // Check for new sequence parameter sets. Changing parameter set is only
  // permitted for the first slice in an RAP picture
  if(NewSequenceParameterSet == NULL)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "non-existing sequence parameter set indicated");
  }

  if(NewSequenceParameterSet != Sessiondata_p->CurrentSequenceParameterSet)
  {
    if(!RapPicFlag || Sessiondata_p->NumberCTUDecodedForPic != 0)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "trying to change SPS in stream");
    }
    NewSPS = 1;
  }

  /* On RAP */
  if(RapPicFlag && (NALType != CRA_NUT || Sessiondata_p->PicOrderCntVal == -1))
  {
    ResetDPBFlag = 1;
    if(!NoOutputOfPriorPicsFlag)
    {
      FlushDPB(Sessiondata_p);
    }
  }

  if(ReallocationNeeded(Sessiondata_p, NewSequenceParameterSet))
  {
    if(!RapPicFlag || Sessiondata_p->NumberCTUDecodedForPic != 0)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SPS_ERROR, "trying to change SPS in stream");
    }

    // Handle the case when packets are lost and then a new SPS is recieved.
    Sessiondata_p->NumberCTUDecodedForPic = 0;
    FreeSequenceMemory(Sessiondata_p);
    Sessiondata_p->CurrentSequenceParameterSet = NewSequenceParameterSet;
    if(AllocateSequenceMemory(Sessiondata_p))
    {
      /* If allocation of sequence parameter set failed we free all sequence parameter sets */
      /* Decoding will resume when a correct SPS is received and allocation succeeds        */
      FreeSequenceMemory(Sessiondata_p);
      for(i = 0; i < MAX_SPS_NUM; i++)
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->SequenceParameterSetArray[i], D65_INTERNAL_DATA);
        Sessiondata_p->SequenceParameterSetArray[i] = NULL;
      }
      Sessiondata_p->CurrentSequenceParameterSet = NULL;
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Error when allocating sequence parameter set");
    }
    ResetDPBFlag = 1;
    Reallocation = 1;
    NewSPS       = 1;
  }

  if(NewSPS == 1)
  {
    Sessiondata_p->CurrentSequenceParameterSet = NewSequenceParameterSet;
    // Calculate how many pixels of last ctu:s are contained in picture (used in SAO filtering)
    log2CtbSize                        = NewSequenceParameterSet->Log2CtbSizeY;
    Sessiondata_p->PicWidthModCtbSize  = NewSequenceParameterSet->PictureWidth  % (1 << log2CtbSize);
    Sessiondata_p->PicHeightModCtbSize = NewSequenceParameterSet->PictureHeight % (1 << log2CtbSize);
  }

  if(ResetDPBFlag)
  {
    // New sequence parameter set is in use, we mark all decoded pictures
    // as available to enable the decoder to return one picture per DecodeNAL
    // call.
    for(i = 0; i < MAX_DPB_SIZE; i++)
    {
      Sessiondata_p->DecodedPictureBuffer[i].PictureMarking = UNUSED_FOR_REFERENCE; 
      Sessiondata_p->DecodedPictureBuffer[i].OutputMarking  = NOT_NEEDED_FOR_OUTPUT; 
      Sessiondata_p->DecodedPictureBuffer[i].PicOrderCnt    = -1;
    }
  }

  if(Sessiondata_p->CurrentSequenceParameterSet == NULL)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "SPS activation failed...Stop decoding current slice");
  }

  // Update Tiling boundaries
  TilingChanged = ComputeTileBoundaries(Sessiondata_p, CurrPPS, NewSPS);

  // Make sure this is done after tiling boundary checks
  Sessiondata_p->CurrentPictureParameterSet = CurrPPS;
  Sessiondata_p->Log2MinCuQpDeltaSize = Sessiondata_p->CurrentPictureParameterSet->CuQpDeltaEnabledFlag ? 
                                        Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - Sessiondata_p->CurrentPictureParameterSet->DiffCuQpDeltaDepth : 0;

  // Update Prediction Unit Array now since we have new SPS and PPS
  if(Reallocation || TilingChanged)
  {
#if MULTI_THREADED
    FreeTilesThreadMemory(Sessiondata_p);
    if(AllocateTilesThreadMemory(Sessiondata_p))
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Error when allocating memory for tiles threading data");
    }
#endif
    InitPredictionUnitArray(Sessiondata_p);
#if MULTI_THREADED
    //update the threaddata pointer in threadspool
    Sessiondata_p->ThreadsPool.JobsDataArray_p = Sessiondata_p->Threaddata_p;
#endif
  }
}

#if MULTI_THREADED
/******************************************************************************
*
* Name:        CompareJobs
*
* Parameters:  a   [in]  Job information
*              b   [in]  Job information
*
* Returns:     -1: Job a's complexity > Job b's complexity
*               0: Job a's complexity = Job b's complexity
*               1: Job a's complexity < Job b's complexity
*
* Description: Compares two jobs' complexity based on encoded bytes
*
******************************************************************************/
int CompareJobs(const void * a, const void * b)
{
  if(((JobsMapping*)a)->Bytes > ((JobsMapping*)b)->Bytes)
  {
    return -1;
  }
  else if(((JobsMapping*)a)->Bytes == ((JobsMapping*)b)->Bytes)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/******************************************************************************
*
* Name:        DecodeTileData
*
* Parameters:  VoidThreaddata_p   [in]    Thread decoding data
*
* Returns:
*
* Description: Tile data decoding function
*
******************************************************************************/
void*
DecodeTileData(void *VoidThreaddata_p)
{
  ThreadData_t  *Threaddata_p  = (ThreadData_t*)VoidThreaddata_p;
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;
  Parser_t      *Parser_p      = &Threaddata_p->Parser;
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  SubEncoder_t  *SubEncoder_p  = &Parser_p->SubEncoder;
#endif

  uint32 Log2CtuSizeInPixel    = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  uint8  CabacInitType         = Sessiondata_p->CurrentCabacInitFlag ? (uint8)CabacInitSliceLut[Sessiondata_p->CurrentSliceType] : Sessiondata_p->CurrentSliceType;
  uint32 TileCount             = 0;
  uint32 CtuY;
  uint32 CtuX;
  uint32 CtuStartY;
  uint32 CtuStopY;
  uint32 CtuStartX;
  uint32 CtuStopX;
  uint8  EndOfSliceSegmentFlag;
  uint8  EndOfSubstreamOneBit;

  D65_ExceptionCode_t ExceptionCode;

  ExceptionCode = (D65_ExceptionCode_t)setjmp(Threaddata_p->LongJumpSlice);
  if(ExceptionCode == 0)
  {
    Parser_p->LongJumpSlice = &Threaddata_p->LongJumpSlice;
    for(TileCount = 0; TileCount < Threaddata_p->TilesNumForThisThread; TileCount++)
    {
      CtuStartX = Threaddata_p->Tiledata_p[TileCount].TileStartCtuX;
      CtuStartY = Threaddata_p->Tiledata_p[TileCount].TileStartCtuY;
      CtuStopX  = Threaddata_p->Tiledata_p[TileCount].TileStopCtuX;
      CtuStopY  = Threaddata_p->Tiledata_p[TileCount].TileStopCtuY;

      //Reset CABAC
      ContextsBufferAssign(&Parser_p->CtxModels);
      ContextsInitialize(&Parser_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
      BacDecoderStart(Parser_p);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
      AssignBitsOutputer(&SubEncoder_p->BitsOutputer, Threaddata_p->Tiledata_p[TileCount].TileStream, 0, 0, 0);
      ResetBitsOutputer(&SubEncoder_p->BitsOutputer);

      ContextsBufferAssign(&SubEncoder_p->CtxModels);
      ContextsInitialize(&SubEncoder_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
      BacEncoderStart(&SubEncoder_p->BacEncoder);
#endif

      // Loop over all the CTUs in this tile
      // for each CTU, do the following:
      //   1 parse the syntax for the CTU, roughly following the syntax tables in the specification
      //   2 motion comp or intra prediction
      //   3 inverse transform
      //   4 reconstruct by adding prediction and residual and clip

      if(Sessiondata_p->CurrentSliceSaoLumaFlag || Sessiondata_p->CurrentSliceSaoChromaFlag)
      {
        for(CtuY = CtuStartY; CtuY < CtuStopY; CtuY++)
        {
          for(CtuX = CtuStartX; CtuX < CtuStopX; CtuX++)
          {
            Parse_sao_syntax(Threaddata_p, CtuX, CtuY);
            Parse_coding_quadtree(Threaddata_p, CtuX << Log2CtuSizeInPixel, CtuY << Log2CtuSizeInPixel, Log2CtuSizeInPixel, 0);
            EndOfSliceSegmentFlag = (uint8)GetBinTrm(Parser_p);
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
            if (((CtuY + 1) << Log2CtuSizeInPixel) < Sessiondata_p->CurrentSequenceParameterSet->PictureHeight ||
                ((CtuX + 1) << Log2CtuSizeInPixel) < Sessiondata_p->CurrentSequenceParameterSet->PictureWidth  )
            {
              EncodeBinTrm(SubEncoder_p, 0);
            }
#endif
          }
        }
        if(!EndOfSliceSegmentFlag)
        {
          EndOfSubstreamOneBit = (uint8)GetBinTrm(Parser_p);
          if(EndOfSubstreamOneBit == 0)
          {
            BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "EndOfSubstreamOneBit not equal to 1");
          }
        }

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
        EncodeBinTrm(SubEncoder_p, 1);
        BacEncoderFinish(SubEncoder_p);
        if (SubEncoder_p->OutputMode)
        {
          PutOutRbspTrailingBits(SubEncoder_p);
        }
#endif
      }
      else
      {
        for(CtuY = CtuStartY; CtuY < CtuStopY; CtuY++)
        {
          for(CtuX = CtuStartX; CtuX < CtuStopX; CtuX++)
          {
            Parse_coding_quadtree(Threaddata_p, CtuX << Log2CtuSizeInPixel, CtuY << Log2CtuSizeInPixel, Log2CtuSizeInPixel, 0);
            EndOfSliceSegmentFlag = (uint8)GetBinTrm(Parser_p);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
            if (((CtuY + 1) << Log2CtuSizeInPixel) < Sessiondata_p->CurrentSequenceParameterSet->PictureHeight ||
                ((CtuX + 1) << Log2CtuSizeInPixel) < Sessiondata_p->CurrentSequenceParameterSet->PictureWidth  )
            {
              EncodeBinTrm(SubEncoder_p, 0);
            }
#endif
          }
        }
        if(!EndOfSliceSegmentFlag)
        {
          EndOfSubstreamOneBit = (uint8)GetBinTrm(Parser_p);
          if(EndOfSubstreamOneBit == 0)
          {
            BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "EndOfSubstreamOneBit not equal to 1");
          }
        }
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
        EncodeBinTrm(SubEncoder_p, 1);
        BacEncoderFinish(SubEncoder_p);
        if (SubEncoder_p->OutputMode)
        {
          PutOutRbspTrailingBits(SubEncoder_p);
        }
#endif
      }

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
      Threaddata_p->Tiledata_p[TileCount].TileStreamSize = SubEncoder_p->BitsOutputer.BufferPos;
#endif
    }
  }
  return NULL;
}

#endif

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        DecodeSliceHeader
*
******************************************************************************/
void
DecodeSliceHeader(SessionData_t *Sessiondata_p,
                  const NAL_TYPE NALType,
                  const sint32 TemporalID,
                  uint32 *CTUnr,
                  sint32 *Quant)
{
  PictureParameterSet_t  *CurrentPictureParameterSet = NULL;
  Parser_t               *Parser_p                   = &Sessiondata_p->Parser;

  uint32 FirstSliceInPicFlag;
  uint32 PictureParameterSetID;
  sint32 PicOrderCntLSB;
  sint32 PicOrderCntVal;
  sint32 PicOrderCntMSB;
  sint32 PrevPicOrderCntLSB;
  sint32 PrevPicOrderCntMSB;
  sint32 MaxPicOrderCntLSB;
  uint32 NoOutputOfPriorPicsFlag = 0;
  uint32 no_output_of_prior_pics_flag = 0;
  sint32 num_ref_idx_l0_active_minus1;
  sint32 num_ref_idx_l1_active_minus1;
  uint32 mvd_l1_zero_flag;
  uint32 collocated_from_l0_flag;
  uint32 slice_disable_deblocking_filter_flag;
  uint32 slice_loop_filter_across_slices_enabled_flag;
  uint32 slice_temporal_mvp_enabled_flag = 0;
  uint32 slice_sao_luma_flag = 0;
  uint32 slice_sao_chroma_flag = 0;
  uint32 CabacInitFlag = 0;
  uint32 num_entry_points_offset = 0;
  sint32 entry_point_offset_minus1[32];
  sint32 offset_len;
  sint32 TotalBytesForTiles = 0;

  sint32 Value;
  sint32 i;
  uint32 SliceType;
  uint32 MaxNumMergeCand;
  sint32 collocated_ref_idx;
  ReferencePictureSet_t RpsInSliceHeader;
  //This is a temp reference picture set and it is used for storing the 
  //RPS set info when parsing the slice header.
  uint32 short_term_ref_pic_set_sps_flag;
  uint32 short_term_ref_pic_set_idx;
  uint32 num_long_term_pics;
  uint32 poc_lsb_lt;
  uint32 poc_lt;
  uint32 used_by_curr_pic_lt_flag;
  uint32 delta_poc_msb_present_flag;
  sint32 delta_poc_msb_cycle_lt;
  sint32 prev_delta_poc_msb_cycle_lt;
  //sint32 delta_poc_msb_cycle_lt;
  uint32 num_ref_idx_active_override_flag;
  uint32 NumLtInRps;
  uint32 NumStInRps;
  uint8 FreePicSlotInDpb;
  ReferencePicture_t* ReferencePicture_p = NULL;
#if SUPPORT_EXTENSIONS
  sint32 j;
#endif
  uint32 NewTileNumber = 0;
  uint8 RapPicFlag = (NALType >= BLA_W_LP) & (NALType <= RSV_IRAP_VCL23);
#if MULTI_THREADED
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  uint8  CurrEntryNeedsRewrite = 0;
  uint32 CurrEntryOffsetLen    = 0;
  const uint8* CurrBytePos     = NULL;
  uint32 CurrBitOffset         = 0;
  uint32 CurrByteValue         = 0;
#endif
  uint32 CurrentNumOfThreads   = 1;
#endif

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  uint8  OutputMode            = 0;

  if (NALType != IDR_W_RADL && NALType != IDR_N_LP && NALType != CRA_NUT)
  {
    OutputMode = 1;
  }

#endif

  FirstSliceInPicFlag = GetFlag(Parser_p, "SH - first_slice_in_pic_flag");
  if(FirstSliceInPicFlag == 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "first_slice_in_pic_flag=0 not supported yet");
  }

  if(RapPicFlag)
  {
    no_output_of_prior_pics_flag = GetFlag(Parser_p, "SH - no_output_of_prior_pics_flag");

    if(NALType == CRA_NUT && Sessiondata_p->PicOrderCntVal == -1)
    {
      NoOutputOfPriorPicsFlag = 1;
    }
    else
    {
      NoOutputOfPriorPicsFlag = no_output_of_prior_pics_flag;
    }
  }

  PictureParameterSetID = GetUVLC(Parser_p, "SH - pic_parameter_set_id");
  if(PictureParameterSetID > MAX_PPS_NUM)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "pic_parameter_set_id out-of-bounds");
  }

  /*******************************/
  /* Take care of PPS activation */
  /*******************************/

  if(Sessiondata_p->PictureParameterSetArray[PictureParameterSetID] == NULL)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_PPS_ERROR, "non-existing picture parameter set indicated");
  }
  CurrentPictureParameterSet = Sessiondata_p->PictureParameterSetArray[PictureParameterSetID];
  ActivatePPS(Sessiondata_p, RapPicFlag, NALType, NoOutputOfPriorPicsFlag, CurrentPictureParameterSet);

  /*******************************/
  /* End of PPS activation */
  /*******************************/
  *CTUnr = 0;
  if(!FirstSliceInPicFlag)
  {
#pragma message("TODO - Implement parsing of slice address here")
    //    *CTUnr = GetBits(Sessiondata_p, ?, "SH - slice_address");
  }


  SliceType = GetUVLC(Parser_p, "SH - slice_type"); // B=0 P=1 I=2
  if(SliceType > 2)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "Slice type unknown (not B, P or I)");
  }

  // if( output_flag_present_flag )
  //   pic_output_flag
  // if( separate_colour_plane_flag  = =  1 )
  //   colour_plane_id
  MaxPicOrderCntLSB = 1 << Sessiondata_p ->CurrentSequenceParameterSet->Log2MaxPicOrderCntLsb;
  if(NALType != IDR_W_RADL && NALType != IDR_N_LP)
  {
    PicOrderCntLSB = GetBits(Parser_p, Sessiondata_p->CurrentSequenceParameterSet->Log2MaxPicOrderCntLsb, "SH - pic_order_cnt_lsb");
    if(NALType == BLA_N_LP || NALType == BLA_W_LP || NALType == BLA_W_RADL)
    {
      PicOrderCntMSB = 0;
      PicOrderCntVal = PicOrderCntLSB;
    }
    else
    {
      PrevPicOrderCntLSB = Sessiondata_p->PrevTid0PicPoc % MaxPicOrderCntLSB;
      PrevPicOrderCntMSB = Sessiondata_p->PrevTid0PicPoc - PrevPicOrderCntLSB;

      if(PicOrderCntLSB < PrevPicOrderCntLSB && ((PrevPicOrderCntLSB - PicOrderCntLSB) >= (MaxPicOrderCntLSB >> 1)))
      {
        PicOrderCntMSB = PrevPicOrderCntMSB + MaxPicOrderCntLSB;
      }
      else if(PicOrderCntLSB > PrevPicOrderCntLSB&& ((PicOrderCntLSB - PrevPicOrderCntLSB) > (MaxPicOrderCntLSB >> 1)))
      {
        PicOrderCntMSB = PrevPicOrderCntMSB - MaxPicOrderCntLSB;
      }
      else
      {
        PicOrderCntMSB = PrevPicOrderCntMSB;
      }
      PicOrderCntVal = PicOrderCntMSB + PicOrderCntLSB;
    }

    short_term_ref_pic_set_sps_flag = GetFlag(Parser_p, "SH - short_term_ref_pic_set_sps_flag");
    if(!short_term_ref_pic_set_sps_flag)
    {
      ReferencePictureSet_t *RpsSetsInSps = Sessiondata_p->CurrentSequenceParameterSet->RPSsetInSPS;
      uint32 NumRpsSetsInSps = Sessiondata_p->CurrentSequenceParameterSet->num_short_term_rps_sets;
      ParseReferencePictureSet(Sessiondata_p, &RpsInSliceHeader, RpsSetsInSps, NumRpsSetsInSps, NumRpsSetsInSps);
    }
    else
    {
      uint32 Bits = 0;
      while((uint32)(1 << Bits) < Sessiondata_p->CurrentSequenceParameterSet->num_short_term_rps_sets)
      {
        Bits++;
      }
      if(Bits == 0)
      {
        short_term_ref_pic_set_idx = 0;
      }
      else
      {
        short_term_ref_pic_set_idx = GetBits(Parser_p, Bits, "SH - short_term_ref_pic_set_idx");
      }
      RpsInSliceHeader = Sessiondata_p->CurrentSequenceParameterSet->RPSsetInSPS[short_term_ref_pic_set_idx];
    }

    NumStInRps = RpsInSliceHeader.NumStRefPics;
    NumLtInRps = 0;
    if(Sessiondata_p->CurrentSequenceParameterSet->LongTermRefPicsPresentFlag)
    {
      if(Sessiondata_p->CurrentSequenceParameterSet->num_long_term_ref_pics_sps > 0)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "num_long_term_ref_pics_flag > 0 not supported yet");
      }
      num_long_term_pics = GetUVLC(Parser_p, "num_long_term_pics");
      if(NumStInRps + num_long_term_pics + 1 > MAX_DPB_SIZE)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_long_term_pics value too large!");
      }
      NumLtInRps += num_long_term_pics;
      prev_delta_poc_msb_cycle_lt = 0;
      for(i = 0; (uint32)i < NumLtInRps; i++)
      {
        poc_lsb_lt = GetBits(Parser_p, Sessiondata_p->CurrentSequenceParameterSet->Log2MaxPicOrderCntLsb,"SH - poc_lsb_lt");
        poc_lt = poc_lsb_lt;
        used_by_curr_pic_lt_flag = GetBits(Parser_p, 1 ,"SH - used_by_curr_pic_lt_flag");
        delta_poc_msb_present_flag = GetBits(Parser_p, 1, "SH - delta_poc_msb_present_flag");
        if(delta_poc_msb_present_flag)
        {
          delta_poc_msb_cycle_lt = GetUVLC(Parser_p, "SH - delta_poc_msb_cycle_lt");
          prev_delta_poc_msb_cycle_lt += delta_poc_msb_cycle_lt;
          poc_lt += PicOrderCntVal- PicOrderCntLSB - prev_delta_poc_msb_cycle_lt*MaxPicOrderCntLSB;
        }
        RpsInSliceHeader.UsedByCurrPicFlag[i+NumStInRps] = (uint8)used_by_curr_pic_lt_flag;
        RpsInSliceHeader.DeltaPocMsbFlag[i+NumStInRps] = (uint8)delta_poc_msb_present_flag;
        if(used_by_curr_pic_lt_flag)
        {
          RpsInSliceHeader.NumCurrRefPics++;
        }
        RpsInSliceHeader.PictureMarking[i+NumStInRps] = USED_FOR_LONG_TERM_REFERENCE;
        RpsInSliceHeader.DeltaPOC[i+NumStInRps] = poc_lt;
      }
    }
    RpsInSliceHeader.NumLtRefPics = NumLtInRps;

    if(Sessiondata_p->CurrentSequenceParameterSet->TemporalMvpEnabledFlag)
    {
      slice_temporal_mvp_enabled_flag = GetBits(Parser_p,1,"SH - slice_temproal_mvp_enabled_flag");
    }
  }
  else
  {
    PicOrderCntVal = 0;
    PicOrderCntLSB = 0;
  }
  if(Sessiondata_p->CurrentSequenceParameterSet->SampleAdaptiveOffsetEnabledFlag == 1)
  {
    slice_sao_luma_flag   = GetBits(Parser_p, 1, "SH - slice_sao_luma_flag");
    slice_sao_chroma_flag = GetBits(Parser_p, 1, "SH - slice_sao_chroma_flag");
  }

  if(SliceType == MODE_INTER || SliceType == MODE_B)
  {
    num_ref_idx_l0_active_minus1 = CurrentPictureParameterSet->num_ref_idx_l0_default_active_minus1;
    num_ref_idx_l1_active_minus1 = CurrentPictureParameterSet->num_ref_idx_l1_default_active_minus1;
    num_ref_idx_active_override_flag = GetBits(Parser_p, 1, "SH - num_ref_idx_active_override_flag");
    if(num_ref_idx_active_override_flag)
    {
      num_ref_idx_l0_active_minus1 = GetUVLC(Parser_p, "SH - num_ref_idx_l0_active_minus1");
      if(num_ref_idx_l0_active_minus1 > 14)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_ref_idx_l0_active_minus1 shall not exceed 14");
      }
      if(SliceType == MODE_B)
      {
        num_ref_idx_l1_active_minus1 = GetUVLC(Parser_p, "SH - num_ref_idx_l1_active_minus1");
        
        if(num_ref_idx_l1_active_minus1 > 14)
        {
          BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_ref_idx_l1_active_minus1 shall not exceed 14");
        }
      }
    }
    if(CurrentPictureParameterSet->lists_modification_present_flag &&
      RpsInSliceHeader.NumCurrRefPics > 1)
    {
      Value = GetBits(Parser_p, 1, "ref_pic_list_modification_flag_l0");
    }

    if(SliceType == MODE_B)
    {
      mvd_l1_zero_flag = GetBits(Parser_p, 1, "mvd_l1_zero_flag");
    }

    if(Sessiondata_p->CurrentPictureParameterSet->CabacInitFlag)
    {
      CabacInitFlag = GetFlag(Parser_p, "cabac_init_flag");
    }

    collocated_ref_idx = -1;
    if(slice_temporal_mvp_enabled_flag)
    {
      collocated_from_l0_flag = 1;
      if(SliceType == MODE_B)
      {
        collocated_from_l0_flag = GetBits(Parser_p, 1, "collocated_from_l0_flag");
      }

      collocated_ref_idx = 0;
      if(( collocated_from_l0_flag && num_ref_idx_l0_active_minus1 > 0) ||
         (!collocated_from_l0_flag && num_ref_idx_l1_active_minus1 > 0))
      {
        collocated_ref_idx = (sint32)GetUVLC(Parser_p, "SH - collocated_ref_idx");
        if(SliceType == MODE_INTER || (SliceType == MODE_B && collocated_from_l0_flag)) 
        {
          if(collocated_ref_idx > num_ref_idx_l0_active_minus1)
          {
            BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "collocated_ref_idx should not be larger than num_ref_idx_l0_active_minus1");
          }
        }
        else if(SliceType == MODE_B && !collocated_from_l0_flag)
        {
          if(collocated_ref_idx > num_ref_idx_l1_active_minus1)
          {
            BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "collocated_ref_idx should not be larger than num_ref_idx_l1_active_minus1");
          }
        }
      }
    }
    MaxNumMergeCand = 5 - GetUVLC(Parser_p, "SH - five_minus_max_num_merge_cand");
    if(MaxNumMergeCand > 5 || MaxNumMergeCand <1)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, " (5 - five_minus_max_num_merge_cand)  should be in the range of 1 to 5");
    }
  }

  /* Weighted prediction not yet supported. Parsing of pred_weight_table should be done here */

  // If framenumber har been increased since the previous slice and there are
  // some decoded macroblocks for the picture, we have decoded a part of a
  // picture but not put it in DPB yet. 
  //  if(Sessiondata_p->CurrentPictureData.FrameNumber != (sint32)CurrentFrameNumber && Sessiondata_p->NumberCTUDecodedForPic != 0)
  //  {
  //    PictureDecoded(Sessiondata_p, Sessiondata_p->PreviousSliceNALType);
  //  }

  *Quant = GetSVLC(Parser_p, "SH - slice_qp_delta");
  *Quant += Sessiondata_p->CurrentPictureParameterSet->PicInitQP;
  if(*Quant > 51 || *Quant < 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "slice_qp_delta out-of-bounds");
  }

  slice_disable_deblocking_filter_flag = CurrentPictureParameterSet->pic_disable_deblocking_filter_flag;
  if(CurrentPictureParameterSet->deblocking_filter_override_enabled_flag)
  {
    Value = GetBits(Parser_p, 1, "SH - deblocking_filter_override_flag");
    if(Value)
    {
      slice_disable_deblocking_filter_flag = GetBits(Parser_p, 1, "SH - slice_disable_deblocking_filter_flag");
      if(!slice_disable_deblocking_filter_flag)
      {
        sint32 beta = 0;
        sint32 tc   = 0;
        beta = GetSVLC(Parser_p, "slice_beta_offset_div2");
        tc   = GetSVLC(Parser_p, "slice_tc_offset_div2");
      }
    }
  }

  slice_loop_filter_across_slices_enabled_flag = CurrentPictureParameterSet->loop_filter_across_slices_enabled_flag;
  if(Sessiondata_p->CurrentPictureParameterSet->loop_filter_across_slices_enabled_flag &&
    (slice_sao_luma_flag || slice_sao_chroma_flag || !slice_disable_deblocking_filter_flag))
  {
    slice_loop_filter_across_slices_enabled_flag = GetBits(Parser_p, 1, "SH - slice_loop_filter_across_slices_enabled_flag");
  }

  if(CurrentPictureParameterSet->TilesEnabledFlag)
  {
    num_entry_points_offset = GetUVLC(Parser_p, "SH - num_entry_point_offsets");
    if(num_entry_points_offset > 32)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Number of entry points offset value out of bounds");
    }
    memset(entry_point_offset_minus1, 0, 32*sizeof(sint32));
    if(num_entry_points_offset != 0)
    {
      uint32 TilesNum = (CurrentPictureParameterSet->num_tile_rows_minus1    + 1) *
                        (CurrentPictureParameterSet->num_tile_columns_minus1 + 1);
      
      if(num_entry_points_offset + 1 > TilesNum)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_entry_points_offset out-of-bounds");
      }
      offset_len = GetUVLC(Parser_p, "SH - num_offset_len_minus1");
      if(offset_len > 31)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "offset_len_minus1 should be in range of 0 to 31");
      }
      offset_len += 1;
#if MULTI_THREADED
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
      CurrEntryNeedsRewrite = OutputMode ? 1 : 0;
      if (CurrEntryNeedsRewrite)
      {
        CurrEntryOffsetLen = offset_len;
        CurrBytePos        = (Parser_p->BitsReader.Buffer32_Pos == 0) ? Parser_p->BitsReader.CurrentBuffer_p : Parser_p->BitsReader.CurrentBuffer_p - 1;
        CurrBitOffset      = (Parser_p->BitsReader.Buffer32_Pos == 0) ? 0 : 7 - Parser_p->BitsReader.Buffer32_Pos + 1;
        CurrByteValue      = (Parser_p->BitsReader.Buffer32_Pos == 0) ? 0 : ((*(CurrBytePos) & 0xFF) - Parser_p->BitsReader.Buffer32) & 0xFF;
      }
#endif
#endif
      for(i = 0; i < (sint32)num_entry_points_offset; i++)
      {
        //parse entry point offset minus1
        Value = GetBits(Parser_p, offset_len, "SH - entry_point_offset_minus1");
        entry_point_offset_minus1[i] = Value + 1;
        //entry_point_offset_minus1[i] = Value;
        TotalBytesForTiles += entry_point_offset_minus1[i];
      }
    }
  }

  GetByteAlignmentBits(Parser_p);

  //Sessiondata_p->CurrentSequenceParameterSet = NewSequenceParameterSet;
  Sessiondata_p->CurrentPictureParameterSet  = CurrentPictureParameterSet;
  Sessiondata_p->Current_num_ref_idx_l0_active_minus1 = (sint8)num_ref_idx_l0_active_minus1;
  Sessiondata_p->Current_num_ref_idx_l1_active_minus1 = (sint8)num_ref_idx_l1_active_minus1;
  Sessiondata_p->no_output_of_prior_pics_flag = (uint8) no_output_of_prior_pics_flag;
  Sessiondata_p->CurrentSliceType = (uint8)SliceType;

  //printf("SliceType = %d\n ",  SliceType);
  Sessiondata_p->CurrentMaxNumMergeCand = (uint8)MaxNumMergeCand;
  Sessiondata_p->PicOrderCntVal = PicOrderCntVal;
  if(TemporalID == 0 &&
    NALType != TRAIL_N && NALType != TSA_N && NALType != STSA_N && 
    NALType != RADL_N && NALType != RADL_R &&
    NALType != RASL_N  && NALType != RASL_R)
  {
    Sessiondata_p->PrevTid0PicPoc = PicOrderCntVal;
  }

  Sessiondata_p->CurrentMvdL1ZeroFlag = (uint8)mvd_l1_zero_flag;

  //printf("POC = %d\n PrevPicMsb = %d PrevPicLsb = %d ",  Sessiondata_p->PicOrderCntVal, PrevPicOrderCntMSB, PrevPicOrderCntLSB);
  Sessiondata_p->CurrentLoopfilterAcrossSliceFlag = (uint8)slice_loop_filter_across_slices_enabled_flag;
  Sessiondata_p->CurrentSliceDisableDeblockingFlag = (uint8)slice_disable_deblocking_filter_flag;
  Sessiondata_p->CurrentSliceSaoLumaFlag = (uint8)slice_sao_luma_flag;
  Sessiondata_p->CurrentSliceSaoChromaFlag = (uint8)slice_sao_chroma_flag;
  Sessiondata_p->CurrentCabacInitFlag = (uint8)CabacInitFlag;
  if(RapPicFlag && (NALType != CRA_NUT || Sessiondata_p->PicOrderCntVal == -1)) 
  {
    FakePictureMarkingProcess(Sessiondata_p);
    FreePicSlotInDpb = GetFreePictureSlot(Sessiondata_p);
    Sessiondata_p->CurrentRecImage = &(Sessiondata_p->DecodedPictureBuffer[FreePicSlotInDpb]);
    Sessiondata_p->CurrentRecImage->PicOrderCnt = PicOrderCntVal;
    Sessiondata_p->CurrentRecImage->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
  }
  else
  {
    PictureMarkingProcess(Sessiondata_p, &RpsInSliceHeader);
    FreePicSlotInDpb = GetFreePictureSlot(Sessiondata_p);
    Sessiondata_p->CurrentRecImage = &(Sessiondata_p->DecodedPictureBuffer[FreePicSlotInDpb]);
    Sessiondata_p->CurrentRecImage->PicOrderCnt = PicOrderCntVal;
    Sessiondata_p->CurrentRecImage->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
  }
  if(Sessiondata_p->CurrentSliceType == MODE_INTER || Sessiondata_p->CurrentSliceType == MODE_B)
  {
    if(num_ref_idx_l0_active_minus1+1 > 0)
    {
      for(i=0; i< num_ref_idx_l0_active_minus1+1; i++)
      {
        if(Sessiondata_p->RefPicList0[i])
        {
          Sessiondata_p->CurrentRecImage->RefPicPOCList0[i]     = Sessiondata_p->RefPicList0[i]->PicOrderCnt;
          Sessiondata_p->CurrentRecImage->RefPicMarkingList0[i] = Sessiondata_p->RefPicList0[i]->PictureMarking;
        }
      }
#if SUPPORT_EXTENSIONS
      if (Sessiondata_p->LayerID != 0)
      {
        if (ReferenceSessiondata_p[0]->CurrentVideoParameterSet == NULL)
          BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_VPS_ERROR, "There is no VPS");
        for (i = (num_ref_idx_l0_active_minus1 + 1 - (sint32)ReferenceSessiondata_p[0]->CurrentVideoParameterSet->NumDirectRefLayers[ReferenceSessiondata_p[0]->CurrentVideoParameterSet->LayerIdVps[Sessiondata_p->LayerID]]), j = 0; i < (num_ref_idx_l0_active_minus1 + 1); i++, j++)
        {
          if(ReferenceSessiondata_p[ReferenceSessiondata_p[0]->CurrentVideoParameterSet->RefLayerId[ReferenceSessiondata_p[0]->CurrentVideoParameterSet->LayerIdVps[Sessiondata_p->LayerID]][j]]->CurrentRecImage->PicOrderCnt == Sessiondata_p->PicOrderCntVal)
          {
            Sessiondata_p->RefPicList0[i] = ReferenceSessiondata_p[ReferenceSessiondata_p[0]->CurrentVideoParameterSet->RefLayerId[ReferenceSessiondata_p[0]->CurrentVideoParameterSet->LayerIdVps[Sessiondata_p->LayerID]][j]]->CurrentRecImage;
            Sessiondata_p->RefPicList0[i]->PictureMarking = USED_FOR_LONG_TERM_REFERENCE;
            Sessiondata_p->CurrentRecImage->RefPicMarkingList0[i] = USED_FOR_LONG_TERM_REFERENCE;
            Sessiondata_p->LongTermMarking = true;
          }
          else
            BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Inter Layer reference picture not present in DPB!");
        }
      }
#endif
    }

    for(i=0; i< num_ref_idx_l0_active_minus1+1; i++)
    {
      if(Sessiondata_p->RefPicList1[i])
      {
        Sessiondata_p->CurrentRecImage->RefPicPOCList1[i]     = Sessiondata_p->RefPicList1[i]->PicOrderCnt;
        Sessiondata_p->CurrentRecImage->RefPicMarkingList1[i] = Sessiondata_p->RefPicList1[i]->PictureMarking;
      }
    }

    if(collocated_ref_idx != -1)
    {
      if(collocated_from_l0_flag)
      {
        ReferencePicture_p = Sessiondata_p->RefPicList0[collocated_ref_idx];
      }
      else
      {
        ReferencePicture_p = Sessiondata_p->RefPicList1[collocated_ref_idx];
      }
      if(ReferencePicture_p == NULL)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "collocated reference picture not available!");
      }
    }
  }

  Sessiondata_p->CurrentCollocatedRefIdx = (sint8)collocated_ref_idx;
  Sessiondata_p->CurrentCollocatedFromL0Flag = (uint8)collocated_from_l0_flag;

#if MULTI_THREADED
  Sessiondata_p->CurrentNumOfEntries = num_entry_points_offset + 1;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  Sessiondata_p->BytePosBeforeEntry   = CurrBytePos;
  Sessiondata_p->EntryOffsetLen       = CurrEntryOffsetLen;
  Sessiondata_p->BitOffsetBeforeEntry = CurrBitOffset;
  Sessiondata_p->ByteValue            = CurrByteValue;
  Sessiondata_p->EntryNeedRewrite     = CurrEntryNeedsRewrite;
#endif

  if(num_entry_points_offset > 0 && Sessiondata_p->CurrentNumOfThreads > 1)
  {
    // check entry point offset value
    if(TotalBytesForTiles > ((sint32)(Parser_p->BitsReader.BufferEnd_p - Parser_p->BitsReader.CurrentBuffer_p)))
    {
//#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
//      // do nothing
//#else
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "wrong entry point offset value!");
//#endif
    }

    Sessiondata_p->JobsMapArray[0].Id = 0;
    Sessiondata_p->JobsMapArray[0].Bytes = entry_point_offset_minus1[0];
    Sessiondata_p->Threaddata_p[0].TilesNumForThisThread = 1;
    Sessiondata_p->Threaddata_p[0].Tiledata_p = &Sessiondata_p->CurrentTiledata_p[0];
    InitBitsReader(&Sessiondata_p->Threaddata_p[0].Parser.BitsReader, Parser_p->BitsReader.CurrentBuffer_p, entry_point_offset_minus1[0]);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
    Sessiondata_p->Threaddata_p[0].Parser.SubEncoder.OutputMode = OutputMode;
    //AssignBitsOutputer(&Sessiondata_p->Threaddata_p[0].Parser.SubEncoder.BitsOutputer,
    //                    Sessiondata_p->CurrentTiledata_p[0].TileStream);
#endif

    for(i = 1; i < (sint32)num_entry_points_offset; i++)
    {
      const uint8 * PrevBufferEnd_p = Sessiondata_p->Threaddata_p[i-1].Parser.BitsReader.BufferEnd_p;
      Sessiondata_p->JobsMapArray[i].Id = i;
      Sessiondata_p->JobsMapArray[i].Bytes = entry_point_offset_minus1[i];
      Sessiondata_p->Threaddata_p[i].TilesNumForThisThread = 1;
      Sessiondata_p->Threaddata_p[i].Tiledata_p = &Sessiondata_p->CurrentTiledata_p[i];
      InitBitsReader(&Sessiondata_p->Threaddata_p[i].Parser.BitsReader, PrevBufferEnd_p, entry_point_offset_minus1[i]);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
      Sessiondata_p->Threaddata_p[i].Parser.SubEncoder.OutputMode = OutputMode;
      //AssignBitsOutputer(&Sessiondata_p->Threaddata_p[i].Parser.SubEncoder.BitsOutputer,
      //                    Sessiondata_p->CurrentTiledata_p[i].TileStream);
#endif
    }

    {
      const uint8 * PrevBufferEnd_p = Sessiondata_p->Threaddata_p[num_entry_points_offset - 1].Parser.BitsReader.BufferEnd_p;
      uint32 LastTileLength = Parser_p->BitsReader.BufferEnd_p - PrevBufferEnd_p;
      Sessiondata_p->JobsMapArray[num_entry_points_offset].Id = num_entry_points_offset;
      Sessiondata_p->JobsMapArray[num_entry_points_offset].Bytes = LastTileLength;
      Sessiondata_p->Threaddata_p[num_entry_points_offset].TilesNumForThisThread = 1;
      Sessiondata_p->Threaddata_p[num_entry_points_offset].Tiledata_p = &Sessiondata_p->CurrentTiledata_p[num_entry_points_offset];
      InitBitsReader(&Sessiondata_p->Threaddata_p[num_entry_points_offset].Parser.BitsReader, PrevBufferEnd_p, LastTileLength);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
      Sessiondata_p->Threaddata_p[num_entry_points_offset].Parser.SubEncoder.OutputMode = OutputMode;
      //AssignBitsOutputer(&Sessiondata_p->Threaddata_p[num_entry_points_offset].Parser.SubEncoder.BitsOutputer,
      //                    Sessiondata_p->CurrentTiledata_p[num_entry_points_offset].TileStream);
#endif
    }
  }
  else
  {
    Sessiondata_p->JobsMapArray[0].Id = 0;
    Sessiondata_p->JobsMapArray[0].Bytes = Parser_p->BitsReader.BufferEnd_p - Parser_p->BitsReader.CurrentBuffer_p;
    Sessiondata_p->Threaddata_p[0].TilesNumForThisThread = 1;
    Sessiondata_p->Threaddata_p[0].Tiledata_p = &Sessiondata_p->CurrentTiledata_p[0];
    if(Sessiondata_p->CurrentPictureParameterSet->TilesEnabledFlag)
    {
      Sessiondata_p->Threaddata_p[0].TilesNumForThisThread = 
        (Sessiondata_p->CurrentPictureParameterSet->num_tile_rows_minus1 + 1) *
        (Sessiondata_p->CurrentPictureParameterSet->num_tile_columns_minus1 + 1);
    }
    InitBitsReader(&Sessiondata_p->Threaddata_p[0].Parser.BitsReader, Parser_p->BitsReader.CurrentBuffer_p, Sessiondata_p->JobsMapArray[0].Bytes);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
    Sessiondata_p->Threaddata_p[0].Parser.SubEncoder.OutputMode = OutputMode;
    //AssignBitsOutputer(&Sessiondata_p->Threaddata_p[0].Parser.SubEncoder.BitsOutputer,
    //                    Sessiondata_p->CurrentTiledata_p[0].TileStream);
#endif
  }
#else
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  Sessiondata_p->Parser.SubEncoder.OutputMode = OutputMode;
#endif
#endif
}

/******************************************************************************
*
* Name:        DecodeSliceData
*
******************************************************************************/
D65_ReturnCode_t
DecodeSliceData(SessionData_t *Sessiondata_p,
                const NAL_TYPE NALType,
                const uint32 SliceStartCTUnr,
                sint32 Quant)
{
#if MULTI_THREADED
  const uint32 PicHeightInCtbsY = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
  const uint32 PicWidthInCtbsY  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;

  uint32 NumEntries = Sessiondata_p->CurrentNumOfEntries;
  ThreadPool_t *Threadpool_p = &Sessiondata_p->ThreadsPool;
  BitsReader_t *Bitsreader_p = &Sessiondata_p->Parser.BitsReader;
  Sessiondata_p->CurrentQuant = Quant;

  if(Sessiondata_p->CurrentNumOfThreads > 1)
  {
    if(NumEntries > 1)
    {
      qsort(Sessiondata_p->JobsMapArray, NumEntries, sizeof(Sessiondata_p->JobsMapArray[0]),CompareJobs);
    }
    pthread_mutex_lock(&Threadpool_p->ThreadLock);
    Threadpool_p->Jobs_p = DecodeTileData;
    Threadpool_p->RemainingJobsNumber = NumEntries;
    Threadpool_p->CurrentJobId = 0;
    Threadpool_p->TotalJobNumber = NumEntries;
    pthread_cond_broadcast(&Threadpool_p->JobsArrayNotEmpty);
    pthread_cond_wait(&Threadpool_p->JobsArrayEmpty, &Threadpool_p->ThreadLock);
    pthread_mutex_unlock(&Threadpool_p->ThreadLock);

    Bitsreader_p->CurrentBuffer_p = Sessiondata_p->Threaddata_p[NumEntries-1].Parser.BitsReader.CurrentBuffer_p;
    Bitsreader_p->Buffer32_Pos    = Sessiondata_p->Threaddata_p[NumEntries-1].Parser.BitsReader.Buffer32_Pos;
  }
  else
  {
    DecodeTileData(&Sessiondata_p->Threaddata_p[0]);
    Bitsreader_p->CurrentBuffer_p = Sessiondata_p->Threaddata_p[0].Parser.BitsReader.CurrentBuffer_p;
    Bitsreader_p->Buffer32_Pos    = Sessiondata_p->Threaddata_p[0].Parser.BitsReader.Buffer32_Pos;
  }

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
    {
      uint32 i, j;
      uint32 id = 0;

      if (Sessiondata_p->EntryNeedRewrite)
      {
        SubEncoder_t *SubEncoder_p = &Sessiondata_p->Parser.SubEncoder;
        AssignBitsOutputer(&SubEncoder_p->BitsOutputer,
                           Sessiondata_p->NalOutputBuffer,
                           Sessiondata_p->NalOutputBufferPos,
                           Sessiondata_p->ByteValue,
                           Sessiondata_p->BitOffsetBeforeEntry);

        for (id = 0; id < Sessiondata_p->CurrentTileNum - 1; id++)
        {
            PutOutBits(SubEncoder_p,
                       Sessiondata_p->EntryOffsetLen,
                       Sessiondata_p->CurrentTiledata_p[id].TileStreamSize - 1);
        }

        PutOutRbspTrailingBits(SubEncoder_p);
        Sessiondata_p->NalOutputBufferPos = SubEncoder_p->BitsOutputer.BufferPos;
      }

      id = 0;
      for (i = 0; i <= Sessiondata_p->CurrentPictureParameterSet->num_tile_rows_minus1; i++)
      {
        for(j = 0; j <= Sessiondata_p->CurrentPictureParameterSet->num_tile_columns_minus1; j++, id++)
        {
          memcpy(Sessiondata_p->NalOutputBuffer + Sessiondata_p->NalOutputBufferPos,
                 Sessiondata_p->CurrentTiledata_p[id].TileStream,
                 Sessiondata_p->CurrentTiledata_p[id].TileStreamSize);
          Sessiondata_p->NalOutputBufferPos += Sessiondata_p->CurrentTiledata_p[id].TileStreamSize;
          Sessiondata_p->CurrentTiledata_p[id].TileStreamSize = 0;
        }
      }
    }
#endif

#else
  sint32 CTUCoordinateY;
  sint32 CTUCoordinateX;
  sint32 CTUStartY;
  sint32 CTUEndY;
  sint32 CTUStartX;
  sint32 CTUEndX;
  uint32 TerminateBit;
  uint32 maxTilesX;
  uint32 maxTilesY;
  uint32 tileCoordX;
  uint32 tileCoordY;
  const PictureParameterSet_t *CurrPicSet = Sessiondata_p->CurrentPictureParameterSet;
  Parser_t     *Parser_p     = &Sessiondata_p->Parser;
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  SubEncoder_t *SubEncoder_p = &Sessiondata_p->Parser.SubEncoder;
#endif
  const uint32 CTUsize = 1<<Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const sint32 PicHeightInCtbsY = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
  const sint32 PicWidthInCtbsY  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
  uint8 CabacInitType = Sessiondata_p->CurrentCabacInitFlag ? (uint8)CabacInitSliceLut[Sessiondata_p->CurrentSliceType] : Sessiondata_p->CurrentSliceType;

  Sessiondata_p->CurrentQuant = Quant;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  // qq
  //SubEncoder_p->OutputMode   = 0;
  //if (NALType != IDR_W_RADL && NALType != IDR_N_LP && NALType != CRA_NUT)
  //{
  //  SubEncoder_p->OutputMode = 1;
  //}
  ResetBitsOutputer(&SubEncoder_p->BitsOutputer);
#endif

  if(CurrPicSet->TilesEnabledFlag == 1)
  {
    maxTilesX = CurrPicSet->num_tile_columns_minus1 + 1;
    maxTilesY = CurrPicSet->num_tile_rows_minus1    + 1;

    for(tileCoordY = 0; tileCoordY < maxTilesY; tileCoordY++)
    {
      for(tileCoordX = 0; tileCoordX < maxTilesX; tileCoordX++)
      {
        // Get start and end CTU coordinates for the tile 
        CTUStartX = Sessiondata_p->colBd[tileCoordX];
        CTUEndX  = Sessiondata_p->colBd[tileCoordX+1];
        CTUStartY = Sessiondata_p->rowBd[tileCoordY];
        CTUEndY  = Sessiondata_p->rowBd[tileCoordY+1];

        // Reset CABAC
        ContextsInitialize(&Parser_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
        BacDecoderStart(Parser_p);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
        ContextsInitialize(&SubEncoder_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
        BacEncoderStart(&SubEncoder_p->BacEncoder);
#endif
        // for loop over the CTUs in the tile
        // for each CTU, do the following (order of 2,3,4 not 100% decided yet):
        //// 1 parse the syntax for the CTU, roughly following the syntax tables in the specification
        //// 2 inverse transform
        //// 3 motion comp or intra prediction
        //// 4 reconstruct by adding prediction and residual and clip
        for(CTUCoordinateY = CTUStartY; CTUCoordinateY < CTUEndY; CTUCoordinateY++)
        {
          for(CTUCoordinateX = CTUStartX ; CTUCoordinateX < CTUEndX; CTUCoordinateX++)
          {
            if(Sessiondata_p->CurrentSliceSaoLumaFlag || Sessiondata_p->CurrentSliceSaoChromaFlag)
            {
              Parse_sao_syntax(Sessiondata_p, CTUCoordinateX, CTUCoordinateY);
            }
            Parse_coding_quadtree(Sessiondata_p, CTUsize * CTUCoordinateX, CTUsize * CTUCoordinateY,Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY,0);
            TerminateBit = GetBinTrm(Parser_p);
            Sessiondata_p->NumberCTUDecodedForPic++;
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
            if (Sessiondata_p->NumberCTUDecodedForPic != PicHeightInCtbsY*PicWidthInCtbsY)
            {
              EncodeBinTrm(SubEncoder_p, 0);
            }
#endif
          }
        }
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
        EncodeBinTrm(SubEncoder_p, 1);
        BacEncoderFinish(SubEncoder_p);
        if (SubEncoder_p->OutputMode)
        {
          PutOutRbspTrailingBits(SubEncoder_p);
        }
#endif
      }
    }
  }
  else
  {
    CTUStartY = 0;
    CTUEndY  = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
    CTUStartX = 0;
    CTUEndX  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY;
    // Reset CABAC
    ContextsInitialize(&Parser_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
    BacDecoderStart(Parser_p);
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
    ContextsInitialize(&SubEncoder_p->CtxModels, CabacInitType, Sessiondata_p->CurrentQuant);
    BacEncoderStart(&SubEncoder_p->BacEncoder);
#endif
    // for loop over the CTUs in the picture
    // for each CTU, do the following (order of 2,3,4 not 100% decided yet):
    //// 1 parse the syntax for the CTU, roughly following the syntax tables in the specification
    //// 2 inverse transform
    //// 3 motion comp or intra prediction
    //// 4 reconstruct by adding prediction and residual and clip
    for(CTUCoordinateY = CTUStartY; CTUCoordinateY < CTUEndY; CTUCoordinateY++)
    {
      for(CTUCoordinateX = CTUStartX; CTUCoordinateX < CTUEndX; CTUCoordinateX++)
      {
        if(Sessiondata_p->CurrentSliceSaoLumaFlag || Sessiondata_p->CurrentSliceSaoChromaFlag)
        {
          Parse_sao_syntax(Sessiondata_p, CTUCoordinateX, CTUCoordinateY);
        }
        Parse_coding_quadtree(Sessiondata_p, CTUsize * CTUCoordinateX, CTUsize * CTUCoordinateY,Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY,0);
        TerminateBit = GetBinTrm(Parser_p);
        Sessiondata_p->NumberCTUDecodedForPic++;
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
        if (Sessiondata_p->NumberCTUDecodedForPic != PicHeightInCtbsY*PicWidthInCtbsY)
        {
          EncodeBinTrm(SubEncoder_p, 0);
        }
#endif
      }
    }
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
    EncodeBinTrm(SubEncoder_p, 1);
    BacEncoderFinish(SubEncoder_p);
    if (SubEncoder_p->OutputMode)
    {
      PutOutRbspTrailingBits(SubEncoder_p);
    }
#endif
  }

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  if (SubEncoder_p->OutputMode)
  {
    memcpy(Sessiondata_p->NalOutputBuffer + Sessiondata_p->NalOutputBufferPos,
           SubEncoder_p->BitsOutputer.BufferStart_p,
           SubEncoder_p->BitsOutputer.BufferPos);
    Sessiondata_p->NalOutputBufferPos += SubEncoder_p->BitsOutputer.BufferPos;
  }
#endif

#endif
  return D65_PICTURE_DECODED;

  if(Sessiondata_p->NumberCTUDecodedForPic == PicHeightInCtbsY*PicWidthInCtbsY)
  {
    return D65_PICTURE_DECODED;
  }
  else
  {
    return D65_SLICE_DECODED;
  }
}