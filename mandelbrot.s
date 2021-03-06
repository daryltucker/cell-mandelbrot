### Mandelbrotin fraktaalin piirt�v� funktio.
###
### Fraktaalin piirto ei toimi, piirtaa vain
### alueen punaiseksi.

.data
	
.align 4
mandelbrot_default_size:	
	.float, 0f4.0
.align 4
zero:
	.float 0.0


.equ LR_OFFSET, 16
.equ FRAME_SIZE, 32		# Ei pinomuuttujia
.equ BYTES_PER_PIXEL, 4

###### Rekistereita

### Argumentit
.equ width, 3
.equ height, 4
.equ offset_x, 5
.equ offset_y, 6
.equ scale, 7
.equ max_iteration, 8
.equ area_buffer, 9
#.equ area_x, 10		# Ei k�ytet�, arvo 0
.equ area_y, 11
#.equ area_width, 12		# Ei k�ytet�, arvo sama kuin width
.equ area_height, 13
#.equ bytes_per_pixel, 14	# Ei k�ytet�, arvo BYTES_PER_PIXEL

###  Ym.
.equ x0, 15
.equ y0, 16
.equ x,  17
.equ y,  18
.equ scale, 21
.equ mand_size, 22
.equ offset_x, 23
.equ offset_y, 24
.equ iteration, 27
.equ color, 28
.equ max_color, 29
.equ line_ptr, 31
.equ y_begin, 36
.equ y_loop_counter, 37
.equ x_tmp, 38
.equ x_loop_counter, 39
.equ word_insertion_mask, 40

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

### Joitain alustuksia
	orbi $max_color, $max_color, 0xFF
	cuflt $max_color, $max_color, 0

	## T�t� tarvitaan kohdassa y_loop_test
	a $y_begin, $area_height, $area_y

### Ryhdytaan kaymaan kuvaa lapi
	lr $y_loop_counter, $area_y
	br y_loop_test
y_loop:	
##     line = areaBuffer + (width * (j - areaY) * bytesPerPixel);
	sf $tmp, $area_y, $y_loop_counter # $tmp = $y_loop_counter - $area_y
	mpy $tmp, $width, $tmp
	mpyi $tmp, $tmp, BYTES_PER_PIXEL

	## T�ss� on tarkoitus kasvattaa osoittimen arvoa:
	## LS-osoitin on 32b, eik�s...
	a $line_ptr, $area_buffer, $tmp

	il $x_loop_counter, 0
	br x_loop_test
x_loop:	
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
	## xTemp = x*x - y*y + x0;
	fm $tmp, $y, $y
	fma $tmp, $x, $x, $tmp
	fa $x_tmp, $tmp, $x0

	## y = 2*x*y + y0;
	fm $tmp, $x, $y
	fa $tmp, $tmp, $tmp
	fa $y, $tmp, $y0
	
	lr $x, $x_tmp
	ai $iteration, $iteration, 1

fractal_loop_test:
	## (MANDELBROT_DEFAULT_SIZE > x*x + y*y && maxIteration > iteration)
	## Tassa vois valmiiksi laskea x*x ja y*y seuraavalle iteraatiolle
	fm $tmp, $y, $y
	fma $tmp, $x, $x, $tmp
	lqr $tmp2, mandelbrot_default_size
	fcgt $tmp_cond1, $tmp2, $tmp
	cgt $tmp_cond2, $max_iteration, $iteration
	
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
	cfltu $color, $tmp, 0

##       if (iteration == maxIteration)
##         color = 0;
	ceq $tmp_cond1, $iteration, $max_iteration
	## sana-alkiossa 0 on 111...1 jos tosi
	## muuten 000...0
	il $tmp, 0
	## $color-rekisterin ekaan sana-alkioon valitaan joko color tai 0
	selb $color, $color, $tmp, $tmp_cond1

	## Maalataan kokeeksi punaista
	il $color, 0xFF
	shli $color, $color, 16

	## iteration-muuttujasta saadaan vari
	# lr $color, $iteration

	## Tilanne:
	## line_ptr --> XXXX|XXXX|XXXX|XXXX || XXXX|XXXX|XXXX|XXXX || ...
	
	## *($line_ptr + i * BYTES_PER_PIXEL)
	mpyi $tmp, $x_loop_counter, BYTES_PER_PIXEL
	a $tmp_ptr, $line_ptr, $tmp

	## Asetetaan sana-alkio oikeaan kohtaan 16 tavun
	## lohkoa muistiin viemistävarten.
	lqd $tmp, 0($tmp_ptr)
	cwd $word_insertion_mask, 0($tmp_ptr)
	shufb $tmp, $color, $tmp, $word_insertion_mask

	stqd $tmp, 0($tmp_ptr)

	ai $x_loop_counter, $x_loop_counter, 1
	
x_loop_test:
	cgt $tmp, $width, $x_loop_counter
	## t�h�n vinkki ett� tod.n�k ep�tosi (?)
	brnz $tmp, x_loop
	
	ai $y_loop_counter, $y_loop_counter, 1
	
y_loop_test:	
	cgt $tmp, $y_begin, $y_loop_counter
	## t�h�n vinkki ett� tod.n�k ep�tosi
	brnz $tmp, y_loop

	## Epilogi
	ai $sp, $sp, FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
