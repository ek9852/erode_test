OpenCL/neon erode (3x3) test program
=============================================================================

This package implements erode 3x3 (edge case not handled propertly currently)

Build
-----
Use standard cmake

To build with Android: (make sure your device have libOpenCL.so as it is not supported in AOSP)

    export NDK_PROJECT_PATH=.
    ndk-build NDK_APPLICATION_MK=./Application.mk

Dependency
----------
opencl

Usage
-----
To run:

    $ ./erode_test

    Example output:
    === Testing cl erode 3x3
    Platform size 1
    Device size 1
    Device Name: Mali-G71
    Device Type: 4 (GPU: 4, CPU: 2, ACCEL: 8)
    Device Vendor: ARM
    Device Max Compute Units: 8
    Device Global Memory: 3734020096
    Device Max Clock Frequency: 400
    Device Max Allocateable Memory: 933505024
    Device Local Memory: 32768
    Device Available: 1
    Kernel Exec  : Queue  to Submit: 376 us
    Kernel Exec  : Submit to Start : 1960 us
    Kernel Exec  : Start  to End   : 75 us
    
    CPU Wall Time spent: 4025521ns
    
    === Testing neon erode 3x3
    CPU Wall Time spent: 193750ns
