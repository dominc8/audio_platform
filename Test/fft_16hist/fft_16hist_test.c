#include "fft_hist.h"
#include <stdio.h>

#define X_SIZE 64
#define Y_SIZE 16

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

void print_array(int16_t *arr, int32_t size)
{
    int32_t i;
    for (i = 0; i < size; ++i)
    {
        printf("%d, ", arr[i]);
    }
    printf("\n");
}



int main(void)
{
    int16_t x[X_SIZE];
    int16_t yl_ref[Y_SIZE];
    int16_t yr_ref[Y_SIZE];
    int16_t yl[Y_SIZE];
    int16_t yr[Y_SIZE];

    load_array(x, sizeof(*x), X_SIZE, "x.dat");
    printf("x:\n");
    print_array(x, X_SIZE);

    load_array(yl_ref, sizeof(*yl_ref), Y_SIZE, "yl.dat");
    printf("yl_ref:\n");
    print_array(yl_ref, Y_SIZE);

    load_array(yr_ref, sizeof(*yr_ref), Y_SIZE, "yr.dat");
    printf("yr_ref:\n");
    print_array(yr_ref, Y_SIZE);

    fft_16hist(yl, yr, x);

    printf("yl:\n");
    print_array(yl, Y_SIZE);

    printf("yr:\n");
    print_array(yr, Y_SIZE);

}

