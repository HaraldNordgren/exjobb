/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains DPB releated functions: RPS parsing, reference picture
* marking process and output reconstructed picture process
*
******************************************************************************/
#include <string.h>
#include "d65_dpb.h"
#include "d65_getbits.h"
#include "d65_deblocking_filter.h"
#include "d65_sample_adaptive_offset.h"


/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        PicturePadding
*
* Parameters:  Image_p       [in]  The image that shall be padded
*
* Returns:     -
*
* Description: Performs padding of an image.
*
******************************************************************************/
void
PicturePadding(YUVImage_t *Image_p)
{
  uint32 Y_Width_Padded   = Image_p->Y_Width_image;
  uint32 Y_Height_Padded  = Image_p->Y_Height_image;
  uint32 C_Width_Padded   = Image_p->C_Width_image;
  uint32 C_Height_Padded  = Image_p->C_Height_image;
  uint8 *Y_Rec  = Image_p->Y  + PADDING * Y_Width_Padded + PADDING;
  uint8 *Cb_Rec = Image_p->Cb + (PADDING>>1)*C_Width_Padded + (PADDING >> 1);
  uint8 *Cr_Rec = Image_p->Cr + (PADDING>>1)*C_Width_Padded + (PADDING >> 1);
  sint32 y;

  for(y = 0; y < (sint32)PADDING; y++)
  {
    memcpy(Image_p->Y + y*Y_Width_Padded + PADDING, Y_Rec, (Y_Width_Padded- 2*PADDING) *sizeof(uint8));
    memcpy(Image_p->Y + (y + Y_Height_Padded - PADDING)*Y_Width_Padded + PADDING, Y_Rec + (Y_Height_Padded - 2*PADDING - 1)*Y_Width_Padded,(Y_Width_Padded- 2*PADDING) *sizeof(uint8));
  }
  for(y = 0; y < (sint32)Y_Height_Padded; y++)
  {
    memset(Image_p->Y + y*Y_Width_Padded, Image_p->Y[y*Y_Width_Padded + PADDING], PADDING*sizeof(uint8));
    memset(Image_p->Y + y*Y_Width_Padded + Y_Width_Padded - PADDING, Image_p->Y[y*Y_Width_Padded+ Y_Width_Padded - PADDING - 1],PADDING*sizeof(uint8));
  }

  for(y = 0; y < (sint32)(PADDING >> 1); y++)
  {
    memcpy(Image_p->Cb + y*C_Width_Padded + (PADDING >> 1), Cb_Rec, (C_Width_Padded- 2*(PADDING >> 1))*sizeof(uint8));
    memcpy(Image_p->Cb + (y + C_Height_Padded - (PADDING >> 1))*C_Width_Padded + (PADDING >> 1), Cb_Rec + (C_Height_Padded - 2*(PADDING>>1)-1)*C_Width_Padded,(C_Width_Padded- 2*(PADDING >> 1)) *sizeof(uint8));
    memcpy(Image_p->Cr + y*C_Width_Padded + (PADDING >> 1), Cr_Rec, (C_Width_Padded- 2*(PADDING >> 1))*sizeof(uint8));
    memcpy(Image_p->Cr + (y + C_Height_Padded - (PADDING >> 1))*C_Width_Padded + (PADDING >> 1), Cr_Rec + (C_Height_Padded - 2*(PADDING>>1)-1)*C_Width_Padded,(C_Width_Padded- 2*(PADDING >> 1)) *sizeof(uint8));
  }
  for(y = 0; y < (sint32)C_Height_Padded; y++)
  {
    memset(Image_p->Cb + y*C_Width_Padded, Image_p->Cb[y*C_Width_Padded + (PADDING >> 1)], (PADDING >> 1)*sizeof(uint8));
    memset(Image_p->Cb + y*C_Width_Padded + C_Width_Padded - (PADDING >> 1), Image_p->Cb[y*C_Width_Padded+ C_Width_Padded - (PADDING >> 1) - 1],(PADDING >> 1)*sizeof(uint8));
    memset(Image_p->Cr + y*C_Width_Padded, Image_p->Cr[y*C_Width_Padded + (PADDING >> 1)], (PADDING >> 1)*sizeof(uint8));
    memset(Image_p->Cr + y*C_Width_Padded + C_Width_Padded - (PADDING >> 1), Image_p->Cr[y*C_Width_Padded+ C_Width_Padded - (PADDING >> 1) - 1],(PADDING >> 1)*sizeof(uint8));
  }
}

