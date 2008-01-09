
typedef unsigned int uint32;
typedef unsigned long long uint64;


typedef struct {
    uint64 image __attribute__((aligned(16)));
    uint32 width __attribute__((aligned(16)));
    uint32 height __attribute__((aligned(16)));
    float re_offset __attribute__((aligned(16)));
    float im_offset __attribute__((aligned(16)));
    float zoom __attribute__((aligned(16)));
    uint32 max_iteration __attribute__((aligned(16)));
    uint32 area_x __attribute__((aligned(16)));
    uint32 area_y __attribute__((aligned(16)));
    uint32 area_width __attribute__((aligned(16)));
    uint32 area_heigth __attribute__((aligned(16)));
    uint32 bytes_per_pixel __attribute__((aligned(16)));
} fractal_parameters;


void draw24bitMandelbrot(char *image, uint32 width, uint32 height,
                         double reOffset, double imOffset,
                         double zoom, uint32 maxIteration,
                         uint32 areaX, uint32 areaY,
                         uint32 areaWidth, uint32 areaHeight,
                         uint32 bytesPerPixel);