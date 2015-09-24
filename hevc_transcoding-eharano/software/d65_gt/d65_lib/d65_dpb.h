/******************************************************************************
* © Copyright (C) Ericsson AB 2014. All rights reserved.
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains DPB releated functions: RPS parsing, reference picture
* marking process and output reconstruced picture process
*
******************************************************************************/
#ifndef D65_DPB_H
#define D65_DPB_H

/******************************************************************************
* Includes
******************************************************************************/
#include "d65_internal.h"

/******************************************************************************
* Declaration of functions
******************************************************************************/
/******************************************************************************
*
* Name:        ParseReferencePictureSet
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     -
*
* Description: Parses the reference picture set and handles reference pictures
*
******************************************************************************/
void
ParseReferencePictureSet(SessionData_t *Sessiondata_p,
                         ReferencePictureSet_t *Rps_p,
                         ReferencePictureSet_t* RpsPredArray_p,
                         uint32 NumStRefPicSets,
                         uint32 idxRps);

/******************************************************************************
*
* Name:        PictureMarkingProcess
*
* Parameters:  Sessiondata_p           [in]  The decoder
*              Rps_p                   [in]  Reference Picture Set
*
* Returns:     -
*
* Description: This function performs reference picture marking according to
*              the RPS. It also sorts the pictures in DPB in following order: 
*              ShortTermUsedByCurr->LongTermUsedByCurr->
*              ShortTermNotUsedByCurr->LongTermNotUsedByCurr.
*              After sorting, the pictures left are marked as UNUSED_FOR_REFERENCE
*
******************************************************************************/
void
PictureMarkingProcess(SessionData_t *Sessiondata_p,
                      const ReferencePictureSet_t* Rps_p);

/******************************************************************************
*
* Name:        FakePictureMarkingProcess
*
* Parameters:  Sessiondata_p           [in]  The decoder
*
* Returns:     -
*
* Description: Performs picture marking when an current picture is an IDR 
*              picture, pictures in DPB should be no longer used for 
*              reference but they are not outputted yet! so we let the pictures
*              have a poc less than 0 and keep them in order.
*
******************************************************************************/
void
FakePictureMarkingProcess(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        GetFreePictureSlot
*
* Parameters:  Sessiondata_p     [in]  The decoder
*
* Returns:     uint8  The slot number of the free image in DPB
*
* Description: Finds a free image slot, the one chosen is the slot containing
*              the picture marked as "not needed" and "unused".
*              Returns the index in DecodedPictureBuffer that points to the 
*              free slot, -1 is returned if no slot was found. 
*              NOTE: The current reconstructed picture is directly stored in DPB, 
*              there should always be a free slot !!!
*
******************************************************************************/
uint8
GetFreePictureSlot(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        PictureDecoded
*
* Parameters:  Sessiondata_p           [in]  The decoder
*              NALType                 [in]  The NAL type
*
* Returns:     -
*
* Description: Called when a pictured is decided to have been fully decoded.
*              Performs deblocking, picture output process and sets up 
*              the decoder for the next picture. 
*
******************************************************************************/
void
PictureDecoded(SessionData_t *Sessiondata_p, 
               const uint32 NALType);


/******************************************************************************
*
* Name:        FlushDPB
*
* Parameters:  Sessiondata_p           [in]  The decoder
*
* Returns:     -
*
* Description: Flushes DecodedPictureBuffer by outputting using callback function
*              Write_Output_Function_p in Sessiondata
*
******************************************************************************/
void
FlushDPB(SessionData_t *Sessiondata_p);

#endif