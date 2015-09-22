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
#ifndef D65_DECODE_PU_H
#define D65_DECODE_PU_H

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
* Name:        Parse_prediction_unit
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      PU x position in pixel coordinates
*              y0              [in]      PU y position in pixel coordinates
*              nPbW            [in]      PU Width of the PU in pixels
*              nPbH            [in]      PU Height of the PU in pixels
*              PartIdx         [in]      PU part index in the CU
*              ctDepth         [in]      CU depth
*              PartMode        [in]      PU partition mode
* Returns:     -
*
* Description: Decodes a (inter) prediction unit (PU)
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
                      const PartitionMode_t PartMode);

/******************************************************************************
*
* Name:        Parse_intra_prediction_unit
*
* Parameters:  Sessiondata_p    [in/out]  The decoder
*              log2CbSize       [in]      log2 CU size
*              PartMode         [in]      Intra partition mode
*              TransQuantBypass [in]      Transform & quantization bypass flag
*              NumberPUinThisCU [in]      Number of PUs in the CU
*              StartX           [in]      Top-right PU coordinate X
*              StartY           [in]      Top-right PU coordinate Y
*
* Returns:     -
*
* Description: Parse intra prediction related syntax
*
******************************************************************************/
void
Parse_intra_prediction_unit(
#if MULTI_THREADED
                                 ThreadData_t *Threaddata_p,
#else
                                 SessionData_t *Sessiondata_p,
#endif
                                 const uint32 log2CbSize,
                                 const uint32 PartMode,
                                 const uint32 TransQuantBypass,
                                 const uint32 NumberPUinThisCU,
                                 const uint32 StartX,
                                 const uint32 StartY);
#endif