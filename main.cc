#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include "erode_cl.h"
#include "erode.h"
#if defined(__ARM_NEON__) || defined(__aarch64__)
#include "erode_neon.h"
#endif
#if defined(__ARM_NEON__) && !defined(__aarch64__) && !defined(__ANDROID__)
#include "erode_halide.h"
#endif
#if defined(WITH_OPENGLES)
#include "erode_gl.h"
#endif
#if defined(__linux__)
#include "pmu_utils.h"
#endif
#if defined(__x86_64__)
#include "erode_sse.h"
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

#ifdef __linux__
    bool use_pmu = false;
    // init pmu counter if exists
    if (setup_pmu_counters() == 0) {
        use_pmu = 1;
    }
#endif

    timespec start, end;
    int64_t start_ns;
    int64_t end_ns; 
    int64_t diff_ns;

    std::cout << "=== Testing c erode 3x3" << std::endl;
    clock_gettime(CLOCK_MONOTONIC, &start);

#ifdef __linux__
    if (use_pmu) start_pmu_counters();
#endif
    erode3x3(h_a, h_b, w, h);
#ifdef __linux__
    if (use_pmu) stop_pmu_counters();
#endif
    clock_gettime(CLOCK_MONOTONIC, &end);

#ifdef __linux__
    if (use_pmu) print_pmu_counters();
#endif

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

    std::cout << "=== Testing cl erode 3x3" << std::endl;
    erode3x3_cl_init(w, h, true);
    clock_gettime(CLOCK_MONOTONIC, &start);
    erode3x3_cl(h_a, h_c, w, h);
    clock_gettime(CLOCK_MONOTONIC, &end);
    erode3x3_cl_destroy();

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

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

#if defined(__x86_64__)
    std::cout << std::endl << "=== Testing sse erode 3x3" << std::endl;
    clock_gettime(CLOCK_MONOTONIC, &start);

#ifdef __linux__
    if (use_pmu) start_pmu_counters();
#endif
    erode3x3_sse(h_a, h_c, w, h);
#ifdef __linux__
    if (use_pmu) stop_pmu_counters();
#endif
    clock_gettime(CLOCK_MONOTONIC, &end);

#ifdef __linux__
    if (use_pmu) print_pmu_counters();
#endif

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

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
    clock_gettime(CLOCK_MONOTONIC, &start);

#ifdef __linux__
    if (use_pmu) start_pmu_counters();
#endif
    erode3x3_neon(h_a, h_c, w, h);
#ifdef __linux__
    if (use_pmu) stop_pmu_counters();
#endif
    clock_gettime(CLOCK_MONOTONIC, &end);

#ifdef __linux__
    if (use_pmu) print_pmu_counters();
#endif

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

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
    clock_gettime(CLOCK_MONOTONIC, &start);
#ifdef __linux__
    if (use_pmu) start_pmu_counters();
#endif
    erode3x3_halide(h_a, h_c, w, h);
#ifdef __linux__
    if (use_pmu) stop_pmu_counters();
#endif
    clock_gettime(CLOCK_MONOTONIC, &end);

#ifdef __linux__
    if (use_pmu) print_pmu_counters();
#endif

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

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
    clock_gettime(CLOCK_MONOTONIC, &start);
    erode3x3_gl(h_a, h_c, w, h);
    clock_gettime(CLOCK_MONOTONIC, &end);
    erode3x3_gl_destroy();

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

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
 
    return 0;
}
