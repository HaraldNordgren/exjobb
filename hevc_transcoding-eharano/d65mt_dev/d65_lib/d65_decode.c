/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains high level decoding functions
*
******************************************************************************/



/******************************************************************************
* Includes
******************************************************************************/
#include <string.h>      /* Definition of memset */
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_getbits.h"
#include "d65_init.h"
#include "d65_decode_ps.h"
#include "d65_decode_slice.h"
#include "d65_decode_sei.h"
#include "d65_dpb.h"

/******************************************************************************
* Declaration of functions
******************************************************************************/
void
#if SUPPORT_EXTENSIONS
GetNALUnitType(Parser_t *Parser_p, NAL_TYPE *NalType, sint32 *LayerID, sint32 *TemporalID);
#else
GetNALUnitType(Parser_t *Parser_p, NAL_TYPE *NalType, sint32 *TemporalID);
#endif



/******************************************************************************
*
* Name:        GetNALUnitType
*
* Parameters:  Sessiondata_p [in/out]  The decoder
*
* Returns:     -
*
* Description: Decodes the NAL unit type
*
*
******************************************************************************/
void
#if SUPPORT_EXTENSIONS
GetNALUnitType(Parser_t *Parser_p, NAL_TYPE *NalType, sint32 *LayerID, sint32 *TemporalID)
#else
GetNALUnitType(Parser_t *Parser_p, NAL_TYPE *NalType, sint32 *TemporalID)
#endif
{
  if(GetBits(Parser_p, 1, "NALU header - forbidden_zero_bit") != 0)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "Forbidden zero bit is not zero");
  }
  *NalType = (NAL_TYPE)GetBits(Parser_p, 6, "NALU header - nal_unit_type");
#if SUPPORT_EXTENSIONS
  *LayerId = GetBits(Parser_p, 6, "NALU header - nuh_layer_id");
#else
  if(GetBits(Parser_p, 6, "NALU header - nuh_reserved_zero_6bits") != 0)
  {
    BIT_ERROR(*Parser_p->LongJumpSlice, D65_BIT_ERROR, "nuh_reserved_zero_6bits is not zero");
  }
#endif
  *TemporalID = GetBits(Parser_p, 3, "NALU header - nuh_temporal_id_plus1") - 1;
}


