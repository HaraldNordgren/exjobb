/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for context models
*
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
#include "d65_context.h"


static const uint8 INIT_SPLIT_CODING_UNIT_FLAG[3][NUM_SPLIT_CODING_UNIT_FLAG_CTX] =  
{ 
  { 107,  139,  126 }, 
  { 107,  139,  126 },
  { 139,  141,  157 },
};

static const uint8 INIT_PREV_INTRA_LUMA_PRED_FLAG[3][NUM_PREV_INTRA_LUMA_PRED_FLAG_CTX] = 
{
  { 183, },
  { 154, }, 
  { 184, }, 
   
};

static const uint8 INIT_INTRA_CHROMA_PRED_MODE[3][NUM_INTRA_CHROMA_PRED_MODE_CTX] = 
{
  { 152,  139, },
  { 152,  139, },
  {  63,  139, },  
   
};

static const uint8 INIT_CBF_LUMA[3][NUM_CBF_LUMA_CTX] =  
{
  { 153,  111, }, 
  { 153,  111, },
  { 111,  141, },
   
};

static const uint8 INIT_CBF_CHROMA[3][NUM_CBF_CHROMA_CTX] =  
{
  { 149,   92,  167, },
  { 149,  107,  167, },
  {  94,  138,  182, }, 
 
};

static const uint8 INIT_LAST_SIGNIFICANT_COEFF_XY_LUMA[3][NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX] =  
{
  { 125,  110,  124,  110,   95,   94,  125,  111,  111,   79,  125,  126,  111,  111,   79, 
  }, 
  { 125,  110,   94,  110,   95,   79,  125,  111,  110,   78,  110,  111,  111,   95,   94,  
  }, 
  { 110,  110,  124,  125,  140,  153,  125,  127,  140,  109,  111,  143,  127,  111,   79,
  }, 
};
static const uint8 INIT_LAST_SIGNIFICANT_COEFF_XY_CHROMA[3][NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX] =  
{
  { 108,  123,   93, }, 
  { 108,  123,  108, }, 
  { 108,  123,   63, }, 
};

static const uint8 INIT_SIGNIFICANT_COEFF_FLAG_LUMA[3][NUM_SIGNIFICANT_COEFF_FLAG_LUMA_CTX] = 
{
  { 170,  154,  139,  153,  139,  123,  123,   63,  124,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154, }, 
  { 155,  154,  139,  153,  139,  123,  123,   63,  153,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154,  166,  183,  140,  136,  153,  154, }, 
  { 111,  111,  125,  110,  110,   94,  124,  108,  124,  107,  125,  141,  179,  153,  125,  107,  125,  141,  179,  153,  125,  107,  125,  141,  179,  153,  125, }, 
};

static const uint8 INIT_SIGNIFICANT_COEFF_FLAG_CHROMA[3][NUM_SIGNIFICANT_COEFF_FLAG_CHROMA_CTX] = 
{
  { 170,  153,  138,  138,  122,  121,  122,  121,  167,  151,  183,  140,  151,  183,  140, }, 
  { 170,  153,  123,  123,  107,  121,  107,  121,  167,  151,  183,  140,  151,  183,  140, }, 
  { 140,  139,  182,  182,  152,  136,  152,  136,  153,  136,  139,  111,  136,  139,  111, }, 
};


static const uint8 INIT_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA[3][NUM_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA_CTX] =  
{
  { 121,  140, },
  { 121,  140, },
  {  91,  171, },  
};

static const uint8 INIT_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA[3][NUM_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA_CTX] =  
{
  {  61,  154, },
  {  61,  154, },
  { 134,  141, },
};

