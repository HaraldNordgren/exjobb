/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains slice decoding functions
*
******************************************************************************/

#ifndef D65_DECODE_SLICE_H
#define D65_DECODE_SLICE_H

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
* Name:        DecodeSliceHeader
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*              RapPicFlag    [in]      RAP flag
*              TemporalID    [in]      The temporal_id of the slice
*              CTUnr         [out]     Starting CTU number
*              Quant         [out]     Starting QP value
*
* Returns:     -
*
* Description: Decodes the slice header. Throws an exception in the case of
*              a decoding error.
*
******************************************************************************/
void
DecodeSliceHeader(SessionData_t *Sessiondata_p,
                  const NAL_TYPE NALType,
                  const sint32 TemporalID,
                  uint32 *CTUnr,
                  sint32 *Quant);


/******************************************************************************
*
* Name:        DecodeSliceData
*
* Parameters:  Sessiondata_p   [in/out]  The decoder
*              SliceStartCTUnr [in]      Starting CTU number
*              Quant           [in]      Starting quantizer
*
* Returns:     -
*
* Description: Decodes the slice. Throws an exception in the case of a
*              decoding error.
*
******************************************************************************/
D65_ReturnCode_t
DecodeSliceData(SessionData_t *Sessiondata_p,
                const NAL_TYPE NALType,
                const uint32 SliceStartCTUnr,
                sint32 Quant);

#endif