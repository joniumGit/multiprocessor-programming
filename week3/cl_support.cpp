#include <iostream>
#include <CL/opencl.hpp>
#include "image.cpp"

const float modelArray[25] = {
        1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25,
        1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25,
        1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25,
        1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25,
        1. / 25, 1. / 25, 1. / 25, 1. / 25, 1. / 25,
};


const char* kernelSource = R"(
kernel void filter(
    global uchar* in,
    global float* out,
    global float* model,
    int width,
    int height
) {
    int pos_x = get_global_id(0);
    int pos_y = get_global_id(1);
    int x;
    int y;
    for ( int i = -2; i < 3; i++ ) {
         for ( int j = -2; j < 3; j++ ) {
             x = pos_x - i;
             y = pos_y - j;
             if ( x >= 0 && x < width && y >= 0 && y < height ) {
                 out[x * height + y] += (uchar) (in[x * height + y] * model[(i + 2) * 5 + (j + 2)]);
             }
         }
    }
})";

inline void separate() {
    for (int i = 0; i < 80; i++) std::cout << std::string("-");
    std::cout << std::endl;
}

void printPlatformInfo(cl::Platform* p) {
    std::cout << "Platform: " << p->getInfo<CL_PLATFORM_NAME>() << std::endl;
    std::string platform_version = p->getInfo<CL_PLATFORM_VERSION>();
    std::cout << "Version: " << platform_version << std::endl;
    std::vector<cl::Device> devices;
    p->getDevices(CL_DEVICE_TYPE_ALL, &devices);
    std::cout << "Device Count: " << std::to_string(devices.size()) << std::endl;
    int i = 1;
    for (auto &device: devices) {
        std::string name = device.getInfo<CL_DEVICE_NAME>();
        unsigned int max_units = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        std::string cl_c_version = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
        std::string version = device.getInfo<CL_DEVICE_VERSION>();
        unsigned int max_item_dims = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
        std::string driver_version = device.getInfo<CL_DRIVER_VERSION>();
        std::cout << " - Device:                   " << std::to_string(i) << std::endl;
        std::cout << "   Name:                     " << name << std::endl;
        std::cout << "   Device Version:           " << version << std::endl;
        std::cout << "   Device Driver version:    " << driver_version << std::endl;
        std::cout << "   OpenCL C Version:         " << cl_c_version << std::endl;
        std::cout << "   Max Compute Units:        " << std::to_string(max_units) << std::endl;
        std::cout << "   Max Work Item Dimensions: " << std::to_string(max_item_dims) << std::endl;
        i++;
    }
}

void create() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    separate();
    for (auto &p: platforms) printPlatformInfo(&p);
}

Image fiveByFive(Image* image) {
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);
    auto selectedPlatform = platforms[0];

    selectedPlatform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    auto selectedDevice = devices[0];

    auto ctx = cl::Context({selectedDevice});
    cl::Program::Sources sources;
    std::string src(kernelSource);
    sources.push_back({src.c_str(), src.length()});

    cl::Program program(ctx, sources);
    if (program.build({selectedDevice}) != CL_SUCCESS) {
        std::cout << "Failed to build!" << std::endl;
        std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(selectedDevice) << std::endl;
        exit(1);
    }

    auto kernel = cl::Kernel(program, "filter");

    std::cout << "Made Kernel" << std::endl;

    cl::Buffer out(ctx, CL_MEM_READ_WRITE, sizeof(float) * image->width * image->height);
    cl::Buffer in(
            ctx,
            CL_MEM_READ_WRITE,
            sizeof(unsigned char) * image->width * image->height,
            (void*) image->data.data()
    );
    cl::Buffer model(ctx, CL_MEM_READ_WRITE, sizeof(float) * image->width * image->height, (void*) modelArray);

    std::cout << "Made Buffers" << std::endl;

    kernel.setArg(0, &in);
    kernel.setArg(1, &out);
    kernel.setArg(2, &model);
    kernel.setArg(3, image->width);
    kernel.setArg(4, image->height);

    std::cout << "Set Args" << std::endl;

    cl::CommandQueue queue(ctx, selectedDevice);
    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(image->width, image->height),
            cl::NullRange
    );

    std::cout << "Queued Kernel" << std::endl;

    std::vector<float> newData(image->data.size());
    queue.enqueueReadBuffer(out, CL_TRUE, 0, newData.size(), (void*) newData.data());

    std::cout << "Copied Result: " << std::to_string(newData.size()) << std::endl;
    std::vector<unsigned char> outData(image->data.size());
    for (auto value: newData) {
        outData.push_back((unsigned char) value);
    }

    return Image(image, &outData);
}