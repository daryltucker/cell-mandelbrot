/*
 * Testifunktio, joka piirt‰‰ yksiv‰risen kuvan. T‰ll‰ n‰kee ett‰
 * mihin kohti s‰ikeet piirt‰v‰t.
 *
 * TODO: Eri v‰rit eri s‰ikeille.
 */ 

#include "fractal.h"


void drawMandelbrotArea( uint32 width, uint32 height,
                         float reOffset, float imOffset,
                         float zoom, uint32 maxIteration,
                         char *areaBuffer,
                         uint32 areaX, uint32 areaY,
                         uint32 areaWidth, uint32 areaHeight,
                         uint32 bytesPerPixel )
{
  unsigned int i, j;
  signed int k;
  char *line;

  for (j = areaY; j < areaHeight + areaY; j++)
  {
    line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
    for (i = areaX; i < areaWidth + areaX; i++)
    {
      for (k = bytesPerPixel - 1; k >= 0; k--)
        *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
            0xFF;
            //(color & ((uint32)0xFF << (8*k))) >> (8*k);
    }
  }
}

