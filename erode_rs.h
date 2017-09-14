#ifndef _ERODE_RS_H_
#define _ERODE_RS_H_

int erode3x3_rs_init(int w, int h);
void erode3x3_rs(uint8_t *in_data, uint8_t *out_data, int w, int h);
void erode3x3_rs_destroy();

#endif


