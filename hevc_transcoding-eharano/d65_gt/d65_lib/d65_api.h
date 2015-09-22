#ifndef D65_API_H
#define D65_API_H

/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file defines the API for D65. The structs and functions defined here may
* be used for implementation of decoders, transcoders etc.
*
******************************************************************************/

#define SUPPORT_EXTENSIONS 0 /* Enable multi-view decoding support */


#define MULTI_THREADED     1 /* Enable multi-threading decoding */

#define D65_BITTRACE       0 /* 0: NO INFO
                              * 1: VPS, SPS, PPS, SH INFO
                              * 2: 1 + CABAC SYNTAX INFO
                              * 3: CABAC STATUS INFO
                              */

#define ENABLE_OPENHEVC    0 /*Use openhevc lib for decoding*/

#define D65_CONFORMANCE    0

#define EXTRA_INFO         0

/* How to access internal data:
 * Define EXPORT_INTERNAL_DATA in your app: 
 *   - Add #define EXPORT_INTERNAL_DATA 1 before including <d65_api.h>
 *     If EXPORT_INTERNAL_DATA is true, d65 will allow access to its internal data,
 *     notably by casting D65_Handler* to SessionData_t*:
 *     const SessionData_t * Sessiondata_p = (const SessionData_t *)D65_Handle_t;
 * Notes:
 *   - Never define EXPORT_INTERNAL_DATA in d65 lib, only in the app if needed
 *   - Don't forget to copy d65_internal.h together with d65_api.h and d65_lib.lib
 *     in your app if you set EXPORT_INTERNAL_DATA to 1
 */


/* Some guidelines on how to use d65's guided transcoding functionality:

   1. Perform decoding of high-quality layer: (Output decoded YUV in decoding order!)
       - Set D65_OUTPUT_IN_DEC_ORDER to 1
      -  Set GT_RESIDUAL_PRUNING     to 0
      -  Set GT_RESIDUAL_RECONSTRUCT to 0

   2. Perform residual prunning:
       - Set D65_OUTPUT_IN_DEC_ORDER to 0
      -  Set GT_RESIDUAL_PRUNING     to 1
      -  Set GT_RESIDUAL_RECONSTRUCT to 0

   3. Perform residual reconstruction:
       - Set D65_OUTPUT_IN_DEC_ORDER to 0
      -  Set GT_RESIDUAL_PRUNING     to 0
      -  Set GT_RESIDUAL_RECONSTRUCT to 1

*/

#define D65_OUTPUT_IN_DEC_ORDER 0 // read the guideline above before use !
#define GT_RESIDUAL_PRUNING     0 // read the guideline above before use !
#define GT_RESIDUAL_RECONSTRUCT 1 // read the guideline above before use !

/******************************************************************************
* Includes
******************************************************************************/
#include "stdlib.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Types and constants
******************************************************************************/
typedef void D65_Handle_t;

typedef  signed char    sint8;    /*  signed    8 bit data entity  */
typedef  unsigned char  uint8;    /*  unsigned  8 bit data entity. */
typedef  signed short   sint16;   /*  signed   16 bit data entity. */
typedef  unsigned short uint16;   /*  unsigned 16 bit data entity. */
typedef  signed long    sint32;   /*  signed   32 bit data entity. */
typedef  unsigned long  uint32;   /*  unsigned 32 bit data entity. */
typedef  unsigned char  Bool;     /*  boolean         data entity. */

typedef enum
{
  D65_PROFILE_UNKNOWN,
  D65_PROFILE_MAIN,
  D65_PROFILE_MAIN_10,
  D65_PROFILE_STILL_PICTURE
}
D65_Profile_t;


enum SliceType
{
  B_SLICE,
  P_SLICE,
  I_SLICE
};

typedef enum
{
  D65_OK,
  D65_SLICE_DECODED,
  D65_PICTURE_DECODED,
  D65_BITSTREAM_ERROR,
  D65_SEQUENCE_PARAMETER_SET_MISSING,
  D65_PICTURE_PARAMETER_SET_MISSING,
#if SUPPORT_EXTENSIONS
  D65_VIDEO_PARAMETER_SET_MISSING,
#endif
  D65_UNKNOWN_SEI,
  D65_UNKNOWN_NAL_TYPE,
  D65_MEMORY_ALLOCATION_ERROR,
  D65_ERROR
}
D65_ReturnCode_t;

typedef struct
{
  uint32 UserPictureValue;
  uint32 Y_Height_image;
  uint32 Y_Width_image;
  uint32 Y_Width_memory;
  uint32 PictureType;
  uint8 *Y;
  uint8 *Cb;
  uint8 *Cr;
} D65_ReturnImage_t;

typedef enum 
{
  D65_INTERNAL_DATA,
  D65_IMAGE_DATA
}
D65_Mem_Type_t;

typedef void*
(*D65_MallocPtr)(const uint32, const D65_Mem_Type_t);

typedef void
(*D65_FreePtr)  (void*, const D65_Mem_Type_t);

