/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains VPS/SPS/PPS decoding functions
*
******************************************************************************/



/******************************************************************************
* Includes
******************************************************************************/
#include "d65_decode_ps.h"
#include "string.h"
#include "d65_api.h"
#include "d65_getbits.h"
#include "d65_init.h"
#include "d65_dpb.h"



/******************************************************************************
* Definition of local variables
******************************************************************************/
static const uint8 ScalingListDefault[16] =
{
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

static const uint8 ScalingListIntra[64] =
{
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 16, 17, 16, 17, 18,
  17, 18, 18, 17, 18, 21, 19, 20, 21, 20, 19, 21, 24, 22, 22, 24,
  24, 22, 22, 24, 25, 25, 27, 30, 27, 25, 25, 29, 31, 35, 35, 31,
  29, 36, 41, 44, 41, 36, 47, 54, 54, 47, 65, 70, 65, 88, 88, 115
};

static const uint8 ScalingListInter[64] = 
{
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18,
  18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 24, 24, 24, 24,
  24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 28, 28, 28, 28, 28,
  28, 33, 33, 33, 33, 33, 41, 41, 41, 41, 54, 54, 54, 71, 71, 91
};

/******************************************************************************
* Definition of local functions
******************************************************************************/
void
GetPTLLimit(Parser_t *Parser_p,
            PTLLimitsData_t *PtlLimitsData_p,
            D65_Profile_t Profile,
            uint8 Tier,
            uint32 Level)
{
  //Currently only get tier and level limits
  switch(Level)
  {
  case 30:
    // Level 1
    PtlLimitsData_p->MaxLumsPs         = 36864;
    PtlLimitsData_p->MaxCPBSize        = 350;
    PtlLimitsData_p->MaxSliceSegPerPic = 16;
    PtlLimitsData_p->MaxTileRows       = 1;
    PtlLimitsData_p->MaxTileCols       = 1;
    break;
  case 60:
    // Level 2
    PtlLimitsData_p->MaxLumsPs         = 122880;
    PtlLimitsData_p->MaxCPBSize        = 1500;
    PtlLimitsData_p->MaxSliceSegPerPic = 16;
    PtlLimitsData_p->MaxTileRows       = 1;
    PtlLimitsData_p->MaxTileCols       = 1;
    break;
  case 63:
    // Level 2.1
    PtlLimitsData_p->MaxLumsPs         = 245760;
    PtlLimitsData_p->MaxCPBSize        = 3000;
    PtlLimitsData_p->MaxSliceSegPerPic = 20;
    PtlLimitsData_p->MaxTileRows       = 1;
    PtlLimitsData_p->MaxTileCols       = 1;
    break;
  case 90:
    // Level 3
    PtlLimitsData_p->MaxLumsPs         = 552960;
    PtlLimitsData_p->MaxCPBSize        = 6000;
    PtlLimitsData_p->MaxSliceSegPerPic = 30;
    PtlLimitsData_p->MaxTileRows       = 2;
    PtlLimitsData_p->MaxTileCols       = 2;
    break;
  case 93:
    // Level 3.1
    PtlLimitsData_p->MaxLumsPs         = 983040;
    PtlLimitsData_p->MaxCPBSize        = 10000;
    PtlLimitsData_p->MaxSliceSegPerPic = 40;
    PtlLimitsData_p->MaxTileRows       = 3;
    PtlLimitsData_p->MaxTileCols       = 3;
    break;
  case 120:
    // Level 4
    PtlLimitsData_p->MaxLumsPs         = 2228224;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 30000 : 12000;
    PtlLimitsData_p->MaxSliceSegPerPic = 75;
    PtlLimitsData_p->MaxTileRows       = 5;
    PtlLimitsData_p->MaxTileCols       = 5;
    break;
  case 123:
    // Level 4.1
    PtlLimitsData_p->MaxLumsPs         = 2228224;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 50000 : 20000;
    PtlLimitsData_p->MaxSliceSegPerPic = 75;
    PtlLimitsData_p->MaxTileRows       = 5;
    PtlLimitsData_p->MaxTileCols       = 5;
    break;
  case 150:
    // Level 5
    PtlLimitsData_p->MaxLumsPs         = 8912896;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 100000 : 25000;
    PtlLimitsData_p->MaxSliceSegPerPic = 200;
    PtlLimitsData_p->MaxTileRows       = 11;
    PtlLimitsData_p->MaxTileCols       = 10;
    break;
  case 153:
    // Level 5.1
    PtlLimitsData_p->MaxLumsPs         = 8912896;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 160000 : 40000;
    PtlLimitsData_p->MaxSliceSegPerPic = 200;
    PtlLimitsData_p->MaxTileRows       = 11;
    PtlLimitsData_p->MaxTileCols       = 10;
    break;
  case 156:
    // Level 5.2
    PtlLimitsData_p->MaxLumsPs         = 8912896;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 240000 : 60000;
    PtlLimitsData_p->MaxSliceSegPerPic = 200;
    PtlLimitsData_p->MaxTileRows       = 11;
    PtlLimitsData_p->MaxTileCols       = 10;
    break;
  case 180:
    // Level 6
    PtlLimitsData_p->MaxLumsPs         = 35651584;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 240000 : 60000;
    PtlLimitsData_p->MaxSliceSegPerPic = 600;
    PtlLimitsData_p->MaxTileRows       = 22;
    PtlLimitsData_p->MaxTileCols       = 20;
    break;
  case 183:
    // Level 6.1
    PtlLimitsData_p->MaxLumsPs         = 35651584;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 480000 : 120000;
    PtlLimitsData_p->MaxSliceSegPerPic = 600;
    PtlLimitsData_p->MaxTileRows       = 22;
    PtlLimitsData_p->MaxTileCols       = 20;
    break;
  case 186:
    // Level 6.2
    PtlLimitsData_p->MaxLumsPs         = 35651584;
    PtlLimitsData_p->MaxCPBSize        = Tier ? 800000 : 240000;
    PtlLimitsData_p->MaxSliceSegPerPic = 600;
    PtlLimitsData_p->MaxTileRows       = 22;
    PtlLimitsData_p->MaxTileCols       = 20;
    break;
  default:
    STB_WARNING("Unsupported level");
    break;
  }
}

void SetDefaultScalingList(ScalingListData_t *ScalListdata_p)
{
  uint8 matrixId;
  for(matrixId = 0; matrixId < 6; matrixId++)
  {
    memcpy(ScalListdata_p->ScalingList[0][matrixId], ScalingListDefault, 16);
  }
  for(matrixId = 0; matrixId < 3; matrixId++)
  {
    memcpy(ScalListdata_p->ScalingList[1][matrixId], ScalingListIntra, 64);
    memcpy(ScalListdata_p->ScalingList[2][matrixId], ScalingListIntra, 64);
  }
  for(matrixId = 3; matrixId < 6; matrixId++)
  {
    memcpy(ScalListdata_p->ScalingList[1][matrixId], ScalingListInter, 64);
    memcpy(ScalListdata_p->ScalingList[2][matrixId], ScalingListInter, 64);
  }
  memcpy(ScalListdata_p->ScalingList[3][0], ScalingListIntra, 64);
  memcpy(ScalListdata_p->ScalingList[3][1], ScalingListInter, 64);
}

void SetDefaultPcmData(PcmData_t *Pcmdata_p)
{
  Pcmdata_p->pcm_sample_bit_depth_luma = 8;
  Pcmdata_p->pcm_sample_bit_depth_chroma = 8;
  Pcmdata_p->log2_min_pcm_cu_size = 1;
  Pcmdata_p->log2_max_pcm_cu_size = 1;
  Pcmdata_p->pcm_loof_filter_disabled_flag = 1;
}

/******************************************************************************
*
* Name:        ParsePTLParameters
*
* Parameters:  Sessiondata_p [In]  The decoder
*
* Returns:     -
*
* Description: Parses PTL parameter syntax
*
*
******************************************************************************/
void
ParsePTLParameters(Parser_t *Parser_p, 
                   PTLData_t *PTLData_p,
                   uint32 MaxNumSubLayerMinus1)
{
  uint32 i;
  uint32 Value;
  ProfileTierData_t *PTData_p = &PTLData_p->GeneralPT;

  PTData_p->ProfileSpace = (uint8)GetBits(Parser_p, 2, "PTL - general_profile_space");
  if(PTData_p->ProfileSpace != 0)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Unknown profile space");
  }

  PTData_p->TierFlag = (uint8)GetBits(Parser_p, 1, "PTL - general_tier_space");
  PTData_p->ProfileIdc = (uint8)GetBits(Parser_p, 5, "PTL - general_profile_idc");
  PTData_p->ProfileCompatibilityFlag[0] =
    (uint8)GetBits(Parser_p, 1, "PTL - general_profile_compatibility_flag[0]");
  PTData_p->ProfileCompatibilityFlag[1] =
    (uint8)GetBits(Parser_p, 1, "PTL - general_profile_compatibility_flag[1]");

  if(PTData_p->ProfileIdc == 1 || PTData_p->ProfileCompatibilityFlag[1] == 1)
  {
    PTLData_p->GeneralProfile = D65_PROFILE_MAIN;
  }
  else
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Unknown or unsupported profile");
  }
  for(i = 2; i < 32; i++)
  {
    PTData_p->ProfileCompatibilityFlag[i] = 
      (uint8)GetBits(Parser_p, 1, "PTL - general_profile_compatibility_flag[j]");
  }
  PTData_p->ProgressiveSrcFlag = 
      (uint8)GetBits(Parser_p, 1, "PTL - general_progressive_source_flag");
  PTData_p->InterlacedSrcFlag = 
      (uint8)GetBits(Parser_p, 1, "PTL - general_interlaced_source_flag");
  PTData_p->NonPackedConstraintFlag =
      (uint8)GetBits(Parser_p, 1, "PTL - general_non_packed_constraint_flag");
  PTData_p->FrameOnlyConstraintFlag =
      (uint8)GetBits(Parser_p, 1, "PTL - general_frame_only_constraint_flag");
  Value = GetBits(Parser_p, 22, "PTL - general_reserved_zero_44bits");
  Value = GetBits(Parser_p, 22, "PTL - general_reserved_zero_44bits");
  PTLData_p->GeneralLevelIdc = GetBits(Parser_p, 8, "PTL - general_level_idc");
  GetPTLLimit(Parser_p, &PTLData_p->GeneralLimits, D65_PROFILE_MAIN, PTData_p->TierFlag, PTLData_p->GeneralLevelIdc);

  if(MaxNumSubLayerMinus1 > 0)
  {
    for(i = 0; i < MaxNumSubLayerMinus1; i++)
    {
      PTLData_p->SubLayerProfilePresentFlag[i] =
        (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_profile_present_flag[i]");
      PTLData_p->SubLayerLevelPresentFlag[i] =
        (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_level_present_flag[i]");
    }

    for(i = MaxNumSubLayerMinus1; i < 8; i++)
    {
      Value = GetBits(Parser_p, 2, "PTL - reserved_zero_2bits[i]");
    }

    for(i = 0; i < MaxNumSubLayerMinus1; i++)
    {
      PTData_p = &PTLData_p->SubLayerPT[i];
      if(PTLData_p->SubLayerProfilePresentFlag[i])
      {
        PTData_p->ProfileSpace = 
          (uint8)GetBits(Parser_p, 2, "PTL - sub_layer_profile_space");
        if(PTData_p->ProfileSpace != 0)
        {
          BIT_ERROR(*Parser_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Unknown profile space");
        }
        PTData_p->TierFlag = 
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_tier_space");
        PTData_p->ProfileIdc = 
          (uint8)GetBits(Parser_p, 5, "PTL - sub_layer_profile_idc");
        PTData_p->ProfileCompatibilityFlag[0] = 
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_profile_compatibility_flag[0]");
        PTData_p->ProfileCompatibilityFlag[1] = 
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_profile_compatibility_flag[1]");
        if(PTData_p->ProfileIdc == 1 || PTData_p->ProfileCompatibilityFlag[1] == 1)
        {
          PTLData_p->SubLayerProfile[i] = D65_PROFILE_MAIN;
        }
        else
        {
          BIT_ERROR(*Parser_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Unknown or unsupported profile");
        }
        for(i = 2; i < 32; i++)
        {
          PTData_p->ProfileCompatibilityFlag[i] = 
            (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_profile_compatibility_flag[j]");
        }
        PTData_p->ProgressiveSrcFlag = 
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_progressive_source_flag");
        PTData_p->InterlacedSrcFlag = 
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_interlaced_source_flag");
        PTData_p->NonPackedConstraintFlag =
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_non_packed_constraint_flag");
        PTData_p->FrameOnlyConstraintFlag =
          (uint8)GetBits(Parser_p, 1, "PTL - sub_layer_frame_only_constraint_flag");
        Value = GetBits(Parser_p, 22, "PTL - sub_layer_reserved_zero_44bits");
        Value = GetBits(Parser_p, 22, "PTL - sub_layer_reserved_zero_44bits");
      }
      else
      {
        PTData_p->TierFlag = 0;
      }
      if(PTLData_p->SubLayerLevelPresentFlag[i])
      {
        PTLData_p->SubLayerLevelIdc[i] = GetBits(Parser_p, 8, "PTL - sub_layer_level_idc");
        GetPTLLimit(Parser_p, &PTLData_p->SubLayerLimits[i], D65_PROFILE_MAIN, PTData_p->TierFlag, PTLData_p->SubLayerLevelIdc[i]);
      }
    }
  }
}

/******************************************************************************
*
* Name:        ParseScalingListData
*
* Parameters:  Sessiondata_p [In]  The decoder
*
* Returns:     -
*
* Description: Parses scaling list data syntax
*
*
******************************************************************************/
void
ParseScalingListData(Parser_t *Parser_p, ScalingListData_t *ScalListdata_p)
{
  uint32 pred_mod_flag;
  uint32 matrix_id_delta;
  sint32 dc_coeff[2][6];
  sint32 delta_coef;
  uint8 sizeId;
  uint8 matrixId;
  uint32 nextCoef;
  uint8 coefNum;
  uint8 i;

  for(matrixId = 0; matrixId < 6; matrixId++)
  {
    dc_coeff[0][matrixId] = 16;
    dc_coeff[1][matrixId] = 16;
  }
  for(sizeId = 0; sizeId < 4; sizeId++)
  {
    for(matrixId = 0; matrixId < ((sizeId == 3) ? 2 : 6); matrixId++)
    {
      pred_mod_flag = GetBits(Parser_p, 1, "scaling_list_pred_mode_flag");
      if(!pred_mod_flag)
      {
        matrix_id_delta = GetUVLC(Parser_p, "scaling_list_pred_matrix_id_delta");
        if(matrix_id_delta)
        {
          memcpy(ScalListdata_p->ScalingList[sizeId][matrixId],
            ScalListdata_p->ScalingList[sizeId][matrixId-matrix_id_delta], sizeId > 0 ? 64 : 16);
          if(sizeId > 1)
          {
            dc_coeff[sizeId-2][matrixId] = dc_coeff[sizeId-2][matrixId-matrix_id_delta];
          }
        }
      }
      else
      {
        nextCoef = 8;
        coefNum = D65_MIN(64, (1 << (4 + (sizeId << 1))));
        if(sizeId > 1)
        {
          dc_coeff[sizeId-2][matrixId] = GetSVLC(Parser_p, "scaling_list_dc_coef_minus8") + 8;
          nextCoef = dc_coeff[sizeId-2][matrixId];
        }
        for(i = 0; i < coefNum; i++)
        {
          delta_coef = GetSVLC(Parser_p, "scaling_list_delta_coef");
          nextCoef = (nextCoef + delta_coef + 256) % 256;
          ScalListdata_p->ScalingList[sizeId][matrixId][i] = (uint8)nextCoef;
        }
      }
    }
  }
}


/******************************************************************************
*
* Name:        ParseHRDParameters
*
* Parameters:  Sessiondata_p [In]  The decoder
*
* Returns:     -
*
* Description: Parses HRD parameter syntax, a part of VUI data (E.2.2). The
*              support for parsing this is required since parsing of the syntax
*              element num_reorder_frames is neccesary for H324M and the
*              position of this element is not given by other means than full
*              VUI parsing. Note that just parsing is necessary, nothing more.
*
*
******************************************************************************/
void
ParseHRDParameters(Parser_t *Parser_p, HrdParameterSet_t *hrd, uint32 MaxNumSubLayer)
{
  uint8  cpb_cnt_minus1 = 0;
  uint32 SchedSelIdx;
  uint8  layer;
  uint32 Value; // Sole use of variable is to avoid ARM lint warning
  Bool   low_delay_hrd_flag = 0;
  Bool   fixed_pic_rate_within_cvs_flag = 1;
  //Default values
  hrd->InitialCpbRemovalDelayLength = 24;
  hrd->AuCpbRemovalDelayLength      = 24;
  hrd->DpbOutputDelayLength         = 24;
  hrd->DpbOutputDuDelayLength       = 24;
  hrd->DuCpbRemovalDelayIncLength   = 24;

  Value = GetFlag(Parser_p, "HRD - nal_hrd_parameters_present_flag");
  hrd->NalParamsPresentFlag = (Bool)Value;
  Value = GetFlag(Parser_p, "HRD - vcl_hrd_parameters_present_flag");
  hrd->VclParamsPresentFlag = (Bool)Value;
  hrd->SubPicParamsPresentFlag = 0;
  hrd->SubPicCpbParamsPicTimingSeiFlag = 0;
  if (hrd->NalParamsPresentFlag || hrd->VclParamsPresentFlag) {
    Value = GetFlag(Parser_p, "HRD - sub_pic_hrd_params_present_flag");
    hrd->SubPicParamsPresentFlag = (Bool)Value;
    if (hrd->SubPicParamsPresentFlag) {
      Value = GetByte(Parser_p,    "HRD - tick_divisor_minus2");
      Value = GetBits(Parser_p, 5, "HRD - du_cpb_removal_delay_increment_length_minus1");
      hrd->DuCpbRemovalDelayIncLength = (uint8)Value+1;
      Value = GetFlag(Parser_p,    "HRD - sub_pic_cpb_params_in_pic_timing_sei_flag");
      hrd->SubPicCpbParamsPicTimingSeiFlag = (Bool)Value;
      Value = GetBits(Parser_p, 5, "HRD - dpb_output_delay_du_length_minus1");
      hrd->DpbOutputDuDelayLength = (uint8)Value+1;
    }
    Value = GetBits(Parser_p, 4, "HRD - bit_rate_scale");
    Value = GetBits(Parser_p, 4, "HRD - cpb_size_scale");
    if (hrd->SubPicParamsPresentFlag)
      Value = GetBits(Parser_p, 4, "HRD - cpb_size_du_scale");
    Value = GetBits(Parser_p, 5, "HRD - initial_cpb_removal_delay_length_minus1");
    hrd->InitialCpbRemovalDelayLength = (uint8)Value+1;
    Value = GetBits(Parser_p, 5, "HRD - au_cpb_removal_delay_length_minus1");
    hrd->AuCpbRemovalDelayLength = (uint8)Value+1;
    Value = GetBits(Parser_p, 5, "HRD - dpb_output_delay_length_minus1");
    hrd->DpbOutputDelayLength = (uint8)Value+1;
  }

  for (layer = 0; layer < MaxNumSubLayer; layer++) {
    cpb_cnt_minus1 = 0;
    low_delay_hrd_flag = 0;
    fixed_pic_rate_within_cvs_flag = 1; // should be in ???
    Value = GetFlag(Parser_p,    "HRD - fixed_pic_rate_general_flag");
    if (!Value) {
      fixed_pic_rate_within_cvs_flag = (Bool)GetFlag(Parser_p,    "HRD - fixed_pic_rate_within_cvs_flag");
    }
    if (fixed_pic_rate_within_cvs_flag)
      Value = GetUVLC(Parser_p,    "HRD - elemental_duration_in_tc_minus1");
    else {
      low_delay_hrd_flag = (Bool)GetFlag(Parser_p,    "HRD - low_delay_hrd_flag");
    }
    hrd->LowDelayHrdFlag[layer] = low_delay_hrd_flag;
    if (!low_delay_hrd_flag) {
      STB_WARNING("HRD - Non-low delay mode not supported yet");
    }
    if (!low_delay_hrd_flag) {
      cpb_cnt_minus1 = (uint8)GetUVLC(Parser_p,    "HRD - cpb_cnt_minus1");
    }
    hrd->CpbCnt[layer] = cpb_cnt_minus1 + 1;
    if (hrd->NalParamsPresentFlag) {
      for (SchedSelIdx = 0 ; SchedSelIdx<=cpb_cnt_minus1 ; SchedSelIdx++)
      {
        Value = GetUVLC(Parser_p, "HRD - NAL - bit_rate_value_minus1");
        Value = GetUVLC(Parser_p, "HRD - NAL - cpb_size_value_minus1");
        if (hrd->SubPicParamsPresentFlag) {
          Value = GetUVLC(Parser_p, "HRD - NAL - cpb_size_du_value_minus1");
          Value = GetUVLC(Parser_p, "HRD - NAL - bit_rate_du_value_minus1");
        }
        Value = GetBits(Parser_p, 1, "HRD - NAL - cbr_flag");
      }
    }
    if (hrd->VclParamsPresentFlag) {
      for (SchedSelIdx = 0 ; SchedSelIdx<=cpb_cnt_minus1 ; SchedSelIdx++)
      {
        Value = GetUVLC(Parser_p, "HRD - VCL - bit_rate_value_minus1");
        Value = GetUVLC(Parser_p, "HRD - VCL - cpb_size_value_minus1");
        if (hrd->SubPicParamsPresentFlag) {
          Value = GetUVLC(Parser_p, "HRD - VCL - cpb_size_du_value_minus1");
          Value = GetUVLC(Parser_p, "HRD - VCL - bit_rate_du_value_minus1");
        }
        Value = GetBits(Parser_p, 1, "HRD - VCL - cbr_flag");
      }
    }
  }
}


/******************************************************************************
*
* Name:        ParseVUIParameters
*
* Parameters:  Sessiondata_p [In]     The decoder
*              VUIParameters [Out]    The VUI parameters set
*
* Returns:     -
*
* Description: Parses VUI data
*
******************************************************************************/
void
ParseVUIParameters(Parser_t *Parser_p, VuiParameterSet_t *vui, uint32 MaxNumSubLayer)
{
  uint32 Value;
  /* Default values */
  vui->HrdParametersPresentFlag = 0;
  /* Aspect ratio */
  Value = GetBits(Parser_p, 1, "SPS(VUI) - aspect_ratio_info_present_flag");
  if(Value)
  {
    Value = GetBits(Parser_p, 8, "SPS(VUI) - aspect_ratio_idc");
    if(Value == 255)
    {
      Value = GetBits(Parser_p, 16, "SPS(VUI) - sar_width");
      Value = GetBits(Parser_p, 16, "SPS(VUI) - sar_height");
    }
  }

  /* Overscan info */
  Value = GetBits(Parser_p, 1, "SPS(VUI) - overscan_info_present_flag");
  if(Value)
  {
    Value = GetBits(Parser_p, 1, "SPS(VUI) - overscan_appropriate_flag");
  }

  /* Video signal type */
  Value = GetBits(Parser_p, 1, "SPS(VUI) - video_signal_type_present_flag");
  if(Value)
  {
    Value = GetBits(Parser_p, 3, "SPS(VUI) - video_format");
    Value = GetBits(Parser_p, 1, "SPS(VUI) - video_full_range_flag");
    Value = GetBits(Parser_p, 1, "SPS(VUI) - colour_description_present_flag");
    if(Value)
    {
      Value = GetBits(Parser_p, 8, "SPS(VUI) - colour_primaries");
      Value = GetBits(Parser_p, 8, "SPS(VUI) - transfer_characteristics");
      Value = GetBits(Parser_p, 8, "SPS(VUI) - matrix_coefficients");
    }
  }

  /* Chroma loc info */
  Value = GetBits(Parser_p, 1, "SPS(VUI) - chroma_loc_info_present_flag");
  if(Value)
  {
    Value = GetUVLC(Parser_p, "SPS(VUI) - chroma_sample_loc_type_top_field");
    Value = GetUVLC(Parser_p, "SPS(VUI) - chroma_sample_loc_type_bottom_field");
  }

  /* Timing info */
  Value = GetBits(Parser_p, 1, "SPS(VUI) - neutral_chroma_indication_flag");
  Value = GetBits(Parser_p, 1, "SPS(VUI) - field_seq_flag");
  Value = GetBits(Parser_p, 1, "SPS(VUI) - frame_field_info_present_flag");
  vui->FrameFieldInfoPresentFlag = (Bool)Value;
  Value = GetBits(Parser_p, 1, "SPS(VUI) - default_display_window_flag");
  if(Value)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "vui_default_display_window_flag = 1 not supported yet"); 
  }
  Value = GetBits(Parser_p, 1, "SPS(VUI) - timing_info_present_flag");
  vui->TimmingInforPresentFlag = (Bool)Value;
  if(vui->TimmingInforPresentFlag)
  {
    Value = GetBits(Parser_p, 20, "SPS(VUI) - vui_num_units_in_tick"); // Read >29 not working so I split the reading... TODO: fix this!!!
    Value = GetBits(Parser_p, 12, "SPS(VUI) - vui_num_units_in_tick");
    Value = GetBits(Parser_p, 20, "SPS(VUI) - vui_time_scale"); // Read >29 not working so I split the reading... TODO: fix this!!!
    Value = GetBits(Parser_p, 12, "SPS(VUI) - vui_time_scale");
    Value = GetFlag(Parser_p,     "SPS(VUI) - vui_poc_proportional_to_timing_flag");
    if (Value)
      Value = GetUVLC(Parser_p,     "SPS(VUI) - vui_num_ticks_poc_diff_one_minus1");
    Value = GetFlag(Parser_p,     "SPS(VUI) - vui_hrd_parameters_present_flag");
    vui->HrdParametersPresentFlag = (Bool)Value;
    /* HRD parameters */
    if (Value)
      ParseHRDParameters(Parser_p, &vui->HrdParameters, MaxNumSubLayer);
  }

  Value = GetBits(Parser_p, 1, "SPS(VUI) - bitstream_restriction_flag");
  if(Value)
  {
    Value = GetBits(Parser_p, 1, "SPS(VUI) - tiles_fixed_structure_flag");
    Value = GetBits(Parser_p, 1, "SPS(VUI) - motion_vectors_over_pic_boundaries_flag");
    Value = GetBits(Parser_p, 1, "SPS(VUI) - restricted_ref_pic_lists_flag");
    Value = GetUVLC(Parser_p, "SPS(VUI) - min_spatial_segmentation_idc");
    Value = GetUVLC(Parser_p, "SPS(VUI) - max_bytes_per_pic_denom");
    Value = GetUVLC(Parser_p, "SPS(VUI) - max_bits_per_min_cu_denom");
    Value = GetUVLC(Parser_p, "SPS(VUI) - log2_max_mv_length_horizontal");
    Value = GetUVLC(Parser_p, "SPS(VUI) - log2_max_mv_length_vertical");
  }
}

/******************************************************************************
*
* Name:        DecodeVideoParameterSet
*
******************************************************************************/
void
DecodeVideoParameterSet(SessionData_t *Sessiondata_p)
{
  uint32 i;
  uint32 j;
  uint32 Value;
  uint32 sIdx;

  uint32 VideoParameterSetID;
  PTLData_t LocalPtl;
  uint32 MaxLayersMinus1;
  uint8  MaxTemporalLayersMinus1;
  DpbData_t LocalDpbData;
  uint32 SubLayerOrderingInfoPresentFlag;
  //uint32 AdditionalLayerOperationPoints;

  uint32 MaxLayerId;
  uint32 NumOpSetPoints;
  uint32 LayerIdIncludedFlag[MAX_OPSET_POINTS][MAX_LAYERS];
  uint8  TimingInfoPresentFlag;
  uint8  ExtensionFlag;
  sint32 DimensionIdLenMinus1[MAX_LAYERS];
  uint32 NuhLayerIdFlag;
  uint32 scalibilityTypes;
  uint32 ProfilePresentFlag[MAX_LAYERS];
  uint32 ProLayerSetRefMinus1[MAX_LAYERS];
  uint32 OutputLayerSetIdx[MAX_LAYERS];
  uint32 OutputLayerFlag[MAX_LAYERS][MAX_LAYERS];
  uint32 DimensionId[MAX_LAYERS][16];
  uint32 ScalabilityId[MAX_LAYERS][16];
  uint32 ViewId[MAX_LAYERS];
  uint32 NumOutputLayerSets = 0;
  Bool DirectDependencyFlag[MAX_LAYERS][MAX_LAYERS];
  sint32 RefLayerId[MAX_LAYERS][MAX_LAYERS];
  uint32 NumDirectRefLayers[MAX_LAYERS];
  sint32 LayerIdNuh[MAX_LAYERS];
  sint32 LayerIdVps[MAX_LAYERS];
  Bool ScalibilityMask[16];
  uint32 VpsNumHrdParameters;
  HrdParameterSet_t HrdParameters;

  VideoParameterSet_t *CurrVideoSet;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  scalibilityTypes = 0;
  for (i = 0; i < MAX_LAYERS; i++)
  {
    LayerIdNuh[i] = ( i == 0 ) ? 0 : -1;
    LayerIdVps[i] = ( i == 0 ) ? 0 : -1;
    DimensionIdLenMinus1[i] = -1;
    ViewId[i] = 0;
    NumDirectRefLayers[i] = 0;
    for (j = 0; j < MAX_LAYERS; j++)
    {
      DirectDependencyFlag [i][j] = 0;
      RefLayerId [i][j] = -1;
      if(j < 16)
      {
        DimensionId[i][j] = 0;
        ScalabilityId[i][j] = 0;
      }
    }
  }

  VideoParameterSetID = GetBits(Parser_p, 4, "VPS - video_parameter_set_id");
  Value = GetBits(Parser_p, 2, "VPS - reserved_three_2bits");

#if SUPPORT_EXTENSIONS
  MaxLayersMinus1 = GetBits(Parser_p, 6, "VPS - max_layers_minus1");
#else
  MaxLayersMinus1 = GetBits(Parser_p, 6, "VPS - reserved_zero_6bits");
#endif

  if((MaxLayersMinus1+1) > 10)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Not more than 10 layers allowed");
  }
  MaxTemporalLayersMinus1 = (uint8)GetBits(Parser_p, 3, "VPS - max_sub_layers_minus1");

  Value = GetBits(Parser_p, 1, "VPS - vps_temporal_id_nesting_flag");
  Value = GetBits(Parser_p, 16, "VPS - extension_offset");

  //Profile Tier and Level info
  ParsePTLParameters(Parser_p, &LocalPtl, MaxTemporalLayersMinus1);

  SubLayerOrderingInfoPresentFlag = GetBits(Parser_p, 1, "VPS - sub_layer_ordering_info_present_flag");
  i = SubLayerOrderingInfoPresentFlag ? 0 : MaxTemporalLayersMinus1;
  for(; i <= MaxTemporalLayersMinus1; i++)
  {
    LocalDpbData.MaxDecPicBuffering[i] = (uint8)GetUVLC(Parser_p, "VPS - max_dec_pic_buffering") + 1;
    LocalDpbData.MaxNumReorderPics[i]  = (uint8)GetUVLC(Parser_p, "VPS - max_num_reorder_pics");
    LocalDpbData.MaxLatencyIncrease[i] = GetUVLC(Parser_p, "VPS - max_latency_increase");
#pragma message("TODO - Check values of vps_max_dec_pic_buffering and vps_max_num_reorder_pics. Check also against MaxDpbSize")
  }

  MaxLayerId = GetBits(Parser_p, 6, "VPS - max_layer_id");
  NumOpSetPoints = GetUVLC(Parser_p, "VPS - vps_num_layer_sets_minus1");
  if (NumOpSetPoints>1023)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Number of layer sets between 0 and 1023 allowed");
  }

  for(i = 1;i <= NumOpSetPoints; i++)
  {
    for(j = 0;j <= MaxLayerId; j++)
    {
      LayerIdIncludedFlag[i][j] = GetBits(Parser_p, 1, "VPS - layer_id_included_flag");
    }
  }

  TimingInfoPresentFlag = (uint8) GetBits(Parser_p, 1, "VPS - timing_info_present_flag");
  if(TimingInfoPresentFlag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "vps_timing_info_present_flag = 1 not supported yet!");
  }
  if(TimingInfoPresentFlag)
  {
    Value = GetBits(Parser_p, 16, "VPS - num_units_in_tick");  //Read in 2 turns
    Value = GetBits(Parser_p, 16, "VPS - num_units_in_tick");  
    Value = GetBits(Parser_p, 16, "VPS - time_scale");         //Read in 2 turns
    Value = GetBits(Parser_p, 16, "VPS - time_scale");
    Value = GetBits(Parser_p, 1,  "VPS -  poc_proportional_to_timing_flag");
    if(Value)
    {
      Value = GetUVLC(Parser_p, "VPS - num_ticks_poc_diff_one_minus1");
    }
    VpsNumHrdParameters = GetUVLC(Parser_p, "VPS - num_hrd_parameters");
    for(i = 0; i < VpsNumHrdParameters; i++)
    {
      Value = GetUVLC(Parser_p, "VPS - hrd_layer_set_idx[ i ]");
      if(i > 0)
      {
        Value = GetBits(Parser_p, 1,  "VPS -  cprms_present_flag [i]");
      }
      ParseHRDParameters(Parser_p, &HrdParameters, MaxTemporalLayersMinus1 + 1);
    }
  }

  ExtensionFlag = (uint8) GetBits(Parser_p, 1, "VPS - extension_flag");
  if(ExtensionFlag)
  {
#if SUPPORT_EXTENSIONS
    while(!CheckByteAllignment(Sessiondata_p))
      Value = GetBits(Parser_p, 1, "VPS - extension_byte_alignment_reserved_one_bit");

    Value = GetBits(Parser_p, 1, "VPS - avc_base_layer_flag");
    Value = GetBits(Parser_p, 1, "VPS - splitting_flag");

    for (i = 0; i < 16 ; i++)
    {
      ScalibilityMask[i] = (Bool)GetBits(Parser_p, 1, "VPS - scalability_mask");
      scalibilityTypes += ScalibilityMask[i];
    }

    //dimension id for scalibility
    for(sIdx = 0; sIdx < scalibilityTypes; sIdx++ )
      DimensionIdLenMinus1[sIdx] = GetBits(Parser_p, 3, "VPS - dimension_id_len_minus1");

    NuhLayerIdFlag = GetBits(Parser_p, 1, "VPS - vps_nuh_layer_id_present_flag");

    for( i = 1; i <= MaxLayersMinus1; i++ ) 
    {
      uint32 layerIdInNuh;
      if(NuhLayerIdFlag)
        layerIdInNuh = GetBits(Parser_p, 6, "VPS - layer_id_in_nuh");
      else
        layerIdInNuh = i;

      LayerIdNuh [i] = layerIdInNuh;
      LayerIdVps [layerIdInNuh] = i;
      for( sIdx = 0; sIdx < scalibilityTypes; sIdx++ )
        DimensionId[i][sIdx] = GetBits(Parser_p, DimensionIdLenMinus1[sIdx]+1, "VPS - dimension_id[i]");
    }
    for (i = 0; i <= MaxLayersMinus1; i++) {
      for( sIdx= 0, j =0; sIdx< 16; sIdx ++ )
      {
        if(( i != 0 )  &&  ScalibilityMask[sIdx] ) 
          ScalabilityId[ i ][ sIdx ] = DimensionId[ i ][ j++ ];
        else 
          ScalabilityId[ i ][ sIdx ] = 0;
      }
      ViewId[ LayerIdNuh[ i ] ] = ScalabilityId[ i ][ 0 ];
    }

    for( i = 1; i <= NumOpSetPoints; i ++ ) 
    {
      ProfilePresentFlag[i] = GetBits(Parser_p, 1, "VPS - profile_present_flag[lsIdx]");
      if(!ProfilePresentFlag[i])
        ProLayerSetRefMinus1[i] = GetUVLC(Parser_p, "VPS - profile_layer_set_ref_minus1[lsIdx]");
      //Placeholder for Profile tier for other layers 
    }

    NumOutputLayerSets = GetUVLC(Parser_p, "VPS - num_output_layer_sets");
    if(NumOutputLayerSets)
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "num_output_layer_sets not supported yet");
    for (i=0;i<NumOutputLayerSets;i++)
    {
      OutputLayerSetIdx[i] = GetUVLC(Parser_p, "VPS - output_layer_set_idx[i]");
      for (j=0;j<MaxLayerId;j++)
        if(LayerIdIncludedFlag[OutputLayerSetIdx[i]][j])
          OutputLayerFlag[OutputLayerSetIdx[i]][j] = GetBits(Parser_p, 1, "VPS - output_layer_flag[ lsIdx ][ j ]");
    }

    for( i = 1; i <= MaxLayersMinus1; i++ )
      for(j=0;j<i;j++)
        DirectDependencyFlag[ i ][ j ] = GetBits(Parser_p, 1, "VPS - direct_dependency_flag[ i ][ j ]");

    for( i = 1; i <= MaxLayersMinus1; i++ )
      for( j = 0, NumDirectRefLayers[ i ] = 0; j < i; j++ )
        if( DirectDependencyFlag[ i ][ j ] )
          RefLayerId[ i ][ NumDirectRefLayers[ i ]++ ] = LayerIdNuh[ j ];

    Value = GetBits(Parser_p, 1, "VPS - extension2_flag");
