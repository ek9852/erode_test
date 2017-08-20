#ifndef _ERODE_SSE_H_
#define _ERODE_SSE_H_

#include <stdint.h>

void erode3x3_sse(uint8_t *in_data, uint8_t *out_data, int w, int h);

#endif


