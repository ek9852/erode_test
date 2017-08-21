#if defined(__ARM_NEON__) || defined(__aarch64__)

#include "erode_neon.h"
#include <arm_neon.h>
#include <assert.h>

static inline void __attribute__((always_inline))
erode3x3_neon_kernel(uint8_t *in_data, uint8_t *out_data, const int stride,
		const bool parallel_2_rows, const bool left, const bool top, const bool right, const bool bottom)
{
    uint8x8_t p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11;

	// | 1 | .... | 8 | 8 |
    uint8_t         *in_ptr   = left ? in_data : (right ? in_data - 8 : in_data - 1);
    uint8x16_t mid_data = vld1q_u8(in_ptr); 
    uint8x16_t top_data = top ? mid_data : vld1q_u8(in_ptr - stride);
    uint8x16_t bot_data = (bottom && !parallel_2_rows) ? mid_data : vld1q_u8(in_ptr + stride);
    uint8x16_t bot_2_data;

    if (parallel_2_rows) bot_2_data = bottom ? bot_data : vld1q_u8(in_ptr + 2 * stride);

    if (right) {
        p0 = vget_high_u8(top_data);
        p1 = vext_u8(vget_high_u8(top_data), vrev64_u8(vget_high_u8(top_data)), 1);
        p2 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 7);

        p3 = vget_high_u8(mid_data);
        p4 = vext_u8(vget_high_u8(mid_data), vrev64_u8(vget_high_u8(mid_data)), 1);
        p5 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 7);

        p6 = vget_high_u8(bot_data);
        p7 = vext_u8(vget_high_u8(bot_data), vrev64_u8(vget_high_u8(bot_data)), 1);
        p8 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 7);

        if (parallel_2_rows) {
            p9 = vget_high_u8(bot_2_data);
            p10 = vext_u8(vget_high_u8(bot_2_data), vrev64_u8(vget_high_u8(bot_2_data)), 1);
            p11 = vext_u8(vget_low_u8(bot_2_data), vget_high_u8(bot_2_data), 7);
        }
    } else if (left) {
        p0 = vget_low_u8(top_data);
        p1 = vext_u8(vrev64_u8(vget_low_u8(top_data)), vget_low_u8(top_data), 7);
        p2 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 1);

        p3 = vget_low_u8(mid_data);
        p4 = vext_u8(vrev64_u8(vget_low_u8(mid_data)), vget_low_u8(mid_data), 7);
        p5 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 1);

        p6 = vget_low_u8(bot_data);
        p7 = vext_u8(vrev64_u8(vget_low_u8(bot_data)), vget_low_u8(bot_data), 7);
        p8 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 1);

        if (parallel_2_rows) {
            p9 = vget_low_u8(bot_2_data);
            p10 = vext_u8(vrev64_u8(vget_low_u8(bot_2_data)), vget_low_u8(bot_2_data), 7);
            p11 = vext_u8(vget_low_u8(bot_2_data), vget_high_u8(bot_2_data), 1);
        }
    } else {
        p0 = vget_low_u8(top_data);
        p1 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 1);
        p2 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 2);

        p3 = vget_low_u8(mid_data);
        p4 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 1);
        p5 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 2);

        p6 = vget_low_u8(bot_data);
        p7 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 1);
        p8 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 2);

        if (parallel_2_rows) {
            p9 = vget_low_u8(bot_2_data);
            p10 = vext_u8(vget_low_u8(bot_2_data), vget_high_u8(bot_2_data), 1);
            p11 = vext_u8(vget_low_u8(bot_2_data), vget_high_u8(bot_2_data), 2);
        }
    }
    p3 = vmin_u8(p3, p4);
    p3 = vmin_u8(p3, p5);
    p3 = vmin_u8(p3, p6);
    p3 = vmin_u8(p3, p7);
    p3 = vmin_u8(p3, p8);

    p0 = vmin_u8(p0, p3);
    p0 = vmin_u8(p0, p1);
    p0 = vmin_u8(p0, p2);

    vst1_u8(out_data, p0);

    if (parallel_2_rows) {
        p9 = vmin_u8(p3, p9);
        p9 = vmin_u8(p9, p10);
        p9 = vmin_u8(p9, p11);

        vst1_u8(out_data+stride, p9);
    }
}

void
erode3x3_neon(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    int i, j;

    assert(w >= 16);
    assert(h > 2);

    // top row
    erode3x3_neon_kernel(in_data, out_data, w, true, true, true, false, false);
    for (i = 8; i < w - 8; i+=8) {
        erode3x3_neon_kernel(in_data+i, out_data+i, w, true, false, true, false, false);
    }
    // move backward last one, so that it align to 8 bytes
    erode3x3_neon_kernel(in_data+w-8, out_data+w-8, w, true, false, true, true, false);
    in_data += 2*w;
    out_data += 2*w;

    // middle row
    for (j = 2; j < h - 2; j+=2) {
        erode3x3_neon_kernel(in_data, out_data, w, true, true, false, false, false);
        for (i = 8; i < w - 8; i+=8) {
            erode3x3_neon_kernel(in_data+i, out_data+i, w, true, false, false, false, false);
        }
        // move backward last one, so that it align to 8 bytes
        erode3x3_neon_kernel(in_data+w-8, out_data+w-8, w, true, false, false, true, false);
        in_data += 2*w;
        out_data += 2*w;
    }

    // left over 2 rows or 1 row
    bool two_rows = (j == (h-2));

    erode3x3_neon_kernel(in_data, out_data, w, two_rows, true, false, false, true);
    for (i = 8; i < w - 8; i+=8) {
        erode3x3_neon_kernel(in_data+i, out_data+i, w, two_rows, false, false, false, true);
    }
    // move backward last one, so that it align to 8 bytes
    erode3x3_neon_kernel(in_data+w-8, out_data+w-8, w, two_rows, false, false, true, true);
}
#endif
