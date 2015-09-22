/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for outputting bits
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <string.h>
#include "d65_putbits.h"

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
void
PutOutByte(BitsOutputer_t *BitsOutputer_p, uint32 Byte)
{
  uint32          Pos            = BitsOutputer_p->BufferPos;

  if (Pos >= 2)
  {
    uint32 PrevByte1 = *(BitsOutputer_p->BufferStart_p + Pos - 1);
    uint32 PrevByte2 = *(BitsOutputer_p->BufferStart_p + Pos - 2);
    if (PrevByte1 == 0 && PrevByte2 == 0 && Byte <= 3)
    {
      *(BitsOutputer_p->BufferStart_p + Pos) = 3;
      BitsOutputer_p->BufferPos++;
    }
  }
 
  *(BitsOutputer_p->BufferStart_p + BitsOutputer_p->BufferPos) = (Byte & 0xFF);
  BitsOutputer_p->BufferPos++;
}


void
PutOutBits(SubEncoder_t *SubEncoder_p,
           uint32 NumBits,
           uint32 Value)
{
  BitsOutputer_t *BitsOutputer_p = &SubEncoder_p->BitsOutputer;
  while(NumBits > 0)
  {
    //while(BitsOutputer_p->CurrByte_Pos < 8)
    {
      uint32 Bit = Value >> (NumBits - 1);
      BitsOutputer_p->CurrByte += Bit << (7 - BitsOutputer_p->CurrByte_Pos);
      BitsOutputer_p->CurrByte_Pos++;
      Value -= (Bit << (NumBits - 1));
      NumBits--;
    }
    if (BitsOutputer_p->CurrByte_Pos == 8)
    {
      PutOutByte(BitsOutputer_p, BitsOutputer_p->CurrByte);
      BitsOutputer_p->CurrByte_Pos = 0;
      BitsOutputer_p->CurrByte     = 0;
    }
  }
}

void
PutOutRbspTrailingBits(SubEncoder_t *SubEncoder_p)
{
  PutOutBits(SubEncoder_p, 1, 1);
  if (SubEncoder_p->BitsOutputer.CurrByte_Pos > 0)
  {
    PutOutByte(&SubEncoder_p->BitsOutputer, SubEncoder_p->BitsOutputer.CurrByte);
    SubEncoder_p->BitsOutputer.CurrByte     = 0;
    SubEncoder_p->BitsOutputer.CurrByte_Pos = 0;
  }
}

void
PutStartCodes(uint8 *Buffer_p, uint32 *BufferPos)
{
  uint8  i;
  uint8 *Dst_p = &Buffer_p[*BufferPos];
  for (i = 0; i < 3; i++)
  {
    *Dst_p = 0x00;
    Dst_p++;
  }
  *Dst_p = 0x01;
  (*BufferPos) += 4;
}

//void
//PutStartCodes(BitsOutputer_t *BitsOutputer_p)
//{
//  uint8  i;
//  for (i = 0; i < 3; i++)
//  {
//    *(BitsOutputer_p->BufferStart_p + BitsOutputer_p->BufferPos) = 0x00;
//    BitsOutputer_p->BufferPos++;
//  }
//  *(BitsOutputer_p->BufferStart_p + BitsOutputer_p->BufferPos) = 0x01;
//  BitsOutputer_p->BufferPos++;
//}

void
PutSimpleCopy(uint8  *Buffer_p,
              uint8  *Src_p,
              uint32 *BufferPos,
              uint32  Length)
{
  PutStartCodes(Buffer_p, BufferPos);
  memcpy(Buffer_p + *BufferPos, Src_p, Length);
  (*BufferPos) += Length;
}

//void
//PutSimpleCopy(BitsOutputer_t *BitsOutputer_p,
//              uint8 *Src_p,
//              uint32 Length)
//{
//  PutStartCodes(BitsOutputer_p->BufferStart_p, &BitsOutputer_p->BufferPos);
//  memcpy(BitsOutputer_p->BufferStart_p + BitsOutputer_p->BufferPos,
//         Src_p,
//         Length);
//  BitsOutputer_p->BufferPos += Length;
//}
#endif
