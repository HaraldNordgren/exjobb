/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for inter prediction
*
******************************************************************************/
#include "d65_inter_prediction.h"

/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        CheckMVAndRefEquality
*
* Parameters:  Mv0    [in]  Motion Vector 0
*              Mv1    [in]  Motion Vector 1
*
* Returns:     uint8
*
* Description: Performs equality check on two motion vector and their reference idx
*
******************************************************************************/
uint8
CheckMVAndRefEquality(Vector_t Mv0, Vector_t Mv1)
{
  if( Mv0.x == Mv1.x &&
      Mv0.y == Mv1.y &&
      Mv0.ref_idx == Mv1.ref_idx )
  {
    return 1;
  }
  return 0;
}

/******************************************************************************
*
* Name:        CheckMVsEquality
*
* Parameters:  Mv0    [in]  Motion Vector 0
*              Mv1    [in]  Motion Vector 1
*
* Returns:     uint8
*
* Description: Performs equality check on two motion vector ignoring reference idx
*
******************************************************************************/
uint8
CheckMVsEquality(Vector_t Mv0, Vector_t Mv1)
{
  if( Mv0.x == Mv1.x &&
      Mv0.y == Mv1.y )
  {
    return 1;
  }
  return 0;
}

/******************************************************************************
*
* Name:        GetColMV
*
* Parameters:  Mv0    [in]  Motion Vector 0
*              Mv1    [in]  Motion Vector 1
*
* Returns:     uint8
*
* Description: Performs equality check on two motion vector
*
******************************************************************************/
Vector_t 
GetColMV(const ReferencePicture_t *CurrRefPic_p,
         const ReferencePicture_t *ColPic_p,
         const uint32              CurrPOC,
         const uint32              ColRefPicPOC,
         const PictureMarking_t    ColRefPicMarking,
         Vector_t                  ColMv,
         uint8                    *PredMvFound)
{
  const sint32 CurrDeltaPOC = CurrPOC - CurrRefPic_p->PicOrderCnt;
  const sint32 ColDeltaPOC  = ColPic_p->PicOrderCnt - ColRefPicPOC;
  const sint32 tb = CLIP3(-128, 127, CurrDeltaPOC);
  const sint32 td = CLIP3(-128, 127, ColDeltaPOC);
  const uint8 CurrRefPicIsLT = (CurrRefPic_p->PictureMarking == USED_FOR_LONG_TERM_REFERENCE);
  const uint8 ColRefPicIsLT  = (ColRefPicMarking == USED_FOR_LONG_TERM_REFERENCE);
  sint32 tx;
  sint32 distScaleFactor;
  sint32 Mvxy;
  Vector_t CurrMv = ColMv;

  if(ColRefPicIsLT == CurrRefPicIsLT)
  {
    if(!(ColRefPicIsLT || CurrRefPicIsLT))
    {
      if(td != tb)
      {
        if(td == 0)
        {
          return CurrMv;
        }
        // scale predicted mv if current mv and predicted mv has different reference index (assumes poc distance of 1 between referece indices)
        tx = (16384 + (abs(td) >> 1)) / td;
        distScaleFactor = CLIP3(-4096, 4095, (tb * tx + 32) >> 6);
        Mvxy = ColMv.x;
        CurrMv.x = (sint16)CLIP3(-32768, 32767, ((distScaleFactor * Mvxy) >= 0 ? 1 : -1) * ((abs(distScaleFactor * Mvxy) + 127) >> 8));
        Mvxy = ColMv.y;
        CurrMv.y = (sint16)CLIP3(-32768, 32767, ((distScaleFactor * Mvxy) >=0 ? 1 : -1) * ((abs(distScaleFactor * Mvxy) + 127) >> 8));
      }
    }
    *PredMvFound = 1;
  }
  return CurrMv;
}

/******************************************************************************
*
* Name:        ScaleMV
*
* Parameters:   CurrPicPOC      [in] POC of current picture
*               CurrRefPic      [in] Pointer to currently referenced picture
*               NeiborRefPic    [in] Pointer to picture referenced by neighbor
*               NeiborMv        [in] Motion vector of neighbor
*               PredMvFound     [out] Set to 1 if predicted mv found, 0 otherwise
*
* Returns:     Vector_t
*
* Description: Returns a scaled motion vector
*
******************************************************************************/
Vector_t
ScaleMV(const sint32 CurrPicPOC, const ReferencePicture_t * CurrRefPic, const ReferencePicture_t * NeiborRefPic, Vector_t NeiborMv, uint8 *PredMvFound)
{
  Vector_t CurrMv = NeiborMv;
  sint32 CurrDeltaPOC = CurrPicPOC - CurrRefPic->PicOrderCnt;
  sint32 NeiborDeltaPOC = CurrPicPOC - NeiborRefPic->PicOrderCnt;
  sint32 tb = CLIP3(-128, 127, CurrDeltaPOC);
  sint32 td = CLIP3(-128, 127, NeiborDeltaPOC);
  uint8 CurrRefPicIsLT = (CurrRefPic->PictureMarking == USED_FOR_LONG_TERM_REFERENCE);
  uint8 NeiborRefPicIsLT = (NeiborRefPic->PictureMarking == USED_FOR_LONG_TERM_REFERENCE);
  sint32 tx;
  sint32 distScaleFactor;
  sint32 Mvxy;

  if(NeiborRefPicIsLT == CurrRefPicIsLT)
  {
    if(!(NeiborRefPicIsLT || CurrRefPicIsLT))
    {
      if(td!=tb)
      {
        if(td == 0)
        {
          return CurrMv;
        }
        // scale predicted mv if current mv and predicted mv has different reference index (assumes poc distance of 1 between referece indices)
        tx = (16384 + (abs(td)>>1))/td;
        distScaleFactor = CLIP3(-4096,4095, (tb*tx+32)>>6);
        Mvxy = NeiborMv.x;
        CurrMv.x = (sint16)CLIP3(-32768, 32767, ((distScaleFactor*Mvxy) >=0? 1:-1)*((abs(distScaleFactor*Mvxy)+127)>>8));
        Mvxy = NeiborMv.y;
        CurrMv.y = (sint16)CLIP3(-32768, 32767, ((distScaleFactor*Mvxy) >=0? 1:-1)*((abs(distScaleFactor*Mvxy)+127)>>8));
      }
    }
    *PredMvFound = 1;
  }
  return CurrMv;
}

/******************************************************************************
*
* Name:         checkPredFlagAndPOCEqualityL0
*
* Parameters:   Sessiondata_p   [in] The decoder
*               candIdx         [in] Index of neighbor candidate in PU array
*               CurrRefPicPOC   [in] POC of current reference picture
*
* Returns:     uint8
*
* Description: Checks if neighbor PU uses MVL0 and if the POC of 
*              its ref pic equals POC of current ref pic
*
******************************************************************************/
uint8 
checkPredFlagAndPOCEqualityL0(const SessionData_t *Sessiondata_p,
                              const PredictionUnit_t * CandPU_p,
                              const sint32 CurrRefPicPOC)
{
  sint32 NeiborRefPicIdx = CandPU_p->MotionvectorL0.ref_idx;
  if(NeiborRefPicIdx != -1)
  {
    return (Sessiondata_p->RefPicList0[NeiborRefPicIdx]->PicOrderCnt == CurrRefPicPOC);
  }
  
  return 0;
}

/******************************************************************************
*
* Name:         checkPredFlagAndPOCEqualityL1
*
* Parameters:   Sessiondata_p   [in] The decoder
*               candIdx         [in] Index of neighbor candidate in PU array
*               CurrRefPicPOC   [in] POC of current reference picture
*
* Returns:     uint8
*
* Description: Checks if neighbor PU uses MVL1 and if the POC of 
*              its ref pic equals POC of current ref pic
*
******************************************************************************/
uint8 
checkPredFlagAndPOCEqualityL1(const SessionData_t *Sessiondata_p,
                              const PredictionUnit_t * CandPU_p,
                              const sint32 CurrRefPicPOC)
{
  sint32 NeiborRefPicIdx = CandPU_p->MotionvectorL1.ref_idx;
  if(NeiborRefPicIdx != -1)
  {
    return (Sessiondata_p->RefPicList1[NeiborRefPicIdx]->PicOrderCnt == CurrRefPicPOC);
  }
  
  return 0;
}

/******************************************************************************
*
* Name:         getMotionVectorL0
*
* Parameters:   PU      [in] The decoder
*
* Returns:      Vector_t
*
* Description:  Returns motion vector L0 of PU
*
******************************************************************************/
Vector_t getMotionVectorL0(const PredictionUnit_t * PU)
{
  return PU->MotionvectorL0;
}

/******************************************************************************
*
* Name:         getMotionVectorL1
*
* Parameters:   PU      [in] The decoder
*
* Returns:      Vector_t
*
* Description:  Returns motion vector L1 of PU
*
******************************************************************************/
Vector_t getMotionVectorL1(const PredictionUnit_t * PU)
{
  return PU->MotionvectorL1;
}

/******************************************************************************
*
* Name:         MotionCompensationLuma
*
* Parameters:   Sessiondata_p   [in] The decoder
*               xP              [in] PU x coordinate in pixels
*               yP              [in] PU y coordinate in pixels
*               nPbW            [in] PU width in pixels
*               nPbH            [in] PU height in pixels
*               RefPic          [in] Reference picture
*               Mv              [in] Motion vector
*
*               predSampleArray [out]
*               RecPic          [out]
*
* Returns:     void
*
* Description: Performs motion compensation for luma component and saves result in
*              predSampleArray or RecPic (the one which is non-NULL). RecPic is used for
*              uniprediction and predSampleArray for biprediction
*
******************************************************************************/
void 
MotionCompensationLuma(const SessionData_t *Sessiondata_p,
                       const uint32         xP,
                       const uint32         yP,
                       const uint32         nPbW,
                       const uint32         nPbH,
                       const Vector_t       Mv,
                       const uint8          UniPred,
                       const uint8         *RefPic,
                       sint16              *Interpl,
                       sint16              *predSampleArray
                       )
{

#if SIMD
  const static sint16 __declspec(align(16)) filterSimd[4][4][8] =
  {
    {
      {   0,   0,   0,   0,   0,   0,   0,   0 },
      {   0,  64,   0,  64,   0,  64,   0,  64 },
      {   0,   0,   0,   0,   0,   0,   0,   0 },
      {   0,   0,   0,   0,   0,   0,   0,   0 }
    },
    {
      {  -1,   4,  -1,   4,  -1,   4,  -1,   4 },
      { -10,  58, -10,  58, -10,  58, -10,  58 },
      {  17, - 5,  17,  -5,  17,  -5,  17,  -5 },
      {   1,   0,   1,   0,   1,   0,   1,   0 }
    },
    {
      {  -1,   4,  -1,   4,  -1,   4,  -1,   4 },
      { -11,  40, -11,  40, -11,  40, -11,  40 },
      {  40, -11,  40, -11,  40, -11,  40, -11 },
      {   4,  -1,   4,  -1,   4,  -1,   4,  -1 }
    },
    {
      {   0,   1,   0,   1,   0,   1,   0,   1 },
      {  -5,  17,  -5,  17,  -5,  17,  -5,  17 },
      {  58, -10,  58, -10,  58, -10,  58, -10 },
      {   4,  -1,   4,  -1,   4,  -1,   4,  -1 }
    }
  };
#endif

  const static sint32 filter[4][8] =
  {
    {  0, 0,   0, 64,  0,   0, 0,  0 },
    { -1, 4, -10, 58, 17,  -5, 1,  0 },
    { -1, 4, -11, 40, 40, -11, 4, -1 },
    {  0, 1,  -5, 17, 58, -10, 4, -1 }
  };

  const static sint32 filterMargin = 3;

  const sint32 PictureWidth  = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  const sint32 PictureHeight = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;
  const uint32 RecPicWidth   = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
  const sint32 xFrac = Mv.x&3;
  const sint32 yFrac = Mv.y&3;
  const uint8  *ptrRef;
  const uint8  *ptrRefTemp;
  uint8  *ptr;
  uint8 *RecPic = Sessiondata_p->CurrentRecImage->Image.Y;
  sint32 xInt   = Mv.x>>2;
  sint32 yInt   = Mv.y>>2;
  sint32 PredSum;
  uint32 y;
  uint32 x;
  uint32 i;
  sint32 DownLim;
  sint32 UpLim;

#if SIMD
  const __m128i offset = _mm_set1_epi16(32);
  const sint32  shift  = 6;
  __m128i source1,source2,sourceLow,sourceHigh,sourceLowAdd,sourceHighAdd;
  __m128i tempArrayLow,tempArrayHigh,tempArray1, tempArray2,tempArrayHigh1,tempArrayLow1;
  __m128i temp;
  __m128i Filter[4];
  __m128i sum;
  sint16  *InterplTemp;
#endif

  DownLim = -(sint32)nPbW - (sint32)xP - filterMargin;
  UpLim   =  PictureWidth - (sint32)xP + filterMargin;
  xInt    =  CLIP3(DownLim, UpLim, xInt);

  DownLim = -(sint32)nPbH  - (sint32)yP - filterMargin;
  UpLim   =  PictureHeight - (sint32)yP + filterMargin;
  yInt    =  CLIP3(DownLim, UpLim, yInt);

  /* Move ptrRef (and ptr) to correct location within the luma part of a YUVImage_t */
  ptrRef = RefPic + (yP+yInt+PADDING)*RecPicWidth + xP + xInt + PADDING;
  if(UniPred)
  {
    ptr = RecPic + (yP+PADDING)*RecPicWidth + xP + PADDING;
  }

  if(xFrac == 0 && yFrac == 0)
  {
#if SIMD
    if(!(nPbW & 0x0F))
    {
      if(UniPred) 
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 16)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRef + x));
            _mm_storeu_si128((__m128i*)(ptr + x), source1);
          }
          ptr    += RecPicWidth;
          ptrRef += RecPicWidth;
        }
      }
      else /* Write to predSampleArray */
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            temp = _mm_loadl_epi64((__m128i*)(ptrRef + x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArray + x), temp);
          }
          predSampleArray  += nPbW;
          ptrRef += RecPicWidth;
        }
      }
    }
    else if(!(nPbW & 0x07))
    {
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRef + x));
            _mm_storel_epi64((__m128i*)(ptr + x),source1);
          }
          ptr    += RecPicWidth;
          ptrRef += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            temp = _mm_loadl_epi64((__m128i*)(ptrRef + x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArray + x), temp);
          }
          predSampleArray  += nPbW;
          ptrRef += RecPicWidth;
        }
      }
    }
    else /* Too small PU size or non SIMD code */
    {
#endif
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          memcpy(ptr,ptrRef,nPbW*sizeof(uint8));
          ptr    += RecPicWidth;
          ptrRef += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            predSampleArray[x] = ptrRef[x] << 6;
          }
          predSampleArray    += nPbW;
          ptrRef += RecPicWidth;
        }
      }
