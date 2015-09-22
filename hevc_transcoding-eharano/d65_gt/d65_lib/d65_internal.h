
#ifndef D65_INTERNAL_H
#define D65_INTERNAL_H


/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains global definitions that are used in the decoder but are
* kept separate from the D65 API since these definitions are not necessary
* for a user of the API to implement a decoder.
*
******************************************************************************/

#include "stdio.h"
#include <setjmp.h>
#include "d65_api.h"

#if defined(EXPORT_INTERNAL_DATA) && EXPORT_INTERNAL_DATA
//Avoid to include thread related data
#undef MULTI_THREADED
#define MULTI_THREADED 0
#endif//defined(EXPORT_INTERNAL_DATA) && EXPORT_INTERNAL_DATA

#if MULTI_THREADED
#include "pthread.h"
#endif

#if ENABLE_OPENHEVC
#include "openHevcWrapper.h"
#endif

#define D65_VERSION "0.01 alpha"

/******************************************************************************
* Macros
******************************************************************************/

#if !D65_BITTRACE
#define GetFlag(Parser_p, String)        GetBitsNoTrace(Parser_p, 1)
#define GetBits(Parser_p, Len, String)   GetBitsNoTrace(Parser_p, Len)
#define GetByte(Parser_p, String)        GetBitsNoTrace(Parser_p, 8)
#define GetUVLC(Parser_p, String)        GetUVLCNoTrace(Parser_p)
#define GetSVLC(Parser_p, String)        GetSVLCNoTrace(Parser_p)
#else
#define GetFlag(Parser_p, String)       GetBitsTrace(Parser_p, 1, String)
#define GetBits(Parser_p, Len, String)  GetBitsTrace(Parser_p, Len,  String)
#define GetByte(Parser_p, String)       GetBitsTrace(Parser_p, 8, String)
#define GetUVLC(Parser_p, String)       GetUVLCTrace(Parser_p, String)
#define GetSVLC(Parser_p, String)       GetSVLCTrace(Parser_p, String)
#endif

#define MAX_VPS_NUM        16
#define MAX_SPS_NUM        16
#define MAX_PPS_NUM        64

#define MAX_DPB_SIZE       16
#define MAX_RPS_IN_SPS     64
#define MAX_LTRPS_IN_SPS   32

#define MAX_SUB_LAYER      7
#define MAX_OPSET_POINTS   1024
#define MAX_LAYERS         10

#define PADDING            72
#define MAX_PU_SIZE        64
#define MAX_TU_SIZE        32
#define MIN_SIMD_SIZE      8
#define MAX_TILES_NUM      20
#define MAX_NUM_COTEXTS    512

#define MAX_GT_NAL_SIZE    300000
#define MAX_COEFFSM_Y_SIZE 2000

#define MAX_NUM_GREATER_ONE 8
#define MAX_CU_DEPTH        7
#define MAX_SCAN_TYPE       3

enum COEFF_SCAN_TYPE
{
  SCAN_DIAGONAL   = 0,
  SCAN_HORIZONTAL = 1,
  SCAN_VERTICAL   = 2
};

#ifdef _WIN32

#define SIMD               1
#define ALIGNED(x) __declspec(align(x))

#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))

#if defined(__SSE4_1__)
#define SIMD               1
#define ALIGNED(x) __attribute__((aligned(x)))
#else
#define SIMD               0
#endif

#else
#define SIMD               0
#endif


/******************************************************************************
*
* Name:        BIT_ERROR
*
* Description: Longjump macro that display error traces if code is compiled
*              in debug mode
*
******************************************************************************/

#ifdef _DEBUG
//#include <windows.h> // Fix: OutputDebugString changed to printf on line 62
#define BIT_ERROR(Label, ErrorCode, String)  { char string[200]; \
                                                   sprintf(string, "Exception in file %s at line %d:\n%s\n", __FILE__, __LINE__, String);\
                                                   printf(string);\
                                                   fprintf(stderr, "%s\n", string); \
                                                   longjmp(Label, ErrorCode); }
#else
#define BIT_ERROR(Label, ErrorCode, String)   longjmp(Label, ErrorCode)
#endif



/******************************************************************************
*
* Name:        STB_ERROR
*
* Description: abort macro that display error traces if code is compiled
*              in debug mode
*
******************************************************************************/
#ifdef _DEBUG
#define STB_ERROR(STRING) { fprintf(stderr, \
                           "Error in file %s, line %d:\n%s\n", \
                            __FILE__, __LINE__ , (STRING)); abort();}
