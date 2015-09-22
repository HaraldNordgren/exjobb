#ifndef D65_CABAC_H
#define D65_CABAC_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for CABAC decoding
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include "d65_api.h"
#include "d65_internal.h"


/******************************************************************************
* Declaration of functions
******************************************************************************/


/******************************************************************************
* Initialization
******************************************************************************/
void BacDecoderStart(Parser_t *Parser_p);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
void BacEncoderStart(BacEncoder_t *BacEncoder_p);
#endif

/******************************************************************************
* Termination Bits
******************************************************************************/
uint32
GetBinTrm(Parser_t *Parser_p);

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
/******************************************************************************
* Encoder functions
******************************************************************************/
void
EncodeBin(SubEncoder_t *SubEncoder_p,
          Context *Ctx_p,
          uint32 Symbol);

void
EncodeBinEP(SubEncoder_t *SubEncoder_p,
            uint32 Symbol);

void
EncodeBinsEP(SubEncoder_t *SubEncoder_p,
             uint32 Symbols,
             uint32 NumBins);

void
EncodeBinTrm(SubEncoder_t *SubEncoder_p,
             uint32 Symbol);

void
EncodeGolombRice(SubEncoder_t *SubEncoder_p,
                 uint32        golombRiceParam,
                 sint32        Value);

void
BacEncoderFinish(SubEncoder_t *SubEncoder_p);

void
EncodeTrSkipLumaFlag(Parser_t *Parser_p,
                     uint32 Symbol);

void
EncodeTrSkipChromaFlag(Parser_t *Parser_p,
                       uint32 Symbol);

void
EncodeLastSigCoeffPrefixXLuma(Parser_t *Parser_p,
                              const uint32 Log2TrafoSize,
                              const uint32 Symbol);

void
EncodeLastSigCoeffPrefixYLuma(Parser_t *Parser_p,
                              const uint32 Log2TrafoSize,
                              const uint32 Symbol);

void
EncodeLastSigCoeffPrefixXChroma(Parser_t *Parser_p,
                                const uint32 Log2TrafoSize,
                                const uint32 Symbol);

void
EncodeLastSigCoeffPrefixYChroma(Parser_t *Parser_p,
                                const uint32 Log2TrafoSize,
                                const uint32 Symbol);

void
EncodeLastSigCoeffLuma(Parser_t *Parser_p,
                       sint32 lastX,
                       sint32 lastY,
                       uint32 Log2TrafoSize);

void
EncodeLastSigCoeffChroma(Parser_t *Parser_p,
                         sint32 lastX,
                         sint32 lastY,
                         uint32 Log2TrafoSize);

void
EncodeCodedSubBlockFlagLuma(Parser_t *Parser_p,
                            uint32 CtxIdx,
                            uint32 Symbol);

void
EncodeCodedSubBlockFlagChroma(Parser_t *Parser_p,
                              uint32    CtxIdx,
                              uint32    Symbol);

void
EncodeSigCoeffFlagLuma(Parser_t *Parser_p,
                            sint32 x,
                            sint32 y,
                            sint32 xOffset,
                            sint32 yOffset,
                            sint32 count,
                            sint32 offset,
                            sint32 width,
                            uint32 Symbol);

void
EncodeSigCoeffFlagChroma(Parser_t *Parser_p,
                         sint32 x,
                         sint32 y,
                         sint32 size,
                         sint32 xOffset,
                         sint32 yOffset,
                         sint32 count,
                         sint32 offset,
                         uint32 Symbol);

void
EncodeCoeffAbsLevelG1Luma(Parser_t *Parser_p,
                          uint32 ctxNum,
                          uint32 Symbol);

void
EncodeCoeffAbsLevelG1Chroma(Parser_t *Parser_p,
                            uint32 ctxNum,
                            const uint32 Symbol);

void
EncodeCoeffAbsLevelG2Luma(Parser_t *Parser_p,
                          uint32 ctxNum,
                          uint32 Symbol);

void
EncodeCoeffAbsLevelG2Chroma(Parser_t *Parser_p,
                            uint32 ctxNum,
                            const uint32 Symbol);

#endif

/******************************************************************************
* SAO functions
******************************************************************************/
uint32
GetSaoMergeFlag(Parser_t *Parser_p);

uint32
GetSaoTypeIdx(Parser_t *Parser_p);

uint32
GetSaoOffsetAbs(Parser_t *Parser_p);

uint32
GetSaoOffsetSign(Parser_t *Parser_p);

uint32
GetSaoBandPosition(Parser_t *Parser_p);

uint32
GetSaoEoClass(Parser_t *Parser_p);


/******************************************************************************
* CU functions
******************************************************************************/
uint32
GetSplitCuFlag(Parser_t *Parser_p,
               PredictionUnit_t *PU_p,
               const uint32 CuSize);

uint32
GetSkipFlag(Parser_t *Parser_p,
            PredictionUnit_t *PU_p);

uint32
GetCuTransquantBypassFlag(Parser_t *Parser_p);

uint32
GetPredModeFlag(Parser_t *Parser_p);

uint32
GetIntraPartMode(Parser_t *Parser_p);

uint32
GetInterPartMode(Parser_t *Parser_p,
                 const uint32 AmpEnabledFlag,
                 const uint32 Log2MinCbSizeY,
                 const uint32 IsLeafCU);

