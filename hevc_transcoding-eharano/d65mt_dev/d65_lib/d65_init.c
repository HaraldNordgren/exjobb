/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains initialization and memory handeling functions
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <string.h> /* memset */
#include "d65_api.h"
#include "d65_internal.h"
#include "d65_init.h"
#include "d65_context.h"
#if MULTI_THREADED
#include "pthread.h"
#endif


/******************************************************************************
* Declaration of functions
******************************************************************************/
uint16
Allocate_Image(const SessionData_t *Sessiondata_p,
               ReferencePicture_t *RefImage,
               const uint32 Y_height_mb,
               const uint32 Y_width_mb,
               const uint32 NumUnit);

void
Free_Image(const SessionData_t *Sessiondata_p,
           ReferencePicture_t *RefImage);

void*
Default_Malloc(const uint32 Size,
               const D65_Mem_Type_t Type);

void
Default_Free(void* Data_p,
             const D65_Mem_Type_t Type);



/******************************************************************************
* Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        Allocate_Image
*
* Parameters:  Sessiondata_p [in]  The decoder
*              Image         [out] The Image to set up
*
* Returns:     uint16        [Out] 0-Allocation succeeded  1-Allocation failed
*
* Description: Allocates an image.
*
******************************************************************************/
uint16
Allocate_Image(const SessionData_t *Sessiondata_p,
               ReferencePicture_t *RefImage,
               const uint32 Y_height,
               const uint32 Y_width,
               const uint32 NumUnit)
{
  uint32 Y_Height_image = Y_height;
  uint32 Y_Width_image  = Y_width;
  uint32 C_Height_image = Y_height>>1;
  uint32 C_Width_image  = Y_width>>1;

  RefImage->Image.Y_Height_image = Y_Height_image;
  RefImage->Image.Y_Width_image  = Y_Width_image;
  RefImage->Image.C_Height_image = C_Height_image;
  RefImage->Image.C_Width_image  = C_Width_image;

  RefImage->Image.Y  = (uint8*) Sessiondata_p->Malloc_Function_p(Y_Width_image*Y_Height_image, D65_IMAGE_DATA);
  if(RefImage->Image.Y == NULL)
  {
    return 1;
  }
  RefImage->Image.Cb = (uint8*) Sessiondata_p->Malloc_Function_p(C_Width_image*C_Height_image, D65_IMAGE_DATA);
  if(RefImage->Image.Cb == NULL)
  {
    return 1;
  }
  RefImage->Image.Cr = (uint8*) Sessiondata_p->Malloc_Function_p(C_Width_image*C_Height_image, D65_IMAGE_DATA);
  if(RefImage->Image.Cr == NULL)
  {
    return 1;
  }

  RefImage->MotionVectorL0 = (Vector_t*) Sessiondata_p->Malloc_Function_p(NumUnit*sizeof(Vector_t), D65_IMAGE_DATA);
  if(RefImage->MotionVectorL0 == NULL)
  {
    return 1;
  }

  RefImage->MotionVectorL1 = (Vector_t*) Sessiondata_p->Malloc_Function_p(NumUnit*sizeof(Vector_t), D65_IMAGE_DATA);
  if(RefImage->MotionVectorL1 == NULL)
  {
    return 1;
  }

  RefImage->PredMode = (uint8*) Sessiondata_p->Malloc_Function_p(NumUnit*sizeof(uint8), D65_IMAGE_DATA);
  if(RefImage->PredMode == NULL)
  {
    return 1;
  }

  memset(RefImage->Image.Y,  128, Y_Width_image*Y_Height_image);
  memset(RefImage->Image.Cb, 128, C_Width_image*C_Height_image);
  memset(RefImage->Image.Cr, 128, C_Width_image*C_Height_image);

  return 0;
}


/******************************************************************************
*
* Name:        Free_Image
*
* Parameters:  Sessiondata_p [in]  The decoder
*              Image         [out] The Image to free
*
* Returns:     -
*
* Description: Free an images
*
******************************************************************************/
void
Free_Image(const SessionData_t *Sessiondata_p, ReferencePicture_t *RefImage)
{
  if(RefImage->Image.Y != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->Image.Y, D65_IMAGE_DATA);
    RefImage->Image.Y = NULL;
  }
  if(RefImage->Image.Cb != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->Image.Cb, D65_IMAGE_DATA);
    RefImage->Image.Cb = NULL;
  }
  if(RefImage->Image.Cr != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->Image.Cr, D65_IMAGE_DATA);
    RefImage->Image.Cr = NULL;
  }
  if(RefImage->MotionVectorL0 != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->MotionVectorL0, D65_IMAGE_DATA);
    RefImage->MotionVectorL0 = NULL;
  }
  if(RefImage->MotionVectorL1 != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->MotionVectorL1, D65_IMAGE_DATA);
    RefImage->MotionVectorL1 = NULL;
  }

  if(RefImage->PredMode != NULL)
  {
    Sessiondata_p->Free_Function_p(RefImage->PredMode, D65_IMAGE_DATA);
    RefImage->PredMode = NULL;
  }
}


/******************************************************************************
*
* Name:        Default_Malloc
*
* Parameters:  Size               [In]   Number of bytes to allocate
*              Type               [In]   Data type of the allocation
*
* Returns:     Allocated_Memory_p [Ret]  The allocated memory
*
* Description: The default allocation function.
*
******************************************************************************/
void*
Default_Malloc(const uint32 Size,
               const D65_Mem_Type_t Type)
{
  void* Allocated_Memory_p = malloc(Size);
  return Allocated_Memory_p;
}


/******************************************************************************
*
* Name:        Default_Free
*
* Parameters:  Data_p  [In]  Pointer to memory that shall be freed
*              Type    [In]  Data type of the memory
*
*
* Returns:     -
*
* Description: The default free function.
*
******************************************************************************/
void
Default_Free(void* Data_p,
             const D65_Mem_Type_t Type)
{
  free(Data_p);
}