#if SIMD
    }
#endif
  }
  else if(xFrac == 0)
  {
    ptrRef -= filterMargin*RecPicWidth;

#if SIMD
    if(!(nPbW & 0x07))
    {
      for(i = 0; i < 4; ++i)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[yFrac][i]);
      }

      for(y = 0; y < nPbH; y++)
      {
        for(x = 0; x < nPbW; x += 8)
        {
          ptrRefTemp = ptrRef + x;

          /* Load first two sets of samples to avoid cost for initializing to zero */
          /* Corresponds to i=0 and i=1 */
          source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);                // Load first 8 reference samples into lower bytes of source1
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));  // Load second 8 reference samples into lower bytes of source2
          source1 = _mm_cvtepu8_epi16(source1);                           // Convert from 8 bit to 16 bit
          source2 = _mm_cvtepu8_epi16(source2);                           // Convert from 8 bit to 16 bit
          sourceLow  = _mm_unpacklo_epi16(source1, source2);              // Interleave low bytes
          sourceHigh = _mm_unpackhi_epi16(source1, source2);              // Interleave high bytes
          sourceLowAdd  = _mm_madd_epi16(sourceLow, Filter[0]);           // Multiply and add pairwise
          sourceHighAdd = _mm_madd_epi16(sourceHigh, Filter[0]);          // Multiply and add pairwise

          /* Continue with the rest of the samples to be used in filtering */
          for(i = 2; i < 8; i += 2)
          {
            ptrRefTemp += 2*RecPicWidth;
            source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);                                      // Load next 8 reference samples into lower bytes of source1
            source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));                        // Load next 8 reference samples into lower bytes of source2
            source1 = _mm_cvtepu8_epi16(source1);                                                 // Convert from 8 bit to 16 bit
            source2 = _mm_cvtepu8_epi16(source2);                                                 // Convert from 8 bit to 16 bit
            sourceLow  = _mm_unpacklo_epi16(source1,source2);                                     // Interleave low bytes
            sourceHigh = _mm_unpackhi_epi16(source1,source2);                                     // Interleave high bytes
            sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[i/2]),sourceLowAdd);    // Multiply and add pairwise. Then add result for this i to sum
            sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[i/2]),sourceHighAdd);  // Multiply and add pairwise. Then add result for this i to sum
          }

          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);   // Pack together into sint16

          if(UniPred) /* Write to reconstructed picture */
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptr + x), sum);
          }
          else /* Store intermediate result in predSampleArray */
          {
            _mm_storeu_si128((__m128i*)(predSampleArray + x), sum);
          }
        }
        /* Move to next line */
        ptrRef += RecPicWidth;
        if(UniPred)
        {
          ptr += RecPicWidth;
        }
        else
        {
          predSampleArray += nPbW;
        }
      }
    }
    else
    {
#endif /* Too small PU size and non SIMD code */
      if(UniPred)
      {
        for(y = 0; y < (nPbH); y++)
        {
          PredSum = 0;
          for(x = 0; x < (nPbW); x++)
          {
            PredSum = 0;
            ptrRefTemp = ptrRef+x;
            for(i = 0; i < 8; i++)
            {
              PredSum += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptr[x] = (uint8)CLIP3(0,255,PredSum);
          }
          ptr += RecPicWidth;
          ptrRef += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < (nPbH); y++)
        {
          for(x = 0; x < (nPbW); x++)
          {
            predSampleArray[x] = 0;
            ptrRefTemp = ptrRef+x;
            for(i = 0; i < 8; i++)
            {
              predSampleArray[x] += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }
          }
          predSampleArray += nPbW;
          ptrRef += RecPicWidth;
        }
      }
#if SIMD
    }
#endif
  }
  else if(yFrac == 0)
  {
    ptrRef -= filterMargin;

#if SIMD
    if(!(nPbW & 0x07))
    {
      for(i = 0; i < 4; ++i)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[xFrac][i]);
      }

      for(y = 0; y < nPbH; y++)
      {
        for(x = 0; x < nPbW; x += 8)
        {
          /* Load first two sets of samples to avoid cost for initializing to zero */
          /* Corresponds to i=0 and i= 1 */
          ptrRefTemp = ptrRef + x;
          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));    // Load first 8 reference samples into lower bytes of source1
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));  // Load second 8 reference samples into lower bytes of source1
          source1 = _mm_cvtepu8_epi16(source1);                 // Convert from 8 bit to 16 bit
          source2 = _mm_cvtepu8_epi16(source2);                 // Convert from 8 bit to 16 bit
          sourceLow  = _mm_unpacklo_epi16(source1,source2);     // Interleave low bytes
          sourceHigh = _mm_unpackhi_epi16(source1,source2);     // Interleave high bytes
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);  // Multiply and add pairwise
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]); // Multiply and add pairwise

          /* Continue with the rest of the samples to be used in filtering */
          for(i = 2; i < 8; i += 2)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+i));                                  // Load next 8 reference samples into lower bytes of source1
            source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+i+1));                                // Load next 8 reference samples into lower bytes of source2
            source1 = _mm_cvtepu8_epi16(source1);                                                 // Convert from 8 bit to 16 bit
            source2 = _mm_cvtepu8_epi16(source2);                                                 // Convert from 8 bit to 16 bit
            sourceLow  = _mm_unpacklo_epi16(source1,source2);                                     // Interleave low samples
            sourceHigh = _mm_unpackhi_epi16(source1,source2);                                     // Interleave high samples
            sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[i/2]),sourceLowAdd);    // Multiply and add pairwise. Then add result for this i to sum
            sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[i/2]),sourceHighAdd);  // Multiply and add pairwise. Then add result for this i to sum
          }
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd); // Pack together into sint16

          if(UniPred)
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptr + x), sum);
          }
          else
          {
            _mm_storeu_si128((__m128i*)(predSampleArray + x), sum);
          }
          
        }
        /* Move to next line */
        ptrRef += RecPicWidth;
        if(UniPred)
        {
          ptr += RecPicWidth;
        }
        else
        {
          predSampleArray += nPbW;
        }
      }
    }
    else
    {
#endif
      if(UniPred)
      {
        for(y = 0; y < (nPbH); y++)
        {
          for(x= 0; x < (nPbW); x++)
          {
            PredSum = 0;
            ptrRefTemp = ptrRef + x;
            for(i = 0; i < 8; i++)
            {
              PredSum += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptr[x] = (uint8)CLIP3(0,255,PredSum);
          }
          ptr += RecPicWidth;
          ptrRef += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < (nPbH); y++)
        {
          for(x= 0; x < (nPbW); x++)
          {
            predSampleArray[x] = 0;
            ptrRefTemp = ptrRef + x;
            for(i = 0; i < 8; i++)
            {
              predSampleArray[x] += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }
          }
          predSampleArray += nPbW;
          ptrRef += RecPicWidth;
        }
      }
#if SIMD
    }
#endif
  }
  else //sub-pel in both directions
  {
    ptrRef -= filterMargin*RecPicWidth + filterMargin;
    
#if SIMD
    if(!(nPbW & 0x07))
    {
      /* Set up Filter array for horizontal filtering */

      for(i = 0; i < 4; ++i)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[xFrac][i]);
      }

      InterplTemp = Interpl;
      /* Horizontal filtering*/
      for(y = 0; y < (nPbH + 8); y++)
      {
        for(x = 0; x < (nPbW); x += 8) // Handle 8 samples per iteration
        {
          ptrRefTemp = ptrRef + x;                              // Move to correct column
          
          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));    // Load first 8 reference samples into lower bytes of source1
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));  // Load second 8 reference samples into lower bytes of source1
          source1 = _mm_cvtepu8_epi16(source1);                 // Convert from 8 bit to 16 bit
          source2 = _mm_cvtepu8_epi16(source2);                 // Convert from 8 bit to 16 bit
          sourceLow  = _mm_unpacklo_epi16(source1,source2);     // Interleave low bytes
          sourceHigh = _mm_unpackhi_epi16(source1,source2);     // Interleave high bytes
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);  // Multiply and add pairwise
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]); // Multiply and add pairwise

          /* Continue with the rest of the samples to be used in filtering */
          for(i = 2; i < 8; i += 2)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+i));                                  // Load next 8 reference samples into lower bytes of source1
            source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+i+1));                                // Load next 8 reference samples into lower bytes of source2
            source1 = _mm_cvtepu8_epi16(source1);                                                 // Convert from 8 bit to 16 bit
            source2 = _mm_cvtepu8_epi16(source2);                                                 // Convert from 8 bit to 16 bit
            sourceLow  = _mm_unpacklo_epi16(source1,source2);                                     // Interleave low samples
            sourceHigh = _mm_unpackhi_epi16(source1,source2);                                     // Interleave high samples
            sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[i/2]),sourceLowAdd);    // Multiply and add pairwise. Then add result for this i to sum
            sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[i/2]),sourceHighAdd);  // Multiply and add pairwise. Then add result for this i to sum
          }
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd); // Pack together into sint16
          _mm_storeu_si128((__m128i*)(InterplTemp+x),sum);  // Write 8 samples to Interpl[y*nPbW+x]
        }
        ptrRef += RecPicWidth;
        InterplTemp += nPbW;
      }

      /* Set up Filter array for vertical filtering */

      for(i = 0; i < 4; ++i)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[yFrac][i]);
      }

      /* Vertical filtering */
      for(y = 0; y < nPbH; y++)
      {
        for(x = 0; x < nPbW; x += 8)
        {
          InterplTemp = Interpl + x;
          tempArray1 = _mm_loadu_si128((__m128i*)(InterplTemp));
          
          InterplTemp += nPbW;
          tempArray2 = _mm_loadu_si128((__m128i*)(InterplTemp));

          tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
          tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

          tempArrayLow  = _mm_madd_epi16(tempArrayLow1,Filter[0]);
          tempArrayHigh = _mm_madd_epi16(tempArrayHigh1,Filter[0]);

          for(i = 2; i < 8; i += 2)
          {
            InterplTemp += nPbW;
            tempArray1 = _mm_loadu_si128((__m128i*)(InterplTemp));

            InterplTemp += nPbW;
            tempArray2 = _mm_loadu_si128((__m128i*)(InterplTemp));

            tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
            tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

            tempArrayLow  = _mm_add_epi32(_mm_madd_epi16(tempArrayLow1,Filter[i/2]),tempArrayLow);
            tempArrayHigh = _mm_add_epi32(_mm_madd_epi16(tempArrayHigh1,Filter[i/2]),tempArrayHigh);
          }
          tempArrayLow  = _mm_srai_epi32(tempArrayLow,shift);
          tempArrayHigh = _mm_srai_epi32(tempArrayHigh,shift);

          if(!UniPred)
          {
            sum = _mm_packs_epi32(tempArrayLow,tempArrayHigh);   // Pack together into sint16
            _mm_storeu_si128((__m128i*)(predSampleArray + x), sum);
          }
          else
          {
            sum = _mm_packs_epi32(tempArrayLow,tempArrayHigh);
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptr + x), sum);
          }
        }
        Interpl += nPbW;
        /* Move to next line */
        if(UniPred)
        {
          ptr += RecPicWidth;
        }
        else
        {
          predSampleArray += nPbW;
        }
      }
    }
    else
    {
#endif
      /* Non SIMD code or too small PU size */

      /* Horizontal filtering */
      for(y = 0; y < (nPbH + 8); y++)
      {
        for(x = 0; x < (nPbW); x++)
        {
          PredSum = 0;
          ptrRefTemp = ptrRef + x;
          for(i = 0; i < 8; i++)
          {
            PredSum += filter[xFrac][i]*ptrRef[x+i];
          }
          Interpl[y*nPbW+x] = (sint16)PredSum;
        }
        ptrRef += RecPicWidth;
      }

      /* Vertical filtering */
      if(UniPred)
      {
        for(y = 0; y < (nPbH); y++)
        {
          for(x = 0; x < (nPbW); x++)
          {
            PredSum = 0;
            for(i = 0; i < 8; i++)
            {
              PredSum += filter[yFrac][i]*Interpl[(y+i)*nPbW + x];
            }
            PredSum = (PredSum + (1<<11)) >> 12;
            ptr[x] = (uint8)CLIP3(0,255,PredSum);
          }
          ptr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < (nPbH); y++)
        {
          for(x = 0; x < (nPbW); x++)
          {
            PredSum = 0;
            for(i = 0; i < 8; i++)
            {
              PredSum += filter[yFrac][i]*Interpl[(y+i)*nPbW + x];
            }
            predSampleArray[x] = (sint16)(PredSum >> 6);
          }
          predSampleArray += nPbW;
        }
      }
    }
