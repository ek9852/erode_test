#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "erode_cl.h"
#ifndef __APPLE__
#include <CL/cl_ext.h>
#ifdef CL_MEM_USE_MSMC_TI
#include "erode_tidsp_cl.h"
#endif
#endif
#include "erode.h"
#include "pmu_utils.h"
#if defined(__ARM_NEON__) || defined(__aarch64__)
#include "erode_neon.h"
#endif
#if defined(__ARM_NEON__) && !defined(__aarch64__) && !defined(__ANDROID__)
#include "erode_halide.h"
#endif
#if defined(WITH_OPENGLES)
#include "erode_gl.h"
#endif
#if defined(__x86_64__)
#include "erode_sse.h"
#endif
#if defined(__ANDROID__)
#include "erode_rs.h"
#endif

int main(int argc, char *argv[])
{
    uint8_t *h_a, *h_b , *h_c;
    int w, h;
    size_t n;
    w = 320;
    h = 240;
    n = w * h;
    h_a = new uint8_t[w * h];
    h_b = new uint8_t[w * h];
    h_c = new uint8_t[w * h];

    // Initialize test data on host
    for(int i = 0; i < n; i++ )
    {
        h_a[i] = i % 256;
    }

    setup_pmu_counters();

    std::cout << "=== Testing c erode 3x3" << std::endl;

    start_pmu_counters();
    erode3x3(h_a, h_b, w, h);
    stop_pmu_counters();
    print_pmu_counters();

    std::cout << "=== Testing cl erode 3x3" << std::endl;
    erode3x3_cl_init(w, h, true);
    start_pmu_counters();
    erode3x3_cl(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();
    erode3x3_cl_destroy();

    // compare c and opencl implementation
    // TODO opencl does not handle edge case currently, skip those
    for (int j = 1; j < h-1; j++) {
        for (int i = 1; i < (w-2)/8*8 + 1; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "C and opencl implementation equals" << std::endl;

#ifdef CL_MEM_USE_MSMC_TI
    std::cout << "=== Testing tidsp cl erode 3x3" << std::endl;
    memset(h_c, 0, w*h);
    erode3x3_tidsp_cl_init(w, h);
    // mapping to device buffer
    uint8_t *d_a = erode3x3_tidsp_cl_map_input_buf();
    memcpy(d_a, h_a, w*h);
    erode3x3_tidsp_cl_unmap_input_buf(d_a);
    start_pmu_counters();
    erode3x3_tidsp_cl();
    stop_pmu_counters();
    print_pmu_counters();
    // mapping to device buffer
    uint8_t *d_b = erode3x3_tidsp_cl_map_output_buf();
    memcpy(h_c, d_b, w*h);
    erode3x3_tidsp_cl_unmap_output_buf(d_b);
    erode3x3_tidsp_cl_destroy();

    // compare c and tidsp opencl implementation
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "C and tidsp opencl implementation equals" << std::endl;
#endif

#ifdef __ANDROID__
    std::cout << "=== Testing renderscript erode 3x3" << std::endl;
    erode3x3_rs_init(w, h);
    start_pmu_counters();
    erode3x3_rs(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();
    erode3x3_rs_destroy();

    // compare c and renderscript implementation
    // TODO opencl does not handle edge case currently, skip those
    for (int j = 1; j < h-1; j++) {
        for (int i = 1; i < (w-2)/8*8 + 1; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "C and renderscript implementation equals" << std::endl;

#endif

#if defined(__x86_64__)
    std::cout << std::endl << "=== Testing sse erode 3x3" << std::endl;

    start_pmu_counters();
    erode3x3_sse(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();

    // compare C and sse implementation
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "SSE and C implementation equals" << std::endl;
#endif

#if defined(__ARM_NEON__) || defined(__aarch64__)
    std::cout << std::endl << "=== Testing neon erode 3x3" << std::endl;

    start_pmu_counters();
    erode3x3_neon(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();

    // compare neon and c implementation
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "Neon and C implementation equals" << std::endl;
#endif

#if defined(__ARM_NEON__) && !defined(__aarch64__) && !defined(__ANDROID__)
    std::cout << std::endl << "=== Testing halide erode 3x3" << std::endl;
    start_pmu_counters();
    erode3x3_halide(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();

    // compare with C implementation
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j << std::endl;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "Halide and C implementation equals" << std::endl;
#endif

#ifdef WITH_OPENGLES
    std::cout << "=== Testing gl erode 3x3" << std::endl;
    erode3x3_gl_init(w, h);
    start_pmu_counters();
    erode3x3_gl(h_a, h_c, w, h);
    stop_pmu_counters();
    print_pmu_counters();
    erode3x3_gl_destroy();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (h_b[j*w + i] != h_c[j*w + i]) {
                std::cout << "Not equal @" << i << "," << j;
                std::cout << " [" <<  (int)h_b[j*w + i] << "," <<  (int)h_c[j*w + i] << "]" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "opengl and C implementation equals" << std::endl;
#endif

    delete[] h_a;
    delete[] h_b;
    delete[] h_c;

    close_pmu_counters();
 
    return 0;
}