#if MULTI_THREADED
void*
StartThreads(void* VoidSessiondata_p)
{
  SessionData_t* Sessiondata_p = (SessionData_t*)VoidSessiondata_p;
  ThreadPool_t *Threadpool_p = &Sessiondata_p->ThreadsPool;
  ThreadData_t *CurrentThreadData_p;
  uint8 WorkingOnLastJob = 0;

  while(1)
  {
    pthread_mutex_lock(&Threadpool_p->ThreadLock);
    while(Threadpool_p->CurrentJobId == Threadpool_p->TotalJobNumber)
    {
      if(Threadpool_p->RetireFlag)
      {
        pthread_mutex_unlock(&Threadpool_p->ThreadLock);
        pthread_exit(NULL);
      }
      pthread_cond_wait(&Threadpool_p->JobsArrayNotEmpty, &Threadpool_p->ThreadLock);
      if(Threadpool_p->RetireFlag)
      {
        pthread_mutex_unlock(&Threadpool_p->ThreadLock);
        pthread_exit(NULL);
      }
    }

    WorkingOnLastJob = 0;
    CurrentThreadData_p = &Threadpool_p->JobsDataArray_p[Threadpool_p->JobsMapArray_p[Threadpool_p->CurrentJobId].Id];
    //CurrentThreadData_p = &Threadpool_p->JobsDataArray_p[Threadpool_p->CurrentJobId];
    Threadpool_p->CurrentJobId++;
    pthread_mutex_unlock(&Threadpool_p->ThreadLock);
    Threadpool_p->Jobs_p((void*)CurrentThreadData_p);
    pthread_mutex_lock(&Threadpool_p->ThreadLock);

    if(Threadpool_p->RemainingJobsNumber > 0)
    {
      Threadpool_p->RemainingJobsNumber--;
    }
    if(Threadpool_p->RemainingJobsNumber == 0)
    {
      pthread_cond_broadcast(&Threadpool_p->JobsArrayEmpty);
    }
    pthread_mutex_unlock(&Threadpool_p->ThreadLock);
  }
}
#endif

/******************************************************************************
*
* Name:        InitQuantGroupData
*
* Parameters:  -
*
* Returns:     -
*
* Description: Initializes quantization group data
*
******************************************************************************/
void
InitQuantGroupData(QuantGroupData_t *QgData_p)
{
    QgData_p->QgX             = -1;
    QgData_p->QgY             = -1;
    QgData_p->PrevQpY         = 26;
    QgData_p->CurrQpY         = 26;
    QgData_p->CuQpDeltaVal    = 0;
    QgData_p->IsCuQpDataCoded = 0;
}

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        InitBitsReader
*
******************************************************************************/
void
InitBitsReader(BitsReader_t *BitsReader_p,
               const uint8 *BufferStart_p,
               uint32 BufferLength)
{
  BitsReader_p->CurrentBuffer_p = BufferStart_p;
  BitsReader_p->BufferEnd_p     = BufferStart_p + BufferLength;
  BitsReader_p->Buffer32        = 0;
  BitsReader_p->Buffer32_Pos    = 0;
  BitsReader_p->PrevByte1       = 0xff; // Not 0
  BitsReader_p->PrevByte2       = 0xff; // Not 0
}


/******************************************************************************
*
* Name:        ResetBitsReader
*
******************************************************************************/
void
ResetBitsReader(BitsReader_t *BitsReader_p)
{
  BitsReader_p->CurrentBuffer_p = NULL;
  BitsReader_p->BufferEnd_p     = NULL;
  BitsReader_p->Buffer32        = 0;
  BitsReader_p->Buffer32_Pos    = 0;
  BitsReader_p->PrevByte1       = 0;
  BitsReader_p->PrevByte2       = 0;
}


/******************************************************************************
*
* Name:        CopyPredInfo
*
******************************************************************************/
void
CopyPredInfo(SessionData_t *Sessiondata_p)
{
  if(Sessiondata_p->CurrentSequenceParameterSet->TemporalMvpEnabledFlag == 1)
  {
    uint32 x;
    uint32 y;
    uint32 PicWidthInPU = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
    uint32 PicHeightInPU = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInMinUnit;
    for(y = 0; y < PicHeightInPU; y+=4)
    {
      for(x = 0; x < PicWidthInPU; x+=4)
      {
        Sessiondata_p->CurrentRecImage->MotionVectorL0[y*PicWidthInPU + x] = Sessiondata_p->PredictionUnitArray[y*PicWidthInPU + x].MotionvectorL0;
        Sessiondata_p->CurrentRecImage->MotionVectorL1[y*PicWidthInPU + x] = Sessiondata_p->PredictionUnitArray[y*PicWidthInPU + x].MotionvectorL1;
        Sessiondata_p->CurrentRecImage->PredMode[y*PicWidthInPU + x] = Sessiondata_p->PredictionUnitArray[y*PicWidthInPU + x].PredMode;
      }
    }
  }
}


/******************************************************************************
*
* Name:        ResetPUData
*
******************************************************************************/
void
ResetPUData(SessionData_t *Sessiondata_p)
{
  uint32 i;
  if(Sessiondata_p->ExportDebugData)
  {
    for(i = 0; i < Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMinUnit ; i++)
    {
      Sessiondata_p->PredictionUnitArray[i].SavedPredMode             = Sessiondata_p->PredictionUnitArray[i].PredMode;
      Sessiondata_p->PredictionUnitArray[i].PredMode                  = NOT_DECODED;
    }
  }
  else
  {
    for(i = 0; i < Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMinUnit ; i++)
    {
      Sessiondata_p->PredictionUnitArray[i].PredMode                  = NOT_DECODED;
    }
  }
}