/******************************************************************************
*
* Name:        CropReturnPicture
*
* Parameters:  Sessiondata_p [in]  The decoder
*              Image_p       [in]  The image that shall be cropped
*
* Returns:     -
*
* Description: Performs cropping of an image, and stores the cropped picture
*              in Sessiondata_p->DecodedPicture.
*
******************************************************************************/
void
CropReturnPicture(SessionData_t *Sessiondata_p,
                  const YUVImage_t *Image_p)
{
  uint16 pic_crop_left_offset;
  uint16 pic_crop_right_offset;
  uint16 pic_crop_top_offset;
  uint16 pic_crop_bottom_offset;

  uint32 Y_Width_image = Image_p->Y_Width_image;
  uint32 C_Width_image = Image_p->C_Width_image;

  Sessiondata_p->DecodedPicture.UserPictureValue = Image_p->UserPictureValue;
  Sessiondata_p->DecodedPicture.PictureType = Image_p->PictureType;

  pic_crop_left_offset  = Sessiondata_p->CurrentSequenceParameterSet->pic_crop_left_offset;
  pic_crop_right_offset = Sessiondata_p->CurrentSequenceParameterSet->pic_crop_right_offset;
  pic_crop_top_offset   = Sessiondata_p->CurrentSequenceParameterSet->pic_crop_top_offset;
  pic_crop_bottom_offset= Sessiondata_p->CurrentSequenceParameterSet->pic_crop_bottom_offset;

  Sessiondata_p->DecodedPicture.Y  = Image_p->Y  + 2*pic_crop_top_offset*Y_Width_image + 2*pic_crop_left_offset + PADDING*Y_Width_image + PADDING;
  Sessiondata_p->DecodedPicture.Cb = Image_p->Cb +   pic_crop_top_offset*C_Width_image +   pic_crop_left_offset + (PADDING >> 1)*C_Width_image + (PADDING >> 1);
  Sessiondata_p->DecodedPicture.Cr = Image_p->Cr +   pic_crop_top_offset*C_Width_image +   pic_crop_left_offset + (PADDING >> 1)*C_Width_image + (PADDING >> 1);

  Sessiondata_p->DecodedPicture.Y_Width_memory = Image_p->Y_Width_image;

  Sessiondata_p->DecodedPicture.Y_Height_image = Image_p->Y_Height_image - 2*(pic_crop_top_offset+pic_crop_bottom_offset) - 2*PADDING;
  Sessiondata_p->DecodedPicture.Y_Width_image  = Image_p->Y_Width_image  - 2*(pic_crop_left_offset+pic_crop_right_offset) - 2*PADDING;

  Sessiondata_p->ReturnedPictureFlag = 1;

}

/******************************************************************************
*
* Name:        GetImageWithLowestPOC
*
* Parameters:  Sessiondata_p [in]  The decoder
*
* Returns:     ReferencePicture_t  The image with the lowest POC
*
* Description: Finds the image needed for output with the lowest POC in the
*              DPB.
*
******************************************************************************/
ReferencePicture_t *
GetImageWithLowestPOC(SessionData_t *Sessiondata_p)
{
  ReferencePicture_t *Image_p;
  sint32 LowestPOC;
  sint16 i;

  Image_p = NULL;
  LowestPOC = 1<<30;
  for(i=0 ; i<Sessiondata_p->CurrentSequenceParameterSet->DpbSize ; i++)
  {
    if(Sessiondata_p->DecodedPictureBuffer[i].OutputMarking == NEEDED_FOR_OUTPUT)
    {
      if(Sessiondata_p->DecodedPictureBuffer[i].PicOrderCnt < LowestPOC)
      {
        Image_p = &(Sessiondata_p->DecodedPictureBuffer[i]);
        LowestPOC = Sessiondata_p->DecodedPictureBuffer[i].PicOrderCnt;
      }
    }
  }
  return Image_p;
}

