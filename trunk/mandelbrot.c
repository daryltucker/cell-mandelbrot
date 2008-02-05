#include "fractal.h"


void drawMandelbrotArea( uint32 width, uint32 height,
                         float reOffset, float imOffset,
                         float zoom, uint32 maxIteration,
                         char *areaBuffer,
                         uint32 areaX, uint32 areaY,
                         uint32 areaWidth, uint32 areaHeight,
                         uint32 bytesPerPixel )
{
  float x0, y0, x, y, xTemp, yTemp;
  float scale, mandSize, offsetX, offsetY;
  unsigned int i, j, iteration, color, maxColor;
  signed int k;
  char *line;

  mandSize = MANDELBROT_DEFAULT_SIZE / zoom;
  scale = mandSize / MIN(width, height);
  offsetX = mandSize / -2.0f * (width > height ? (float)width/height : 1.0f)
            + reOffset;
  offsetY = mandSize / -2.0f * (width < height ? (float)height/width : 1.0f)
            + imOffset;

  for (j = areaY; j < areaHeight + areaY; j++)
  {
    line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
    for (i = areaX; i < areaWidth + areaX; i++)
    {
      x0 = i * scale + offsetX;
      y0 = j * scale + offsetY;
      x = x0;
      y = y0;
      iteration = 0;

      while (x*x + y*y < MANDELBROT_DEFAULT_SIZE && iteration < maxIteration)
      {
        xTemp = x*x - y*y + x0;
        yTemp = 2*x*y + y0;
        x = xTemp;
        y = yTemp;
        iteration++;
      }

      maxColor = 0;
      for (k = 0; k < bytesPerPixel; k++)
        maxColor = (maxColor << 8) + 0xFF;
      color = (unsigned int)((float)(iteration)/maxIteration * maxColor);
      if (iteration == maxIteration)
        color = 0;
      for (k = bytesPerPixel - 1; k >= 0; k--)
        *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
          (color & ((uint32)0xFF << (8*k))) >> (8*k);
    }
  }
}

