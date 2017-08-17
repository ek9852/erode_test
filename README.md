OpenCL/OpenGL/neon erode (3x3) performance test program
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

    === Testing gl erode 3x3
    EGL initialized. Version: 1.4
    Num of configs matching: 2
    [0] Config:
      Sizes: 32[8,8,8][0,8,0]
      Bind: [0,0] BufferType: [12430] Config: [12344,9,4]
      Depth: [0]Level: [0]
      Swap interval: [10,1] Native: [0,0]
      Renderable: [4] Samples: [0,0]
      Stencil: [0], Surface: [5]
      Tranparent: 12344[0,0,0]
    [1] Config:
      Sizes: 32[8,8,8][0,8,0]
      Bind: [0,0] BufferType: [12430] Config: [12344,8,4]
      Depth: [24]Level: [0]
      Swap interval: [10,1] Native: [0,0]
      Renderable: [4] Samples: [0,0]
      Stencil: [8], Surface: [5]
      Tranparent: 12344[0,0,0]
    Extension support: GL_OES_rgb8_rgba8 GL_OES_depth24 GL_OES_vertex_half_float GL_OES_texture_float GL_OES_texture_half_float GL_OES_element_indexg
    CPU Wall Time spent: 19153090ns
