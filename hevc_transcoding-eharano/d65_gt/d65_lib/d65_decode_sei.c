/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains SEI decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include "d65_decode_sei.h"
#include "d65_getbits.h"

#if MULTI_THREADED
#include "pthread.h"
#endif


/******************************************************************************
* Definition of local functions
******************************************************************************/


/******************************************************************************
*
* Name:        DecodeBPSEI
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     Success status
*
* Description: Decodes a Buffering Period SEI message
*
******************************************************************************/
D65_ReturnCode_t
DecodeBPSEI(SessionData_t *Sessiondata_p)
{
  sint32 Value;
  uint8  idx;
  uint8 subLayerId = 0; //CpbCnt is set equal to cpb_cnt_minus1[ subLayerId ], From SPS(HRD)
  Bool irap_cpb_params_present_flag = 0;
  Bool NalHrdBpPresentFlag;
  Bool VclHrdBpPresentFlag;
  HrdParameterSet_t *hrd =  &Sessiondata_p->CurrentSequenceParameterSet->VuiParameters.HrdParameters;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  if (!Sessiondata_p->CurrentSequenceParameterSet->VuiParameters.HrdParametersPresentFlag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SEI_ERROR, "SEI(BP) - HRD Parameters Present Flag not true in SPS(VUI)");
  }

  NalHrdBpPresentFlag = hrd->NalParamsPresentFlag;
  VclHrdBpPresentFlag = hrd->VclParamsPresentFlag;
  Value = GetUVLC(Parser_p, "SEI(BP) - bp_seq_parameter_set_id");
  if(!hrd->SubPicParamsPresentFlag)
  {//sub_pic_hrd_params_present_flag
    irap_cpb_params_present_flag = (Bool)GetFlag(Parser_p, "SEI(BP) - irap_cpb_params_present_flag");
  }
  if (irap_cpb_params_present_flag)
  {
    Value = GetBits(Parser_p, hrd->AuCpbRemovalDelayLength, "SEI(BP) - cpb_delay_offset");
    Value = GetBits(Parser_p, hrd->DpbOutputDelayLength, "SEI(BP) - dpb_delay_offset");
  }
  Value = GetFlag(Parser_p,     "SEI(BP) - concatenation_flag");
  Value = GetBits(Parser_p, hrd->AuCpbRemovalDelayLength, "SEI(BP) - au_cpb_removal_delay_delta_minus1");
  if (NalHrdBpPresentFlag)
  {
    for (idx = 0; idx <= hrd->CpbCnt[subLayerId]; idx++) {
      Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - nal_initial_cpb_removal_delay");
      Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - nal_initial_cpb_removal_offset");
      if (hrd->SubPicParamsPresentFlag || irap_cpb_params_present_flag)
      {
        //sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag
        Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - nal_initial_alt_cpb_removal_delay");
        Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - nal_initial_alt_cpb_removal_offset");
      }
    }
  }
  if (VclHrdBpPresentFlag)
  {
    for (idx = 0; idx <= hrd->CpbCnt[subLayerId]; idx++) {
      Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - vcl_initial_cpb_removal_delay");
      Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - vcl_initial_cpb_removal_offset");
      if (hrd->SubPicParamsPresentFlag || irap_cpb_params_present_flag)
      {//sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag
        Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - vcl_initial_alt_cpb_removal_delay");
        Value = GetBits(Parser_p, hrd->InitialCpbRemovalDelayLength, "SEI(BP) - vcl_initial_alt_cpb_removal_offset");
      }
    }
  }
  return D65_OK;
}

/******************************************************************************
*
* Name:        DecodePTSEI
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     Success status
*
* Description: Decodes a Picture Timing SEI message
*
******************************************************************************/
D65_ReturnCode_t
DecodePTSEI(SessionData_t *Sessiondata_p)
{
  sint32 Value;
  uint8  idx;
  uint32 num_decoding_units_minus1      = 0;
  Bool du_common_cpb_removal_delay_flag = 0;
  uint32 CpbDpbDelaysPresentFlag        = 0;
  HrdParameterSet_t *hrd = &Sessiondata_p->CurrentSequenceParameterSet->VuiParameters.HrdParameters;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  if (!Sessiondata_p->CurrentSequenceParameterSet->VuiParameters.TimmingInforPresentFlag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SEI_ERROR, "SEI(BP) - Timming Infor Present Flag not true in SPS(VUI)");
  }

  CpbDpbDelaysPresentFlag = hrd->NalParamsPresentFlag || hrd->VclParamsPresentFlag;
  if (Sessiondata_p->CurrentSequenceParameterSet->VuiParameters.FrameFieldInfoPresentFlag)
  {
    Value = GetBits(Parser_p, 4, "SEI(PT) - pic_struct");
    Value = GetBits(Parser_p, 2, "SEI(PT) - source_scan_type");
    Value = GetFlag(Parser_p,    "SEI(PT) - duplicate_flag");
  }
  if (CpbDpbDelaysPresentFlag)
  {
    Value = GetBits(Parser_p, hrd->AuCpbRemovalDelayLength, "SEI(PT) - au_cpb_removal_delay_minus1");
    Value = GetBits(Parser_p, hrd->DpbOutputDelayLength, "SEI(PT) - pic_dpb_output_delay");
    if (hrd->SubPicParamsPresentFlag)
    {
      Value = GetBits(Parser_p, hrd->DpbOutputDuDelayLength,  "SEI(PT) - pic_dpb_output_du_delay");
    }
    if (hrd->SubPicParamsPresentFlag && hrd->SubPicCpbParamsPicTimingSeiFlag)
    {
      num_decoding_units_minus1 = GetUVLC(Parser_p, "SEI(PT) - num_decoding_units_minus1");
      du_common_cpb_removal_delay_flag = (Bool)GetFlag(Parser_p, "SEI(PT) - du_common_cpb_removal_delay_flag");
      if(du_common_cpb_removal_delay_flag)
      {
        for (idx = 0; idx <= num_decoding_units_minus1; idx++)
        {
          Value = GetUVLC(Parser_p, "SEI(PT) - num_nalus_in_du_minus1");
          if(!du_common_cpb_removal_delay_flag && idx < num_decoding_units_minus1)
          {
            Value = GetBits(Parser_p, hrd->DuCpbRemovalDelayIncLength,  "SEI(PT) - du_cpb_removal_delay_increment_minus1");
          }
        }
      }
    }
  }
  return D65_OK;
}

