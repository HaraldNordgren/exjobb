/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains inverse quantization, inverse transform, and add&clip functions
*
******************************************************************************/

#include "d65_transform_reconstruct.h"

/******************************************************************************
* Definition of local functions
******************************************************************************/


/******************************************************************************
*
* Name:        partialInverse4
*
* Parameters:  source     [in/out]  The input block
*              target     [in]      The output block
*              shift      [in]      The shift
*
* Returns:     void
*
* Description: Performs a part of the 8x8 inverse transform, must be called
*              twice for full inverse transform. Reads from the source block
*              and writes on the target block.
*
******************************************************************************/
void partialInverse4(const sint16 *source, sint16 *target, const sint32 shift)
{
  const static sint16 factors[4][4] =
  {
    { 64, 64, 64, 64},
    { 83, 36,-36,-83},
    { 64,-64,-64, 64},
    { 36,-83, 83,-36}
  };

  sint32 E[2];
  sint32 O[2];
  sint32 add = 1<<(shift-1);
  sint32 col;

  for(col=0; col<4; col++)
  {
    O[0] = factors[1][0]*source[1*4 + col] + factors[3][0]*source[3*4 + col];
    O[1] = factors[1][1]*source[1*4 + col] + factors[3][1]*source[3*4 + col];
    E[0] = factors[0][0]*source[col] + factors[2][0]*source[2*4 + col];
    E[1] = factors[0][1]*source[col] + factors[2][1]*source[2*4 + col];

    target[col*4]     = (sint16)CLIP3(-32768, 32767, (E[0]+O[0]+add)>>shift);
    target[col*4 + 1] = (sint16)CLIP3(-32768, 32767, (E[1]+O[1]+add)>>shift);
    target[col*4 + 2] = (sint16)CLIP3(-32768, 32767, (E[1]-O[1]+add)>>shift);
    target[col*4 + 3] = (sint16)CLIP3(-32768, 32767, (E[0]-O[0]+add)>>shift);
  }
}

/******************************************************************************
*
* Name:        partialInverse4DST
*
* Parameters:  source     [in/out]  The input block
*              target     [in]      The output block
*              shift      [in]      The shift
*
* Returns:     void
*
* Description: Performs a part of the 4x4 inverse discrete sine transform, must be called
*              twice for full inverse transform. Reads from the source block
*              and writes on the target block.
*
******************************************************************************/
void partialInverse4DST(const sint16 *source, sint16 *target, const sint32 shift)
{
  sint32 col;
  sint32 c[4];
  sint32 add = 1<<(shift-1);
  for (col = 0; col < 4; col++)
  {  
    // Intermediate Variables
    c[0] = source[col] + source[8+col];
    c[1] = source[8+col] + source[12+col];
    c[2] = source[  col] - source[12+col];
    c[3] = 74* source[4+col];

    target[4*col+0] = (sint16)CLIP3( -32768, 32767, ( 29 * c[0]         + 55 * c[1]       + c[3]              + add ) >> shift );
    target[4*col+1] = (sint16)CLIP3( -32768, 32767, ( 55 * c[2]         - 29 * c[1]       + c[3]              + add ) >> shift );
    target[4*col+2] = (sint16)CLIP3( -32768, 32767, ( 74 * (source[col] - source[8+col]   + source[12+col])   + add ) >> shift );
    target[4*col+3] = (sint16)CLIP3( -32768, 32767, ( 55 * c[0]         + 29 * c[2]       - c[3]              + add ) >> shift );
  }
}

/******************************************************************************
*
* Name:        partialInverse8
*
* Parameters:  source     [in/out]  The input block
*              target     [in]      The output block
*              shift      [in]      The shift
*
* Returns:     void
*
* Description: Performs a part of the 8x8 inverse transform, must be called
*              twice for full inverse transform. Reads from the source block
*              and writes on the target block.
*
******************************************************************************/
void
partialInverse8(const sint16 *source, sint16 *target, const sint32 shift)
{
  const static sint16 factors[8][8] =
  {
    { 64, 64, 64, 64, 64, 64, 64, 64},
    { 89, 75, 50, 18,-18,-50,-75,-89},
    { 83, 36,-36,-83,-83,-36, 36, 83},
    { 75,-18,-89,-50, 50, 89, 18,-75},
    { 64,-64,-64, 64, 64,-64,-64, 64},
    { 50,-89, 18, 75,-75,-18, 89,-50},
    { 36,-83, 83,-36,-36, 83,-83, 36},
    { 18,-50, 75,-89, 89,-75, 50,-18}
  };

  sint32 col;

  sint32 add = 1<<(shift-1);
#if SIMD
  sint32 __declspec(align(16))E[8];
  sint32 __declspec(align(16))EE[4];
  sint32 __declspec(align(16))EO[4];
  __m128i ADD   = _mm_set1_epi32(add);
  __m128i E_p;
  __m128i O_p ;
  __m128i EO_p, OE_p;
    __m128i FactorLine1, FactorLine2;
  __m128i FactorLow;
  __m128i Source;
#else
  sint32 i;
  sint32 E[8];
  sint32 O[8];
  sint32 EE[4];
  sint32 EO[4];
#endif

  for(col=0; col<8; col++)
  {
#if SIMD
    FactorLine1 = _mm_loadl_epi64((__m128i*)factors[1]);
    FactorLine2 = _mm_loadl_epi64((__m128i*)factors[3]);
    FactorLow = _mm_unpacklo_epi16(FactorLine1, FactorLine2);
    Source = _mm_unpacklo_epi16(_mm_set1_epi16(source[8+col]), _mm_set1_epi16(source[24+col]));
    O_p = _mm_madd_epi16(Source, FactorLow);

    FactorLine1 = _mm_loadl_epi64((__m128i*)factors[5]);
    FactorLine2 = _mm_loadl_epi64((__m128i*)factors[7]);
    FactorLow = _mm_unpacklo_epi16(FactorLine1, FactorLine2);
    Source = _mm_unpacklo_epi16(_mm_set1_epi16(source[40+col]), _mm_set1_epi16(source[56+col]));
    O_p = _mm_add_epi32(O_p, _mm_madd_epi16(Source, FactorLow));
#else
    for (i=0; i<4; i++)
    {
      O[i] = factors[1][i] * source[1*8 + col] + 
             factors[3][i] * source[3*8 + col] +
             factors[5][i] * source[5*8 + col] +
             factors[7][i] * source[7*8 + col];
    }
#endif
    EO[0]=factors[2][0]*source[2*8 + col] + factors[6][0]*source[6*8 + col];
    EO[1]=factors[2][1]*source[2*8 + col] + factors[6][1]*source[6*8 + col];
    EE[0]=factors[0][0]*source[0*8 + col] + factors[4][0]*source[4*8 + col];
    EE[1]=factors[0][1]*source[0*8 + col] + factors[4][1]*source[4*8 + col];

    E[0]=EE[0]+EO[0];
    E[3]=EE[0]-EO[0];
    E[1]=EE[1]+EO[1];
    E[2]=EE[1]-EO[1];

#if SIMD
    E_p = _mm_load_si128((__m128i*)E);

    EO_p = _mm_add_epi32(E_p,O_p);
    EO_p = _mm_add_epi32(EO_p, ADD);
    EO_p = _mm_srai_epi32(EO_p,shift);

    OE_p = _mm_sub_epi32(_mm_shuffle_epi32(E_p,_MM_SHUFFLE(0,1,2,3)) ,_mm_shuffle_epi32(O_p,_MM_SHUFFLE(0,1,2,3)));
    OE_p = _mm_add_epi32(OE_p, ADD);
    OE_p = _mm_srai_epi32(OE_p,shift);

    _mm_storel_epi64((__m128i*)(target+8*col),_mm_packs_epi32(EO_p,EO_p)); 
    _mm_storel_epi64((__m128i*)(target+8*col+4),_mm_packs_epi32(OE_p,OE_p)); 
#else
    for (i=0; i<4; i++)
    {
      target[col*8 + i    ] = (sint16)CLIP3(-32768, 32767, (E[  i] + O[  i] + add)>>shift);
      target[col*8 + i + 4] = (sint16)CLIP3(-32768, 32767, (E[3-i] - O[3-i] + add)>>shift);
    }
#endif
  }
}

