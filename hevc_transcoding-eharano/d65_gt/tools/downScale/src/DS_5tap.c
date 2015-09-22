

#ifdef _WIN32

#define fseek64   _fseeki64
#define ftell64   _ftelli64

#define SIMD               1
#define ALIGNED(x) __declspec(align(x))

#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))

#define fseek64   fseek
#define ftell64   ftell

#if defined(__SSE4_1__)
#define SIMD               1
#define ALIGNED(x) __attribute__((aligned(x)))
#else
#define SIMD               0
#endif

#else

#define fseek64   fseek
#define ftell64   ftell
#define SIMD      0

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if SIMD
#include <emmintrin.h>
#include <smmintrin.h>
#endif

void PadImage(unsigned char *src,
              unsigned char *dst,
              int height,
              int width,
              int pad)
{
  int j;
  int widthPad  = width  + 2 * pad;

  unsigned char *BufSrc, *BufDst;

  for (j = 0; j < pad; j++)
  {
    BufDst = dst + j * widthPad;
    BufSrc = src;

    memset(BufDst, BufSrc[0], pad * sizeof(unsigned char));
    BufDst += pad;

    memcpy(BufDst, BufSrc, width * sizeof(unsigned char));
    BufDst += width;

    memset(BufDst, BufSrc[width - 1], pad * sizeof(unsigned char));
  }

  for (j = 0; j < height; j++)
  {
    BufDst = dst + j * widthPad + 2 * widthPad;
    BufSrc = src + j * width;

    memset(BufDst, BufSrc[0], pad * sizeof(unsigned char));
    BufDst += pad;

    memcpy(BufDst, BufSrc, width * sizeof(unsigned char));
    BufDst += width;

    memset(BufDst, BufSrc[width - 1], pad * sizeof(unsigned char));
  }

  for (j = 0; j < pad; j++)
  {
    BufDst = dst + height * widthPad + 2 * widthPad;
    BufSrc = src + (height - 1) * width;

    memset(BufDst, BufSrc[0], pad * sizeof(unsigned char));
    BufDst += pad;

    memcpy(BufDst, BufSrc, width * sizeof(unsigned char));
    BufDst += width;

    memset(BufDst, BufSrc[width - 1], pad * sizeof(unsigned char));
  }
}

typedef void DownScalImageFunc(unsigned char *src,
                               unsigned char *dst,
                               int *temp,
                               int height1,
                               int width1,
                               int plane);

void DownScaleImage2x(unsigned char *src,
                      unsigned char *dst,
                      int *temp,
                      int height1,
                      int width1,
                      int plane)
{
  // 5 tap version for 2x
  static const int phase_filter_0[4][5] =
  {
    { -4, 39, 58, 39, -4},
    { -9, 31, 57, 47,  2},
    {-11, 22, 53, 53, 11},
    {-11, 13, 47, 57, 12}
  };

  int shift   = 14;
  int round   = 1 << (shift - 1);
  int height2 = height1 >> 1;
  int width2  = width1  >> 1;

  int j1, i1;
  int i0, div_i0;
  int j0, div_j0;

  int iSum;
  int k;

  int *p_tmp;

  unsigned char *p_src;
  unsigned char *p_dst;

#if SIMD

  __m128i x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, c1, c2, c3, c4, c5, c6, c7, c8;
  __m128i y1, y2, y3, y4, y5, y6, y7, y8;
  __m128i z1, z2, z3, z4, z5, z6, z7, z8;
  __m128i r1, r2;
  __m128i rd   = _mm_set1_epi32(round);

  int pos;

const static short ALIGNED(16) ds_x2_h_sse[3][8] = 
{
    { -4, 39, -4, 39, -4, 39, -4, 39 },
    { 58, 39, 58, 39, 58, 39, 58, 39 },
    { -4,  0, -4,  0, -4,  0, -4,  0 }
};

const static int ALIGNED(32) ds_x2_v_sse_luma[5][4] = 
{
    { -4, -4, -4, -4 },
    { 39, 39, 39, 39 },
    { 58, 58, 58, 58 },
    { 39, 39, 39, 39 },
    { -4, -4, -4, -4 }
};

const static int ALIGNED(32) ds_x2_v_sse_chroma[5][4] = 
{
    { -9, -9, -9, -9 },
    { 31, 31, 31, 31 },
    { 57, 57, 57, 57 },
    { 47, 47, 47, 47 },
    {  2,  2,  2,  2 }
};

  c1 = _mm_load_si128((__m128i *) ds_x2_h_sse[0]);
  c2 = _mm_load_si128((__m128i *) ds_x2_h_sse[1]);
  c3 = _mm_load_si128((__m128i *) ds_x2_h_sse[2]);

  c4 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x2_v_sse_luma[0]) : _mm_load_si128((__m128i *) ds_x2_v_sse_chroma[0]);
  c5 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x2_v_sse_luma[1]) : _mm_load_si128((__m128i *) ds_x2_v_sse_chroma[1]);
  c6 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x2_v_sse_luma[2]) : _mm_load_si128((__m128i *) ds_x2_v_sse_chroma[2]);
  c7 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x2_v_sse_luma[3]) : _mm_load_si128((__m128i *) ds_x2_v_sse_chroma[3]);
  c8 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x2_v_sse_luma[4]) : _mm_load_si128((__m128i *) ds_x2_v_sse_chroma[4]);

