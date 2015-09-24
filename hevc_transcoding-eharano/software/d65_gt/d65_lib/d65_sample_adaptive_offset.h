#ifndef D65_SAMPLE_ADAPTIVE_OFFSET_H
#define D65_SAMPLE_ADAPTIVE_OFFSET_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for SAO
*
******************************************************************************/
#include "string.h"
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_init.h"

#if SIMD
#include <emmintrin.h>
#endif
/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        SaoProcess
*
* Parameters:  Sessiondata_p  [in/out]  The decoder
*
* Returns:     void
*
* Description: Perform SAO filtering for current picture
*
******************************************************************************/
void
SaoProcess(SessionData_t *Sessiondata_p);
#endif