/******************************************************************************
*
* Name:        partialInverse16
*
* Parameters:  source     [in/out]  The input block
*              target     [in]      The output block
*              shift      [in]      The shift
*
* Returns:     void
*
* Description: Performs a part of the 16x16 inverse transform, must be called
*              twice for full inverse transform. Reads from the source block
*              and writes on the target block.
*
******************************************************************************/
void
partialInverse16(const sint16 *source, sint16 *target, const sint32 shift)
{
#if SIMD
  sint32 k;
  sint32 __declspec(align(16))EE[4];
  sint32 __declspec(align(16))EEO[2];
  sint32 __declspec(align(16))EEE[2];
  __m128i FactLine1, FactLine2;
  __m128i FactLow, FactHigh;
  __m128i Source;
  __m128i O_low,O_high;
  __m128i EO_low;
  __m128i EE_temp;
  __m128i E1,E2,E1_temp,E2_temp;
  __m128i ADD;
  const static sint16 __declspec(align(16))factors[16][16] =
#else
  sint32 E[8];
  sint32 O[8];
  sint32 EE[4];
  sint32 EO[4];
  sint32 EEE[2];
  sint32 EEO[2];
  sint32 size = 16;
  const static sint16 factors[16][16] =
#endif
  {
    { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
    { 90, 87, 80, 70, 57, 43, 25,  9, -9,-25,-43,-57,-70,-80,-87,-90},
    { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
    { 87, 57,  9,-43,-80,-90,-70,-25, 25, 70, 90, 80, 43, -9,-57,-87},
    { 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83},
    { 80,  9,-70,-87,-25, 57, 90, 43,-43,-90,-57, 25, 87, 70, -9,-80},
    { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
    { 70,-43,-87,  9, 90, 25,-80,-57, 57, 80,-25,-90, -9, 87, 43,-70},
    { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
    { 57,-80,-25, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 25, 80,-57},
    { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
    { 43,-90, 57, 25,-87, 70,  9,-80, 80, -9,-70, 87,-25,-57, 90,-43},
    { 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36},
    { 25,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-25},
    { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
    {  9,-25, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 25, -9}
  };

  sint32 col;
  sint32 i;
  sint32 add = 1<<(shift-1);

#if SIMD

  ADD = _mm_set1_epi32(add);

  for (col = 0; col < 16; col++)
  {
    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[16+col])),_mm_set1_epi16((source[48+col])));
    FactLine1 = _mm_load_si128((__m128i*)(factors[1]));
    FactLine2 = _mm_load_si128((__m128i*)(factors[3]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
    O_low  = _mm_madd_epi16(FactLow,  Source);
    O_high = _mm_madd_epi16(FactHigh, Source);

    for(i = 5; i < 16; i += 4)
    {
      Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[(i << 4) +col])), _mm_set1_epi16((source[(i << 4)+col+32])));
      FactLine1 = _mm_load_si128((__m128i*)(factors[i]));
      FactLine2 = _mm_load_si128((__m128i*)(factors[i+2]));
      FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
      FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
      O_low  = _mm_add_epi32(_mm_madd_epi16(FactLow,  Source), O_low);
      O_high = _mm_add_epi32(_mm_madd_epi16(FactHigh, Source), O_high);
    }

    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[32+col])), _mm_set1_epi16((source[96+col])));
    FactLine1 = _mm_loadl_epi64((__m128i*)(factors[2]));
    FactLine2 = _mm_loadl_epi64((__m128i*)(factors[6]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    EO_low  = _mm_madd_epi16(FactLow,  Source);

    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[160+col])), _mm_set1_epi16((source[224+col])));
    FactLine1 = _mm_loadl_epi64((__m128i*)(factors[10]));
    FactLine2 = _mm_loadl_epi64((__m128i*)(factors[14]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    EO_low  = _mm_add_epi32(_mm_madd_epi16(FactLow,  Source), EO_low);
    EEO[0] = factors[4][0]*source[4*16+col] + factors[12][0]*source[12*16+col];
    EEE[0] = factors[0][0]*source[col] + factors[8][0]*source[8*16+col];
    EEO[1] = factors[4][1]*source[4*16+col] + factors[12][1]*source[12*16+col];
    EEE[1] = factors[0][1]*source[col] + factors[8][1]*source[8*16+col];

    for (k = 0; k < 2; k++)
    {
      EE[k]   = EEE[k] + EEO[k];
      EE[k+2] = EEE[1-k] - EEO[1-k];
    }    
    EE_temp = _mm_load_si128((__m128i*)EE);
    E1 = _mm_add_epi32(EE_temp,EO_low);
    E2 = _mm_sub_epi32(_mm_shuffle_epi32(EE_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(EO_low,_MM_SHUFFLE(0,1,2,3)));

    E1_temp = _mm_add_epi32(E1,ADD);
    E1 = _mm_sub_epi32(_mm_shuffle_epi32(E1_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_low,_MM_SHUFFLE(0,1,2,3)));
    E1_temp = _mm_add_epi32(E1_temp,O_low);
    E1_temp = _mm_srai_epi32(E1_temp,shift);

    E2_temp = _mm_add_epi32(E2,ADD);
    E2 = _mm_sub_epi32(_mm_shuffle_epi32(E2_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_high,_MM_SHUFFLE(0,1,2,3)));
    E2_temp = _mm_add_epi32(E2_temp,O_high);
    E2_temp = _mm_srai_epi32(E2_temp,shift);
    _mm_storeu_si128((__m128i*)(target+16*col),_mm_packs_epi32(E1_temp,E2_temp));

    E2 = _mm_srai_epi32(E2,shift);
    E1 = _mm_srai_epi32(E1,shift);
    _mm_storeu_si128((__m128i*)(target+16*col+8),_mm_packs_epi32(E2,E1)); 
  }
#else
  for (col=0; col<size; col++)
  {
    for (i=0; i<8; i++)
    {
      O[i] = factors[1][i]  * source[1*16  + col] + 
             factors[3][i]  * source[3*16  + col] + 
             factors[5][i]  * source[5*16  + col] + 
             factors[7][i]  * source[7*16  + col] + 
             factors[9][i]  * source[9*16  + col] + 
             factors[11][i] * source[11*16 + col] + 
             factors[13][i] * source[13*16 + col] + 
             factors[15][i] * source[15*16 + col];
    }
    for (i=0; i<4; i++)
    {
      EO[i] = factors[2][i]  * source[2*16  + col] +
              factors[6][i]  * source[6*16  + col] +
              factors[10][i] * source[10*16 + col] +
              factors[14][i] * source[14*16 + col];
    }
    EEO[0] = factors[4][0]*source[4*16 + col] + factors[12][0]*source[12*16 + col];
    EEE[0] = factors[0][0]*source[0*16 + col] + factors[ 8][0]*source[8*16 + col];
    EEO[1] = factors[4][1]*source[4*16 + col] + factors[12][1]*source[12*16 + col];
    EEE[1] = factors[0][1]*source[0*16 + col] + factors[ 8][1]*source[8*16 + col];

    for (i=0; i<2; i++)
    {
      EE[i]   = EEE[i]   + EEO[i];
      EE[i+2] = EEE[1-i] - EEO[1-i];
    }
    for (i=0; i<4; i++)
    {
      E[i]   = EE[i]   + EO[i];
      E[i+4] = EE[3-i] - EO[3-i];
    }
    for (i=0; i<8; i++)
    {
      target[col*16 + i]     = (sint16)CLIP3(-32768, 32767, (E[i]   + O[i]   + add)>>shift);
      target[col*16 + i + 8] = (sint16)CLIP3(-32768, 32767, (E[7-i] - O[7-i] + add)>>shift);
    }
  }
#endif
}

/******************************************************************************
*
* Name:        partialInverse32
*
* Parameters:  source     [in/out]  The input block
*              target     [in]      The output block
*              shift      [in]      The shift
*
* Returns:     void
*
* Description: Performs a part of the 32x32 inverse transform, must be called
*              twice for full inverse transform. Reads from the source block
*              and writes on the target block.
*
******************************************************************************/
void
partialInverse32(const sint16 *source, sint16 *target, const sint32 shift)
{
#if SIMD
  const static sint16 __declspec(align(16))factors[32][32] =
#else
  const static sint16 factors[32][32] =
#endif
  {
    { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
    { 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13,  4, -4,-13,-22,-31,-38,-46,-54,-61,-67,-73,-78,-82,-85,-88,-90,-90},
    { 90, 87, 80, 70, 57, 43, 25,  9, -9,-25,-43,-57,-70,-80,-87,-90,-90,-87,-80,-70,-57,-43,-25, -9,  9, 25, 43, 57, 70, 80, 87, 90},
    { 90, 82, 67, 46, 22, -4,-31,-54,-73,-85,-90,-88,-78,-61,-38,-13, 13, 38, 61, 78, 88, 90, 85, 73, 54, 31,  4,-22,-46,-67,-82,-90},
    { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89, 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},
    { 88, 67, 31,-13,-54,-82,-90,-78,-46, -4, 38, 73, 90, 85, 61, 22,-22,-61,-85,-90,-73,-38,  4, 46, 78, 90, 82, 54, 13,-31,-67,-88},
    { 87, 57,  9,-43,-80,-90,-70,-25, 25, 70, 90, 80, 43, -9,-57,-87,-87,-57, -9, 43, 80, 90, 70, 25,-25,-70,-90,-80,-43,  9, 57, 87},
    { 85, 46,-13,-67,-90,-73,-22, 38, 82, 88, 54, -4,-61,-90,-78,-31, 31, 78, 90, 61,  4,-54,-88,-82,-38, 22, 73, 90, 67, 13,-46,-85},
    { 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83},
    { 82, 22,-54,-90,-61, 13, 78, 85, 31,-46,-90,-67,  4, 73, 88, 38,-38,-88,-73, -4, 67, 90, 46,-31,-85,-78,-13, 61, 90, 54,-22,-82},
    { 80,  9,-70,-87,-25, 57, 90, 43,-43,-90,-57, 25, 87, 70, -9,-80,-80, -9, 70, 87, 25,-57,-90,-43, 43, 90, 57,-25,-87,-70,  9, 80},
    { 78, -4,-82,-73, 13, 85, 67,-22,-88,-61, 31, 90, 54,-38,-90,-46, 46, 90, 38,-54,-90,-31, 61, 88, 22,-67,-85,-13, 73, 82,  4,-78},
    { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75, 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},
    { 73,-31,-90,-22, 78, 67,-38,-90,-13, 82, 61,-46,-88, -4, 85, 54,-54,-85,  4, 88, 46,-61,-82, 13, 90, 38,-67,-78, 22, 90, 31,-73},
    { 70,-43,-87,  9, 90, 25,-80,-57, 57, 80,-25,-90, -9, 87, 43,-70,-70, 43, 87, -9,-90,-25, 80, 57,-57,-80, 25, 90,  9,-87,-43, 70},
    { 67,-54,-78, 38, 85,-22,-90,  4, 90, 13,-88,-31, 82, 46,-73,-61, 61, 73,-46,-82, 31, 88,-13,-90, -4, 90, 22,-85,-38, 78, 54,-67},
    { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},
    { 61,-73,-46, 82, 31,-88,-13, 90, -4,-90, 22, 85,-38,-78, 54, 67,-67,-54, 78, 38,-85,-22, 90,  4,-90, 13, 88,-31,-82, 46, 73,-61},
    { 57,-80,-25, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 25, 80,-57,-57, 80, 25,-90,  9, 87,-43,-70, 70, 43,-87, -9, 90,-25,-80, 57},
    { 54,-85, -4, 88,-46,-61, 82, 13,-90, 38, 67,-78,-22, 90,-31,-73, 73, 31,-90, 22, 78,-67,-38, 90,-13,-82, 61, 46,-88,  4, 85,-54},
    { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50, 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},
    { 46,-90, 38, 54,-90, 31, 61,-88, 22, 67,-85, 13, 73,-82,  4, 78,-78, -4, 82,-73,-13, 85,-67,-22, 88,-61,-31, 90,-54,-38, 90,-46},
    { 43,-90, 57, 25,-87, 70,  9,-80, 80, -9,-70, 87,-25,-57, 90,-43,-43, 90,-57,-25, 87,-70, -9, 80,-80,  9, 70,-87, 25, 57,-90, 43},
    { 38,-88, 73, -4,-67, 90,-46,-31, 85,-78, 13, 61,-90, 54, 22,-82, 82,-22,-54, 90,-61,-13, 78,-85, 31, 46,-90, 67,  4,-73, 88,-38},
    { 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36},
    { 31,-78, 90,-61,  4, 54,-88, 82,-38,-22, 73,-90, 67,-13,-46, 85,-85, 46, 13,-67, 90,-73, 22, 38,-82, 88,-54, -4, 61,-90, 78,-31},
    { 25,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-25,-25, 70,-90, 80,-43, -9, 57,-87, 87,-57,  9, 43,-80, 90,-70, 25},
    { 22,-61, 85,-90, 73,-38, -4, 46,-78, 90,-82, 54,-13,-31, 67,-88, 88,-67, 31, 13,-54, 82,-90, 78,-46,  4, 38,-73, 90,-85, 61,-22},
    { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18, 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},
    { 13,-38, 61,-78, 88,-90, 85,-73, 54,-31,  4, 22,-46, 67,-82, 90,-90, 82,-67, 46,-22, -4, 31,-54, 73,-85, 90,-88, 78,-61, 38,-13},
    {  9,-25, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 25, -9, -9, 25,-43, 57,-70, 80,-87, 90,-90, 87,-80, 70,-57, 43,-25,  9},
    {  4,-13, 22,-31, 38,-46, 54,-61, 67,-73, 78,-82, 85,-88, 90,-90, 90,-90, 88,-85, 82,-78, 73,-67, 61,-54, 46,-38, 31,-22, 13, -4}
  };

#if SIMD
  sint32 k;
  sint32 i;
  sint32 col;
  sint32 add = 1<<(shift-1);
  sint32 __declspec(align(16))EE[8];
  sint32 __declspec(align(16))EEO[4];
  sint32 __declspec(align(16))EEEE[2],EEEO[2];
  sint32 __declspec(align(16))EEE[4];
  __m128i FactLine1, FactLine2;
  __m128i FactLow, FactHigh;
  __m128i Source;
  __m128i O_low[2],O_high[2];
  __m128i EO_low, EO_high;
  __m128i EE_low, EE_high;
  __m128i E1,E2,E3,E4,E1_temp,E2_temp,E3_temp,E4_temp, EEO_temp;
  __m128i ADD;
  ADD = _mm_set1_epi32(add);
  
  for (col = 0; col < 32; col++)
  {
    for (k = 0; k < 16; k+=8)
    {
      Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[32+col])),_mm_set1_epi16((source[96+col])));
      FactLine1 = _mm_load_si128((__m128i*)(factors[1]+k));
      FactLine2 = _mm_load_si128((__m128i*)(factors[3]+k));
      FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
      FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
      O_low[k/8]  = _mm_madd_epi16(FactLow,  Source);
      O_high[k/8] = _mm_madd_epi16(FactHigh, Source);

      for(i = 5; i < 32; i += 4)
      {
        Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[(i << 5) +col])), _mm_set1_epi16((source[(i << 5)+col+64])));
        FactLine1 = _mm_load_si128((__m128i*)(factors[i]+k));
        FactLine2 = _mm_load_si128((__m128i*)(factors[i+2]+k));
        FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
        FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
        O_low[k/8]  = _mm_add_epi32(_mm_madd_epi16(FactLow,  Source), O_low[k/8]);
        O_high[k/8] = _mm_add_epi32(_mm_madd_epi16(FactHigh, Source), O_high[k/8]);
      }
    }

    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[64+col])), _mm_set1_epi16((source[192+col])));
    FactLine1 = _mm_load_si128((__m128i*)(factors[2]));
    FactLine2 = _mm_load_si128((__m128i*)(factors[6]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
    EO_low  = _mm_madd_epi16(FactLow,  Source);
    EO_high = _mm_madd_epi16(FactHigh, Source);

    for(i = 10; i < 32; i += 8)
    {
      Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[(i<<5)+col])), _mm_set1_epi16((source[(i<<5)+128+col])));
      FactLine1 = _mm_load_si128((__m128i*)(factors[i]));
      FactLine2 = _mm_load_si128((__m128i*)(factors[i+4]));
      FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
      FactHigh = _mm_unpackhi_epi16(FactLine1, FactLine2);
      EO_low  = _mm_add_epi32(_mm_madd_epi16(FactLow,  Source), EO_low);
      EO_high = _mm_add_epi32(_mm_madd_epi16(FactHigh, Source), EO_high);
    }

    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[128+col])), _mm_set1_epi16((source[384+col])));
    FactLine1 = _mm_loadl_epi64((__m128i*)(factors[4]));
    FactLine2 = _mm_loadl_epi64((__m128i*)(factors[12]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    EEO_temp  = _mm_madd_epi16(FactLow,  Source);

    Source    = _mm_unpacklo_epi16(_mm_set1_epi16((source[640+col])), _mm_set1_epi16((source[896+col])));
    FactLine1 = _mm_loadl_epi64((__m128i*)(factors[20]));
    FactLine2 = _mm_loadl_epi64((__m128i*)(factors[28]));
    FactLow  = _mm_unpacklo_epi16(FactLine1, FactLine2);
    EEO_temp  = _mm_add_epi32(_mm_madd_epi16(FactLow,  Source), EEO_temp);

    _mm_storeu_si128((__m128i*)(EEO),EEO_temp);

    EEEO[0] = factors[8][0]*source[ 8*32 + col ] + factors[24][0]*source[ 24*32 + col ];
    EEEO[1] = factors[8][1]*source[ 8*32 + col ] + factors[24][1]*source[ 24*32 + col ];
    EEEE[0] = factors[0][0]*source[ 0*32 + col ] + factors[16][0]*source[ 16*32 + col ];
    EEEE[1] = factors[0][1]*source[ 0*32 + col ] + factors[16][1]*source[ 16*32 + col ];

    EEE[0] = EEEE[0] + EEEO[0];
    EEE[3] = EEEE[0] - EEEO[0];
    EEE[1] = EEEE[1] + EEEO[1];
    EEE[2] = EEEE[1] - EEEO[1];   

    for (k=0;k<4;k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+4] = EEE[3-k] - EEO[3-k];
    }    

    EE_low = _mm_load_si128((__m128i*)EE);
    EE_high = _mm_load_si128((__m128i*)EE+1);

    E1 = _mm_add_epi32(EE_low,EO_low);
    E2 = _mm_add_epi32(EE_high,EO_high);

    E3 = _mm_sub_epi32(_mm_shuffle_epi32(EE_high,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(EO_high,_MM_SHUFFLE(0,1,2,3)));
    E4 = _mm_sub_epi32(_mm_shuffle_epi32(EE_low,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(EO_low,_MM_SHUFFLE(0,1,2,3)));

    E1_temp = _mm_add_epi32(E1,ADD);
    E1 = _mm_sub_epi32(_mm_shuffle_epi32(E1_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_low[0],_MM_SHUFFLE(0,1,2,3)));
    E1_temp = _mm_add_epi32(E1_temp,O_low[0]);
    E1_temp = _mm_srai_epi32(E1_temp,shift);

    E2_temp = _mm_add_epi32(E2,ADD);
    E2 = _mm_sub_epi32(_mm_shuffle_epi32(E2_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_high[0],_MM_SHUFFLE(0,1,2,3)));
    E2_temp = _mm_add_epi32(E2_temp,O_high[0]);
    E2_temp = _mm_srai_epi32(E2_temp,shift);
    _mm_storeu_si128((__m128i*)(target + 32*col),_mm_packs_epi32(E1_temp,E2_temp));

    E3_temp = _mm_add_epi32(E3,ADD);
    E3 = _mm_sub_epi32(_mm_shuffle_epi32(E3_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_low[1],_MM_SHUFFLE(0,1,2,3)));
    E3_temp = _mm_add_epi32(E3_temp,O_low[1]);
    E3_temp = _mm_srai_epi32(E3_temp,shift);

    E4_temp = _mm_add_epi32(E4,ADD);
    E4 = _mm_sub_epi32(_mm_shuffle_epi32(E4_temp,_MM_SHUFFLE(0,1,2,3)),_mm_shuffle_epi32(O_high[1],_MM_SHUFFLE(0,1,2,3)));
    E4_temp = _mm_add_epi32(E4_temp,O_high[1]);
    E4_temp = _mm_srai_epi32(E4_temp,shift);
    _mm_storeu_si128((__m128i*)(target + 32*col + 8),_mm_packs_epi32(E3_temp,E4_temp));     

    E4 = _mm_srai_epi32(E4,shift);   
    E3 = _mm_srai_epi32(E3,shift);
    _mm_storeu_si128((__m128i*)(target + 32*col + 16),_mm_packs_epi32(E4,E3));
   
    E2 = _mm_srai_epi32(E2,shift);
    E1 = _mm_srai_epi32(E1,shift);
    _mm_storeu_si128((__m128i*)(target + 32*col + 24),_mm_packs_epi32(E2,E1));
  }
#else
  sint32 col;
  sint32 k;
  sint32 E[16];
  sint32 O[16];
  sint32 EE[8];
  sint32 EO[8];
  sint32 EEE[4];
  sint32 EEO[4];
  sint32 EEEE[2];
  sint32 EEEO[2];
  sint32 add = 1<<(shift-1);
  sint32 size = 32;

  for(col=0; col<size; col++)
  {    
    /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
    for(k=0; k<16; k++)
    {
      O[k] = factors[ 1][k]*source[ 1*32 + col  ] + factors[ 3][k]*source[ 3*32 + col  ] + 
             factors[ 5][k]*source[ 5*32 + col  ] + factors[ 7][k]*source[ 7*32 + col  ] + 
             factors[ 9][k]*source[ 9*32 + col  ] + factors[11][k]*source[ 11*32 + col ] + 
             factors[13][k]*source[ 13*32 + col ] + factors[15][k]*source[ 15*32 + col ] + 
             factors[17][k]*source[ 17*32 + col ] + factors[19][k]*source[ 19*32 + col ] + 
             factors[21][k]*source[ 21*32 + col ] + factors[23][k]*source[ 23*32 + col ] + 
             factors[25][k]*source[ 25*32 + col ] + factors[27][k]*source[ 27*32 + col ] + 
             factors[29][k]*source[ 29*32 + col ] + factors[31][k]*source[ 31*32 + col ];
    }
    for(k=0; k<8; k++)
    {
      EO[k] = factors[ 2][k]*source[ 2*32 + col ] + factors[ 6][k]*source[ 6*32 + col ] + 
              factors[10][k]*source[ 10*32 + col ] + factors[14][k]*source[ 14*32 + col ] + 
              factors[18][k]*source[ 18*32 + col ] + factors[22][k]*source[ 22*32 + col ] + 
              factors[26][k]*source[ 26*32 + col ] + factors[30][k]*source[ 30*32 + col ];
    }
    for(k=0; k<4; k++)
    {
      EEO[k] = factors[4 ][k]*source[ 4*32 + col ] + factors[12][k]*source[ 12*32 + col ] + 
               factors[20][k]*source[ 20*32 + col ] + factors[28][k]*source[ 28*32 + col ];
    }
    EEEO[0] = factors[8][0]*source[ 8*32 + col ] + factors[24][0]*source[ 24*32 + col ];
    EEEO[1] = factors[8][1]*source[ 8*32 + col ] + factors[24][1]*source[ 24*32 + col ];
    EEEE[0] = factors[0][0]*source[ 0*32 + col ] + factors[16][0]*source[ 16*32 + col ];    
    EEEE[1] = factors[0][1]*source[ 0*32 + col ] + factors[16][1]*source[ 16*32 + col ];

    /* Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector */
    EEE[0] = EEEE[0] + EEEO[0];
    EEE[3] = EEEE[0] - EEEO[0];
    EEE[1] = EEEE[1] + EEEO[1];
    EEE[2] = EEEE[1] - EEEO[1];    
    for(k=0; k<4; k++)
    {
      EE[k] = EEE[k] + EEO[k];
      EE[k+4] = EEE[3-k] - EEO[3-k];
    }    
    for(k=0; k<8; k++)
    {
      E[k] = EE[k] + EO[k];
      E[k+8] = EE[7-k] - EO[7-k];
    }    
    for(k=0; k<16; k++)
    {
      target[col*32 + k]    = (sint16)CLIP3( -32768, 32767, (E[k] + O[k] + add)>>shift );
      target[col*32 + k+16] = (sint16)CLIP3( -32768, 32767, (E[15-k] - O[15-k] + add)>>shift );
    }
  }
#endif
}

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        performDequantization
*
******************************************************************************/
void
performDequantization(sint16 *Array, const sint32 log2Size, const sint32 qp)
{
  sint32 y;
  static const sint32 quantScale[6] =
  {
    40,45,51,57,64,72
  };
  sint32 transformShift = 7-log2Size;
  sint32 shift = 6-transformShift;
  sint32 roundAdd = 1 << (shift-1);
  sint32 scale = quantScale[qp%6] << (qp/6);
  sint32 size = 1<<log2Size;
#if SIMD
  __m128i temp;
  __m128i coeffsLowLow,coeffsLowHigh,coeffsHighLow,coeffsHighHigh,coeffsLow,coeffsHigh;
#endif

#if SIMD
  if(log2Size == 5)
  {
    sint32 x;
    for(y = 0; y < 32; y++)
    {
      for(x = 0; x < 32; x += 16)
      {
        //load
        coeffsLow  = _mm_loadu_si128((__m128i*)(Array+y*size + x));
        coeffsHigh = _mm_loadu_si128((__m128i*)(Array+y*size+8 + x));

        // 16bit to 32bit
        coeffsLowLow   = _mm_unpacklo_epi16(coeffsLow, coeffsLow);
        coeffsLowHigh  = _mm_unpackhi_epi16(coeffsLow, coeffsLow);
        coeffsHighLow  = _mm_unpacklo_epi16(coeffsHigh, coeffsHigh);
        coeffsHighHigh = _mm_unpackhi_epi16(coeffsHigh, coeffsHigh);

        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , 16);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , 16);
        coeffsHighLow  = _mm_srai_epi32(coeffsHighLow , 16);
        coeffsHighHigh = _mm_srai_epi32(coeffsHighHigh, 16);

        //dequantize
        // clippedQuantized * scale
        temp = _mm_set1_epi32((sint16)scale);
        coeffsLowLow   = _mm_mullo_epi32(coeffsLowLow   , temp);
        coeffsLowHigh  = _mm_mullo_epi32(coeffsLowHigh  , temp);
        coeffsHighLow  = _mm_mullo_epi32(coeffsHighLow  , temp);
        coeffsHighHigh = _mm_mullo_epi32(coeffsHighHigh , temp);

        // clippedQuantized * scale +roundAdd
        temp = _mm_set1_epi32(roundAdd);
        coeffsLowLow   = _mm_add_epi32(coeffsLowLow  , temp);
        coeffsLowHigh  = _mm_add_epi32(coeffsLowHigh , temp);
        coeffsHighLow  = _mm_add_epi32(coeffsHighLow , temp);
        coeffsHighHigh = _mm_add_epi32(coeffsHighHigh, temp);

        // ( clippedQuantized * scale + roundAdd ) >> shift
        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , shift);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , shift);
        coeffsHighLow  = _mm_srai_epi32(coeffsHighLow , shift);
        coeffsHighHigh = _mm_srai_epi32(coeffsHighHigh, shift);

        //32bit to 16bit
        coeffsLow = _mm_packs_epi32(coeffsLowLow, coeffsLowHigh);
        coeffsHigh = _mm_packs_epi32(coeffsHighLow, coeffsHighHigh);

        //store
        _mm_storeu_si128((__m128i*)(Array+y*size + x),   coeffsLow);
        _mm_storeu_si128((__m128i*)(Array+y*size + 8 + x), coeffsHigh);
      }
    }
  }
  if(log2Size == 4)
  {
    for(y = 0; y < 16; y++)
    {
      //for(Int x=0; x<16; x++)
      {
        //load
        coeffsLow  = _mm_loadu_si128((__m128i*)(Array+y*size));
        coeffsHigh = _mm_loadu_si128((__m128i*)(Array+y*size+8));

        // 16bit to 32bit
        coeffsLowLow   = _mm_unpacklo_epi16(coeffsLow, coeffsLow);
        coeffsLowHigh  = _mm_unpackhi_epi16(coeffsLow, coeffsLow);
        coeffsHighLow  = _mm_unpacklo_epi16(coeffsHigh, coeffsHigh);
        coeffsHighHigh = _mm_unpackhi_epi16(coeffsHigh, coeffsHigh);

        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , 16);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , 16);
        coeffsHighLow  = _mm_srai_epi32(coeffsHighLow , 16);
        coeffsHighHigh = _mm_srai_epi32(coeffsHighHigh, 16);

        //dequantize
        // clippedQuantized * scale
        temp = _mm_set1_epi32((sint16)scale);
        coeffsLowLow   = _mm_mullo_epi32(coeffsLowLow   , temp);
        coeffsLowHigh  = _mm_mullo_epi32(coeffsLowHigh  , temp);
        coeffsHighLow  = _mm_mullo_epi32(coeffsHighLow  , temp);
        coeffsHighHigh = _mm_mullo_epi32(coeffsHighHigh , temp);

        // clippedQuantized * scale +roundAdd
        temp = _mm_set1_epi32(roundAdd);
        coeffsLowLow   = _mm_add_epi32(coeffsLowLow  , temp);
        coeffsLowHigh  = _mm_add_epi32(coeffsLowHigh , temp);
        coeffsHighLow  = _mm_add_epi32(coeffsHighLow , temp);
        coeffsHighHigh = _mm_add_epi32(coeffsHighHigh, temp);

        // ( clippedQuantized * scale + roundAdd ) >> shift
        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , shift);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , shift);
        coeffsHighLow  = _mm_srai_epi32(coeffsHighLow , shift);
        coeffsHighHigh = _mm_srai_epi32(coeffsHighHigh, shift);

        //32bit to 16bit
        coeffsLow = _mm_packs_epi32(coeffsLowLow, coeffsLowHigh);
        coeffsHigh = _mm_packs_epi32(coeffsHighLow, coeffsHighHigh);

        //store
        _mm_storeu_si128((__m128i*)(Array+y*size),   coeffsLow);
        _mm_storeu_si128((__m128i*)(Array+y*size+8), coeffsHigh);
      }
    }
  }
  else if(log2Size == 3)
  {
    for(y = 0; y < 8; y++)
    {
      {
        //load
        coeffsLow = _mm_loadu_si128((__m128i*)(Array+y*8));

        // 16bit to 32bit
        coeffsLowLow  = _mm_unpacklo_epi16(coeffsLow, coeffsLow);
        coeffsLowHigh = _mm_unpackhi_epi16(coeffsLow, coeffsLow);

        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , 16);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , 16);

        //dequantize
        // clippedQuantized * scale
        temp = _mm_set1_epi32((sint16)scale);
        coeffsLowLow   = _mm_mullo_epi32(coeffsLowLow   , temp);
        coeffsLowHigh  = _mm_mullo_epi32(coeffsLowHigh  , temp);

        // clippedQuantized * scale +roundAdd
        temp = _mm_set1_epi32(roundAdd);
        coeffsLowLow   = _mm_add_epi32(coeffsLowLow  , temp);
        coeffsLowHigh  = _mm_add_epi32(coeffsLowHigh , temp);

        // ( clippedQuantized * scale + roundAdd ) >> shift
        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , shift);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , shift);

        //32bit to 16bit
        coeffsLow = _mm_packs_epi32(coeffsLowLow, coeffsLowHigh);

        //store
        _mm_storeu_si128((__m128i*)(Array+y*8), coeffsLow);
      }
    }
  }
  else if(log2Size == 2)
  {
    for(y=0; y < 4; y += 2)
    {
      {
        //load
        coeffsLow = _mm_loadu_si128((__m128i*)(Array+y*4));

        // 16bit to 32bit
        coeffsLowLow = _mm_unpacklo_epi16(coeffsLow, coeffsLow);
        coeffsLowHigh = _mm_unpackhi_epi16(coeffsLow, coeffsLow);

        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , 16);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , 16);

        //dequantize
        // clippedQuantized * scale
        temp = _mm_set1_epi32((sint32)scale);
        coeffsLowLow   = _mm_mullo_epi32(coeffsLowLow   , temp);
        coeffsLowHigh  = _mm_mullo_epi32(coeffsLowHigh  , temp);

        // clippedQuantized * scale +roundAdd
        temp = _mm_set1_epi32(roundAdd);
        coeffsLowLow   = _mm_add_epi32(coeffsLowLow  , temp);
        coeffsLowHigh  = _mm_add_epi32(coeffsLowHigh , temp);

        // ( clippedQuantized * scale + roundAdd ) >> shift
        coeffsLowLow   = _mm_srai_epi32(coeffsLowLow  , shift);
        coeffsLowHigh  = _mm_srai_epi32(coeffsLowHigh , shift);

        //32bit to 16bit
        coeffsLow = _mm_packs_epi32(coeffsLowLow, coeffsLowHigh);

        //store
        _mm_storeu_si128((__m128i*)(Array+y*4), coeffsLow);
      }
    }
  }
