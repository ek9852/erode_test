cmake_minimum_required (VERSION 2.6)
project (erode_test)

find_package(OpenCL REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/include")
add_executable(erode_test main.cc erode_cl.cc erode_neon.cc)
target_link_libraries(erode_test ${OpenCL_LIBRARY} )
install(TARGETS erode_test DESTINATION bin)

