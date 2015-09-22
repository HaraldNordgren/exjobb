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

#ifndef D65_DECODE_SEI_H
#define D65_DECODE_SEI_H

/******************************************************************************
* Includes
******************************************************************************/
#include "string.h"
#include "d65_api.h"
#include "d65_internal.h"

#if MULTI_THREADED
#include "pthread.h"
#endif

/******************************************************************************
* Declaration of functions
******************************************************************************/

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
DecodeSEI(SessionData_t *Sessiondata_p, sint32 nalUnitType);

#endif //D65_DECODE_SEI_H
