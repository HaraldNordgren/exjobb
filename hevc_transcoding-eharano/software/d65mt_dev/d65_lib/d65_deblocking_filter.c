/******************************************************************************
* © Copyright (C) Ericsson AB 2012 - 2015. All rights reserved. 
* No part of this code may be reproduced in any form 
* without the written permission of the copyright holder. 
*******************************************************************************
* 
* DESCRIPTION
* 
* This file contains functions for deblocking filtering
*
******************************************************************************/
#include "d65_deblocking_filter.h"

/******************************************************************************
* Definition of local variables
******************************************************************************/
const static uint8 tc[54] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,10,11,13,14,16,18,20,22,24
};

const static uint8 beta[52] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

/******************************************************************************
* Definition of local functions
******************************************************************************/
sint32
GetQpL(SessionData_t *Sessiondata_p,
       const uint32 xC,
       const uint32 yC,
       const uint8 edgeType)
{
  const uint32 PicWidthInPu  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 PuSizeInPixel = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const uint32 PuCurrAddr    = ((yC >> PuSizeInPixel) * PicWidthInPu) + (xC >> PuSizeInPixel);
  uint32 xNb;
  uint32 yNb;
  uint32 PuNbAddr;
  sint32 QpL;

  if(edgeType == EDGE_VER)
  {
    xNb = xC - 1;
    yNb = yC;
  }
  else
  {
    xNb = xC;
    yNb = yC - 1;
  }
  PuNbAddr = ((yNb >> PuSizeInPixel) * PicWidthInPu) + (xNb >> PuSizeInPixel);
  QpL = (Sessiondata_p->PredictionUnitArray[PuCurrAddr].QpY + Sessiondata_p->PredictionUnitArray[PuNbAddr].QpY + 1) >> 1;
  return QpL;
}

/******************************************************************************
*
* Name:        EdgeFilteringChroma
*
* Parameters:  Sessiondata_p [in/out] The decoder
*              xC      [in]           Luma x coordinate in pixels
*              yC      [in]           Luma y coordinate in pixels
*              edgeType[in]           Edge type
*              cIdx    [in]           Color component index
*              boundaryStrength [in]  Boundary strength
*
* Returns:     -
*
* Description: Performs edge filtering for chroma
*
******************************************************************************/
void
EdgeFilteringChroma(SessionData_t *Sessiondata_p,
              const uint32 xC,
              const uint32 yC,
              const uint8 edgeType,
              const uint8 cIdx,
              uint8 boundaryStrength)
{
  static const sint32 CHROMA_QP_FROM_LUMA_QP[70] = 
  { 
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 
    36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63
  };
  sint32 QpC;
  sint32 Tc;
  sint32 q0,q1;
  sint32 p0,p1;
  sint32 k;
  sint32 delta;
  sint32 pqm;
  uint32 RecPicWidth = Sessiondata_p->CurrentRecImage->Image.C_Width_image;
  uint8 *ptr;
  if(cIdx == 1)
  {
    ptr = Sessiondata_p->CurrentRecImage->Image.Cb + (yC+(PADDING>>1))*RecPicWidth + xC + (PADDING>>1);
  }
  else
  {
    ptr = Sessiondata_p->CurrentRecImage->Image.Cr + (yC+(PADDING>>1))*RecPicWidth + xC + (PADDING>>1);
  }

  QpC = CHROMA_QP_FROM_LUMA_QP[D65_MIN(69, D65_MAX(0, GetQpL(Sessiondata_p, xC << 1, yC << 1, edgeType)))];//Sessiondata_p->CurrentQuant;];
  Tc = QpC + 2*(boundaryStrength - 1);
  Tc = tc[CLIP3(0,53,Tc)];
  if(edgeType == EDGE_VER)
  {
    for(k = 0; k < 2; k++)
    {
      p0 = ptr[-1];
      p1 = ptr[-2];
      q0 = ptr[0];
      q1 = ptr[1];
      delta = ((((q0-p0)<<2)+p1-q1+4)>>3);
      delta = CLIP3(-Tc,Tc,delta);
      pqm = delta + p0;
      ptr[-1] = (uint8)CLIP3(0,255,pqm);
      pqm = q0 - delta;
      ptr[0] = (uint8)CLIP3(0,255,pqm);
      ptr += RecPicWidth;
    }
  }
  else
  {
    for(k = 0; k < 2; k++)
    {
      p0 = ptr[-1*RecPicWidth+k];
      p1 = ptr[-2*RecPicWidth+k];
      q0 = ptr[k];
      q1 = ptr[RecPicWidth+k];
      delta = ((((q0-p0)<<2)+p1-q1+4)>>3);
      delta = CLIP3(-Tc,Tc,delta);
      pqm = delta + p0;
      ptr[-1*RecPicWidth+k] = (uint8)CLIP3(0,255,pqm);
      pqm = q0 - delta;
      ptr[k] = (uint8)CLIP3(0,255,pqm);
    }
  }

}

