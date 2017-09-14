#pragma version(1)
#pragma rs java_package_name(unused)
#pragma rs_fp_relaxed

int32_t gWidth;
int32_t gHeight;
rs_allocation gIn;

uint8_t RS_KERNEL root(uint32_t x, uint32_t y) {
    uint32_t x1 = min((int32_t)x+1, gWidth-1);
    uint32_t x2 = max((int32_t)x-1, 0);
    uint32_t y1 = min((int32_t)y+1, gHeight-1);
    uint32_t y2 = max((int32_t)y-1, 0);

    uint8_t r = rsGetElementAt_uchar(gIn, x1, y);
    r = min(r, rsGetElementAt_uchar(gIn, x, y));
    r = min(r, rsGetElementAt_uchar(gIn, x2, y));
    r = min(r, rsGetElementAt_uchar(gIn, x1, y1));
    r = min(r, rsGetElementAt_uchar(gIn, x, y1));
    r = min(r, rsGetElementAt_uchar(gIn, x2, y1));
    r = min(r, rsGetElementAt_uchar(gIn, x1, y2));
    r = min(r, rsGetElementAt_uchar(gIn, x, y2));
    r = min(r, rsGetElementAt_uchar(gIn, x2, y2));

    return r;
}
