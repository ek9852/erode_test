#include <assert.h>
#include <x86intrin.h>
#include "erode_sse.h"

unsigned char arrangeLMask[16] __attribute__((aligned(16))) = {
   0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

unsigned char arrangeRMask[16] __attribute__((aligned(16))) = {
   1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15
};

static inline void __attribute__((always_inline))
erode3x3_sse_kernel(uint8_t *in_data, uint8_t *out_data, const int in_stride,
		const bool left, const bool top, const bool right, const bool bottom)
{
    __m128i p0, p1;
    __m128i top_data, mid_data, bot_data;
    const __m128i lmask = _mm_load_si128((__m128i*)arrangeLMask);
    const __m128i rmask = _mm_load_si128((__m128i*)arrangeRMask);

    // top row
    if (!top) {
        top_data = _mm_loadu_si128((__m128i *)&in_data[-in_stride]);
        if (!left)
            p0 = _mm_loadu_si128((__m128i *)&in_data[-in_stride-1]);
        else
            p0 = _mm_shuffle_epi8(top_data, lmask);
        if (!right)
            p1 = _mm_loadu_si128((__m128i *)&in_data[-in_stride+1]);
        else
            p1 = _mm_shuffle_epi8(top_data, rmask);
        top_data = _mm_min_epu8(top_data, p0);
        top_data = _mm_min_epu8(top_data, p1);
    }

    // middle row
    mid_data = _mm_loadu_si128((__m128i *)&in_data[0]);
    if (!left)
        p0 = _mm_loadu_si128((__m128i *)&in_data[-1]);
    else
        p0 = _mm_shuffle_epi8(mid_data, lmask);
    if (!right)
        p1 = _mm_loadu_si128((__m128i *)&in_data[+1]);
    else
        p1 = _mm_shuffle_epi8(mid_data, rmask);
    mid_data = _mm_min_epu8(mid_data, p0);
    mid_data = _mm_min_epu8(mid_data, p1);

    // bottom row
    if (!bottom) {
        bot_data = _mm_loadu_si128((__m128i *)&in_data[in_stride]);
        if (!left)
            p0 = _mm_loadu_si128((__m128i *)&in_data[in_stride-1]);
        else
            p0 = _mm_shuffle_epi8(bot_data, lmask);
        if (!right)
            p1 = _mm_loadu_si128((__m128i *)&in_data[in_stride+1]);
        else
            p1 = _mm_shuffle_epi8(bot_data, rmask);
        bot_data = _mm_min_epu8(bot_data, p0);
        bot_data = _mm_min_epu8(bot_data, p1);
    }

    if ((!top) && (!bottom)) {
        p0 = _mm_min_epu8(top_data, mid_data);
        p0 = _mm_min_epu8(bot_data, p0);
    } else if (top) {
        p0 = _mm_min_epu8(bot_data, mid_data);
    } else { // bottom
        p0 = _mm_min_epu8(top_data, mid_data);
    }
    _mm_storeu_si128((__m128i *) out_data, p0);
}

void
erode3x3_sse(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    int i, j;

    assert(w > 16);
    assert(h >= 2);

    // top row
    erode3x3_sse_kernel(in_data, out_data, w, true, true, false, false);
    for (i = 16; i < w - 16; i+=16) {
        erode3x3_sse_kernel(in_data+i, out_data+i, w, false, true, false, false);
    }
    // move backward last one, so that it align to 16 bytes
    erode3x3_sse_kernel(in_data+w-16, out_data+w-16, w, false, true, true, false);
    in_data += w;
    out_data += w;

    // middle row
    for (j = 1; j < h - 1; j++) {
        erode3x3_sse_kernel(in_data, out_data, w, true, false, false, false);
        for (i = 16; i < w - 16; i+=16) {
            erode3x3_sse_kernel(in_data+i, out_data+i, w, false, false, false, false);
        }
        // move backward last one, so that it align to 16 bytes
        erode3x3_sse_kernel(in_data+w-16, out_data+w-16, w, false, false, true, false);
        in_data += w;
        out_data += w;
    }

    // bottom row
    erode3x3_sse_kernel(in_data, out_data, w, true, false, false, true);
    for (i = 16; i < w - 16; i+=16) {
        erode3x3_sse_kernel(in_data+i, out_data+i, w, false, false, false, true);
    }
    // move backward last one, so that it align to 16 bytes
    erode3x3_sse_kernel(in_data+w-16, out_data+w-16, w, false, false, true, true);
    in_data += w;
    out_data += w;
}
