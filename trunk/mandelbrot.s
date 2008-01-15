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

	## Epilogi
	ai $sp, $sp, FACT_FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
