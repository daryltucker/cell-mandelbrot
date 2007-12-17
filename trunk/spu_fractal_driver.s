### Spu-koodi, joka k‰sittelee p‰‰muistia
### ja kutsuu fraktaali-aliohjelmaa.

.text

.align 4
ihanan_kallista:
	.ascii "SPU: Elama on!\n\0"

### Pino spu_main:n stack frame:
#
# ( register argument save area )
# ( general register save area )
# ( local variables )
# ( parameter list )
#   link register
#   back chain

.equ LR_OFFSET, 16
.equ MAIN_FRAME_SIZE, 32


.global main
.type spu_main, @function

spu_main:
	## Prologi
	stqd $lr, LR_OFFSET($sp)
	stqd $sp, -MAIN_FRAME_SIZE($sp)
	ai $sp, $sp, -MAIN_FRAME_SIZE

	## Testi-tulostusta
	ila $3, ihanan_kallista
	brsl $lr, puts
	
	## Epilogi
	ai $sp, $sp, MAIN_FRAME_SIZE
	lqd $lr, LR_OFFSET($sp)
	bi $lr
