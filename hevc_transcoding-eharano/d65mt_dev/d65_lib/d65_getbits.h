
#ifndef D65_GETBITS_H
#define D65_GETBITS_H

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
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        GetRbspTrailingBits
*
* Parameters:  BitsReader_p   [In/Out]
*              JmpPos
*
* Returns:     -
*
* Description: Does byte alignment
*
******************************************************************************/
void 
GetRbspTrailingBits(Parser_t *Parser_p);


/******************************************************************************
*
* Name:        GetByteAlignmentBits
*
* Parameters:  BitsReader_p   [In/Out]
*              JmpPos
*
* Returns:     -
*
* Description: Does byte alignment
*
******************************************************************************/
void 
GetByteAlignmentBits(Parser_t *Parser_p);


/******************************************************************************
*
* Name:        CheckByteAllignment
*
* Parameters:  BitsReader_p   [In]
*
* Returns:     Bool            [Ret] True/False if byte alligned or not
*
* Description: Checks byte alignment
*
******************************************************************************/
Bool 
CheckByteAllignment(BitsReader_t *BitsReader_p);


/******************************************************************************
*
* Name:        GetUVLC
*
* Parameters:  Sessiondata_p   [In/Out]  The decoder
*              TraceString     [in]      Text string describing the symbol
*
* Returns:     uint32          [Ret]     The index
*
* Description: Returns the UVLC index of the next symbol
*
******************************************************************************/
#if !D65_BITTRACE
uint32
GetUVLCNoTrace(Parser_t *Parser_p);
#else
uint32
GetUVLCTrace(Parser_t *Parser_p,
             const char *TraceString);
#endif


/******************************************************************************
*
* Name:        GetSignedUVLC
*
* Parameters:  Sessiondata_p   [In/Out]  The decoder
*              TraceString     [in]      Text string describing the symbol
*
* Returns:     sint32          [Ret]     The index
*
* Description: Returns the signed UVLC index of the next symbol
*
******************************************************************************/
#if !D65_BITTRACE
sint32
GetSVLCNoTrace(Parser_t *Parser_p);
#else
sint32
GetSVLCTrace(Parser_t *Parser_p,
             const char *TraceString);
#endif


/******************************************************************************
*
* Name:        GetBits
*
* Parameters:  Sessiondata_p   [In/Out]  The decoder
*              NumberOfBits    [in]      Number of bits to read
*              TraceString     [in]      Text string describing the symbol
*
* Returns:     The next bits in bitstream
*
* Description: Returns the next NumberOfBits bits.
*
******************************************************************************/
#if !D65_BITTRACE
uint32
GetBitsNoTrace(Parser_t *Parser_p,
               const sint32 NumberOfBits);
#else
uint32
GetBitsTrace(Parser_t *Parser_p,
             const sint32 NumberOfBits,
             const char *TraceString);
#endif

#endif  /* D65_GETBITS_H */

