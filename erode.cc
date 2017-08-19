#include <assert.h>
#include "erode.h"

inline int min( uint8_t a, uint8_t b ) { return a < b ? a : b; }

void
erode3x3(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    int i, j;
    uint8_t tmp1, tmp2;

    assert(h>=2);
    assert(w>=2);

    // top row
    tmp1 = min(in_data[0],  in_data[1]);
    tmp2 = min(in_data[+w], in_data[+w+1]);
    out_data[0] = min(tmp1, tmp2);
    for (i = 1; i < w-1; i++) {
        tmp1 = min(min(in_data[i-1],   in_data[i]),   in_data[i+1]);
        tmp2 = min(min(in_data[i+w-1], in_data[i+w]), in_data[i+w+1]);
        out_data[i] = min(tmp1, tmp2);
    }
    tmp1 = min(in_data[i-1],   in_data[i]);
    tmp2 = min(in_data[i+w-1], in_data[i+w]);
    out_data[i] = min(tmp1, tmp2);

    in_data += w;
    out_data += w;

    // middle rows
    for (j = 1; j < h-1; j++) {
        tmp1 = min(in_data[-w], in_data[-w+1]);
        tmp2 = min(in_data[0],  in_data[1]);
        tmp1 = min(tmp1, tmp2);
        tmp2 = min(in_data[+w], in_data[+w+1]);
        out_data[0] = min(tmp1, tmp2);
        for (i = 1; i < w-1; i++) {
            tmp1 = min(min(in_data[i-w-1], in_data[i-w]), in_data[i-w+1]);
            tmp2 = min(min(in_data[i-1],   in_data[i]),   in_data[i+1]);
            tmp1 = min(tmp1, tmp2);
            tmp2 = min(min(in_data[i+w-1], in_data[i+w]), in_data[i+w+1]);
            out_data[i] = min(tmp1, tmp2);
        }
        tmp1 = min(in_data[i-w-1], in_data[i-w]);
        tmp2 = min(in_data[i-1],   in_data[i]);
        tmp1 = min(tmp1, tmp2);
        tmp2 = min(in_data[i+w-1], in_data[i+w]);
        out_data[i] = min(tmp1, tmp2);

        in_data += w;
        out_data += w;
    }

    // bottom row
    tmp1 = min(in_data[-w], in_data[-w+1]);
    tmp2 = min(in_data[0],  in_data[1]);
    out_data[0] = min(tmp1, tmp2);
    for (i = 1; i < w-1; i++) {
        tmp1 = min(min(in_data[i-w-1], in_data[i-w]), in_data[i-w+1]);
        tmp2 = min(min(in_data[i-1],   in_data[i]),   in_data[i+1]);
        out_data[i] = min(tmp1, tmp2);
    }
    tmp1 = min(in_data[i-w-1], in_data[i-w]);
    tmp2 = min(in_data[i-1],   in_data[i]);
    out_data[i] = min(tmp1, tmp2);

    in_data += w;
    out_data += w;
}