#endif

  // Horizontal Filtering
#if SIMD

  for (j1 = 0; j1 < (height1 + 4); j1++)
  {
    //i0    = -8;
    p_tmp = temp + j1 * width2;

    for (i1 = 0; i1 < width2; i1 += 8)
    {
      pos = j1 * (width1 + 4) + (i1 * 2);

      x1 = _mm_loadu_si128((__m128i *) &src[pos     ]);
      x2 = _mm_loadu_si128((__m128i *) &src[pos +  2]);
      x3 = _mm_loadu_si128((__m128i *) &src[pos +  4]);
      x4 = _mm_loadu_si128((__m128i *) &src[pos +  6]);
      x5 = _mm_loadu_si128((__m128i *) &src[pos +  8]);
      x6 = _mm_loadu_si128((__m128i *) &src[pos + 10]);
      x7 = _mm_loadu_si128((__m128i *) &src[pos + 12]);
      x8 = _mm_loadu_si128((__m128i *) &src[pos + 14]);

      x1 = _mm_cvtepu8_epi16(x1);
      x2 = _mm_cvtepu8_epi16(x2);
      x3 = _mm_cvtepu8_epi16(x3);
      x4 = _mm_cvtepu8_epi16(x4);
      x5 = _mm_cvtepu8_epi16(x5);
      x6 = _mm_cvtepu8_epi16(x6);
      x7 = _mm_cvtepu8_epi16(x7);
      x8 = _mm_cvtepu8_epi16(x8);

      y1 = _mm_unpacklo_epi32(x1, x2);
      y2 = _mm_unpackhi_epi32(x1, x2);
      y3 = _mm_unpacklo_epi32(x3, x4);
      y4 = _mm_unpackhi_epi32(x3, x4);
      y5 = _mm_unpacklo_epi32(x5, x6);
      y6 = _mm_unpackhi_epi32(x5, x6);
      y7 = _mm_unpacklo_epi32(x7, x8);
      y8 = _mm_unpackhi_epi32(x7, x8);

      z1 = _mm_unpacklo_epi64(y1, y3);
      z2 = _mm_unpackhi_epi64(y1, y3);
      z3 = _mm_unpacklo_epi64(y2, y4);
      z4 = _mm_unpackhi_epi64(y2, y4); // check
      z5 = _mm_unpacklo_epi64(y5, y7);
      z6 = _mm_unpackhi_epi64(y5, y7);
      z7 = _mm_unpacklo_epi64(y6, y8);
      z8 = _mm_unpackhi_epi64(y6, y8); // check

      z1 = _mm_madd_epi16(z1, c1);
      z2 = _mm_madd_epi16(z2, c2);
      z3 = _mm_madd_epi16(z3, c3);

      r1 = _mm_add_epi32(z1 , _mm_add_epi32(z2, z3));

      z5 = _mm_madd_epi16(z5, c1);
      z6 = _mm_madd_epi16(z6, c2);
      z7 = _mm_madd_epi16(z7, c3);

      r2 = _mm_add_epi32(z5 , _mm_add_epi32(z6, z7));
      _mm_store_si128((__m128i *)(p_tmp + i1), r1);
      _mm_store_si128((__m128i *)(p_tmp + i1 + 4), r2);
    }
  }
