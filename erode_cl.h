#ifndef _ERODE_CL_H_
#define _ERODE_CL_H_

int erode3x3_cl_init(int w, int h, bool use_host_ptr);
void erode3x3_cl(uint8_t *in_data, uint8_t *out_data, int w, int h);
void erode3x3_cl_destroy();

#endif