uint32
GetNoResidualSyntaxFlag(Parser_t *Parser_p);


/******************************************************************************
* Intra PU functions
******************************************************************************/
uint32
GetPrevIntraLumaPredFlag(Parser_t *Parser_p);

uint32
GetMpmIdx(Parser_t *Parser_p);

uint32
GetRemIntraLumaPredMode(Parser_t *Parser_p);

uint8
GetIntraChromaPredMode(Parser_t *Parser_p);


/******************************************************************************
* Inter PU functions
******************************************************************************/
uint8
GetMergeIdx(Parser_t *Parser_p,
            const uint8 NumMergeCand);

uint8
GetMergeFlag(Parser_t *Parser_p);

uint8
GetMinInterPredIdc(Parser_t *Parser_p);

uint8
GetInterPredIdc(Parser_t *Parser_p,
                const uint32 ctDepth);

uint32
GetRefIdx(Parser_t *Parser_p,
          const uint32 NumRefIdxMinus1);

uint32
GetAbsMvdGreater0Flag(Parser_t *Parser_p);

uint32
GetAbsMvdGreater1Flag(Parser_t *Parser_p);

uint32 
GetAbsMvdMinus2(Parser_t *Parser_p);

uint32
GetMvdSignFlag(Parser_t *Parser_p);

uint32
GetMvpLxFlag(Parser_t *Parser_p);


/******************************************************************************
* TU functions
******************************************************************************/
uint32
GetSplitTransformFlag(Parser_t *Parser_p,
                      const uint32 Log2TrafoSize);

uint32
GetCbfChroma(Parser_t *Parser_p,
             const uint32 trafoDepth);

uint32
GetCbfLuma(Parser_t *Parser_p,
           const uint32 trafoDepth);

uint32
GetCuQpDeltaAbs(Parser_t * Parser_p);

uint8
GetCuQpDeltaSignFlag(Parser_t *Parser_p);


/******************************************************************************
* Residual functions
******************************************************************************/

typedef uint32 GetLastSigCoeffPrefixLumaFunc(Parser_t * );

uint8
GetTrSkipLumaFlag(Parser_t *Parser_p);

uint8
GetTrSkipChromaFlag(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixXLuma32x32(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixXLuma16x16(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixXLuma8x8(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixXLuma4x4(Parser_t *Parser_p);

static GetLastSigCoeffPrefixLumaFunc* const GetLastSigCoeffPrefixXLumaArray[4] =
{
  GetLastSigCoeffPrefixXLuma4x4,
  GetLastSigCoeffPrefixXLuma8x8,
  GetLastSigCoeffPrefixXLuma16x16,
  GetLastSigCoeffPrefixXLuma32x32
};

uint32
GetLastSigCoeffPrefixXChroma(Parser_t *Parser_p,
                            const uint32 Log2TrafoSize);

uint32
GetLastSigCoeffPrefixYLuma32x32(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixYLuma16x16(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixYLuma8x8(Parser_t *Parser_p);

uint32
GetLastSigCoeffPrefixYLuma4x4(Parser_t *Parser_p);

static GetLastSigCoeffPrefixLumaFunc* const GetLastSigCoeffPrefixYLumaArray[4] =
{
  GetLastSigCoeffPrefixYLuma4x4,
  GetLastSigCoeffPrefixYLuma8x8,
  GetLastSigCoeffPrefixYLuma16x16,
  GetLastSigCoeffPrefixYLuma32x32
};

uint32
GetLastSigCoeffPrefixYChroma(Parser_t *Parser_p,
                            const uint32 Log2TrafoSize);

uint32
GetLastSigCoeffSuffix(Parser_t *Parser_p,
                      const uint32 LastSigCoeffPrefix);

uint8
GetCodedSubBlockFlagLuma(Parser_t *Parser_p,
                         const uint32 SubBlockFlagContextIdx);

uint8
GetCodedSubBlockFlagChroma(Parser_t *Parser_p,
                           const uint32 SubBlockFlagContextIdx);

uint8
GetSigCoeffFlagLuma(Parser_t *Parser_p,
                    const uint32 ContextChoiceFlag,
                    const uint32 posInSubBlock,
                    const uint32 ContextOffset,
                    const uint32 Offset);

uint8
GetSigCoeffFlagChroma(Parser_t *Parser_p,
                      const uint32 Log2TrafoSize,
                      const uint32 SubBlockIdx,
                      const uint32 posInSubBlock,
                      const uint32 ContextOffset,
                      const uint32 Offset);

uint8
GetCoeffAbsLevelGreater1FlagLuma(Parser_t *Parser_p,
                                 const uint32 ctxIdx);

uint8
GetCoeffAbsLevelGreater1FlagChroma(Parser_t *Parser_p,
                                   const uint32 ctxIdx);

uint8
GetCoeffAbsLevelGreater2FlagLuma(Parser_t *Parser_p,
                                 const uint32 ctxIdx);

uint8
GetCoeffAbsLevelGreater2FlagChroma(Parser_t *Parser_p,
                                   const uint32 ctxIdx);

uint8
GetCoeffSignFlag(Parser_t *Parser_p);

uint32
GetCoeffAbsLevelRemaining(Parser_t *Parser_p,
                          const uint32 golombRiceParam);

#endif