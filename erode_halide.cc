#if defined(__ARM_NEON__) && !defined(__aarch64__) && !defined(__ANDROID__)

#include "HalideBuffer.h"
#include "HalideRuntime.h"
#include "erode_arm_32_linux.h"
#include "erode_halide.h"

void
erode3x3_halide(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    halide_buffer_t srcBuf = {0};
    halide_dimension_t srcDim[2];
    halide_buffer_t dstBuf = {0};
    halide_dimension_t dstDim[2];

    srcBuf.host = (uint8_t *)in_data;
    srcBuf.set_host_dirty();
    srcBuf.dim = srcDim;
    srcBuf.dim[0].min = 0;
    srcBuf.dim[0].extent = w;
    srcBuf.dim[0].stride = 1;
    srcBuf.dim[1].min = 0;
    srcBuf.dim[1].extent = h;
    srcBuf.dim[1].stride = w;
    srcBuf.type = halide_type_of<uint8_t>();

    dstBuf.host = (uint8_t *)out_data;
    dstBuf.dim = dstDim;
    dstBuf.dim[0].min = 0;
    dstBuf.dim[0].extent = w;
    dstBuf.dim[0].stride = 1;
    dstBuf.dim[1].min = 0;
    dstBuf.dim[1].extent = h;
    dstBuf.dim[1].stride = w;
    dstBuf.type = halide_type_of<uint8_t>();

    erode_halide(&srcBuf, &dstBuf);

    halide_copy_to_host(NULL, &dstBuf);
}

#endif
