/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen 2007.
 */

#include <stdio.h>
#include <pthread.h>
#include <libspe2.h>


extern spe_program_handle_t convert_to_upper_handle;

spe_context_ptr_t speid;
unsigned int entry = SPE_DEFAULT_ENTRY;


int main(int argc, char *argv[])
{
    int img_width = 300, img_height = 300;

    if (spe_cpu_info_get(SPE_COUNT_USABLE_SPES,-1) < 1){
	fprintf(stderr, "Ei ole vapaata SPE-ydint‰\n");
	exit(1);
    }

    if ((speid = spe_context_create(0,NULL)) == NULL) {
	fprintf(stderr, "Kontekstin luonti ep‰onnistui\n");
	exit(1);
    }

    // Ladataan SPE-ohjelma kontekstiin...

    // Suoritetaan SPE-ohjelma kontekstissa...

    // Tuhotaan konteksti...

    // Piirret‰‰n lopputulos (tiedostoon ja/tai ikkunaan)

    return 0;
}