#else
  for(y=0 ; y<(1<<(2*log2Size)) ; y++)
  {
    sint32 clippedQuantized = CLIP3(-32768, 32767, Array[y]);
    sint32 dequantized = (clippedQuantized * scale + roundAdd) >> shift;
    Array[y] = (sint16)CLIP3(-32768,32767,dequantized);
  }
#endif
}

/******************************************************************************
*
* Name:        performInverseTransform
*
******************************************************************************/
void
performInverseTransform(sint16 *TrafoCoeffs, 
                         const uint32 TrafoSize, 
                         uint32 TrafoSkipFlag,
                         uint8 DST)
{
  sint32 y;
  sint32 x;

  switch (TrafoSize)
  {
  case 4:
    if(TrafoSkipFlag)
    {
      for(y = 0; y < 4; y++)
      {
        for(x = 0; x < 4; x++) 
        {
          TrafoCoeffs[x] = (sint16)CLIP3(-32768, 32767, ((TrafoCoeffs[x] << 7)+(1 << 11))>>12);
        }
        TrafoCoeffs += 4;
      }
    }
    else
    {
      if(DST)
      {
        partialInverse4DST(TrafoCoeffs,      TrafoCoeffs + 16,  7);
        partialInverse4DST(TrafoCoeffs + 16, TrafoCoeffs,      12);
      }
      else
      {
        partialInverse4(TrafoCoeffs,      TrafoCoeffs + 16,  7);
        partialInverse4(TrafoCoeffs + 16, TrafoCoeffs,      12);
      }
    }
    break;
  case 8:
    partialInverse8(TrafoCoeffs,      TrafoCoeffs + 64,  7);
    partialInverse8(TrafoCoeffs + 64, TrafoCoeffs,      12);
    break;
  case 16:
    partialInverse16(TrafoCoeffs,       TrafoCoeffs + 256,  7);
    partialInverse16(TrafoCoeffs + 256, TrafoCoeffs,       12);
    break;
  case 32:
    partialInverse32(TrafoCoeffs,        TrafoCoeffs + 1024,  7);
    partialInverse32(TrafoCoeffs + 1024, TrafoCoeffs,        12);
    break;
  default:
    break;
  }

}
/******************************************************************************
*
* Name:        performReconstruction
*
******************************************************************************/
void
performReconstruction(sint16 *TrafoCoeffs,
                       uint8 *RecBlock_p,
                       const uint32 TrafoSize,
                       const uint32 RecPicWidth
                      )
{
  sint32 y;
  sint32 x;

  switch (TrafoSize)
  {
  case 4:
    for(y = 0; y < 4; y++)
    {
      for(x = 0; x < 4; x++) 
      {
        RecBlock_p[x] = (uint8)CLIP3(0,255,TrafoCoeffs[x] + RecBlock_p[x]);
      }
      TrafoCoeffs += 4;
      RecBlock_p  += RecPicWidth;
    }
    break;
  case 8:
#if SIMD
    //Add with prediction, clip and write back
    for(y = 0; y < 8; y++)
    {
      __m128i Pred_p = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)RecBlock_p),_mm_setzero_si128());
      __m128i TrafoCoeffs_p = _mm_loadu_si128((__m128i*)TrafoCoeffs);
      __m128i Rec_p = _mm_add_epi16(Pred_p, TrafoCoeffs_p);
      _mm_storel_epi64((__m128i*)RecBlock_p,_mm_packus_epi16(Rec_p,Rec_p));
      TrafoCoeffs += 8;
      RecBlock_p  += RecPicWidth;
    }