/******************************************************************************
*
* Name:        EdgeFilteringLuma
*
* Parameters:  Sessiondata_p [in/out] The decoder
*              xC      [in]           Luma x coordinate in pixels
*              yC      [in]           Luma y coordinate in pixels
*              edgeType[in]           Edge type
*              cIdx    [in]           Color component index
*              boundaryStrength [in]  Boundary strength
*
* Returns:     -
*
* Description: Performs edge filtering for luma
*
******************************************************************************/
void
EdgeFilteringLuma(SessionData_t *Sessiondata_p,
              const uint32 xC,
              const uint32 yC,
              const uint8 edgeType,
              uint8 boundaryStrength)
{

  sint32 QpL;
  sint32 Beta;
  sint32 Tc;
  sint32 dpq;
  sint32 dpq0;
  sint32 dpq3;
  sint32 dp;
  sint32 dq;
  sint32 dqp;
  sint32 d;
  sint32 dp0;
  sint32 dp3;
  sint32 dq0;
  sint32 dq3;
//sint32 dE;
  sint32 dEp;
  sint32 dEq;
  sint32 k;
  sint32 dpTmp1;
  sint32 dpTmp2;
  sint32 dpTmp3;

  sint32 dSam0;
  sint32 dSam3;
  sint32 p0,p1,p2,p3;
  sint32 q0,q1,q2,q3;
  sint32 delta;
  sint32 pqm;
  sint32 commonAddSum;

  uint32 RecPicWidth = Sessiondata_p->CurrentRecImage->Image.Y_Width_image;
  uint8 *ptr  = Sessiondata_p->CurrentRecImage->Image.Y+(yC+PADDING)*RecPicWidth+xC+PADDING;
  uint8 *ptr2 = ptr + 3*RecPicWidth; 

  QpL = GetQpL(Sessiondata_p, xC, yC, edgeType);//Sessiondata_p->CurrentQuant;
  Beta = beta[CLIP3(0,51,QpL)];
  Tc   = QpL + 2*(boundaryStrength-1);
  Tc   = tc[CLIP3(0,53,Tc)];

  if(edgeType == EDGE_VER)
  {
    dp0 =  ptr[-3] - 2*ptr[-2] + ptr[-1]; 
    dp0 = D65_ABS(dp0);
    dp3 = ptr2[-3] - 2*ptr2[-2] + ptr2[-1];
    dp3 = D65_ABS(dp3);
    dq0 = ptr[0] - 2*ptr[1] + ptr[2];
    dq0 = D65_ABS(dq0);
    dq3 = ptr2[0] - 2*ptr2[1] + ptr2[2];
    dq3 = D65_ABS(dq3);

    dpq0 = dp0 + dq0;
    dpq3 = dp3 + dq3;
    d    = dpq0 + dpq3;

    if(d < Beta)
    {
      dpq = dpq0 << 1;
      dSam0 = 0;

      dpTmp1 = ptr[-4] - ptr[-1];
      dpTmp1 = D65_ABS(dpTmp1);
      dpTmp2 = ptr[0] - ptr[3];
      dpTmp2 = D65_ABS(dpTmp2);
      dpTmp3 = ptr[-1] - ptr[0];
      dpTmp3 = D65_ABS(dpTmp3);
      
      if((dpq < (Beta>>2)) 
        && ((dpTmp1+dpTmp2) < (Beta>>3))
        && (dpTmp3 < ((5*Tc + 1)>>1)))
      {
        dSam0 = 1;
      }

      dqp = dpq3 << 1;
      dSam3 = 0;
      dpTmp1 = ptr2[-4] - ptr2[-1];
      dpTmp1 = D65_ABS(dpTmp1);
      dpTmp2 = ptr2[0] - ptr2[3];
      dpTmp2 = D65_ABS(dpTmp2);
      dpTmp3 = ptr2[-1] - ptr2[0];
      dpTmp3 = D65_ABS(dpTmp3);
      if((dqp < (Beta>>2))
        &&((dpTmp1+dpTmp2)<(Beta >> 3))
        &&(dpTmp3 < ((5*Tc + 1)>>1)))
      {
        dSam3 = 1;
      }
      //dE = 1;
      if(dSam0 && dSam3)
      {
        //dE = 2;
        for(k = 0; k < 4; k++)
        {
          p3 = ptr[-4];
          p2 = ptr[-3];
          p1 = ptr[-2];
          p0 = ptr[-1];
          q0 = ptr[0];
          q1 = ptr[1];
          q2 = ptr[2];
          q3 = ptr[3];

          commonAddSum = p1 + p0 + q0 + 2;
          pqm = (p2 + (commonAddSum << 1) + q1) >> 3;
          ptr[-1] = (uint8)CLIP3(p0-2*Tc,p0+2*Tc,pqm);
          pqm = (p2 + commonAddSum) >> 2;
          ptr[-2] = (uint8)CLIP3(p1-2*Tc,p1+2*Tc,pqm);
          pqm = (2*p3 + 3*p2 + commonAddSum + 2) >> 3;
          ptr[-3] = (uint8)CLIP3(p2-2*Tc,p2+2*Tc,pqm);
          commonAddSum = p0 + q0 + q1 + 2;
          pqm = (p1 + (commonAddSum << 1) + q2) >> 3;
          ptr[0] = (uint8)CLIP3(q0-2*Tc,q0+2*Tc,pqm);
          pqm = (commonAddSum + q2) >> 2;
          ptr[1] = (uint8)CLIP3(q1-2*Tc,q1+2*Tc,pqm);
          pqm = (commonAddSum+ 3*q2 + 2*q3 + 2) >> 3;
          ptr[2] = (uint8)CLIP3(q2-2*Tc,q2+2*Tc,pqm);
          ptr += RecPicWidth;
        }
      }
      else
      {
        dp   = dp0 + dp3;
        dq   = dq0 + dq3;
        dEp  = 0;
        dEq  = 0;
        if (dp < ((Beta+(Beta>>1))>>3))
        {
          dEp = 1;
        }
        if (dq < ((Beta+(Beta>>1))>>3))
        {
          dEq = 1;
        }
        for(k = 0; k < 4; k++)
        {
          p2 = ptr[-3];
          p1 = ptr[-2];
          p0 = ptr[-1];
          q0 = ptr[0];
          q1 = ptr[1];
          q2 = ptr[2];

          delta = (9*(q0-p0) - 3*(q1-p1) + 8) >> 4;
          if(D65_ABS(delta) < 10*Tc)
          {
            delta = CLIP3(-Tc,Tc,delta);
            pqm   = p0 + delta;
            ptr[-1] = (uint8)CLIP3(0,255,pqm);
            pqm   = q0 - delta;
            ptr[0] = (uint8)CLIP3(0,255,pqm);
            if(dEp == 1)
            {
              pqm = (((p2 + p0 + 1)>>1) - p1 + delta)>>1;
              pqm = CLIP3(-(Tc>>1),(Tc>>1),pqm);
              pqm += p1;
              ptr[-2] = (uint8)CLIP3(0,255,pqm);
            }
            if(dEq == 1)
            {
              pqm = (((q2 + q0 + 1)>>1) - q1 - delta) >> 1;
              pqm = CLIP3(-(Tc>>1),(Tc>>1),pqm);
              pqm += q1;
              ptr[1] = (uint8)CLIP3(0,255,pqm);
            }
          }
          ptr += RecPicWidth;
        }
      }
    }
  }
  else
  {
    uint8 *ptrAbove4 = ptr - 4*RecPicWidth;
    uint8 *ptrAbove3 = ptrAbove4 + RecPicWidth;
    uint8 *ptrAbove2 = ptrAbove3 + RecPicWidth;
    uint8 *ptrAbove1 = ptrAbove2 + RecPicWidth;
    uint8 *ptrBelow1 = ptr + RecPicWidth;
    uint8 *ptrBelow2 = ptrBelow1 + RecPicWidth;
    uint8 *ptrBelow3 = ptrBelow2 + RecPicWidth;

    dp0 = ptrAbove3[0] - 2*ptrAbove2[0] + ptrAbove1[0];
    dp0 = D65_ABS(dp0);
    dp3 = ptrAbove3[3] - 2*ptrAbove2[3] + ptrAbove1[3];
    dp3 = D65_ABS(dp3);
    dq0 = ptr[0] - 2*ptrBelow1[0] + ptrBelow2[0];
    dq0 = D65_ABS(dq0);
    dq3 = ptr[3] - 2*ptrBelow1[3] + ptrBelow2[3];
    dq3 = D65_ABS(dq3);

    dpq0 = dp0 + dq0;
    dpq3 = dp3 + dq3;
    d = dpq0 + dpq3;

    if(d < Beta)
    {
      dpq = 2*dpq0;
      dpTmp1 = ptrAbove4[0] - ptrAbove1[0];
      dpTmp1 = D65_ABS(dpTmp1);
      dpTmp2 = ptr[0] - ptrBelow3[0];
      dpTmp2 = D65_ABS(dpTmp2);
      dpTmp3 = ptrAbove1[0] - ptr[0];
      dpTmp3 = D65_ABS(dpTmp3);

      dSam0 = 0;
      if((dpq < (Beta >> 2))
        && ((dpTmp1 + dpTmp2) < (Beta >> 3))
        && (dpTmp3 < ((5*Tc+1)>>1)))
      {
        dSam0 = 1;
      }

      dqp = 2*dpq3;
      dpTmp1 = ptrAbove4[3] - ptrAbove1[3];
      dpTmp1 = D65_ABS(dpTmp1);
      dpTmp2 = ptr[3] - ptrBelow3[3];
      dpTmp2 = D65_ABS(dpTmp2);
      dpTmp3 = ptrAbove1[3] - ptr[3];
      dpTmp3 = D65_ABS(dpTmp3);

      dSam3 = 0;
      if((dqp < (Beta >> 2))
        && ((dpTmp1 + dpTmp2) < (Beta >> 3))
        && (dpTmp3 < ((5*Tc+1)>>1)))
      {
        dSam3 = 1;
      }

      // dE = 1;
      if(dSam0 && dSam3)
      {
        // dE = 2;
        for(k = 0; k < 4; k++)
        {
          p0 = ptrAbove1[k];
          p1 = ptrAbove2[k];
          p2 = ptrAbove3[k];
          q0 = ptr[k];
          q1 = ptrBelow1[k];
          q2 = ptrBelow2[k];
          p3 = ptrAbove4[k];
          q3 = ptrBelow3[k];

          commonAddSum = p1 + p0 + q0 + 2;
          pqm = (p2 + (commonAddSum << 1) + q1) >> 3;
          ptrAbove1[k] = (uint8)CLIP3(p0-2*Tc,p0+2*Tc,pqm);
          pqm = (p2 + commonAddSum) >> 2;
          ptrAbove2[k] = (uint8)CLIP3(p1-2*Tc,p1+2*Tc,pqm);
          pqm = (2*p3 + 3*p2 + commonAddSum + 2) >> 3;
          ptrAbove3[k] = (uint8)CLIP3(p2-2*Tc,p2+2*Tc,pqm);
          commonAddSum = p0 + q0 + q1 + 2;
          pqm = (p1 + (commonAddSum << 1) + q2) >> 3;
          ptr[k] = (uint8)CLIP3(q0-2*Tc,q0+2*Tc,pqm);
          pqm = (commonAddSum + q2) >> 2;
          ptrBelow1[k] = (uint8)CLIP3(q1-2*Tc,q1+2*Tc,pqm);
          pqm = (commonAddSum + 3*q2 + 2*q3 + 2) >> 3;
          ptrBelow2[k] = (uint8)CLIP3(q2-2*Tc,q2+2*Tc,pqm);
        }

      }
      else
      {
        dp = dp0 + dp3;
        dq = dq0 + dq3;
        dEp = 0;
        dEq = 0;
        if (dp < ((Beta+(Beta>>1))>>3))
        {
          dEp = 1;
        }
        if (dq < ((Beta+(Beta>>1))>>3))
        {
          dEq = 1;
        }
        for(k = 0; k < 4; k++)
        {
          p0 = ptrAbove1[k];
          p1 = ptrAbove2[k];
          p2 = ptrAbove3[k];
          q0 = ptr[k];
          q1 = ptrBelow1[k];
          q2 = ptrBelow2[k];
          delta = (9*(q0-p0) - 3*(q1-p1) + 8) >> 4;
          if(D65_ABS(delta) < 10*Tc)
          {
            delta = CLIP3(-Tc,Tc,delta);
            pqm   = p0 + delta;
            ptrAbove1[k]  = (uint8)CLIP3(0,255,pqm);
            pqm   = q0 - delta;
            ptr[k]      = (uint8)CLIP3(0,255,pqm);
            if(dEp == 1)
            {
              pqm = (((p2 + p0 + 1)>>1) - p1 + delta)>>1;
              pqm = CLIP3(-(Tc>>1),(Tc>>1),pqm);
              pqm += p1;
              ptrAbove2[k] = (uint8)CLIP3(0,255,pqm);
            }
            if(dEq == 1)
            {
              pqm = (((q2 + q0 + 1)>>1) - q1 - delta) >> 1;
              pqm = CLIP3(-(Tc>>1),(Tc>>1),pqm);
              pqm += q1;
              ptrBelow1[k] = (uint8)CLIP3(0,255,pqm);
            }
          }
        }
      }
    }
    }
}