#else
#define STB_ERROR(STRING) 
#endif


/******************************************************************************
*
* Name:        STB_WARNING
*
* Description: printf macro that display error traces if code is compiled
*              in debug mode
*
******************************************************************************/
#ifdef _DEBUG
#define STB_WARNING(STRING) { fprintf(stdout, \
                             "Minor error in file %s, line %d:\n%s\n", \
                              __FILE__, __LINE__ , (STRING)); }
#else
#define STB_WARNING(STRING) 
#endif


/******************************************************************************
*
* Name:        CLIP3
*
* Description: Given a lower (l) and an upper (u) bound, the function clips a variable (v).
*
******************************************************************************/
#define CLIP3(l, u, v) ((v) < (l) ? (l) :  (v) > (u) ? (u) : (v))

/******************************************************************************
*
* Name:        D65_ABS
*
* Description: Performs abs()
*
******************************************************************************/
#define D65_ABS(X) (((X)<0) ? -(X) : (X))

/******************************************************************************
*
* Name:        D65_MAX
*
* Description: Performs max(X,Y)
*
******************************************************************************/
#define D65_MAX(X, Y) (((X)<(Y)) ? (Y) : (X))

/******************************************************************************
*
* Name:        D65_MIN
*
* Description: Performs max(X,Y)
*
******************************************************************************/
#define D65_MIN(X, Y) (((X)<(Y)) ? (X) : (Y))

/******************************************************************************
*
* Name:        D65_SIGN
*
* Description: Performs sign(X,Y)
*
******************************************************************************/
#define D65_SIGN(X) (((X) > 0) ? (1) : (((X) == 0) ? (0) : (-1)))

/******************************************************************************
*
* Name:        D65_ASSERT
*
* Description: Evaluates an expression and crashes the decoder if it is false.
*
******************************************************************************/
#ifdef _WIN32
#define D65_ASSERT(EXPRESSION, STRING) if(!(EXPRESSION)) STB_ERROR(STRING);
#else
#define D65_ASSERT(EXPRESSION, STRING)
#endif

/******************************************************************************
* Types and constants
******************************************************************************/


/* Exception codes, used in setjmp and longjmp on bit-errors*/
typedef enum  
{
  D65_NO_ERROR,
  D65_BIT_ERROR,
  D65_SYNTAX_VALUE_EXCEEDED,
  D65_TRAILING_BITS_IN_NAL,
  D65_READING_OUTSIDE_NAL,
  D65_PPS_ERROR,
  D65_SPS_ERROR,
#if SUPPORT_EXTENSIONS
  D65_VPS_ERROR,
#endif
  D65_SEI_ERROR,
  D65_MALLOC_ERROR,
  D65_NOT_SUPPORTED_YET
} D65_ExceptionCode_t;

/* D65 decoding status*/
typedef enum  
{
  D65_DECODING_NALBYTE,
  D65_DECODING_SLICEHEADER,
  D65_DECODING_SLICEDATA,
  D65_DECODING_VIDEOPARAMETERSET,
  D65_DECODING_SEQUENCEPARAMETERSET,
  D65_DECODING_PICTUREPARAMETERSET,
  D65_DECODING_SEI
}
D65_DecoderMode_t;

/*YUV format*/
typedef enum
{
  YUV_400_MONOCHROME = 0,
  YUV_420,
  YUV_422,
  YUV_444
}
ChromaFormatIDC;