/******************************************************************************
*
* Name:        FetchPictureInDpb
*
* Parameters:  Sessiondata_p [in]  The decoder
*
* Returns:     ReferencePicture_t  The image
*
* Description: Finds an image according to the POC value
*
******************************************************************************/
ReferencePicture_t *
FetchPictureInDpb(SessionData_t *Sessiondata_p, sint32 POC, uint8 CheckLsb)
{
  uint32 DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  uint32 i;
  sint32 RefPOC;

  for(i = 0 ; i < DpbSize; i++)
  {
    RefPOC = Sessiondata_p->DecodedPictureBuffer[i].PicOrderCnt;
    if(CheckLsb)
    {
      RefPOC = RefPOC % (1 << Sessiondata_p->CurrentSequenceParameterSet->Log2MaxPicOrderCntLsb);
    }
    if(RefPOC == POC && Sessiondata_p->DecodedPictureBuffer[i].PictureMarking != UNUSED_FOR_REFERENCE)
    {
      return &(Sessiondata_p->DecodedPictureBuffer[i]);
    }
  }
  return NULL;
}

/******************************************************************************
*
* Name:        PrepareRefPicMarking
*
* Parameters:  Sessiondata_p [in]  The decoder
*
* Returns:     -
*
* Description: Reset marking flags and reference picture list.
*
******************************************************************************/
void
PrepareRefPicMarking(SessionData_t *Sessiondata_p)
{
  uint32 DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  uint32 i;

  for(i = 0 ; i < DpbSize; i++)
  {
    Sessiondata_p->DecodedPictureBuffer[i].RefMarkingUpdated = 0;
    Sessiondata_p->RefPicList0[i] = NULL;
    Sessiondata_p->RefPicList1[i] = NULL;
  }
}

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        GetFreePictureSlot
*
******************************************************************************/
uint8
GetFreePictureSlot(SessionData_t *Sessiondata_p)
{
  sint32 FreeSlotIndex;
  sint32 i;
  uint32  DpbSize;

  DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  FreeSlotIndex = -1;
  for(i = 0; i < (sint32)DpbSize; i++)
  {
    if(Sessiondata_p->DecodedPictureBuffer[i].OutputMarking == NOT_NEEDED_FOR_OUTPUT &&
      Sessiondata_p->DecodedPictureBuffer[i].PictureMarking == UNUSED_FOR_REFERENCE)
    {
      FreeSlotIndex = i;
      break;
    }
  }
  D65_ASSERT(FreeSlotIndex >= 0, "Internal error - There will always be a free slot here");
  return (uint8)FreeSlotIndex;
}

/*****************************************************************************
*
* Name:            FlushDPB
*
*****************************************************************************/

void
FlushDPB(SessionData_t *Sessiondata_p)
{
#if D65_CONFORMANCE
  ReferencePicture_t *Image_p;
  
  if(Sessiondata_p->AllocatedDpbSize == 0)
    return;

  while((Image_p = GetImageWithLowestPOC(Sessiondata_p)) != NULL)
  {
    // Crop picture and set up the Sessiondata_p->DecodedPicture struct
    CropReturnPicture(Sessiondata_p,&Image_p->Image);
    Image_p->OutputMarking = NOT_NEEDED_FOR_OUTPUT;
    
    Sessiondata_p->Write_Output_Function_p(&(Sessiondata_p->DecodedPicture), Sessiondata_p->OutputLocation);
  }
#endif
}