/******************************************************************************
*
* Name:        CheckReferencePicturesEqual
*
* Parameters:  Sessiondata_p [in] The decoder
*              RefIdxAL0     [in] List 0 reference index for MV A
*              RefIdxAL1     [in] List 1 reference index for MV A
*              RefIdxBL0     [in] List 0 reference index for MV B
*              RefIdxBL1     [in] List 1 reference index for MV B
*
* Returns:     - uint8
*
* Description: Check if the sets of reference pictures used by MV A and B are equal
*              Assumes RefIdxYLX does not equal -1 for Y in {A,B} and X in {0,1}
*
******************************************************************************/
uint8 
CheckReferencePicturesEqual(SessionData_t *Sessiondata_p, 
                            sint8 RefIdxAL0, 
                            sint8 RefIdxAL1, 
                            sint8 RefIdxBL0, 
                            sint8 RefIdxBL1)
{

  ReferencePicture_t *RefA0 = Sessiondata_p->RefPicList0[RefIdxAL0];
  ReferencePicture_t *RefA1 = Sessiondata_p->RefPicList1[RefIdxAL1];
  ReferencePicture_t *RefB0 = Sessiondata_p->RefPicList0[RefIdxBL0];
  ReferencePicture_t *RefB1 = Sessiondata_p->RefPicList1[RefIdxBL1];

  if(RefA0 && RefA1 && RefB0 && RefB1)
  {
    if((RefA0->PicOrderCnt == RefB0->PicOrderCnt) &&
       (RefA1->PicOrderCnt == RefB1->PicOrderCnt))
    {
      return 1;
    }
    else if((RefA0->PicOrderCnt == RefB1->PicOrderCnt) &&
            (RefA1->PicOrderCnt == RefB0->PicOrderCnt))
    {
      return 1;
    }
    return 0;
  }
  return 1;
}

