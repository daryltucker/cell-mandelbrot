### Mandelbrotin fraktaalin piirtävä funktio.

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
.equ BYTES_PER_PIXEL, 4

### Rekistereitä
.equ IMG_PTR, 31

.equ X_LOOP_COUNTER, 39
.equ Y_LOOP_COUNTER, 37
.equ TEMP, 75
.equ TEMP_PTR, 76


.text

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
## $10   <-- uint32 areaX -- Ei käytetä, arvo 0
## $11   <-- uint32 areaY
## $12   <-- uint32 areaWidth -- Ei käytetä, arvo sama kuin width
## $13   <-- uint32 areaHeight
## $14   <-- uint32 bytesPerPixel  -- Ei käytetä, arvo BYTES_PER_PIXEL
## )

## {
##   float 
##        $15  <-- x0,
##        $16  <-- y0,
##        $17  <-- x,
##        $18  <-- y,
##        $19  <-- xTemp,
##        $20  <-- yTemp;
##   float 
##        $21  <-- scale,
##        $22  <-- mandSize,
##        $23  <-- offsetX,
##        $24  <-- offsetY;
##   unsigned int 
##        $25  <-- i,
##        $26  <-- j,
##        $27  <-- iteration,
##        $28  <-- color,
##        $29  <-- maxColor;
##   signed int 
##        $30  <-- k;
##   char *
##        $31  <-- line;

##   mandSize = MANDELBROT_DEFAULT_SIZE / zoom;
	frest $32, $7		# r32 = 1.0f / r7;
	fi $32, $7, $32		# tarkennetaan käänteislukua
	lqr $33, mandelbrot_default_size
	fm $22, $33, $32		# r22 = r33 * r32

##   scale = mandSize / MIN(width, height);
	cgt $33, $3, $4
	## Tähän varmaan on elegantimpikin ratkaisu...
	brz $33, width_min
	## Okei, $33:iin tulee minimi
height_min:	
	lr $33, $4
	br min_done
width_min:
	lr $33, $3
min_done:
	## 1/min $34:iin
	frest $34, $33
	fi $34, $33, $34
	fm $35, $22, $34	## r35 = r22 * r34

	## Kerrotaan mitä on saatu tähän asti aikaiseksi
	## wrch SPU_WrOutMbox, $35

##   offsetX = mandSize / -2.0 * (width > height ? (float)width/height : 1.0)
##             + reOffset;
##   offsetY = mandSize / -2.0 * (width < height ? (float)height/width : 1.0)
##             + imOffset;

	## Olkoon nyt offsetX ja offsetY 0.0
	lqr $23, zero 		# Ei tietenkään ila, vaan se arvo pitää ladata
	lqr $24, zero
	
##   for (j = areaY;
	a $36, $13, $11		# Lasketaan (areaHeight + areaY) etukäteen
	lr $Y_LOOP_COUNTER, $11

y_loop:	
##  areaHeight + areaY > j; j++)
	cgt $TEMP, $36, $Y_LOOP_COUNTER
	## tähän vinkki että tod.näk tosi
	brz $TEMP, finish_y_loop
	
##   {
	## Tässä vois jotain tulostella kokeeksi
	## mutta pitäis panna parametrit talteen ennen aliohjelman kutsua

##     line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
	# line == $31
	# areaY $11
	# sitten j - areaY
	sf $38, $11, $Y_LOOP_COUNTER
	mpy $38, $3, $38
	mpyi $38, $38, BYTES_PER_PIXEL

	## Tässä on tarkoitus kasvattaa osoittimen arvoa:
	## LS-osoitin on 32b, eikös...
	a $IMG_PTR, $9, $38

##     for (i = 0;
	il $X_LOOP_COUNTER, 0

x_loop:	
##     areaWidth > i; i++)
	cgt $TEMP, $3, $X_LOOP_COUNTER
	brz $TEMP, finish_x_loop

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

	## Tähän vois nyt aluksi jotain väriä töhertää
	##
	## Tilanne:
	## IMG_PTR --> XXXX|XXXX|XXXX|XXXX || XXXX|XXXX|XXXX|XXXX || ...
	
	## *(line + 4 * i + bytesPerPixel)
	mpyi $TEMP, $X_LOOP_COUNTER, BYTES_PER_PIXEL
	a $TEMP, $TEMP, $BYTES_PER_PIXEL
	a $TEMP_PTR, $IMG_PTR, $TEMP
	
	il $TEMP, 255
	stqd $TEMP, 0($TEMP_PTR)

	ai $X_LOOP_COUNTER, $X_LOOP_COUNTER, 1
##     }
finish_x_loop:	
	
	ai $Y_LOOP_COUNTER, $Y_LOOP_COUNTER, 1
##   }
finish_y_loop:	
## }
	
	## Epilogi
	ai $sp, $sp, FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