/******************************************************************************
*
* Name:        D65_Initialize
*
******************************************************************************/
D65_ReturnCode_t D65_Initialize(D65_Handle_t** Decoder_pp, 
                                const D65_DecoderParameters_t *Parameters_p)
{

  SessionData_t *Sessiondata_p;

  uint32 i;
  uint32 j;
  D65_MallocPtr Malloc_Function_p = NULL;
  D65_FreePtr   Free_Function_p   = NULL;

  // Since
  // shifting negative numbers is undefined in ANSI-C, the following check
  // is done to ensure that it is possible to use shifts in the particular
  // compiler/processor combination used. By analyzing the filter it can be
  // found that negative input values for the shift are in the range -1 to
  // -25500 inclusive.
  sint32 ii;
  for(ii = -25500 ; ii<0; ii++)
  {
    if((ii << 2) != ii*4)
    {
      return D65_ERROR;
    }
  }

  if(Parameters_p->Free_Function_p == NULL)
  {
    Free_Function_p = Default_Free;
  }
  else
  {
    Free_Function_p   = Parameters_p->Free_Function_p;
  }
  if(Parameters_p->Malloc_Function_p == NULL)
  {
    Malloc_Function_p = Default_Malloc;
  }
  else
  {
    Malloc_Function_p = Parameters_p->Malloc_Function_p;
  }

  /* Allocate sessiondata */
  Sessiondata_p =  (SessionData_t*)  Malloc_Function_p(sizeof(SessionData_t), D65_INTERNAL_DATA);
  if(Sessiondata_p == NULL)
  {
    return D65_ERROR;
  }

  Sessiondata_p->Malloc_Function_p = Malloc_Function_p;
  Sessiondata_p->Free_Function_p   = Free_Function_p;


  /* Reset Video, Sequence and Parameter sets */
  for(i = 0 ; i < MAX_VPS_NUM; i++)
  {
    Sessiondata_p->VideoParameterSetArray[i] = NULL;
  }

  for(i = 0; i < MAX_SPS_NUM; i++)
  {
    Sessiondata_p->SequenceParameterSetArray[i] = NULL;
  }

  for(i = 0; i < MAX_PPS_NUM; i++)
  {
    Sessiondata_p->PictureParameterSetArray[i] = NULL;
  }

  Sessiondata_p->AllocatedImageHeight = 0;
  Sessiondata_p->AllocatedImageWidth = 0;
  Sessiondata_p->AllocatedDpbSize = 0;

  Sessiondata_p->ReturnedPictureFlag = 0;

  Sessiondata_p->PredictionUnitArray = NULL;

  Sessiondata_p->Interpl = NULL;

  Sessiondata_p->PredSampleArrayL0 = NULL;
  Sessiondata_p->PredSampleArrayL1 = NULL;


  Sessiondata_p->NumberCTUDecodedForPic = 0;

  Sessiondata_p->no_output_of_prior_pics_flag = 0;

#if D65_BITTRACE
  /* Set up trace variables */
  TraceFlag = Parameters_p->TraceFlag;
  TraceBitCounter = 0;
  Tracefile_p = Parameters_p->Tracefile_p;
#endif

  Sessiondata_p->Bs = NULL;
  Sessiondata_p->EdgeFlags = NULL;
  Sessiondata_p->EdgeFlagsHor = NULL;

  for(i = 0; i < 2; i++)
  {
    Sessiondata_p->SaoTypeIdx[i] = NULL;
  }

  for(i = 0; i < 3; i++)
  {
    for(j = 0; j < 5; j++)
    {
      Sessiondata_p->SaoOffsetVal[i][j] = NULL;
    }
    Sessiondata_p->SaoTypeClass[i] = NULL;
  }
  Sessiondata_p->SaoOffsetToUse = NULL;
  Sessiondata_p->TransformArray = NULL;

  for(i = 0; i < MAX_DPB_SIZE; i++)
  {
    Sessiondata_p->DecodedPictureBuffer[i].Image.Y  = NULL;
    Sessiondata_p->DecodedPictureBuffer[i].Image.Cb = NULL;
    Sessiondata_p->DecodedPictureBuffer[i].Image.Cr = NULL;
    Sessiondata_p->DecodedPictureBuffer[i].Image.Y_Width_image  = 0;
    Sessiondata_p->DecodedPictureBuffer[i].Image.Y_Height_image = 0;
    Sessiondata_p->DecodedPictureBuffer[i].Image.C_Width_image  = 0;
    Sessiondata_p->DecodedPictureBuffer[i].Image.C_Height_image = 0;
    Sessiondata_p->DecodedPictureBuffer[i].PictureMarking   = UNUSED_FOR_REFERENCE;
    Sessiondata_p->DecodedPictureBuffer[i].OutputMarking    = NOT_NEEDED_FOR_OUTPUT;
    Sessiondata_p->DecodedPictureBuffer[i].Image.UserPictureValue = 0;
    Sessiondata_p->DecodedPictureBuffer[i].PicOrderCnt      = -1;
    Sessiondata_p->DecodedPictureBuffer[i].MotionVectorL0 = NULL;
    Sessiondata_p->DecodedPictureBuffer[i].MotionVectorL1 = NULL;
    Sessiondata_p->DecodedPictureBuffer[i].PredMode = NULL;
    memset(Sessiondata_p->DecodedPictureBuffer[i].RefPicPOCList0, 0, MAX_DPB_SIZE*sizeof(sint32));
    memset(Sessiondata_p->DecodedPictureBuffer[i].RefPicMarkingList0, 0, MAX_DPB_SIZE*sizeof(PictureMarking_t));
  }
  memset(Sessiondata_p->RefPicList0,-1,MAX_DPB_SIZE*sizeof(YUVImage_t*));
  memset(Sessiondata_p->RefPicList1,-1,MAX_DPB_SIZE*sizeof(YUVImage_t*));
  Sessiondata_p->CurrentRecImage = &Sessiondata_p->DecodedPictureBuffer[0];

  /* Reset POC variables */
  Sessiondata_p->PicOrderCntVal = 0;
  Sessiondata_p->PrevTid0PicPoc  = 0;

  /* Reset slice variables */
  Sessiondata_p->CurrentSliceNumber = 0;
  Sessiondata_p->CurrentSliceType = 0;
  Sessiondata_p->Current_num_ref_idx_l0_active_minus1 = 0;

  /* Reset pointers */
//  Sessiondata_p->MbDataArray = NULL;
  Sessiondata_p->CurrentSequenceParameterSet = NULL;
  Sessiondata_p->CurrentPictureParameterSet = NULL;
#if SUPPORT_EXTENSIONS
  Sessiondata_p->CurrentVideoParameterSet = NULL;
#endif

  /* Reset bytebuffer stuff */
  ResetBitsReader(&Sessiondata_p->Parser.BitsReader);

  // Reset Quantization Group Data
  InitQuantGroupData(&Sessiondata_p->QgData);

#if SUPPORT_EXTENSIONS
  /* Set layer data */
  Sessiondata_p->LayerID = Parameters_p->LayerID;
  Sessiondata_p->LongTermMarking = false;
#endif

  Sessiondata_p->ExportDebugData           = 0;
  Sessiondata_p->CurrentTileColumnMinus1   = 0;
  Sessiondata_p->CurrentTileRowMinus1      = 0;
  Sessiondata_p->CurrentUniformSpacingFlag = 1;

#if MULTI_THREADED

  Sessiondata_p->ThreadsPool.CurrentJobId = 0;
  Sessiondata_p->ThreadsPool.TotalJobNumber = 0;
  Sessiondata_p->ThreadsPool.JobsDataArray_p = NULL;
  Sessiondata_p->ThreadsPool.JobsMapArray_p  = NULL;
  Sessiondata_p->ThreadsPool.RemainingJobsNumber = 0;
  Sessiondata_p->ThreadsPool.RetireFlag = 0;
  Sessiondata_p->ThreadsPool.ThreadsArray = NULL;

  Sessiondata_p->Threaddata_p = NULL;

  for(i = 0; i < MAX_TILES_NUM; i++)
  {
    memset(Sessiondata_p->CurrentTiledata_p, 0, sizeof(TileData_t));
    memset(Sessiondata_p->JobsMapArray, 0, sizeof(JobsMapping));
  }

  Sessiondata_p->CurrentNumOfThreads = Parameters_p->Threads_Number;
  Sessiondata_p->CurrentNumOfEntries = 1;
  Sessiondata_p->CurrentTileNum = 1;
#endif

#if ENABLE_OPENHEVC
  Sessiondata_p->OpenHevcDecState = 0;
  if(Parameters_p->OpenHevcFlag)
  {
    Sessiondata_p->OpenHevcDecState = 1;
    Sessiondata_p->OpenHevcDec.openHevcHandle = NULL;
    Sessiondata_p->OpenHevcDec.ReturnPic.Y_Height_image = 0;
    Sessiondata_p->OpenHevcDec.ReturnPic.Y_Width_image  = 0;
    Sessiondata_p->OpenHevcDec.ReturnPic.Y  = NULL;
    Sessiondata_p->OpenHevcDec.ReturnPic.Cb = NULL;
    Sessiondata_p->OpenHevcDec.ReturnPic.Cr = NULL;

    Sessiondata_p->OpenHevcDec.openHevcHandle = libOpenHevcInit(0); 
    if(Sessiondata_p->OpenHevcDec.openHevcHandle)
    {
      Sessiondata_p->OpenHevcDecState = 2;
      libOpenHevcSetCheckMD5(Sessiondata_p->OpenHevcDec.openHevcHandle, 0);
      libOpenHevcSetDisableAU(Sessiondata_p->OpenHevcDec.openHevcHandle, 1);
      //fprintf(stdout, "OpenHEVC version = %s\n", libOpenHevcVersion(Sessiondata_p->OpenHevcDec.openHevcHandle));
    }
  }
#endif

#if D65_CONFORMANCE
  Sessiondata_p->Write_Output_Function_p = Parameters_p->Write_Output_Function_p;
  Sessiondata_p->OutputLocation = Parameters_p->OutputLocation;
#endif


  /* Set decoder */
  *Decoder_pp = (D65_Handle_t *) Sessiondata_p;

  return D65_OK;
}


