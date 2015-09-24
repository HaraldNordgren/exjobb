/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file includes functions that uses the D65 API and implements a command-
* line decoder.
*
******************************************************************************/


/******************************************************************************
* Includes
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d65_api.h"
#ifdef _WIN32
#include <windows.h>
#endif//_WIN32
#include <sys/types.h> // _ftime() for timezone
#include <sys/timeb.h> // struct _timeb


/******************************************************************************
* Types and constants
******************************************************************************/

#if SUPPORT_EXTENSIONS
const unsigned int MAX_NBR_OF_LAYERS = 64;

typedef struct
{
  FILE                   *Outfile_p;
  D65_Handle_t           *Decoder_p;
  D65_DecoderParameters_t Parameters_p;
  D65_ReturnImage_t      *DecodedPicture_p;
} LayerData_t;
#endif

/******************************************************************************
* Declaration of functions
******************************************************************************/
void *
DecoderAllocate(const uint32 Size,
                const D65_Mem_Type_t Type);

void
DecoderFree(void* Data_p,
            const D65_Mem_Type_t Type);

void
Usage(const char *ProgramName,
      const char *ErrorString);

void
FindNextNAL(const uint8 *BitstreamBuffer,
            const sint32 Offset,
            const sint32 FileSize,
            sint32 *Position,
            sint32 *StartCodeLength);

#if SUPPORT_EXTENSIONS
uint32
GetLayerIDForNAL(const D65_InputNAL_t *InputNAL_p);

sint32
AddLayer(LayerData_t **LayerData_pp, uint32 LayerID, const uint8* OutputFilenameBaseLayer
#if MULTI_THREADED
         , const uint32 Threads_Number
#endif
         );

sint32 
FreeLayers(LayerData_t **LayerData_pp);
#endif

void
WritePictureOnFile(D65_ReturnImage_t *DecodedPicture_p,
                   FILE *Outfile_p);

sint32
DecodeSequence(const char *ProgramName,
               const char *Bitstream_Filename,
               const char *Output_Filename,
               const uint32 TraceFlag,
               const char *Trace_Filename
#if MULTI_THREADED
               ,const uint32 Threads_Number
#endif
#if ENABLE_OPENHEVC
               ,const uint8 OpenHevcFlag
#endif
               );


/******************************************************************************
 * Definition of local functions
******************************************************************************/

/******************************************************************************
*
* Name:        DecoderAllocate
*
* Parameters:  Size               [In]   Number of bytes to allocate
*              Type               [In]   Data type of the allocation
*
* Returns:     Allocated_Memory_p [Ret]  The allocated memory
*
* Description: The decoder function used by the decoder. A pointer to this
*              function is given to the decoder and it will be called when
*              the decoder performs allocations.
*
******************************************************************************/
void*
DecoderAllocate(const uint32 Size,
                const D65_Mem_Type_t Type)
{
  void* Allocated_Memory_p = malloc(Size);
  return Allocated_Memory_p;
}


/******************************************************************************
*
* Name:        DecoderFree
*
* Parameters:  Data_p  [In]  Pointer to memory that shall be freed
*              Type    [In]  Data type of the memory
*
*
* Returns:     -
*
* Description: The free function used by the decoder. A pointer to this
*              function is given to the decoder and it will be called when
*              the decoder is freeing memory.
*
******************************************************************************/
void
DecoderFree(void* Data_p,
            const D65_Mem_Type_t Type)
{
  free(Data_p);
}


/******************************************************************************
*
* Name:        Usage
*
* Parameters:  ErrorString  [In]   Error message
*
* Returns:     -
*
* Description: This function prints out how this command-line based program
*              should be used with regards to the arguments
*
******************************************************************************/
void
Usage(const char *ProgramName,
      const char *ErrorString)
{
  char version[50];
  D65_GetVersion(version);
  fprintf (stdout,"\nD65 - Ericsson H.265 Video Decoder Platform %s.\n\n", version);
  fprintf(stdout, "ERROR - %s\n", ErrorString);
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "%s [options] -i <filename> [more options]\n", ProgramName);
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "\t-i [filename] bitstream/rtpdump file [required parameter]\n");
  fprintf(stdout, "\t-o [filename] decoded file\n");

#if D65_BITTRACE
  fprintf(stdout, "\t-t [filename] tracefile, if filename omitted, stdout will be used\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\tRecompile without D65_BITTRACE in d65_api.h to maximize speed\n");