static const uint8 INIT_COEFF_ABS_LEVEL_GREATER1_LUMA[3][NUM_COEFF_ABS_LEVEL_GREATER1_LUMA_CTX] = 
{
  { 154,  196,  167,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  122, },
  { 154,  196,  196,  167,  154,  152,  167,  182,  182,  134,  149,  136,  153,  121,  136,  137, },
  { 140,   92,  137,  138,  140,  152,  138,  139,  153,   74,  149,   92,  139,  107,  122,  152, }, 
};
static const uint8 INIT_COEFF_ABS_LEVEL_GREATER1_CHROMA[3][NUM_COEFF_ABS_LEVEL_GREATER1_CHROMA_CTX] = 
{
  { 169,  208,  166,  167,  154,  152,  167,  182, },
  { 169,  194,  166,  167,  154,  167,  137,  182, },
  { 140,  179,  166,  182,  140,  227,  122,  197, },  
   
};
static const uint8 INIT_COEFF_ABS_LEVEL_GREATER2_LUMA[3][NUM_COEFF_ABS_LEVEL_GREATER2_LUMA_CTX] = 
{
  { 107, 167,  91, 107, }, 
  { 107, 167,  91, 122, },
  { 138, 153, 136, 167, },
   
};
static const uint8 INIT_COEFF_ABS_LEVEL_GREATER2_CHROMA[3][NUM_COEFF_ABS_LEVEL_GREATER2_CHROMA_CTX] = 
{
  { 107, 167},
  { 107, 167},
  { 152, 152},  
   
};
static const uint8 INIT_PRED_MODE_FLAG[3][NUM_PRED_MODE_FLAG_CTX] = 
{
  { 134, }, 
  { 149, },
  {   0, },
   
};
static const uint8 INIT_SKIP_FLAG[3][NUM_SKIP_FLAG_CTX] =  
{
  { 197,  185,  201, },
  { 197,  185,  201, },
  {   0,    0,    0, },
  
};

static const uint8 INIT_MERGE_FLAG[3][NUM_MERGE_FLAG_CTX] = 
{
  { 154, },
  { 110, },
  {   0, },
  
};

static const uint8 INIT_REF_IDX[3][NUM_REF_IDX_CTX] = 
{
  { 153,  153, },
  { 153,  153, },
  {   0,    0, }   
};

static const uint8 INIT_MVD[3][NUM_MVD_CTX] =  
{
  { 169,  198, },
  { 140,  198, },
  {   0,    0, } 
};

static const uint8 INIT_PART_MODE[3][NUM_PART_MODE_CTX] =  
{
  { 154,  139,  154,  154},
  { 154,  139,  154,  154},
  { 184,    0,    0,    0}, 
   
   
};

static const uint8 INIT_MVP_FLAG[3][NUM_MVP_FLAG_CTX] =  
{
  { 168, },
  { 168, },
  {   0, }, 
};

static const uint8 INIT_NO_RESIDUAL_DATA_FLAG[3][NUM_NO_RESIDUAL_DATA_FLAG_CTX] = 
{
  {  79, },
  {  79, },
  {   0, },  
};

static const uint8 INIT_MERGE_IDX[3][NUM_MERGE_IDX_CTX] =  
{
  { 137, },
  { 122, },
  {   0, }, 
};

static const uint8 INIT_SPLIT_TRANSFORM_FLAG[3][NUM_SPLIT_TRANSFORM_FLAG_CTX] = 
{
  { 224,  167,  122, },
  { 124,  138,   94, },
  { 153,  138,  138, },
};

static const uint8 INIT_SAO_MERGE_FLAG[3][NUM_SAO_MERGE_CTX] =
{
  { 153, },
  { 153, },
  { 153, },
};

static const uint8 INIT_SAO_TYPE_IDX[3][NUM_SAO_TYPE_IDX_CTX] = 
{
  { 160, },
  { 185, },
  { 200, },
};

static const uint8 INIT_TRANSFORMSKIP_FLAG[3][NUM_TRANSFORMSKIP_FLAG_CTX] =
{
  { 139, 139 },
  { 139, 139 },
  { 139, 139 }
};

static const uint8 INIT_CU_TRANSQUANT_BYPASS_FLAG[3][NUM_CU_TRANSQUANT_BYPASS_FLAG_CTX] =
{
  { 154, },
  { 154, },
  { 154, },
};

static const uint8 INIT_INTER_PRED_IDC[3][NUM_INTER_PRED_IDC_CTX] =
{
  { 95, 79, 63, 31, 31 }, 
  { 95, 79, 63, 31, 31 },
  {  0,  0,  0,  0,  0 }
};

