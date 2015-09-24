
#ifndef D65_TRANSFORM_RECONSTRUCT_H
#define D65_TRANSFORM_RECONSTRUCT_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains inverse quantization, inverse transform, and add&clip functions
*
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
#include "d65_internal.h"

#if SIMD
#include <emmintrin.h>
#include <smmintrin.h>
#endif

/******************************************************************************
* Declaration of functions
******************************************************************************/

#if GT_RESIDUAL_RECONSTRUCT
/******************************************************************************
*
* Name:        CalculateResiduals
*
* Parameters:  
*
* Returns:     void
*
* Description: Calculate difference between source block and prediction block
*
******************************************************************************/
void
CalculateResiduals(uint8  *original,
                   uint8  *prediction,
                   sint16 *residuals,
                   uint32  oWidth,
                   uint32  pWidth,
                   uint32  blocksize);

/******************************************************************************
*
* Name:        PerformTransform
*
* Parameters:  
*
* Returns:     void
*
* Description: Perform forward transform
*
******************************************************************************/
void
PerformTransform(sint16 *source,
                 sint16 *target,
                 sint32 log2Size,
                 uint32 TrafoSkipFlag,
                 uint8 DST );

/******************************************************************************
*
* Name:        performQuantization
*
* Parameters:  
*
* Returns:     void
*
* Description: Perform quantization
*
******************************************************************************/
void performQuantization(sint16* source,
                         sint16* target,
                         sint32 log2Size,
                         sint32 qp,
                         uint32 sliceType);

#endif
/******************************************************************************
*
* Name:        performDequantization
*
* Parameters:  Array      [in/out]  The coefficients
*              log2Size   [in]      The log2 size of the block (e.g. 3->8x8 block)
*              qp         [in]      The QP
*
* Returns:     void
*
* Description: Performs inverse quantization. The Array is overwritten by
*              inverse quantization values
*
******************************************************************************/
void
performDequantization(sint16 *Array, const sint32 log2Size, const sint32 qp);

/******************************************************************************
*
* Name:        performInverseTransform
*
* Parameters:  TrafoCoeffs     [in]  The transform coefficients block
*              TrafoSize       [in]      Transform size in pixels
*              TrafoSkipFlag...[in]      Trasform skip enabled flag
*              DST...          [in]      DST enabled flag
*
* Returns:     void
*
* Description: Performs inverse transform.
*
******************************************************************************/
void
performInverseTransform(sint16 *TrafoCoeffs,
                        const uint32 TrafoSize,
                        uint32 TrafoSkipFlag,
                        uint8 DST);

/******************************************************************************
*
* Name:        performReconstruction
*
* Parameters:  TrafoCoeffs     [in]  The transform coefficients block
*              RecBlock_p      [in/out]  Pointer to the reconstructed block
*              TrafoSize       [in]      Transform size in pixels
*              RecPicWidth.....[in]      Reconstructed block width in pixels
*
* Returns:     void
*
* Description: Add inverse-transformed coefficients with predictions
*
******************************************************************************/
void
performReconstruction(sint16 *TrafoCoeffs,
                        uint8 *RecBlock_p,
                        const uint32 TrafoSize,
                        const uint32 RecPicWidth
                     );
void partialInverse(const sint16* source, const sint16* target, const uint32 TrafoSize);
#endif  /* D65_TRANSFORM_RECONSTRUCT_H */

