#ifndef _ERODE_NEON_H_
#define _ERODE_NEON_H_

#include <stdint.h>

void erode3x3_neon(uint8_t *in_data, uint8_t *out_data, int w, int h);

#endif