/******************************************************************************
*
* Name:        ParseReferencePictureSet
*
******************************************************************************/
void
ParseReferencePictureSet(SessionData_t *Sessiondata_p,
                         ReferencePictureSet_t *Rps_p,
                         ReferencePictureSet_t *RpsPredArray_p,
                         uint32 NumStRefPicSets,
                         uint32 idxRps)
{
  sint32 Value;
  uint32 num_negative_pics;
  uint32 num_positive_pics;
  uint32 inter_ref_pic_set_prediction_flag;
  uint32 i;
  sint32 prev_delta_poc;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  inter_ref_pic_set_prediction_flag = 0;
  if(idxRps!= 0)
  {
    inter_ref_pic_set_prediction_flag = GetBits(Parser_p, 1, "RPS - inter_ref_pic_set_prediction_flag");
  }
  if(inter_ref_pic_set_prediction_flag)
  {
    uint32 DeltaIdx = 1;
    uint32 DeltaRpsSign;
    uint32 AbsDetaRps;
    sint32 DeltaRps;
    uint8 UsedByCurrPicFlag;
    uint8 UseDeltaFlag;
    uint8 NumRefPics = 0;
    sint32 k1,k2;
    sint32 DeltaPOC;
    ReferencePictureSet_t *RpsPred_p = NULL;
    if(idxRps == NumStRefPicSets)
    {
      DeltaIdx = GetUVLC(Parser_p, "RPS - delta_idx_minus1") + 1;
    }
    DeltaRpsSign = GetBits(Parser_p, 1, "RPS - delta_rps_sign");
    AbsDetaRps = GetUVLC(Parser_p, "RPS - abs_delta_rps_minus1") + 1;
    DeltaRps = (1 - 2*DeltaRpsSign)*AbsDetaRps;
    RpsPred_p = RpsPredArray_p + idxRps - DeltaIdx;
    num_negative_pics = 0;
    for(i = 0; i <= RpsPred_p->NumStRefPics; i++)
    {
      UsedByCurrPicFlag = (uint8)GetBits(Parser_p, 1, "RPS - used_by_curr_pic_flag[i]");
      UseDeltaFlag = 1;
      if(!UsedByCurrPicFlag)
      {
        UseDeltaFlag = (uint8)GetBits(Parser_p, 1, "RPS - use_delta_flag[j]");
      }
      if(UsedByCurrPicFlag || UseDeltaFlag)
      {
        DeltaPOC = ((i == RpsPred_p->NumStRefPics) ? 0 : RpsPred_p->DeltaPOC[i]) + DeltaRps;
        Rps_p->DeltaPOC[NumRefPics]           = DeltaPOC;
        Rps_p->UsedByCurrPicFlag[NumRefPics] = UsedByCurrPicFlag;
        Rps_p->PictureMarking[NumRefPics++]  = USED_FOR_SHORT_TERM_REFERENCE;
        if(DeltaPOC < 0)
        {
          num_negative_pics++;
        }
      }
    }
    Rps_p->NumCurrRefPics = NumRefPics;
    Rps_p->NumStRefPics   = NumRefPics;
    if(NumRefPics > 1)
    {
      for(k1 = 1; k1 < (sint32)NumRefPics; k1++)
      {
        DeltaPOC = Rps_p->DeltaPOC[k1];
        UsedByCurrPicFlag = Rps_p->UsedByCurrPicFlag[k1];
        for(k2 = k1 - 1; k2 >= 0; k2--)
        {
          if(DeltaPOC < Rps_p->DeltaPOC[k2])
          {
            Rps_p->DeltaPOC[k2+1] = Rps_p->DeltaPOC[k2];
            Rps_p->UsedByCurrPicFlag[k2+1] = Rps_p->UsedByCurrPicFlag[k2];
            Rps_p->DeltaPOC[k2] = DeltaPOC;
            Rps_p->UsedByCurrPicFlag[k2] = UsedByCurrPicFlag;
          }
        }
      }
      for(k1 = 0, k2 = (sint32)num_negative_pics - 1; k1 < ((sint32)num_negative_pics >> 1); k1++, k2--)
      {
        DeltaPOC = Rps_p->DeltaPOC[k1];
        UsedByCurrPicFlag = Rps_p->UsedByCurrPicFlag[k1];
        Rps_p->DeltaPOC[k1] = Rps_p->DeltaPOC[k2];
        Rps_p->UsedByCurrPicFlag[k1] = Rps_p->UsedByCurrPicFlag[k2];
        Rps_p->DeltaPOC[k2] = DeltaPOC;
        Rps_p->UsedByCurrPicFlag[k2] = UsedByCurrPicFlag;
      }
    }
  }
  else
  {
    num_negative_pics = GetUVLC(Parser_p, "RPS - num_negative_pics");
    if(num_negative_pics + 1 > MAX_DPB_SIZE)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_negative_pics value exceeds the MaxDpbSize - 1");
    }
    num_positive_pics = GetUVLC(Parser_p, "RPS - num_positive_pics");
    if(num_positive_pics + num_negative_pics + 1 > MAX_DPB_SIZE)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_positive_pics value exceeds the MaxDpbSize - 1 - num_negative_pics");
    }
    prev_delta_poc = 0;
    Rps_p->NumCurrRefPics = 0;
    for(i = 0; i < num_negative_pics; i++)
    {
      Value = GetUVLC(Parser_p, "RPS - delta_poc_s0_minus1");
      prev_delta_poc -= (Value + 1);
      Rps_p->DeltaPOC[i] = prev_delta_poc;
      Value = GetBits(Parser_p, 1, "RPS - used_by_curr_pic_s0_flag");
      Rps_p->UsedByCurrPicFlag[i] = (uint8)Value;
      if(Value)
      {
        Rps_p->NumCurrRefPics++;
      }
    }
    prev_delta_poc = 0;
    for(i = num_negative_pics; i < num_negative_pics + num_positive_pics; i++)
    {
      Value = GetUVLC(Parser_p, "RPS - delta_poc_s1_minus1");
      prev_delta_poc += (Value + 1);
      Rps_p->DeltaPOC[i] = prev_delta_poc;
      Value = GetBits(Parser_p, 1, "RPS - used_by_curr_pic_s1_flag");
      Rps_p->UsedByCurrPicFlag[i] = (uint8)Value;
      if(Value)
      {
        Rps_p->NumCurrRefPics++;
      }
    }
    Rps_p->NumStRefPics = num_negative_pics + num_positive_pics;
    for(i = 0; i < Rps_p->NumStRefPics; i++)
    {
      Rps_p->PictureMarking[i] = USED_FOR_SHORT_TERM_REFERENCE;
    }

  }
   //printf("\nRPS %d\n", idxRps);
   //for(i = 0; i < Rps_p->NumStRefPics; i++)
   //{
   //  printf(" %d|%d  ", Rps_p->DeltaPOC[i], Rps_p->UsedByCurrPicFlag[i]);
   //}
}



