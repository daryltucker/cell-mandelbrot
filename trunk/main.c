/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen 2007.
 */

#include <stdio.h>
#include <pthread.h>
#include <libspe2.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384


// extern spe_program_handle_t fractal_handle;


typedef struct {
    spe_context_ptr_t context __attribute__((aligned(16)));
    // ...
} thread_arguments;


void *run_spu_thread(void *arg)
{
    thread_arguments args;
    unsigned int entry = SPE_DEFAULT_ENTRY;
    args = *((thread_arguments *) arg);

    // K‰ynnistet‰‰n s‰ie...

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    int img_width = 300, img_height = 300;

    if (spe_cpu_info_get(SPE_COUNT_USABLE_SPES,-1) < 1){
	fprintf(stderr, "Ei ole vapaata SPE-ydint‰\n");
	exit(1);
    }

    // Ladataan spu-konteksteja ja
    // k‰ynnistell‰‰n s‰ikeit‰...

    // Piirret‰‰n lopputulos (tiedostoon ja/tai ikkunaan)

    return 0;
}
