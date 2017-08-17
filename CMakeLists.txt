cmake_minimum_required (VERSION 3.1)
project (erode_test)

#-------------------------------------------------------------------------------
#add local find scripts to CMAKE path
list(APPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake
)

find_package(OpenCL REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/halide")

add_executable(erode_test main.cc erode_cl.cc erode_neon.cc erode_halide.cc)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    target_link_libraries(erode_test ${PROJECT_SOURCE_DIR}/halide/erode_arm_32_linux.o pthread ${CMAKE_DL_LIBS} )
endif ()

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

