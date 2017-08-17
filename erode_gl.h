#ifndef _ERODE_GL_H_
#define _ERODE_GL_H_

int erode3x3_gl_init(int w, int h);
void erode3x3_gl(uint8_t *in_data, uint8_t *out_data, int w, int h);
void erode3x3_gl_destroy();

#endif