/******************************************************************************
*
* Name:        D65_Release
*
******************************************************************************/
D65_ReturnCode_t
D65_Release(D65_Handle_t* Decoder_p)
{
  uint32 i;
  SessionData_t *Sessiondata_p = (SessionData_t*) Decoder_p;

  FreeSequenceMemory(Sessiondata_p);
#if MULTI_THREADED
  FreeTilesThreadMemory(Sessiondata_p);
#endif

  /* Free picture parameter set */
  for(i = 0; i < MAX_PPS_NUM; i++)
  {
    if(Sessiondata_p->PictureParameterSetArray[i] != NULL)
    {
      Sessiondata_p->Free_Function_p(Sessiondata_p->PictureParameterSetArray[i], D65_INTERNAL_DATA);
      Sessiondata_p->PictureParameterSetArray[i] = NULL;
    }
  }

  for(i = 0; i < MAX_SPS_NUM; i++)
  {
    if(Sessiondata_p->SequenceParameterSetArray[i] != NULL)
    {
      Sessiondata_p->Free_Function_p(Sessiondata_p->SequenceParameterSetArray[i], D65_INTERNAL_DATA);
      Sessiondata_p->SequenceParameterSetArray[i] = NULL;
    }
  }

  for(i = 0; i < MAX_VPS_NUM; i++)
  {
    if(Sessiondata_p->VideoParameterSetArray[i] != NULL)
    {
      Sessiondata_p->Free_Function_p(Sessiondata_p->VideoParameterSetArray[i], D65_INTERNAL_DATA);
      Sessiondata_p->VideoParameterSetArray[i] = NULL;
    }
  }

#if ENABLE_OPENHEVC
  if(Sessiondata_p->OpenHevcDecState >= 2)
  {
    FreeOpenHevcReturnPic(Sessiondata_p);
    Sessiondata_p->OpenHevcDecState = 0;
    libOpenHevcClose(Sessiondata_p->OpenHevcDec.openHevcHandle);
  }
#endif
  /* Free sessiondata */
  Sessiondata_p->Free_Function_p(Sessiondata_p, D65_INTERNAL_DATA);
  Sessiondata_p = NULL;

  return D65_OK;
}


/******************************************************************************
*
* Name:        InitPredictionUnitArray
*
******************************************************************************/