/******************************************************************************
*
* Name:        GetBoundaryStrength
*
* Parameters:  CurrentPU_p [in] Pointer to current prediction unit
*              NeiborPU_p  [in] Pointer to neighboring prediction unit (left or up)
*
* Returns:     - uint8
*
* Description: Get Boundary strength for a CU
*
******************************************************************************/
uint8
GetBoundaryStrength(SessionData_t * Sessiondata_p,
                    PredictionUnit_t * CurrentPU_p,
                    PredictionUnit_t * NeiborPU_p,
                    uint8 CheckCbf)
{
  Vector_t MvCurL0; 
  Vector_t MvCurL1;
  Vector_t MvNeiL0;
  Vector_t MvNeiL1;
  uint8 numCurMVs;
  uint8 numNeiMVs;
  sint32 MvHorDiff;
  sint32 MvVerDiff;
  uint8 RefPicEqual;
  ReferencePicture_t *CurRefPic = NULL;
  ReferencePicture_t *NeiRefPic = NULL;

  /* Intramode in one of the PUs gives boundary strength 2 */
  if(CurrentPU_p->PredMode == MODE_INTRA || NeiborPU_p->PredMode == MODE_INTRA)
  {
    return 2;
  }
  else if(CheckCbf && (CurrentPU_p->CbfLuma || NeiborPU_p->CbfLuma))
  {
    return 1;
  }

  MvCurL0 = CurrentPU_p->MotionvectorL0;
  MvCurL1 = CurrentPU_p->MotionvectorL1;
  MvNeiL0 = NeiborPU_p->MotionvectorL0;
  MvNeiL1 = NeiborPU_p->MotionvectorL1;

  /* Calculate the number of MVs used in current pb and neighboring pb. */
  numCurMVs = (MvCurL0.ref_idx != -1) + (MvCurL1.ref_idx != -1);
  numNeiMVs = (MvNeiL0.ref_idx != -1) + (MvNeiL1.ref_idx != -1);

  if(numCurMVs != numNeiMVs)
  {
    /* Uniprediction for one and biprediction for the other */
    return 1;
  }
  else if(numCurMVs == 1) 
  {
    /* Uniprediction. Need to check if different ref pictures are used and if vector difference is large. */
    if(MvCurL0.ref_idx != -1)
    {
      if(MvNeiL0.ref_idx != -1)
      {
        MvHorDiff = MvCurL0.x - MvNeiL0.x;
        MvVerDiff = MvCurL0.y - MvNeiL0.y;
        CurRefPic   = Sessiondata_p->RefPicList0[MvCurL0.ref_idx];
        NeiRefPic   = Sessiondata_p->RefPicList0[MvNeiL0.ref_idx];
        RefPicEqual = (CurRefPic && NeiRefPic) ? (CurRefPic->PicOrderCnt == NeiRefPic->PicOrderCnt) : 0;
      }
      else
      {
        MvHorDiff = MvCurL0.x - MvNeiL1.x;
        MvVerDiff = MvCurL0.y - MvNeiL1.y;
        CurRefPic   = Sessiondata_p->RefPicList0[MvCurL0.ref_idx];
        NeiRefPic   = Sessiondata_p->RefPicList1[MvNeiL1.ref_idx];
        RefPicEqual = (CurRefPic && NeiRefPic) ? (CurRefPic->PicOrderCnt == NeiRefPic->PicOrderCnt) : 0;
      }
    }
    else
    {
      if(MvNeiL0.ref_idx != -1)
      {
        MvHorDiff = MvCurL1.x - MvNeiL0.x;
        MvVerDiff = MvCurL1.y - MvNeiL0.y;
        CurRefPic   = Sessiondata_p->RefPicList1[MvCurL1.ref_idx];
        NeiRefPic   = Sessiondata_p->RefPicList0[MvNeiL0.ref_idx];
        RefPicEqual = (CurRefPic && NeiRefPic) ? (CurRefPic->PicOrderCnt == NeiRefPic->PicOrderCnt) : 0;
      }
      else
      {
        MvHorDiff = MvCurL1.x - MvNeiL1.x;
        MvVerDiff = MvCurL1.y - MvNeiL1.y;
        CurRefPic   = Sessiondata_p->RefPicList1[MvCurL1.ref_idx];
        NeiRefPic   = Sessiondata_p->RefPicList1[MvNeiL1.ref_idx];
        RefPicEqual = (CurRefPic && NeiRefPic) ? (CurRefPic->PicOrderCnt == NeiRefPic->PicOrderCnt) : 0;
      }
    }
    return (!RefPicEqual) || (D65_ABS(MvHorDiff) >= 4) || (D65_ABS(MvVerDiff) >= 4);
  }
  /* Biprediction used in both PBs */
  else if(!CheckReferencePicturesEqual(Sessiondata_p, MvCurL0.ref_idx, MvCurL1.ref_idx, MvNeiL0.ref_idx, MvNeiL1.ref_idx))
  {
    /* Reference pictures are not the same for current and neibor PB */
    return 1;
  }
  else if(Sessiondata_p->RefPicList0[MvCurL0.ref_idx] != Sessiondata_p->RefPicList1[MvCurL1.ref_idx])
  {
    /* Two different reference pictures used in each PB and they are equal */
    if(Sessiondata_p->RefPicList0[MvCurL0.ref_idx] == Sessiondata_p->RefPicList0[MvNeiL0.ref_idx])
    {
      MvHorDiff = MvCurL0.x - MvNeiL0.x;
      MvVerDiff = MvCurL0.y - MvNeiL0.y;
      if((D65_ABS(MvHorDiff) >= 4) || (D65_ABS(MvVerDiff) >= 4))
      {
        return 1;
      }

      MvHorDiff = MvCurL1.x - MvNeiL1.x;
      MvVerDiff = MvCurL1.y - MvNeiL1.y;
      if((D65_ABS(MvHorDiff) >= 4) || (D65_ABS(MvVerDiff) >= 4))
      {
        return 1;
      }
    }
    else
    {
      MvHorDiff = MvCurL0.x - MvNeiL1.x;
      MvVerDiff = MvCurL0.y - MvNeiL1.y;
      if((D65_ABS(MvHorDiff) >= 4) || (D65_ABS(MvVerDiff) >= 4))
      {
        return 1;
      }

      MvHorDiff = MvCurL1.x - MvNeiL0.x;
      MvVerDiff = MvCurL1.y - MvNeiL0.y;
      if((D65_ABS(MvHorDiff) >= 4) || (D65_ABS(MvVerDiff) >= 4))
      {
        return 1;
      }
    }
  }
  else
  {
    /* One reference picture used by both PBs */
    
    /* Check vector difference for lists with same index */
    MvHorDiff = MvCurL0.x - MvNeiL0.x;
    MvVerDiff = MvCurL0.y - MvNeiL0.y;
    if((D65_ABS(MvHorDiff) < 4) && (D65_ABS(MvVerDiff) < 4))
    {

      MvHorDiff = MvCurL1.x - MvNeiL1.x;
      MvVerDiff = MvCurL1.y - MvNeiL1.y;
      if((D65_ABS(MvHorDiff) < 4) && (D65_ABS(MvVerDiff) < 4))
      {
        return 0;
      }
    }

    /* Check vector difference for lists with opposite index */
    MvHorDiff = MvCurL0.x - MvNeiL1.x;
    MvVerDiff = MvCurL0.y - MvNeiL1.y;
    if((D65_ABS(MvHorDiff) < 4) && (D65_ABS(MvVerDiff) < 4))
    {
      MvHorDiff = MvCurL1.x - MvNeiL0.x;
      MvVerDiff = MvCurL1.y - MvNeiL0.y;
      if((D65_ABS(MvHorDiff) < 4) && (D65_ABS(MvVerDiff) < 4))
      {
        return 0;
      }
    }
    return 1;
  }

  return 0;
}