#else
  for (j1 = 0; j1 < (height1 + 4); j1++)
  {
    i0    = -8;
    p_tmp = temp + j1 * width2;

    for (i1 = 0; i1 < width2; i1++)
    {
      i0    += 8;
      div_i0 = i0 / 4;
      p_src  = src + j1 * (width1 + 4) + div_i0;
      iSum   = 0;
      for (k = 0; k < 5; k++)
      {
        iSum += (*p_src++) * phase_filter_0[i0 - div_i0 * 4][k];
      }
      *p_tmp++ = iSum;
    }
  }
#endif

#if SIMD

  for (j1 = 0; j1 < height2; j1++)
  {
    p_dst = dst + j1 * width2;

    for (i1 = 0; i1 < width2; i1 += 8)
    {
      pos = (j1 * 2) * width2 + i1;

      x1  = _mm_loadu_si128((__m128i *) &temp[pos                 ]);
      x2  = _mm_loadu_si128((__m128i *) &temp[pos +              4]);
      x3  = _mm_loadu_si128((__m128i *) &temp[pos +     width2    ]);
      x4  = _mm_loadu_si128((__m128i *) &temp[pos +     width2 + 4]);
      x5  = _mm_loadu_si128((__m128i *) &temp[pos + 2 * width2    ]);
      x6  = _mm_loadu_si128((__m128i *) &temp[pos + 2 * width2 + 4]);
      x7  = _mm_loadu_si128((__m128i *) &temp[pos + 3 * width2    ]);
      x8  = _mm_loadu_si128((__m128i *) &temp[pos + 3 * width2 + 4]);
      x9  = _mm_loadu_si128((__m128i *) &temp[pos + 4 * width2    ]);
      x10 = _mm_loadu_si128((__m128i *) &temp[pos + 4 * width2 + 4]);

      x1  = _mm_mullo_epi32(x1, c4);
      x2  = _mm_mullo_epi32(x2, c4);
      x3  = _mm_mullo_epi32(x3, c5);
      x4  = _mm_mullo_epi32(x4, c5);
      x5  = _mm_mullo_epi32(x5, c6);
      x6  = _mm_mullo_epi32(x6, c6);
      x7  = _mm_mullo_epi32(x7, c7);
      x8  = _mm_mullo_epi32(x8, c7);
      x9  = _mm_mullo_epi32(x9,  c8);
      x10 = _mm_mullo_epi32(x10, c8);

      y1  = _mm_add_epi32(x1, x3);
      y1  = _mm_add_epi32(y1, x5);
      y1  = _mm_add_epi32(y1, x7);
      y1  = _mm_add_epi32(y1, x9);

      y2  = _mm_add_epi32(x2,  x4);
      y2  = _mm_add_epi32(y2,  x6);
      y2  = _mm_add_epi32(y2,  x8);
      y2  = _mm_add_epi32(y2, x10);


      y1  = _mm_add_epi32(y1, rd);
      y2  = _mm_add_epi32(y2, rd);

      y1  = _mm_srai_epi32(y1, shift);
      y2  = _mm_srai_epi32(y2, shift);

      z1  = _mm_packs_epi32(y1, y1);
      z2  = _mm_packs_epi32(y2, y2);

      r1  = _mm_packus_epi16(z1, z1);
      r2  = _mm_packus_epi16(z2, z2);
      r1  = _mm_unpacklo_epi32(r1, r2);
      _mm_storel_epi64((__m128i*)(p_dst + i1), r1);
    }
  }

#else
  // Vertical Filtering
  j0 = (plane == 0) ? -8 : -7;

  for (j1 = 0; j1 < height2; j1++)
  {
    j0    += 8;
    div_j0 = j0 / 4;
    p_dst  = dst + j1 * width2;
    p_tmp  = temp + div_j0 * width2;

    for (i1 = 0; i1 < width2; i1++)
    {
      iSum = 0;
      for (k = 0; k < 5; k++)
      {
        iSum += p_tmp[k * width2 + i1] * phase_filter_0[j0 - div_j0 * 4][k];
      }
      iSum = (iSum + round) >> shift;
      *p_dst++ = (unsigned char)(iSum > 255 ? 255 : iSum < 0 ? 0 : iSum);
    }
  }