#else
  fprintf(stdout, "\n");
  fprintf(stdout, "\tRecompile with D65_BITTRACE to turn on bitstream trace functionality\n");
#endif
  
}


/******************************************************************************
*
* Name:        FindNextNAL
*
* Parameters:  BitstreamBuffer   [In]  The bitstream buffer
*              Offset            [In]  Buffer byte offset where search starts
*              FileSize          [In]  The size of the bitstream buffer
*              Position         [Out]  Buffer byte offset to the NAL start
*              StartCodeLength  [Out]  The length of the startcode in bytes
*
* Returns:     -
*
* Description: This function searches for the next NAL in a bitstream buffer.
*              Offset specifies where in the bitstream buffer the search shall
*              start. Position is an offset where the next NAL starts and
*              StartCodeLength tells you the size of the startcode, typically
*              this is 3 or 4 bytes (for the 0x000001 and 0x00000001 startcodes
*              respectively).
*
******************************************************************************/
void
FindNextNAL(const uint8 *BitstreamBuffer,
            const sint32 Offset,
            const sint32 FileSize,
            sint32 *Position,
            sint32 *StartCodeLength)
{
  sint32 zerocounter;
  sint32 position;
  
  zerocounter = 0;
  
  for(position = Offset ; position < FileSize-1 ; position++)
  {
    switch(BitstreamBuffer[position])
    {
    case 0:
      zerocounter++;
      break;
    case 1:
      if(zerocounter >= 2)
      {
        *Position = position+1;
        *StartCodeLength = zerocounter+1;
        return;
      }
      else
      {
        zerocounter = 0;
      }
      break;
    default:
      zerocounter = 0;
      break;
    }
  }
  
  *Position = -1;
  *StartCodeLength = -1;
}

#if SUPPORT_EXTENSIONS
/******************************************************************************
*
* Name:        GetLayerIdForNAL
*
* Parameters:  InputNAL_p          [In]      Pointer to NAL data
*              
* Returns:     NuhLayerId          [Ret]     Layer ID of NAL unit
*
* Description: Peeks in the NAL unit header for the layer ID of the NAL.
*
******************************************************************************/
uint32 
GetLayerIDForNAL(const D65_InputNAL_t *InputNAL_p) 
{
  return ((InputNAL_p->NALData_p[0]&1)<<5) + (InputNAL_p->NALData_p[1]>>3); 
}

/******************************************************************************
*
* Name:        AddLayer
*
* Parameters:  LayerData_pp        [In/Out]  Pointer to layer data for current layer
*              LayerID             [In]      Layer ID for the layer to be added
*              OutputFilenameBaselayer [In]  Output file name of base layer
*      
* Returns:     Return code         [Ret]
*
* Description: Adds and allocates a new layer with specified layer ID
*
******************************************************************************/
sint32
AddLayer(LayerData_t **LayerData_pp, uint32 LayerID, const uint8* OutputFilenameBaseLayer
#if MULTI_THREADED
         , const uint32 Threads_Number
#endif
         ) {
  char * pch;
  char temp[256];
  char OutputFilenameCurrentLayer[256];
  char LayerIDString[16];

  // Allocate data for layer
  LayerData_t *LayerData_p = malloc(sizeof(LayerData_t));
  *LayerData_pp = LayerData_p;

  LayerData_p->DecodedPicture_p = NULL;
  LayerData_p->Outfile_p = NULL;
  LayerData_p->Parameters_p.Malloc_Function_p = DecoderAllocate;
  LayerData_p->Parameters_p.Free_Function_p = DecoderFree;
  LayerData_p->Parameters_p.LayerID = LayerID;
#if MULTI_THREADED
  LayerData_p->Parameters_p.Threads_Number = Threads_Number;
#endif
#if ENABLE_OPENHEVC
  LayerData_p->Parameters_p.OpenHevcFlag = 0;
#endif
  // Init the decoder
  if(D65_Initialize(&LayerData_p->Decoder_p, &LayerData_p->Parameters_p) != D65_OK)
  {
    fprintf(stderr, "Error - D65_Initialize of non base layer\n");
    return(1);
  }


  if (OutputFilenameBaseLayer == NULL) 
  {
    LayerData_p->Outfile_p = NULL;
  }
  else
  {
    // Set and open output file for layer
#ifdef _WIN32
    _itoa(LayerID,LayerIDString,10);
#else//_WIN32
    snprintf(LayerIDString,10, (int)LayerID);
#endif//_WIN32
    strcpy(temp,OutputFilenameBaseLayer);
    OutputFilenameCurrentLayer[0] = '\0';
    pch = strtok(temp,".");
    strcat(OutputFilenameCurrentLayer, pch);
    strcat(OutputFilenameCurrentLayer, "_");
    strcat(OutputFilenameCurrentLayer, LayerIDString);
    strcat(OutputFilenameCurrentLayer, ".");
    pch = strtok(NULL,".");
    if (pch != NULL)
    {
      strcat(OutputFilenameCurrentLayer, pch);
    }

    LayerData_p->Outfile_p = fopen(OutputFilenameCurrentLayer, "wb");
    if(LayerData_p->Outfile_p == NULL)
    {
      fprintf(stderr, "Could not open output file for layer\n");
      return(1);
    }
  }

  return (0);
}