#else
    for(y = 0; y < 8; y++)
    {
      for(x = 0; x < 8; x++) 
      {
        RecBlock_p[x] = CLIP3(0,255,TrafoCoeffs[x] + RecBlock_p[x]);
      }
      TrafoCoeffs += 8;
      RecBlock_p  += RecPicWidth;
    }
#endif
    break;
  case 16:
#if SIMD
    //Add with prediction, clip and write back
    for(y = 0; y < 16; y++)
    {
      __m128i Pred_p     = _mm_loadu_si128((__m128i*)RecBlock_p);
      __m128i Zero =  _mm_setzero_si128();
      __m128i PredLow_p  = _mm_unpacklo_epi8(Pred_p,Zero);
      __m128i PredHigh_p = _mm_unpackhi_epi8(Pred_p,Zero);
      __m128i TrafoCoeffsLow_p  = _mm_loadu_si128((__m128i*)TrafoCoeffs);
      __m128i TrafoCoeffsHigh_p = _mm_loadu_si128((__m128i*)(TrafoCoeffs + 8));
      __m128i RecLow_p  = _mm_add_epi16(PredLow_p, TrafoCoeffsLow_p);
      __m128i RecHigh_p = _mm_add_epi16(PredHigh_p, TrafoCoeffsHigh_p);
      _mm_storeu_si128((__m128i*)RecBlock_p,_mm_packus_epi16(RecLow_p, RecHigh_p));
      RecBlock_p  += RecPicWidth;
      TrafoCoeffs += 16;
    }