/******************************************************************************
*
* Name:        PictureDecoded
*
******************************************************************************/
void
PictureDecoded(SessionData_t *Sessiondata_p, const uint32 NALType)
{
  const uint32 DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  ReferencePicture_t *Image_p;
  sint32 i;
  uint32 DpbFullness;
  uint32 NumPicNeededForOutput;

  // Conseal the picture if not all CTUs for a picture were decoded
  //  if(Sessiondata_p->NumberCTUDecodedForPic != PicHeightInCtbsY*PicWidthInCtbsY)
  //  {
  //    ConsealPicture(Sessiondata_p);
  //  }

  Sessiondata_p->NumberCTUDecodedForPic = 0;

  DeblockingFilter(Sessiondata_p);
  SaoProcess(Sessiondata_p);

  /* Mark current picture needed for output */
  Sessiondata_p->CurrentRecImage->OutputMarking = NEEDED_FOR_OUTPUT;
  Sessiondata_p->CurrentRecImage->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
  Sessiondata_p->CurrentRecImage->Image.UserPictureValue = Sessiondata_p->CurrentRecImage->Image.UserPictureValue;
  Sessiondata_p->CurrentRecImage->Image.PictureType = Sessiondata_p->CurrentRecImage->Image.PictureType;
  //Pad current picture
  PicturePadding(&Sessiondata_p->CurrentRecImage->Image);

  //Copy prediction information
  CopyPredInfo(Sessiondata_p);

  //Reset PU data array
  ResetPUData(Sessiondata_p);
  memset(Sessiondata_p->EdgeFlags,    0, Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMinUnit * sizeof(uint8));
  memset(Sessiondata_p->EdgeFlagsHor, 0, Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMinUnit * sizeof(uint8));

  if(Sessiondata_p->CurrentSequenceParameterSet->SampleAdaptiveOffsetEnabledFlag == 1)
  {
    uint32 PicSizeInCtbY = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInCtbsY * Sessiondata_p->CurrentSequenceParameterSet->PicHeightInCtbsY;
    memset(Sessiondata_p->SaoTypeIdx[0], 0, PicSizeInCtbY * sizeof(uint8));
    memset(Sessiondata_p->SaoTypeIdx[1], 0, PicSizeInCtbY * sizeof(uint8));
  }

  //Output picture process:
  Sessiondata_p->ReturnedPictureFlag = 0;
  DpbFullness = 0;
  NumPicNeededForOutput = 0;
  for(i = 0; i < (sint32)DpbSize; i++)
  {
    if(Sessiondata_p->DecodedPictureBuffer[i].OutputMarking == NEEDED_FOR_OUTPUT)
    {
      NumPicNeededForOutput++;
    }
    if(Sessiondata_p->DecodedPictureBuffer[i].OutputMarking == NEEDED_FOR_OUTPUT ||
      Sessiondata_p->DecodedPictureBuffer[i].PictureMarking != UNUSED_FOR_REFERENCE)
    {
      DpbFullness++;
    }
  }
  if(DpbFullness == DpbSize)
  {
    Image_p = GetImageWithLowestPOC(Sessiondata_p);
    //Image_p->PictureMarking = UNUSED_FOR_REFERENCE;
    CropReturnPicture(Sessiondata_p, &Image_p->Image); 
    Image_p->OutputMarking = NOT_NEEDED_FOR_OUTPUT;
    //Image_p->PicOrderCnt = -1;
  }
  else
  {
#if D65_OUTPUT_IN_DEC_ORDER
    uint32 NumReorderPics = 0;
#else
    uint32 NumTLayers = Sessiondata_p->CurrentSequenceParameterSet->MaxSubLayers;
    uint32 NumReorderPics = Sessiondata_p->CurrentSequenceParameterSet->DpbData.MaxNumReorderPics[NumTLayers-1];
#endif
    if(NumPicNeededForOutput == (NumReorderPics + 1))
    {
      Image_p = GetImageWithLowestPOC(Sessiondata_p);
      CropReturnPicture(Sessiondata_p, &Image_p->Image);
      Image_p->OutputMarking = NOT_NEEDED_FOR_OUTPUT;
    }
  }
}


