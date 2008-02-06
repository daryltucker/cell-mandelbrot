### Mandelbrotin fraktaalin piirt‰v‰ funktio
###
### T‰ss‰ k‰ytet‰‰n aina float-arvoja double-arvojen sijaan.

.data
	
.align 4
mandelbrot_default_size:	
	.float, 0f4.0
.align 4
zero:
	.float 0.0
.align 4
lol:	
	.ascii "lol \0"

.equ LR_OFFSET, 16
.equ FRAME_SIZE, 32		# Ei pinomuuttujia


.global drawMandelbrotArea
.type drawMandelbrotArea,@function
drawMandelbrotArea:	
	## Prologi
	stqd $lr, LR_OFFSET($sp)
	stqd $sp, FRAME_SIZE($sp)
	ai $sp, $sp, -FRAME_SIZE

## void drawMandelbrotArea(
## $3    <-- uint32 width
## $4    <-- uint32 height
## $5    <-- float reOffset
## $6    <-- float imOffset
## $7    <-- float zoom
## $8    <-- uint32 maxIteration
## $9    <-- char *areaBuffer
## $10   <-- uint32 areaX -- Ei k‰ytet‰, arvo 0
## $11   <-- uint32 areaY
## $12   <-- uint32 areaWidth -- Ei k‰ytet‰, arvo sama kuin width
## $13   <-- uint32 areaHeight
## $14   <-- uint32 bytesPerPixel
## )

## {
##   float 
##        $80  <-- x0,
##        $81  <-- y0,
##        $82  <-- x,
##        $83  <-- y,
##        $84  <-- xTemp,
##        $85  <-- yTemp;
##   float 
##        $86  <-- scale,
##        $87  <-- mandSize,
##        $88  <-- offsetX,
##        $89  <-- offsetY;
##   unsigned int 
##        $90  <-- i,
##        $91  <-- j,
##        $92  <-- iteration,
##        $93  <-- color,
##        $94  <-- maxColor;
##   signed int 
##        $95  <-- k;
##   char *
##        $96  <-- line;

##   mandSize = MANDELBROT_DEFAULT_SIZE / zoom;
	frest $97, $7		# r97 = 1.0f / r7;
	fi $97, $7, $97		# tarkennetaan k‰‰nteislukua
	lqr $98, mandelbrot_default_size
	fm $87, $98, $97		# r87 = r98 * r97

##   scale = mandSize / MIN(width, height);
	cgt $98, $3, $4
	## T‰h‰n varmaan on elegantimpikin ratkaisu...
	brz $98, width_min
	## Okei, $98:iin tulee minimi
height_min:	
	lr $98, $4
	br min_done
width_min:
	lr $98, $3
min_done:
	## 1/min $99:iin
	frest $99, $98
	fi $99, $98, $99
	fm $100, $87, $99	## r100 = r87 * r99

	## Kerrotaan mit‰ on saatu t‰h‰n asti aikaiseksi
	wrch SPU_WrOutMbox, $100

##   offsetX = mandSize / -2.0 * (width > height ? (float)width/height : 1.0)
##             + reOffset;
##   offsetY = mandSize / -2.0 * (width < height ? (float)height/width : 1.0)
##             + imOffset;

	## Olkoon nyt offsetX ja offsetY 0.0
	lqr $88, zero 		# Ei tietenk‰‰n ila, vaan se arvo pit‰‰ ladata
	lqr $89, zero
	
##   for (j = areaY;
	a $101, $13, $11	# Lasketaan (areaHeight + areaY) etuk‰teen
	lr $102, $11

outer_loop:	
##  areaHeight + areaY > j; j++)
	cgt $103, $101, $102
	## t‰h‰n vinkki ett‰ tod.n‰k tosi
	brz $103, finish
	
##   {
	## T‰ss‰ vois jotain tulostella kokeeksi
	## mutta pit‰is panna parametrit talteen ennen aliohjelman kutsua

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
##       color = (unsigned int)((float)(iteration)/maxIteration * maxColor);
##       if (iteration == maxIteration)
##         color = 0;
##       for (k = bytesPerPixel - 1; k >= 0; k--)
##         *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
##           (color & ((uint32)0xFF << (8*k))) >> (8*k);
##     }
	
	ai $102, $102, 1	# j:n kasvatus
##   }
finish:	
## }
	
	## Epilogi
	ai $sp, $sp, FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
