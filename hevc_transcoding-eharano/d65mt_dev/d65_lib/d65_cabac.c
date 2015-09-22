/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains cabac decoding functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include "d65_cabac.h"
#include "d65_context.h"
#include "d65_getbits.h"


/******************************************************************************
* Definition of local variables
******************************************************************************/
#define MIN_RANGE 32768

static const uint8 NextStateMPS[ 128 ] = 
{
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
  34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
  66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
  82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 124, 125, 126, 127
};

static const uint8 NextStateLPS[ 128 ] =
{
  1, 0, 0, 1, 2, 3, 4, 5, 4, 5, 8, 9, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 18, 19, 22, 23, 22, 23, 24, 25,
  26, 27, 26, 27, 30, 31, 30, 31, 32, 33, 32, 33, 36, 37, 36, 37,
  38, 39, 38, 39, 42, 43, 42, 43, 44, 45, 44, 45, 46, 47, 48, 49,
  48, 49, 50, 51, 52, 53, 52, 53, 54, 55, 54, 55, 56, 57, 58, 59,
  58, 59, 60, 61, 60, 61, 60, 61, 62, 63, 64, 65, 64, 65, 66, 67,
  66, 67, 66, 67, 68, 69, 68, 69, 70, 71, 70, 71, 70, 71, 72, 73,
  72, 73, 72, 73, 74, 75, 74, 75, 74, 75, 76, 77, 76, 77, 126, 127
};

static const uint8 LPSTable[64][4] = 
{
  { 128, 176, 208, 240},
  { 128, 167, 197, 227},
  { 128, 158, 187, 216},
  { 123, 150, 178, 205},
  { 116, 142, 169, 195},
  { 111, 135, 160, 185},
  { 105, 128, 152, 175},
  { 100, 122, 144, 166},
  {  95, 116, 137, 158},
  {  90, 110, 130, 150},
  {  85, 104, 123, 142},
  {  81,  99, 117, 135},
  {  77,  94, 111, 128},
  {  73,  89, 105, 122},
  {  69,  85, 100, 116},
  {  66,  80,  95, 110},
  {  62,  76,  90, 104},
  {  59,  72,  86,  99},
  {  56,  69,  81,  94},
  {  53,  65,  77,  89},
  {  51,  62,  73,  85},
  {  48,  59,  69,  80},
  {  46,  56,  66,  76},
  {  43,  53,  63,  72},
  {  41,  50,  59,  69},
  {  39,  48,  56,  65},
  {  37,  45,  54,  62},
  {  35,  43,  51,  59},
  {  33,  41,  48,  56},
  {  32,  39,  46,  53},
  {  30,  37,  43,  50},
  {  29,  35,  41,  48},
  {  27,  33,  39,  45},
  {  26,  31,  37,  43},
  {  24,  30,  35,  41},
  {  23,  28,  33,  39},
  {  22,  27,  32,  37},
  {  21,  26,  30,  35},
  {  20,  24,  29,  33},
  {  19,  23,  27,  31},
  {  18,  22,  26,  30},
  {  17,  21,  25,  28},
  {  16,  20,  23,  27},
  {  15,  19,  22,  25},
  {  14,  18,  21,  24},
  {  14,  17,  20,  23},
  {  13,  16,  19,  22},
  {  12,  15,  18,  21},
  {  12,  14,  17,  20},
  {  11,  14,  16,  19},
  {  11,  13,  15,  18},
  {  10,  12,  15,  17},
  {  10,  12,  14,  16},
  {   9,  11,  13,  15},
  {   9,  11,  12,  14},
  {   8,  10,  12,  14},
  {   8,   9,  11,  13},
  {   7,   9,  11,  12},
  {   7,   9,  10,  12},
  {   7,   8,  10,  11},
  {   6,   8,   9,  11},
  {   6,   7,   9,  10},
  {   6,   7,   8,   9},
  {   2,   2,   2,   2}
};

static const uint8 RenormTable[32]  =
{
  6,  5,  4,  4,
  3,  3,  3,  3,
  2,  2,  2,  2,
  2,  2,  2,  2,
  1,  1,  1,  1,
  1,  1,  1,  1,
  1,  1,  1,  1,
  1,  1,  1,  1
};


