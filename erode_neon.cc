#if defined(__ARM_NEON__) || defined(__aarch64__)

#include "erode_neon.h"
#include <arm_neon.h>

inline void
erode3x3_neon_kernel(uint8_t *in_data, uint8_t *out_data, const int in_stride)
{
        uint8_t         *in_ptr   = in_data - 1;
        const uint8x16_t top_data = vld1q_u8(in_ptr - in_stride);
        const uint8x16_t mid_data = vld1q_u8(in_ptr); 
        const uint8x16_t bot_data = vld1q_u8(in_ptr + in_stride);
        
        uint8x8_t top_high_data = vget_high_u8(top_data);
        uint8x8_t top_low_data  = vget_low_u8(top_data);
        
        uint8x8_t mid_high_data = vget_high_u8(mid_data);
        uint8x8_t mid_low_data  = vget_low_u8(mid_data);
        
        uint8x8_t bot_high_data = vget_high_u8(bot_data);
        uint8x8_t bot_low_data  = vget_low_u8(bot_data);
        
        uint8x8_t p0, p1;
        
        p0 = top_low_data;
        p1 = vext_u8(top_low_data, top_high_data, 1);
        p0 = vmin_u8(p0, p1);
        
        p1 = vext_u8(top_low_data, top_high_data, 2);
        p0 = vmin_u8(p0, p1);
        
        p1 = mid_low_data;
        p0 = vmin_u8(p0, p1);
        
        p1 = vext_u8(mid_low_data, mid_high_data, 1);
        p0 = vmin_u8(p0, p1);
        
        p1 = vext_u8(mid_low_data, mid_high_data, 2);
        p0 = vmin_u8(p0, p1);
        
        p1 = bot_low_data;
        p0 = vmin_u8(p0, p1);
        
        p1 = vext_u8(bot_low_data, bot_high_data, 1);
        p0 = vmin_u8(p0, p1);
        
        p1 = vext_u8(bot_low_data, bot_high_data, 2);
        p0 = vmin_u8(p0, p1);
        
        vst1_u8(out_data, p0);
}

void
erode3x3_neon(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
	int i, j;
	// Ignore edge for the mean time
	in_data += w;
	out_data += w;
	for (j = 1; j < h - 1; j++) {
		for (i = 1; i < w - 8; i+=8) {
			erode3x3_neon_kernel(in_data+i, out_data+i, w);
		}
		in_data += w;
		out_data += w;
	}
}
#endif
