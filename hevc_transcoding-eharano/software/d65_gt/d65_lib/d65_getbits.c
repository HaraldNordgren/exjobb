/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains bitreading and bittrace functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_getbits.h"

#if D65_BITTRACE
#include "string.h" // definition of strcmp
#endif

/******************************************************************************
* Declaration of functions
******************************************************************************/


/******************************************************************************
* Definition of local functions
******************************************************************************/


/******************************************************************************
*
* Name:        TraceBits
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*              Length        [in]      Length of codeword
*              BitPattern    [in]      The bitpattern of the codeword
*              TraceString   [in]      Text string describing the symbol
*
* Returns:     -
*
* Description: Puts out a trace on Sessiondata_p->Tracefile_p
*
******************************************************************************/

void
TraceBits(const uint32 Length, 
          const uint32 BitPattern,
          const char *TraceString)
{
#if D65_BITTRACE == 1 || D65_BITTRACE == 2
  if(TraceFlag)
  {
    FILE *TraceFile_p = Tracefile_p;

    if(TraceFile_p != NULL)
    {
      uint32 Chars;
      uint32 i;
      
      // Put out bitposition
      putc('@', TraceFile_p);
      Chars = fprintf(TraceFile_p, "%i", TraceBitCounter);
      TraceBitCounter += Length;
      while(Chars++ < 9)
      {
        putc(' ',TraceFile_p);
      }
      
      // Put out tracestring
      Chars = fprintf(TraceFile_p, "%s ", TraceString);
      while(Chars++ < 52)
      {
        putc(' ',TraceFile_p);
      }
      
      // Put out codeword
      for(i=0 ; i<Length ; i++)
      {
        if(((BitPattern>>(Length-i-1)) &0x1) == 0)
        {
          putc('0',TraceFile_p);
        }
        else
        {
          putc('1',TraceFile_p);
        }
      }
      
      // Put out newline
      Chars = fprintf(TraceFile_p, "\n");
    }
  }
#endif
}


/******************************************************************************
* Definition of external functions
******************************************************************************/


/******************************************************************************
*
* Name:        GetRbspTrailingBits
*
******************************************************************************/
void 
GetRbspTrailingBits(Parser_t *Parser_p)
{
  BitsReader_t *BitsReader_p = &(Parser_p->BitsReader);
  sint32 BitsLeft;

  if(GetBits(Parser_p, 1, NULL) != 1)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "RBSP shall start with a 1, not 0");
  }

  BitsLeft = BitsReader_p->Buffer32_Pos;

  if(BitsLeft != 0)
  {
#if D65_BITTRACE
    TraceBits(BitsLeft + 1, 1 << BitsLeft, "RbspTrailingBits");
#endif
    if(GetBits(Parser_p, BitsLeft, NULL) != 0)
    {
      BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "RBSP shall end with zeros only");
    }
  }
#if D65_BITTRACE
  else
  {
    TraceBits(1, 1, "RbspTrailingBits");
  }
#endif
  
  // There should be no bytes left of the NAL after decoding it
  if(BitsReader_p->CurrentBuffer_p != BitsReader_p->BufferEnd_p)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_TRAILING_BITS_IN_NAL,
              "NAL syntactically decoded but more data exists in NAL");
  }
  
  // There should be no bits left of the NAL after decoding it
  if(BitsReader_p->Buffer32_Pos)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_TRAILING_BITS_IN_NAL,
              "NAL syntactically decoded but more data exists in NAL");
  }
}

/******************************************************************************
*
* Name:        GetByteAlignmentBits
*
******************************************************************************/
void
GetByteAlignmentBits(Parser_t *Parser_p)
{
  sint32 BitsLeft;

  if(GetBits(Parser_p, 1, NULL) != 1)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR,
              "byte_alignment() shall start with a 1, not 0");
  }

  BitsLeft = Parser_p->BitsReader.Buffer32_Pos;

  if(BitsLeft != 0)
  {
#if D65_BITTRACE
    TraceBits(BitsLeft + 1, 1 << BitsLeft, "byte_alignment()");
#endif
    if(GetBits(Parser_p, BitsLeft, NULL) != 0)
    {
      BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR,
                "byte_alignment() shall end with zeros only");
    }
  }
