#include "fractal.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TRANSFER_SIZE 16384
#define TRANSFER_BUFFER_COUNT 4
//#define DRAW_DEBUG_AREAS
#define PRINT_DEBUG_INFO

unsigned int my_id;
char image_buffer[TRANSFER_BUFFER_COUNT][MAX_TRANSFER_SIZE];


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

    // Calculate how many rows it's possible to fit into one buffer.
    uint32 rows_per_buf = MAX_TRANSFER_SIZE / (p.area_width*p.bytes_per_pixel);
    // Calculate the whole amount of data for the slice to be able to process.
    uint32 rows_per_slice = rows_per_buf * TRANSFER_BUFFER_COUNT;
    // Make sure the slices and the buffers aren't too big.
    if (rows_per_slice > p.area_heigth) 
    {
      rows_per_slice = p.area_heigth;
      if (rows_per_buf > rows_per_slice)
        rows_per_buf = rows_per_slice;
    }

    printf("SPU %u whole area (x:%u,y:%u,w:%u,h:%u, "
           "horizontal slices:%u, MFC write events:%u\n", 
           my_id, p.area_x, p.area_y, 
           p.area_width, p.area_heigth,
           p.area_heigth / rows_per_slice + MIN(p.area_heigth%rows_per_slice,1),
           p.area_heigth / rows_per_buf + MIN(p.area_heigth%rows_per_buf,1));

    uint32 y;
    uint32 buffer_mask = 0;
    // Create a mask, that informs which buffers are used.
    for (y = 0; y < TRANSFER_BUFFER_COUNT; y++)
      buffer_mask |= (1<<y);
    // Loop the whole area through (a slice after another).
    for (y = 0; y < p.area_heigth; y += rows_per_slice)
    {
      uint32 height = rows_per_slice;      
      if (height > p.area_heigth - y) 
        height = p.area_heigth - y;
        
#ifdef PRINT_DEBUG_INFO        
      printf("=> SPU %u slice (x:%u,y:%u,w:%u,h:%u)\n", 
             my_id, p.area_x, y + p.area_y,
             p.area_width, height);
#endif             
       
      // Split the size of the current slice to match the size of the MFC buffers. 
      uint32 buf_y;
      for (buf_y = y; buf_y < y + height; buf_y += rows_per_buf)
      {
        uint32 buf_height = rows_per_buf;
        if (buf_height > height + y - buf_y)
          buf_height = height + y - buf_y;
          
        // Tell the MFC which buffers are tracked.
        mfc_write_tag_mask(buffer_mask);
        // Ask which buffers are ready to be used.
        uint32 buffer_ready_mask = spu_mfcstat(MFC_TAG_UPDATE_ANY);
        
        // Calculate the next free id from the mask. 
        uint32 current_buf = spu_extract(spu_sub((uint32)31, 
          spu_cntlz( spu_promote((uint32)buffer_ready_mask, 0) )), 0);
          
#ifdef PRINT_DEBUG_INFO
        printf("  => SPU %u buffer's area (x:%u,y:%u,w:%u,h:%u), MFC-mask:0x%X->%u\n",
               my_id, p.area_x, p.area_y+buf_y, p.area_width, buf_height,
               buffer_ready_mask, current_buf);
#endif              

#ifndef DRAW_DEBUG_AREAS
        drawMandelbrotArea( p.width, p.height,
                            p.re_offset, p.im_offset, p.zoom, 
                            p.max_iteration, image_buffer[current_buf],
                            p.area_x, p.area_y + buf_y,
                            p.area_width, buf_height,
                            (uint) p.bytes_per_pixel );
#else
        uint32 _x_, _y_;
        for (_y_ = 0; _y_ < buf_height; _y_++)
        {
          for (_x_ = 0; _x_ < p.area_width; _x_++)
          {
            char *ptr = image_buffer[current_buf] + 
                        p.bytes_per_pixel*(p.area_width*_y_+_x_);
            *((uint32*)ptr) = (my_id<<16)*2 + (0xFFFF / p.area_heigth)*y + 100;
          }
        }
#endif

        mfc_putb( image_buffer[current_buf],
                  p.image + (p.area_y*p.width + buf_y*p.area_width)*p.bytes_per_pixel,
	              p.area_width*p.bytes_per_pixel*buf_height,
                  current_buf, 0, 0 );
      }
    }

    // Wait for the MFC-requests to be finished.
    mfc_write_tag_mask(buffer_mask);
    spu_mfcstat(MFC_TAG_UPDATE_ALL);

    return 0;
}