/******************************************************************************
*
* Name:        D65_DecodeNAL
*
******************************************************************************/
D65_ReturnCode_t D65_DecodeNAL(
                                 D65_Handle_t          *Decoder_p,
                                 const D65_InputNAL_t  *InputNAL_p,
                                 D65_ReturnImage_t    **DecodedPicture_pp
                              )
{
  SessionData_t      *Sessiondata_p = (SessionData_t*) Decoder_p;
  Parser_t           *Parser_p      = &Sessiondata_p->Parser;
  D65_ReturnCode_t    ReturnValue   = D65_OK;
  D65_DecoderMode_t   DecoderMode   = D65_DECODING_NALBYTE;
  D65_ExceptionCode_t ExceptionCode;
  NAL_TYPE            NALType;

  uint32 SliceStartCTUnr = 0;
  sint32 TemporalID      = 0;
  sint32 Quant           = 26;

#if SUPPORT_EXTENSIONS
  sint32 LayerID;
  sint32 i;
#endif

#if ENABLE_OPENHEVC
  sint8 GotPicture = 0;
#endif

#if SUPPORT_EXTENSIONS
  ReferenceSessiondata_p[Sessiondata_p->LayerID] = Sessiondata_p;
#endif

  *DecodedPicture_pp                 = NULL;
  Sessiondata_p->ReturnedPictureFlag = 0;
  Sessiondata_p->ExportDebugData     = InputNAL_p->ExportDebugData;

  /* Initilize bits reader for the new NAL */
  InitBitsReader(&Parser_p->BitsReader, InputNAL_p->NALData_p, InputNAL_p->NALSize);

  ExceptionCode = (D65_ExceptionCode_t)setjmp(Sessiondata_p->LongJumpSlice);
  if (ExceptionCode == 0)
  {
    Parser_p->LongJumpSlice = &Sessiondata_p->LongJumpSlice;

#if ENABLE_OPENHEVC
    if(Sessiondata_p->OpenHevcDecState >= 2)
    {
      GotPicture = libOpenHevcDecode(Sessiondata_p->OpenHevcDec.openHevcHandle, Sessiondata_p->CurrentBuffer_p , (int)InputNAL_p->NALSize, 0);
      if(GotPicture > 0)
      {
        libOpenHevcGetPictureInfo(Sessiondata_p->OpenHevcDec.openHevcHandle, &Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo);
        {
          sint32 CurrentPicW = Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo.nWidth;
          sint32 CurrentPicH = Sessiondata_p->OpenHevcDec.openHevcFrame.frameInfo.nHeight;
          if(AllocateOpenHevcReturnPic(Sessiondata_p, CurrentPicH, CurrentPicW))
          {
            FreeOpenHevcReturnPic(Sessiondata_p);
            BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_MALLOC_ERROR, "Error when allocating picture buffer for OpenHEVC");
          }
        }
        libOpenHevcGetOutputCpy(Sessiondata_p->OpenHevcDec.openHevcHandle, 1, &Sessiondata_p->OpenHevcDec.openHevcFrame);
        Sessiondata_p->ReturnedPictureFlag = 1;
        ReturnValue = D65_PICTURE_DECODED;
      }
    }
    else
    {
#endif

#if SUPPORT_EXTENSIONS
    GetNALUnitType(Parser_p, &NALType, &LayerID, &TemporalID);
#else
    GetNALUnitType(Parser_p, &NALType, &TemporalID);
#endif

#pragma message("TODO - Add support for more NAL unit types")
    switch(NALType)
    {
    case TRAIL_N:
    case TRAIL_R:
    case RASL_R: // Leading picture. Currently treated as regular trailing picture.
    case RASL_N:
    case TSA_R:
    case TSA_N:
        // non-IDR slice
      if(Sessiondata_p->PicOrderCntVal == -1)
      {
        // Make a fake IDR picture
        STB_WARNING("Error in bitstream - Lost first IDR picture");
        Sessiondata_p->CurrentRecImage->PicOrderCnt = 0;
        Sessiondata_p->CurrentRecImage->OutputMarking = NOT_NEEDED_FOR_OUTPUT;
        Sessiondata_p->CurrentRecImage->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
      }

      DecoderMode = D65_DECODING_SLICEHEADER;
      DecodeSliceHeader(Sessiondata_p, NALType, TemporalID, &SliceStartCTUnr, &Quant);

      Sessiondata_p->CurrentRecImage->Image.UserPictureValue = InputNAL_p->UserPictureValue;
      Sessiondata_p->CurrentRecImage->Image.PictureType      = Sessiondata_p->CurrentSliceType; // P- or B-frame

      DecoderMode = D65_DECODING_SLICEDATA;
      ReturnValue = DecodeSliceData(Sessiondata_p, SliceStartCTUnr, Quant);
      break;

    case IDR_W_RADL: // IDR that may have DLP
    case IDR_N_LP: // IDR without leading pictures
    case CRA_NUT:
      DecoderMode = D65_DECODING_SLICEHEADER;
      DecodeSliceHeader(Sessiondata_p, NALType, TemporalID, &SliceStartCTUnr, &Quant);

      Sessiondata_p->CurrentRecImage->Image.UserPictureValue = InputNAL_p->UserPictureValue;
      Sessiondata_p->CurrentRecImage->Image.PictureType      = MODE_INTRA; // I-frame

      DecoderMode = D65_DECODING_SLICEDATA;
      ReturnValue = DecodeSliceData(Sessiondata_p, SliceStartCTUnr, Quant);
      break;

#if SUPPORT_EXTENSIONS
    case VPS_NUT:
      // Video parameter set
      if(TemporalID != 0) // (HEVC section 7.4.1.2)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "TemporalID shall be zero when NALType is 25");
      }
      DecoderMode = D65_DECODING_VIDEOPARAMETERSET;
      DecodeVideoParameterSet(Sessiondata_p);
      break;
#endif

    case SPS_NUT:
      // Sequence parameter set
      if(TemporalID != 0) // (HEVC section 7.4.1.2)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "TemporalID shall be zero when NALType is 26");
      }
      DecoderMode = D65_DECODING_SEQUENCEPARAMETERSET;
      DecodeSequenceParameterSet(Sessiondata_p);
      break;

    case PPS_NUT:
      // Picture parameter set
      if(TemporalID != 0) // (HEVC section 7.4.1.2)
      {
        BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_BIT_ERROR, "TemporalID be zero when NALType is 27");
      }
      DecoderMode = D65_DECODING_PICTUREPARAMETERSET;

      DecodePictureParameterSet(Sessiondata_p);
      break;

    case PREFIX_SEI_NUT://PREFIX_SEI_NUT
    case SUFFIX_SEI_NUT://SUFFIX_SEI_NUT
      DecoderMode = D65_DECODING_SEI;
      return DecodeSEI(Sessiondata_p, NALType);
      break;

    default:
      return D65_UNKNOWN_NAL_TYPE;
    }

    if(Parser_p->BitsReader.CurrentBuffer_p != Parser_p->BitsReader.BufferEnd_p)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "NAL unit not byte aligned");
    }

    if(Parser_p->BitsReader.Buffer32_Pos != 0)
    {
      BIT_ERROR(Sessiondata_p->LongJumpSlice, D65_NOT_SUPPORTED_YET, "NAL unit not byte aligned");
    }

    // There should be no bytes left of the NAL after decoding it
    D65_ASSERT((Parser_p->BitsReader.CurrentBuffer_p == Parser_p->BitsReader.BufferEnd_p), "Should been checked in GetRbspTrailingBits for all NALs"); // Should never get here

    // There should be no bits left of the NAL after decoding it
    D65_ASSERT(Parser_p->BitsReader.Buffer32_Pos==0, "Remove this, checked in GetRbspTrailingBits for all NALs"); // Should never get here