/******************************************************************************
*
* Name:        PictureMarkingProcess
*
******************************************************************************/
void
PictureMarkingProcess(SessionData_t *Sessiondata_p, const ReferencePictureSet_t *Rps_p)
{
  const sint32 Poc = Sessiondata_p->PicOrderCntVal;
  sint32 RefPicPoc;
  uint32 NumStRefPic = Rps_p->NumStRefPics;
  uint32 NumLtRefPic = Rps_p->NumLtRefPics;
  uint32 List0Idx = 0;
  uint32 List1Idx = 0;
  uint32 BufferSizeAfterSorting = 0;
  uint32 i;
  uint32 num_after = 0;
  uint32 DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  uint8  LtRefPicDeltaMsbFlag;
  ReferencePicture_t *RefPic_p;

  PrepareRefPicMarking(Sessiondata_p);

  /* Find the number of elements after the current that are used by current picture as short term reference */
  for(i = 0; i < NumStRefPic; i++)
  {
    if( Rps_p->DeltaPOC[i] > 0 &&
        Rps_p->UsedByCurrPicFlag[i] &&
        Rps_p->PictureMarking[i] == USED_FOR_SHORT_TERM_REFERENCE)
    {
      num_after++;
    }
  }
  
  List1Idx = num_after; // List1 should contain st ref pictures with positive delta poc in the beginning
  for(i = 0; i < NumStRefPic; i++)
  {
    RefPicPoc = Poc + Rps_p->DeltaPOC[i];
    
    if(Rps_p->UsedByCurrPicFlag[i] && Rps_p->PictureMarking[i] == USED_FOR_SHORT_TERM_REFERENCE)
    {
      /* When deltapoc > 0 all ref pictures with neg delta poc has been added */
      if(Rps_p->DeltaPOC[i] > 0 && List1Idx >= num_after)
      {
        List1Idx = 0;
      }
      RefPic_p = FetchPictureInDpb(Sessiondata_p, RefPicPoc, 0);
      if(RefPic_p == NULL)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Reference picture not present in DPB !");
      }
      else if(RefPic_p->PictureMarking == USED_FOR_LONG_TERM_REFERENCE)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Mark a long term reference picture to short term is not a allowed !");
      }
      else
      {
        RefPic_p->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
        RefPic_p->RefMarkingUpdated = 1;
        Sessiondata_p->RefPicList0[List0Idx++] = RefPic_p;
        Sessiondata_p->RefPicList1[List1Idx++] = RefPic_p;
      }
    }
  }
  //List1Idx += num_after; // Should equal List0Idx
  List1Idx = List0Idx;

  for(i = 0; i < NumLtRefPic; i++)
  {
    if(Rps_p->UsedByCurrPicFlag[NumStRefPic+i] && Rps_p->PictureMarking[NumStRefPic+i] == USED_FOR_LONG_TERM_REFERENCE)
    {
      RefPicPoc = Rps_p->DeltaPOC[NumStRefPic+i];
      LtRefPicDeltaMsbFlag = Rps_p->DeltaPocMsbFlag[NumStRefPic+i];
      RefPic_p = FetchPictureInDpb(Sessiondata_p, RefPicPoc, !LtRefPicDeltaMsbFlag);
      if(RefPic_p == NULL)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Reference picture not present in DPB !");
      }
      else
      {
        RefPic_p->PictureMarking = USED_FOR_LONG_TERM_REFERENCE;
        RefPic_p->RefMarkingUpdated = 1;
        Sessiondata_p->RefPicList0[List0Idx++] = RefPic_p;
        Sessiondata_p->RefPicList1[List1Idx++] = RefPic_p;
      }
    }
  }
  
  for(i = 0; i < NumStRefPic; i++)
  {
    RefPicPoc = Poc + Rps_p->DeltaPOC[i];
    if(Rps_p->UsedByCurrPicFlag[i] == 0 && Rps_p->PictureMarking[i] == USED_FOR_SHORT_TERM_REFERENCE)
    {
      RefPic_p = FetchPictureInDpb(Sessiondata_p, RefPicPoc, 0);
      if(RefPic_p == NULL)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Reference picture not present in DPB !");
      }
      else
      {
        RefPic_p->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
        RefPic_p->RefMarkingUpdated = 1;
      }
    }
  }

  for(i = 0; i < NumLtRefPic; i++)
  {
    if(Rps_p->UsedByCurrPicFlag[NumStRefPic+i] == 0 && Rps_p->PictureMarking[NumStRefPic+i] == USED_FOR_LONG_TERM_REFERENCE)
    {
      RefPicPoc = Rps_p->DeltaPOC[NumStRefPic+i];
      LtRefPicDeltaMsbFlag = Rps_p->DeltaPocMsbFlag[NumStRefPic+i];
      RefPic_p  = FetchPictureInDpb(Sessiondata_p, RefPicPoc, !LtRefPicDeltaMsbFlag);
      if(RefPic_p == NULL)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_ERROR, "Reference picture not present in DPB !");
      }
      else
      {
        RefPic_p->PictureMarking = USED_FOR_LONG_TERM_REFERENCE;
        RefPic_p->RefMarkingUpdated = 1;
      }
    }
  }

  for(i = 0; i < DpbSize; i++)
  {
    RefPic_p = &Sessiondata_p->DecodedPictureBuffer[i];
    if(RefPic_p->RefMarkingUpdated == 0)
    {
      RefPic_p->PictureMarking = UNUSED_FOR_REFERENCE;
      if(RefPic_p->OutputMarking == NOT_NEEDED_FOR_OUTPUT)
      {
        RefPic_p->Image.UserPictureValue = 0;
        RefPic_p->Image.PictureType = 0;
        RefPic_p->PicOrderCnt = -1;
      }
    }
  }
}