#endif
  }
  GetRbspTrailingBits(Parser_p);

  /* Allocate Video parameter set */
  if(Sessiondata_p->VideoParameterSetArray[VideoParameterSetID] == NULL)
  {
    Sessiondata_p->VideoParameterSetArray[VideoParameterSetID] =
      (VideoParameterSet_t*) Sessiondata_p->Malloc_Function_p(sizeof(VideoParameterSet_t), D65_INTERNAL_DATA);
    if(Sessiondata_p->VideoParameterSetArray[VideoParameterSetID] == NULL)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Could not allocate VPS");
    }
  }

  CurrVideoSet = Sessiondata_p->VideoParameterSetArray[VideoParameterSetID];
  CurrVideoSet->VpsPtl = LocalPtl;

  CurrVideoSet->MaxSubLayers = (uint8)(MaxTemporalLayersMinus1+1);
  CurrVideoSet->SubLDpbInfoPresent = (uint8)SubLayerOrderingInfoPresentFlag;
  CurrVideoSet->DpbData = LocalDpbData;
  CurrVideoSet->DpbSize = MAX_DPB_SIZE;

  CurrVideoSet->MaxLayers = MaxLayersMinus1+1;
  CurrVideoSet->MaxLayerId = MaxLayerId;
  CurrVideoSet->NumOutputLayerSets = NumOutputLayerSets;

  for (i = 0; i < MAX_LAYERS; i++)
  {
    CurrVideoSet->LayerIdNuh[i] = LayerIdNuh[i];
    CurrVideoSet->LayerIdVps[i] = LayerIdVps[i];
    for( j = 0; j < MAX_LAYERS; j++ )
    {
      if(j < 16)
      {
        CurrVideoSet->ScalabilityId[i][j] = ScalabilityId[i][j];  //ScalabilityId[ i ][ smIdx ] specifying the identifier of the smIdx-th scalability dimension type of the i-th layer
        if(i == 0)
          CurrVideoSet->ScalibilityMask[j] = ScalibilityMask[j];
      }

      CurrVideoSet->DirectDependencyFlag [i][j] = 1;
      CurrVideoSet->RefLayerId [i][j] = -1;
      if (j < i)
      {
        CurrVideoSet->DirectDependencyFlag [i][j] = DirectDependencyFlag [i][j];
        CurrVideoSet->RefLayerId [i][j] = RefLayerId [i][j];       // Specifies which layers are used as reference by a certain layer
      }
    } 
    CurrVideoSet->NumDirectRefLayers [i] = NumDirectRefLayers [i]; // Specifies how many reference layers are used for specific layer
    CurrVideoSet->ViewId [i] = ViewId [i];
  }
  Sessiondata_p->CurrentVideoParameterSet = CurrVideoSet;
}


