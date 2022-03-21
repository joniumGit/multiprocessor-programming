#pragma once

#include <iostream>
#include <CL/opencl.hpp>
#include "zncc.cpp"


const std::string kernelSource = R"(
__kernel void filter(
    __global uchar* left,
    __global uchar* right,
    __global int* out1,
    __global int* out2,
    int window,
    int disparity_max,
    int width
) {
    int x = get_global_id(1);
    int y = get_global_id(0);
    int index = (y * width) + x;
    int bestDisparity;
    float topZncc;

    topZncc = 0;
    bestDisparity = 0;
    for (int d = 0; d < disparity_max; d++) {
        if (x + d + window >= width) continue;
        if (x + d - window < 0) continue;

        float avgL = 0;
        float avgR = 0;

        for (int j = -window; j < window + 1; j++) {
            avgL += left[index + j] / (float) window;
            avgR += right[index + j + d] / (float) window;
        }

        float top_sum = 0;
        float bot_sum_l = 0;
        float bot_sum_r = 0;

        for (int j = -window; j < window + 1; j++) {
            float valL = left[index + j] - avgL;
            float valR = right[index + j + d] - avgR;
            top_sum += valL * valR;
            bot_sum_l += valL * valL;
            bot_sum_r += valR * valR;
        }

        bot_sum_l = sqrt((float) bot_sum_l);
        bot_sum_r = sqrt((float) bot_sum_r);

        float zncc = top_sum / (bot_sum_l * bot_sum_r);

        if (zncc > topZncc) {
            topZncc = zncc;
            bestDisparity = d;
        }
    }

    out1[index] = (int) bestDisparity;

    topZncc = 0;
    bestDisparity = 0;
    for (int d = 0; d < disparity_max; d++) {
        if (x - d + window >= width) continue;
        if (x - d - window < 0) continue;

        float avgL = 0;
        float avgR = 0;

        for (int j = -window; j < window + 1; j++) {
            avgL += right[index + j] / (float) window;
            avgR += left[index + j - d] / (float) window;
        }

        float top_sum = 0;
        float bot_sum_l = 0;
        float bot_sum_r = 0;

        for (int j = -window; j < window + 1; j++) {
            float valL = right[index + j] - avgL;
            float valR = left[index + j - d] - avgR;
            top_sum += valL * valR;
            bot_sum_l += valL * valL;
            bot_sum_r += valR * valR;
        }

        bot_sum_l = sqrt((float) bot_sum_l);
        bot_sum_r = sqrt((float) bot_sum_r);

        float zncc = top_sum / (bot_sum_l * bot_sum_r);

        if (zncc > topZncc) {
            topZncc = zncc;
            bestDisparity = d;
        }
    }

    out2[index] = (int) bestDisparity;
}
)";


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
        std::cout << "   Max Work Item Sizes:      ";
        int j = 0;
        for (auto &value: device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()) {
            if (j != 0) {
                std::cout << ",";
            }
            j++;
            std::cout << std::to_string(value);
        }
        std::cout << std::endl;
        i++;
    }
}

void inspect_context() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    for (auto &p: platforms) printPlatformInfo(&p);
}


std::tuple<Image, Image> doZNCC(Image* left, Image* right, int window, int max_disparity) {
    int width = (int) left->width;
    int height = (int) left->height;
    int size = width * height;
    int err;

    if (sizeof(unsigned char) != sizeof(cl_uchar)) {
        std::cout << "Bad Char Size" << std::endl;
        exit(1);
    }

    if (sizeof(int) != sizeof(cl_int)) {
        std::cout << "Bad Int Size" << std::endl;
        exit(1);
    }

    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);
    auto selectedPlatform = platforms[0];

    selectedPlatform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    auto selectedDevice = devices[0];

    auto ctx = cl::Context({selectedDevice});
    cl::Program::Sources sources;
    sources.push_back({kernelSource.c_str(), kernelSource.length()});

    cl::Program program(ctx, sources);
    if (program.build({selectedDevice}) != CL_SUCCESS) {
        std::cout << "Failed to build!" << std::endl;
        std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(selectedDevice) << std::endl;
        exit(1);
    }

    auto kernel = cl::Kernel(program, "filter");
    std::cout << "Made Kernel" << std::endl;

    cl::Buffer out1Data(
            ctx,
            CL_MEM_WRITE_ONLY,
            sizeof(cl_int) * size,
            nullptr,
            &err
    );
    if (err != CL_SUCCESS) {
        std::cout << "Failed to allocate: " << err << std::endl;
        exit(1);
    }
    cl::Buffer out2Data(
            ctx,
            CL_MEM_WRITE_ONLY,
            sizeof(cl_int) * size,
            nullptr,
            &err
    );
    if (err != CL_SUCCESS) {
        std::cout << "Failed to allocate: " << err << std::endl;
        exit(1);
    }
    cl::Buffer leftData(
            ctx,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_uchar) * size,
            (void*) &left->data[0],
            &err
    );
    if (err != CL_SUCCESS) {
        std::cout << "Failed to write: " << err << std::endl;
        exit(1);
    }
    cl::Buffer rightData(
            ctx,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_uchar) * size,
            (void*) &right->data[0],
            &err
    );
    if (err != CL_SUCCESS) {
        std::cout << "Failed to write: " << err << std::endl;
        exit(1);
    }

    std::cout << "Made Buffers" << std::endl;

    err = kernel.setArg(0, leftData);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg0: " << err << std::endl;
    err = kernel.setArg(1, rightData);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg:1 " << err << std::endl;
    err = kernel.setArg(2, out1Data);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg2: " << err << std::endl;
    err = kernel.setArg(3, out2Data);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg3: " << err << std::endl;
    err = kernel.setArg(4, window);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg4: " << err << std::endl;
    err = kernel.setArg(5, max_disparity);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg5: " << err << std::endl;
    err = kernel.setArg(6, width);
    if (err != CL_SUCCESS) std::cout << "Failed to set arg6: " << err << std::endl;
    std::cout << "Set Args" << std::endl;

    cl::CommandQueue queue(ctx, selectedDevice);
    err = queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(height, width),
            cl::NDRange(5, 2)
    );
    if (err != CL_SUCCESS) {
        std::cout << "Failed to queue: " << err << std::endl;
        std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(selectedDevice) << std::endl;
        exit(1);
    }

    std::cout << "Queued Kernel" << std::endl;

    auto new1Data = new cl_int[size];
    auto new2Data = new cl_int[size];

    err = queue.enqueueReadBuffer(out1Data, CL_TRUE, 0, sizeof(cl_int) * size, (void*) &new1Data[0]);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to read: " << err << std::endl;
        exit(1);
    }
    err = queue.enqueueReadBuffer(out2Data, CL_TRUE, 0, sizeof(cl_int) * size, (void*) &new2Data[0]);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to read: " << err << std::endl;
        exit(1);
    }

    std::vector<double> outLeftData(size);
    std::vector<double> outRightData(size);
    bool nonZero;

    for (int i = 0; i < size; i++) {
        if (!new1Data[i] || !new2Data[i]) nonZero = true;
        outLeftData[i] = (double) new1Data[i];
        outRightData[i] = (double) new2Data[i];
    }

    free(new1Data);
    free(new2Data);

    if (!nonZero) {
        std::cout << "Failed to produce non-zero values!" << err << std::endl;
        exit(1);
    }

    auto data1 = handleDisparity(outLeftData, width, height);
    auto data2 = handleDisparity(outRightData, width, height);

    return {data1, data2};
}