/*NAL Unit Types*/
typedef enum
{
  TRAIL_N = 0,       // 0
  TRAIL_R,           // 1
  TSA_N,
  TSA_R,
  STSA_N,
  STSA_R,
  RADL_N,
  RADL_R,
  RASL_N,
  RASL_R,
  RSV_VCL_N10,
  RSV_VCL_R11,
  RSV_VCL_N12,
  RSV_VCL_R13,
  RSV_VCL_N14,
  RSV_VCL_R15,
  BLA_W_LP,          // 16
  BLA_W_RADL,        // 17
  BLA_N_LP,          // 18
  IDR_W_RADL,        // 19
  IDR_N_LP,          // 20
  CRA_NUT,           // 21
  RSV_IRAP_VCL22,    // 22
  RSV_IRAP_VCL23,    // 23
  RSV_VCL24,
  RSV_VCL25,
  RSV_VCL26,
  RSV_VCL27,
  RSV_VCL28,
  RSV_VCL29,
  RSV_VCL30,
  RSV_VCL31,
  VPS_NUT,          // 32
  SPS_NUT,          // 33
  PPS_NUT,          // 34
  AUD_NUT,
  EOS_NUT,
  EOB_NUT,
  FD_NUT,
  PREFIX_SEI_NUT,   // 39
  SUFFIX_SEI_NUT,   // 40
  RSV_NVCL41,
  RSV_NVCL42,
  RSV_NVCL43,
  RSV_NVCL44,
  RSV_NVCL45,
  RSV_NVCL46,
  RSV_NVCL47,
  UNSPEC48,
  UNSPEC49,
  UNSPEC50,
  UNSPEC51,
  UNSPEC52,
  UNSPEC53,
  UNSPEC54,
  UNSPEC55,
  UNSPEC56,
  UNSPEC57,
  UNSPEC58,
  UNSPEC59,
  UNSPEC60,
  UNSPEC61,
  UNSPEC62,
  UNSPEC63,
}
NAL_TYPE;


typedef enum
{
  MODE_B      = 0,
  MODE_INTER  = 1,
  MODE_INTRA  = 2,
  NOT_DECODED = 3
}
PredictionMode_t;


typedef enum
{
  PRED_L0 = 0,
  PRED_L1 = 1,
  PRED_BI = 2
}
InterPredictionDir_t;


typedef enum
{
  INTRA_PLANAR = 0,
  INTRA_DC     = 1,
  INTRA_HOR    = 10,
  INTRA_VER    = 26
}
IntraPredictionMode_t;


typedef enum  
{
  PART_2Nx2N = 0,
  PART_2NxN  = 1,
  PART_Nx2N  = 2,
  PART_NxN   = 3,
  PART_2NxnU = 4,
  PART_2NxnD = 5,
  PART_nLx2N = 6,
  PART_nRx2N = 7
}
PartitionMode_t;

typedef enum
{
  EDGE_VER = 0,
  EDGE_HOR = 1
}
DeblockEdgeType_t;

typedef enum 
{
  CHANNEL_Y,
  CHANNEL_CB,
  CHANNEL_CR
}
YUVChannel;

typedef struct
{
  sint16 y;
  sint16 x;
  sint8  ref_idx;
} Vector_t;

typedef enum
{
  USED_FOR_SHORT_TERM_REFERENCE,
  USED_FOR_LONG_TERM_REFERENCE,
  UNUSED_FOR_REFERENCE
}
PictureMarking_t;

typedef enum
{
  NOT_NEEDED_FOR_OUTPUT,
  NEEDED_FOR_OUTPUT
}
OutputMarking_t;

typedef struct
{
  uint8 ProfileSpace;
  uint8 TierFlag;
  uint8 ProfileIdc;
  uint8 ProfileCompatibilityFlag[32];
  uint8 ProgressiveSrcFlag;
  uint8 InterlacedSrcFlag;
  uint8 NonPackedConstraintFlag;
  uint8 FrameOnlyConstraintFlag;
}
ProfileTierData_t;

typedef struct
{
  uint32 MaxLumsPs;
  uint32 MaxCPBSize;
  uint32 MaxSliceSegPerPic;
  uint32 MaxTileRows;
  uint32 MaxTileCols;
}
PTLLimitsData_t;

typedef struct
{
  ProfileTierData_t GeneralPT;
  uint32            GeneralLevelIdc;
  D65_Profile_t     GeneralProfile;
  PTLLimitsData_t   GeneralLimits;

  uint8             SubLayerProfilePresentFlag[MAX_SUB_LAYER-1];
  ProfileTierData_t SubLayerPT[MAX_SUB_LAYER-1];

  uint8             SubLayerLevelPresentFlag[MAX_SUB_LAYER-1];
  uint32            SubLayerLevelIdc[MAX_SUB_LAYER-1];

  D65_Profile_t     SubLayerProfile[MAX_SUB_LAYER-1];
  PTLLimitsData_t   SubLayerLimits[MAX_SUB_LAYER-1];
}
PTLData_t;

