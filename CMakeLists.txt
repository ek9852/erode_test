cmake_minimum_required (VERSION 3.0)
project (erode_test)

find_package(OpenCL REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/halide")
add_executable(erode_test main.cc erode_cl.cc erode_neon.cc erode_halide.cc)
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_link_libraries(erode_test ${PROJECT_SOURCE_DIR}/halide/erode_arm_32_linux.o pthread ${CMAKE_DL_LIBS} )
endif ()

target_link_libraries(erode_test ${OpenCL_LIBRARY} )
install(TARGETS erode_test DESTINATION bin)