#if SIMD
  }
#endif
}

/******************************************************************************
*
* Name:         MotionCompensationChroma
*
* Parameters:   Sessiondata_p   [in] The decoder
*               xP              [in] PU x coordinate in pixels
*               yP              [in] PU y coordinate in pixels
*               nPbW            [in] PU width in pixels
*               nPbH            [in] PU height in pixels
*               RefPic          [in] Reference picture
*               Mv              [in] Motion vector
*
*               predSampleArray [out]
*               RecPic          [out]
*
* Returns:     void
*
* Description: Performs motion compensation for chroma components and saves result 
*              in predSampleArray or writes directly to RecPic
*
******************************************************************************/
void
MotionCompensationChroma(const SessionData_t *Sessiondata_p,
                         const uint32         xP,
                         const uint32         yP,
                         const uint32         nPbW,
                         const uint32         nPbH,
                         const Vector_t       Mv,
                         const uint8          UniPred,
                         uint8               *RefPicCb,
                         uint8               *RefPicCr,
                         sint16              *Interpl,
                         sint16              *predSampleArrayCb,
                         sint16              *predSampleArrayCr)
{

#if SIMD
  const static sint16 __declspec(align(16)) filterSimd[8][2][8] =
  {
    {
      {  0,  64,   0,  64,   0,  64,   0,  64 },
      {  0,   0,   0,   0,   0,   0,   0,   0 }
    },
    {
      { -2,  58,  -2,  58,  -2,  58,  -2,  58 },
      { 10,  -2,  10,  -2,  10,  -2,  10,  -2 }
    },
    {
      { -4,  54,  -4,  54,  -4,  54,  -4,  54 },
      { 16,  -2,  16,  -2,  16,  -2,  16,  -2 }
    },
    {
      { -6,  46,  -6,  46,  -6,  46,  -6,  46 },
      { 28,  -4,  28,  -4,  28,  -4,  28,  -4 }
    },
    {
      { -4,  36,  -4,  36,  -4,  36,  -4,  36 },
      { 36,  -4,  36,  -4,  36,  -4,  36,  -4 }
    },
    {
      { -4,  28,  -4,  28,  -4,  28,  -4,  28 },
      { 46,  -6,  46,  -6,  46,  -6,  46,  -6 }
    },
    {
      { -2,  16,  -2,  16,  -2,  16,  -2,  16 },
      { 54,  -4,  54,  -4,  54,  -4,  54,  -4 }
    },
    {
      { -2,  10,  -2,  10,  -2,  10,  -2,  10 },
      { 58,  -2,  58,  -2,  58,  -2,  58,  -2 }
    },
  };

#endif

  const static sint32 filter[8][4] =
  {
    {  0, 64,  0,  0 },
    { -2, 58, 10, -2 },
    { -4, 54, 16, -2 },
    { -6, 46, 28, -4 },
    { -4, 36, 36, -4 },
    { -4, 28, 46, -6 },
    { -2, 16, 54, -4 },
    { -2, 10, 58, -2 }
  };

  const static sint32 filterMargin = 1;

  const sint32 PictureWidth  = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureWidth  >> 1;
  const sint32 PictureHeight = (sint32)Sessiondata_p->CurrentSequenceParameterSet->PictureHeight >> 1;
  const uint32 RecPicWidth   = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
  const uint32 Offset        = (yP + (PADDING >> 1)) * RecPicWidth + xP + (PADDING >> 1);

  sint32    xFrac = Mv.x&7;
  sint32    yFrac = Mv.y&7;
  sint32    xInt = Mv.x>>3;
  sint32    yInt = Mv.y>>3;
  

  sint16    *InterplCb = Interpl;
  sint16    *InterplCr = Interpl + (MAX_PU_SIZE >> 1) * ((MAX_PU_SIZE >> 1) + 4);
  uint8     *ptrCb;
  uint8     *ptrCr;
  uint8     *ptrRefCb;
  uint8     *ptrRefCr;
  uint8     *ptrRefTemp;

  sint32    PredSum;
  sint32    DownLim;
  sint32    UpLim;
  uint32    y;
  uint32    x;
  uint32    i;

#if SIMD
  const __m128i offset = _mm_set1_epi16(32);
  const sint32 shift = 6;
  __m128i Filter[2];
  __m128i source1,source2,sourceLow,sourceHigh,sourceLowAdd,sourceHighAdd, sum;
  __m128i tempArrayLow,tempArrayHigh,tempArray1,tempArrayHigh1,tempArrayLow1;
  __m128i temp;
  __m128i tempArray2;
  sint16 *InterplCbTemp;
  sint16 *InterplCrTemp;
#endif

  DownLim = -(sint32)nPbW -(sint32)xP - filterMargin;
  UpLim   = PictureWidth  -(sint32)xP + filterMargin;
  xInt = CLIP3(DownLim, UpLim, xInt);

  DownLim = -(sint32)nPbH - (sint32)yP - filterMargin;
  UpLim   = PictureHeight - (sint32)yP + filterMargin;
  yInt = CLIP3(DownLim, UpLim, yInt);

  /* Move pointers to correct position in reference (and reconstructed) pictures */
  ptrRefCb = RefPicCb + Offset + yInt*RecPicWidth + xInt;
  ptrRefCr = RefPicCr + Offset + yInt*RecPicWidth + xInt;

  if(UniPred)
  {
    ptrCb = Sessiondata_p->CurrentRecImage->Image.Cb + Offset;
    ptrCr = Sessiondata_p->CurrentRecImage->Image.Cr + Offset;
  }

  if(xFrac == 0 && yFrac == 0)
  {
#if SIMD
    if(!(nPbW & 0x0F))
    {
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 16)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRefCb+x));
            _mm_storeu_si128((__m128i*)(ptrCb+x), source1);

            source1 = _mm_loadu_si128((__m128i*)(ptrRefCr+x));
            _mm_storeu_si128((__m128i*)(ptrCr+x), source1);
          }
          ptrCb    += RecPicWidth;
          ptrRefCb += RecPicWidth;
          ptrCr    += RecPicWidth;
          ptrRefCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            temp = _mm_loadl_epi64((__m128i*)(ptrRefCb+x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArrayCb + x), temp);

            temp = _mm_loadl_epi64((__m128i*)(ptrRefCr+x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArrayCr + x), temp);
          }
          predSampleArrayCb += nPbW;
          ptrRefCb += RecPicWidth;
          predSampleArrayCr += nPbW;
          ptrRefCr += RecPicWidth;
        }
      }
    }
    else if(!(nPbW & 0x07))
    {
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            source1 = _mm_loadu_si128((__m128i*)(ptrRefCb + x));
            _mm_storel_epi64 ((__m128i*)(ptrCb + x),source1);

            source1 = _mm_loadu_si128((__m128i*)(ptrRefCr + x));
            _mm_storel_epi64 ((__m128i*)(ptrCr + x),source1);
          }
          ptrCb    += RecPicWidth;
          ptrRefCb += RecPicWidth;
          ptrCr    += RecPicWidth;
          ptrRefCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x += 8)
          {
            temp = _mm_loadl_epi64((__m128i*)(ptrRefCb+x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArrayCb + x), temp);

            temp = _mm_loadl_epi64((__m128i*)(ptrRefCr+x));
            temp = _mm_cvtepu8_epi16(temp);
            temp = _mm_slli_epi16(temp, shift);
            _mm_storeu_si128((__m128i*)(predSampleArrayCr + x), temp);
          }
          predSampleArrayCb += nPbW;
          ptrRefCb += RecPicWidth;
          predSampleArrayCr += nPbW;
          ptrRefCr += RecPicWidth;
        }
      }
    }
    else
    {
#endif
      /* Non SIMD code or too small PU size */
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          memcpy(ptrCb,ptrRefCb,nPbW*sizeof(uint8));
          memcpy(ptrCr,ptrRefCr,nPbW*sizeof(uint8));
          ptrCb += RecPicWidth;
          ptrRefCb += RecPicWidth;
          ptrCr += RecPicWidth;
          ptrRefCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            predSampleArrayCb[x] = ptrRefCb[x] << 6;
            predSampleArrayCr[x] = ptrRefCr[x] << 6;
          }
          predSampleArrayCb += nPbW;
          ptrRefCb += RecPicWidth;
          predSampleArrayCr += nPbW;
          ptrRefCr += RecPicWidth;
        }
      }
#if SIMD
    }