typedef struct
{
  Bool  SubPicParamsPresentFlag;
  Bool  NalParamsPresentFlag;
  Bool  VclParamsPresentFlag;
  Bool  SubPicCpbParamsPicTimingSeiFlag;
  Bool  LowDelayHrdFlag[MAX_SUB_LAYER-1];
  uint8 CpbCnt[MAX_SUB_LAYER-1];
  //Lengths
  uint8 InitialCpbRemovalDelayLength;
  uint8 AuCpbRemovalDelayLength;
  uint8 DpbOutputDelayLength;
  uint8 DpbOutputDuDelayLength;
  uint8 DuCpbRemovalDelayIncLength;
} HrdParameterSet_t;

typedef struct
{
  Bool FrameFieldInfoPresentFlag;
  //Timing params
  Bool TimmingInforPresentFlag;
  Bool HrdParametersPresentFlag;
  //HRD params
  HrdParameterSet_t HrdParameters;
} VuiParameterSet_t;

typedef struct
{
  uint8  MaxDecPicBuffering[MAX_SUB_LAYER];
  uint8  MaxNumReorderPics[MAX_SUB_LAYER];
  uint32 MaxLatencyIncrease[MAX_SUB_LAYER];
}
DpbData_t;

typedef struct
{
  uint8 ScalingList[4][6][64];
  //uint8 ScalingFactor[4][6][64];
}ScalingListData_t;

typedef struct
{
  uint8 pcm_sample_bit_depth_luma;
  uint8 pcm_sample_bit_depth_chroma;
  uint8 log2_min_pcm_cu_size;
  uint8 log2_max_pcm_cu_size;
  uint8 pcm_loof_filter_disabled_flag;
}PcmData_t;


typedef struct
{
  uint32 UserPictureValue;
  uint32 PictureType;

  uint32 Y_Width_image;
  uint32 Y_Height_image;

  uint32 C_Width_image;
  uint32 C_Height_image;

  uint8 *Y;
  uint8 *Cb;
  uint8 *Cr;

} YUVImage_t;

typedef struct
{
  YUVImage_t Image;

  PictureMarking_t  PictureMarking;
  OutputMarking_t   OutputMarking;
  uint8 RefMarkingUpdated;

  sint32 PicOrderCnt;

  uint8 *PredMode;
  Vector_t *MotionVectorL0;
  Vector_t *MotionVectorL1;
  sint32 RefPicPOCList0[MAX_DPB_SIZE];
  sint32 RefPicPOCList1[MAX_DPB_SIZE];
  PictureMarking_t RefPicMarkingList0[MAX_DPB_SIZE];
  PictureMarking_t RefPicMarkingList1[MAX_DPB_SIZE];
} ReferencePicture_t;


typedef struct
{

  uint32 NumStRefPics;
  uint32 NumLtRefPics;
  uint32 NumCurrRefPics;
  sint32 DeltaPOC[MAX_DPB_SIZE];
  //this array stores delta POC for short term reference picture and 
  //POC for long term reference picture.The array name is quite misleading. 
  uint8  UsedByCurrPicFlag[MAX_DPB_SIZE];
  uint8  DeltaPocMsbFlag[MAX_DPB_SIZE];
  PictureMarking_t  PictureMarking[MAX_DPB_SIZE];

} ReferencePictureSet_t;

typedef struct
{
  uint8  VideoParameterSetID;
  uint32 MaxLayers;
  uint8  MaxSubLayers;
  PTLData_t VpsPtl;
  DpbData_t DpbData;
  uint8 SubLDpbInfoPresent;
  uint32 MaxLayerId;

  uint8 DpbSize;
  uint8 NumReorderPics;
  Bool ScalibilityMask[16];
  uint32 NumOutputLayerSets;
  sint32 LayerIdNuh[MAX_LAYERS];
  sint32 LayerIdVps[MAX_LAYERS];
  uint32 ScalabilityId[MAX_LAYERS][16];
  Bool DirectDependencyFlag[MAX_LAYERS][MAX_LAYERS];
  sint32 RefLayerId[MAX_LAYERS][MAX_LAYERS];
  uint32 NumDirectRefLayers[MAX_LAYERS];
  uint32 ViewId[MAX_LAYERS];

} VideoParameterSet_t;

