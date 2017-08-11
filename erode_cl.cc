#define __CL_ENABLE_EXCEPTIONS
#include <cstdio>
#include <cstdlib>
#include <iostream>
 
#include "cl.hpp"
#include "erode_cl.h"

const char *kernelSource =                                     "\n" \
"__kernel void erode(  __global uchar *in_data,                 \n" \
"                      __global uchar *out_data,                \n" \
"                      int stride_x)                            \n" \
"{                                                              \n" \
"                                                               \n" \
"    in_data += (get_global_id(0) - 1) * 8 + 1 + get_global_id(1) * stride_x; \n" \
"    out_data += (get_global_id(0) - 1) * 8 + 1 + get_global_id(1) * stride_x;\n" \
"                                                               \n" \
// TODO vload16 must be 16 bytes aligned, otherwise result is undefined by opencl specification.
// however most imeplementation allow 8 bytes alignment.
"    uchar16 top    = vload16(0, in_data - 1 - stride_x);       \n" \
"    uchar16 middle = vload16(0, in_data - 1);                  \n" \
"    uchar16 bottom = vload16(0, in_data - 1 + stride_x);       \n" \
"                                                               \n" \
"    uchar16 tmp = min(top, min(middle, bottom));               \n" \
"    uchar8  out = min(tmp.s01234567, min(tmp.s12345678, tmp.s23456789)); \n" \
"                                                               \n" \
"    vstore8(out, 0, out_data);                                 \n" \
"}                                                              \n";

static cl::Kernel kernel;
static cl::CommandQueue queue;
static cl::Buffer d_a;
static cl::Buffer d_b;
static cl::Context context;
static bool useHostPtr;

static void
ocl_event_times(const cl::Event &ev, const char* name)
{
     cl_ulong t_que, t_sub, t_strt, t_end;

     ev.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &t_que);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &t_sub);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_START,  &t_strt);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_END,    &t_end);

     /*----------------------------------------------------------------------
     * Normalize the time to microseconds
     *--------------------------------------------------------------------*/
     t_que  /= 1000; t_sub  /= 1000; t_strt /= 1000; t_end  /= 1000;

     if (!name) name = "";

     std::cout<< name << " : Queue  to Submit: " << t_sub-t_que  << " us" << std::endl;
     std::cout<< name << " : Submit to Start : " << t_strt-t_sub << " us" << std::endl;
     std::cout<< name << " : Start  to End   : " << t_end-t_strt << " us" << std::endl;
     std::cout<< std::endl;
}

int
erode3x3_cl_init(int w, int h, bool use_host_ptr)
{
    size_t bytes = w * h;
    cl_int err = CL_SUCCESS;

    useHostPtr = use_host_ptr;

#ifdef __CL_ENABLE_EXCEPTIONS
    try {
#endif
        // Query platforms
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0) {
            std::cout << "Platform size 0\n";
            return -1;
        }
        std::cout << "Platform size " << platforms.size() << std::endl;
 
        // Get list of devices on default platform and create context
        cl_context_properties properties[] =
           { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
        context = cl::Context(CL_DEVICE_TYPE_DEFAULT, properties);
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        std::cout << "Device size " << devices.size() << std::endl;

	cl::Device device  = devices[0];

        std::cout << "Device Name: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;  
        std::cout << "Device Type: " << device.getInfo<CL_DEVICE_TYPE>();
        std::cout << " (GPU: " << CL_DEVICE_TYPE_GPU << ", CPU: " << CL_DEVICE_TYPE_CPU << ", ACCEL: " << CL_DEVICE_TYPE_ACCELERATOR << ")" << std::endl;  
        std::cout << "Device Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;
        std::cout << "Device Max Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
        std::cout << "Device Global Memory: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << std::endl;
        std::cout << "Device Max Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << std::endl;
        std::cout << "Device Max Allocateable Memory: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << std::endl;
        std::cout << "Device Local Memory: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << std::endl;
        std::cout << "Device Available: " << device.getInfo< CL_DEVICE_AVAILABLE>() << std::endl;
 
        // Create command queue for first device
	queue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);

        // Create device memory buffers
	if (!use_host_ptr) {
        	d_a = cl::Buffer(context, CL_MEM_READ_ONLY, bytes);
        	d_b = cl::Buffer(context, CL_MEM_WRITE_ONLY, bytes);
	}
 
        //Build kernel from source string
        cl::Program::Sources source(1,
            std::make_pair(kernelSource,strlen(kernelSource)));
        cl::Program program_ = cl::Program(context, source);
        program_.build(devices);
 
        // Create kernel object
        kernel = cl::Kernel(program_, "erode", &err);
 
#ifdef __CL_ENABLE_EXCEPTIONS
    } catch (cl::Error err) {
         std::cerr
            << "ERROR: "<<err.what()<<"("<<err.err()<<")"<<std::endl;
         return -1;
    }
#endif
 
    return 0; 
}

void
erode3x3_cl(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
	size_t bytes = w * h;

#ifdef __CL_ENABLE_EXCEPTIONS
    try {
#endif
	if (useHostPtr) {
        	d_a = cl::Buffer(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, bytes, in_data);
        	d_b = cl::Buffer(context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, bytes, out_data);
	} else {
        	// Bind memory buffers
        	queue.enqueueWriteBuffer(d_a, CL_TRUE, 0, bytes, in_data);
	} 

        // Bind kernel arguments to kernel
        kernel.setArg(0, d_a);
        kernel.setArg(1, d_b);
        kernel.setArg(2, w);
 
        // Number of work items in each local work group
        cl::NDRange localSize; // let underlying to choose
        // Number of total work items - localSize must be divisor
        cl::NDRange globalSize((w-2)/8, h-2);
        cl::NDRange offset(1, 1);

        // Enqueue kernel
        cl::Event event;
        queue.enqueueNDRangeKernel(
            kernel,
            offset,
            globalSize,
            localSize,
            NULL,
            &event);
        // Block until kernel completion
        event.wait();

	ocl_event_times(event, "Kernel Exec ");
 
        // Read back d_b
	if (!useHostPtr)
	        queue.enqueueReadBuffer(d_b, CL_TRUE, 0, bytes, out_data);
#ifdef __CL_ENABLE_EXCEPTIONS
    } catch (cl::Error err) {
         std::cerr
            << "ERROR: "<<err.what()<<"("<<err.err()<<")"<<std::endl;
    }
#endif
}

void
erode3x3_cl_destroy()
{
    // TODO
}
