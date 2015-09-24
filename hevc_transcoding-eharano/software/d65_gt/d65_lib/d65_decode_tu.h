/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains TU tree/TU/Residual decoding functions
*
******************************************************************************/
#ifndef D65_DECODE_TU_H
#define D65_DECODE_TU_H

/******************************************************************************
* Includes
******************************************************************************/
#include "d65_api.h"
#include "d65_internal.h"


/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        UpdateQp
*
* Parameters:  Sessiondata_p     [in]      The decoder
*              QgData_p          [in/out]  Quantization group data
*              xBase             [in]      Current CU X
*              yBase             [in]      Current CU Y
*
* Returns:     -
*
* Description: Update quantization parameter for current CU
*
******************************************************************************/
void
UpdateQp(SessionData_t    *Sessiondata_p,
         QuantGroupData_t *QgData_p,
         const uint32      xBase,
         const uint32      yBase);

/******************************************************************************
*
* Name:        Parse_transform_tree
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      x position of TU in pixel coordinates
*              y0              [in]      y position of TU in pixel coordinates
*              log2TrafoSize   [in]      log size of the TU in pixels
*              trafoDepth      [in]      coding tree depth realtive the CU
*              blkIdx          [in]      block index
*              MaxTrafoDepth   [in]      maximum transform tree depth
*              IntraSplitFlag  [in]      intra split flag (use PART_NxN)
*              InterSplitFlag  [in]      inter split flag (PU splitted)
*              CbfChroma       [in]      coded block flag for chroma
*              PredMode        [in]      prediction mode
*
* Returns:     -
*
* Description: Decodes a transform unit tree 
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
                     const uint32 PredMode);

#endif