#endif
  }
  else if(xFrac == 0)
  {
    ptrRefCb -= filterMargin*RecPicWidth;
    ptrRefCr -= filterMargin*RecPicWidth;

#if SIMD
    if(!(nPbW & 0x07))
    {
      for(i = 0; i < 2; i++)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[yFrac][i]);
      }

      for(y = 0; y < nPbH; y++)
      {
        /* Cb component */
        for(x = 0; x < nPbW; x+=8)
        {
          ptrRefTemp = ptrRefCb + x;
          source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1, source2);
          sourceHigh = _mm_unpackhi_epi16(source1, source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow, Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh, Filter[0]);
          
          ptrRefTemp += 2*RecPicWidth;
          source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCb + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCb + x), sum);
          }
        }

          /* Cr component */
        for(x = 0; x < nPbW; x+=8)
        {
          ptrRefTemp = ptrRefCr + x;
          source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1, source2);
          sourceHigh = _mm_unpackhi_epi16(source1, source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow, Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh, Filter[0]);
          
          ptrRefTemp += 2*RecPicWidth;
          source1 = _mm_loadu_si128((__m128i*)ptrRefTemp);
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+RecPicWidth));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCr + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCr + x), sum);
          }
        }
        ptrRefCb += RecPicWidth;
        ptrRefCr += RecPicWidth;
        if(UniPred)
        {
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
        }
        else
        {
          predSampleArrayCb += nPbW;
          predSampleArrayCr += nPbW;
        }
      }
    }
    else
    {
#endif
      /* Non SIMD code or too small PU size */
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            PredSum = 0;
            ptrRefTemp = ptrRefCb + x;
            for(i = 0; i < 4; i++)
            {
              PredSum += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptrCb[x] = (uint8)CLIP3(0,255,PredSum);

            PredSum = 0;
            ptrRefTemp = ptrRefCr + x;
            for(i = 0; i < 4; i++)
            {
              PredSum += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptrCr[x] = (uint8)CLIP3(0,255,PredSum);

          }
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
          ptrRefCb += RecPicWidth;
          ptrRefCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            predSampleArrayCb[x] = 0;
            ptrRefTemp = ptrRefCb + x;
            for(i = 0; i < 4; i++)
            {
              predSampleArrayCb[x] += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }

            predSampleArrayCr[x] = 0;
            ptrRefTemp = ptrRefCr + x;
            for(i = 0; i < 4; i++)
            {
              predSampleArrayCr[x] += (sint16)filter[yFrac][i]*ptrRefTemp[i*RecPicWidth];
            }
          }
          ptrRefCb += RecPicWidth;
          ptrRefCr += RecPicWidth;
          predSampleArrayCb += nPbW;
          predSampleArrayCr += nPbW;
        }
      }
#if SIMD
    }
#endif
  }
  else if(yFrac == 0)
  {
    ptrRefCb -= filterMargin;
    ptrRefCr -= filterMargin;

#if SIMD
    if(!(nPbW & 0x07))
    {
      for(i = 0; i < 2; i++)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[xFrac][i]);
      }

      for(y = 0; y < nPbH; y++)
      {
        for(x = 0; x < nPbW; x += 8)
        {
          ptrRefTemp = ptrRefCb + x;

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]);

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+2));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+3));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCb + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCb + x), sum);
          }
        }
       
        for(x = 0; x < nPbW; x += 8)
        {
          ptrRefTemp = ptrRefCr + x;

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]);

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+2));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+3));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCr + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCr + x), sum);
          }
        }

        ptrRefCb += RecPicWidth;
        ptrRefCr += RecPicWidth;
        if(UniPred)
        {
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
        }
        else
        {
          predSampleArrayCb += nPbW;
          predSampleArrayCr += nPbW;
        }
      }
    }
    else
    {
#endif
      /* Non SIMD code or too small PU size */
      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            PredSum = 0;
            ptrRefTemp = ptrRefCb + x;
            for(i = 0; i < 4; i++)
            {
              PredSum += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptrCb[x] = (uint8)CLIP3(0,255,PredSum);

            PredSum = 0;
            ptrRefTemp = ptrRefCr + x;
            for(i = 0; i < 4; i++)
            {
              PredSum += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }
            PredSum = (PredSum+(1<<5))>>6;
            ptrCr[x] = (uint8)CLIP3(0,255,PredSum);
          }
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
          ptrRefCb += RecPicWidth;
          ptrRefCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            predSampleArrayCb[x] = 0;
            ptrRefTemp = ptrRefCb + x;
            for(i = 0; i < 4; i++)
            {
              predSampleArrayCb[x] += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }

            predSampleArrayCr[x] = 0;
            ptrRefTemp = ptrRefCr + x;
            for(i = 0; i < 4; i++)
            {
              predSampleArrayCr[x] += (sint16)filter[xFrac][i]*ptrRefTemp[i];
            }
          }
          ptrRefCb += RecPicWidth;
          ptrRefCr += RecPicWidth;
          predSampleArrayCb += nPbW;
          predSampleArrayCr += nPbW;
        }
      }
#if SIMD
    }
#endif
  }
  else //sub-pel in both directions
  {
    ptrRefCb -= filterMargin*RecPicWidth + filterMargin;
    ptrRefCr -= filterMargin*RecPicWidth + filterMargin;

#if SIMD
    if(!(nPbW & 0x07))
    {
      for(i = 0; i < 2; i++)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[xFrac][i]);
      }

      InterplCbTemp = InterplCb;
      InterplCrTemp = InterplCr;
      /* Horizontal filtering */
      for(y=0; y < nPbH + 4; y++)
      {
        /* Cb component */
        for(x=0; x < nPbW; x += 8)
        {
          
          ptrRefTemp = ptrRefCb + x;

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]);

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+2));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+3));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          _mm_storeu_si128((__m128i*)(InterplCbTemp+x),sum);     // Store in InterplCb
        }

        /* Cr component */
        for(x=0; x < nPbW; x += 8)
        {
          ptrRefTemp = ptrRefCr + x;
          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+1));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_madd_epi16(sourceLow,Filter[0]);
          sourceHighAdd = _mm_madd_epi16(sourceHigh,Filter[0]);

          source1 = _mm_loadu_si128((__m128i*)(ptrRefTemp+2));
          source2 = _mm_loadu_si128((__m128i*)(ptrRefTemp+3));
          source1 = _mm_cvtepu8_epi16(source1);
          source2 = _mm_cvtepu8_epi16(source2);
          sourceLow  = _mm_unpacklo_epi16(source1,source2);
          sourceHigh = _mm_unpackhi_epi16(source1,source2);
          sourceLowAdd  = _mm_add_epi32(_mm_madd_epi16(sourceLow,Filter[1]),sourceLowAdd);
          sourceHighAdd = _mm_add_epi32(_mm_madd_epi16(sourceHigh,Filter[1]),sourceHighAdd);
          sum = _mm_packs_epi32(sourceLowAdd,sourceHighAdd);
          _mm_storeu_si128((__m128i*)(InterplCrTemp+x),sum);
        }

        ptrRefCb += RecPicWidth; // Move to next line
        ptrRefCr += RecPicWidth;
        InterplCbTemp += nPbW;
        InterplCrTemp += nPbW;
      }

      for(i = 0; i < 2; i++)
      {
        Filter[i] = _mm_load_si128((__m128i*)filterSimd[yFrac][i]);
      }

      /* Vertical filtering */
      for(y = 0; y < nPbH; y++)
      {
        /* Cb component */
        for(x = 0; x < nPbW; x += 8)
        {
          InterplCbTemp = InterplCb + x;
          tempArray1 = _mm_loadu_si128((__m128i*)(InterplCbTemp));

          InterplCbTemp += nPbW;
          tempArray2 = _mm_loadu_si128((__m128i*)(InterplCbTemp));

          tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
          tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

          tempArrayLow  = _mm_madd_epi16(tempArrayLow1,Filter[0]);
          tempArrayHigh = _mm_madd_epi16(tempArrayHigh1,Filter[0]);
          for(i = 2; i < 4; i += 2)
          {
            InterplCbTemp += nPbW;
            tempArray1 = _mm_loadu_si128((__m128i*)(InterplCbTemp));

            InterplCbTemp += nPbW;
            tempArray2 = _mm_loadu_si128((__m128i*)(InterplCbTemp));

            tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
            tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

            tempArrayLow  = _mm_add_epi32(_mm_madd_epi16(tempArrayLow1,Filter[i/2]),tempArrayLow);
            tempArrayHigh = _mm_add_epi32(_mm_madd_epi16(tempArrayHigh1,Filter[i/2]),tempArrayHigh);
          }

          tempArrayLow = _mm_srai_epi32(tempArrayLow,shift);
          tempArrayHigh = _mm_srai_epi32(tempArrayHigh,shift);

          sum = _mm_packs_epi32(tempArrayLow,tempArrayHigh);
          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCb + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCb + x), sum);
          }
        }

        /* Cr component */
        for(x = 0; x < nPbW; x += 8)
        {
          InterplCrTemp = InterplCr + x;
          tempArray1 = _mm_loadu_si128((__m128i*)(InterplCrTemp));

          InterplCrTemp += nPbW;
          tempArray2 = _mm_loadu_si128((__m128i*)(InterplCrTemp));

          tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
          tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

          tempArrayLow  = _mm_madd_epi16(tempArrayLow1,Filter[0]);
          tempArrayHigh = _mm_madd_epi16(tempArrayHigh1,Filter[0]);
          for(i = 2; i < 4; i += 2)
          {
            InterplCrTemp += nPbW;
            tempArray1 = _mm_loadu_si128((__m128i*)(InterplCrTemp));

            InterplCrTemp += nPbW;
            tempArray2 = _mm_loadu_si128((__m128i*)(InterplCrTemp));

            tempArrayLow1  = _mm_unpacklo_epi16(tempArray1,tempArray2);
            tempArrayHigh1 = _mm_unpackhi_epi16(tempArray1,tempArray2);

            tempArrayLow  = _mm_add_epi32(_mm_madd_epi16(tempArrayLow1,Filter[i/2]),tempArrayLow);
            tempArrayHigh = _mm_add_epi32(_mm_madd_epi16(tempArrayHigh1,Filter[i/2]),tempArrayHigh);
          }
 
          tempArrayLow = _mm_srai_epi32(tempArrayLow,shift);
          tempArrayHigh = _mm_srai_epi32(tempArrayHigh,shift);

          sum = _mm_packs_epi32(tempArrayLow,tempArrayHigh);

          if(!UniPred)
          {
            _mm_storeu_si128((__m128i*)(predSampleArrayCr + x), sum);
          }
          else
          {
            sum = _mm_add_epi16(sum,offset);
            sum = _mm_srai_epi16(sum,shift);
            sum = _mm_packus_epi16(sum,sum);
            _mm_storel_epi64((__m128i*)(ptrCr + x), sum);
          }
        }

        InterplCb += nPbW;
        InterplCr += nPbW;

        if(UniPred)
        {
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
        }
        else
        {
          predSampleArrayCb += nPbW;  // Move to next line
          predSampleArrayCr += nPbW;
        }
      }
    }
    else
    {
#endif
      /* Non SIMD code or too small PU size */
      for(y = 0; y < nPbH + 4; y++)
      {
        for(x = 0; x < nPbW; x++)
        {
          PredSum = 0;
          ptrRefTemp = ptrRefCb + x;
          for(i = 0; i < 4; i++)
          {
            PredSum += filter[xFrac][i]*ptrRefTemp[i];
          }
          InterplCb[y*nPbW+x] = (sint16)PredSum;

          PredSum = 0;
          ptrRefTemp = ptrRefCr + x;

          for(i = 0; i < 4; i++)
          {
            PredSum += filter[xFrac][i]*ptrRefTemp[i];
          }
          InterplCr[y*nPbW+x] = (sint16)PredSum;
        }
        ptrRefCb += RecPicWidth;
        ptrRefCr += RecPicWidth;
      }

      if(UniPred)
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            PredSum = 0;
            for(i = 0; i < 4; i++)
            {
              PredSum += filter[yFrac][i]*InterplCb[(y+i)*nPbW+x];
            }
            
            PredSum = (PredSum+(1<<11))>>12;
            ptrCb[x] = (uint8)CLIP3(0,255,PredSum);
            
            PredSum = 0;
            for(i = 0; i < 4; i++)
            {
              PredSum += filter[yFrac][i]*InterplCr[(y+i)*nPbW+x];
            }
            PredSum = (PredSum+(1<<11))>>12;
            ptrCr[x] = (uint8)CLIP3(0,255,PredSum);

          }
          ptrCb += RecPicWidth;
          ptrCr += RecPicWidth;
        }
      }
      else
      {
        for(y = 0; y < nPbH; y++)
        {
          for(x = 0; x < nPbW; x++)
          {
            PredSum = 0;
            for(i = 0; i < 4; i++)
            {
              PredSum += filter[yFrac][i]*InterplCb[(y+i)*nPbW+x];
            }
            
            predSampleArrayCb[x] = (sint16)(PredSum >> 6);
            
            PredSum = 0;
            for(i = 0; i < 4; i++)
            {
              PredSum += filter[yFrac][i]*InterplCr[(y+i)*nPbW+x];
            }
            
            predSampleArrayCr[x] = (sint16)(PredSum >> 6);
          }
          predSampleArrayCb += nPbW;
          predSampleArrayCr += nPbW;
        }
      }
#if SIMD
    }
#endif
  }
}



