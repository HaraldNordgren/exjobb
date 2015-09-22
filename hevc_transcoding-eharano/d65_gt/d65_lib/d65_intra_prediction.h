#ifndef D65_INTRA_PREDICTION_H
#define D65_INTRA_PREDICTION_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for intra prediction
*
******************************************************************************/

#include "string.h" /* memset */
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_init.h"

/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        IntraPredictionLuma
*
* Parameters:  Sessiondata_p  [in]  The decoder
*              x0         [in]      Current TU X coordinate in pixels;
*              y0         [in]      Current TU Y coordinate in pixels
*              Log2TrafoSize  [in]  Log2 current TU size in pixels
*
* Returns:     void
*
* Description: Intra Prediction (Luma) for current TU
*
******************************************************************************/
void
IntraPredictionLuma(SessionData_t *Sessiondata_p, const uint32 x0, const uint32 y0, const uint32 Log2TrafoSize);

/******************************************************************************
*
* Name:        IntraPredictionChroma
*
* Parameters:  Sessiondata_p  [in]  The decoder
*              x0         [in]      Current TU X coordinate in pixels;
*              y0         [in]      Current TU Y coordinate in pixels
*              Log2TrafoSize  [in]  Log2 current TU size in pixels
*
* Returns:     void
*
* Description: Intra Prediction (Luma) for current TU
*
******************************************************************************/
void
IntraPredictionChroma(SessionData_t *Sessiondata_p, const uint32 x0, const uint32 y0, const uint32 Log2TrafoSize);
#endif