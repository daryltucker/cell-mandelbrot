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

###### Rekistereita

### Argumentit
.equ width, 3
.equ height, 4
.equ re_offset, 5
.equ im_offset, 6
.equ zoom, 7
.equ max_iteration, 8
.equ area_buffer, 9
#.equ area_x, 10		# Ei käytetä, arvo 0
.equ area_y, 11
#.equ area_width, 12		# Ei käytetä, arvo sama kuin width
.equ area_height, 13
#.equ bytes_per_pixel, 14	# Ei käytetä, arvo BYTES_PER_PIXEL

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
.equ max_color, 29
.equ y_begin, 36
.equ x_loop_counter, 39
.equ y_loop_counter, 37
.equ x_tmp, 38

### Scratch registers
.equ tmp, 75
.equ tmp_ptr, 76
.equ tmp_cond1, 77
.equ tmp_cond2, 78
.equ tmp2, 79			# <-- Viimeinen volatile-rekisteri


.text

.global drawMandelbrotArea
.type drawMandelbrotArea,@function
drawMandelbrotArea:	
	## Prologi
	stqd $lr, LR_OFFSET($sp)
	stqd $sp, FRAME_SIZE($sp)
	ai $sp, $sp, -FRAME_SIZE
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
	cgt $33, $width, $height
	## Tähän varmaan on elegantimpikin ratkaisu...
	brz $33, width_min
	## Okei, $33:iin tulee minimi
height_min:	
	lr $33, $height
	br min_done
width_min:
	lr $33, $width
min_done:
	## 1/min $34:iin
	frest $34, $33
	fi $34, $33, $34
	fm $scale, $22, $34	## $SCALE = r22 * r34

	## Kerrotaan mitä on saatu tähän asti aikaiseksi
	## wrch SPU_WrOutMbox, $35

##   offsetX = mandSize / -2.0 * (width > height ? (float)width/height : 1.0)
##             + reOffset;
##   offsetY = mandSize / -2.0 * (width < height ? (float)height/width : 1.0)
##             + imOffset;

### Joitain alustuksia
	orbi $max_color, $max_color, 0xFF
	cuflt $max_color, $max_color, 0

	## Olkoon nyt offsetX ja offsetY 0.0
	lqr $offset_x, zero
	lqr $offset_y, zero

	## Tätä tarvitaan kohdassa y_loop_test
	a $y_begin, $area_height, $area_y

### Ryhdytaan kaymaan kuvaa lapi
	lr $y_loop_counter, $area_y
	br y_loop_test
y_loop:	
##   {
	## Tässä vois jotain tulostella kokeeksi
	## mutta pitäis panna parametrit talteen ennen aliohjelman kutsua

##     line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
	sf $tmp, $area_y, $y_loop_counter # $tmp = $y_loop_counter - $area_y
	mpy $tmp, $width, $tmp
	mpyi $tmp, $tmp, BYTES_PER_PIXEL

	## Tässä on tarkoitus kasvattaa osoittimen arvoa:
	## LS-osoitin on 32b, eikös...
	a $img_ptr, $area_buffer, $tmp

	il $x_loop_counter, 0
	br x_loop_test
x_loop:	
##     {
##       x0 = i * scale + offsetX;
	cuflt $tmp, $x_loop_counter, 0 	# $tmp = (float) i;
	fma $x0, $tmp, $scale, $offset_x
	
##       y0 = j * scale + offsetY;
	cuflt $tmp, $y_loop_counter, 0
	fma $y0, $tmp, $scale, $offset_y

	lr $x, $x0
	lr $y, $y0

##       while ( ... )
	il $iteration, 0
	br fractal_loop_test
fractal_loop:
##       {
	## xTemp = x*x - y*y + x0;
	## saiskohan tan menee 2:lla kaskylla?
	fm $tmp, $y, $y
	fma $tmp, $x, $x, $tmp
	fa $x_tmp, $tmp, $x0

	## y = 2*x*y + y0;
	fm $tmp, $x, $y
	fa $tmp, $tmp, $tmp
	fa $y, $tmp, $y0
	
	lr $x, $x_tmp
	ai $iteration, $iteration, 1
##       }
fractal_loop_test:
	## (MANDELBROT_DEFAULT_SIZE > x*x + y*y && maxIteration > iteration)
	## Tassa vois valmiiksi laskea x*x ja y*y seuraavalle iteraatiolle
	fm $tmp, $y, $y
	fma $tmp, $x, $x, $tmp
	lqr $33, mandelbrot_default_size
	
	fcgt $tmp_cond1, $33, $tmp
	cgt $tmp_cond2, $max_iteration, $iteration
	## and... ja meitähän kiinnostaa vain ekan sana-alkion bitit
	and $tmp_cond1, $tmp_cond1, $tmp_cond2
	brnz $tmp_cond1, fractal_loop

	## Ei ehka tarviis menna floatin kautta, emt.
##       color = (unsigned int)((float)(iteration)/maxIteration * maxColor);
	cuflt $tmp, $max_iteration, 0
	frest $tmp2, $max_iteration
	fi $tmp, $tmp, $tmp2
	cuflt $tmp2, $iteration, 0
	
	fm $tmp, $tmp2, $tmp
	fm $tmp, $tmp, $max_color

##       if (iteration == maxIteration)
##         color = 0;
##       for (k = bytesPerPixel - 1; k >= 0; k--)
##         *(line + bytesPerPixel * (i - areaX) + (bytesPerPixel - k - 1)) =
##           (color & ((uint32)0xFF << (8*k))) >> (8*k);

	## Tähän vois nyt aluksi jotain väriä töhertää
	##
	## Tilanne:
	## img_ptr --> XXXX|XXXX|XXXX|XXXX || XXXX|XXXX|XXXX|XXXX || ...
	
	## *($img_ptr + i * BYTES_PER_PIXEL)
	mpyi $tmp, $x_loop_counter, BYTES_PER_PIXEL
	a $tmp_ptr, $img_ptr, $tmp

	## Tama ei tayta kaikkia tavuja
	il $tmp, 255
	## Tama tallettaa 128b lohkoja
	stqd $tmp, 0($tmp_ptr)

	ai $x_loop_counter, $x_loop_counter, 1
##     }
x_loop_test:
	cgt $tmp, $width, $x_loop_counter
	## tähän vinkki että tod.näk epätosi (?)
	brnz $tmp, x_loop
	
	ai $y_loop_counter, $y_loop_counter, 1
##   }
y_loop_test:	
	cgt $tmp, $36, $y_loop_counter
	## tähän vinkki että tod.näk epätosi
	brnz $tmp, y_loop

	## Epilogi
	ai $sp, $sp, FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
## }
