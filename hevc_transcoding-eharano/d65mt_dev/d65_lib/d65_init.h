
#ifndef D65_INIT_H
#define D65_INIT_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains init decoding functions
*
******************************************************************************/


/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        CopyPredInfo
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Returns:
*
* Description: Resets the non-constant variables of the prediction unit array.
*
******************************************************************************/
void
CopyPredInfo(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        ResetPUData
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Returns:
*
* Description: Resets the non-constant variables of the prediction unit array.
*
******************************************************************************/
void
ResetPUData(SessionData_t *Sessiondata_p);


/******************************************************************************
*
* Name:        InitPredictionUnitArray(SessionData_t *Sessiondata_p)
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Description: Sets up the prediction unit array with LEFT, UP etc.
*
******************************************************************************/
void
InitPredictionUnitArray(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        AllocateSequenceMemory
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Returns:     uint8          [Out] 0-Allocation succeeded  1-Allocation failed
*
* Description: Allocates memory for the decoder
*
******************************************************************************/
uint8
AllocateSequenceMemory(SessionData_t *Sessiondata_p);

/******************************************************************************
*
* Name:        FreeSequenceMemory
*
* Parameters:  Sessiondata_p           [in]  The decoder
*
* Returns:     -
*
* Description: Free memory for the decoder
*
******************************************************************************/
void
FreeSequenceMemory(SessionData_t *Sessiondata_p);


/******************************************************************************
*
* Name:        ResetBitsReader
*
* Parameters:  BitsReader_t           [in]  The bits reader
*
* Returns:     -
*
* Description: Reset bits reader
*
******************************************************************************/
void
ResetBitsReader(BitsReader_t *BitsReader_p);


/******************************************************************************
*
* Name:        InitBitsReader
*
* Parameters:  BitsReader_t           [in]  The bits reader
*              BufferStart_p          [in]  Bits buffer starting position
*              BufferLength           [in]  Bits buffer length
* Returns:     -
*
* Description: InitBitsReader
*
******************************************************************************/
void
InitBitsReader(BitsReader_t *BitsReader_p,
               const uint8 *BufferStart_p,
               uint32 BufferLength
               );

#if MULTI_THREADED


/******************************************************************************
*
* Name:        AllocateThreadsPoolMemory
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Returns:     uint8          [Out] 0-Allocation succeeded  1-Allocation failed
*
* Description: Allocates memory for the decoder
*
******************************************************************************/
uint8
AllocateThreadsPoolMemory(SessionData_t *Sessiondata_p);


/******************************************************************************
*
* Name:        AllocateTilesThreadMemory
*
* Parameters:  Sessiondata_p  [in]  The decoder
*
* Returns:     uint8          [Out] 0-Allocation succeeded  1-Allocation failed
*
* Description: Allocates memory for the decoder
*
******************************************************************************/
uint8
AllocateTilesThreadMemory(SessionData_t *Sessiondata_p);

void
FreeThreadPoolMemory(SessionData_t *Sessiondata_p);

void
FreeTilesThreadMemory(SessionData_t *Sessiondata_p);

#endif

#if ENABLE_OPENHEVC
uint8
AllocateOpenHevcReturnPic(SessionData_t *Sessiondata_p,
                          const uint32 Y_height,
                          const uint32 Y_width_mb);

void
FreeOpenHevcReturnPic(SessionData_t *Sessiondata_p);
#endif

#endif  /* D65_INIT_H */