void
InitPredictionUnitArray(SessionData_t *Sessiondata_p)
{
  const SequenceParameterSet_t *CurrSPS = Sessiondata_p->CurrentSequenceParameterSet;
  const PictureParameterSet_t  *CurrPPS = Sessiondata_p->CurrentPictureParameterSet;
  const uint32 PicHeightPU     = CurrSPS->PicHeightInMinUnit;
  const uint32 PicWidthPU      = CurrSPS->PicWidthInMinUnit;
  const uint32 Log2CtbSizeY    = CurrSPS->Log2CtbSizeY;
  const uint32 Log2MinPuSizeY  = CurrSPS->Log2MinUnitSize;
  const uint32 Log2CtbSizeInPU = Log2CtbSizeY - Log2MinPuSizeY;

  uint32 maxTilesX = 0;
  uint32 maxTilesY = 0;
  uint32 tileCoordX;
  uint32 tileCoordY;
  uint32 colBdPU[257];
  uint32 rowBdPU[257];
  uint32 y;
  uint32 x;
  uint32 tileId;
  uint32 PUAddr;

  // First check if we have a picture parameter set present and tiling informatino available
  if(CurrPPS != NULL && CurrPPS->TilesEnabledFlag == 1)
  {
    maxTilesX = CurrPPS->num_tile_columns_minus1+1;
    maxTilesY = CurrPPS->num_tile_rows_minus1+1;

    for(y = 0; y < maxTilesX+1; y++)
    {
      colBdPU[y] = (Sessiondata_p->colBd[y] << Log2CtbSizeInPU);
    }
    for(y = 0; y < maxTilesY+1; y++)
    {
      rowBdPU[y] = (Sessiondata_p->rowBd[y] << Log2CtbSizeInPU);
    }
  }
  else
  {
    // We do not have a PPS or no tiling data, use just one big tile.
    maxTilesX = 1;
    maxTilesY = 1;

    colBdPU[0] = 0;
    colBdPU[1] = PicWidthPU;
    
    rowBdPU[0] = 0;
    rowBdPU[1] = PicHeightPU;
  }

#if MULTI_THREADED
  tileId = 0;
  for(tileCoordY = 0; tileCoordY < maxTilesY; tileCoordY++)
  {
    for(tileCoordX = 0; tileCoordX < maxTilesX; tileCoordX++,tileId++)
    {
      //Initializing tile location info
      Sessiondata_p->CurrentTiledata_p[tileId].TileStartCtuX = colBdPU[tileCoordX] >> Log2CtbSizeInPU;
      Sessiondata_p->CurrentTiledata_p[tileId].TileStartCtuY = rowBdPU[tileCoordY] >> Log2CtbSizeInPU;
      Sessiondata_p->CurrentTiledata_p[tileId].TileStopCtuX  = colBdPU[tileCoordX+1] >> Log2CtbSizeInPU;
      Sessiondata_p->CurrentTiledata_p[tileId].TileStopCtuY  = rowBdPU[tileCoordY+1] >> Log2CtbSizeInPU;
    }
  }
#endif

  tileId = 0;
  for(tileCoordX = 0; tileCoordX < maxTilesX; tileCoordX++)
  {
    for(tileCoordY = 0; tileCoordY < maxTilesY; tileCoordY++, tileId++)
    {
      x = colBdPU[tileCoordX];
      y = rowBdPU[tileCoordY];
      PUAddr = y * PicWidthPU + x;
      //Sessiondata_p->PredictionUnitArray[0].Position = UPPER_LEFT;
      Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = NULL;

      x = colBdPU[tileCoordX+1] - 1;
      y = rowBdPU[tileCoordY];
      PUAddr = y * PicWidthPU + x;
      //Sessiondata_p->PredictionUnitArray[PicWidthPU-1].Position = UPPER_RIGHT;
      Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = Sessiondata_p->PredictionUnitArray + PUAddr + PicWidthPU - 1;

      x = colBdPU[tileCoordX];
      y = rowBdPU[tileCoordY+1]-1;
      PUAddr = y * PicWidthPU + x;
      //Sessiondata_p->PredictionUnitArray[(PicHeightPU-1)*PicWidthPU].Position = LOWER_LEFT;
      Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU + 1;

      x = colBdPU[tileCoordX+1]-1;
      y = rowBdPU[tileCoordY+1]-1;
      Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].TileId     = tileId;
      PUAddr = y * PicWidthPU + x;
      //Sessiondata_p->PredictionUnitArray[PicHeightPU*PicWidthPU-1].Position = LOWER_RIGHT;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = NULL;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
      Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU - 1;

      for(y = rowBdPU[tileCoordY] + 1 ; y < rowBdPU[tileCoordY+1] - 1; y++)
      {
        x = colBdPU[tileCoordX];
        PUAddr = y * PicWidthPU + x;
        //Sessiondata_p->PredictionUnitArray[y*PicWidthPU].Position = LEFT;
        Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU + 1;

        x = colBdPU[tileCoordX+1]-1;
        PUAddr = y * PicWidthPU + x;
        //Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].Position = RIGHT;
        Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU - 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = Sessiondata_p->PredictionUnitArray + PUAddr + PicWidthPU - 1;
      }
      for(x = colBdPU[tileCoordX] + 1; x < colBdPU[tileCoordX+1] - 1; x++)
      {
        y = rowBdPU[tileCoordY];
        PUAddr = y * PicWidthPU + x;
        //Sessiondata_p->PredictionUnitArray[x].Position = UPPER;
        Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = NULL;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = Sessiondata_p->PredictionUnitArray + PUAddr + PicWidthPU - 1;

        y = rowBdPU[tileCoordY+1]-1;
        PUAddr = y * PicWidthPU + x;
        //Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].Position = LOWER;
        Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU - 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU + 1;
        Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = NULL;
      }
      for(y=rowBdPU[tileCoordY]+1 ; y<rowBdPU[tileCoordY+1]-1; y++)
      {
        for(x=colBdPU[tileCoordX]+1, PUAddr = y * PicWidthPU + colBdPU[tileCoordX]+1; x<colBdPU[tileCoordX+1]-1; x++)
        {
          //Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].Position = MIDDLE;
          Sessiondata_p->PredictionUnitArray[PUAddr].TileId     = tileId;
          Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitLeft_p     = Sessiondata_p->PredictionUnitArray + PUAddr - 1;
          Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUp_p       = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU;
          Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpLeft_p   = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU - 1;
          Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitUpRight_p  = Sessiondata_p->PredictionUnitArray + PUAddr - PicWidthPU + 1;
          Sessiondata_p->PredictionUnitArray[PUAddr].PredictionUnitDownLeft_p = Sessiondata_p->PredictionUnitArray + PUAddr + PicWidthPU - 1;
          PUAddr++;
        }
      }
    }
  }

  // Set Prediction unit coordinates
  // (This can be done irrespectively of the tile structure)
  //for(y=0 ; y<PicHeightPU ; y++)
  //{
  //  for(x=0 ; x<PicWidthPU ; x++)
  //  {
  //    Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].PUCoordinateY = y;
  //    Sessiondata_p->PredictionUnitArray[y*PicWidthPU+x].PUCoordinateX = x;
  //  }
  //}

}


#if MULTI_THREADED
uint8
AllocateThreadsPoolMemory(SessionData_t *Sessiondata_p)
{
  uint32 ThreadsNum = Sessiondata_p->CurrentNumOfThreads;

  if(ThreadsNum < 2)
  {
    return 0;
  }

  Sessiondata_p->ThreadsPool.ThreadsArray = (pthread_t*)Sessiondata_p->Malloc_Function_p(ThreadsNum*sizeof(pthread_t), D65_INTERNAL_DATA);
  if(Sessiondata_p->ThreadsPool.ThreadsArray == NULL)
  {
    return 1;
  }

  if(pthread_mutex_init(&(Sessiondata_p->ThreadsPool.ThreadLock), NULL))
  {
    return 1;
  }

  if(pthread_cond_init(&(Sessiondata_p->ThreadsPool.JobsArrayEmpty), NULL))
  {
    return 1;
  }

  if(pthread_cond_init(&(Sessiondata_p->ThreadsPool.JobsArrayNotEmpty), NULL))
  {
    return 1;
  }

  Sessiondata_p->ThreadsPool.TotalJobNumber = 0;
  Sessiondata_p->ThreadsPool.CurrentJobId = 0;
  Sessiondata_p->ThreadsPool.JobsDataArray_p = Sessiondata_p->Threaddata_p;
  Sessiondata_p->ThreadsPool.JobsMapArray_p  = Sessiondata_p->JobsMapArray;
  Sessiondata_p->ThreadsPool.RemainingJobsNumber = 0;
  Sessiondata_p->ThreadsPool.RetireFlag = 0;

  if(Sessiondata_p->CurrentNumOfThreads > 1)
  {
    uint32 i;
    for(i = 0; i < Sessiondata_p->CurrentNumOfThreads; i++)
    {
      pthread_create(&Sessiondata_p->ThreadsPool.ThreadsArray[i], NULL, StartThreads, (void*)Sessiondata_p);
    }
  }
  return 0;
}

