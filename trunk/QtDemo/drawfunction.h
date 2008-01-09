
//! A function that will draw a mandelbrot on a given buffer.
/*! This function will draw a specified area of the mandelbrot fractal
    to a given image. The image can also be given as portions of the 
    whole image. This will provide a convenient way to draw the whole 
    image in parallel.
   \param width The width of the whole image.
   \param height The height of the whole image.
   \param reOffset The offset in the real-axes on a complex plane.
   \param imOffset The offset in the imaginary-axes on a complex plane.
   \param zoom The level of the magnification.
   \param maxIteration The maximum count of iterations performed on 
    each pixel.
   \param areaBuffer A pointer to the beginning of the drawable area.
   \param areaX The horizontal starting point on the whole image.
   \param areaY The vertical starting point on the whole image.
   \param areaWidth The width of the drawable area.
   \param areaHeight The height of the drawable area.
   \bytesPerPixel The amount of bytes reserved for one pixel.
 */
extern "C" void drawMandelbrotArea( uint width, uint height,
                                    double reOffset, double imOffset,
                                    double zoom, uint maxIteration,
                                    char *areaBuffer,
                                    uint areaX, uint areaY,
                                    uint areaWidth, uint areaHeight,
                                    uint bytesPerPixel );