#if D65_BITTRACE
  else
  {
    TraceBits(1, 1, "byte_alignment()");
  }
#endif
}

/******************************************************************************
*
* Name:        CheckByteAllignment
*
******************************************************************************/
Bool
CheckByteAllignment(BitsReader_t *BitsReader_p)
{
  sint32 BitsLeft = BitsReader_p->Buffer32_Pos;
  if(BitsLeft != 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/******************************************************************************
*
* Name:        GetUVLC
*
******************************************************************************/
#if !D65_BITTRACE
uint32
GetUVLCNoTrace(Parser_t *Parser_p)
{
  sint32 LeadingZeros = -1;
  uint32 TrailingBits = 0;
  uint32 Bit = 0;
  uint32 Val = 0;

  Bit = GetBits(Parser_p, 1, NULL);
  if(Bit == 0)
  {
    LeadingZeros++;
    while(Bit == 0)
    {
      Bit = GetBits(Parser_p, 1, NULL);
      LeadingZeros++;
    }
    TrailingBits = GetBits(Parser_p, LeadingZeros, NULL);
    Val = (1 << LeadingZeros) - 1 + TrailingBits;
  }
  return Val;
}

#else
uint32
GetUVLCTrace(Parser_t *Parser_p,
             const char *TraceString)
{
  sint32 LeadingZeros = -1;
  uint32 TrailingBits = 0;
  uint32 Bit = 0;
  uint32 Val = 0;

  Bit = GetBits(Parser_p, 1, NULL);
  if(Bit == 0)
  {
    LeadingZeros++;
    while(Bit == 0)
    {
      Bit = GetBits(Parser_p, 1, NULL);
      LeadingZeros++;
    }
    TrailingBits = GetBits(Parser_p, LeadingZeros, NULL);
    Val = (1 << LeadingZeros) - 1 + TrailingBits;
    TraceBits(LeadingZeros*2+1, (1<<LeadingZeros) + TrailingBits, TraceString);
  }
  else
  {
    TraceBits(1, 0, TraceString);
  }
  return Val;
}

#endif

/******************************************************************************
*
* Name:        GetSVLCNoTrace
*
******************************************************************************/
#if !D65_BITTRACE
sint32
GetSVLCNoTrace(Parser_t *Parser_p)
{
  uint32 Index;
  sint32 Value;

  Index = GetUVLC(Parser_p, NULL);
  Value = (Index + 1) >> 1;
  if((Index % 2) == 0)
  {
    Value = -Value;
  }
  return Value;
}
#else
sint32
GetSVLCTrace(Parser_t *Parser_p,
             const char *TraceString)
{
  uint32 Index;
  sint32 Value;

  Index = GetUVLC(Parser_p, TraceString);
  Value = (Index + 1) >> 1;
  if((Index % 2) == 0)
  {
    Value = -Value;
  }
  return Value;
}
#endif

/******************************************************************************
*
* Name:        GetBits
*
******************************************************************************/
#if !D65_BITTRACE
uint32
GetBitsNoTrace(Parser_t *Parser_p,
               const sint32 NumberOfBits)
{
  uint32 Bits;
  sint32 RemNumberOfBits = NumberOfBits;
  sint32 RemBytesInBuffer;
  uint32 BytesToRead = 0;
  sint32 NumBytesToRead;
  uint32 NextBuffer32Pos;
  uint32 Byte;
  uint32 Mask = ~(0x0);
  BitsReader_t *BitsReader_p = &Parser_p->BitsReader;
  const uint8* Buff_p = BitsReader_p->CurrentBuffer_p;

  if(BitsReader_p->Buffer32_Pos > NumberOfBits)
  {
    Bits = BitsReader_p->Buffer32 >> (BitsReader_p->Buffer32_Pos - NumberOfBits);
    Bits &= ~(Mask << NumberOfBits);
    BitsReader_p->Buffer32_Pos -= NumberOfBits;
    BitsReader_p->Buffer32 &= ~(Mask << BitsReader_p->Buffer32_Pos);
    return Bits;
  }

  RemNumberOfBits -= BitsReader_p->Buffer32_Pos;
  Bits = (BitsReader_p->Buffer32) & ~(Mask << (BitsReader_p->Buffer32_Pos));
  Bits = Bits << RemNumberOfBits;

  NumBytesToRead = (RemNumberOfBits - 1) >> 3;
  RemBytesInBuffer = (sint32)(BitsReader_p->BufferEnd_p - BitsReader_p->CurrentBuffer_p);
  if(NumBytesToRead > RemBytesInBuffer)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "Run out of bits during decoding");
  }

  while(NumBytesToRead >= 0)
  {
    Byte = *Buff_p++;
    if(Byte == 0x03 && BitsReader_p->PrevByte1 == 0 && BitsReader_p->PrevByte2 == 0)
    {
      BitsReader_p->PrevByte1 = 0x03;
      continue;
    }
    else
    {
      BitsReader_p->PrevByte2 = BitsReader_p->PrevByte1;
      BitsReader_p->PrevByte1 = Byte;
      BytesToRead |= (Byte << (NumBytesToRead << 3));
      NumBytesToRead--;
    }
  }

  NextBuffer32Pos = (32 - RemNumberOfBits) % 8;

  Bits = Bits | (BytesToRead >> NextBuffer32Pos);

  BitsReader_p->Buffer32_Pos = NextBuffer32Pos;
  BitsReader_p->Buffer32 = BytesToRead;
  BitsReader_p->Buffer32 &= ~(Mask << (BitsReader_p->Buffer32_Pos));
  BitsReader_p->CurrentBuffer_p = Buff_p;

  return (Bits);
}
#else
uint32
GetBitsTrace(Parser_t *Parser_p,
             const sint32 NumberOfBits,
             const char *TraceString)
{
  uint32 Bits;
  sint32 RemNumberOfBits = NumberOfBits;
  sint32 RemBytesInBuffer;
  uint32 BytesToRead = 0;
  sint32 NumBytesToRead;
  uint32 NextBuffer32Pos;
  uint32 Byte;
  uint32 Mask = ~(0x0);
  BitsReader_t *BitsReader_p = &Parser_p->BitsReader;
  const uint8* Buff_p = BitsReader_p->CurrentBuffer_p;

  if(BitsReader_p->Buffer32_Pos > NumberOfBits)
  {
    Bits = BitsReader_p->Buffer32 >> (BitsReader_p->Buffer32_Pos - NumberOfBits);
    Bits &= ~(Mask << NumberOfBits);
    BitsReader_p->Buffer32_Pos -= NumberOfBits;
    BitsReader_p->Buffer32 &= ~(Mask << BitsReader_p->Buffer32_Pos);
    if(TraceString)
    {
      TraceBits(NumberOfBits, Bits, TraceString);
    }
    return Bits;
  }

  RemNumberOfBits -= BitsReader_p->Buffer32_Pos;
  Bits = (BitsReader_p->Buffer32) & ~(Mask << (BitsReader_p->Buffer32_Pos));
  Bits = Bits << RemNumberOfBits;

  NumBytesToRead = (RemNumberOfBits - 1) >> 3;
  RemBytesInBuffer = BitsReader_p->BufferEnd_p - BitsReader_p->CurrentBuffer_p;
  if(NumBytesToRead > RemBytesInBuffer)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "Run out of bits during decoding");
  }

  while(NumBytesToRead >= 0)
  {
    Byte = *Buff_p++;
    if(Byte == 0x03 && BitsReader_p->PrevByte1 == 0 && BitsReader_p->PrevByte2 == 0)
    {
      BitsReader_p->PrevByte1 = 0x03;
      continue;
    }
    else
    {
      BitsReader_p->PrevByte2 = BitsReader_p->PrevByte1;
      BitsReader_p->PrevByte1 = Byte;
      BytesToRead |= (Byte << (NumBytesToRead << 3));
      NumBytesToRead--;
    }
  }

  NextBuffer32Pos = (32 - RemNumberOfBits) % 8;

  Bits = Bits | (BytesToRead >> NextBuffer32Pos);

  BitsReader_p->Buffer32_Pos = NextBuffer32Pos;
  BitsReader_p->Buffer32 = BytesToRead;
  BitsReader_p->Buffer32 &= ~(Mask << (BitsReader_p->Buffer32_Pos));
  BitsReader_p->CurrentBuffer_p = Buff_p;

  if(TraceString)
  {
    TraceBits(NumberOfBits, Bits, TraceString);
  }

  return (Bits);
}

#endif