/******************************************************************************
* Definition of local functions
******************************************************************************/
#if D65_BITTRACE == 2
/******************************************************************************
*
* Name:        TraceCabacSyntax
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
TraceCabacSyntax(const uint32 SyntaxValue,
                 const char *TraceString)
{

  if(TraceFlag)
  {
    FILE *TraceFile_p = Tracefile_p;

    if(TraceFile_p != NULL)
    {
      uint32 Chars;
      
      // Put out bitposition
      putc('@', TraceFile_p);
      Chars = fprintf(TraceFile_p, "%i", TraceBitCounter);
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
      Chars = fprintf(TraceFile_p, "%u", SyntaxValue, "\n");
      // Put out newline
      Chars = fprintf(TraceFile_p, "\n");
    }
  }
}
#endif

#if D65_BITTRACE == 3
/******************************************************************************
*
* Name:        TraceCabacStatus
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
TraceCabacStatus(const uint32 Range,
                 const uint32 Value,
                 const sint32 BitsNeeded)
{
  if(TraceFlag)
  {
    FILE *TraceFile_p = Tracefile_p;

    if(TraceFile_p != NULL)
    {
      fprintf(TraceFile_p, "%s %u, ", "Value =", Value);
      fprintf(TraceFile_p, "%s %u, ", "Range =", Range);
      fprintf(TraceFile_p, "%s %i", "Bit =", BitsNeeded);
      fprintf(TraceFile_p, "\n");
    }
  }
}
#endif

/******************************************************************************
*
* Name:        DecodeBin
*
******************************************************************************/
uint32 DecodeBin(Parser_t *Parser_p, Context *Ctx_p)
{
  BacDecoder_t *BacDecoder_p = &Parser_p->BacDecoder;
  uint32 *range = &BacDecoder_p->Range;
  uint32 *value = &BacDecoder_p->Value;
  sint32 *bitNeeded = &BacDecoder_p->BitsNeeded;
  uint32 *state = &Ctx_p->m_usState;
  uint32 uiLPS = LPSTable[(*state) >> 1][((*range) >> 6) - 4];
  uint32 scaledRange;
  uint32 numBits;
  uint32 Value;
  uint32 ruiBin;

#if D65_BITTRACE == 3
TraceCabacStatus(BacDecoder_p->Range, BacDecoder_p->Value, BacDecoder_p->BitsNeeded);
#endif
  (*range) -= uiLPS;
  scaledRange = (*range) << 7;
  if((*value) < scaledRange)
  {
    ruiBin = ((*state) & 1);
    (*state) = NextStateMPS[(*state)];
    if(scaledRange >= MIN_RANGE)
    {
      return ruiBin;
    }
    (*range) = scaledRange >> 6;
    (*value) <<= 1;
    if(++(*bitNeeded) == 0)
    {
      (*bitNeeded) = -8;
      Value = GetByte(Parser_p, NULL);
      (*value)    += Value;
    }
  }
  else
  {
    numBits = RenormTable[uiLPS >> 3];
    (*value) = ((*value) - scaledRange) << numBits;
    (*range) = uiLPS << numBits;
    ruiBin = 1 - ((*state) & 1);
    (*state) = NextStateLPS[(*state)];
    (*bitNeeded) += numBits;
    if((*bitNeeded) >= 0)
    {
      Value = GetByte(Parser_p, NULL);
      (*value) += Value << (*bitNeeded);
      (*bitNeeded) -= 8;
    }
  }
  return ruiBin;
}

/******************************************************************************
*
* Name:        DecodeBinEP
*
******************************************************************************/
uint32 DecodeBinEP(Parser_t *Parser_p)
{
  BacDecoder_t *BacDecoder_p = &Parser_p->BacDecoder;
  uint32 *value = &(BacDecoder_p->Value);
  sint32 *bitNeeded = &(BacDecoder_p->BitsNeeded);
  uint32 scaledRange = (BacDecoder_p->Range) << 7;

#if D65_BITTRACE == 3
TraceCabacStatus(BacDecoder_p->Range, BacDecoder_p->Value, BacDecoder_p->BitsNeeded);
#endif
  (*value) <<= 1;
  if(++(*bitNeeded) >= 0)
  {
    (*bitNeeded) -= 8;
    (*value) += GetByte(Parser_p, NULL);
  }

  if((*value) >= scaledRange)
  {
    (*value) -= scaledRange;
    return 1;
  }
  return 0;
}

/******************************************************************************
*
* Name:        DecodeBinsEP
*
******************************************************************************/
uint32 DecodeBinsEP(Parser_t *Parser_p,
                    uint32 numBins)
{
  BacDecoder_t *BacDecoder_p = &Parser_p->BacDecoder;
  uint32 i;
  uint32 ruiBin = 0;
  uint32 scaledRange;
  uint32 *value = &(BacDecoder_p->Value);
  sint32 *bitNeeded = &(BacDecoder_p->BitsNeeded);

#if D65_BITTRACE == 3
TraceCabacStatus(BacDecoder_p->Range, BacDecoder_p->Value, BacDecoder_p->BitsNeeded);
#endif
  while(numBins > 8)
  {
    (*value) = ((*value) << 8) + (GetByte(Parser_p, NULL) << (8 + (*bitNeeded)));
    scaledRange = (BacDecoder_p->Range) << 15;
    for (i = 0; i < 8; i++)
    {
      ruiBin += ruiBin;
      scaledRange >>= 1;
      if ((*value) >= scaledRange )
      {
        ruiBin++;
        (*value) -= scaledRange;
      }
    }
    numBins -= 8;
  }

  (*bitNeeded) += numBins;
  (*value) <<= numBins;

  if((*bitNeeded) >= 0)
  {
    (*value) += GetByte(Parser_p, NULL) << (*bitNeeded);
    (*bitNeeded) -= 8;
  }

  scaledRange = (BacDecoder_p->Range) << (numBins + 7);
  for(i = 0; i < numBins; i++)
  {
    ruiBin += ruiBin;
    scaledRange >>= 1;
    if ((*value) >= scaledRange )
    {
      ruiBin++;
      (*value) -= scaledRange;
    }
  }
  return ruiBin;
}

/******************************************************************************
*
* Name:        DecodeBinTrm
*
******************************************************************************/
uint32 DecodeBinTrm(Parser_t *Parser_p)
{
  BacDecoder_t *BacDecoder_p = &Parser_p->BacDecoder;
  uint32 scaledRange;

#if D65_BITTRACE == 3
TraceCabacStatus(BacDecoder_p->Range, BacDecoder_p->Value, BacDecoder_p->BitsNeeded);
#endif
  BacDecoder_p->Range -= 2;
  scaledRange = BacDecoder_p->Range << 7;
  if(BacDecoder_p->Value >= scaledRange )
  {
    return 1;
  }
  else
  {
    if(scaledRange < MIN_RANGE)
    {
      BacDecoder_p->Range = scaledRange >> 6;
      BacDecoder_p->Value <<= 1;

      if(++BacDecoder_p->BitsNeeded == 0)
      {
        BacDecoder_p->BitsNeeded = -8;
        BacDecoder_p->Value += GetByte(Parser_p, NULL);
      }
    }
  }
  return 0;
}