/******************************************************************************
*
* Name:        Deblock_Tile
*
* Parameters:  Sessiondata_p [in/out] The decoder
*              x0        [in]           TU x coordinate in PUs
*              y0        [in]           TU y coordinate in PUs
*              edgeType  [in]           Edge type
*
* Returns:     -
*
* Description: Perform Deblocking for a Tile
*
******************************************************************************/
void
Deblock_Tile(SessionData_t *Sessiondata_p,
             sint32 StartX,
             sint32 StartY,
             sint32 StopX,
             sint32 StopY,
             uint8 edgeType)
{
  const uint32 PicWidthInPU               = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 Log2MinPUSizeInPixels      = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;

  uint32 PuAddr;
  uint8 boundaryStrength;
  sint32 xD;
  sint32 yD;
  sint32 x;
  sint32 y;
  switch(edgeType)
  {
  case EDGE_VER:
    StartX = (StartX == 0) ? 2 : StartX;
    for(yD = StartY; yD < StopY; yD += 1)
    {
      for(xD = StartX ; xD < StopX ; xD += 2)
      {
        PuAddr = yD*PicWidthInPU + xD;
        if(Sessiondata_p->EdgeFlags[PuAddr])
        {
          boundaryStrength = GetBoundaryStrength(Sessiondata_p, Sessiondata_p->PredictionUnitArray + PuAddr, Sessiondata_p->PredictionUnitArray + PuAddr - 1, Sessiondata_p->EdgeFlags[PuAddr] & 0x2);

          if(boundaryStrength > 0)
          {
            x = xD << Log2MinPUSizeInPixels;
            y = yD << Log2MinPUSizeInPixels;
            EdgeFilteringLuma(Sessiondata_p,x,y,0,boundaryStrength);
            if(boundaryStrength > 1 && (x % 16 == 0))
            {
              EdgeFilteringChroma(Sessiondata_p,x >> 1,y >> 1,0,1,boundaryStrength);
              EdgeFilteringChroma(Sessiondata_p,x >> 1,y >> 1,0,2,boundaryStrength);
            }
          }
        }
      }
    }
    break;
  case EDGE_HOR:
    StartY = (StartY == 0) ? 2 : StartY;
    for(xD = StartX; xD < StopX; xD += 1)
    {
      for(yD = StartY; yD < StopY; yD += 2)
      {
        PuAddr = yD*PicWidthInPU + xD;
        if(Sessiondata_p->EdgeFlagsHor[PuAddr])
        {
          boundaryStrength = GetBoundaryStrength(Sessiondata_p, Sessiondata_p->PredictionUnitArray + PuAddr, Sessiondata_p->PredictionUnitArray + PuAddr - PicWidthInPU, Sessiondata_p->EdgeFlagsHor[PuAddr] & 0x2);

          if(boundaryStrength > 0)
          {
            x = xD << Log2MinPUSizeInPixels;
            y = yD << Log2MinPUSizeInPixels;
            EdgeFilteringLuma(Sessiondata_p,x,y,1,boundaryStrength);
            if(boundaryStrength > 1 && (y % 16 == 0))
            {
              EdgeFilteringChroma(Sessiondata_p,x >> 1,y >> 1,1,1,boundaryStrength);
              EdgeFilteringChroma(Sessiondata_p,x >> 1,y >> 1,1,2,boundaryStrength);
            }
          }
        }
      }
    }
    break;
  default:
    break;
  }
}