#else
    for(y = 0; y < 16; y++)
    {
      for(x = 0; x < 16; x++) 
      {
        RecBlock_p[x] = CLIP3(0,255,TrafoCoeffs[x] + RecBlock_p[x]);
      }
      TrafoCoeffs += 16;
      RecBlock_p  += RecPicWidth;
    }
#endif
    break;
  case 32:
#if SIMD
    //Add with prediction, clip and write back
    for(y = 0; y < 32; y++)
    {
      for(x = 0; x < 32; x+=16)
      {
        __m128i Pred_p     = _mm_loadu_si128((__m128i*)(RecBlock_p + x));
        __m128i Zero =  _mm_setzero_si128();
        __m128i PredLow_p  = _mm_unpacklo_epi8(Pred_p,Zero);
        __m128i PredHigh_p = _mm_unpackhi_epi8(Pred_p,Zero);
        __m128i TrafoCoeffsLow_p  = _mm_loadu_si128((__m128i*)(TrafoCoeffs + x));
        __m128i TrafoCoeffsHigh_p = _mm_loadu_si128((__m128i*)(TrafoCoeffs + 8 + x));
        __m128i RecLow_p  = _mm_add_epi16(PredLow_p, TrafoCoeffsLow_p);
        __m128i RecHigh_p = _mm_add_epi16(PredHigh_p, TrafoCoeffsHigh_p);
        _mm_storeu_si128((__m128i*)(RecBlock_p + x),_mm_packus_epi16(RecLow_p, RecHigh_p));
        
      }
      RecBlock_p  += RecPicWidth;
      TrafoCoeffs += 32;
    }
#else
    for(y = 0; y < 32; y++)
    {
      for(x = 0; x < 32; x++) 
      {
        RecBlock_p[x] = CLIP3(0,255,TrafoCoeffs[x] + RecBlock_p[x]);
      }
      TrafoCoeffs += 32;
      RecBlock_p  += RecPicWidth;
    }
#endif
    break;
  default:
    break;
  }
}