typedef struct
{
  uint8 VpsId;
  uint8 MaxSubLayers;
  PTLData_t SpsPtl;

  uint8  TemporalIdNestingFlag;
  uint8  Log2MaxPicOrderCntLsb; // Needed to parse sliceheader
  /* Partition variables */
  uint32 PictureWidth;
  uint32 PictureHeight;
  uint32 PicSizeInSamplesY;
  uint32 PicSizeInMinCbsY;
  uint32 PicHeightInMinCbsY;
  uint32 PicWidthInCtbsY; 
  uint32 PicHeightInCtbsY;
  uint32 PicWidthInMinCbsY;
  uint32 Log2MinCbSizeY;
  uint32 Log2CtbSizeY;
  uint32 MinCbSizeY;
  uint32 CtbSizeY;
  uint32 Log2MinTrafoSize;
  uint32 Log2MaxTrafoSize;
  uint32 Log2MinUnitSize;

  uint32 PicWidthInMinUnit;
  uint32 PicHeightInMinUnit;
  uint32 PicSizeInMinUnit;

  uint32 max_transform_hierarchy_depth_inter;
  uint32 max_transform_hierarchy_depth_intra;
  
  /* High Profile stuff */
  ChromaFormatIDC chromaFormatIDC;
  uint16 BitDepthLuma;
  uint16 BitDepthChroma;

  /* Crop stuff */
  uint16 pic_crop_left_offset;
  uint16 pic_crop_right_offset;
  uint16 pic_crop_top_offset;
  uint16 pic_crop_bottom_offset;

  /* Picture buffer variables */
  uint8 SubLDpbInfoPresent;
  DpbData_t DpbData;
  uint8 DpbSize;

  /* Reference picture sets */
  uint32 num_short_term_rps_sets;
  ReferencePictureSet_t RPSsetInSPS[MAX_RPS_IN_SPS];
  uint8 LongTermRefPicsPresentFlag;
  uint32 num_long_term_ref_pics_sps;
  uint32 LtRefPicPocLsbSps[MAX_LTRPS_IN_SPS];
  uint32 UsedByCurrPicLtSpsFlag[MAX_LTRPS_IN_SPS];

  /*flags for coding tools*/
  uint8 ScalingListEnabledFlag;
  uint8 ScalingListDataPresent;
  ScalingListData_t ScalingListData;
  uint8 AmpEnabledFlag;
  uint8 SampleAdaptiveOffsetEnabledFlag;
  uint8 PcmEnabledFlag;
  PcmData_t PcmData;
  uint8 TemporalMvpEnabledFlag;
  uint8 StrongIntraSmoothingFlag;

  /*VUI parameters*/
  Bool VuiParametersPresentFlag;
  VuiParameterSet_t VuiParameters;

} SequenceParameterSet_t;


typedef struct
{
  uint8 SequenceParameterSetID;
  uint8 PictureParameterSetID;

  uint8 DependentSliceSegEnabledFlag;
  uint8 OutputFlagPresentFlag;
  uint8 NumExtraSliceHeaderBits;
  uint8 SignDataHidingFlag;
  uint8 CabacInitFlag;
  uint8 num_ref_idx_l0_default_active_minus1;
  uint8 num_ref_idx_l1_default_active_minus1;
  uint8 PicInitQP;
  uint8 ConstrainedIntraPredFlag;
  uint8 TransformSkipFlag;
  uint8 CuQpDeltaEnabledFlag;
  uint8 DiffCuQpDeltaDepth;
  sint8 CbQpOffset;
  sint8 CrQpOffset;
  uint8 SliceChromaQpOffsetPresentFlag;
  uint8 WeightedPredFlag;
  uint8 WeightedBiPredFlag;
  uint8 transquant_bypass_enabled_flag;

  uint8 TilesEnabledFlag;
  uint8 EntropyCodingSyncEnabledFlag;
  uint32 num_tile_columns_minus1;
  uint32 num_tile_rows_minus1;
  uint8 uniform_spacing_flag;
  uint32 column_width[256];
  uint32 row_height[256];
  uint8 loop_filter_across_tiles_enabled_flag;

  uint8 loop_filter_across_slices_enabled_flag;
  uint8 deblocking_filter_control_present_flag;
  uint8 deblocking_filter_override_enabled_flag;
  uint8 pic_disable_deblocking_filter_flag;
  sint8 beta_offset;
  sint8 tc_offset;

  uint8 ScalingListPresentFlag;
  ScalingListData_t ScalListData;

  uint8 lists_modification_present_flag;
  uint8 log2_parallel_merge_level;
  uint8 slice_seg_header_extention_present_flag;

} PictureParameterSet_t;

