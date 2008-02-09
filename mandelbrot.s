### Mandelbrotin fraktaalin piirt�v� funktio.

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

###### Rekistereit�
.equ max_iteration, 8

### Argumentit

###  Ym.
.equ x0, 15
.equ y0, 16
.equ x,  17
.equ y,  18
.equ scale, 21
.equ img_ptr, 31
.equ offset_x, 23
.equ offset_y, 24
.equ iteration, 27
.equ y_begin, 36
.equ x_loop_counter, 39
.equ y_loop_counter, 37

### Scratch registers
.equ temp, 75
.equ temp_ptr, 76
.equ temp_cond1, 77
.equ temp_cond2, 78
### 79 vapaana


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
## $10   <-- uint32 areaX -- Ei k�ytet�, arvo 0
## $11   <-- uint32 areaY
## $12   <-- uint32 areaWidth -- Ei k�ytet�, arvo sama kuin width
## $13   <-- uint32 areaHeight
## $14   <-- uint32 bytesPerPixel  -- Ei k�ytet�, arvo BYTES_PER_PIXEL
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
	fi $32, $7, $32		# tarkennetaan k��nteislukua
	lqr $33, mandelbrot_default_size
	fm $22, $33, $32		# r22 = r33 * r32

##   scale = mandSize / MIN(width, height);
	cgt $33, $3, $4
	## T�h�n varmaan on elegantimpikin ratkaisu...
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
	fm $scale, $22, $34	## $SCALE = r22 * r34

	## Kerrotaan mit� on saatu t�h�n asti aikaiseksi
	## wrch SPU_WrOutMbox, $35

##   offsetX = mandSize / -2.0 * (width > height ? (float)width/height : 1.0)
##             + reOffset;
##   offsetY = mandSize / -2.0 * (width < height ? (float)height/width : 1.0)
##             + imOffset;

	## Olkoon nyt offsetX ja offsetY 0.0
	lqr $offset_x, zero
	lqr $offset_y, zero

	## T�t� tarvitaan kohdassa y_loop_test
	a $y_begin, $13, $11
	
	lr $y_loop_counter, $11
	br y_loop_test
y_loop:	
##   {
	## T�ss� vois jotain tulostella kokeeksi
	## mutta pit�is panna parametrit talteen ennen aliohjelman kutsua

##     line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
	# line == $31
	# areaY $11
	# sitten j - areaY
	sf $temp, $11, $y_loop_counter
	mpy $temp, $3, $temp
	mpyi $temp, $temp, BYTES_PER_PIXEL

	## T�ss� on tarkoitus kasvattaa osoittimen arvoa:
	## LS-osoitin on 32b, eik�s...
	a $img_ptr, $9, $temp

	il $x_loop_counter, 0
	br x_loop_test
x_loop:	
##     {
##       x0 = i * scale + offsetX;
	cuflt $temp, $x_loop_counter, 0 	# $temp = (float) i;
	fma $x0, $temp, $scale, $offset_x
	
##       y0 = j * scale + offsetY;
	cuflt $temp, $y_loop_counter, 0
	fma $y0, $temp, $scale, $offset_y

	lr $x, $x0
	lr $y, $y0
	il $iteration, 0
	br fractal_loop_test

##       while ( ... )
fractal_loop:
##       {
##         xTemp = x*x - y*y + x0;
##         yTemp = 2*x*y + y0;
##         x = xTemp;
##         y = yTemp;
##         iteration++;
##       }
fractal_loop_test:
	## (MANDELBROT_DEFAULT_SIZE > x*x + y*y && maxIteration > iteration)
	fm $temp, $y, $y
	fma $temp, $x, $x, $temp
	lqr $33, mandelbrot_default_size
	
	fcgt $temp_cond1, $33, $temp
	cgt $temp_cond2, $max_iteration, $iteration
	## and... ja meit�h�n kiinnostaa vain ekan sana-alkion bitit
	and $temp_cond1, $temp_cond1, $temp_cond2
	brnz $temp_cond1, fractal_loop

##       maxColor = 0;
##       for (k = 0; k < bytesPerPixel; k++)
##         maxColor = (maxColor << 8) + 0xFF;
##       color = (unsigned int)((float)(iteration)/maxIteration * maxColor);
##       if (iteration == maxIteration)
##         color = 0;
##       for (k = bytesPerPixel - 1; k >= 0; k--)
##         *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
##           (color & ((uint32)0xFF << (8*k))) >> (8*k);

	## T�h�n vois nyt aluksi jotain v�ri� t�hert��
	##
	## Tilanne:
	## IMG_PTR --> XXXX|XXXX|XXXX|XXXX || XXXX|XXXX|XXXX|XXXX || ...
	
	## *(line + 4 * i)
	mpyi $temp, $x_loop_counter, BYTES_PER_PIXEL
	a $temp_ptr, $img_ptr, $temp
	
	il $temp, 255
	stqd $temp, 0($temp_ptr)

	ai $x_loop_counter, $x_loop_counter, 1
##     }
x_loop_test:
	cgt $temp, $3, $x_loop_counter
	## t�h�n vinkki ett� tod.n�k ep�tosi (?)
	brnz $temp, x_loop
	
	ai $y_loop_counter, $y_loop_counter, 1
##   }
y_loop_test:	
	cgt $temp, $36, $y_loop_counter
	## t�h�n vinkki ett� tod.n�k ep�tosi
	brnz $temp, y_loop

	## Epilogi
	ai $sp, $sp, FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
## }
