#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TRANSFER_SIZE 16384


unsigned int my_id;
char image_buffer[MAX_TRANSFER_SIZE];


int main(uint64 spe_id, uint64 fractal_parameter_ea)
{
    fractal_parameters p;
    uint32 message;

    my_id = (unsigned int) spe_id;

    // Odotetaan lähtölupaa
    message = spu_readch(SPU_RdInMbox);

    // Ladataan parametrit:
    mfc_write_tag_mask(1<<0);  //DMA-tunniste = 0
    mfc_get(&p, fractal_parameter_ea, sizeof(fractal_parameters), 0, 0, 0);
    spu_mfcstat(MFC_TAG_UPDATE_ALL); // Ja odotellaan niitä.

    // Make sure that one horizontal slice isn't greater that the MFC limit.
    if (p.area_width*p.bytes_per_pixel > MAX_TRANSFER_SIZE) 
    {
      printf("SPU %u failed, because horizontal size is too large (%u)\n", 
             my_id, p.area_width);
      return -1;
    }

    // Calculate how many rows it's possible to copy at once.
    uint32 h_slice_size = MAX_TRANSFER_SIZE / (p.area_width*p.bytes_per_pixel);
    if (h_slice_size > p.area_heigth)
      h_slice_size = p.area_heigth;

    printf("SPU %u: area_width: %u, area_height: %u, "
           "area_x: %u, area_y: %u, horizontal slices: %u\n", my_id,
           p.area_width, p.area_heigth,
           p.area_x, p.area_y,
           p.area_heigth / h_slice_size);

    uint32 y = 0;
    for (y = 0; y < p.area_heigth; y += h_slice_size)
    {
      uint32 height = h_slice_size;      
      if (height > p.area_heigth - y) 
        height = p.area_heigth - y;

      drawMandelbrotArea( p.width, p.height,
                          p.re_offset, p.im_offset, p.zoom, 
                          p.max_iteration, image_buffer,
                          p.area_x, p.area_y + y,
                          p.area_width, height,
                          (uint) p.bytes_per_pixel );

      mfc_put( image_buffer,
               p.image + (p.area_y*p.width + y*p.area_width)*p.bytes_per_pixel,
	       p.area_width*p.bytes_per_pixel*height,
               0, 0, 0 );

      // Odotellaan kaikki siirrot valmiiksi, varmuuden vuoksi.
      spu_mfcstat(MFC_TAG_UPDATE_ALL);
    }
    return 0;
}