/******************************************************************************
*
* Name:        DecodeEpExGolomb
*
******************************************************************************/
uint32 DecodeEpExGolomb(Parser_t *Parser_p,
                        uint32 numBins)
{
  uint32 uiSymbol = 0;
  uint32 uiBit = 1;

  while(uiBit)
  {
    uiBit = DecodeBinEP(Parser_p);
    uiSymbol += uiBit << numBins++;
  }

  if(--numBins)
  {
    uiBit = DecodeBinsEP(Parser_p, numBins);
    uiSymbol += uiBit;
  }

  return uiSymbol;
}


/******************************************************************************
* Definition of external functions
******************************************************************************/


/******************************************************************************
*
* Name:        BacDecoderStart
*
******************************************************************************/
void BacDecoderStart(Parser_t *Parser_p)
{
  BacDecoder_t *BacDecoder_p = &Parser_p->BacDecoder;
  BacDecoder_p->Value        = GetBits(Parser_p, 16, NULL);
  BacDecoder_p->Range        = 510;
  BacDecoder_p->BitsNeeded   = -8;
}

/******************************************************************************
*
* Name:        GetBinTrm
*
******************************************************************************/
uint32
GetBinTrm(Parser_t *Parser_p)
{
  uint32 TerminateBit;
  TerminateBit = DecodeBinTrm(Parser_p);
  return TerminateBit;
}

/******************************************************************************
*
* Name:        GetSaoMergeFlag
*
******************************************************************************/
uint32
GetSaoMergeFlag(Parser_t *Parser_p)
{
  uint32 SaoMergeLeftFlag;
  SaoMergeLeftFlag = DecodeBin(Parser_p, Parser_p->CtxModels.SaoMergeCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SaoMergeLeftFlag, "SAO - sao_merge_left_flag");
#endif
  return SaoMergeLeftFlag;
}

/******************************************************************************
*
* Name:        GetSaoTypeIdx
*
******************************************************************************/
uint32
GetSaoTypeIdx(Parser_t *Parser_p)
{
  uint32 SaoTypeIdx;
  SaoTypeIdx = DecodeBin(Parser_p, Parser_p->CtxModels.SaoTypeIdxCtx);
  if(SaoTypeIdx == 0)
  {
#if D65_BITTRACE == 2
    TraceCabacSyntax(0, "SAO - sao_type_idx");
#endif
    return 0;
  }
  else
  {
    SaoTypeIdx = DecodeBinEP(Parser_p);
    if(SaoTypeIdx == 0)
    {
#if D65_BITTRACE == 2
    TraceCabacSyntax(1, "SAO - sao_type_idx");
#endif
      return 1;
    }
    else
    {
#if D65_BITTRACE == 2
    TraceCabacSyntax(2, "SAO - sao_type_idx");
#endif
      return 2;
    }
  }
#if D65_BITTRACE == 2
    TraceCabacSyntax(0, "SAO - sao_type_idx");
#endif
  return 0;
}

/******************************************************************************
*
* Name:        GetSaoOffsetAbs
*
******************************************************************************/
uint32
GetSaoOffsetAbs(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 SaoOffsetAbs = 0;
  for( ; SaoOffsetAbs < 7; SaoOffsetAbs++)
  {
    ValueTmp = DecodeBinEP(Parser_p);
    if(ValueTmp == 0)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(SaoOffsetAbs, "SAO - sao_offset_abs");
#endif
  return SaoOffsetAbs;
}

/******************************************************************************
*
* Name:        GetSaoOffsetSign
*
******************************************************************************/
uint32
GetSaoOffsetSign(Parser_t *Parser_p)
{
  uint32 SaoOffsetSign = DecodeBinEP(Parser_p);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SaoOffsetSign, "SAO - sao_offset_abs");
#endif
  return SaoOffsetSign;
}

/******************************************************************************
*
* Name:        GetSaoBandPosition
*
******************************************************************************/
uint32
GetSaoBandPosition(Parser_t *Parser_p)
{
  uint32 SaoBandPosition = DecodeBinsEP(Parser_p, 5);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SaoBandPosition, "SAO - sao_band_position");
#endif
  return SaoBandPosition;
}

/******************************************************************************
*
* Name:        GetSaoEoClass
*
******************************************************************************/
uint32
GetSaoEoClass(Parser_t *Parser_p)
{
  uint32 SaoEoClass = DecodeBinsEP(Parser_p, 2);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SaoEoClass, "SAO - sao_eo_class");
#endif
  return SaoEoClass;
}

/******************************************************************************
*
* Name:        GetSplitCuFlag
*
******************************************************************************/
uint32
GetSplitCuFlag(Parser_t *Parser_p,
               PredictionUnit_t *PU_p,
               const uint32 CuSize)
{
  uint32 SplitCuFlag;
  uint32 CtxIdx = 0;

  if(PU_p->PredictionUnitUp_p)
  {
    CtxIdx += (PU_p->PredictionUnitUp_p->CUSize < CuSize) ? 1 : 0;
  }
  if(PU_p->PredictionUnitLeft_p)
  {
    CtxIdx += (PU_p->PredictionUnitLeft_p->CUSize < CuSize) ? 1 : 0;
  }

  SplitCuFlag = DecodeBin(Parser_p, Parser_p->CtxModels.SplitCuFlagCtx + CtxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SplitCuFlag, "CU - split_cu_flag");
#endif
  return SplitCuFlag;
}