/******************************************************************************
*
* Name:        FreeLayers
*
* Parameters:  LayerData_pp      [In]  The list of layer data to free
*
* Returns:     Return code       [Ret]
*
* Description: This function writes a decoded picture on file if there is one.
*
******************************************************************************/
sint32
FreeLayers(LayerData_t **layerData_pp)
{
  uint32 LayerID;
  for (LayerID = 0; LayerID < MAX_NBR_OF_LAYERS; LayerID++)
  {
    if (layerData_pp[LayerID] != NULL)
    {

      if(layerData_pp[LayerID]->Outfile_p != NULL)
      {
        fclose(layerData_pp[LayerID]->Outfile_p);
      }
    
      //Release the decoder object for the current layer  
      if(D65_Release(layerData_pp[LayerID]->Decoder_p) != D65_OK)
      {
        fprintf(stderr, "Error - D65_Release for layer\n");
        return(1);
      }
    }
  }
  return (0);
}

#endif

/******************************************************************************
*
* Name:        WritePictureOnFile
*
* Parameters:  DecodedPicture_p  [In]  The image to write
*              Outfile_p         [In]  The file to write to
*
* Returns:     -
*
* Description: This function writes a decoded picture on file if there is one.
*
******************************************************************************/
void
WritePictureOnFile(D65_ReturnImage_t *DecodedPicture_p,
                   FILE *Outfile_p)
{
  uint32 y;

  if(Outfile_p == NULL)
  {
    return;
  }
  
  // Write decoded image on file
  for(y=0 ; y< DecodedPicture_p->Y_Height_image ; y++)
  {
    fwrite(DecodedPicture_p->Y + y*DecodedPicture_p->Y_Width_memory,
           sizeof(uint8),
           DecodedPicture_p->Y_Width_image,
           Outfile_p);
  }
  for(y=0 ; y< DecodedPicture_p->Y_Height_image/2 ; y++)
  {
    fwrite(DecodedPicture_p->Cb + y*(DecodedPicture_p->Y_Width_memory>>1),
           sizeof(uint8),
           DecodedPicture_p->Y_Width_image/2,
           Outfile_p);
  }
  for(y=0 ; y< DecodedPicture_p->Y_Height_image/2 ; y++)
  {
    fwrite(DecodedPicture_p->Cr + y*(DecodedPicture_p->Y_Width_memory>>1),
           sizeof(uint8),
           DecodedPicture_p->Y_Width_image/2,
           Outfile_p);
  }
  fflush(Outfile_p);
}




