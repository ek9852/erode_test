cmake_minimum_required (VERSION 2.8)
project (erode_test)

#-------------------------------------------------------------------------------
#add local find scripts to CMAKE path
list(APPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake
)

if(MAKE_MAJOR_VERSION GREATER 2)
  find_package(OpenCL REQUIRED)
else()
# Determine machine bitness
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS 64)
  else()
    set(BITNESS 32)
  endif()
  
  # Find OpenCL include directories
  find_path( OpenCL_INCLUDE_DIRS
    NAMES CL/cl.h OpenCL/cl.h
    HINTS
      $ENV{AMDAPPSDKROOT}/include
      $ENV{INTELOCLSDKROOT}/include
      $ENV{CUDA_PATH}/include
      $ENV{OPENCL_ROOT}/include
    PATHS
      /usr/include
      /usr/local/include
    )
  
  # Find OpenCL libraries
  if(BITNESS EQUAL 64)
    find_library( OpenCL_LIBRARY
      NAMES OpenCL
      HINTS
        $ENV{AMDAPPSDKROOT}/lib
        $ENV{INTELOCLSDKROOT}/lib
        $ENV{CUDA_PATH}/lib
        $ENV{OPENCL_ROOT}/lib
      PATH_SUFFIXES x86_64 x64
      PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib
    )
  elseif(BITNESS EQUAL 32)
    find_library( OpenCL_LIBRARY
      NAMES OpenCL
      HINTS
        $ENV{AMDAPPSDKROOT}/lib
        $ENV{INTELOCLSDKROOT}/lib
        $ENV{CUDA_PATH}/lib
        $ENV{OPENCL_ROOT}/lib
      PATH_SUFFIXES x86 Win32
      PATHS
        /usr/lib32
        /usr/lib
        /usr/local/lib
    )
  endif()
  
  if( (NOT OpenCL_INCLUDE_DIRS) OR (NOT OpenCL_LIBRARY ) )
    message( FATAL_ERROR "Could not find OpenCL include/libs. Set OPENCL_ROOT to your OpenCL SDK. Download AMD APP SDK "
        "http://developer.amd.com/tools-and-sdks/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/ for x86/x64 "
        "or pocl http://pocl.sourceforge.net/ for ARM systems")
  else()
    message(STATUS "Selected OpenCL includes from ${OpenCL_INCLUDE_DIRS}")
    message(STATUS "Selected OpenCL lib ${OpenCL_LIBRARY}")
  endif()

endif()


include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/halide")

add_executable(erode_test main.cc erode.cc erode_tidsp_cl.cc erode_cl.cc erode_neon.cc erode_halide.cc pmu_utils.cc)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_link_libraries(erode_test ${PROJECT_SOURCE_DIR}/halide/erode_arm_32_linux.o pthread ${CMAKE_DL_LIBS} )
endif ()

find_package(OpenMP QUIET)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DWITH_OPENMP)
    message("Using OpenMP")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    target_sources(erode_test PRIVATE erode_sse.cc)
endif()


find_package(OpenGLES)
find_package(EGL)
if (OPENGLES_FOUND AND EGL_FOUND)
    include_directories(${OPENGLES_INCLUDE_DIRS} ${EGL_INCLUDE_DIRS})
    set( GLES_SRC_FILES erode_gl.cc )
    target_sources(erode_test PRIVATE erode_gl.cc)
    target_link_libraries(erode_test ${OPENGLES_LIBRARIES} ${EGL_LIBRARIES} )

    # ignore unresolved symbols in shared libraries
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--unresolved-symbols=ignore-in-shared-libs")
    add_definitions(-DWITH_OPENGLES)
endif (OPENGLES_FOUND AND EGL_FOUND)

target_link_libraries(erode_test ${OpenCL_LIBRARY} )
install(TARGETS erode_test DESTINATION bin)