/******************************************************************************
*
* Name:        GetSkipFlag
*
******************************************************************************/
uint32
GetSkipFlag(Parser_t *Parser_p,
            PredictionUnit_t *PU_p)
{
  uint32 SkipFlag;
  uint32 CtxIdx = 0;

  if(PU_p->PredictionUnitUp_p)
  {
    CtxIdx += (PU_p->PredictionUnitUp_p->skip_flag == 1) ? 1 : 0;
  }

  if(PU_p->PredictionUnitLeft_p)
  {
    CtxIdx += (PU_p->PredictionUnitLeft_p->skip_flag == 1 ) ? 1 : 0;
  }

  SkipFlag = DecodeBin(Parser_p, Parser_p->CtxModels.SkipFlagCtx + CtxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SkipFlag, "CU - cu_skip_flag");
#endif
  return SkipFlag;
}

/******************************************************************************
*
* Name:        GetCuTransquantBypassFlag
*
******************************************************************************/
uint32
GetCuTransquantBypassFlag(Parser_t *Parser_p)
{
  uint32 CuTransquantBypassFlag;
  CuTransquantBypassFlag = DecodeBin(Parser_p, Parser_p->CtxModels.CuTrQBypassFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CuTransquantBypassFlag, "CU - cu_transquant_bypass_flag");
#endif
  return CuTransquantBypassFlag;
}