#endif
}

void DownScaleImage1_5x(unsigned char *src,
                        unsigned char *dst,
                        int *temp,
                        int height1,
                        int width1,
                        int plane)
{
  // 5 tap version for 1.5x
  static const int phase_filter_1[8][5] = 
  {
    {-10, 36, 76, 36,-10},
    {-12, 29, 74, 44, -7},
    {-13, 21, 72, 51, -3},
    {-13, 14, 68, 57,  2},
    {-13,  8, 63, 63,  7},
    {-12,  2, 57, 68, 13},
    {-10, -3, 50, 70, 21},
    { -8, -7, 43, 72, 28},
  };

  int shift   = 14;
  int round   = 1 << (shift - 1);
  int height2 = (height1 * 8) / 12;
  int width2  = (width1  * 8) / 12;

  int j1, i1;
  int i0, div_i0;
  int j0, div_j0;

  int iSum;
  int k;

  int *p_tmp;

  unsigned char *p_src;
  unsigned char *p_dst;

#if SIMD

  __m128i x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
  __m128i c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13;
  __m128i y1, y2, y3, y4, y5, y6, y7, y8;
  __m128i z1, z2, z3, z4, z5, z6, z7, z8;
  __m128i r1, r2;
  __m128i rd   = _mm_set1_epi32(round);

  int pos;

const static short ALIGNED(16) ds_x1_5_h_sse[3][8] = 
{
    {-10, 36,-13,  8,-10, 36,-13,  8},
    { 76, 36, 63, 63, 76, 36, 63, 63},
    {-10,  0,  7,  0,-10,  0,  7,  0}
};

const static int ALIGNED(32) ds_x1_5_v0_sse_luma[5][4] = 
{
    {-10,-10,-10,-10},
    { 36, 36, 36, 36},
    { 76, 76, 76, 76},
    { 36, 36, 36, 36},
    {-10,-10,-10,-10}
};

const static int ALIGNED(32) ds_x1_5_v1_sse_luma[5][4] = 
{
    {-13,-13,-13,-13},
    {  8,  8,  8,  8},
    { 63, 63, 63, 63},
    { 63, 63, 63, 63},
    {  7,  7,  7,  7}
};

const static int ALIGNED(32) ds_x1_5_v0_sse_chroma[5][4] = 
{
    {-12,-12,-12,-12},
    { 29, 29, 29, 29},
    { 74, 74, 74, 74},
    { 44, 44, 44, 44},
    { -7, -7, -7, -7}
};

const static int ALIGNED(32) ds_x1_5_v1_sse_chroma[5][4] = 
{
    {-12,-12,-12,-12},
    {  2,  2,  2,  2},
    { 57, 57, 57, 57},
    { 68, 68, 68, 68},
    { 13, 13, 13, 13}
};

  c1  = _mm_load_si128((__m128i *) ds_x1_5_h_sse[0]);
  c2  = _mm_load_si128((__m128i *) ds_x1_5_h_sse[1]);
  c3  = _mm_load_si128((__m128i *) ds_x1_5_h_sse[2]);

  c4  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v0_sse_luma[0]) : _mm_load_si128((__m128i *) ds_x1_5_v0_sse_chroma[0]);
  c5  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v0_sse_luma[1]) : _mm_load_si128((__m128i *) ds_x1_5_v0_sse_chroma[1]);
  c6  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v0_sse_luma[2]) : _mm_load_si128((__m128i *) ds_x1_5_v0_sse_chroma[2]);
  c7  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v0_sse_luma[3]) : _mm_load_si128((__m128i *) ds_x1_5_v0_sse_chroma[3]);
  c8  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v0_sse_luma[4]) : _mm_load_si128((__m128i *) ds_x1_5_v0_sse_chroma[4]);

  c9  = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v1_sse_luma[0]) : _mm_load_si128((__m128i *) ds_x1_5_v1_sse_chroma[0]);
  c10 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v1_sse_luma[1]) : _mm_load_si128((__m128i *) ds_x1_5_v1_sse_chroma[1]);
  c11 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v1_sse_luma[2]) : _mm_load_si128((__m128i *) ds_x1_5_v1_sse_chroma[2]);
  c12 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v1_sse_luma[3]) : _mm_load_si128((__m128i *) ds_x1_5_v1_sse_chroma[3]);
  c13 = (plane == 0) ? _mm_load_si128((__m128i *) ds_x1_5_v1_sse_luma[4]) : _mm_load_si128((__m128i *) ds_x1_5_v1_sse_chroma[4]);