#if MULTI_THREADED
void*
Deblock_Tile_Ver_threaded(void *VoidThreaddata_p)
{
  ThreadData_t *Threaddata_p = (ThreadData_t*)VoidThreaddata_p;
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;

  const uint32 Log2MinPUSizeInPixels      = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  uint32 Log2CtuSizeInPU = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - Log2MinPUSizeInPixels;

  uint32 StartX;
  uint32 StartY;
  uint32 StopX;
  uint32 StopY;

  uint32 TileNum;

  for(TileNum = 0; TileNum < Threaddata_p->TilesNumForThisThread; TileNum++)
  {
    StartX = Threaddata_p->Tiledata_p[TileNum].TileStartCtuX << Log2CtuSizeInPU;
    StartY = Threaddata_p->Tiledata_p[TileNum].TileStartCtuY << Log2CtuSizeInPU;
    StopX  = Threaddata_p->Tiledata_p[TileNum].TileStopCtuX  << Log2CtuSizeInPU;
    StopY  = Threaddata_p->Tiledata_p[TileNum].TileStopCtuY  << Log2CtuSizeInPU;

    Deblock_Tile(Sessiondata_p, StartX, StartY, StopX, StopY, EDGE_VER);
  }
  return NULL;
}

void*
Deblock_Tile_Hor_threaded(void *VoidThreaddata_p)
{
  ThreadData_t *Threaddata_p = (ThreadData_t*)VoidThreaddata_p;
  SessionData_t *Sessiondata_p = (SessionData_t*)Threaddata_p->Sessiondata_p;

  const uint32 Log2MinPUSizeInPixels      = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  uint32 Log2CtuSizeInPU = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY - Log2MinPUSizeInPixels;

  uint32 StartX;
  uint32 StartY;
  uint32 StopX;
  uint32 StopY;
  
  uint32 TileNum;

  for(TileNum = 0; TileNum < Threaddata_p->TilesNumForThisThread; TileNum++)
  {
    StartX = Threaddata_p->Tiledata_p[TileNum].TileStartCtuX << Log2CtuSizeInPU;
    StartY = Threaddata_p->Tiledata_p[TileNum].TileStartCtuY << Log2CtuSizeInPU;
    StopX  = Threaddata_p->Tiledata_p[TileNum].TileStopCtuX  << Log2CtuSizeInPU;
    StopY  = Threaddata_p->Tiledata_p[TileNum].TileStopCtuY  << Log2CtuSizeInPU;

    Deblock_Tile(Sessiondata_p, StartX, StartY, StopX, StopY, EDGE_HOR);
  }
  return NULL;
}

