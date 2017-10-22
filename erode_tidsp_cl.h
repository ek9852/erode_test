#ifndef _ERODE_TIDSP_CL_H_
#define _ERODE_TIDSP_CL_H_

int erode3x3_tidsp_cl_init(int w, int h);
void erode3x3_tidsp_cl();
uint8_t* erode3x3_tidsp_cl_map_input_buf();
uint8_t* erode3x3_tidsp_cl_map_output_buf();
void erode3x3_tidsp_cl_unmap_input_buf(uint8_t *srcA);
void erode3x3_tidsp_cl_unmap_output_buf(uint8_t *srcB);
void erode3x3_tidsp_cl_destroy();

#endif