uint8
AllocateTilesThreadMemory(SessionData_t *Sessiondata_p)
{
  uint32 TilesNum = 1;
  uint32 i;
  uint32 TransformSize = MAX_TU_SIZE * MAX_TU_SIZE;

  if(Sessiondata_p->CurrentPictureParameterSet && Sessiondata_p->CurrentPictureParameterSet->TilesEnabledFlag)
  {
    TilesNum = (Sessiondata_p->CurrentPictureParameterSet->num_tile_columns_minus1 + 1) * 
      (Sessiondata_p->CurrentPictureParameterSet->num_tile_rows_minus1 + 1);
  }

  Sessiondata_p->Threaddata_p = (ThreadData_t*)Sessiondata_p->Malloc_Function_p(TilesNum*sizeof(ThreadData_t), D65_INTERNAL_DATA);
  if(Sessiondata_p->Threaddata_p == NULL)
  {
    return 1;
  }

  for(i = 0; i < TilesNum; i++)
  {
    Sessiondata_p->Threaddata_p[i].Sessiondata_p = (void*)Sessiondata_p;
    Sessiondata_p->Threaddata_p[i].TransformArray = (sint16*)Sessiondata_p->Malloc_Function_p(2*TransformSize*sizeof(sint16), D65_INTERNAL_DATA);
    if(Sessiondata_p->Threaddata_p[i].TransformArray == NULL)
    {
      return 1;
    }

    Sessiondata_p->Threaddata_p[i].Interpl = (sint16*)Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE * (MAX_PU_SIZE + 8) * sizeof(sint16), D65_INTERNAL_DATA);

    if(Sessiondata_p->Threaddata_p[i].Interpl == NULL)
    {
      return 1;
    }

    /* Allocate memory for predSampleArray */
    Sessiondata_p->Threaddata_p[i].PredSampleArrayL0 = (sint16 *) Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE*MAX_PU_SIZE*sizeof(sint16), D65_INTERNAL_DATA);
    Sessiondata_p->Threaddata_p[i].PredSampleArrayL1 = (sint16 *) Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE*MAX_PU_SIZE*sizeof(sint16), D65_INTERNAL_DATA);

    if(Sessiondata_p->Threaddata_p[i].PredSampleArrayL0 == NULL || Sessiondata_p->Threaddata_p[i].PredSampleArrayL1 == NULL)
    {
      return 1;
    }

  }
  Sessiondata_p->CurrentTileNum = TilesNum;
  return 0;
}

#endif

/******************************************************************************
*
* Name:        AllocateSequenceMemory
*
******************************************************************************/
uint8
AllocateSequenceMemory(SessionData_t *Sessiondata_p)
{
  const SequenceParameterSet_t *CurrSPS = Sessiondata_p->CurrentSequenceParameterSet;
  const uint32 Y_width           = CurrSPS->PictureWidth;
  const uint32 Y_height          = CurrSPS->PictureHeight;
  const uint32 PicSizeInMinUnits = CurrSPS->PicSizeInMinUnit;
  const uint32 PicSizeInCtbY     = (CurrSPS->PicWidthInCtbsY) * (CurrSPS->PicHeightInCtbsY);
  const uint32 nSInPU            = 1 << (CurrSPS->Log2CtbSizeY - CurrSPS->Log2MinUnitSize);
  const uint32 nS                = 1 << (CurrSPS->Log2CtbSizeY);
  const uint8  DpbSize           = CurrSPS->DpbSize;
  uint32 i;
  uint32 j;

  for(i = 0; i < DpbSize; i++)
  {
    if(Allocate_Image(Sessiondata_p, &(Sessiondata_p->DecodedPictureBuffer[i]), Y_height+2*PADDING, Y_width+2*PADDING, PicSizeInMinUnits))
    {
      return 1;
    }
  }

  // Allocate Edge flags and Boundary strength array
  Sessiondata_p->EdgeFlags = (uint8*)Sessiondata_p->Malloc_Function_p(PicSizeInMinUnits * sizeof(uint8),D65_INTERNAL_DATA);
  if(Sessiondata_p->EdgeFlags == NULL)
  {
    return 1;
  }
  Sessiondata_p->EdgeFlagsHor = (uint8*)Sessiondata_p->Malloc_Function_p(PicSizeInMinUnits * sizeof(uint8),D65_INTERNAL_DATA);
  if(Sessiondata_p->EdgeFlagsHor == NULL)
  {
    return 1;
  }

  Sessiondata_p->Bs = (uint8*)Sessiondata_p->Malloc_Function_p(nSInPU * nSInPU * sizeof(uint8),D65_INTERNAL_DATA);
  if(Sessiondata_p->Bs == NULL)
  {
    return 1;
  }

  if(CurrSPS->SampleAdaptiveOffsetEnabledFlag == 1)
  {
    for(i = 0; i < 2; i++)
    {
      Sessiondata_p->SaoTypeIdx[i] = (uint8*)Sessiondata_p->Malloc_Function_p(PicSizeInCtbY * sizeof(uint8), D65_INTERNAL_DATA);
      if(Sessiondata_p->SaoTypeIdx[i] == NULL)
      {
        return 1;
      }
    }
    for(i = 0; i < 3; i++)
    {
      for(j = 0; j < 5; j++)
      {
        Sessiondata_p->SaoOffsetVal[i][j] = (sint16*)Sessiondata_p->Malloc_Function_p(PicSizeInCtbY * sizeof(sint16),D65_INTERNAL_DATA);
        if(Sessiondata_p->SaoOffsetVal[i][j] == NULL)
        {
          return 1;
        }
      }
      Sessiondata_p->SaoTypeClass[i] = (uint8*)Sessiondata_p->Malloc_Function_p(PicSizeInCtbY * sizeof(uint8), D65_INTERNAL_DATA);
      if(Sessiondata_p->SaoTypeClass[i] == NULL)
      {
        return 1;
      }
    }
    Sessiondata_p->SaoOffsetToUse = (sint16*)Sessiondata_p->Malloc_Function_p(PicSizeInCtbY * nS * nS * sizeof(sint16), D65_INTERNAL_DATA);
    if(Sessiondata_p->SaoOffsetToUse == NULL)
    {
      return 1;
    }
  }

  /* Allocate PredictionUnitArray */
  /* Sessiondata_p->CurrentSequenceParameterSet->PicSizeInMinCbsY holds the */
  /* number of minimum CU blocks in the picture. The maximum number of PUs  */
  /* in a CU is 4. */
  Sessiondata_p->PredictionUnitArray = (PredictionUnit_t*) Sessiondata_p->Malloc_Function_p(PicSizeInMinUnits * sizeof(PredictionUnit_t), D65_INTERNAL_DATA);
  if(Sessiondata_p->PredictionUnitArray == NULL)
  {
    return 1;
  }

#if !(MULTI_THREADED)
  // Allocate TransformArray
  // The size of the array is two times the maximum transform block size
  // The factor of two is to give room for a temporal storage when doing inverse transformation
  Sessiondata_p->TransformArray = (sint16*) Sessiondata_p->Malloc_Function_p(2 * MAX_TU_SIZE * MAX_TU_SIZE * sizeof(sint16), D65_INTERNAL_DATA);
  if(Sessiondata_p->TransformArray == NULL)
  {
    return 1;
  }

  /* Allocate Interpl arrays for luma and chroma */
  /* Luma array needs to fit PUSize*(PUSize + 8) elements */
  /* Chroma arrays need to fit (PUSize/2)*(4 + PUSize/2) */
  Sessiondata_p->Interpl = (sint16*) Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE * (MAX_PU_SIZE + 8) * sizeof(sint16), D65_INTERNAL_DATA);
  if(Sessiondata_p->Interpl == NULL)
  {
    return 1;
  }

  /* Allocate memory for predSampleArray */
  Sessiondata_p->PredSampleArrayL0 = (sint16*) Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE * MAX_PU_SIZE*sizeof(sint16), D65_INTERNAL_DATA);
  Sessiondata_p->PredSampleArrayL1 = (sint16*) Sessiondata_p->Malloc_Function_p(MAX_PU_SIZE * MAX_PU_SIZE*sizeof(sint16), D65_INTERNAL_DATA);

  if(Sessiondata_p->PredSampleArrayL0 == NULL || Sessiondata_p->PredSampleArrayL1 == NULL)
  {
    return 1;
  }
