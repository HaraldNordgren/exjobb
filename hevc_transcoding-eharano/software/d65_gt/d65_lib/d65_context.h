#ifndef D65_Context_H
#define D65_Context_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for context models
******************************************************************************/
#include "d65_api.h"
#include "d65_internal.h"

#define NUM_SPLIT_CODING_UNIT_FLAG_CTX 3
#define NUM_PREV_INTRA_LUMA_PRED_FLAG_CTX 1
#define NUM_INTRA_CHROMA_PRED_MODE_CTX 2
#define NUM_CBF_LUMA_CTX 2
#define NUM_CBF_CHROMA_CTX 3
#define NUM_LAST_SIGNIFICANT_COEFF_XY_LUMA_CTX 15
#define NUM_LAST_SIGNIFICANT_COEFF_XY_CHROMA_CTX 3
#define NUM_SIGNIFICANT_COEFF_FLAG_LUMA_CTX 27
#define NUM_SIGNIFICANT_COEFF_FLAG_CHROMA_CTX 15
#define NUM_SIGNIFICANT_COEFF_GROUP_FLAG_LUMA_CTX 2
#define NUM_SIGNIFICANT_COEFF_GROUP_FLAG_CHROMA_CTX 2
#define NUM_COEFF_ABS_LEVEL_GREATER1_LUMA_CTX 16
#define NUM_COEFF_ABS_LEVEL_GREATER1_CHROMA_CTX 8
#define NUM_COEFF_ABS_LEVEL_GREATER2_LUMA_CTX 4
#define NUM_COEFF_ABS_LEVEL_GREATER2_CHROMA_CTX 2
#define NUM_PRED_MODE_FLAG_CTX 1
#define NUM_SKIP_FLAG_CTX 3
#define NUM_MERGE_FLAG_CTX 1
#define NUM_MVD_CTX 2
#define NUM_PART_MODE_CTX 4
#define NUM_MVP_FLAG_CTX 1
#define NUM_NO_RESIDUAL_DATA_FLAG_CTX 1
#define NUM_MERGE_IDX_CTX 1
#define NUM_SPLIT_TRANSFORM_FLAG_CTX 3
#define NUM_REF_IDX_CTX 2
#define NUM_SAO_MERGE_CTX 1
#define NUM_SAO_TYPE_IDX_CTX 1
#define NUM_TRANSFORMSKIP_FLAG_CTX 2
#define NUM_CU_TRANSQUANT_BYPASS_FLAG_CTX 1
#define NUM_INTER_PRED_IDC_CTX 5
#define NUM_CU_QP_DELTA_ABS_CTX 2

/******************************************************************************
*
* Name:        Context model initialization
*
* Parameters:  -
*
* Returns:     D65_ReturnCode_t.OK         

* Description: -
*
******************************************************************************/
void ContextInitialize( Context* table, sint32 size, uint8* initValue, uint32 SliceType, sint32 qp);

/******************************************************************************
*
* Name:        set MPS and State
*
* Parameters:  -
*
* Returns:     -         

* Description: -
*
******************************************************************************/
void ContextSetState(Context* Cabac_p, uint8 ucState, uint8 ucMPS);

/******************************************************************************
*
* Name:        Context Buffer Assignment
*
* Parameters:  -
*
* Returns:     -         

* Description: -
*
******************************************************************************/
void ContextsBufferAssign(ContextModels *ContextModels_p);

/******************************************************************************
*
* Name:        ContextsInitialize
*
* Parameters:  -
*
* Returns:     -         

* Description: -
*
******************************************************************************/
void ContextsInitialize(ContextModels* ContextModels_p, uint32 SliceType, const sint32 QP);
#endif