/******************************************************************************
*
* Name:        DecodeSequence
*
* Parameters:  ProgramName        [In]  The name of the program: argv[0]
*              Bitstream_Filename [In]  The name of the bitstream file
*              Output_Filename    [In]  The name of the decoded sequence file
*              Trace_Filename     [In]  The name of the trace file
*              TraceFlag          [In]  Whether trace is on or off
*
* Returns:     -
*
* Description: This function decodes one sequence. Open files, sets up a decoder,
*              performs decoding, releases the decoder and closes files.
*
******************************************************************************/
sint32
DecodeSequence(const char *ProgramName,
               const char *Bitstream_Filename,
               const char *Output_Filename,
               const uint32 TraceFlag,
               const char *Trace_Filename
#if MULTI_THREADED
               ,const uint32 Threads_Number
#endif
#if ENABLE_OPENHEVC
               ,const uint8 OpenHevcFlag
#endif
               )
{
  FILE *Infile_p;
  FILE *Outfile_p;
#if D65_BITTRACE
  FILE *Tracefile_p;
#endif
  uint32 Offset;
  uint32 FileSize;
  uint8 *BitstreamBuffer;
  D65_Handle_t* Decoder_p;
  D65_DecoderParameters_t Parameters_p;
  D65_ReturnCode_t Result = D65_OK;
  
  sint32 NALSize;
  sint32 CurrentNALPosition;
  sint32 NextNALPosition;
  sint32 StartCodeSize;
  
  D65_InputNAL_t InputNAL;
  D65_ReturnImage_t *DecodedPicture_p = NULL;
#if SUPPORT_EXTENSIONS
  uint32 i;
  uint32 LayerID = 0;
  // How to use MAX_NBR_OF_LAYERS here?
  LayerData_t *LayerData_p[64/*MAX_NBR_OF_LAYERS*/]; // Allocate pointers to layer data 
#endif

  static int NALcounter = 0;

  Outfile_p = NULL;
  Decoder_p = NULL;
#if SUPPORT_EXTENSIONS
  for (i = 0; i < MAX_NBR_OF_LAYERS; i++) {
    LayerData_p[i] = NULL;
  }
#endif
  
#ifdef _WIN32
  {
    __int64 counter, freq;
    double time;
    QueryPerformanceCounter( (LARGE_INTEGER*)&counter );
    QueryPerformanceFrequency( (LARGE_INTEGER*)&freq );
    time = counter / (DOUBLE)freq;
#endif
    

    // Open output file
    if(Output_Filename != NULL)
    {
      Outfile_p = fopen(Output_Filename, "wb");
      if(Outfile_p == NULL)
      {
        Usage(ProgramName, "Could not open output file");
        return(1);
      }
    }

        // Open bitstream file
    Infile_p = fopen(Bitstream_Filename, "rb");
    if(Infile_p == NULL)
    {
      Usage(ProgramName, "Could not open input file");
      return(1);
    }

    // Get filesize
    fseek (Infile_p, 0, SEEK_END);
    FileSize = ftell(Infile_p);
    fseek (Infile_p, 0, SEEK_SET);

    if(FileSize == 0)
    {
      Usage(ProgramName, "Bitstream file has zero size");
      return(1);
    }

    
    
    BitstreamBuffer = malloc(FileSize);
    if(BitstreamBuffer == NULL)
    {
      fprintf(stderr, "Error - Malloc returned NULL\n");
      return(1);
    }
    if(fread(BitstreamBuffer, 1, FileSize, Infile_p) != FileSize)
    {
      fprintf(stderr, "Error - fread did not read the entire file\n");
      return(1);
    }


    Parameters_p.Malloc_Function_p = DecoderAllocate;
    Parameters_p.Free_Function_p = DecoderFree;
#if SUPPORT_EXTENSIONS
    Parameters_p.LayerID = 0;
#endif

#if D65_CONFORMANCE
    Parameters_p.Write_Output_Function_p = WritePictureOnFile;
    Parameters_p.OutputLocation = Outfile_p;
#endif
    // Check parameters
    if(strlen(Bitstream_Filename) == 0)
    {
      Usage(ProgramName, "Required parameter missing - bitstream filename");
      return(1);
    }
    // Check parameters
#if D65_BITTRACE
    Tracefile_p = stdout;
    if(strcmp(Trace_Filename, "") != 0)
    {
      // Open trace file
      Tracefile_p = fopen(Trace_Filename, "wb");
      if(Tracefile_p == NULL)
      {
        Usage(ProgramName, "Could not open trace file");
        return(1);
      }
    }
       
    // Set encoder parameters
    Parameters_p.TraceFlag = TraceFlag;
    Parameters_p.Tracefile_p = Tracefile_p;
#endif

#if MULTI_THREADED
    Parameters_p.Threads_Number = Threads_Number;
#endif

#if ENABLE_OPENHEVC
    Parameters_p.OpenHevcFlag = OpenHevcFlag;
#endif

    // Init the decoder
    if(D65_Initialize(&Decoder_p, &Parameters_p) != D65_OK)
    {
      fprintf(stderr, "Error - D65_Initialize\n");
      return(1);
    }

    printf("Decoding %s\n", Bitstream_Filename);

    
   
      /************************/
      /* DECODE THE BITSTREAM */
      /************************/
      
      // Find first startcode
      FindNextNAL(BitstreamBuffer,
                  0,
                  FileSize,
                  &CurrentNALPosition,
                  &StartCodeSize);
      
      Offset = StartCodeSize;
      while(Offset < FileSize)
      {
        // If no startcode was found we are done with decoding the bitstream
        if(CurrentNALPosition == -1)
        {
          break;
        }
        
        // Find second startcode
        FindNextNAL(BitstreamBuffer,
                    CurrentNALPosition,
                    FileSize,
                    &NextNALPosition,
                    &StartCodeSize);
        
        // Set size of current NAL
        NALSize = NextNALPosition - CurrentNALPosition - StartCodeSize;
        
        // If no second startcode was found, we set NALSize to decode until EOF
        if(NextNALPosition == -1)
        {
          NALSize = FileSize - CurrentNALPosition;
        }
        
        InputNAL.NALData_p              = BitstreamBuffer + CurrentNALPosition;
        InputNAL.NALSize                = NALSize;
        InputNAL.UserPictureValue       = CurrentNALPosition;
        InputNAL.ExportDebugData        = 0;
        InputNAL.LastSliceInPictureFlag = 0;
        
        /******************/
        /* DECODE ONE NAL */
        /******************/
#if !SUPPORT_EXTENSIONS
#if EXTRA_INFO
        printf("\n*** Decoding NAL number %i ***\n", NALcounter);
#endif

        Result = D65_DecodeNAL(Decoder_p, &InputNAL, &DecodedPicture_p);
#else // SUPPORT_EXTENSIONS
        // Peek on LayerID in NAL header
        LayerID = GetLayerIDForNAL(&InputNAL);
#if EXTRA_INFO
        printf("\n*** Decoding NAL number %i for layer %i ***\n", NALcounter, LayerID);
#endif
        if (LayerID) 
        {
          if (LayerData_p[LayerID] == NULL)
          {
#if MULTI_THREADED
            AddLayer(&LayerData_p[LayerID], LayerID, Output_Filename, Threads_Number);
#else
            AddLayer(&LayerData_p[LayerID], LayerID, Output_Filename);
#endif
          }
          Result = D65_DecodeNAL(LayerData_p[LayerID]->Decoder_p, 
                                 &InputNAL, 
                                 &LayerData_p[LayerID]->DecodedPicture_p);
        }
        else
        {
          Result = D65_DecodeNAL(Decoder_p, &InputNAL, &DecodedPicture_p);
        }
#endif
        // Advance bitstream pointers to the next NAL
        Offset += NALSize;
        Offset += StartCodeSize;
        CurrentNALPosition = NextNALPosition;
        NALcounter++;

        switch(Result)
        {
        case D65_OK:
#if EXTRA_INFO
          printf("NAL decoded\n");
#endif
          break;
          
        case D65_SLICE_DECODED:
#if EXTRA_INFO
          printf("Slice decoded\n");
#endif
          break;
          
        case D65_PICTURE_DECODED:
#if EXTRA_INFO
          printf("Picture decoded\n");
#endif
          break;
          
        case D65_UNKNOWN_NAL_TYPE:
#if EXTRA_INFO
          printf("Unknown NAL type\n");
#endif
          break;
          
        case D65_BITSTREAM_ERROR:
#if EXTRA_INFO
          printf("Bitstream error\n");
#endif
          break;
          
        case D65_PICTURE_PARAMETER_SET_MISSING:
#if EXTRA_INFO
          printf("Picture parameter set missing\n");
#endif
          break;
          
        case D65_SEQUENCE_PARAMETER_SET_MISSING:
#if EXTRA_INFO
          printf("Sequence parameter set missing\n");
#endif
          break;

#if SUPPORT_EXTENSIONS
        case D65_VIDEO_PARAMETER_SET_MISSING:
#if EXTRA_INFO
          printf("VIDEO parameter set missing\n");
#endif
          break;
#endif

        case D65_MEMORY_ALLOCATION_ERROR:
#if EXTRA_INFO
          printf("Memory allocation error\n");
#endif
          break;
          
        default:
#if EXTRA_INFO
          printf("Unknown NAL return type\n");
#endif
          abort();
          break;
        }
#if SUPPORT_EXTENSIONS
        if (LayerID)
        {
          if(LayerData_p[LayerID]->DecodedPicture_p != NULL)
          {
            WritePictureOnFile(LayerData_p[LayerID]->DecodedPicture_p,LayerData_p[LayerID]->Outfile_p);
          }
        }
        else
        {
          if(DecodedPicture_p != NULL)
          {
            WritePictureOnFile(DecodedPicture_p,Outfile_p);
          }
        }
#else
        if(DecodedPicture_p != NULL)
        {
          WritePictureOnFile(DecodedPicture_p,Outfile_p);
        }
#endif

      }
#if SUPPORT_EXTENSIONS

      for (LayerID = 0; LayerID < MAX_NBR_OF_LAYERS; LayerID++) 
      {
        if(LayerID)
        {
          if (LayerData_p[LayerID] != NULL)
          {
            do
            {
              LayerData_p[LayerID]->DecodedPicture_p = D65_ForceOutOneDecodedPicture(LayerData_p[LayerID]->Decoder_p);
              if(LayerData_p[LayerID]->DecodedPicture_p != NULL)
              {
                WritePictureOnFile(LayerData_p[LayerID]->DecodedPicture_p,LayerData_p[LayerID]->Outfile_p);
              }
            } while (LayerData_p[LayerID]->DecodedPicture_p != NULL);
          }
        }
        else
        {
          do
          {
            DecodedPicture_p = D65_ForceOutOneDecodedPicture(Decoder_p);
            if(DecodedPicture_p != NULL)
            {
              WritePictureOnFile(DecodedPicture_p,Outfile_p);
            }
          } while(DecodedPicture_p != NULL);
        }
      }
#else
    
    // Get the rest of the picture from the DPB
    do
    {
      DecodedPicture_p = D65_ForceOutOneDecodedPicture(Decoder_p);
      if(DecodedPicture_p != NULL)
      {
        WritePictureOnFile(DecodedPicture_p,Outfile_p);
      }
    } while (DecodedPicture_p != NULL);
    
#endif
    
#ifdef _WIN32
    QueryPerformanceCounter( (LARGE_INTEGER*)&counter );
    QueryPerformanceFrequency( (LARGE_INTEGER*)&freq );
    printf("Time = %.4f\n", (counter / (DOUBLE)freq - time));
  }
#endif
  
  //Release the decoder 
  if(D65_Release(Decoder_p) != D65_OK)
  {
    fprintf(stderr, "Error - D65_Release\n");
    return(1);
  }
#if SUPPORT_EXTENSIONS
  FreeLayers(LayerData_p);
#endif

  fclose(Infile_p);
  free(BitstreamBuffer);

  if(Outfile_p != NULL)
  {
    fclose(Outfile_p);
  }
  return 0;
}


