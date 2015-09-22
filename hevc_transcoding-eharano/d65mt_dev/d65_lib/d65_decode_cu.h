/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains SAO/CU tree/CU decoding functions
*
******************************************************************************/
#ifndef D65_DECODE_CU_H
#define D65_DECODE_CU_H

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
* Name:        Parse_sao_syntax
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      x position in CTU coordinates
*              y0              [in]      y position in CTU coordinates
*
* Returns:     -
*
* Description: Parse SAO syntax
*
******************************************************************************/
void Parse_sao_syntax(
#if MULTI_THREADED
                      ThreadData_t  *Threaddata_p,
#else
                      SessionData_t *Sessiondata_p,
#endif
                      uint32 rx,
                      uint32 ry);

/******************************************************************************
*
* Name:        Parse_coding_quadtree
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              x0              [in]      x position in pixel coordinates
*              y0              [in]      y position in pixel coordinates
*              log2CbSize      [in]      log size of the CU in pixels
*              ctDepth         [in]      coding tree depth realtive the CTU
*
* Returns:     -
*
* Description: Recursive function to decode the split flags.
*              Generates function calls to Parse_coding_unit for each CU leaf
*
******************************************************************************/
void
Parse_coding_quadtree(
#if MULTI_THREADED
                      ThreadData_t *Threaddata_p,
#else
                      SessionData_t *Sessiondata_p,
#endif
                      const uint32 x0,
                      const uint32 y0,
                      const uint32 log2CbSize,
                      const uint32 ctDepth);
#endif