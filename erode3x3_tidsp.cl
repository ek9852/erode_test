R"=====(
// Original from ti-opencv Texas Instruments Inc 2016
// Modified by Keith Mok <ek9852@gmail.com>
// adding boundary condition handling without padding needed
//
#include <dsp_c.h>
#include <dsp_edmamgr.h>

// must be divisible by 8, and size including padding
#define MAX_LINE_SIZE 2048

// no padding needed, handle boundary condition correctly
// rows > 2
// cols must be divisible by 8 and < 2046
__attribute__((reqd_work_group_size(1,1,1))) __kernel void tidsp_morph_erode (__global const uchar * srcptr,
                    __global uchar * dstptr,
                    int cols, int rows)
{
  uchar * restrict dest_ptr;
  uchar * restrict yprev_ptr;
  uchar * restrict y_ptr;
  uchar * restrict ynext_ptr;
  int   rd_idx, start_rd_idx, fetch_rd_idx;
  int   id   = get_global_id(0);
  int   chunks = get_global_size(0);
  EdmaMgr_Handle evIN  = EdmaMgr_alloc(3);
  unsigned int srcPtr[3], dstPtr[3], numBytes[3];
  local uchar img_lines[4][MAX_LINE_SIZE]; //3 hor.lines needed for processing and one inflight via EDMA
  int clk_start, clk_end;
  int i, j;

  long r0_76543210, r1_76543210, r2_76543210, min8, min8_a, min8_b, min8_d1, min8_d2;
  unsigned int r0_98, r1_98, r2_98, min2;

  if (!evIN) { printf("Failed to alloc edmaIN handle.\n"); return; }

#ifdef TIDSP_OPENCL_VERBOSE
  clk_start = __clock();
#endif
  /* if not enough work for all cores, only first (chunks) cores compute */
  /* e.g. if we have 256 CU, but only got 128 rows */
  if (rows < chunks && id >= rows)  return;

  int mLocal    = rows < chunks ? 1 : rows / chunks;
  int offset    = mLocal * id;
  /* if rows > chunks and (chunks) does not divide (rows) evenly,
   * first (rows % chunks) cores get one extra row to compute */
  if (rows > chunks)
  {
      mLocal += (id < (rows % chunks) ? 1 : 0);
      offset += (id < (rows % chunks) ? id : (rows % chunks));
  }
  dest_ptr = (uchar *)dstptr;
  numBytes[0] = numBytes[1] = numBytes[2] = cols;

  srcPtr[0] = (unsigned int)(srcptr + (offset - 1) * cols);
  srcPtr[1] = (unsigned int)(srcptr + (offset    ) * cols);
  srcPtr[2] = (unsigned int)(srcptr + (offset + 1) * cols);
  dstPtr[0] = (unsigned int)&img_lines[0][1];
  dstPtr[1] = (unsigned int)&img_lines[1][1];
  dstPtr[2] = (unsigned int)&img_lines[2][1];

  if(offset == 0)
  { /* Top boundary case of image */
    srcPtr[0] = srcPtr[1];
    fetch_rd_idx = 2 * cols;
  } else if(offset  == (rows-1))
  { /* Bottom boundary of image */
    srcPtr[2] = srcPtr[1];
    fetch_rd_idx = (rows - 1) * cols;
    dest_ptr += offset * cols;
  } else
  { /* middle of image */
    fetch_rd_idx = (offset + 2) * cols;
    dest_ptr += offset * cols;
  }
  EdmaMgr_copy1D1DLinked(evIN, srcPtr, dstPtr, numBytes, 3);
  // boundary condition without need of padding
  img_lines[0][0] = 255;
  img_lines[0][cols+1] = 255;
  img_lines[1][0] = 255;
  img_lines[1][cols+1] = 255;
  img_lines[2][0] = 255;
  img_lines[2][cols+1] = 255;
  img_lines[3][0] = 255;
  img_lines[3][cols+1] = 255;
  start_rd_idx = 0;

  for (int y = 0; y < mLocal; y ++)
  {
    EdmaMgr_wait(evIN);
    rd_idx  = start_rd_idx;
    yprev_ptr = (uchar *)img_lines[rd_idx];
    rd_idx = (rd_idx + 1) & 3;
    start_rd_idx = rd_idx;
    y_ptr     = (uchar *)img_lines[rd_idx];
    rd_idx = (rd_idx + 1) & 3;
    ynext_ptr = (uchar *)img_lines[rd_idx];
    rd_idx = (rd_idx + 1) & 3;
    EdmaMgr_copyFast(evIN, (void*)(srcptr + fetch_rd_idx), (void*)&img_lines[rd_idx][1]);
    if ((offset+y+3) < rows) /* handle boundary case to just fetch previous row */
      fetch_rd_idx += cols;
#pragma unroll 2
    for (i = 0; i < cols; i += 8) {
       /* Read 10 bytes from each of the 3 lines to produce 8 outputs. */
       r0_76543210 = _amem8_const(&yprev_ptr[i]);
       r1_76543210 = _amem8_const(&y_ptr[i]);
       r2_76543210 = _amem8_const(&ynext_ptr[i]);
       r0_98 = _amem2_const(&yprev_ptr[i + 8]);
       r1_98 = _amem2_const(&y_ptr[i + 8]);
       r2_98 = _amem2_const(&ynext_ptr[i + 8]);

       /* Find min of each column */
       min8 = _dminu4(r0_76543210, r1_76543210);
       min8 = _dminu4(min8, r2_76543210);
       min2 = _minu4(r0_98, r1_98);
       min2 = _minu4(min2, r2_98);

       /* Shift and find min of result twice */
       /*    7 6 5 4 3 2 1 0 = min8
        *    8 7 6 5 4 3 2 1 = min8_d1
        *    9 8 7 6 5 4 3 2 = min8_d2
        */

       /* Create shifted copies of min8, and column-wise find the min */
       min8_d1 = _itoll(_shrmb(min2, _hill(min8)), _shrmb(_hill(min8), _loll(min8)));
       min8_d2 = _itoll(_packlh2(min2, _hill(min8)), _packlh2(_hill(min8), _loll(min8)));

       min8_a = _dminu4(min8, min8_d1);
       min8_b = _dminu4(min8_a, min8_d2);

       /* store 8 min values */
       _amem8(&dest_ptr[i]) = min8_b;
    }
    dest_ptr += cols;
  }
  EdmaMgr_wait(evIN);
  EdmaMgr_free(evIN);

#ifdef TIDSP_OPENCL_VERBOSE
  clk_end = __clock();
  printf ("TIDSP erode clockdiff=%d\n", clk_end - clk_start);
#endif
}
)====="
