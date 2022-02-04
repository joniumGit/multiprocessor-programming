#include <CL/cl.h>

const char* kernelSource = "kernel void add_matrix(\n"
                           "    global int* a,\n"
                           "    global int* b,\n"
                           "    global int* out\n"
                           ") {\n"
                           "    int i = get_global_id(0);"
                           "    out[i] = a[i] + b[i];\n"
                           "}";

typedef struct DataStore {
    cl_kernel kernel;
    cl_context ctx;
    cl_command_queue queue;
    cl_mem a;
    cl_mem b;
    cl_mem out;
    size_t size;
} CLData;

CLData create_kernel() {
    cl_int err = 0;
    cl_device_id device_id;
    cl_platform_id platform_id;
    cl_program program;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_mem a;
    cl_mem b;
    cl_mem out;

    clGetPlatformIDs(1, &platform_id, NULL);
    clGetDeviceIDs(
            platform_id,
            CL_DEVICE_TYPE_DEFAULT,
            1,
            &device_id,
            NULL
    );
    cl_context context = clCreateContext(
            0,
            1,
            &device_id,
            NULL,
            NULL,
            &err
    );
    if (err != CL_SUCCESS) {
        printf("Failed to create context\n");
        exit(1);
    }


    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &err);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to build\n");
        exit(1);
    }

    kernel = clCreateKernel(program, "add_matrix", &err);
    if (err != CL_SUCCESS) {
        printf("Failed to create kernel\n");
        exit(1);
    }

    queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &err);
    if (err != CL_SUCCESS) {
        printf("Failed to create queue\n");
        exit(1);
    }

    size_t size = sizeof(cl_int) * MATRIX_SIZE;

    out = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE,
            size,
            NULL,
            &err
    );

    a = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE,
            size,
            NULL,
            &err
    );

    b = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE,
            size,
            NULL,
            &err
    );

    if (err != CL_SUCCESS) {
        printf("Failed to make buffers\n");
        exit(1);
    }

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &a);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &b);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &out);

    if (err != CL_SUCCESS) {
        printf("Failed to set args\n");
        exit(1);
    }

    size_t valueSize;
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &valueSize);
    char* value = (char*) malloc(valueSize);
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, valueSize, value, NULL);

    printf(value);
    printf("\n");

    CLData data = {.kernel = kernel, .ctx = context, .queue = queue, .a = a, .b = b, .out = out, .size = size};
    return data;
}

void addMatrixOpenCL(const cl_int* m1, const cl_int* m2, cl_int* result, CLData* cfg) {
    cl_int err;
    size_t localWorkSize[1], globalWorkSize[1];
    localWorkSize[0] = 100;
    globalWorkSize[0] = MATRIX_SIZE;
    cl_event done[2];

    err = clEnqueueWriteBuffer(
            cfg->queue,
            cfg->a,
            CL_FALSE,
            0,
            cfg->size,
            m1,
            0,
            NULL,
            &done[0]
    );

    err |= clEnqueueWriteBuffer(
            cfg->queue,
            cfg->a,
            CL_FALSE,
            0,
            cfg->size,
            m2,
            0,
            NULL,
            &done[1]
    );

    err |= clEnqueueNDRangeKernel(
            cfg->queue,
            cfg->kernel,
            1,
            NULL,
            globalWorkSize,
            localWorkSize,
            2,
            done,
            NULL
    );

    if (err != CL_SUCCESS) {
        printf("Failed to copy data and enqueue kernel\n");
        exit(1);
    }

    err = clEnqueueReadBuffer(
            cfg->queue,
            cfg->out,
            CL_TRUE,
            0,
            cfg->size,
            result,
            0,
            NULL,
            NULL
    );

    if (err != CL_SUCCESS) {
        printf("Failed to read result\n");
        exit(1);
    }

}