/******************************************************************************
*
* Name:        DecodePictureHashSEI
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     Success status
*
* Description: Decodes a Picture Hash SEI message
*
******************************************************************************/
D65_ReturnCode_t
DecodePictureHashSEI(SessionData_t *Sessiondata_p)
{
  sint32 Value;
  uint8 cIdx, idx;
  sint32 HashType;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  HashType = GetByte(Parser_p, "SEI(DPH) - hash_type");
  if(HashType > 2)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SEI_ERROR, "SEI - Hash type not supported");
  }
  for(cIdx = 0; cIdx < (Sessiondata_p->CurrentSequenceParameterSet->chromaFormatIDC == 0 ? 1 : 3); cIdx++)
  {
    if (HashType == 0)
    {
      for (idx = 0; idx < 16; idx++)
      {
        Value = GetBits(Parser_p, 8, "SEI(DPH) - picture_md5");
      }
    }
    else if (HashType == 1)
    {
      Value = GetBits(Parser_p, 16, "SEI(DPH) - picture_crc");
    }
    else if (HashType == 2)
    {
      Value = GetBits(Parser_p, 32, "SEI(DPH) - picture_checksum");
    }
  }
  return D65_OK;
}


/******************************************************************************
*
* Name:        DecodeSEI
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*              nalUnitType   [in]      NAL type
*
* Returns:     Success status
*
* Description: Decodes a SEI message
*
******************************************************************************/
D65_ReturnCode_t
DecodeSEI(SessionData_t *Sessiondata_p, sint32 nalUnitType)
{
  sint32 Value;
  uint32 idx;
  uint32 payloadType = 0;
  uint32 payloadSize = 0;
  sint32 savedBufferPos32 = 0;
  D65_ReturnCode_t return_status = D65_OK;
  Parser_t *Parser_p = &Sessiondata_p->Parser;
  BitsReader_t *BitsReader_p = &Parser_p->BitsReader;

  if(!Sessiondata_p->CurrentSequenceParameterSet)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SEI_ERROR, "SEI(BP) - Missing current SPS");
  }

  Value = GetByte(Parser_p, "SEI - last_payload_type_byte or ff_byte");
  payloadType = Value;
  while(Value == 0xFF)
  {
    Value = GetByte(Parser_p, "SEI - last_payload_type_byte or ff_byte");
    payloadType += Value;
  }
  Value = GetByte(Parser_p, "SEI - payloadSize");
  payloadSize = Value;
  while(Value == 0xFF)
  {
    Value = GetByte(Parser_p, "SEI - last_payload_size_byte or ff_byte");
    payloadSize += Value;
  }
  //printf("DecodeSEI payload %d\n", payloadType);
  savedBufferPos32 = BitsReader_p->Buffer32_Pos;
  if(nalUnitType == 39)
  {//Prefix
    switch(payloadType)
    {
      case 0://buffering_period
        return_status = DecodeBPSEI(Sessiondata_p);
        break;
      case 1://Picture_timing
        return_status = DecodePTSEI(Sessiondata_p);
        break;
      default:
        return_status = D65_UNKNOWN_SEI;
    }
  }
  else if(nalUnitType == 40)
  {//Suffix
    switch(payloadType)
    {
      case 132://decoded picture hash
        return_status = DecodePictureHashSEI(Sessiondata_p);
        break;
      default:
        return_status = D65_UNKNOWN_SEI;
    }
  }
  else
  {
    return_status = D65_UNKNOWN_SEI;
  }
  if(return_status != D65_OK)
  {
    //Parsing remaining unread bits
    idx = 0;
    if(BitsReader_p->Buffer32_Pos > savedBufferPos32)
    {
      idx = BitsReader_p->Buffer32_Pos- savedBufferPos32 + 1;
    }
    for(; idx < 8*payloadSize-4; idx++)
    {
      Value = GetBits(Parser_p, 1, "SEI - reserved_payload_extension_data");
    }
  }
  if(!CheckByteAllignment(BitsReader_p) || (BitsReader_p->Buffer32_Pos-savedBufferPos32) != 8 * (sint32)payloadSize)
  {//more_data_in_payload
    GetRbspTrailingBits(Parser_p);
  }
  return return_status;
}
