#include <stdlib.h>

typedef struct {
    char *image __attribute__((aligned(16)));
    uint width __attribute__((aligned(16)));
    uint height __attribute__((aligned(16)));
    float re_offset __attribute__((aligned(16)));
    float im_offset __attribute__((aligned(16)));
    float zoom __attribute__((aligned(16)));
    uint max_iteration __attribute__((aligned(16)));
    uint area_x __attribute__((aligned(16)));
    uint area_y __attribute__((aligned(16)));
    uint area_width __attribute__((aligned(16)));
    uint area_heigth __attribute__((aligned(16)));
    uint bytes_per_pixel __attribute__((aligned(16)));
} fractal_parameters;