typedef struct PredictionUnit PredictionUnit_t;
typedef struct PredictionUnit
{
  /* IMPORTANT */
  // If a variable is added here, make sure it is initialized
  // If the variable is constant (e.g. PUCoordinateY), initialize it in AllocateSequenceMemory()
  // If the variable is not constant (e.g. merge_idx), initialize it in ResetPUData()

  //Tile Info
  uint32 TileId;

  // Coding unit part
  uint32 CUSize;
  sint32 QpY;
  sint32 QpPrimeY;

  uint8 TransQuantBypass;
  uint8 skip_flag;
  uint8 PredMode;
  uint8 PartMode;

  //Copy of data that are reset after decoding a picture
  uint8 SavedPredMode;

  // Prediction unit related variables
  //uint32 PUCoordinateY;
  //uint32 PUCoordinateX;
  PredictionUnit_t *PredictionUnitLeft_p;
  PredictionUnit_t *PredictionUnitUp_p;
  PredictionUnit_t *PredictionUnitUpLeft_p;
  PredictionUnit_t *PredictionUnitUpRight_p;
  PredictionUnit_t *PredictionUnitDownLeft_p;
  //uint8 Position;

  // Inter related variables
  uint8 merge_idx;
  uint8 merge_flag;
  uint8 inter_pred_idc;
  Vector_t MotionvectorL0;
  Vector_t MotionvectorL1;
  
  // Intra related variables
  uint8 IntraPredMode;
  uint8 intraChromaPredMode;

  // Transform Unit related variables
  uint8 TUSize;
  uint8 CbfLuma;
};

typedef struct{
  uint32 m_usState;
}Context;

typedef struct{
  Context  ContextBuffer[MAX_NUM_COTEXTS];
  Context* SplitCuFlagCtx;
  Context* PrevIntraLumaPredFlagCtx;
  Context* IntraChromaPredModeCtx;
  Context* CbfLumaCtx;
  Context* CbfChromaCtx;
  Context* LastSigCoeffXLumaCtx;
  Context* LastSigCoeffYLumaCtx;
  Context* LastSigCoeffXChromaCtx;
  Context* LastSigCoeffYChromaCtx;
  Context* SigCoeffFlagLumaCtx;
  Context* SigCoeffFlagChromaCtx;
  Context* SigCoeffGroupFlagLumaCtx;
  Context* SigCoeffGroupFlagChromaCtx;
  Context* CoeffAbsLevelG1LumaCtx;
  Context* CoeffAbsLevelG1ChromaCtx;
  Context* CoeffAbsLevelG2LumaCtx;
  Context* CoeffAbsLevelG2ChromaCtx;
  Context* PredModeFlagCtx;
  Context* SkipFlagCtx;
  Context* MergeFlagCtx;
  Context* RefIdxCtx;
  Context* MvdCtx;
  Context* PartModeCtx;
  Context* MvpFlagCtx;
  Context* NoResiDataFlagCtx;
  Context* MergeIdxCtx;
  Context* SplitTransformFlagCtx;
  Context* SaoMergeCtx;
  Context* SaoTypeIdxCtx;
  Context* TrafoSkipFlagCtx;
  Context* CuTrQBypassFlagCtx;
  Context* InterPredIdcCtx;
  Context* CuQpDeltaAbsCtx;
}ContextModels;

typedef struct{

  uint32 Range;
  uint32 Value;
  sint32 BitsNeeded;

}BacDecoder_t;

typedef struct{

  uint32 Range;
  uint32 LowValue;
  uint32 NumBufferedBits;
  uint32 NumBufferedBytes;
  uint32 BufferedByte;

}BacEncoder_t;

typedef struct{

  const uint8 *BufferEnd_p;
  const uint8 *CurrentBuffer_p;
  uint32 Buffer32;
  sint32 Buffer32_Pos;
  uint32 PrevByte1;
  uint32 PrevByte2;

}BitsReader_t;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
typedef struct{

  uint32  BufferPos;
  uint8  *BufferStart_p;

  uint32  CurrByte;
  uint32  CurrByte_Pos;

}BitsOutputer_t;

typedef struct{

  ContextModels   CtxModels;
  BacEncoder_t    BacEncoder;
  BitsOutputer_t  BitsOutputer;
  uint8           OutputMode;

}SubEncoder_t;
#endif