/******************************************************************************
*
* Name:        main
*
* Parameters:  Argc   [In]   Number of arguments
*              Argv   [In]   Array of arguments
*
* Returns:     uint32 [Out]  Standard main return value
*
* Description: The main function of the program. Sets up a decoder, performs
*              decoding and releases the decoder.
*
******************************************************************************/
uint32
main(const uint32 Argc,
     const char *Argv_pp[])
{
  uint32 i;
  uint32 TraceFlag;
  uint8 *OutputFileNamePointer_p;
  
  uint8 Bitstream_Filename[256] = "\0";
  uint8 Output_Filename[256] = "\0";
  uint8 Trace_Filename[256] = "\0";

#if MULTI_THREADED
  uint8 Threads_Number = 1;
#endif

#if ENABLE_OPENHEVC
  uint8 OpenHevcFlag = 0;
#endif
  
  TraceFlag = 0;
  OutputFileNamePointer_p = NULL;

  /* Parse parameters */
  for (i = 1; i < Argc; i++) 
  {
    if (*(Argv_pp[i]) == '-') 
    {
      switch(*(++Argv_pp[i])) 
      {
      case 'i':
      case 'b':
        if(i < Argc-1)
        {
          strcpy(Bitstream_Filename, Argv_pp[++i]);
        }
        break;
      case 'o':
        if(i < Argc-1)
        {
          strcpy(Output_Filename, Argv_pp[++i]);
          OutputFileNamePointer_p = Output_Filename;
        }
        break;
#if D65_BITTRACE
      case 't':
        TraceFlag = 1;
        if(i < Argc-1)
        {
          if(Argv_pp[i+1][0] != '-')
          {
            strcpy(Trace_Filename, Argv_pp[++i]);
          }
        }
        break;
#endif
#if MULTI_THREADED
      case 'T':
        if(i < Argc - 1)
        {
          if(isdigit(Argv_pp[i+1][0]))
          {
            Threads_Number = atoi(Argv_pp[++i]);
          }
        }
        break;
#endif
#if ENABLE_OPENHEVC
      case 'z':
        OpenHevcFlag = 1;
        break;
#endif
      }
    }
  }
  
  DecodeSequence(Argv_pp[0],
                 Bitstream_Filename,
                 OutputFileNamePointer_p,
                 TraceFlag,
                 Trace_Filename
#if MULTI_THREADED
                 ,Threads_Number
#endif
#if ENABLE_OPENHEVC
                 ,OpenHevcFlag
#endif
                 );
}