#if ENABLE_OPENHEVC
  }
#endif
  }
  else
  {
    // Check for unknown exception codes
    switch(ExceptionCode)
    {
    case D65_BIT_ERROR:
    case D65_SYNTAX_VALUE_EXCEEDED:
    case D65_TRAILING_BITS_IN_NAL:
    case D65_READING_OUTSIDE_NAL:
    case D65_NOT_SUPPORTED_YET:
      break;
    case D65_PPS_ERROR:
      ReturnValue = D65_PICTURE_PARAMETER_SET_MISSING;
      break;
    case D65_SPS_ERROR:
      ReturnValue = D65_SEQUENCE_PARAMETER_SET_MISSING;
      break;
#if SUPPORT_EXTENSIONS
    case D65_VPS_ERROR:
      ReturnValue = D65_VIDEO_PARAMETER_SET_MISSING;
      break;
#endif
    case D65_SEI_ERROR:
      ReturnValue = D65_BITSTREAM_ERROR;
      break;
    case D65_MALLOC_ERROR:
      ReturnValue = D65_MEMORY_ALLOCATION_ERROR;
      break;
    default:
      D65_ASSERT(0, "Exceptioncode not taken care of"); // Should never get here
      break;
    }

    if(DecoderMode == D65_DECODING_SLICEDATA)
    {
      PictureDecoded(Sessiondata_p, NALType);
    }

    ReturnValue = D65_BITSTREAM_ERROR;

    D65_ASSERT(Sessiondata_p->NumberCTUDecodedForPic == 0, "Very important that NumberCTUDecodedForPic is 0 here");

    // If decoder got an exception during slicedata decoding, mark the
    // macroblock in the current slice as unknown. If an exception is
    // received and not in slicedata decoding, just ignore the NAL.
    if(DecoderMode == D65_DECODING_SLICEDATA)
    {
      /*
      for(CTUnr=0 ; CTUnr<Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMapUnits ; CTUnr++)
      {
      MbData_p = Sessiondata_p->MbDataArray + CTUnr;
      if(MbData_p->SliceNumber == CurrentSliceNumber)
      {
      MbData_p->MbType = UNKNOWN;
      // Decrease NumberCTUDecodedForPic. Since NumberCTUDecodedForPic
      // is increased first when a macroblock has been correctly decoded there may exist
      // macroblocks that have SliceGroup and SliceNumber set but than a biterror
      // prohibited NumberCTUDecodedForPic to be increased, therefore the > 0
      // check is needed (an alternative would be to increase the value of
      // NumberCTUDecodedForPic before the decoding of it is started, that is
      // when SliceGroup and SliceNumber is set in ResetMacroblockBeforeDecoding)
      if(Sessiondata_p->NumberCTUDecodedForPic > 0)
      {
      Sessiondata_p->NumberCTUDecodedForPic--;
      }
      }
      }
      */
    }
  }

  if(ReturnValue == D65_PICTURE_DECODED
#if ENABLE_OPENHEVC
    && Sessiondata_p->OpenHevcDecState == 0
#endif
    )
  {
    PictureDecoded(Sessiondata_p, NALType);
  }
#if SUPPORT_EXTENSIONS
  if ((ReturnValue == D65_PICTURE_DECODED || ReturnValue == D65_BITSTREAM_ERROR || ReturnValue == D65_ERROR) && Sessiondata_p->LayerID != 0
#if ENABLE_OPENHEVC
    && Sessiondata_p->OpenHevcDecState == 0
#endif
    )
  {
    for(i = 0; i < Sessiondata_p->Current_num_ref_idx_l0_active_minus1 + 1; i++)
    {
      if(Sessiondata_p->LongTermMarking)
        if(Sessiondata_p->RefPicList0[i]->PictureMarking == USED_FOR_LONG_TERM_REFERENCE)
          Sessiondata_p->RefPicList0[i]->PictureMarking = USED_FOR_SHORT_TERM_REFERENCE;
    }
    Sessiondata_p->LongTermMarking = false;
  }
#endif

  if(Sessiondata_p->ReturnedPictureFlag)
  {
    *DecodedPicture_pp = &(Sessiondata_p->DecodedPicture);
  }

  return ReturnValue;
}

/******************************************************************************
*
* Name:        D65_GetVersion
*
******************************************************************************/
void
D65_GetVersion(char version[50])
{

#if D65_BITTRACE
  char bittrace[] = "BITTRACE:ON";
#else
  char bittrace[] = "BITTRACE:OFF";
#endif
  if (version != 0)
  {
    if(sprintf(version, "%s %s", D65_VERSION, bittrace) >= 50)
    {
      STB_ERROR("Internal error - String size too small");
    }
  }
}