#endif

  // Horizontal Filtering

#if SIMD

  for (j1 = 0; j1 < (height1 + 4); j1++)
  {
    i0    = -12;
    p_tmp = temp + j1 * width2;

    for (i1 = 0; i1 < width2; i1 += 8)
    {
      pos = j1 * (width1 + 4) + (i1 * 12 / 8);

      x1 = _mm_loadu_si128((__m128i *) &src[pos     ]);
      x2 = _mm_loadu_si128((__m128i *) &src[pos +  1]);
      x3 = _mm_loadu_si128((__m128i *) &src[pos +  3]);
      x4 = _mm_loadu_si128((__m128i *) &src[pos +  4]);
      x5 = _mm_loadu_si128((__m128i *) &src[pos +  6]);
      x6 = _mm_loadu_si128((__m128i *) &src[pos +  7]);
      x7 = _mm_loadu_si128((__m128i *) &src[pos +  9]);
      x8 = _mm_loadu_si128((__m128i *) &src[pos + 10]);

      x1 = _mm_cvtepu8_epi16(x1);
      x2 = _mm_cvtepu8_epi16(x2);
      x3 = _mm_cvtepu8_epi16(x3);
      x4 = _mm_cvtepu8_epi16(x4);
      x5 = _mm_cvtepu8_epi16(x5);
      x6 = _mm_cvtepu8_epi16(x6);
      x7 = _mm_cvtepu8_epi16(x7);
      x8 = _mm_cvtepu8_epi16(x8);

      y1 = _mm_unpacklo_epi32(x1, x2);
      y2 = _mm_unpackhi_epi32(x1, x2);
      y3 = _mm_unpacklo_epi32(x3, x4);
      y4 = _mm_unpackhi_epi32(x3, x4);
      y5 = _mm_unpacklo_epi32(x5, x6);
      y6 = _mm_unpackhi_epi32(x5, x6);
      y7 = _mm_unpacklo_epi32(x7, x8);
      y8 = _mm_unpackhi_epi32(x7, x8);

      z1 = _mm_unpacklo_epi64(y1, y3);
      z2 = _mm_unpackhi_epi64(y1, y3);
      z3 = _mm_unpacklo_epi64(y2, y4);
      z4 = _mm_unpackhi_epi64(y2, y4); // check
      z5 = _mm_unpacklo_epi64(y5, y7);
      z6 = _mm_unpackhi_epi64(y5, y7);
      z7 = _mm_unpacklo_epi64(y6, y8);
      z8 = _mm_unpackhi_epi64(y6, y8); // check

      z1 = _mm_madd_epi16(z1, c1);
      z2 = _mm_madd_epi16(z2, c2);
      z3 = _mm_madd_epi16(z3, c3);

      r1 = _mm_add_epi32(z1 , _mm_add_epi32(z2, z3));

      z5 = _mm_madd_epi16(z5, c1);
      z6 = _mm_madd_epi16(z6, c2);
      z7 = _mm_madd_epi16(z7, c3);

      r2 = _mm_add_epi32(z5 , _mm_add_epi32(z6, z7));
      _mm_store_si128((__m128i *)(p_tmp + i1), r1);
      _mm_store_si128((__m128i *)(p_tmp + i1 + 4), r2);
    }
  }
