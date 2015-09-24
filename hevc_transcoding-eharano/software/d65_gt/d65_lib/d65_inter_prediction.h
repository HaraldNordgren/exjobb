#ifndef D65_INTER_PREDICTION_H
#define D65_INTER_PREDICTION_H

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
#include <string.h>
#include "d65_api.h"
#include "d65_internal.h"

#if SIMD
#include <tmmintrin.h>
#include <smmintrin.h>
#endif
/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        GetMotionMergeCandidate(P or B)
*
* Parameters:  Sessiondata_p    [in]      The decoder
*              xP               [in]      Current PU X coordinate in pixels
*              yP               [in]      Current CU size in pixels
*              nPbW             [in]      Current PU width in pixels
*              bPbH             [in]      Current PU height in pixels
*              MergeIdx         [in]      Current PU's motion merge index
*              PartIdx          [in]      Current PU index in the CU
*              PartMode         [in]      Current partition mode
*              MotionvectorL0_p [out]     Motion Vector L0
*              MotionvectorL1_p [out]     Motion Vector L1
*
* Returns:     void
*
* Description: Generate motion merge candidate list and set the derived motion
*              vector into MotionvectorL0 and MotionvectorL1 
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
                         Vector_t *MotionvectorL1_p);

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
                         Vector_t *MotionvectorL1_p);

typedef void GetMotionMergeCandidate (SessionData_t *, 
                                      const uint32 , 
                                      const uint32 , 
                                      const uint32 , 
                                      const uint32 , 
                                      const uint8 ,
                                      const uint8 ,
                                      const PartitionMode_t ,
                                      Vector_t *,
                                      Vector_t *);

static GetMotionMergeCandidate* const GetMergedMotion[2] = {GetMotionMergeCandidateB,
                                                            GetMotionMergeCandidateP};

/******************************************************************************
*
* Name:        GetMvpCandidateLX
*
* Parameters:  Sessiondata_p  [in]      The decoder
*              xP             [in]      Current PU X coordinate in pixels
*              yP             [in]      Current CU size in pixels
*              nPbW           [in]      Current PU width in pixels
*              bPbH           [in]      Current PU height in pixels
*              MvpLXFlag      [in]      Current PU's motion vector predictor index
*              CurrRefPicIdx  [in]      Current reference picture index in ListX (0 or 1)
*              ListX          [in]      Deriving motion vector for reference list ListX (0 or 1)
*
* Returns:     Vector_t
*
* Description: Generate AMVP candidate list and return the derived motion
*              vector for reference list ListX 
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
                  const uint8 ListX);

/******************************************************************************
*
* Name:        PerformUniprediction
*
* Parameters:  Sessiondata_p  [in]      The decoder
*              xP             [in]      Current PU X coordinate in pixels
*              yP             [in]      Current CU size in pixels
*              nPbW           [in]      Current PU width in pixels
*              bPbH           [in]      Current PU height in pixels
*              RefPicList     [in]      Reference picture list to be used
*              Mv             [in]      Motion vector
*
* Returns:     void
*
* Description: Unipredictive motion compensation for luma and chroma components
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
                     const Vector_t Mv);

/******************************************************************************
*
* Name:        PerformBiprediction
*
* Parameters:  Sessiondata_p  [in/out]  The decoder
*              xP             [in]      Current PU X coordinate in pixels
*              yP             [in]      Current CU size in pixels
*              nPbW           [in]      Current PU width in pixels
*              nPbH           [in]      Current PU height in pixels
*              Mv0            [in]      Motion vector for list L0
*              Mv1            [in]      Motion vector for list L1
*
* Returns:     void
*
* Description: Bipredictive motion compensation for luma and chroma components
*
******************************************************************************/
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
                    const Vector_t Mv1);

#endif