/******************************************************************************
*
* Name:        DecodeSequenceParameterSet
*
******************************************************************************/
void
DecodeSequenceParameterSet(SessionData_t *Sessiondata_p)
{
  uint32 i;
  uint32 Value;
  uint32 VpsId;
  uint32 MaxTemporalLayers;
  uint32 TemporalIdNestingFlag;
  PTLData_t LocalPtl;
  uint32 SequenceParameterSetID;
  uint32 ChromaFormat;
  
  uint32 PictureWidth;
  uint32 PictureHeight;

  uint32 ConfWindowFlag;
  uint32 pic_crop_left_offset = 0;
  uint32 pic_crop_right_offset = 0;
  uint32 pic_crop_top_offset = 0;
  uint32 pic_crop_bottom_offset = 0;

  uint32 BitDepthLuma;
  uint32 BitDepthChroma;
  uint32 Log2MaxPicOrderCntLsb;
  uint32 SubLayerOrderingInfoPresentFlag;
  DpbData_t LocalDpbData;

  uint32 Log2MinCbSizeY;
  uint32 Log2DiffMaxMinLumaCodingBlockSize;
  uint32 Log2MinTrafoSize;
  uint32 Log2MaxTrafoSize;
  uint32 max_transform_hierarchy_depth_inter;
  uint32 max_transform_hierarchy_depth_intra;

  uint32 scaling_list_enable_flag;
  uint32 scaling_list_data_present_flag = 0;
  ScalingListData_t LocalScalingListData;


  uint32 AmpEnabledFlag;
  uint32 sample_adaptive_offset_enabled_flag;
  uint32 PcmEnabledFlag;
  PcmData_t LocalPcmData;

  uint32 num_short_term_ref_pic_sets;
  ReferencePictureSet_t sps_reference_picture_set[MAX_RPS_IN_SPS];
  //This is a temp ReferencePictureSet array and it is used for 
  //storing the reference picture set when parsing. After correctly 
  //parsed the rest of syntax in SPS, copy it to reference picture set array 
  //in current SPS

  uint32 long_term_ref_pics_present_flag;
  uint32 num_long_term_ref_pics_sps = 0;
  uint32 lt_ref_pic_poc_lsb_sps[MAX_LTRPS_IN_SPS];
  uint32 used_by_curr_pic_lt_sps_flag[MAX_LTRPS_IN_SPS];

  uint32 temporal_mvp_enabled_flag;
  uint32 StrongIntraSmoothingFlag;
  Bool   vui_parameters_present_flag;
  VuiParameterSet_t vui;
  uint32 sps_extension_flag;

  uint32 PicSizeInSamplesY;
  uint32 MaxLumaPS;
  static const uint32 MaxDpbPicBuf = 6;
  uint32 MaxDpbSize;
  SequenceParameterSet_t *CurrParamSet;
  uint32 FirstParameterSetFlag;
  uint32 Log2CtbSizeY;
  uint32 MinCbSizeY;
  uint32 CtbSizeY;

  uint32 PicWidthInMinCbsY;
  uint32 PicHeightInMinCbsY;
  uint32 PicWidthInCtbsY; 
  uint32 PicHeightInCtbsY;
  uint32 PicSizeInMinCbsY;
  Parser_t *Parser_p = &Sessiondata_p->Parser;

  VpsId = GetBits(Parser_p, 4, "SPS - video_parameter_set_id");
  if(VpsId > MAX_VPS_NUM)
  {
    STB_ERROR("Video parameter set ID out-of-bounds");
  }

  MaxTemporalLayers     = GetBits(Parser_p, 3, "SPS - sps_max_sub_layers_minus1") + 1;

  TemporalIdNestingFlag = GetFlag(Parser_p, "SPS - temporal_id_nesting_flag");

  ParsePTLParameters(Parser_p, &LocalPtl, MaxTemporalLayers - 1);

  SequenceParameterSetID = GetUVLC(Parser_p, "SPS - seq_parameter_set_id");
  if(SequenceParameterSetID > MAX_SPS_NUM)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Sequence parameter set ID out-of-bounds");
  }

  ChromaFormat = GetUVLC(Parser_p, "SPS - chroma_format_idc");
  if(ChromaFormat > 1)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Unsupported chroma_format_idc");
  }

  // Decode picture size
  PictureWidth      = GetUVLC(Parser_p, "SPS - pic_width_in_luma_samples" );
  PictureHeight     = GetUVLC(Parser_p, "SPS - pic_height_in_luma_samples");
  PicSizeInSamplesY = PictureWidth * PictureHeight;
  MaxLumaPS         = LocalPtl.GeneralLimits.MaxLumsPs;

  if(PicSizeInSamplesY <= (MaxLumaPS >> 2 ))
  {
    MaxDpbSize = 4 * MaxDpbPicBuf;
  }
  else if (PicSizeInSamplesY <= ( MaxLumaPS >> 1 ))
  {
    MaxDpbSize = 2 * MaxDpbPicBuf;
  }
  else if (PicSizeInSamplesY <= ( MaxLumaPS << 1) / 3)
  {
    MaxDpbSize = (3 * MaxDpbPicBuf) >> 1;
  }
  else if (PicSizeInSamplesY <= ( ( 3 * MaxLumaPS ) >> 2 ))
  {
    MaxDpbSize = (4 * MaxDpbPicBuf) / 3;
  }
  else
  {
    MaxDpbSize = MaxDpbPicBuf;
  }
  if(MaxDpbSize > 16)
  {
    MaxDpbSize = 16;
  }

  ConfWindowFlag           = GetFlag(Parser_p, "SPS - conformance_window_flag");
  if(ConfWindowFlag)
  {
    pic_crop_left_offset   = GetUVLC(Parser_p, "SPS - conf_win_left_offset"   );
    pic_crop_right_offset  = GetUVLC(Parser_p, "SPS - conf_win_right_offset"  );
    pic_crop_top_offset    = GetUVLC(Parser_p, "SPS - conf_win_top_offset"    );
    pic_crop_bottom_offset = GetUVLC(Parser_p, "SPS - conf_win_bottom_offset" );
  }

  BitDepthLuma = GetUVLC(Parser_p, "SPS - bit_depth_luma_minus8") + 8;
  if(BitDepthLuma != 8)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "bit_depth_luma_minus8 must be equal to 0");
  }

  BitDepthChroma = GetUVLC(Parser_p, "SPS - bit_depth_chroma_minus8") + 8;
  if(BitDepthChroma != 8)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "bit_depth_chroma_minus8 must be equal to 0");
  }

  Log2MaxPicOrderCntLsb = GetUVLC(Parser_p, "SPS - log2_max_pic_order_cnt_lsb_minus4") + 4;

  SubLayerOrderingInfoPresentFlag = (uint8)GetFlag(Parser_p, "SPS - sub_layer_ordering_info_present_flag");
  i = SubLayerOrderingInfoPresentFlag ? 0 : MaxTemporalLayers - 1;
  for( ; i < MaxTemporalLayers ; i++)
  {
    LocalDpbData.MaxDecPicBuffering[i] = (uint8)GetUVLC(Parser_p, "SPS - sps_max_dec_pic_buffering") + 1;
    LocalDpbData.MaxNumReorderPics[i]  = (uint8)GetUVLC(Parser_p, "SPS - sps_max_num_reorder_pics");
    LocalDpbData.MaxLatencyIncrease[i] = GetUVLC(Parser_p, "SPS - sps_max_latency_increase");
#pragma message("TODO - Check values of sps_max_dec_pic_buffering and sps_max_num_reorder_pics. Check also against MaxDpbSize")
  }

  Log2MinCbSizeY = GetUVLC(Parser_p, "SPS - log2_min_luma_coding_block_size_minus3") + 3;
  Log2DiffMaxMinLumaCodingBlockSize = GetUVLC(Parser_p, "SPS - log2_diff_max_min_luma_coding_block_size");
  Log2CtbSizeY       = Log2MinCbSizeY + Log2DiffMaxMinLumaCodingBlockSize;

  if(Log2CtbSizeY > 6 || Log2CtbSizeY < 4)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Log2CtbSizeY out of range");
    //CtbLog2SizeY derived according to active SPSs for the base layer shall be in the range of 4 to 6, inclusive.
  }

  MinCbSizeY         = 1 << Log2MinCbSizeY;
  CtbSizeY           = 1 << Log2CtbSizeY;
  PicWidthInCtbsY    = (PictureWidth  + CtbSizeY - 1) / CtbSizeY;
  PicHeightInCtbsY   = (PictureHeight + CtbSizeY - 1) / CtbSizeY;
  PicWidthInMinCbsY  = PicWidthInCtbsY   * (1 << Log2DiffMaxMinLumaCodingBlockSize);//PictureWidth / MinCbSizeY;
  PicHeightInMinCbsY = PicHeightInCtbsY  * (1 << Log2DiffMaxMinLumaCodingBlockSize); //PictureHeight / MinCbSizeY;
  PicSizeInMinCbsY   = PicWidthInMinCbsY * PicHeightInMinCbsY;

  Log2MinTrafoSize = GetUVLC(Parser_p, "SPS - log2_min_transform_block_size_minus2") + 2;
  if(Log2MinTrafoSize >= Log2MinCbSizeY)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "log2_min_transform_block_size_minus2 too large");
  }
  Log2MaxTrafoSize = Log2MinTrafoSize + GetUVLC(Parser_p, "SPS - log2_diff_max_min_transform_block_size");

  if (Log2MaxTrafoSize < 2 || Log2MaxTrafoSize > 5)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Log2MaxTrafoSize out of range");
  }

  max_transform_hierarchy_depth_inter = GetUVLC(Parser_p, "SPS - max_transform_hierarchy_depth_inter");
  // Check: The value of max_transform_hierarchy_depth_inter shall be in the range of 0 to Log2CtbSizeY - Log2MinTrafoSize, inclusive.
  if(max_transform_hierarchy_depth_inter > Log2CtbSizeY - Log2MinTrafoSize)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "max_transform_hierarchy_depth_inter too large");
  }

  max_transform_hierarchy_depth_intra = GetUVLC(Parser_p, "SPS - max_transform_hierarchy_depth_intra");
  // Check: The value of max_transform_hierarchy_depth_intra shall be in the range of 0 to Log2CtbSizeY - Log2MinTrafoSize, inclusive.
  if(max_transform_hierarchy_depth_intra > Log2CtbSizeY - Log2MinTrafoSize)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "max_transform_hierarchy_depth_intra too large");
  }

  scaling_list_enable_flag = GetBits(Parser_p, 1, "SPS - scaling_list_enabled_flag");
  SetDefaultScalingList(&LocalScalingListData);
  if(scaling_list_enable_flag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "scaling_list_enabled_flag=1 not supported yet");
    scaling_list_data_present_flag = GetBits(Parser_p, 1, "SPS - scaling_list_data_present_flag");
    if(scaling_list_data_present_flag)
    {
      ParseScalingListData(Parser_p, &LocalScalingListData);
    }
  }
  AmpEnabledFlag = GetBits(Parser_p, 1, "SPS - asymmetric_motion_partitions_enabled_flag");

  sample_adaptive_offset_enabled_flag = GetBits(Parser_p, 1, "SPS - sample_adaptive_offset_enabled_flag");
  PcmEnabledFlag = GetBits(Parser_p, 1, "SPS - pcm_enabled_flag");
  SetDefaultPcmData(&LocalPcmData);
  if(PcmEnabledFlag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "pcm_enabled_flag must be equal to 0");
    LocalPcmData.pcm_sample_bit_depth_luma = (uint8)GetBits(Parser_p, 4, "SPS - pcm_sample_bit_depth_luma_minus1") + 1;
    LocalPcmData.pcm_sample_bit_depth_chroma = (uint8)GetBits(Parser_p, 4, "SPS - pcm_sample_bit_depth_chroma_minus1") + 1;
    Value = GetUVLC(Parser_p, "SPS - log2_min_pcm_luma_coding_block_size_minus3") + 3;
    LocalPcmData.log2_min_pcm_cu_size = (uint8)Value;
    LocalPcmData.log2_max_pcm_cu_size = (uint8)(Value + GetUVLC(Parser_p, "SPS - log2_diff_max_min_pcm_luma_coding_block_size"));
    LocalPcmData.pcm_loof_filter_disabled_flag = (uint8)GetBits(Parser_p, 1, "SPS - pcm_loop_filter_disabled_flag");
  }

  num_short_term_ref_pic_sets = GetUVLC(Parser_p, "SPS - num_short_term_ref_pic_sets");
  if(num_short_term_ref_pic_sets < 0 || num_short_term_ref_pic_sets > MAX_RPS_IN_SPS)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "num_short_term_ref_pic_sets out of range");
  }
  if(num_short_term_ref_pic_sets)
  {
    for(i = 0; i < num_short_term_ref_pic_sets; i++)
    {
      ParseReferencePictureSet(Sessiondata_p, &(sps_reference_picture_set[i]), 
                               sps_reference_picture_set, num_short_term_ref_pic_sets, i);
    }
  }
  long_term_ref_pics_present_flag = (uint8) GetBits(Parser_p, 1, "SPS - long_term_ref_pics_present_flag");
  if(long_term_ref_pics_present_flag)
  {
    num_long_term_ref_pics_sps = GetUVLC(Parser_p, "SPS - num_long_term_ref_pics_sps");
    if(num_long_term_ref_pics_sps)
    {
      for(i = 0; i < num_long_term_ref_pics_sps; i++)
      {
        lt_ref_pic_poc_lsb_sps[i] = GetUVLC(Parser_p, "SPS - lt_ref_pic_poc_lsb_sps[i]");
        used_by_curr_pic_lt_sps_flag[i] = GetBits(Parser_p, 1, "used_by_curr_pic_lt_sps_flag[i]");
      }
    }
  }

  temporal_mvp_enabled_flag = GetBits(Parser_p, 1, "SPS - sps_temporal_mvp_enable_flag");
  StrongIntraSmoothingFlag = GetBits(Parser_p, 1, "SPS - strong_intra_smoothing_enable_flag");

  Value = GetBits(Parser_p, 1, "SPS - vui_parameters_present_flag");
  vui_parameters_present_flag = (Bool)Value;
  if(vui_parameters_present_flag)
  {
    ParseVUIParameters(Parser_p, &vui, MaxTemporalLayers);
  } 
  sps_extension_flag = GetBits(Parser_p, 1, "SPS - sps_extension_flag");
  if(sps_extension_flag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "sps_extension_flag=1 not supported yet");
  }

  GetRbspTrailingBits(Parser_p);

  // If this is the first sequence parameter set, then this sequence parameter
  // set is made current and memory is allocated. Additional checks for
  // sequence parameter sets is performed in the DecodeSliceHeader() functiom.
  FirstParameterSetFlag = 1;
  for(i=0 ; i<32 ; i++)
  {
    if(Sessiondata_p->SequenceParameterSetArray[i] != NULL)
    {
      FirstParameterSetFlag = 0;
      break;
    }
  }

  // Allocate Sequence Parameter Set
  if(Sessiondata_p->SequenceParameterSetArray[SequenceParameterSetID] == NULL)
  {
    Sessiondata_p->SequenceParameterSetArray[SequenceParameterSetID] =
      (SequenceParameterSet_t*) Sessiondata_p->Malloc_Function_p(sizeof(SequenceParameterSet_t), D65_INTERNAL_DATA);
    if(Sessiondata_p->SequenceParameterSetArray[SequenceParameterSetID] == NULL)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Error when allocating sequence parameter set");
    }
  }

  // Copy sequence parameter set to Sessiondata
  CurrParamSet = Sessiondata_p->SequenceParameterSetArray[SequenceParameterSetID];
  CurrParamSet->VpsId  = (uint8)VpsId;
  CurrParamSet->MaxSubLayers = (uint8)MaxTemporalLayers;
  CurrParamSet->SpsPtl = LocalPtl;
  CurrParamSet->chromaFormatIDC = (ChromaFormatIDC)ChromaFormat;
  CurrParamSet->PictureHeight = (uint16)PictureHeight;
  CurrParamSet->PictureWidth = (uint16)PictureWidth;
  CurrParamSet->PicSizeInSamplesY = (PictureHeight*PictureWidth);

  CurrParamSet->pic_crop_left_offset   = (uint16)pic_crop_left_offset;
  CurrParamSet->pic_crop_right_offset  = (uint16)pic_crop_right_offset;
  CurrParamSet->pic_crop_top_offset    = (uint16)pic_crop_top_offset;
  CurrParamSet->pic_crop_bottom_offset = (uint16)pic_crop_bottom_offset;
  
  CurrParamSet->BitDepthLuma = (uint16)BitDepthLuma;
  CurrParamSet->BitDepthChroma = (uint16)BitDepthChroma;
  CurrParamSet->Log2MaxPicOrderCntLsb = (uint8)Log2MaxPicOrderCntLsb;
  CurrParamSet->SubLDpbInfoPresent = (uint8)SubLayerOrderingInfoPresentFlag;
  CurrParamSet->DpbData = LocalDpbData;
  //CurrParamSet->DpbSize         = (uint8) (MaxDecPicBuffering + NumReorderPics);
  CurrParamSet->DpbSize = MAX_DPB_SIZE;

  CurrParamSet->PicSizeInMinCbsY = PicSizeInMinCbsY;
  CurrParamSet->PicHeightInMinCbsY = PicHeightInMinCbsY;
  CurrParamSet->PicWidthInMinCbsY  = PicWidthInMinCbsY;
  CurrParamSet->PicWidthInCtbsY    = PicWidthInCtbsY;
  CurrParamSet->PicHeightInCtbsY   = PicHeightInCtbsY;
  CurrParamSet->Log2MinCbSizeY = Log2MinCbSizeY;
  CurrParamSet->Log2CtbSizeY = Log2CtbSizeY;
  CurrParamSet->MinCbSizeY = MinCbSizeY;
  CurrParamSet->CtbSizeY = CtbSizeY;
  CurrParamSet->Log2MinTrafoSize = Log2MinTrafoSize;
  CurrParamSet->Log2MaxTrafoSize = Log2MaxTrafoSize;

  CurrParamSet->Log2MinUnitSize = 2; // D65_MIN(Log2MinCbSizeY - 1, Log2MinTrafoSize);
  CurrParamSet->PicWidthInMinUnit  = PicWidthInCtbsY  << (Log2CtbSizeY - CurrParamSet->Log2MinUnitSize);
  CurrParamSet->PicHeightInMinUnit = PicHeightInCtbsY << (Log2CtbSizeY - CurrParamSet->Log2MinUnitSize);
  CurrParamSet->PicSizeInMinUnit   = CurrParamSet->PicWidthInMinUnit * CurrParamSet->PicHeightInMinUnit;

  CurrParamSet->max_transform_hierarchy_depth_inter = max_transform_hierarchy_depth_inter;
  CurrParamSet->max_transform_hierarchy_depth_intra = max_transform_hierarchy_depth_intra;

  CurrParamSet->ScalingListEnabledFlag = (uint8)scaling_list_enable_flag;
  CurrParamSet->ScalingListDataPresent = (uint8)scaling_list_data_present_flag;
  CurrParamSet->ScalingListData = LocalScalingListData;
  
  CurrParamSet->AmpEnabledFlag = (uint8)AmpEnabledFlag;
  CurrParamSet->SampleAdaptiveOffsetEnabledFlag = (uint8)sample_adaptive_offset_enabled_flag;
  CurrParamSet->PcmEnabledFlag = (uint8)PcmEnabledFlag;
  CurrParamSet->PcmData = LocalPcmData;
  
  CurrParamSet->num_short_term_rps_sets = num_short_term_ref_pic_sets;
  for(i = 0; i < num_short_term_ref_pic_sets; i++)
  {
    CurrParamSet->RPSsetInSPS[i] = sps_reference_picture_set[i];
  }

  CurrParamSet->LongTermRefPicsPresentFlag = (uint8) long_term_ref_pics_present_flag;
  CurrParamSet->num_long_term_ref_pics_sps = num_long_term_ref_pics_sps;
  for(i = 0; i < num_long_term_ref_pics_sps; i++)
  {
    CurrParamSet->LtRefPicPocLsbSps[i] = lt_ref_pic_poc_lsb_sps[i];
    CurrParamSet->UsedByCurrPicLtSpsFlag[i] = used_by_curr_pic_lt_sps_flag[i];
  }

  CurrParamSet->TemporalMvpEnabledFlag = (uint8)temporal_mvp_enabled_flag;
  CurrParamSet->StrongIntraSmoothingFlag = (uint8)StrongIntraSmoothingFlag;
  CurrParamSet->VuiParametersPresentFlag = vui_parameters_present_flag;
  CurrParamSet->VuiParameters = vui;
}