#endif

#if MULTI_THREADED
  if(AllocateThreadsPoolMemory(Sessiondata_p))
  {
    return 1;
  }
#endif

  // Reset the non-constant variables of PredictionUnitArray
  ResetPUData(Sessiondata_p);

  Sessiondata_p->AllocatedImageHeight = Y_height;
  Sessiondata_p->AllocatedImageWidth  = Y_width;
  Sessiondata_p->AllocatedDpbSize     = DpbSize;

  for(i = 0; i < MAX_DPB_SIZE; i++)
  {
    Sessiondata_p->RefPicList0[i] = 0;
    Sessiondata_p->RefPicList1[i] = 0;
  }

#if !MULTI_THREADED
  ContextsBufferAssign(&Sessiondata_p->Parser.CtxModels);
#endif

  memset(Sessiondata_p->EdgeFlags,   0, PicSizeInMinUnits * sizeof(uint8));
  memset(Sessiondata_p->EdgeFlagsHor,0, PicSizeInMinUnits * sizeof(uint8));

  if(CurrSPS->SampleAdaptiveOffsetEnabledFlag == 1)
  {
    memset(Sessiondata_p->SaoTypeIdx[0], 0, PicSizeInCtbY * sizeof(uint8));
    memset(Sessiondata_p->SaoTypeIdx[1], 0, PicSizeInCtbY * sizeof(uint8));
  }
  return 0;
}


/******************************************************************************
*
* Name:        FreeSequenceMemory
*
******************************************************************************/
void
FreeSequenceMemory(SessionData_t *Sessiondata_p)
{
  uint32 i;
  uint32 j;
#if MULTI_THREADED
  FreeThreadPoolMemory(Sessiondata_p);
#endif

  /* Free Images */
  for(i = 0; i < Sessiondata_p->AllocatedDpbSize; i++)
  {
    Free_Image(Sessiondata_p, &(Sessiondata_p->DecodedPictureBuffer[i]));
  }

  if(Sessiondata_p->EdgeFlags)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->EdgeFlags, D65_INTERNAL_DATA);
    Sessiondata_p->EdgeFlags = NULL;
  }
  if(Sessiondata_p->EdgeFlagsHor)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->EdgeFlagsHor, D65_INTERNAL_DATA);
    Sessiondata_p->EdgeFlagsHor = NULL;
  }

  if(Sessiondata_p->Bs)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->Bs, D65_INTERNAL_DATA);
    Sessiondata_p->Bs = NULL;
  }

  if(Sessiondata_p->TransformArray)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->TransformArray, D65_INTERNAL_DATA);
    Sessiondata_p->TransformArray = NULL;
  }
  for(i = 0; i < 2; i++)
  {
    if(Sessiondata_p->SaoTypeIdx[i])
    {
      Sessiondata_p->Free_Function_p(Sessiondata_p->SaoTypeIdx[i], D65_INTERNAL_DATA);
      Sessiondata_p->SaoTypeIdx[i] = NULL;
    }
  }
  for(i = 0; i < 3; i++)
  {
    for(j = 0; j < 5; j++)
    {
      if(Sessiondata_p->SaoOffsetVal[i][j])
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->SaoOffsetVal[i][j], D65_INTERNAL_DATA);
        Sessiondata_p->SaoOffsetVal[i][j] = NULL;
      }
    }
    if(Sessiondata_p->SaoTypeClass[i])
    {
      Sessiondata_p->Free_Function_p(Sessiondata_p->SaoTypeClass[i], D65_INTERNAL_DATA);
      Sessiondata_p->SaoTypeClass[i] = NULL;
    }
  }
  if(Sessiondata_p->SaoOffsetToUse)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->SaoOffsetToUse, D65_INTERNAL_DATA);
    Sessiondata_p->SaoOffsetToUse = NULL;
  }

  /* Free PredictionData */
  if(Sessiondata_p->PredictionUnitArray)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->PredictionUnitArray, D65_INTERNAL_DATA);
    Sessiondata_p->PredictionUnitArray = NULL;
  }

  /* Free Interpl arrays */
  if(Sessiondata_p->Interpl)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->Interpl, D65_INTERNAL_DATA);
    Sessiondata_p->Interpl = NULL;
  }

  /* Free PredSampleArrays */
  if(Sessiondata_p->PredSampleArrayL0)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->PredSampleArrayL0, D65_INTERNAL_DATA);
    Sessiondata_p->PredSampleArrayL0 = NULL;
  }

  if(Sessiondata_p->PredSampleArrayL1)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->PredSampleArrayL1, D65_INTERNAL_DATA);
    Sessiondata_p->PredSampleArrayL1 = NULL;
  }

  Sessiondata_p->AllocatedDpbSize     = 0;
  Sessiondata_p->AllocatedImageHeight = 0;
  Sessiondata_p->AllocatedImageWidth  = 0;
}


#if MULTI_THREADED
/******************************************************************************
*
* Name:        FreeThreadPoolMemory
*
******************************************************************************/
void
FreeThreadPoolMemory(SessionData_t *Sessiondata_p)
{
  uint32 i;
  ThreadPool_t *Threadpool_p;

  if(Sessiondata_p->CurrentNumOfThreads < 2)
  {
    return;
  }

  Threadpool_p = &Sessiondata_p->ThreadsPool;

  if(Threadpool_p->ThreadsArray == NULL)
  {
    return;
  }

  pthread_mutex_lock(&Threadpool_p->ThreadLock);
  Threadpool_p->RetireFlag = 1;
  pthread_cond_broadcast(&Threadpool_p->JobsArrayNotEmpty);
  pthread_mutex_unlock(&Threadpool_p->ThreadLock);
  for(i = 0; i < Sessiondata_p->CurrentNumOfThreads; i++)
  {
    pthread_join(Threadpool_p->ThreadsArray[i], NULL);
  }

  Sessiondata_p->Free_Function_p(Threadpool_p->ThreadsArray, D65_INTERNAL_DATA);
  Threadpool_p->ThreadsArray = NULL;
  pthread_mutex_destroy(&Threadpool_p->ThreadLock);
  pthread_cond_destroy(&Threadpool_p->JobsArrayEmpty);
  pthread_cond_destroy(&Threadpool_p->JobsArrayNotEmpty);
}

