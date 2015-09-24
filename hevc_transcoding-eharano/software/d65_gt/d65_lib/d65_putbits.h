#ifndef D65_PUTBITS_H
#define D65_PUTBITS_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for outputing bits
*
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
#include "d65_internal.h"


/******************************************************************************
* Declaration of functions
******************************************************************************/
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
void
PutOutByte(BitsOutputer_t *BitsOutputer_p, uint32 Byte);

void
PutOutBits(SubEncoder_t *SubEncoder_p,
           uint32 NumBits,
           uint32 Value);

void
PutOutRbspTrailingBits(SubEncoder_t *SubEncoder_p);

//void
//PutStartCodes(BitsOutputer_t *BitsOutputer_p);
void
PutStartCodes(uint8 *Buffer_p, uint32 *BufferPos);

void
PutSimpleCopy(uint8  *Buffer_p,
              uint8  *Src_p,
              uint32 *BufferPos,
              uint32  Length);

//void
//PutSimpleCopy(BitsOutputer_t *BitsOutputer_p,
//              uint8 *Src_p,
//              uint32 Length);
#endif
#endif