/******************************************************************************
*
* Name:        DecodePictureParameterSet
*
******************************************************************************/
void
DecodePictureParameterSet(SessionData_t *Sessiondata_p)
{
  uint32 i;

  uint32 PictureParameterSetID;
  uint32 SequenceParameterSetID;
  uint32 dependent_slice_seg_enabled_flag;
  uint32 output_flag_present_flag;
  uint32 num_extra_slice_header_bits;
  uint32 sign_data_hiding_enabled_flag;
  uint32 cabac_init_present_flag;
  uint32  num_ref_idx_l0_default_active_minus1;
  uint32  num_ref_idx_l1_default_active_minus1;
  sint32 PicInitQP;

  uint32 ConstrainedIntraPredFlag;
  uint32 transform_skip_enabled_flag;
  uint32 cu_qp_delta_enabled_flag;
  uint32 cu_qp_delta_depth = 0;
  sint32 QpOffsetCb;
  sint32 QpOffsetCr;
  uint32 slice_chroma_qp_offsets_present;
  uint32 weighted_pred_flag;
  uint32 weighted_bipred_flag;
  uint32 transquant_bypass_enable_flag;

  uint32 TilesEnabledFlag;
  uint32 EntropyCodingSyncEnabledFlag;

  uint32 num_tile_columns_minus1;
  uint32 num_tile_rows_minus1;
  uint32 uniform_spacing_flag = 1;
  uint32 column_width_minus1[256];
  uint32 row_height_minus1[256];
  uint32 loop_filter_across_tiles_enabled_flag = 0;

  uint32 loop_filter_across_slices_enabled_flag;
  uint32 deblocking_filter_control_present_flag;
  uint32 deblocking_filter_override_enabled_flag;
  uint32 pic_disable_deblocking_filter_flag;
  sint32 beta_offset = 0;
  sint32 tc_offset = 0;

  uint32 scaling_list_data_present_flag;
  ScalingListData_t LocalScalListData;
  uint32 lists_modification_present_flag;
  uint32 log2_parallel_merge_level;
  uint32 slice_seg_header_extesion_present;
  uint32 pps_extension_flag;

  uint32 FirstParameterSetFlag;
  Parser_t *Parser_p = &Sessiondata_p->Parser;
  PictureParameterSet_t *CurrPicSet;

  // The ID shall be in the range of 0 to 63 inclusive
  PictureParameterSetID = GetUVLC(Parser_p, "PPS - pic_parameter_set_id");
  if(PictureParameterSetID > MAX_PPS_NUM)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "Picture parameter set ID out-of-bounds");
  }

  // The ID shall be in the range of 0 to 15 inclusive
  SequenceParameterSetID = GetUVLC(Parser_p, "PPS - seq_parameter_set_id");
  if(SequenceParameterSetID > MAX_SPS_NUM)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "Non-existing sequence parameter set indicated");
  }

  dependent_slice_seg_enabled_flag = GetBits(Parser_p, 1, "PPS - dependent_slice_segments_enabled_flag");
  if(dependent_slice_seg_enabled_flag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Dependent slice not supported yet");
  }
  output_flag_present_flag = GetBits(Parser_p, 1, "PPS - output_flag_present_flag");
  if(output_flag_present_flag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "pps_output_flag_present_flag = 1 not supported yet");
  }
  num_extra_slice_header_bits = GetBits(Parser_p, 3, "PPS - num_extra_slice_header_bits");
  sign_data_hiding_enabled_flag = GetBits(Parser_p, 1, "PPS - sign_data_hiding_flag");

  cabac_init_present_flag = GetBits(Parser_p, 1, "PPS - cabac_init_present_flag");

  num_ref_idx_l0_default_active_minus1 = (sint8)GetUVLC(Parser_p, "PPS - num_ref_idx_l0_default_active_minus1");
  if(num_ref_idx_l0_default_active_minus1 > 14)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_ref_idx_l0_active_minus1 shall not exceed 14");
  }

  num_ref_idx_l1_default_active_minus1 = (sint8)GetUVLC(Parser_p, "PPS - num_ref_idx_l1_default_active_minus1");
  if(num_ref_idx_l1_default_active_minus1 > 14)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "num_ref_idx_l1_active_minus1 shall not exceed 14");
  }

  PicInitQP = 26 + GetSVLC(Parser_p, "PPS - pic_init_qp_minus26");
  if(PicInitQP > 51 || PicInitQP < 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_SYNTAX_VALUE_EXCEEDED, "pic_init_qp_minus26 out-of-bounds");
  }

  ConstrainedIntraPredFlag = GetBits(Parser_p, 1, "PPS - constrained_intra_pred_flag");
  transform_skip_enabled_flag = GetBits(Parser_p, 1, "PPS - transform_skip_enabled_flag");

  cu_qp_delta_enabled_flag = GetBits(Parser_p, 1, "PPS - cu_qp_delta_enabled_flag");
  if(cu_qp_delta_enabled_flag)
  {
    cu_qp_delta_depth = GetUVLC(Parser_p, "PPS - diff_cu_qp_delta_depth");
  }

  QpOffsetCb = GetSVLC(Parser_p, "PPS - pic_cb_qp_offset");
  QpOffsetCr = GetSVLC(Parser_p, "PPS - pic_cr_qp_offset");

  slice_chroma_qp_offsets_present = GetBits(Parser_p, 1, "PPS - pic_slice_level_chroma_qp_offsets_present_flag");
  if(slice_chroma_qp_offsets_present != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "pic_slice_level_chroma_qp_offsets_present_flag=1 not supported yet");
  }
  weighted_pred_flag = GetBits(Parser_p, 1, "PPS - weighted_pred_flag");
  if(weighted_pred_flag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "weighted_pred_flag=1 not supported yet");
  }
  weighted_bipred_flag = GetBits(Parser_p, 1, "PPS - weighted_bipred_flag");
  if(weighted_bipred_flag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "weighted_bipred_flag=1 not supported yet");
  }
  transquant_bypass_enable_flag = GetBits(Parser_p, 1, "PPS - transquant_bypass_enable_flag");
  if(transquant_bypass_enable_flag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "transquant_bypass_enable_flag=1 not supported yet");
  }

  TilesEnabledFlag = GetBits(Parser_p, 1, "PPS - tiles_enabled_flag");
  EntropyCodingSyncEnabledFlag = GetBits(Parser_p, 1, "PPS - entropy_coding_sync_enabled_flag");
  if(EntropyCodingSyncEnabledFlag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "entropy_coding_sync_enabled_flag=1 not supported yet");
  }

  if(TilesEnabledFlag)
  {
    num_tile_columns_minus1 = GetUVLC(Parser_p, "PPS - num_tile_columns_minus1");
    if(num_tile_columns_minus1 > 255)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "num_tile_columns_minus1 larger than 255 not supported yet");
    }

    num_tile_rows_minus1 = GetUVLC(Parser_p, "PPS - num_tile_rows_minus1");
    if(num_tile_rows_minus1 > 255)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "num_tile_rows_minus1 larger than 255 not supported yet");
    }

    uniform_spacing_flag = GetBits(Parser_p, 1, "PPS - uniform_spacing_flag");
    if(!uniform_spacing_flag)
    {
      for( i = 0; i < num_tile_columns_minus1; i++ )
      {
        column_width_minus1[i] = GetUVLC(Parser_p, "PPS - column_width_minus1[i]");

      }
      for( i = 0; i < num_tile_rows_minus1; i++ )
      {
        row_height_minus1[i] = GetUVLC(Parser_p, "PPS - row_height_minus1[i]");
      }
    }

    
    if (num_tile_rows_minus1 > 0 || num_tile_columns_minus1 > 0) {
      loop_filter_across_tiles_enabled_flag = GetBits(Parser_p, 1, "PPS - loop_filter_across_tiles_enabled_flag");
      if(loop_filter_across_tiles_enabled_flag == 0)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "loop_filter_across_tiles_enabled_flag=0 not supported yet");
      }
    }
  }
  else
  {
    num_tile_columns_minus1 = 0; // "When not present, the value of num_tile_columns_minus1 is inferred to be equal to 0."
    num_tile_rows_minus1 = 0;// "When not present, the value of num_tile_rows_minus1 is inferred to be equal to 0."
    uniform_spacing_flag = 1;
  }

  loop_filter_across_slices_enabled_flag = GetBits(Parser_p, 1, "PPS - loop_filter_across_slices_enabled_flag");
  if(loop_filter_across_slices_enabled_flag != 0) //INFO HM set it to 1 even when no slices...
  {
    STB_WARNING("loop_filter_across_slices_enabled_flag = 1 not supported yet, possible decoding mismatch!");
  }
  deblocking_filter_control_present_flag = GetBits(Parser_p, 1, "PPS - deblocking_filter_control_present_flag");
  if(deblocking_filter_control_present_flag)
  {
    deblocking_filter_override_enabled_flag = GetBits(Parser_p, 1, "PPS - deblocking_filter_override_enabled_flag");
    pic_disable_deblocking_filter_flag = GetBits(Parser_p, 1, "PPS - pic_disable_deblocking_filter_flag");
    if(!pic_disable_deblocking_filter_flag)
    {
      beta_offset  = GetSVLC(Parser_p, "PPS - beta_offset_div2") << 1;
      tc_offset    = GetSVLC(Parser_p, "PPS - tc_offset_div2") << 1;
    }
  }
  else
  {
    deblocking_filter_override_enabled_flag = 0;
    pic_disable_deblocking_filter_flag = 0;
  }
  scaling_list_data_present_flag = GetBits(Parser_p, 1, "PPS - pic_scaling_list_data_present_flag");
  SetDefaultScalingList(&LocalScalListData);
  if(scaling_list_data_present_flag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "pic_scaling_list_data_present_flag=1 not supported yet");
    ParseScalingListData(Parser_p, &LocalScalListData);
  }

  lists_modification_present_flag = GetBits(Parser_p, 1, "PPS - lists_modification_present_flag");
  if(lists_modification_present_flag)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "lists_modification_present_flag=1 not supported yet");
  }
  log2_parallel_merge_level = GetUVLC(Parser_p, "PPS - log2_parallel_merge_level_minus2") + 2;

  slice_seg_header_extesion_present = GetBits(Parser_p, 1, "PPS - slice_header_extension_present_flag");
  if(slice_seg_header_extesion_present != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "loop_filter_across_tiles_enabled_flag=1 not supported yet");
  }
  pps_extension_flag = GetBits(Parser_p, 1, "PPS - pps_extension_flag");
  if(pps_extension_flag != 0)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "extension_flag=1 not supported yet");
  }

  GetRbspTrailingBits(Parser_p);
  FirstParameterSetFlag = 1;
  for(i = 0; i < MAX_PPS_NUM;i++)
  {
    if(Sessiondata_p->PictureParameterSetArray[i] != NULL)
    {
      FirstParameterSetFlag = 0;
      break;
    }
  }

  /* Allocate picture parameter set */
  if(Sessiondata_p->PictureParameterSetArray[PictureParameterSetID] == NULL)
  {
    Sessiondata_p->PictureParameterSetArray[PictureParameterSetID] =
      (PictureParameterSet_t*) Sessiondata_p->Malloc_Function_p(sizeof(PictureParameterSet_t), D65_INTERNAL_DATA);
    if(Sessiondata_p->PictureParameterSetArray[PictureParameterSetID] == NULL)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Could not allocate PPS");
    }
  }

  CurrPicSet = Sessiondata_p->PictureParameterSetArray[PictureParameterSetID];
  CurrPicSet->SequenceParameterSetID               = (uint8)SequenceParameterSetID;
  CurrPicSet->PictureParameterSetID                = (uint8)PictureParameterSetID;
  CurrPicSet->DependentSliceSegEnabledFlag         = (uint8)dependent_slice_seg_enabled_flag;
  CurrPicSet->OutputFlagPresentFlag                = (uint8)output_flag_present_flag;
  CurrPicSet->NumExtraSliceHeaderBits              = (uint8)num_extra_slice_header_bits;
  CurrPicSet->SignDataHidingFlag                   = (uint8)sign_data_hiding_enabled_flag;
  CurrPicSet->CabacInitFlag                        = (uint8)cabac_init_present_flag;
  CurrPicSet->num_ref_idx_l0_default_active_minus1 = (uint8)num_ref_idx_l0_default_active_minus1;
  CurrPicSet->num_ref_idx_l1_default_active_minus1 = (uint8)num_ref_idx_l1_default_active_minus1;
  CurrPicSet->PicInitQP                            = (uint8)PicInitQP;
  CurrPicSet->ConstrainedIntraPredFlag             = (uint8)ConstrainedIntraPredFlag;
  CurrPicSet->TransformSkipFlag                    = (uint8)transform_skip_enabled_flag;
  CurrPicSet->CuQpDeltaEnabledFlag                 = (uint8)cu_qp_delta_enabled_flag;
  CurrPicSet->DiffCuQpDeltaDepth                   = (uint8)cu_qp_delta_depth;
  CurrPicSet->CbQpOffset                           = (sint8)QpOffsetCb;
  CurrPicSet->CrQpOffset                           = (sint8)QpOffsetCr;
  CurrPicSet->SliceChromaQpOffsetPresentFlag       = (uint8)slice_chroma_qp_offsets_present;
  CurrPicSet->WeightedPredFlag                     = (uint8)weighted_pred_flag;
  CurrPicSet->WeightedBiPredFlag                   = (uint8)weighted_bipred_flag;
  CurrPicSet->transquant_bypass_enabled_flag       = (uint8)transquant_bypass_enable_flag;
  CurrPicSet->TilesEnabledFlag                     = (uint8)TilesEnabledFlag;
  CurrPicSet->EntropyCodingSyncEnabledFlag         = (uint8)EntropyCodingSyncEnabledFlag;
  CurrPicSet->num_tile_columns_minus1              = num_tile_columns_minus1;
  CurrPicSet->num_tile_rows_minus1                 = num_tile_rows_minus1;
  CurrPicSet->uniform_spacing_flag                 = (uint8)uniform_spacing_flag;
  CurrPicSet->loop_filter_across_tiles_enabled_flag = (uint8)loop_filter_across_tiles_enabled_flag;
  if(TilesEnabledFlag == 1)
  {
    for(i = 0; i < num_tile_columns_minus1+1; i++)
    {
      CurrPicSet->column_width[i] = column_width_minus1[i]+1;
    }

    for(i = 0; i < num_tile_rows_minus1+1; i++)
    {
      CurrPicSet->row_height[i] = row_height_minus1[i]+1;
    }
  }
  CurrPicSet->loop_filter_across_slices_enabled_flag   = (uint8)loop_filter_across_slices_enabled_flag;
  CurrPicSet->deblocking_filter_control_present_flag   = (uint8)deblocking_filter_control_present_flag;
  CurrPicSet->deblocking_filter_override_enabled_flag  = (uint8)deblocking_filter_override_enabled_flag;
  CurrPicSet->pic_disable_deblocking_filter_flag       = (uint8)pic_disable_deblocking_filter_flag;
  CurrPicSet->beta_offset = (sint8)beta_offset;
  CurrPicSet->tc_offset   = (sint8)tc_offset;
  CurrPicSet->ScalingListPresentFlag               = (uint8)scaling_list_data_present_flag;
  CurrPicSet->ScalListData                         = LocalScalListData;
  CurrPicSet->lists_modification_present_flag      = (uint8)lists_modification_present_flag;
  CurrPicSet->log2_parallel_merge_level            = (uint8)log2_parallel_merge_level;
  CurrPicSet->slice_seg_header_extention_present_flag = (uint8)slice_seg_header_extesion_present;
}
