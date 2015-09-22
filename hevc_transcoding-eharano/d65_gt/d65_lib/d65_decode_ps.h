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

#ifndef D65_DECODE_PS_H
#define D65_DECODE_PS_H




/******************************************************************************
* Includes
******************************************************************************/
#include "d65_internal.h"




/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        DecodeVideoParameterSet
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     -
*
* Description: Decodes a video parameter set
*
*
******************************************************************************/
void
DecodeVideoParameterSet(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        DecodeSequenceParameterSet
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     -
*
* Description: Decodes a sequence parameter set
*
*
******************************************************************************/
void
DecodeSequenceParameterSet(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        DecodePictureParameterSet
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     -
*
* Description: Decodes a picture parameter set
*
*
******************************************************************************/
void
DecodePictureParameterSet(SessionData_t *Sessiondata_p);

#endif