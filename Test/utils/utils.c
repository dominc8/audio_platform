#include "utils.h"
#include <stdio.h>

int32_t load_array(void *arr, int32_t el_size, int32_t n, const char* filename)
{
    FILE *f = fopen(filename, "rb");
    if (NULL == f)
        return 0;
    int32_t n_read = fread(arr, el_size, n, f);
    fclose(f);
    return n_read;
}

void print_farray(float *arr, int32_t size)
{
    int32_t i;
    for (i = 0; i < size; ++i)
    {
        printf("%f, ", arr[i]);
    }
    printf("\n");
}

void print_i16array(int16_t *arr, int32_t size)
{
    int32_t i;
    for (i = 0; i < size; ++i)
    {
        printf("%d, ", arr[i]);
    }
    printf("\n");
}

