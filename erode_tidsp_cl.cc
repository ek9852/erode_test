// Using TI C66x optimizated OpenCL kernel

#define __CL_ENABLE_EXCEPTIONS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <assert.h>
#include <string.h>
 
#include "cl.hpp"

// Only include source code if TI OpenCL extension is found
#ifdef CL_MEM_USE_MSMC_TI

#include "erode_tidsp_cl.h"

static const char *kernelSource =
#include "erode3x3_tidsp.cl"
;

static cl::Kernel kernel;
static cl::CommandQueue queue;
static cl::Buffer d_a;
static cl::Buffer d_b;
static cl::Context context;
static bool useHostPtr;
static int NUMCOMPUNITS;

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
erode3x3_tidsp_cl_init(int w, int h, bool use_host_ptr)
{
    size_t bytes = w * h;
    cl_int err = CL_SUCCESS;

    assert(w % 8 == 0);
    assert(w < 2046);
    assert(h > 2);

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
        context = cl::Context(CL_DEVICE_TYPE_ALL, properties);
        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        std::cout << "Device size " << devices.size() << std::endl;

        cl::Device device  = devices[0];

        std::cout << "Device Name: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;  
        std::cout << "Device Version: " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;  
        std::cout << "Device Type: " << device.getInfo<CL_DEVICE_TYPE>();
        std::cout << " (GPU: " << CL_DEVICE_TYPE_GPU << ", CPU: " << CL_DEVICE_TYPE_CPU << ", ACCEL: " << CL_DEVICE_TYPE_ACCELERATOR << ")" << std::endl;  
        std::cout << "Device Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;

        device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);
        std::cout << "Device Max Compute Units: " << NUMCOMPUNITS << std::endl;
        std::cout << "Device Global Memory: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << std::endl;
        std::cout << "Device Max Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << std::endl;
        std::cout << "Device Max Allocateable Memory: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << std::endl;
        std::cout << "Device Local Memory: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << std::endl;
        cl_ulong msmc_mem   = 0;
        device.getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI,  &msmc_mem);
        std::cout << "Device TI MSMC Memory: " << msmc_mem << std::endl;
        std::cout << "Device max constant buffer size: " << device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() << std::endl;
        std::cout << "Device max work item dims: " << device.getInfo< CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << std::endl;
        std::cout << "Device max work group size: " << device.getInfo< CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
        std::vector<size_t> device_max_work_item_sizes = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
        std::cout << "Max work item sizes are: ";
        for (std::vector<size_t>::const_iterator i = device_max_work_item_sizes.begin(); i != device_max_work_item_sizes.end(); ++i)
            std::cout << *i << ' ';
        std::cout << std::endl;
        std::cout << "Device Available: " << device.getInfo< CL_DEVICE_AVAILABLE>() << std::endl;
        std::cout << "Device Extensions: " << device.getInfo< CL_DEVICE_EXTENSIONS>() << std::endl;
 
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
        kernel = cl::Kernel(program_, "tidsp_morph_erode", &err);
 
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
erode3x3_tidsp_cl(uint8_t *in_data, uint8_t *out_data, int w, int h)
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
        kernel.setArg(3, h);
 
        // Number of work items in each local work group
        cl::NDRange localSize(1);
        // Number of total work items - localSize must be divisor
        cl::NDRange globalSize(NUMCOMPUNITS);

        // Enqueue kernel
        cl::Event event;
        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            globalSize,
            localSize,
            NULL,
            &event);
        // Block until kernel completion
        event.wait();

	ocl_event_times(event, "Kernel Exec ");
 
        // Read back d_b (it turns out that even with use host ptr, we still need this
        // at least on Adreno and Mali GPU)
	queue.enqueueReadBuffer(d_b, CL_TRUE, 0, bytes, out_data);
#ifdef __CL_ENABLE_EXCEPTIONS
    } catch (cl::Error err) {
         std::cerr
            << "ERROR: "<<err.what()<<"("<<err.err()<<")"<<std::endl;
    }
#endif
}

void
erode3x3_tidsp_cl_destroy()
{
    // TODO
}
#endif // CL_MEM_USE_MSMC_TI