#else
  for (j1 = 0; j1 < (height1 + 4); j1++)
  {
    i0    = -12;
    p_tmp = temp + j1 * width2;

    for (i1 = 0; i1 < width2; i1++)
    {
      i0    += 12;
      div_i0 = i0 / 8;
      p_src  = src + j1 * (width1 + 4) + div_i0;
      iSum   = 0;
      for (k = 0; k < 5; k++)
      {
        iSum += (*p_src++) * phase_filter_1[i0 - div_i0 * 8][k];
      }
      *p_tmp++ = iSum;
    }
  }
#endif

  // Vertical Filtering

#if SIMD

  for (j1 = 0; j1 < height2; j1++)
  {
    p_dst = dst + j1 * width2;

    for (i1 = 0; i1 < width2; i1 += 8)
    {
      pos   = (j1 * 12 / 8) * width2 + i1;

      x1  = _mm_loadu_si128((__m128i *) &temp[pos                 ]);
      x2  = _mm_loadu_si128((__m128i *) &temp[pos +              4]);
      x3  = _mm_loadu_si128((__m128i *) &temp[pos +     width2    ]);
      x4  = _mm_loadu_si128((__m128i *) &temp[pos +     width2 + 4]);
      x5  = _mm_loadu_si128((__m128i *) &temp[pos + 2 * width2    ]);
      x6  = _mm_loadu_si128((__m128i *) &temp[pos + 2 * width2 + 4]);
      x7  = _mm_loadu_si128((__m128i *) &temp[pos + 3 * width2    ]);
      x8  = _mm_loadu_si128((__m128i *) &temp[pos + 3 * width2 + 4]);
      x9  = _mm_loadu_si128((__m128i *) &temp[pos + 4 * width2    ]);
      x10 = _mm_loadu_si128((__m128i *) &temp[pos + 4 * width2 + 4]);

      if ((j1 % 2) == 0)
      {
        x1  = _mm_mullo_epi32(x1,  c4);
        x2  = _mm_mullo_epi32(x2,  c4);
        x3  = _mm_mullo_epi32(x3,  c5);
        x4  = _mm_mullo_epi32(x4,  c5);
        x5  = _mm_mullo_epi32(x5,  c6);
        x6  = _mm_mullo_epi32(x6,  c6);
        x7  = _mm_mullo_epi32(x7,  c7);
        x8  = _mm_mullo_epi32(x8,  c7);
        x9  = _mm_mullo_epi32(x9,  c8);
        x10 = _mm_mullo_epi32(x10, c8);
      }
      else
      {
        x1  = _mm_mullo_epi32(x1,  c9);
        x2  = _mm_mullo_epi32(x2,  c9);
        x3  = _mm_mullo_epi32(x3, c10);
        x4  = _mm_mullo_epi32(x4, c10);
        x5  = _mm_mullo_epi32(x5, c11);
        x6  = _mm_mullo_epi32(x6, c11);
        x7  = _mm_mullo_epi32(x7, c12);
        x8  = _mm_mullo_epi32(x8, c12);
        x9  = _mm_mullo_epi32(x9, c13);
        x10 = _mm_mullo_epi32(x10,c13);
      }

      y1  = _mm_add_epi32(x1, x3);
      y1  = _mm_add_epi32(y1, x5);
      y1  = _mm_add_epi32(y1, x7);
      y1  = _mm_add_epi32(y1, x9);

      y2  = _mm_add_epi32(x2,  x4);
      y2  = _mm_add_epi32(y2,  x6);
      y2  = _mm_add_epi32(y2,  x8);
      y2  = _mm_add_epi32(y2, x10);


      y1  = _mm_add_epi32(y1, rd);
      y2  = _mm_add_epi32(y2, rd);

      y1  = _mm_srai_epi32(y1, shift);
      y2  = _mm_srai_epi32(y2, shift);

      z1  = _mm_packs_epi32(y1, y1);
      z2  = _mm_packs_epi32(y2, y2);

      r1  = _mm_packus_epi16(z1, z1);
      r2  = _mm_packus_epi16(z2, z2);
      r1  = _mm_unpacklo_epi32(r1, r2);
      _mm_storel_epi64((__m128i*)(p_dst + i1), r1);
    }
  }