void
FreeTilesThreadMemory(SessionData_t *Sessiondata_p)
{
  uint32 i;
  uint32 TilesNum = 1;

  if(Sessiondata_p->CurrentPictureParameterSet == NULL)
  {
    return;
  }
  //if(Sessiondata_p->CurrentPictureParameterSet && Sessiondata_p->CurrentPictureParameterSet->TilesEnabledFlag)
  //{
  //  TilesNum = (Sessiondata_p->CurrentPictureParameterSet->num_tile_columns_minus1 + 1) * 
  //    (Sessiondata_p->CurrentPictureParameterSet->num_tile_rows_minus1 + 1);
  //}
  TilesNum = Sessiondata_p->CurrentTileNum;

  if(Sessiondata_p->Threaddata_p)
  {
    for(i = 0; i < TilesNum; i++)
    {
      if(Sessiondata_p->Threaddata_p[i].TransformArray)
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->Threaddata_p[i].TransformArray, D65_INTERNAL_DATA);
        Sessiondata_p->Threaddata_p[i].TransformArray = NULL;
      }

      if(Sessiondata_p->Threaddata_p[i].Interpl)
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->Threaddata_p[i].Interpl, D65_INTERNAL_DATA);
        Sessiondata_p->Threaddata_p[i].Interpl = NULL;
      }

      if(Sessiondata_p->Threaddata_p[i].PredSampleArrayL0)
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->Threaddata_p[i].PredSampleArrayL0, D65_INTERNAL_DATA);
        Sessiondata_p->Threaddata_p[i].PredSampleArrayL0 = NULL;
      }

      if(Sessiondata_p->Threaddata_p[i].PredSampleArrayL1)
      {
        Sessiondata_p->Free_Function_p(Sessiondata_p->Threaddata_p[i].PredSampleArrayL1, D65_INTERNAL_DATA);
        Sessiondata_p->Threaddata_p[i].PredSampleArrayL1 = NULL;
      }
    }

    Sessiondata_p->Free_Function_p(Sessiondata_p->Threaddata_p, D65_INTERNAL_DATA);
    Sessiondata_p->Threaddata_p = NULL;
  }
}
#endif

#if ENABLE_OPENHEVC
uint8
AllocateOpenHevcReturnPic(SessionData_t *Sessiondata_p,
                          const uint32 Y_height,
                          const uint32 Y_width)
{
  YUVImage_t *Image_p = &Sessiondata_p->OpenHevcDec.ReturnPic;
  sint32 ReturnPicW  = Image_p->Y_Width_image;
  sint32 ReturnPicH  = Image_p->Y_Height_image;

  if(Sessiondata_p->OpenHevcDecState != 4 || ReturnPicW != Y_width || ReturnPicH != Y_height )
  {
    Image_p->Y_Width_image  = Y_width;
    Image_p->Y_Height_image = Y_height;

    if(Image_p->Y)
    {
      Sessiondata_p->Free_Function_p(Image_p->Y, D65_INTERNAL_DATA);
      Image_p->Y = NULL;
    }
    if(Image_p->Cb)
    {
      Sessiondata_p->Free_Function_p(Image_p->Cb, D65_INTERNAL_DATA);
      Image_p->Cb = NULL;
    }
    if(Image_p->Cr)
    {
      Sessiondata_p->Free_Function_p(Image_p->Cr, D65_INTERNAL_DATA);
      Image_p->Cr = NULL;
    }

    Image_p->Y  = (uint8*)Sessiondata_p->Malloc_Function_p(Y_width * Y_height, D65_INTERNAL_DATA);
    if(Image_p->Y == NULL)
    {
      return 1;
    }
    Image_p->Cb = (uint8*)Sessiondata_p->Malloc_Function_p((Y_width >> 1)*(Y_height >> 1), D65_INTERNAL_DATA);
    if(Image_p->Cb == NULL)
    {
      return 1;
    }
    Image_p->Cr = (uint8*)Sessiondata_p->Malloc_Function_p((Y_width >> 1)*(Y_height >> 1), D65_INTERNAL_DATA);
    if(Image_p->Cr == NULL)
    {
      return 1;
    }

    Sessiondata_p->OpenHevcDec.openHevcFrame.pvY = (void*)Image_p->Y;
    Sessiondata_p->OpenHevcDec.openHevcFrame.pvU = (void*)Image_p->Cb;
    Sessiondata_p->OpenHevcDec.openHevcFrame.pvV = (void*)Image_p->Cr;

    Sessiondata_p->DecodedPicture.Y_Width_image  = Image_p->Y_Width_image;
    Sessiondata_p->DecodedPicture.Y_Height_image = Image_p->Y_Height_image;
    Sessiondata_p->DecodedPicture.Y_Width_memory = Image_p->Y_Width_image;
    Sessiondata_p->DecodedPicture.Y  = Image_p->Y;
    Sessiondata_p->DecodedPicture.Cb = Image_p->Cb;
    Sessiondata_p->DecodedPicture.Cr = Image_p->Cr;
    Sessiondata_p->OpenHevcDecState = 4;
  }

  return 0;
}

void
FreeOpenHevcReturnPic(SessionData_t *Sessiondata_p)
{
  if(Sessiondata_p->OpenHevcDecState < 2)
  {
    return;
  }

  if(Sessiondata_p->OpenHevcDec.ReturnPic.Y)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->OpenHevcDec.ReturnPic.Y, D65_INTERNAL_DATA);
    Sessiondata_p->OpenHevcDec.ReturnPic.Y = NULL;
  }

  if(Sessiondata_p->OpenHevcDec.ReturnPic.Cb)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->OpenHevcDec.ReturnPic.Cb, D65_INTERNAL_DATA);
    Sessiondata_p->OpenHevcDec.ReturnPic.Cb = NULL;
  }

  if(Sessiondata_p->OpenHevcDec.ReturnPic.Cr)
  {
    Sessiondata_p->Free_Function_p(Sessiondata_p->OpenHevcDec.ReturnPic.Cr, D65_INTERNAL_DATA);
    Sessiondata_p->OpenHevcDec.ReturnPic.Cr = NULL;
  }

  Sessiondata_p->OpenHevcDec.openHevcFrame.pvY = NULL;
  Sessiondata_p->OpenHevcDec.openHevcFrame.pvU = NULL;
  Sessiondata_p->OpenHevcDec.openHevcFrame.pvV = NULL;
  Sessiondata_p->OpenHevcDec.ReturnPic.Y_Width_image = 0;
  Sessiondata_p->OpenHevcDec.ReturnPic.Y_Height_image = 0;

  Sessiondata_p->DecodedPicture.Y_Width_image  = 0;
  Sessiondata_p->DecodedPicture.Y_Height_image = 0;
  Sessiondata_p->DecodedPicture.Y_Width_memory = 0;
  Sessiondata_p->DecodedPicture.Y  = NULL;
  Sessiondata_p->DecodedPicture.Cb = NULL;
  Sessiondata_p->DecodedPicture.Cr = NULL;
}
#endif