static const uint8 INIT_CU_QP_DELTA_ABS[3][NUM_CU_QP_DELTA_ABS_CTX] = 
{
  { 154, 154 },
  { 154, 154 },
  { 154, 154 }
};

void ContextInitialize( Context* Table, sint32 Size, uint8* InitValue, uint32 SliceType, sint32 QP)
{
  sint32 Slope;
  sint32 Offset;
  sint32 InitState;
  sint32 MPState;
  uint8 i;

  InitValue += SliceType * Size;
  for(i = 0; i < Size; i++)
  {
    Slope  = ((*InitValue) >> 4) * 5 - 45;
    Offset = (((*InitValue) & 15) << 3) - 16;
    InitState = CLIP3(1, 126, ((Slope * QP) >> 4) + Offset);
    MPState   = (InitState >= 64);
    Table->m_usState = ( (MPState? (InitState - 64):(63 - InitState)) <<1) + MPState; 
    Table++;
    InitValue++;
  }
}

void ContextSetState( Context* Context_p, uint8 ucState, uint8 ucMPS)
{
  Context_p->m_usState = ( ucState << 1) + ucMPS;
}

void ContextsBufferAssign(ContextModels *ContextModels_p)
{
  Context *CtxBuffer = ContextModels_p->ContextBuffer;

  ContextModels_p->SplitCuFlagCtx = CtxBuffer;
  CtxBuffer += NUM_SPLIT_CODING_UNIT_FLAG_CTX;

  ContextModels_p->PrevIntraLumaPredFlagCtx = CtxBuffer;
  CtxBuffer += NUM_PREV_INTRA_LUMA_PRED_FLAG_CTX;

  ContextModels_p->IntraChromaPredModeCtx = CtxBuffer;
  CtxBuffer += NUM_INTRA_CHROMA_PRED_MODE_CTX;

  ContextModels_p->CbfLumaCtx = CtxBuffer;
  CtxBuffer += NUM_CBF_LUMA_CTX;

  ContextModels_p->CbfChromaCtx = CtxBuffer;
  CtxBuffer += NUM_CBF_CHROMA_CTX;

  ContextModels_p->LastSigCoeffXLumaCtx = CtxBuffer;
  CtxBuffer += NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX;

  ContextModels_p->LastSigCoeffYLumaCtx = CtxBuffer;
  CtxBuffer += NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX;

  ContextModels_p->LastSigCoeffXChromaCtx = CtxBuffer;
  CtxBuffer += NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX;

  ContextModels_p->LastSigCoeffYChromaCtx = CtxBuffer;
  CtxBuffer += NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX;

  ContextModels_p->SigCoeffFlagLumaCtx = CtxBuffer;
  CtxBuffer += NUM_SIGNIFICANT_COEFF_FLAG_LUMA_CTX;

  ContextModels_p->SigCoeffFlagChromaCtx = CtxBuffer;
  CtxBuffer += NUM_SIGNIFICANT_COEFF_FLAG_CHROMA_CTX;

  ContextModels_p->SigCoeffGroupFlagLumaCtx = CtxBuffer;
  CtxBuffer += NUM_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA_CTX;

  ContextModels_p->SigCoeffGroupFlagChromaCtx = CtxBuffer;
  CtxBuffer += NUM_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA_CTX;

  ContextModels_p->CoeffAbsLevelG1LumaCtx = CtxBuffer;
  CtxBuffer += NUM_COEFF_ABS_LEVEL_GREATER1_LUMA_CTX;

  ContextModels_p->CoeffAbsLevelG1ChromaCtx = CtxBuffer;
  CtxBuffer += NUM_COEFF_ABS_LEVEL_GREATER1_CHROMA_CTX;

  ContextModels_p->CoeffAbsLevelG2LumaCtx = CtxBuffer;
  CtxBuffer += NUM_COEFF_ABS_LEVEL_GREATER2_LUMA_CTX;

  ContextModels_p->CoeffAbsLevelG2ChromaCtx = CtxBuffer;
  CtxBuffer += NUM_COEFF_ABS_LEVEL_GREATER2_CHROMA_CTX;

  ContextModels_p->PredModeFlagCtx = CtxBuffer;
  CtxBuffer += NUM_PRED_MODE_FLAG_CTX;

  ContextModels_p->SkipFlagCtx = CtxBuffer;
  CtxBuffer += NUM_SKIP_FLAG_CTX;

  ContextModels_p->MergeFlagCtx = CtxBuffer;
  CtxBuffer += NUM_MERGE_FLAG_CTX;

  ContextModels_p->RefIdxCtx = CtxBuffer;
  CtxBuffer += NUM_REF_IDX_CTX;

  ContextModels_p->MvdCtx = CtxBuffer;
  CtxBuffer += NUM_MVD_CTX;

  ContextModels_p->PartModeCtx = CtxBuffer;
  CtxBuffer += NUM_PART_MODE_CTX;

  ContextModels_p->MvpFlagCtx = CtxBuffer;
  CtxBuffer += NUM_MVP_FLAG_CTX;

  ContextModels_p->NoResiDataFlagCtx = CtxBuffer;
  CtxBuffer += NUM_NO_RESIDUAL_DATA_FLAG_CTX;

  ContextModels_p->MergeIdxCtx = CtxBuffer;
  CtxBuffer += NUM_MERGE_IDX_CTX;

  ContextModels_p->SplitTransformFlagCtx = CtxBuffer;
  CtxBuffer += NUM_SPLIT_TRANSFORM_FLAG_CTX;

  ContextModels_p->SaoMergeCtx = CtxBuffer;
  CtxBuffer += NUM_SAO_MERGE_CTX;

  ContextModels_p->SaoTypeIdxCtx = CtxBuffer;
  CtxBuffer += NUM_SAO_TYPE_IDX_CTX;

  ContextModels_p->TrafoSkipFlagCtx = CtxBuffer;
  CtxBuffer += NUM_TRANSFORMSKIP_FLAG_CTX;

  ContextModels_p->CuTrQBypassFlagCtx = CtxBuffer;
  CtxBuffer += NUM_CU_TRANSQUANT_BYPASS_FLAG_CTX;

  ContextModels_p->InterPredIdcCtx = CtxBuffer;
  CtxBuffer += NUM_INTER_PRED_IDC_CTX;

  ContextModels_p->CuQpDeltaAbsCtx = CtxBuffer;
  CtxBuffer += NUM_CU_QP_DELTA_ABS_CTX;
}

