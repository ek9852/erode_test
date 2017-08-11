#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include "erode_cl.h"
#if defined(__ARM_NEON__) || defined(__aarch64__)
#include "erode_neon.h"
#endif

int main(int argc, char *argv[])
{
    uint8_t *h_a, *h_b;
    int w, h;
    size_t n;
    w = 320;
    h = 240;
    n = w * h;
    h_a = new uint8_t[w * h];
    h_b = new uint8_t[w * h];

    // Initialize test data on host
    for(int i = 0; i < n; i++ )
    {
        h_a[i] = sinf(i)*sinf(i);
    }

    timespec start, end;
    int64_t start_ns;
    int64_t end_ns; 
    int64_t diff_ns;

    std::cout << "=== Testing cl erode 3x3" << std::endl;
    erode3x3_cl_init(w, h, true);
    clock_gettime(CLOCK_MONOTONIC, &start);
    erode3x3_cl(h_a, h_b, w, h);
    clock_gettime(CLOCK_MONOTONIC, &end);
    erode3x3_cl_destroy();

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;

#if defined(__ARM_NEON__) || defined(__aarch64__)
    std::cout << std::endl << "=== Testing neon erode 3x3" << std::endl;
    clock_gettime(CLOCK_MONOTONIC, &start);
    erode3x3_neon(h_a, h_b, w, h);
    clock_gettime(CLOCK_MONOTONIC, &end);

    start_ns = start.tv_sec * 1000000000LL + start.tv_nsec;
    end_ns = end.tv_sec * 1000000000LL + end.tv_nsec;
    diff_ns = end_ns - start_ns;

    std::cout << "CPU Wall Time spent: " << diff_ns << "ns" << std::endl;
#endif

 
    delete[] h_a;
    delete[] h_b;
 
    return 0;
}