#if D65_CONFORMANCE
typedef void
(*D65_WriteOutputPtr)(D65_ReturnImage_t *ReturnImage_p, void * OutputLocation);
#endif

typedef void
(*D65_WriteNalPtr)(void *OuputLocation, uint8 *NalBuffer, uint32 NalLength);

// The D65_DecoderParameters_t struct is used when an encoder is initialized
// as a parameter to D65_Initialize()
typedef struct 
{
  D65_MallocPtr Malloc_Function_p;
  D65_FreePtr   Free_Function_p;

#if GT_RESIDUAL_PRUNING || GT_RESIDUAL_RECONSTRUCT
  uint32 NalBufferSize;
  D65_WriteNalPtr Write_Nal_Function_p;
  void *NalOutputLocation;
#endif

#if GT_RESIDUAL_RECONSTRUCT
  FILE *InputYuvFile_p; 
#endif

#if D65_BITTRACE
  FILE   *Tracefile_p;
  uint32  TraceFlag;
#endif

#if D65_CONFORMANCE
  D65_WriteOutputPtr Write_Output_Function_p;
  void *OutputLocation;
#endif

#if SUPPORT_EXTENSIONS
  uint32 LayerID;
#endif

#if MULTI_THREADED
  uint32 Threads_Number;
#endif

#if ENABLE_OPENHEVC
  uint8 OpenHevcFlag;
#endif

} D65_DecoderParameters_t;

typedef struct
{
  uint8  *NALData_p;
  uint32  NALSize;
  uint32  UserPictureValue;
  Bool    ExportDebugData;
  uint8   LastSliceInPictureFlag;
} D65_InputNAL_t;



/******************************************************************************
* Declaration of functions
******************************************************************************/

/******************************************************************************
*
* Name:        D65_Initialize
*
* Parameters:  Decoder_pp        [In/Out]  The decoder
*              Parameters_p      [In]      The encoder parameters
*
* Returns:     D65_ReturnCode_t  [Ret]     Return value
*
* Description: This function allocates the decoder and sets it up to be able to
*              start decoding 
*
******************************************************************************/
D65_ReturnCode_t D65_Initialize(D65_Handle_t** Decoder_pp, 
                                const D65_DecoderParameters_t *Parameters_p);


/******************************************************************************
*
* Name:        D65_Release
*
* Parameters:  Decoder_p        [In/Out]  The decoder to release
*
* Returns:     D65_ReturnCode_t [Ret]     Return value
*
* Description: Releases the decoder, free all memory allocated for the decoder.
*
******************************************************************************/
D65_ReturnCode_t D65_Release(D65_Handle_t* Decoder_p);


/******************************************************************************
*
* Name:        D65_DecodeNAL
*
* Parameters:  Decoder_p           [In/Out]  The decoder
*              InputNAL_p          [In]      Pointer to NAL data
*              DecodedPicture_pp   [Out]     Returning a pointer to a decoded image
*
* Returns:     D65_ReturnCode_t    [Ret]     Return value
*
* Description: Decodes one NAL. In the case of a resulting decoded picture,
*              a pointer to the picture will be available in *DecodedPicture_pp.
*              It will be NULL if there is no resulting decoded picture.
*
******************************************************************************/
D65_ReturnCode_t D65_DecodeNAL(D65_Handle_t* Decoder_p,
                               const D65_InputNAL_t *InputNAL_p,
                               D65_ReturnImage_t **DecodedPicture_pp);

/******************************************************************************
*
* Name:        D65_ForceOutOneDecodedPicture
*
* Parameters:  Decoder_p          [In]   The decoder
*
* Returns:     D65_ReturnImage_t  [Ret]  Returning a pointer to a decoded image
*
* Description: Get the next decoded picture in output order from the decoder
*              when the session has ended. NULL will be returned if there are
*              no more pictures.
*
******************************************************************************/
D65_ReturnImage_t *
D65_ForceOutOneDecodedPicture(D65_Handle_t* Decoder_p);


/******************************************************************************
*
* Name:        D65_GetWidthAndHeight
*
* Parameters:  Decoder_p        [In/Out]  The decoder to release
*
* Returns:     Width            [Ret]     Picture width
*              Height           [Ret]     Picture height
*
* Returns:     D65_ReturnCode_t [Ret]     Return value
*
* Description: Returns the width and height of the current sequence parameter set
*
******************************************************************************/
D65_ReturnCode_t D65_GetWidthAndHeight(D65_Handle_t* Decoder_p,
                                       int *Width,
                                       int *Height);


/******************************************************************************
*
* Name:        D65_GetVersion
*
* Parameters:  version        [Ret]  ASCII string 
*
* Description: Returns the version as an ascii string
*
******************************************************************************/
void
D65_GetVersion(char version[50]);


#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(EXPORT_INTERNAL_DATA) && EXPORT_INTERNAL_DATA
#include "d65_internal.h"
#endif//defined(EXPORT_INTERNAL_DATA) && EXPORT_INTERNAL_DATA

#endif  /* D65_API_H */