/******************************************************************************
*
* Name:         WriteAvgToCurrentRecImg
*
* Parameters:   nPbW                [in] PU width in pixels
*               nPbH                [in] PU height in pixels
*               RecPicWidth         [in]
*               predSampleArrayL0   [in] Prediction sample array for list L0
*               predSampleArrayL1   [in] Prediction sample array for list L1
*
*               RecImgPtr           [out] Pointer to current PU in reconstructed image
*
* Returns:      void
*
* Description:  Write average of predSampleArrays to RecImgPtr
*
******************************************************************************/
void 
WriteAvgToCurrentRecImg(const uint32 nPbW,
                        const uint32 nPbH,
                        const uint32 RecPicWidth,
                        const sint16 *predSampleArrayL0,
                        const sint16 *predSampleArrayL1,
                        uint8 *RecImgPtr)
{
  sint32 PredSum;
  uint32 y;
  uint32 x;

#if SIMD
  __m128i srcL0, srcL1, sum, aa, bb,cc,dd, sum1, sum2;
  __m128i offset = _mm_set1_epi32(1<<6); 
  
  if(!(nPbW  & 0x0F))
  {
    for(y =0 ; y < nPbH; y++)
    {
      for(x = 0; x < nPbW; x+=16)
      {
        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + x));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + x));

        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);

        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        aa = _mm_srai_epi32(sum, 7);

        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + x + 4));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + x + 4));

        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);

        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        bb = _mm_srai_epi32(sum, 7);
        sum = _mm_packus_epi32(aa,bb);

        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + x + 8));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + x + 8));
        
        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);
        
        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        cc = _mm_srai_epi32(sum, 7);

        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + x + 12));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + x + 12));
        
        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);
        
        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        dd = _mm_srai_epi32(sum, 7);

        sum1 = _mm_packus_epi32(aa,bb);
        sum2 = _mm_packus_epi32(cc,dd);

        sum = _mm_packus_epi16(sum1,sum2);

        _mm_storeu_si128((__m128i*)(RecImgPtr + x), sum);
      }
      predSampleArrayL0 += nPbW;
      predSampleArrayL1 += nPbW;
      RecImgPtr += RecPicWidth;
    }
  }
  else if(!(nPbW & 0x07))
  {
    for(y =0 ; y < nPbH; y++)
    {
      for(x = 0; x < nPbW; x+= 8)
      {
        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + x));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + x));

        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);

        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        aa = _mm_srai_epi32(sum, 7);

        srcL0 = _mm_loadu_si128((__m128i*)(predSampleArrayL0 + 4 + x));
        srcL1 = _mm_loadu_si128((__m128i*)(predSampleArrayL1 + 4 + x));

        srcL0 = _mm_cvtepi16_epi32(srcL0);
        srcL1 = _mm_cvtepi16_epi32(srcL1);

        sum = _mm_add_epi32(srcL0, srcL1);
        sum = _mm_add_epi32(sum, offset);
        bb = _mm_srai_epi32(sum, 7);

        sum = _mm_packus_epi32(aa,bb);
        sum = _mm_packus_epi16(sum,sum);

        _mm_storel_epi64((__m128i*)(RecImgPtr + x), sum);
      }
   
      predSampleArrayL0 += nPbW;
      predSampleArrayL1 += nPbW;
      RecImgPtr += RecPicWidth;
    }
  }
  else
  {
#endif
    for(y = 0; y < nPbH; y++)
    {
      for(x = 0; x < nPbW; x++)
      {
        PredSum = (predSampleArrayL0[y*nPbW + x] + predSampleArrayL1[y*nPbW + x] + (1<<6)) >> 7;
        RecImgPtr[x] = (uint8)CLIP3(0,255,PredSum);
      }
      RecImgPtr += RecPicWidth;
    }
#if SIMD
  }
#endif
}

/******************************************************************************
*
* Name:         AllRefPicPOCDiffNegative
*
* Parameters:   CurrPoc             [in] POC of current picture
*               RefPicList          [in] A reference picture list
*               maxIdx              [in] Maximal active index in RefPicList
*
* Returns:      uint8
*
* Description:  Returns 1 if all POC-differences between the POC of
*               each element in RefPicList and currPOC less than or equal to 0. 
*               Otherwise returns 0.
*
******************************************************************************/
uint8 
AllRefPicPOCDiffNegative(const sint32 CurrPOC,
                         const ReferencePicture_t **RefPicList,
                         int maxIdx)
{
  sint32 idx;
  for(idx = 0; idx < maxIdx; idx++)
  {
    if(RefPicList[idx])
    {
      if(RefPicList[idx]->PicOrderCnt > CurrPOC)
      {
        return 0;
      }
    }
  }
  return 1;
}

/******************************************************************************
*
* Name:         GetTemporalMVLX
*
* Parameters:   Sessiondata_p       [in] The decoder
*               xP                  [in] Position in pixels
*               yP                  [in] Position in pixels
*               nPbW                [in] PU width in pixels
*               nPbH                [in] PU height in pixels
*               ListX               [in] Reference list for which TMV is 
*                                        derived (0 or 1)
*
* Returns:      Vector_t
*
* Description:  Derive collocated vector for list ListX. Returns vector with 
*               ref_idx = -1 if not available.
*
******************************************************************************/
Vector_t GetTemporalMVLX(SessionData_t *Sessiondata_p,
                         const uint32 refIdxLX,
                         const uint32 xP,
                         const uint32 yP,
                         const uint32 nPbW,
                         const uint32 nPbH,
                         const uint8 ListX)
{
  const uint32 Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 Log2CtbSizeY = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const uint32 PicHeightInLuma = Sessiondata_p->CurrentSequenceParameterSet->PictureHeight;
  const uint32 PicWidthInLuma = Sessiondata_p->CurrentSequenceParameterSet->PictureWidth;
  const uint32 PicWidthInPU = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  uint32 xPcolInPuAddr;
  uint32 yPcolInPuAddr;
  ReferencePicture_t **RefPicListX;
  Vector_t ColMv;
  Vector_t TmpMv = {0,0,-1};
  uint32 yNeibor = yP + nPbH;
  uint32 xNeibor = xP + nPbW;
  ReferencePicture_t *ColPic_p;
  uint32 ColPUIndices[2];
  uint32 ColPUIdx;
  uint32 ColRefPicPOC;
  PictureMarking_t ColRefPicMarking;
  uint8 ColMVFound = 0;
  uint32 numAvailable = 0;
  uint32 i;

  RefPicListX = (ListX == 0) ? Sessiondata_p->RefPicList0 : Sessiondata_p->RefPicList1;

  if(Sessiondata_p->CurrentSliceType == MODE_B && !Sessiondata_p->CurrentCollocatedFromL0Flag)
  {
    ColPic_p = Sessiondata_p->RefPicList1[Sessiondata_p->CurrentCollocatedRefIdx];
  }
  else
  {
    ColPic_p = Sessiondata_p->RefPicList0[Sessiondata_p->CurrentCollocatedRefIdx];
  }

  if(ColPic_p == NULL || ColPic_p->PictureMarking == UNUSED_FOR_REFERENCE)
  {
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "Referenced picture marked unused for reference");
  }

  /* Derive available collocated PU indices */
  if(((yNeibor >> Log2CtbSizeY) == (yP >> Log2CtbSizeY) )&& 
    yNeibor < PicHeightInLuma && xNeibor < PicWidthInLuma)
  {
    /* Bottom right collocated */
    xPcolInPuAddr = ((xNeibor >> 4) << 4)>> Log2PUSizeInPixels;
    yPcolInPuAddr = ((yNeibor >> 4) << 4)>> Log2PUSizeInPixels;
    ColPUIdx = yPcolInPuAddr*PicWidthInPU + xPcolInPuAddr;
    ColPUIndices[numAvailable++] = ColPUIdx;
  }

  /* Central collocated */
  yNeibor = yP + (nPbH >> 1);
  xNeibor = xP + (nPbW >> 1);
  xPcolInPuAddr = ((xNeibor >> 4) << 4)>> Log2PUSizeInPixels;
  yPcolInPuAddr = ((yNeibor >> 4) << 4)>> Log2PUSizeInPixels;
  ColPUIdx = yPcolInPuAddr*PicWidthInPU + xPcolInPuAddr;
  ColPUIndices[numAvailable++] = ColPUIdx;

  for(i = 0; i < numAvailable; i++)
  {
    ColPUIdx = ColPUIndices[i];

    if(ColPic_p->PredMode[ColPUIdx] == MODE_INTER)
    {
      if(ColPic_p->MotionVectorL0[ColPUIdx].ref_idx == -1)
      {
        ColMv = ColPic_p->MotionVectorL1[ColPUIdx];
        ColRefPicPOC = ColPic_p->RefPicPOCList1[ColMv.ref_idx];
        ColRefPicMarking = ColPic_p->RefPicMarkingList1[ColMv.ref_idx];
      }
      else if(ColPic_p->MotionVectorL0[ColPUIdx].ref_idx != -1 &&
              ColPic_p->MotionVectorL1[ColPUIdx].ref_idx == -1)
      {
        ColMv = ColPic_p->MotionVectorL0[ColPUIdx];
        ColRefPicPOC = ColPic_p->RefPicPOCList0[ColMv.ref_idx];
        ColRefPicMarking = ColPic_p->RefPicMarkingList0[ColMv.ref_idx];
      }
      else
      {
        if(AllRefPicPOCDiffNegative(Sessiondata_p->PicOrderCntVal, Sessiondata_p->RefPicList0, Sessiondata_p->Current_num_ref_idx_l0_active_minus1+1) &&
           AllRefPicPOCDiffNegative(Sessiondata_p->PicOrderCntVal, Sessiondata_p->RefPicList1, Sessiondata_p->Current_num_ref_idx_l1_active_minus1+1))
        {
          if(ListX == 0)
          {
            ColMv = ColPic_p->MotionVectorL0[ColPUIdx];
            ColRefPicPOC = ColPic_p->RefPicPOCList0[ColMv.ref_idx];
            ColRefPicMarking = ColPic_p->RefPicMarkingList0[ColMv.ref_idx];
          }
          else
          {
            ColMv = ColPic_p->MotionVectorL1[ColPUIdx];
            ColRefPicPOC = ColPic_p->RefPicPOCList1[ColMv.ref_idx];
            ColRefPicMarking = ColPic_p->RefPicMarkingList1[ColMv.ref_idx];
          }
        }
        else
        {
          if(Sessiondata_p->CurrentCollocatedFromL0Flag == 1)
          {
            ColMv = ColPic_p->MotionVectorL1[ColPUIdx];
            ColRefPicPOC = ColPic_p->RefPicPOCList1[ColMv.ref_idx];
            ColRefPicMarking = ColPic_p->RefPicMarkingList1[ColMv.ref_idx];
          }
          else
          {
            ColMv = ColPic_p->MotionVectorL0[ColPUIdx];
            ColRefPicPOC = ColPic_p->RefPicPOCList0[ColMv.ref_idx];
            ColRefPicMarking = ColPic_p->RefPicMarkingList0[ColMv.ref_idx];
          }
        }
      }

      TmpMv = GetColMV(RefPicListX[refIdxLX], ColPic_p, Sessiondata_p->PicOrderCntVal, ColRefPicPOC, ColRefPicMarking, ColMv, &ColMVFound);

      if(ColMVFound)
      {
        TmpMv.ref_idx = (uint8)refIdxLX;
        return TmpMv;
      }
    }
  }

  return TmpMv;
}


/******************************************************************************
* Definition of external functions
******************************************************************************/

#ifndef min
#define min(a,b) a>b? b : a
#endif


