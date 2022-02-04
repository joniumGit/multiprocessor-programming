#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MATRIX_SIZE 100 * 100

#include <CL/cl.h>
#include "addcl.c"

void addMatrix2(const cl_int* m1, const cl_int* m2, cl_int* result) {
    for (cl_int i = 0; i < MATRIX_SIZE; i++) {
        *(result + i) = *(m1 + i) + *(m2 + i);
    }
}

void fillMatrix(cl_int* arr) {
    for (cl_int i = 0; i < MATRIX_SIZE; i++) {
        *(arr + i) = rand();
    }
}

double testRun(void* adder(), void* ptr) {
    cl_int* matrix_a = calloc(MATRIX_SIZE, sizeof(cl_int));
    cl_int* matrix_b = calloc(MATRIX_SIZE, sizeof(cl_int));
    fillMatrix(matrix_a);
    fillMatrix(matrix_b);

    clock_t start, end;
    start = clock();
    if (ptr != NULL) {
        (*adder)(matrix_a, matrix_b, matrix_a, ptr);
    } else {
        (*adder)(matrix_a, matrix_b, matrix_a);
    }
    end = clock();

    free(matrix_a);
    free(matrix_b);
    return (double) (end - start) / CLOCKS_PER_SEC;
}

cl_int main() {
    CLData data = create_kernel();

    char* strings[] = {"OpenCL Took %f ms", "C(i) Took %f ms"};
    void* calls[] = {(void*) &addMatrixOpenCL, (void*) &addMatrix2};
    void* args[] = {&data, NULL};

    for (int i = 0; i < 2; i++) {
        srand(0); // Seed for same result
        int j = 0;
        double result = testRun(calls[i], args[i]);
        j++;
        do {
            result = (testRun(calls[i], args[i]) + result) / 2;
            j++;
        } while (j < 10000);
        printf(strings[i], result * 1000);
        printf("\n");
    }

    return 0;
}