void ContextsInitialize(ContextModels* ContextModels_p, uint32 SliceType, const sint32 QP)
{
  ContextInitialize(ContextModels_p->SplitCuFlagCtx,NUM_SPLIT_CODING_UNIT_FLAG_CTX,(uint8*)INIT_SPLIT_CODING_UNIT_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->PrevIntraLumaPredFlagCtx,NUM_PREV_INTRA_LUMA_PRED_FLAG_CTX,(uint8*)INIT_PREV_INTRA_LUMA_PRED_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->IntraChromaPredModeCtx,NUM_INTRA_CHROMA_PRED_MODE_CTX,(uint8*)INIT_INTRA_CHROMA_PRED_MODE,SliceType,QP);
  ContextInitialize(ContextModels_p->CbfLumaCtx,NUM_CBF_LUMA_CTX,(uint8*)INIT_CBF_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->CbfChromaCtx,NUM_CBF_CHROMA_CTX,(uint8*)INIT_CBF_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->LastSigCoeffXLumaCtx,NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX,(uint8*)INIT_LAST_SIGNIFICANT_COEFF_XY_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->LastSigCoeffYLumaCtx,NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX,(uint8*)INIT_LAST_SIGNIFICANT_COEFF_XY_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->LastSigCoeffXChromaCtx,NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX,(uint8*)INIT_LAST_SIGNIFICANT_COEFF_XY_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->LastSigCoeffYChromaCtx,NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX,(uint8*)INIT_LAST_SIGNIFICANT_COEFF_XY_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->SigCoeffFlagLumaCtx,NUM_SIGNIFICANT_COEFF_FLAG_LUMA_CTX,(uint8*)INIT_SIGNIFICANT_COEFF_FLAG_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->SigCoeffFlagChromaCtx,NUM_SIGNIFICANT_COEFF_FLAG_CHROMA_CTX,(uint8*)INIT_SIGNIFICANT_COEFF_FLAG_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->SigCoeffGroupFlagLumaCtx,NUM_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA_CTX,(uint8*)INIT_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->SigCoeffGroupFlagChromaCtx,NUM_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA_CTX,(uint8*)INIT_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->CoeffAbsLevelG1LumaCtx,NUM_COEFF_ABS_LEVEL_GREATER1_LUMA_CTX,(uint8*)INIT_COEFF_ABS_LEVEL_GREATER1_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->CoeffAbsLevelG1ChromaCtx,NUM_COEFF_ABS_LEVEL_GREATER1_CHROMA_CTX,(uint8*)INIT_COEFF_ABS_LEVEL_GREATER1_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->CoeffAbsLevelG2LumaCtx,NUM_COEFF_ABS_LEVEL_GREATER2_LUMA_CTX,(uint8*)INIT_COEFF_ABS_LEVEL_GREATER2_LUMA,SliceType,QP);
  ContextInitialize(ContextModels_p->CoeffAbsLevelG2ChromaCtx,NUM_COEFF_ABS_LEVEL_GREATER2_CHROMA_CTX,(uint8*)INIT_COEFF_ABS_LEVEL_GREATER2_CHROMA,SliceType,QP);
  ContextInitialize(ContextModels_p->PredModeFlagCtx,NUM_PRED_MODE_FLAG_CTX,(uint8*)INIT_PRED_MODE_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->SkipFlagCtx,NUM_SKIP_FLAG_CTX,(uint8*)INIT_SKIP_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->MergeFlagCtx,NUM_MERGE_FLAG_CTX,(uint8*)INIT_MERGE_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->RefIdxCtx, NUM_REF_IDX_CTX, (uint8*)INIT_REF_IDX, SliceType, QP);
  ContextInitialize(ContextModels_p->MvdCtx,NUM_MVD_CTX,(uint8*)INIT_MVD,SliceType,QP);
  ContextInitialize(ContextModels_p->PartModeCtx,NUM_PART_MODE_CTX,(uint8*)INIT_PART_MODE,SliceType,QP);
  ContextInitialize(ContextModels_p->MvpFlagCtx,NUM_MVP_FLAG_CTX,(uint8*)INIT_MVP_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->NoResiDataFlagCtx,NUM_NO_RESIDUAL_DATA_FLAG_CTX,(uint8*)INIT_NO_RESIDUAL_DATA_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->MergeIdxCtx,NUM_MERGE_IDX_CTX,(uint8*)INIT_MERGE_IDX,SliceType,QP);
  ContextInitialize(ContextModels_p->SplitTransformFlagCtx,NUM_SPLIT_TRANSFORM_FLAG_CTX,(uint8*)INIT_SPLIT_TRANSFORM_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->SaoMergeCtx,NUM_SAO_MERGE_CTX, (uint8*)INIT_SAO_MERGE_FLAG,SliceType,QP);
  ContextInitialize(ContextModels_p->SaoTypeIdxCtx,NUM_SAO_TYPE_IDX_CTX,(uint8*)INIT_SAO_TYPE_IDX,SliceType,QP);
  ContextInitialize(ContextModels_p->TrafoSkipFlagCtx, NUM_TRANSFORMSKIP_FLAG_CTX, (uint8*)INIT_TRANSFORMSKIP_FLAG, SliceType, QP);
  ContextInitialize(ContextModels_p->CuTrQBypassFlagCtx, NUM_CU_TRANSQUANT_BYPASS_FLAG_CTX, (uint8*)INIT_CU_TRANSQUANT_BYPASS_FLAG, SliceType, QP);
  ContextInitialize(ContextModels_p->InterPredIdcCtx, NUM_INTER_PRED_IDC_CTX, (uint8*)INIT_INTER_PRED_IDC, SliceType, QP);
  ContextInitialize(ContextModels_p->CuQpDeltaAbsCtx, NUM_CU_QP_DELTA_ABS_CTX, (uint8*)INIT_CU_QP_DELTA_ABS, SliceType, QP);
}