typedef struct{

  ContextModels CtxModels;
  BacDecoder_t BacDecoder;
  BitsReader_t BitsReader;
  jmp_buf *LongJumpSlice;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  SubEncoder_t SubEncoder;
#endif

}Parser_t;

typedef struct{

  sint32 QgX;
  sint32 QgY;
  sint32 PrevQpY;
  sint32 CurrQpY;
  sint32 CuQpDeltaVal;
  uint8  IsCuQpDataCoded;

}QuantGroupData_t;

#if ENABLE_OPENHEVC
typedef struct{
  OpenHevc_Frame_cpy openHevcFrame;
  OpenHevc_Handle    openHevcHandle;
  YUVImage_t ReturnPic;
}OpenHevcData_t;
#endif

#if MULTI_THREADED

typedef struct
{
  uint32 Bytes;
  uint32 Id;

}JobsMapping;

typedef void*JobsToBeDone(void*);

typedef struct{

  uint32 TileStartCtuX;
  uint32 TileStopCtuX;
  uint32 TileStartCtuY;
  uint32 TileStopCtuY;
#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  uint8 *TileStream;
  uint32 TileStreamSize;
#endif
}TileData_t;

typedef struct{

  void   *Sessiondata_p;

  uint32 NumberCtuDecodedForThread;
  sint16 *TransformArray;
  sint16 *ResTrQuantArray;
  sint16 *Interpl;
  sint16 *PredSampleArrayL0;
  sint16 *PredSampleArrayL1;

  Parser_t   Parser;
  TileData_t *Tiledata_p;
  uint32     TilesNumForThisThread;
  
  QuantGroupData_t QgData;

  jmp_buf LongJumpSlice;

}ThreadData_t;


typedef struct{

  uint32 CurrentJobId;
  uint32 TotalJobNumber;
  uint32 RemainingJobsNumber;
  ThreadData_t *JobsDataArray_p;
  JobsMapping *JobsMapArray_p;

  pthread_t *ThreadsArray;

  pthread_mutex_t ThreadLock;
  pthread_cond_t  JobsArrayNotEmpty;
  pthread_cond_t  JobsArrayEmpty;

  JobsToBeDone*  Jobs_p;

  uint8 RetireFlag;

}ThreadPool_t;