/******************************************************************************
*
* Name:        FakePictureMarkingProcess
*
******************************************************************************/
void
FakePictureMarkingProcess(SessionData_t *Sessiondata_p)
{
  sint32 i;
  sint32 HighestPOC = 0;
  sint32 DpbSize = Sessiondata_p->CurrentSequenceParameterSet->DpbSize;
  ReferencePicture_t *RefPic_p;

  PrepareRefPicMarking(Sessiondata_p);

  for(i=0 ; i<DpbSize ; i++)
  {
    RefPic_p = &Sessiondata_p->DecodedPictureBuffer[i];
    if(RefPic_p->OutputMarking == NEEDED_FOR_OUTPUT)
    {
      if(RefPic_p->PicOrderCnt > HighestPOC)
      {
        HighestPOC = RefPic_p->PicOrderCnt;
      }
    }
  }

  for(i=0 ; i<DpbSize ; i++)
  {
    RefPic_p = &Sessiondata_p->DecodedPictureBuffer[i];
    RefPic_p->PictureMarking = UNUSED_FOR_REFERENCE;
    if(RefPic_p->OutputMarking == NEEDED_FOR_OUTPUT)
    {
      RefPic_p->PicOrderCnt -= (HighestPOC+2);
    }
    else
    {
      RefPic_p->Image.UserPictureValue = 0;
      RefPic_p->Image.PictureType = 0;
      RefPic_p->PicOrderCnt = -1;
    }
  }
}