/******************************************************************************
*
* Name:        GetPredModeFlag
*
******************************************************************************/
uint32
GetPredModeFlag(Parser_t *Parser_p)
{
  uint32 PredModeFlag;
  PredModeFlag = DecodeBin(Parser_p, Parser_p->CtxModels.PredModeFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(PredModeFlag, "CU - pred_mode_flag");
#endif
  return PredModeFlag;
}

/******************************************************************************
*
* Name:        GetIntraPartMode
*
******************************************************************************/
uint32
GetIntraPartMode(Parser_t *Parser_p)
{
  uint32 PartMode;
  uint32 ValueTmp;
  ValueTmp = DecodeBin(Parser_p, Parser_p->CtxModels.PartModeCtx);
  PartMode = ValueTmp ? 0 : 3;
#if D65_BITTRACE == 2
  TraceCabacSyntax(PartMode, "CU - part_mode");
#endif
  return PartMode;
}

/******************************************************************************
*
* Name:        GetInterPartMode
*
******************************************************************************/
uint32
GetInterPartMode(Parser_t *Parser_p,
                 const uint32 AmpEnabledFlag,
                 const uint32 Log2MinCbSizeY,
                 const uint32 IsLeafCU)
{
  uint32 PartMode;
  uint32 ValueTmp;
  uint32 MaxNumBit = 2;
  Context *Ctx_p = Parser_p->CtxModels.PartModeCtx;
  if(IsLeafCU && (Log2MinCbSizeY > 3))
  {
    MaxNumBit++;
  }
  PartMode = 0;
  for(; PartMode < MaxNumBit; PartMode++)
  {
    ValueTmp = DecodeBin(Parser_p, Ctx_p + PartMode);
    if(ValueTmp)
    {
      break;
    }
  }
  if(AmpEnabledFlag && !IsLeafCU)
  {
    if(PartMode == 1)
    {
      ValueTmp = DecodeBin(Parser_p, Ctx_p + 3);
      if(ValueTmp == 0)
      {
        ValueTmp = DecodeBinEP(Parser_p);
        PartMode = ValueTmp ? 5 : 4;
      }
    }
    else if(PartMode == 2)
    {
      ValueTmp = DecodeBin(Parser_p, Ctx_p + 3);
      if(ValueTmp == 0)
      {
        ValueTmp = DecodeBinEP(Parser_p);
        PartMode = ValueTmp ? 7 : 6;
      }
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(PartMode, "CU - part_mode");
#endif
  return PartMode;
}

/******************************************************************************
*
* Name:        GetNoResidualSyntaxFlag
*
******************************************************************************/
uint32
GetNoResidualSyntaxFlag(Parser_t *Parser_p)
{
  uint32 NoResidualSyntaxFlag;
  NoResidualSyntaxFlag = DecodeBin(Parser_p, Parser_p->CtxModels.NoResiDataFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(!NoResidualSyntaxFlag, "CU - rqt_root_cbf");
#endif
  return (!NoResidualSyntaxFlag);
}

/******************************************************************************
*
* Name:        GetPrevIntraLumaPredFlag
*
******************************************************************************/
uint32
GetPrevIntraLumaPredFlag(Parser_t *Parser_p)
{
  uint32 PrevIntraLumaPredFlag;
  PrevIntraLumaPredFlag = DecodeBin(Parser_p, Parser_p->CtxModels.PrevIntraLumaPredFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(PrevIntraLumaPredFlag, "CU - prev_intra_luma_pred_flag");
#endif
  return PrevIntraLumaPredFlag;
}

/******************************************************************************
*
* Name:        GetMpmIdx
*
******************************************************************************/
uint32
GetMpmIdx(Parser_t *Parser_p)
{
  uint32 MpmIdx;
  uint32 ValueTmp;
  MpmIdx   = 0;
  for( ; MpmIdx < 2; MpmIdx++)
  {
    ValueTmp = DecodeBinEP(Parser_p);
    if(ValueTmp == 0)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(MpmIdx, "CU - mpm_idx");
#endif
  return MpmIdx;
}

/******************************************************************************
*
* Name:        GetRemIntraLumaPredMode
*
******************************************************************************/
uint32
GetRemIntraLumaPredMode(Parser_t *Parser_p)
{
  uint32 RemIntraLumaPredMode;
  RemIntraLumaPredMode = DecodeBinsEP(Parser_p, 5);
#if D65_BITTRACE == 2
  TraceCabacSyntax(RemIntraLumaPredMode, "CU - rem_intra_luma_pred_mode");
#endif
  return RemIntraLumaPredMode;
}

/******************************************************************************
*
* Name:        GetIntraChromaPredMode
*
******************************************************************************/
uint8
GetIntraChromaPredMode(Parser_t *Parser_p)
{
  uint32 IntraChromaPredMode;
  IntraChromaPredMode = DecodeBin(Parser_p, Parser_p->CtxModels.IntraChromaPredModeCtx);
  if(IntraChromaPredMode == 0)
  {
    IntraChromaPredMode = 4;
  }
  else
  {
    IntraChromaPredMode = DecodeBinsEP(Parser_p, 2);
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(IntraChromaPredMode, "CU - intra_chroma_pred_mode");
#endif
  return (uint8)IntraChromaPredMode;
}

/******************************************************************************
*
* Name:        GetMergeIdx
*
******************************************************************************/
uint8
GetMergeIdx(Parser_t *Parser_p,
            const uint8 NumMergeCand)
{
  uint8 MergeUnaryIdx = 0;
  uint32 ValueTmp;
  
  ValueTmp = DecodeBin(Parser_p, Parser_p->CtxModels.MergeIdxCtx);
  if(ValueTmp == 0)
  {
#if D65_BITTRACE == 2
    TraceCabacSyntax(MergeUnaryIdx, "PU - merge_idx");
#endif
    return MergeUnaryIdx;
  }
  MergeUnaryIdx++;
  for(  ;MergeUnaryIdx < NumMergeCand - 1; MergeUnaryIdx++)
  {
    ValueTmp = DecodeBinEP(Parser_p);
    if(ValueTmp == 0)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(MergeUnaryIdx, "PU - merge_idx");
#endif
  return MergeUnaryIdx;
}

/******************************************************************************
*
* Name:        GetMergeFlag
*
******************************************************************************/
uint8
GetMergeFlag(Parser_t *Parser_p)
{
  uint8 MergeFlag;
  MergeFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.MergeFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(MergeFlag, "PU - merge_flag");
#endif
  return MergeFlag;
}

/******************************************************************************
*
* Name:        GetMinInterPredIdc
*
******************************************************************************/
uint8
GetMinInterPredIdc(Parser_t *Parser_p)
{
  uint8 InterPredIdc;
  uint32 Value;
  Value = DecodeBin(Parser_p, Parser_p->CtxModels.InterPredIdcCtx + 4);
  InterPredIdc = Value ? PRED_L1 : PRED_L0;
#if D65_BITTRACE == 2
  TraceCabacSyntax(InterPredIdc, "PU - inter_pred_idc");
#endif
  return InterPredIdc;
}

/******************************************************************************
*
* Name:        GetInterPredIdc
*
******************************************************************************/
uint8
GetInterPredIdc(Parser_t *Parser_p,
                const uint32 ctDepth)
{
  uint8 InterPredIdc;
  uint32 Value;
  Value = DecodeBin(Parser_p, Parser_p->CtxModels.InterPredIdcCtx + ctDepth);
  if(Value)
  {
    InterPredIdc = PRED_BI;
  }
  else 
  {
    Value = DecodeBin(Parser_p, Parser_p->CtxModels.InterPredIdcCtx + 4);
    InterPredIdc = Value ? PRED_L1 : PRED_L0;
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(InterPredIdc, "PU - inter_pred_idc");
#endif
  return InterPredIdc;
}

/******************************************************************************
*
* Name:        GetRefIdx
*
******************************************************************************/
uint32
GetRefIdx(Parser_t *Parser_p,
          const uint32 NumRefIdxMinus1)
{
  uint32 RefIdx;
  uint32 ValueTmp;
  ValueTmp = DecodeBin(Parser_p, Parser_p->CtxModels.RefIdxCtx);
  if(ValueTmp)
  {
    for(RefIdx = 0; RefIdx < NumRefIdxMinus1 - 1; RefIdx++)
    {
      if(RefIdx == 0)
      {
        ValueTmp = DecodeBin(Parser_p,Parser_p->CtxModels.RefIdxCtx + 1);
      }
      else
      {
        ValueTmp = DecodeBinEP(Parser_p);
      }
      if(ValueTmp == 0)
      {
        break;
      }
    }
    RefIdx++;
  }
  else
  {
    RefIdx = ValueTmp;
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(RefIdx, "PU - ref_idx_lx");
#endif
  return RefIdx;
}

/******************************************************************************
*
* Name:        GetAbsMvdGreater0Flag
*
******************************************************************************/
uint32
GetAbsMvdGreater0Flag(Parser_t *Parser_p)
{
  uint32 AbsMvdGreater0Flag;
  AbsMvdGreater0Flag = DecodeBin(Parser_p, Parser_p->CtxModels.MvdCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(AbsMvdGreater0Flag, "MVD - abs_mvd_greater0_flag");
#endif
  return AbsMvdGreater0Flag;
}

/******************************************************************************
*
* Name:        GetAbsMvdGreater1Flag
*
******************************************************************************/
uint32
GetAbsMvdGreater1Flag(Parser_t *Parser_p)
{
  uint32 AbsMvdGreater1Flag;
  AbsMvdGreater1Flag = DecodeBin(Parser_p, Parser_p->CtxModels.MvdCtx + 1);
#if D65_BITTRACE == 2
  TraceCabacSyntax(AbsMvdGreater1Flag, "MVD - abs_mvd_greater1_flag");
#endif
  return AbsMvdGreater1Flag;
}

/******************************************************************************
*
* Name:        GetAbsMvdMinus2
*
******************************************************************************/
uint32 
GetAbsMvdMinus2(Parser_t *Parser_p)
{
  uint32 AbsMvdMinus2;
  AbsMvdMinus2 = DecodeEpExGolomb(Parser_p, 1);
#if D65_BITTRACE == 2
  TraceCabacSyntax(AbsMvdMinus2, "MVD - abs_mvd_minus2");
#endif
  return AbsMvdMinus2;
}

/******************************************************************************
*
* Name:        GetMvdSignFlag
*
******************************************************************************/
uint32
GetMvdSignFlag(Parser_t *Parser_p)
{
  uint32 MvdSignFlag;
  MvdSignFlag = DecodeBinEP(Parser_p);
#if D65_BITTRACE == 2
  TraceCabacSyntax(MvdSignFlag, "MVD - mvd_sign_flag");
#endif
  return MvdSignFlag;
}

/******************************************************************************
*
* Name:        GetMvpLxFlag
*
******************************************************************************/
uint32
GetMvpLxFlag(Parser_t *Parser_p)
{
  uint32 MvpL0Flag;
  MvpL0Flag = DecodeBin(Parser_p, Parser_p->CtxModels.MvpFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(MvpL0Flag, "PU - mvp_lx_flag");
#endif
  return MvpL0Flag;
}

/******************************************************************************
*
* Name:        GetSplitTransformFlag
*
******************************************************************************/
uint32
GetSplitTransformFlag(Parser_t *Parser_p,
                      const uint32 Log2TrafoSize)
{
  uint32 CtxIdx = 5 - Log2TrafoSize;
  uint32 SplitTrasformFlag;
  SplitTrasformFlag = DecodeBin(Parser_p, Parser_p->CtxModels.SplitTransformFlagCtx + CtxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SplitTrasformFlag, "TU - split_transform_flag");
#endif
  return SplitTrasformFlag;
}

/******************************************************************************
*
* Name:        GetCbfChroma
*
******************************************************************************/
uint32
GetCbfChroma(Parser_t *Parser_p,
             const uint32 trafoDepth)
{
  uint32 CbfChroma;
  CbfChroma = DecodeBin(Parser_p, Parser_p->CtxModels.CbfChromaCtx + trafoDepth);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CbfChroma, "TU - cbf_chroma");
#endif
  return CbfChroma;
}

/******************************************************************************
*
* Name:        GetCbfLuma
*
******************************************************************************/
uint32
GetCbfLuma(Parser_t *Parser_p,
           const uint32 trafoDepth)
{
  uint32 CtxIdx = (trafoDepth == 0) ? 1 : 0;
  uint32 CbfLuma;
  CbfLuma = DecodeBin(Parser_p, Parser_p->CtxModels.CbfLumaCtx + CtxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CbfLuma, "TU - cbf_chroma");
#endif
  return CbfLuma;
}

/******************************************************************************
*
* Name:        GetCuQpDeltaAbs
*
******************************************************************************/
uint32
GetCuQpDeltaAbs(Parser_t * Parser_p)
{
  uint32 CuQpDeltaAbs = 0;
  uint32 ValueTmp     = DecodeBin(Parser_p, Parser_p->CtxModels.CuQpDeltaAbsCtx);

  if(ValueTmp == 0)
  {
    return CuQpDeltaAbs;
  }

  ++CuQpDeltaAbs;
  for( ; CuQpDeltaAbs < 5; ++CuQpDeltaAbs)
  {
    ValueTmp     = DecodeBin(Parser_p, Parser_p->CtxModels.CuQpDeltaAbsCtx + 1);
    if(ValueTmp == 0)
    {
      break;
    }
  }

  if(CuQpDeltaAbs > 4)
  {
    CuQpDeltaAbs += DecodeEpExGolomb(Parser_p, 0);
  }

  return CuQpDeltaAbs;
}

/******************************************************************************
*
* Name:        GetCuQpDeltaSignFlag
*
******************************************************************************/
uint8
GetCuQpDeltaSignFlag(Parser_t *Parser_p)
{
  uint8 CuQpDeltaSignFlag;
  CuQpDeltaSignFlag = (uint8)DecodeBinEP(Parser_p);
  return CuQpDeltaSignFlag;
}

/******************************************************************************
*
* Name:        GetTrSkipLumaFlag
*
******************************************************************************/
uint8
GetTrSkipLumaFlag(Parser_t *Parser_p)
{
  uint8 TrSkipLumaFlag;
  TrSkipLumaFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.TrafoSkipFlagCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(TrSkipLumaFlag, "Residual - transform_skip_flag");
#endif
  return TrSkipLumaFlag;
}

/******************************************************************************
*
* Name:        GetTrSkipChromaFlag
*
******************************************************************************/
uint8
GetTrSkipChromaFlag(Parser_t *Parser_p)
{
  uint8 TrSkipChromaFlag;
  TrSkipChromaFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.TrafoSkipFlagCtx + 1);
#if D65_BITTRACE == 2
  TraceCabacSyntax(TrSkipChromaFlag, "Residual - transform_skip_flag");
#endif
  return TrSkipChromaFlag;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixLuma32x32
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixXLuma32x32(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixContext = Parser_p->CtxModels.LastSigCoeffXLumaCtx + 10;
  for(Prefix = 0; Prefix < 9; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixContext + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixLuma16x16
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixXLuma16x16(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffXLumaCtx + 6;

  for(Prefix = 0; Prefix < 7; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixLuma8x8
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixXLuma8x8(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffXLumaCtx + 3;

  for(Prefix = 0; Prefix < 5; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixLuma4x4
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixXLuma4x4(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffXLumaCtx;

  for(Prefix = 0; Prefix < 3; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + Prefix);
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixChroma
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixXChroma(Parser_t *Parser_p,
                             const uint32 Log2TrafoSize)
{
  uint32 ValueTmp;
  uint32 ctxShift;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffXChromaCtx;

  const static uint32 GroupIdx[32]   = {0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9};

  ctxShift = Log2TrafoSize - 2;
  for(Prefix = 0; Prefix < GroupIdx[(1 << (Log2TrafoSize)) - 1]; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> ctxShift));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixYLuma32x32
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixYLuma32x32(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixContext = Parser_p->CtxModels.LastSigCoeffYLumaCtx + 10;
  for(Prefix = 0; Prefix < 9; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixContext + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixYLuma16x16
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixYLuma16x16(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffYLumaCtx + 6;

  for(Prefix = 0; Prefix < 7; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixYLuma8x8
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixYLuma8x8(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffYLumaCtx + 3;

  for(Prefix = 0; Prefix < 5; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> 1));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixYLuma4x4
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixYLuma4x4(Parser_t *Parser_p)
{
  uint32 ValueTmp;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffYLumaCtx;

  for(Prefix = 0; Prefix < 3; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + Prefix);
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffPrefixYChroma
*
******************************************************************************/
uint32
GetLastSigCoeffPrefixYChroma(Parser_t *Parser_p,
                             const uint32 Log2TrafoSize)
{
  uint32 ValueTmp;
  uint32 ctxShift;
  uint32 Prefix;
  Context *LastSigCoeffPrefixCtx = Parser_p->CtxModels.LastSigCoeffYChromaCtx;
  const static uint32 GroupIdx[32]   = {0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9};

  ctxShift = Log2TrafoSize - 2;
  for(Prefix = 0; Prefix < GroupIdx[(1 << (Log2TrafoSize)) - 1]; Prefix++)
  {
    ValueTmp = DecodeBin(Parser_p, LastSigCoeffPrefixCtx + (Prefix >> ctxShift));
    if(!ValueTmp)
    {
      break;
    }
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Prefix, "Residual - last_sig_coeff_prefix");
#endif
  return Prefix;
}

/******************************************************************************
*
* Name:        GetLastSigCoeffSuffix
*
******************************************************************************/
uint32
GetLastSigCoeffSuffix(Parser_t *Parser_p,
                      const uint32 LastSigCoeffPrefix)
{
  uint32 Suffix = 0;
  sint32 i;
  uint32 ValueTmp;

  for(i =((LastSigCoeffPrefix -2) >> 1) - 1; i >= 0; i--)
  {
    ValueTmp = DecodeBinEP(Parser_p);
    Suffix += (ValueTmp << i);
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(Suffix, "Residual - last_sig_coeff_suffix");
#endif
  return Suffix;
}

/******************************************************************************
*
* Name:        GetCodedSubBlockFlagLuma
*
******************************************************************************/
uint8
GetCodedSubBlockFlagLuma(Parser_t *Parser_p,
                         const uint32 SubBlockFlagContextIdx)
{
  uint8 CodedSubBlockFlag;
  CodedSubBlockFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.SigCoeffGroupFlagLumaCtx + SubBlockFlagContextIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CodedSubBlockFlag, "Residual - coded_sub_block_flag");
#endif
  return CodedSubBlockFlag;
}

/******************************************************************************
*
* Name:        GetCodedSubBlockFlagChroma
*
******************************************************************************/
uint8
GetCodedSubBlockFlagChroma(Parser_t *Parser_p,
                           const uint32 SubBlockFlagContextIdx)
{
  uint8 CodedSubBlockFlag;
  CodedSubBlockFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.SigCoeffGroupFlagChromaCtx + SubBlockFlagContextIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CodedSubBlockFlag, "Residual - coded_sub_block_flag");
#endif
  return CodedSubBlockFlag;
}

/******************************************************************************
*
* Name:        GetSigCoeffFlagLuma
*
******************************************************************************/
uint8
GetSigCoeffFlagLuma(Parser_t *Parser_p,
                    const uint32 ContextChoiceFlag,
                    const uint32 posInSubBlock,
                    const uint32 ContextOffset,
                    const uint32 Offset)
{
  uint32 sigCtx;
  uint8 SigCoeffFlag;

  const static uint32 contextTable[15] =
  {
    0,  1,  4,  5,
    2,  3,  4,  5,
    6,  6,  8,  8,
    7,  7,  8,  
  };

  const static uint32 cnt0Table[16] =
  {
    2,  1,  1,  0,
    1,  1,  0,  0,
    1,  0,  0,  0,
    0,  0,  0,  0
  };
  const static uint32 cnt1Table[16] =
  {
    2,  2,  2,  2,
    1,  1,  1,  1,
    0,  0,  0,  0,
    0,  0,  0,  0
  };
  const static uint32 cnt2Table[16] =
  {
    2,  1,  0,  0,
    2,  1,  0,  0,
    2,  1,  0,  0,
    2,  1,  0,  0
  };

  switch (ContextChoiceFlag)
  {
  case 0:
    switch (ContextOffset)
    {
    case 0:
      sigCtx = Offset + cnt0Table[posInSubBlock];
      break;
    case 1:
      sigCtx = Offset + cnt1Table[posInSubBlock];
      break;
    case 2:
      sigCtx = Offset + cnt2Table[posInSubBlock];
      break;
    default:
      sigCtx = Offset + 2;
    }
    break;
  case 1:
    sigCtx = 0;
    break;
  default:
    sigCtx = contextTable[posInSubBlock];
    break;
  }

  SigCoeffFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.SigCoeffFlagLumaCtx + sigCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SigCoeffFlag, "Residual - sig_coeff_flag");
#endif
  return SigCoeffFlag;
}

/******************************************************************************
*
* Name:        GetSigCoeffFlagChroma
*
******************************************************************************/
uint8
GetSigCoeffFlagChroma(Parser_t *Parser_p,
                      const uint32 Log2TrafoSize,
                      const uint32 SubBlockIdx,
                      const uint32 posInSubBlock,
                      const uint32 ContextOffset,
                      const uint32 Offset)
{
  uint32 sigCtx;
  uint8 SigCoeffFlag;

  const static uint32 contextTable[15] =
  {
    0,  1,  4,  5,
    2,  3,  4,  5,
    6,  6,  8,  8,
    7,  7,  8,  
  };

  const static uint32 cnt0Table[16] =
  {
    2,  1,  1,  0,
    1,  1,  0,  0,
    1,  0,  0,  0,
    0,  0,  0,  0
  };
  const static uint32 cnt1Table[16] =
  {
    2,  2,  2,  2,
    1,  1,  1,  1,
    0,  0,  0,  0,
    0,  0,  0,  0
  };
  const static uint32 cnt2Table[16] =
  {
    2,  1,  0,  0,
    2,  1,  0,  0,
    2,  1,  0,  0,
    2,  1,  0,  0
  };

  if (Log2TrafoSize == 2)
  {
    sigCtx = contextTable[posInSubBlock];
  }
  else if (SubBlockIdx ==0 && posInSubBlock == 0)
  {
    sigCtx = 0;
  }
  else
  {
    switch (ContextOffset)
    {
    case 0:
      sigCtx = Offset + cnt0Table[posInSubBlock];
      break;
    case 1:
      sigCtx = Offset + cnt1Table[posInSubBlock];
      break;
    case 2:
      sigCtx = Offset + cnt2Table[posInSubBlock];
      break;
    default:
      sigCtx = Offset + 2;
    }
  }

  SigCoeffFlag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.SigCoeffFlagChromaCtx + sigCtx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(SigCoeffFlag, "Residual - sig_coeff_flag");
#endif
  return SigCoeffFlag;
}

/******************************************************************************
*
* Name:        GetCoeffAbsLevelGreater1FlagLuma
*
******************************************************************************/
uint8
GetCoeffAbsLevelGreater1FlagLuma(Parser_t *Parser_p,
                                 const uint32 ctxIdx)
{
  uint8 CoeffAbsLevelGreater1Flag;
  CoeffAbsLevelGreater1Flag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.CoeffAbsLevelG1LumaCtx + ctxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffAbsLevelGreater1Flag, "Residual - coeff_abs_level_greater1_flag");
#endif
  return CoeffAbsLevelGreater1Flag;
}

/******************************************************************************
*
* Name:        GetCoeffAbsLevelGreater1FlagChroma
*
******************************************************************************/
uint8
GetCoeffAbsLevelGreater1FlagChroma(Parser_t *Parser_p,
                                   const uint32 ctxIdx)
{
  uint8 CoeffAbsLevelGreater1Flag;
  CoeffAbsLevelGreater1Flag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.CoeffAbsLevelG1ChromaCtx + ctxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffAbsLevelGreater1Flag, "Residual - coeff_abs_level_greater1_flag");
#endif
  return CoeffAbsLevelGreater1Flag;
}

/******************************************************************************
*
* Name:        GetCoeffAbsLevelGreater2FlagLuma
*
******************************************************************************/
uint8
GetCoeffAbsLevelGreater2FlagLuma(Parser_t *Parser_p,
                                 const uint32 ctxIdx)
{
  uint8 CoeffAbsLevelGreater2Flag;
  CoeffAbsLevelGreater2Flag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.CoeffAbsLevelG2LumaCtx + ctxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffAbsLevelGreater2Flag, "Residual - coeff_abs_level_greater2_flag");
#endif
  return CoeffAbsLevelGreater2Flag;
}

/******************************************************************************
*
* Name:        GetCoeffAbsLevelGreater2FlagChroma
*
******************************************************************************/
uint8
GetCoeffAbsLevelGreater2FlagChroma(Parser_t *Parser_p,
                                   const uint32 ctxIdx)
{
  uint8 CoeffAbsLevelGreater2Flag;
  CoeffAbsLevelGreater2Flag = (uint8)DecodeBin(Parser_p, Parser_p->CtxModels.CoeffAbsLevelG2ChromaCtx + ctxIdx);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffAbsLevelGreater2Flag, "Residual - coeff_abs_level_greater2_flag");
#endif
  return CoeffAbsLevelGreater2Flag;
}

/******************************************************************************
*
* Name:        GetCoeffSignFlag
*
******************************************************************************/
uint8
GetCoeffSignFlag(Parser_t *Parser_p)
{
  uint8 CoeffSignFlag;
  CoeffSignFlag = (uint8)DecodeBinEP(Parser_p);
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffSignFlag, "Residual - coeff_sign_flag");
#endif
  return CoeffSignFlag;
}

/******************************************************************************
*
* Name:        GetCoeffAbsLevelRemaining
*
******************************************************************************/
uint32
GetCoeffAbsLevelRemaining(Parser_t *Parser_p,
                          const uint32 golombRiceParam)
{
  uint32 prefix   = 0;
  uint32 codeWord = 0;
  uint32 CoeffAbsLevelRemaining;
  do
  {
    prefix++;
    codeWord = DecodeBinEP(Parser_p);
  }while(codeWord);

  codeWord  = 1 - codeWord;
  prefix -= codeWord;
  codeWord=0;

  if (prefix < 3)
  {
    codeWord = DecodeBinsEP(Parser_p, golombRiceParam);
    CoeffAbsLevelRemaining = (prefix<<golombRiceParam) + codeWord;
  }
  else
  {
    codeWord = DecodeBinsEP(Parser_p, prefix - 3 + golombRiceParam);
    CoeffAbsLevelRemaining = (((1 << (prefix-3)) + 3 - 1) << golombRiceParam) + codeWord;
  }
#if D65_BITTRACE == 2
  TraceCabacSyntax(CoeffAbsLevelRemaining, "Residual - coeff_abs_level_remaining");
#endif
  return CoeffAbsLevelRemaining;
}
