#include "fractal.h"

// Define the size of the mandelbrot area, ie. 4 -> x e[-2,2], y e[-2,2].
#define MANDELBROT_DEFAULT_SIZE 4.0
#define MIN(_A_, _B_) ((_A_ < _B_) ? _A_ : _B_)


void drawMandelbrotArea( uint32 width, uint32 height,
                         double reOffset, double imOffset,
                         double zoom, uint32 maxIteration,
                         char *areaBuffer,
                         uint32 areaX, uint32 areaY,
                         uint32 areaWidth, uint32 areaHeight,
                         uint32 bytesPerPixel )
{
  double x0, y0, x, y, xTemp, yTemp;
  double scale, mandSize, offsetX, offsetY;
  unsigned int i, j, iteration, color, maxColor;
  signed int k;
  char *line;

  mandSize = MANDELBROT_DEFAULT_SIZE / zoom;
  scale = mandSize / MIN(width, height);
  offsetX = mandSize / -2.0 * (width > height ? (double)width/height : 1.0)
            + reOffset;
  offsetY = mandSize / -2.0 * (width < height ? (double)height/width : 1.0)
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
      color = (unsigned int)((double)(iteration)/maxIteration * maxColor);
      if (iteration == maxIteration)
        color = 0;
      for (k = bytesPerPixel - 1; k >= 0; k--)
        *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
          (color & ((uint32)0xFF << (8*k))) >> (8*k);
    }
  }
}