#else
  j0 = (plane == 0) ? -12 : -11;

  for (j1 = 0; j1 < height2; j1++)
  {
    j0    += 12;
    div_j0 = j0 / 8;
    p_dst  = dst + j1 * width2;
    p_tmp  = temp + div_j0 * width2;

    for (i1 = 0; i1 < width2; i1++)
    {
      iSum = 0;
      for (k = 0; k < 5; k++)
      {
        iSum += p_tmp[k * width2 + i1] * phase_filter_1[j0 - div_j0 * 8][k];
      }
      iSum = (iSum + round) >> shift;
      *p_dst++ = (unsigned char)(iSum > 255 ? 255 : iSum < 0 ? 0 : iSum);
    }
  }
#endif
}

static DownScalImageFunc* const DownScaleImage[2] = { DownScaleImage2x, DownScaleImage1_5x };

int main(int argc, char *argv[])
{

  int width_org,  width_org_c,  width_scaled,  width_scaled_c;
  int height_org, height_org_c, height_scaled, height_scaled_c;
  int size_org,   size_org_c,   size_scaled,   size_scaled_c;
  int heigh_scaled_div8;
  int size_scaled_div8, size_scaled_div8_c;
  int size_output, size_output_c;

  unsigned char *Y1, *U1, *V1;
  unsigned char *Y2, *U2, *V2;
  unsigned char *temp_luma, *temp_chroma;
  int *tempY, *tempU, *tempV;

  FILE * infile;
  FILE * outfile;

  int i;
  int ratio;
  int Frames       = 10000;
  int DsHeightDiv8 = 0;
  int TotalFrames  = 0;
  int StartFrame   = 0;
  int FrameSkip    = 0;

  if (argc < 5)
  {
    printf("\nIncorrect number of arguments!!!\n\n");
    printf("Syntax: \n");
    printf("%s <width> <height> <input> <output> <method> [downscaled_video_to_have_height_dividable_by_8] [frames_to_process] [skip_frames] [start_frame]\n\n", argv[0]);
    printf("Examples: \n");
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv 0 \n", argv[0]);
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv 0 100 0 0\n", argv[0]);
    printf("%s 1920 1080 input_1920x1080_24p.yuv output_960x540_24p.yuv 0 100 0 10\n\n", argv[0]);
    return -1;
  }

  width_org  = atoi  (argv[1]);
  height_org = atoi  (argv[2]);
  infile     = fopen (argv[3], "rb");
  outfile    = fopen (argv[4], "wb");
  ratio      = atoi  (argv[5]); 

  for (i = 6; i < argc; i++)
  {
    switch(i)
    {
    case 6:
      DsHeightDiv8 = atoi(argv[6]);
      break;
    case 7:
      Frames = atoi(argv[7]);
      break;
    case 8:
      FrameSkip = atoi(argv[8]);
      break;
    case 9:
      StartFrame = atoi(argv[9]);
      break;
    default:
      printf("Too many input arguments");
      break;
    }
  }

  if (width_org < 4 || height_org < 4 || infile == NULL || outfile == NULL ||
      Frames < 1 || FrameSkip < 0 || StartFrame < 0 || ratio > 1 || ratio < 0 ||
      DsHeightDiv8 > 1 || DsHeightDiv8 < 0)
  {
    printf("input error\n");
    return -1;
  }

  fseek64 (infile, 0, SEEK_END);

  TotalFrames = (int) ((ftell64(infile) * 2) / (width_org * height_org * 3));

  if (StartFrame >= TotalFrames)
  {
    printf("StartFrame value too large (%d)\n", StartFrame);
    return -1;
  }

  TotalFrames -= StartFrame;
  TotalFrames  = ((Frames - 1) * (1 + FrameSkip) + 1) > TotalFrames ? TotalFrames : ((Frames - 1) * (1 + FrameSkip) + 1);

  fseek64(infile, 0, SEEK_SET);

  size_org         = height_org   * width_org;

  width_org_c      = width_org  >> 1;
  height_org_c     = height_org >> 1;
  size_org_c       = size_org   >> 2;

  width_scaled     = (ratio == 0) ? (width_org  / 2) : (width_org  * 2 / 3);
  height_scaled    = (ratio == 0) ? (height_org / 2) : (height_org * 2 / 3);
  size_scaled      = width_scaled * height_scaled;

  width_scaled_c  = width_scaled  >> 1;
  height_scaled_c = height_scaled >> 1;
  size_scaled_c   = size_scaled   >> 2;

  heigh_scaled_div8  = DsHeightDiv8 ? ((int)(height_scaled / 8)) * 8 : height_scaled;
  size_scaled_div8   = width_scaled * heigh_scaled_div8;
  size_scaled_div8_c = size_scaled_div8 >> 2;

  size_output   = DsHeightDiv8 ? size_scaled_div8   : size_scaled;
  size_output_c = DsHeightDiv8 ? size_scaled_div8_c : size_scaled_c;

  printf("\n=============================================================\n");
  printf("\n Input  = %s", argv[3]);
  printf("\n Output = %s\n", argv[4]);
  printf("\n Rescaling input from (%d,%d) to (%d,%d) resolution\n", width_org, height_org, width_scaled, heigh_scaled_div8);
  printf("\n=============================================================\n\n");


  // Memory allocation for Source YUV

  Y1 = (unsigned char*)malloc((height_org   + 4) * (width_org   + 4) * sizeof(unsigned char));
  U1 = (unsigned char*)malloc((height_org_c + 4) * (width_org_c + 4) * sizeof(unsigned char));
  V1 = (unsigned char*)malloc((height_org_c + 4) * (width_org_c + 4) * sizeof(unsigned char));

  tempY = (int*)calloc((height_org   + 4) * width_org   , sizeof(int));
  tempU = (int*)calloc((height_org_c + 4) * width_org_c , sizeof(int));
  tempV = (int*)calloc((height_org_c + 4) * width_org_c , sizeof(int));

  Y2 = (unsigned char*)malloc(size_scaled   * sizeof(unsigned char));
  U2 = (unsigned char*)malloc(size_scaled_c * sizeof(unsigned char));
  V2 = (unsigned char*)malloc(size_scaled_c * sizeof(unsigned char));

  temp_luma   = (unsigned char*)malloc(size_org   * sizeof(unsigned char));
  temp_chroma = (unsigned char*)malloc(size_org_c * sizeof(unsigned char));

  if (StartFrame != 0)
  {
    fseek64(infile, (size_org * StartFrame * 3) >> 1, SEEK_SET);
  }

  for (i = 0; i < TotalFrames; i++)
  {
    //fprintf(stdout,"Rescaling frame %d\r", i);
    //fflush(stdout);

    // read and pad Y
    fread(temp_luma, sizeof(unsigned char), size_org, infile);
    PadImage(temp_luma, Y1, height_org, width_org, 2);

    // read and pad U
    fread(temp_chroma, sizeof(unsigned char), size_org_c, infile);
    PadImage(temp_chroma, U1, height_org_c, width_org_c, 2);

    // read and pad V
    fread(temp_chroma, sizeof(unsigned char), size_org_c, infile);
    PadImage(temp_chroma, V1, height_org_c, width_org_c, 2);

    // generate scaled YUV
    DownScaleImage[ratio](Y1, Y2, tempY, height_org,   width_org,   0);
    DownScaleImage[ratio](U1, U2, tempU, height_org_c, width_org_c, 1);
    DownScaleImage[ratio](V1, V2, tempV, height_org_c, width_org_c, 2);

    // write scaled YUV
    fwrite(Y2, sizeof(unsigned char), size_output,   outfile);
    fwrite(U2, sizeof(unsigned char), size_output_c, outfile);
    fwrite(V2, sizeof(unsigned char), size_output_c, outfile);

    if (FrameSkip != 0)
    {
      if ((i + FrameSkip) < TotalFrames)
      {
        fseek64(infile, (size_org * FrameSkip * 3) / 2, SEEK_CUR);
        i += FrameSkip;
      }
      else
      {
        break;
      }
    }
  }

  //printf("\nEnd of rescaling process.\n");

  free(Y1);
  free(U1);
  free(V1);

  free(Y2);
  free(U2);
  free(V2);

  free(tempY);
  free(tempU);
  free(tempV);

  free(temp_luma);
  free(temp_chroma);

  fclose(infile);
  fclose(outfile);

  return 0;
}