/******************************************************************************
*
* Name:        GetMotionMergeCandidateP 
*
******************************************************************************/
void 
GetMotionMergeCandidateP(SessionData_t *Sessiondata_p, 
                         const uint32 xP, 
                         const uint32 yP, 
                         const uint32 nPbW, 
                         const uint32 nPbH, 
                         const uint8 MergeIdx,
                         const uint8 PartIdx,
                         const PartitionMode_t PartMode,
                         Vector_t *MotionvectorL0_p,
                         Vector_t *MotionvectorL1_p)
{
  uint32      Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  uint32      PicWidthInPU       = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  uint8       Log2ParMrgLevel    = Sessiondata_p->CurrentPictureParameterSet->log2_parallel_merge_level;
  uint32      xPInPUAddr = xP >> Log2PUSizeInPixels;
  uint32      yPInPUAddr = yP >> Log2PUSizeInPixels;
  uint32      BlockWidthInPU = nPbW >> Log2PUSizeInPixels;
  uint32      BlockHeightInPU = nPbH >> Log2PUSizeInPixels;
  uint32      xPInMrgLevel    = xP   >> Log2ParMrgLevel;
  uint32      yPInMrgLevel    = yP   >> Log2ParMrgLevel;
  uint32      QualifiedCandCount;
  Vector_t    MvL0 = {0, 0, -1};
  Vector_t    MvL1 = {0, 0, -1};
  Vector_t    Mv;
  sint32      RefPicIdxL0;
  sint32      zeroIdx;
  sint32      numRefIdx;

  //Pointers to neighboring prediction units
  //
  //Note that if PB size is greater than 4x4
  //one needs to locate the correct PredictionUnit_t:s 
  //within the current PB to check neighbor availability.
  //For A1 and PB size 8 we need the left neighbor of 
  //the PredictionUnit_t below PU
  //
  //         _ _         _ _ _ _   
  //        |B2 |       |B1 |B0 |  
  //        |_ _|_ _ _ _|_ _|_ _|  
  //            |PU         |      
  //            |           |      
  //         _ _|           |      
  //        |A1 |           |      
  //        |_ _|_ _ _ _ _ _|      
  //        |A0 |                  
  //        |_ _|                  
  //                               
  const PredictionUnit_t *PU_p = Sessiondata_p->PredictionUnitArray + yPInPUAddr*PicWidthInPU + xPInPUAddr;
  const PredictionUnit_t *A1_p;
  const PredictionUnit_t *A0_p;
  const PredictionUnit_t *B0_p;
  const PredictionUnit_t *B1_p;
  const PredictionUnit_t *B2_p;

  uint8 NeiborAvailableA1;
  uint8 NeiborAvailableA0;

  uint8 NeiborAvailableB1;
  uint8 NeiborAvailableB0;
  uint8 NeiborAvailableB2;

  uint8 A1InSameCU = (PartIdx == 1) && (PartMode == PART_Nx2N || PartMode == PART_nLx2N || PartMode == PART_nRx2N);
  uint8 B1InSameCU = (PartIdx == 1) && (PartMode == PART_2NxN || PartMode == PART_2NxnU || PartMode == PART_2NxnD);

  QualifiedCandCount = MergeIdx;

  // check position A1
  A1_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitLeft_p;
  NeiborAvailableA1 = A1_p ? ((A1_p->PredMode == MODE_INTER) && !A1InSameCU && 
                             ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP + nPbH - 1) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableA1)
  {
    MvL0 = A1_p->MotionvectorL0;
    if(QualifiedCandCount == 0)
    {
      *MotionvectorL0_p = MvL0;
      *MotionvectorL1_p = MvL1;
      return;
    }
    else
    {
      QualifiedCandCount--;
    }
  } 

  // check position B1
  B1_p = PU_p[BlockWidthInPU-1].PredictionUnitUp_p;
  NeiborAvailableB1 = B1_p ? ((B1_p->PredMode == MODE_INTER) && !B1InSameCU &&
                             ((xPInMrgLevel != ((xP + nPbW - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableB1)
  {
    if(!(NeiborAvailableA1 &&
         CheckMVAndRefEquality(B1_p->MotionvectorL0, A1_p->MotionvectorL0)))
    {
      MvL0 = B1_p->MotionvectorL0;
      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;
        return;
      }
      else
      {
        QualifiedCandCount--;
      }
    }
  }

  // check position B0 
  B0_p = PU_p[BlockWidthInPU-1].PredictionUnitUpRight_p;
  NeiborAvailableB0 = B0_p ? ((B0_p->PredMode == MODE_INTER) &&
                             ((xPInMrgLevel != ((xP + nPbW) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableB0)
  {
    if(!(NeiborAvailableB1 &&
         CheckMVAndRefEquality(B0_p->MotionvectorL0, B1_p->MotionvectorL0)))
    {
      MvL0 = B0_p->MotionvectorL0;

      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;

        return;
      }
      else
      {
        QualifiedCandCount--;
      }
    }
  }

  // check position A0
  A0_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitDownLeft_p;
  NeiborAvailableA0 = A0_p ? ((A0_p->PredMode == MODE_INTER) &&
                             ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP + nPbH) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableA0)
  {
    if(!(NeiborAvailableA1 &&
         CheckMVAndRefEquality(A0_p->MotionvectorL0, A1_p->MotionvectorL0)))
    {
      MvL0 = A0_p->MotionvectorL0;
      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;
        return;
      }
      else
      {
        QualifiedCandCount--;
      }
    }
  }
  
  if((QualifiedCandCount + 4) != MergeIdx)
  {
    // check position B2
    B2_p = PU_p->PredictionUnitUpLeft_p;
    NeiborAvailableB2 = B2_p ? ((B2_p->PredMode == MODE_INTER) &&
                               ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;
    if(NeiborAvailableB2)
    {
      if(!(NeiborAvailableA1 && 
           CheckMVAndRefEquality(B2_p->MotionvectorL0, A1_p->MotionvectorL0)) 
        && 
         !(NeiborAvailableB1 && 
           CheckMVAndRefEquality(B2_p->MotionvectorL0, B1_p->MotionvectorL0)))
      {
          MvL0 = B2_p->MotionvectorL0;
        if(QualifiedCandCount == 0)
        {
          *MotionvectorL0_p = MvL0;
          *MotionvectorL1_p = MvL1;
          return;
        }
        else
        {
          QualifiedCandCount--;
        }
      }
    }
  }
  
  //get temporal motion vector (not used by c65 in B-slices)
  if(Sessiondata_p->CurrentCollocatedRefIdx != -1)
  {
    MvL0 = GetTemporalMVLX(Sessiondata_p, 0, xP, yP, nPbW, nPbH, 0); /* 0 for list 0 */
    if(MvL0.ref_idx != -1)
    {
      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;
        return;
      }
      else
      {
        QualifiedCandCount--;
      }
    }
  }

  zeroIdx = 0;
  Mv.x = 0;
  Mv.y = 0;
  numRefIdx = Sessiondata_p->Current_num_ref_idx_l0_active_minus1+1;
  while(QualifiedCandCount >= 0)
  {
    //add zero merge candidate
    RefPicIdxL0 = (zeroIdx < (numRefIdx)) ? zeroIdx : 0;
    if(QualifiedCandCount == 0)
    {
      break;
    }
    QualifiedCandCount--;
    zeroIdx++;
  }

  *MotionvectorL0_p = Mv;
  MotionvectorL0_p->ref_idx = (sint8)RefPicIdxL0;
  *MotionvectorL1_p = MvL1;
}


/******************************************************************************
*
* Name:        GetMotionMergeCandidateB 
*
******************************************************************************/
void 
GetMotionMergeCandidateB(SessionData_t *Sessiondata_p, 
                         const uint32 xP, 
                         const uint32 yP, 
                         const uint32 nPbW, 
                         const uint32 nPbH, 
                         const uint8 MergeIdx,
                         const uint8 PartIdx,
                         const PartitionMode_t PartMode,
                         Vector_t *MotionvectorL0_p,
                         Vector_t *MotionvectorL1_p)
{
  typedef struct
  {
    Vector_t MotionvectorL0;
    Vector_t MotionvectorL1;
  } Candidate_t;

  static const sint8 l0CandIdx[] = {0,1,0,2,1,2,0,3,1,3,2,3};
  static const sint8 l1CandIdx[] = {1,0,2,0,2,1,3,0,3,1,3,2};

  uint32      Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  uint32      PicWidthInPU       = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  uint8       Log2ParMrgLevel    = Sessiondata_p->CurrentPictureParameterSet->log2_parallel_merge_level;

  uint32      xPInPUAddr      = xP   >> Log2PUSizeInPixels;
  uint32      yPInPUAddr      = yP   >> Log2PUSizeInPixels;
  uint32      BlockWidthInPU  = nPbW >> Log2PUSizeInPixels;
  uint32      BlockHeightInPU = nPbH >> Log2PUSizeInPixels;
  uint32      xPInMrgLevel    = xP   >> Log2ParMrgLevel;
  uint32      yPInMrgLevel    = yP   >> Log2ParMrgLevel;

  uint32      QualifiedCandCount;
  Vector_t    MvL0 = {0, 0, -1};
  Vector_t    MvL1 = {0, 0, -1};
  Vector_t    Mv;
  Candidate_t mergeCandList[4];
  uint32      numCand = 0;
  sint32      RefPicIdxL0;
  sint32      RefPicIdxL1;
  sint32      zeroIdx;
  sint32      numRefIdx;

  //Pointers to neighboring prediction units
  //
  //Note that if PB size is greater than 4x4
  //one needs to locate the correct PredictionUnit_t:s 
  //within the current PB to check neighbor availability.
  //For A1 and PB size 8 we need the left neighbor of 
  //the PredictionUnit_t below PU
  //
  //         _ _         _ _ _ _   
  //        |B2 |       |B1 |B0 |  
  //        |_ _|_ _ _ _|_ _|_ _|  
  //            |PU         |      
  //            |           |      
  //         _ _|           |      
  //        |A1 |           |      
  //        |_ _|_ _ _ _ _ _|      
  //        |A0 |                  
  //        |_ _|                  
  //                               
  const PredictionUnit_t *PU_p = Sessiondata_p->PredictionUnitArray + yPInPUAddr*PicWidthInPU + xPInPUAddr;
  const PredictionUnit_t *A1_p;
  const PredictionUnit_t *A0_p;
  const PredictionUnit_t *B0_p;
  const PredictionUnit_t *B1_p;
  const PredictionUnit_t *B2_p;

  uint8 NeiborAvailableA1;
  uint8 NeiborAvailableA0;

  uint8 NeiborAvailableB1;
  uint8 NeiborAvailableB0;
  uint8 NeiborAvailableB2;

  uint8 A1InSameCU = (PartIdx == 1) && (PartMode == PART_Nx2N || PartMode == PART_nLx2N || PartMode == PART_nRx2N);
  uint8 B1InSameCU = (PartIdx == 1) && (PartMode == PART_2NxN || PartMode == PART_2NxnU || PartMode == PART_2NxnD);

  QualifiedCandCount = MergeIdx;

  // check position A1
  A1_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitLeft_p;
  NeiborAvailableA1 = A1_p ? ((A1_p->PredMode == MODE_INTER) && !A1InSameCU && 
                             ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP + nPbH - 1) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableA1)
  {
    MvL0 = A1_p->MotionvectorL0;
    MvL1 = A1_p->MotionvectorL1;

    if(QualifiedCandCount == 0)
    {
      *MotionvectorL0_p = MvL0;
      *MotionvectorL1_p = MvL1;
      return;
    }
    else
    {
      /* Qualified candidate, but not with correct merge index */
      mergeCandList[numCand].MotionvectorL0 = MvL0;
      mergeCandList[numCand].MotionvectorL1 = MvL1;
      numCand++;

      QualifiedCandCount--;
    }
  } 

  // check position B1
  B1_p = PU_p[BlockWidthInPU-1].PredictionUnitUp_p;
  NeiborAvailableB1 = B1_p ? ((B1_p->PredMode == MODE_INTER) && !B1InSameCU &&
                             ((xPInMrgLevel != ((xP + nPbW - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;

  if(NeiborAvailableB1)
  {
    if(!(NeiborAvailableA1 &&
         CheckMVAndRefEquality(B1_p->MotionvectorL0, A1_p->MotionvectorL0) &&
         CheckMVAndRefEquality(B1_p->MotionvectorL1, A1_p->MotionvectorL1)))
    {
      MvL0 = B1_p->MotionvectorL0;
      MvL1 = B1_p->MotionvectorL1;

      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;
        return;
      }
      else
      {
        /* Qualified candidate, but not with correct merge index */
        mergeCandList[numCand].MotionvectorL0 = MvL0;
        mergeCandList[numCand].MotionvectorL1 = MvL1;
        numCand++;

        QualifiedCandCount--;
      }
    }
  }

  // check position B0 
  B0_p = PU_p[BlockWidthInPU-1].PredictionUnitUpRight_p;
  NeiborAvailableB0 = B0_p ? ((B0_p->PredMode == MODE_INTER) &&
                             ((xPInMrgLevel != ((xP + nPbW) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableB0)
  {
    if(!(NeiborAvailableB1 &&
         CheckMVAndRefEquality(B0_p->MotionvectorL0, B1_p->MotionvectorL0) &&
         CheckMVAndRefEquality(B0_p->MotionvectorL1, B1_p->MotionvectorL1)))
    {
      MvL0 = B0_p->MotionvectorL0;
      MvL1 = B0_p->MotionvectorL1;

      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;

        return;
      }
      else
      {
        /* Qualified candidate, but not with correct merge index */
        mergeCandList[numCand].MotionvectorL0 = MvL0;
        mergeCandList[numCand].MotionvectorL1 = MvL1;
        numCand++;

        QualifiedCandCount--;
      }
    }
  }

  // check position A0
  A0_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitDownLeft_p;
  NeiborAvailableA0 = A0_p ? ((A0_p->PredMode == MODE_INTER) &&
                             ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP + nPbH) >> Log2ParMrgLevel)))) : 0;
  if(NeiborAvailableA0)
  {
    if(!(NeiborAvailableA1 &&
         CheckMVAndRefEquality(A0_p->MotionvectorL0, A1_p->MotionvectorL0) &&
         CheckMVAndRefEquality(A0_p->MotionvectorL1, A1_p->MotionvectorL1)))
    {
      MvL0 = A0_p->MotionvectorL0;
      MvL1 = A0_p->MotionvectorL1;

      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;
        return;
      }
      else
      {
        /* Qualified candidate, but not with correct merge index */
        mergeCandList[numCand].MotionvectorL0 = MvL0;
        mergeCandList[numCand].MotionvectorL1 = MvL1;
        numCand++;

        QualifiedCandCount--;
      }
    }
  }
  
  if((QualifiedCandCount + 4) != MergeIdx)
  {
    // check position B2
    B2_p = PU_p->PredictionUnitUpLeft_p;
    NeiborAvailableB2 = B2_p ? ((B2_p->PredMode == MODE_INTER) &&
                               ((xPInMrgLevel != ((xP - 1) >> Log2ParMrgLevel)) || (yPInMrgLevel != ((yP - 1) >> Log2ParMrgLevel)))) : 0;
    if(NeiborAvailableB2)
    {
      if(!(NeiborAvailableA1 && 
           CheckMVAndRefEquality(B2_p->MotionvectorL0, A1_p->MotionvectorL0)&&
           CheckMVAndRefEquality(B2_p->MotionvectorL1, A1_p->MotionvectorL1)) 
        && 
         !(NeiborAvailableB1 && 
           CheckMVAndRefEquality(B2_p->MotionvectorL0, B1_p->MotionvectorL0)&&
           CheckMVAndRefEquality(B2_p->MotionvectorL1, B1_p->MotionvectorL1)))
      {
          MvL0 = B2_p->MotionvectorL0;
          MvL1 = B2_p->MotionvectorL1;

        if(QualifiedCandCount == 0)
        {
          *MotionvectorL0_p = MvL0;
          *MotionvectorL1_p = MvL1;
          return;
        }
        else
        {
          /* Qualified candidate, but not with correct merge index */
          mergeCandList[numCand].MotionvectorL0 = MvL0;
          mergeCandList[numCand].MotionvectorL1 = MvL1;
          numCand++;

          QualifiedCandCount--;
        }
      }
    }
  }
  
  //get temporal motion vector (not used by c65 in B-slices)
  if(Sessiondata_p->CurrentCollocatedRefIdx != -1)
  {
    MvL0 = GetTemporalMVLX(Sessiondata_p, 0, xP, yP, nPbW, nPbH, 0); /* 0 for list 0 */
    if(Sessiondata_p->CurrentSliceType == MODE_B)
    {
    MvL1 = GetTemporalMVLX(Sessiondata_p, 0, xP, yP, nPbW, nPbH, 1); /* 1 for list 1 */
    }

    if(MvL0.ref_idx != -1 || MvL1.ref_idx != -1)
    {
      if(QualifiedCandCount == 0)
      {
        *MotionvectorL0_p = MvL0;
        *MotionvectorL1_p = MvL1;

        return;
      }
      else
      {
        /* Qualified candidate, but not with correct merge index */
        mergeCandList[numCand].MotionvectorL0 = MvL0;
        mergeCandList[numCand].MotionvectorL1 = MvL1;
        numCand++;

        QualifiedCandCount--;
      }
    }
  }

  /* Derivation process for combined bi-predictive merging candidates */
 if(Sessiondata_p->CurrentSliceType == MODE_B)
  {
    uint32 combIdx = 0;
    Candidate_t *l0Cand;
    Candidate_t *l1Cand;
    while(combIdx < numCand*(numCand-1))
    {
      l0Cand = mergeCandList + l0CandIdx[combIdx];
      l1Cand = mergeCandList + l1CandIdx[combIdx];

      if(l0Cand->MotionvectorL0.ref_idx != -1 &&
         l1Cand->MotionvectorL1.ref_idx != -1 &&
         (Sessiondata_p->RefPicList0[l0Cand->MotionvectorL0.ref_idx]->PicOrderCnt !=
            Sessiondata_p->RefPicList1[l1Cand->MotionvectorL1.ref_idx]->PicOrderCnt ||
         !CheckMVsEquality(l0Cand->MotionvectorL0, l1Cand->MotionvectorL1)))
      {
        if(QualifiedCandCount == 0)
        {
          *MotionvectorL0_p = l0Cand->MotionvectorL0;
          *MotionvectorL1_p = l1Cand->MotionvectorL1;

          return;
        }
        else
        {
          QualifiedCandCount--;
        }
      }
      combIdx++;
    }
  }

  zeroIdx = 0;
  Mv.x = 0;
  Mv.y = 0;
  if(Sessiondata_p->CurrentSliceType == MODE_B)
  {
    numRefIdx = min(Sessiondata_p->Current_num_ref_idx_l0_active_minus1+1, Sessiondata_p->Current_num_ref_idx_l1_active_minus1+1);
  }
  else
  {
    numRefIdx = Sessiondata_p->Current_num_ref_idx_l0_active_minus1+1;
  }
  while(QualifiedCandCount >= 0)
  {
    //add zero merge candidate
    RefPicIdxL0 = (zeroIdx < (numRefIdx)) ? zeroIdx : 0;
    RefPicIdxL1 = (zeroIdx < (numRefIdx)) ? zeroIdx : 0;
    
    if(QualifiedCandCount == 0)
    {
      break;
    }
    QualifiedCandCount--;
    zeroIdx++;
  }

  if(Sessiondata_p->CurrentSliceType != MODE_B)
  {
    RefPicIdxL1 = -1;
  }
 

  *MotionvectorL0_p = Mv;
  MotionvectorL0_p->ref_idx = (sint8)RefPicIdxL0;
  *MotionvectorL1_p = Mv;
  MotionvectorL1_p->ref_idx = (sint8)RefPicIdxL1;

}



/******************************************************************************
*
* Name:         GetMvpCandidateLX
*
******************************************************************************/
Vector_t
GetMvpCandidateLX(SessionData_t * Sessiondata_p, 
                  const uint32 xP, 
                  const uint32 yP, 
                  const uint32 nPbW, 
                  const uint32 nPbH, 
                  const uint8 MvpLXFlag,
                  const sint32 CurrRefPicIdx,
                  const uint8 ListX)
{
  Vector_t  MotionVectorCandList[2];
  uint8     LeftNeiborAvailable;
  uint8     LeftNeiborPredFound;
  uint8     AboveNeiborPredFound;
  uint8     NeiborAvailable;
  uint32    Log2PUSizeInPixels = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  uint32    PicWidthInPU = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  uint32    xPInPUAddr = xP >> Log2PUSizeInPixels;
  uint32    yPInPUAddr = yP >> Log2PUSizeInPixels;
  uint32    BlockWidthInPU = nPbW >> Log2PUSizeInPixels;
  uint32    BlockHeightInPU = nPbH >> Log2PUSizeInPixels;
  uint32    QualifiedCandCount;
  sint32    CurrRefPicPOC;
  Vector_t  TmpMv;
  Vector_t  TmpNeiborMv;
  sint32    NeiborRefPicIdx;

  /* Pointers to neighboring PUs. See getMotionMergeCandidate for details */
  const PredictionUnit_t *PU_p = Sessiondata_p->PredictionUnitArray + yPInPUAddr*PicWidthInPU + xPInPUAddr;
  const PredictionUnit_t *A0_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitDownLeft_p;
  const PredictionUnit_t *A1_p = PU_p[(BlockHeightInPU-1)*PicWidthInPU].PredictionUnitLeft_p;
  const PredictionUnit_t *B0_p = PU_p[BlockWidthInPU-1].PredictionUnitUpRight_p;
  const PredictionUnit_t *B1_p = PU_p[BlockWidthInPU-1].PredictionUnitUp_p;
  const PredictionUnit_t *B2_p = PU_p->PredictionUnitUpLeft_p;

  uint8 (*checkPredFlagAndPOCEqualityLX)(const SessionData_t *, const PredictionUnit_t *, const sint32);
  uint8 (*checkPredFlagAndPOCEqualityLY)(const SessionData_t *, const PredictionUnit_t *, const sint32);

  Vector_t (*getMvLX)(const PredictionUnit_t *);
  Vector_t (*getMvLY)(const PredictionUnit_t *);

  const ReferencePicture_t ** RefPicListX;
  const ReferencePicture_t ** RefPicListY;

  if(ListX == 0)
  {
    checkPredFlagAndPOCEqualityLX = checkPredFlagAndPOCEqualityL0;
    checkPredFlagAndPOCEqualityLY = checkPredFlagAndPOCEqualityL1;

    getMvLX = getMotionVectorL0;
    getMvLY = getMotionVectorL1;

    RefPicListX = Sessiondata_p->RefPicList0;
    RefPicListY = Sessiondata_p->RefPicList1;
  }
  else
  {
    checkPredFlagAndPOCEqualityLX = checkPredFlagAndPOCEqualityL1;
    checkPredFlagAndPOCEqualityLY = checkPredFlagAndPOCEqualityL0;

    getMvLX = getMotionVectorL1;
    getMvLY = getMotionVectorL0;

    RefPicListX = Sessiondata_p->RefPicList1;
    RefPicListY = Sessiondata_p->RefPicList0;
  }

  CurrRefPicPOC = RefPicListX[CurrRefPicIdx]->PicOrderCnt;

  QualifiedCandCount = 0;
  LeftNeiborAvailable = 0;
  LeftNeiborPredFound = 0;
  AboveNeiborPredFound = 0;
  NeiborAvailable = 0;



  /* DERIVATION OF mvLXA */

  //check position A0
  LeftNeiborAvailable = A0_p && (A0_p->PredMode == MODE_INTER);

  if(!LeftNeiborAvailable)
  {
    //Check position A1
    LeftNeiborAvailable = A1_p && (A1_p->PredMode == MODE_INTER);
  }

  //check position A0

  NeiborAvailable = A0_p && (A0_p->PredMode == MODE_INTER);
  if(NeiborAvailable)
  {

    if(checkPredFlagAndPOCEqualityLX(Sessiondata_p, A0_p, CurrRefPicPOC))
    {
      MotionVectorCandList[QualifiedCandCount++] = getMvLX(A0_p);
      LeftNeiborPredFound = 1;
    }
    else if(checkPredFlagAndPOCEqualityLY(Sessiondata_p, A0_p, CurrRefPicPOC))
    {
      MotionVectorCandList[QualifiedCandCount++] = getMvLY(A0_p);
      LeftNeiborPredFound = 1;
    }
  }
  
  if(!LeftNeiborPredFound)
  {
    // check position A1
    NeiborAvailable = A1_p && (A1_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      if(checkPredFlagAndPOCEqualityLX(Sessiondata_p, A1_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLX(A1_p);
        LeftNeiborPredFound = 1;
      }
      else if(checkPredFlagAndPOCEqualityLY(Sessiondata_p, A1_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLY(A1_p);
        LeftNeiborPredFound = 1;
      }
    }
  }

  if(!LeftNeiborPredFound)
  {
    // A0
    NeiborAvailable = A0_p && (A0_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      /* Ignoring long term reference lists for now */
      /* Choose available reference list and scale */
      if(-1 != (NeiborRefPicIdx = getMvLX(A0_p).ref_idx))
      {
        TmpNeiborMv = getMvLX(A0_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListX[NeiborRefPicIdx], TmpNeiborMv, &LeftNeiborPredFound);
      }
      else if(-1 != (NeiborRefPicIdx = getMvLY(A0_p).ref_idx))
      {
        TmpNeiborMv = getMvLY(A0_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListY[NeiborRefPicIdx], TmpNeiborMv, &LeftNeiborPredFound);
      }

      if(LeftNeiborPredFound)
      {
        MotionVectorCandList[QualifiedCandCount++] = TmpMv;
      }
    }
  }

  if(!LeftNeiborPredFound)
  {
    // A1
    NeiborAvailable = A1_p && (A1_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      /* Ignoring long term reference lists for now */
      /* Choose available reference list and scale */
      if(-1 != (NeiborRefPicIdx = getMvLX(A1_p).ref_idx))
      {
        TmpNeiborMv = getMvLX(A1_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListX[NeiborRefPicIdx], TmpNeiborMv, &LeftNeiborPredFound);
      }
      else if(-1 != (NeiborRefPicIdx = getMvLY(A1_p).ref_idx))
      {
        TmpNeiborMv = getMvLY(A1_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListY[NeiborRefPicIdx], TmpNeiborMv, &LeftNeiborPredFound);
      }

      if(LeftNeiborPredFound)
      {
        MotionVectorCandList[QualifiedCandCount++] = TmpMv;
      }
    }
  }
  

  /* DERIVATION OF mvLXB */

  //check position B0
  NeiborAvailable = B0_p && (B0_p->PredMode == MODE_INTER);
  if(NeiborAvailable)
  {
    if(checkPredFlagAndPOCEqualityLX(Sessiondata_p, B0_p, CurrRefPicPOC))
    {
      MotionVectorCandList[QualifiedCandCount++] = getMvLX(B0_p);
      AboveNeiborPredFound = 1;
    }
    else if(checkPredFlagAndPOCEqualityLY(Sessiondata_p, B0_p, CurrRefPicPOC))
    {
      MotionVectorCandList[QualifiedCandCount++] = getMvLY(B0_p);
      AboveNeiborPredFound = 1;
    }
  }
  
  if(!AboveNeiborPredFound)
  {
    //check position B1
    NeiborAvailable = B1_p && (B1_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      if(checkPredFlagAndPOCEqualityLX(Sessiondata_p, B1_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLX(B1_p);
        AboveNeiborPredFound = 1;
      }
      else if(checkPredFlagAndPOCEqualityLY(Sessiondata_p, B1_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLY(B1_p);
        AboveNeiborPredFound = 1;
      }
    }
  }

  if(!AboveNeiborPredFound)
  {
    //check position B2
    NeiborAvailable = B2_p && (B2_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      if(checkPredFlagAndPOCEqualityLX(Sessiondata_p, B2_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLX(B2_p);
        AboveNeiborPredFound = 1;
      }
      else if(checkPredFlagAndPOCEqualityLY(Sessiondata_p, B2_p, CurrRefPicPOC))
      {
        MotionVectorCandList[QualifiedCandCount++] = getMvLY(B2_p);
        AboveNeiborPredFound = 1;
      }
    }
  }

  if(!LeftNeiborAvailable )
  {
    AboveNeiborPredFound = 0;
    //check position B0
    NeiborAvailable = B0_p && (B0_p->PredMode == MODE_INTER);
    if(NeiborAvailable)
    {
      if(-1 != (NeiborRefPicIdx = getMvLX(B0_p).ref_idx))
      {
        TmpNeiborMv = getMvLX(B0_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListX[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
      }
      else if(-1 != (NeiborRefPicIdx = getMvLY(B0_p).ref_idx))
      {
        TmpNeiborMv = getMvLY(B0_p);
        TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListY[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
      }
        
      if(AboveNeiborPredFound)
      {
        MotionVectorCandList[QualifiedCandCount++] = TmpMv;
      }
    }

    if(!AboveNeiborPredFound)
    {
      //check position B1
      NeiborAvailable = B1_p && (B1_p->PredMode == MODE_INTER);
      if(NeiborAvailable)
      {
        if(-1 != (NeiborRefPicIdx = getMvLX(B1_p).ref_idx))
        {
          TmpNeiborMv = getMvLX(B1_p);
          TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListX[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
        }
        else if(-1 != (NeiborRefPicIdx = getMvLY(B1_p).ref_idx))
        {
          TmpNeiborMv = getMvLY(B1_p);
          TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListY[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
        }
          
        if(AboveNeiborPredFound)
        {
          MotionVectorCandList[QualifiedCandCount++] = TmpMv;
        }
      }
    }

    if(AboveNeiborPredFound == 0)
    {
      //check position B2
      NeiborAvailable = B2_p && (B2_p->PredMode == MODE_INTER);
      if(NeiborAvailable)
      {
        if(-1 != (NeiborRefPicIdx = getMvLX(B2_p).ref_idx))
        {
          TmpNeiborMv = getMvLX(B2_p);
          TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListX[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
        }
        else if(-1 != (NeiborRefPicIdx = getMvLY(B2_p).ref_idx))
        {
          TmpNeiborMv = getMvLY(B2_p);
          TmpMv = ScaleMV(Sessiondata_p->PicOrderCntVal, RefPicListX[CurrRefPicIdx], RefPicListY[NeiborRefPicIdx], TmpNeiborMv, &AboveNeiborPredFound);
        }
          
        if(AboveNeiborPredFound)
        {
          MotionVectorCandList[QualifiedCandCount++] = TmpMv;
        }
      }
    }
    
  }
  if(QualifiedCandCount == 2)
  {
    if(CheckMVsEquality(MotionVectorCandList[0], MotionVectorCandList[1]))
    {
      QualifiedCandCount--;
    }
  }

  if(QualifiedCandCount < 2 && Sessiondata_p->CurrentCollocatedRefIdx != -1)
  {
    uint8 ColMVFound = 0;
    Vector_t TmpMvLX = GetTemporalMVLX(Sessiondata_p, CurrRefPicIdx, xP, yP, nPbW, nPbH, ListX);

    if(TmpMvLX.ref_idx != -1)
    {
      MotionVectorCandList[QualifiedCandCount++] = TmpMvLX;
    }
  }

  while (QualifiedCandCount < 2)
  {
    MotionVectorCandList[QualifiedCandCount].x = 0;
    MotionVectorCandList[QualifiedCandCount].y = 0;
    MotionVectorCandList[QualifiedCandCount].ref_idx = 0;
    QualifiedCandCount++;
  }

  return MotionVectorCandList[MvpLXFlag];
}


/******************************************************************************
*
* Name:        PerformUniprediction 
*
******************************************************************************/
void
PerformUniprediction(
#if MULTI_THREADED
                     ThreadData_t *Threaddata_p,
#else
                     SessionData_t *Sessiondata_p,
#endif
                     const uint32 xP,
                     const uint32 yP,
                     const uint32 nPbW,
                     const uint32 nPbH,
                     const ReferencePicture_t **RefPicList,
                     const Vector_t Mv)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*) Threaddata_p->Sessiondata_p;
  sint16 *Interpl = Threaddata_p->Interpl;
#else
  sint16 *Interpl = Sessiondata_p->Interpl;
#endif
  const ReferencePicture_t *RefPic = RefPicList[Mv.ref_idx];
  ReferencePicture_t *RecPic = Sessiondata_p->CurrentRecImage;

  if(RefPic)
  {
    /* Perform motion luma compensation and save result to reconstructed YUVImage */
    MotionCompensationLuma(Sessiondata_p, xP, yP, nPbW, nPbH, Mv, 1, RefPic->Image.Y, Interpl, NULL);
    /* Perform motion chroma compensation and save result to reconstructed YUVImage */
    MotionCompensationChroma(Sessiondata_p, xP>>1, yP>>1, nPbW>>1, nPbH>>1, Mv, 1, RefPic->Image.Cb, RefPic->Image.Cr, Interpl, NULL, NULL);
  }
  else
  {
#if MULTI_THREADED
    BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "Illegal motion vector");
#else
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "Illegal motion vector");
#endif
  }
}


/************************************************************************************
*
* Name:                 PerformBiprediction
*
*************************************************************************************/
void
PerformBiprediction(
#if MULTI_THREADED
                    ThreadData_t *Threaddata_p,
#else
                    SessionData_t *Sessiondata_p,
#endif
                    const uint32 xP,
                    const uint32 yP,
                    const uint32 nPbW,
                    const uint32 nPbH,
                    const Vector_t Mv0,
                    const Vector_t Mv1)
{
#if MULTI_THREADED
  SessionData_t *Sessiondata_p = (SessionData_t*) Threaddata_p->Sessiondata_p;
  sint16 *Interpl = Threaddata_p->Interpl;
  sint16 *predSampleArrayL0 = Threaddata_p->PredSampleArrayL0;
  sint16 *predSampleArrayL1 = Threaddata_p->PredSampleArrayL1;
#else
  sint16 *Interpl           = Sessiondata_p->Interpl;
  sint16 *predSampleArrayL0 = Sessiondata_p->PredSampleArrayL0;
  sint16 *predSampleArrayL1 = Sessiondata_p->PredSampleArrayL1;
#endif
  sint16 *predSampleArrayCrL0 = predSampleArrayL0 + (MAX_PU_SIZE * MAX_PU_SIZE) / 2;
  sint16 *predSampleArrayCrL1 = predSampleArrayL1 + (MAX_PU_SIZE * MAX_PU_SIZE) / 2;
  ReferencePicture_t *RefPicL0 = Sessiondata_p->RefPicList0[Mv0.ref_idx];
  ReferencePicture_t *RefPicL1 = Sessiondata_p->RefPicList1[Mv1.ref_idx];
  uint8  *ptr;
  uint8  *ptrCb;
  uint8  *ptrCr;

  uint32 RecPicWidth;

  if(RefPicL0 && RefPicL1)
  {
    /* LUMA PART */
    /* Compute predSampleArrayL0 */
    MotionCompensationLuma(Sessiondata_p, xP, yP, nPbW, nPbH, Mv0, 0, RefPicL0->Image.Y, Interpl, predSampleArrayL0);
    /* Compute predSampleArrayL1 */
    MotionCompensationLuma(Sessiondata_p, xP, yP, nPbW, nPbH, Mv1, 0, RefPicL1->Image.Y, Interpl, predSampleArrayL1);
    /* Output result to ptr */
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
    ptr         = Sessiondata_p->CurrentRecImage->Image.Y+(yP+PADDING)*RecPicWidth+xP+PADDING;
    WriteAvgToCurrentRecImg(nPbW, nPbH, RecPicWidth, predSampleArrayL0, predSampleArrayL1, ptr);

    /* CHROMA PART */
    /* Compute predSampleArrayL0 for Cb and Cr */
    MotionCompensationChroma(Sessiondata_p, xP>>1, yP>>1, nPbW>>1, nPbH>>1, Mv0, 0, RefPicL0->Image.Cb, RefPicL0->Image.Cr, Interpl, predSampleArrayL0, predSampleArrayCrL0);
    /* Compute predSampleArrayL1 for Cb and Cr */
    MotionCompensationChroma(Sessiondata_p, xP>>1, yP>>1, nPbW>>1, nPbH>>1, Mv1, 0, RefPicL1->Image.Cb, RefPicL1->Image.Cr, Interpl, predSampleArrayL1, predSampleArrayCrL1);
    /* Output result to ptrCb and ptrCr */
    RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
    ptrCb       = Sessiondata_p->CurrentRecImage->Image.Cb+((yP>>1)+(PADDING>>1))*RecPicWidth+(xP>>1)+(PADDING>>1);
    ptrCr       = Sessiondata_p->CurrentRecImage->Image.Cr+((yP>>1)+(PADDING>>1))*RecPicWidth+(xP>>1)+(PADDING>>1);
    WriteAvgToCurrentRecImg(nPbW>>1, nPbH>>1, RecPicWidth, predSampleArrayL0,   predSampleArrayL1,   ptrCb);
    WriteAvgToCurrentRecImg(nPbW>>1, nPbH>>1, RecPicWidth, predSampleArrayCrL0, predSampleArrayCrL1, ptrCr);
  }
  else
  {
#if MULTI_THREADED
    BIT_ERROR(Threaddata_p->LongJumpSlice, D65_BIT_ERROR, "Illegal motion vector");
#else
    BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "Illegal motion vector");
#endif
  }
}