/******************************************************************************
*
* Name:        D65_ForceOutOneDecodedPicture
*
******************************************************************************/
D65_ReturnImage_t *
D65_ForceOutOneDecodedPicture(D65_Handle_t* Decoder_p)
{
  SessionData_t *Sessiondata_p;
  ReferencePicture_t *Image_p;

  Sessiondata_p = (SessionData_t*) Decoder_p;
  Sessiondata_p->ReturnedPictureFlag = 0;

  if(Sessiondata_p->CurrentSequenceParameterSet == NULL
#if ENABLE_OPENHEVC
    && (Sessiondata_p->OpenHevcDecState == 0)
#endif
    )
  {
    return NULL;
  }

#if ENABLE_OPENHEVC
  if(Sessiondata_p->OpenHevcDecState >= 2)
  {
    int got_picture = libOpenHevcDecode(Sessiondata_p->OpenHevcDec.openHevcHandle, Sessiondata_p->CurrentBuffer_p, 0, 0);
    if(got_picture)
    {
      libOpenHevcGetPictureInfo(Sessiondata_p->OpenHevcDec.openHevcHandle, &Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo);
      {
        sint32 CurrentPicW = Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo.nWidth;
        sint32 CurrentPicH = Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo.nHeight;
        if(AllocateOpenHevcReturnPic(Sessiondata_p, CurrentPicH, CurrentPicW))
        {
          FreeOpenHevcReturnPic(Sessiondata_p);
          return NULL;
        }
      }
      libOpenHevcGetOutputCpy(Sessiondata_p->OpenHevcDec.openHevcHandle, 1, &Sessiondata_p->OpenHevcDec.openHevcFrame);
      Sessiondata_p->ReturnedPictureFlag = 1;
      return(&(Sessiondata_p->DecodedPicture));
    }
    return NULL;
  }
  else
  {
#endif
  // If there are decoded macroblocks in the current picture we call the
  // PictureDecoded function to conseal it and add it to the output list.
  //if(Sessiondata_p->NumberMacroblocksDecodedForPic != 0)
  //{
  //  PictureDecoded(Sessiondata_p,
  //                 Sessiondata_p->PreviousSliceNALType,
  //                 Sessiondata_p->PreviousSliceUseForPrediction);

  //  if(Sessiondata_p->ReturnedPictureFlag)
  //  {
  //    return(&(Sessiondata_p->DecodedPicture));
  //  }
  //}
  Image_p = GetImageWithLowestPOC(Sessiondata_p);

  if(Image_p != NULL)
  {
    // Crop picture and set up the Sessiondata_p->DecodedPicture struct
    CropReturnPicture(Sessiondata_p,&Image_p->Image);
    Image_p->OutputMarking = NOT_NEEDED_FOR_OUTPUT;
    return(&(Sessiondata_p->DecodedPicture));
  }
  else
  {
    return NULL;
  }
#if ENABLE_OPENHEVC
  }
#endif
}
