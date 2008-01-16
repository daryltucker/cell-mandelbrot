### Mandelbrotin fraktaalin piirtävä funktio

.data

.align 4

.equ LR_OFFSET, 16
# .equ FRAME_SIZE, 

asm_mandelbrot:	
	## Prologi
	stqd $lr, LR_OFFSET($sp)
	stqd $sp, FRAME_SIZE($sp)
	ai $sp, $sp, -FRAME_SIZE

## void drawMandelbrotArea(
## $3    <-- uint32 width
## $4    <-- uint32 height
## $5    <-- double reOffset
## $6    <-- double imOffset
## $7    <-- double zoom
## $8    <-- uint32 maxIteration
## $9    <-- char *areaBuffer
## $10   <-- uint32 areaX
## $11   <-- uint32 areaY
## $12   <-- uint32 areaWidth
## $13   <-- uint32 areaHeight
## $14   <-- uint32 bytesPerPixel
## )

## {
##   double 
##        $14  <-- x0,
##        $15  <-- y0,
##        $16  <-- x,
##        $17  <-- y,
##        $18  <-- xTemp,
##        $19  <-- yTemp;
##   double 
##        $20  <-- scale,
##        $21  <-- mandSize,
##        $22  <-- offsetX,
##        $23  <-- offsetY;
##   unsigned int 
##        $24  <-- i,
##        $25  <-- j,
##        $26  <-- iteration,
##        $27  <-- color,
##        $28  <-- maxColor;
##   signed int 
##        $29  <-- k;
##   char *
##        $30  <-- line;

##   mandSize = MANDELBROT_DEFAULT_SIZE / zoom;
##   scale = mandSize / MIN(width, height);
##   offsetX = mandSize / -2.0 * (width > height ? (double)width/height : 1.0)
##             + reOffset;
##   offsetY = mandSize / -2.0 * (width < height ? (double)height/width : 1.0)
##             + imOffset;

##   for (j = areaY; j < areaHeight + areaY; j++)
##   {
##     line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
##     for (i = areaX; i < areaWidth + areaX; i++)
##     {
##       x0 = i * scale + offsetX;
##       y0 = j * scale + offsetY;
##       x = x0;
##       y = y0;
##       iteration = 0;

##       while (x*x + y*y < MANDELBROT_DEFAULT_SIZE && iteration < maxIteration)
##       {
##         xTemp = x*x - y*y + x0;
##         yTemp = 2*x*y + y0;
##         x = xTemp;
##         y = yTemp;
##         iteration++;
##       }

##       maxColor = 0;
##       for (k = 0; k < bytesPerPixel; k++)
##         maxColor = (maxColor << 8) + 0xFF;
##       color = (unsigned int)((double)(iteration)/maxIteration * maxColor);
##       if (iteration == maxIteration)
##         color = 0;
##       for (k = bytesPerPixel - 1; k >= 0; k--)
##         *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
##           (color & ((uint32)0xFF << (8*k))) >> (8*k);
##     }
##   }
## }
	
	## Epilogi
	ai $sp, $sp, FACT_FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