#endif
/* SessionData */
typedef struct 
{
  VideoParameterSet_t    *VideoParameterSetArray[MAX_VPS_NUM];
  SequenceParameterSet_t *SequenceParameterSetArray[MAX_SPS_NUM];
  PictureParameterSet_t  *PictureParameterSetArray[MAX_PPS_NUM];

  VideoParameterSet_t    *CurrentVideoParameterSet;
  SequenceParameterSet_t *CurrentSequenceParameterSet;
  PictureParameterSet_t  *CurrentPictureParameterSet;

  uint32 AllocatedImageHeight;
  uint32 AllocatedImageWidth;
  uint8  AllocatedDpbSize;

  uint32 colBd[257];
  uint32 rowBd[257];

  D65_MallocPtr Malloc_Function_p;
  D65_FreePtr   Free_Function_p;

#if D65_CONFORMANCE
  void (*Write_Output_Function_p)(D65_ReturnImage_t *ReturnImage_p, void *OutputLocation);
  void *OutputLocation;
#endif

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  void (*Write_Nal_Function_p)(void *OuputLocation, uint8 *NalBuffer, uint32 NalLength);
  void *NalOutputLocation;
  uint32 NalBufferSize;
  uint8 *NalOutputBuffer;
  uint32 NalOutputBufferPos;
#endif

#if GT_RESIDUAL_RECONSTRUCT
  FILE   *InputYuvFile_p;

  sint32 *g_xScan[MAX_SCAN_TYPE][ MAX_CU_DEPTH ];
  sint32 *g_yScan[MAX_SCAN_TYPE][ MAX_CU_DEPTH ];

  sint16 *ResTrQuantArray;

  uint32 FrameNum;

  YUVImage_t SourceYUV;
#endif
  /* Longjump variables */
  jmp_buf LongJumpSlice;

  /* Pictures and picture data */
  ReferencePicture_t *CurrentRecImage;
  ReferencePicture_t  DecodedPictureBuffer[MAX_DPB_SIZE];
  ReferencePicture_t *RefPicList0[MAX_DPB_SIZE];
  ReferencePicture_t *RefPicList1[MAX_DPB_SIZE];

  /* POC stuff */
  sint32 PicOrderCntVal;
  sint32 PrevTid0PicPoc;

  uint8 no_output_of_prior_pics_flag;

  /* Current slice information */
  uint16 CurrentSliceNumber;
  uint8  CurrentSliceType;
  sint8  Current_num_ref_idx_l0_active_minus1;
  sint8  Current_num_ref_idx_l1_active_minus1;
  uint8  CurrentMvdL1ZeroFlag;
  uint8  CurrentMaxNumMergeCand;
  uint8  CurrentSliceDisableDeblockingFlag;
  sint8  CurrentCollocatedRefIdx;
  uint8  CurrentCollocatedFromL0Flag;
  uint8  CurrentSliceSaoLumaFlag;
  uint8  CurrentSliceSaoChromaFlag;
  uint8  CurrentLoopfilterAcrossSliceFlag;
  uint8  CurrentCabacInitFlag;
  /* Variables dealing with decoded pictures */
  uint8 ReturnedPictureFlag;
  D65_ReturnImage_t DecodedPicture;

  /* PU array */
  PredictionUnit_t *PredictionUnitArray;
  uint32 NumberCTUDecodedForPic;

  /* Sub-pel interpolation array */
  sint16 *Interpl;

  /* PredSampleArray*/
  sint16 *PredSampleArrayL0;
  sint16 *PredSampleArrayL1;

  /* TU data */
  sint16 *TransformArray;
  sint32 CurrentQuant;

  /* Deblocking Stuff*/
  uint8 *EdgeFlags;
  uint8 *EdgeFlagsHor;

  uint8 *Bs;

  /* SAO Stuff */
  uint8  *SaoTypeIdx[2];
  uint8  *SaoTypeClass[3];
  sint16 *SaoOffsetVal[3][5];
  sint16 *SaoOffsetToUse;
  uint32 PicWidthModCtbSize;
  uint32 PicHeightModCtbSize;

  uint32 Log2MinCuQpDeltaSize;

  // Quantization Group Data
  QuantGroupData_t QgData;

  /* Bytebuffer stuff */
  Parser_t Parser;

#if SUPPORT_EXTENSIONS
  uint32 LayerID;
  Bool LongTermMarking;
#endif

  Bool ExportDebugData;

#if MULTI_THREADED
  ThreadPool_t ThreadsPool;
  JobsMapping  JobsMapArray[MAX_TILES_NUM];
  TileData_t   CurrentTiledata_p[MAX_TILES_NUM];
  ThreadData_t *Threaddata_p;
  uint32 CurrentNumOfThreads;
  uint32 CurrentNumOfEntries;
  uint32 CurrentTileNum;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  uint8  EntryNeedRewrite;
  uint32 EntryOffsetLen;
  uint32 BitOffsetBeforeEntry;
  uint32 ByteValue;
  const uint8 *BytePosBeforeEntry;
#endif

#endif

  uint32 CurrentTileColumnMinus1;
  uint32 CurrentTileRowMinus1;
  uint32 CurrentUniformSpacingFlag;

#if ENABLE_OPENHEVC
  uint8 OpenHevcDecState;
  OpenHevcData_t OpenHevcDec;
#endif
}
SessionData_t;

static const sint32 transformLastScan32x32[ 64 ] =
{
  0,     8,    1,   16,    9,    2,   24,   17,
  10,    3,   32,   25,   18,   11,    4,   40,
  33,   26,   19,   12,    5,   48,   41,   34,
  27,   20,   13,    6,   56,   49,   42,   35,
  28,   21,   14,    7,   57,   50,   43,   36,
  29,   22,   15,   58,   51,   44,   37,   30,
  23,   59,   52,   45,   38,   31,   60,   53,
  46,   39,   61,   54,   47,   62,   55,   63
};
#if D65_BITTRACE
  /* Trace stuff */
  uint32 TraceFlag;
  uint32 TraceBitCounter;
  FILE  *Tracefile_p;
#endif
#ifndef EXPORT_INTERNAL_DATA
SessionData_t *ReferenceSessiondata_p[MAX_LAYERS];
#endif//EXPORT_INTERNAL_DATA

#endif  /* D65_INTERNAL_H */