#endif

/******************************************************************************
* Definition of external functions
******************************************************************************/

/******************************************************************************
*
* Name:        DeblockingFilter 
*
******************************************************************************/

void
DeblockingFilter(SessionData_t *Sessiondata_p)
{
#if MULTI_THREADED
  uint8  DeblockingOnFlag;
  uint32 NumThreads;
  uint32 NumEntries = Sessiondata_p->CurrentNumOfEntries;
  ThreadPool_t *Threadpool_p = &Sessiondata_p->ThreadsPool;
  DeblockingOnFlag = !(Sessiondata_p->CurrentSliceDisableDeblockingFlag);

  if(DeblockingOnFlag)
  {
    NumThreads = Sessiondata_p->CurrentNumOfThreads;
    if(NumThreads > 1)
    {
      pthread_mutex_lock(&Threadpool_p->ThreadLock);
      Threadpool_p->Jobs_p = Deblock_Tile_Ver_threaded;
      Threadpool_p->RemainingJobsNumber = NumEntries;
      Threadpool_p->CurrentJobId = 0;
      Threadpool_p->TotalJobNumber = NumEntries;
      pthread_cond_broadcast(&Threadpool_p->JobsArrayNotEmpty);
      pthread_cond_wait(&Threadpool_p->JobsArrayEmpty, &Threadpool_p->ThreadLock);
      pthread_mutex_unlock(&Threadpool_p->ThreadLock);


      pthread_mutex_lock(&Threadpool_p->ThreadLock);
      Threadpool_p->Jobs_p = Deblock_Tile_Hor_threaded;
      Threadpool_p->RemainingJobsNumber = NumEntries;
      Threadpool_p->CurrentJobId = 0;
      Threadpool_p->TotalJobNumber = NumEntries;
      pthread_cond_broadcast(&Threadpool_p->JobsArrayNotEmpty);
      pthread_cond_wait(&Threadpool_p->JobsArrayEmpty, &Threadpool_p->ThreadLock);
      pthread_mutex_unlock(&Threadpool_p->ThreadLock);
    }
    else
    {
      Deblock_Tile_Ver_threaded((void*)&(Sessiondata_p->Threaddata_p[0]));
      Deblock_Tile_Hor_threaded((void*)&(Sessiondata_p->Threaddata_p[0]));
    }
  }
#else
  const uint32 Log2CTUsize                = Sessiondata_p->CurrentSequenceParameterSet->Log2CtbSizeY;
  const uint32 PicWidthInPU               = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
  const uint32 Log2MinPUSizeInPixels      = Sessiondata_p->CurrentSequenceParameterSet->Log2MinUnitSize;
  const PictureParameterSet_t *CurrPicSet = Sessiondata_p->CurrentPictureParameterSet;
  uint32 Log2CTUsizeInPU = Log2CTUsize - Log2MinPUSizeInPixels;
  uint32 maxTilesX;
  uint32 maxTilesY;
  uint32 tileCoordX;
  uint32 tileCoordY;
  sint32 StartY;
  sint32 StopY;
  sint32 StartX;
  sint32 StopX;
  uint8  DeblockingOnFlag;

  DeblockingOnFlag = !(Sessiondata_p->CurrentSliceDisableDeblockingFlag);
  if(DeblockingOnFlag)
  {
    if(CurrPicSet->TilesEnabledFlag == 1)
    {
      // If there are tiles, deblock each tile
      maxTilesX = CurrPicSet->num_tile_columns_minus1+1;
      maxTilesY = CurrPicSet->num_tile_rows_minus1+1;

      for(tileCoordY = 0; tileCoordY < maxTilesY; tileCoordY++)
      {
        for(tileCoordX = 0; tileCoordX < maxTilesX; tileCoordX++)
        {
          // Get start and end CTU coordinates for the tile 
          StartX = (Sessiondata_p->colBd[tileCoordX])   << Log2CTUsizeInPU;
          StopX  = (Sessiondata_p->colBd[tileCoordX+1]) << Log2CTUsizeInPU;
          StartY = (Sessiondata_p->rowBd[tileCoordY])   << Log2CTUsizeInPU;
          StopY  = (Sessiondata_p->rowBd[tileCoordY+1]) << Log2CTUsizeInPU;
          Deblock_Tile(Sessiondata_p, StartX, StartY, StopX, StopY, 0);
        }
      }
      for(tileCoordY = 0; tileCoordY < maxTilesY; tileCoordY++)
      {
        for(tileCoordX = 0; tileCoordX < maxTilesX; tileCoordX++)
        {
          // Get start and end CTU coordinates for the tile 
          StartX = (Sessiondata_p->colBd[tileCoordX])   << Log2CTUsizeInPU;
          StopX  = (Sessiondata_p->colBd[tileCoordX+1]) << Log2CTUsizeInPU;
          StartY = (Sessiondata_p->rowBd[tileCoordY])   << Log2CTUsizeInPU;
          StopY  = (Sessiondata_p->rowBd[tileCoordY+1]) << Log2CTUsizeInPU;
          Deblock_Tile(Sessiondata_p, StartX, StartY, StopX, StopY, 1);
        }
      }
    }
    else
    {
      // If no tiles, deblock the entire picture
      StopY  = Sessiondata_p->CurrentSequenceParameterSet->PicHeightInMinUnit;
      StopX  = Sessiondata_p->CurrentSequenceParameterSet->PicWidthInMinUnit;
      Deblock_Tile(Sessiondata_p, 0, 0, StopX, StopY, 0);
      Deblock_Tile(Sessiondata_p, 0, 0, StopX, StopY, 1);
    }
  }
#endif
}