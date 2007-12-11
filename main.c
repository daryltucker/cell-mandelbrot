/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen 2007.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libspe2.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384


// extern spe_program_handle_t mandelbrot_handle;


typedef struct {
    spe_context_ptr_t context __attribute__((aligned(16)));
    // ...
} thread_arguments;


void *run_spu_thread(void *arg)
{
    thread_arguments args;
    unsigned int entry = SPE_DEFAULT_ENTRY;
    args = *((thread_arguments *) arg);

    if ( spe_context_run(args.context, &entry, 0,
			 &foo_bar, NULL, NULL) < 0 ) {
	perror("SPE kontekstin k‰ynnistys ei onnistunut");
	exit(1);
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    int img_width = 300, img_height = 300;
    int i, spu_threads;
    thread_arguments thread_args[MAX_SPU_THREADS];
    pthread_t threads[MAX_SPU_THREADS];

    if ((spu_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1)) < 1){
	fprintf(stderr, "Ei ole vapaata SPE-ydint‰\n");
	exit(1);
    }

    if (spu_threads > MAX_SPU_THREADS) spu_threads = MAX_SPU_THREADS;

    // Varataan muistia kuvaa varten...

    // Ladataan spu-konteksteja ja
    // k‰ynnistell‰‰n s‰ikeit‰...

    for (i=0; i<spu_threads; i++)
    {
	thread_arguments *arg = &thread_args[i];

	// Asetetaan s‰ikeelle osoitin kuvaan
	// sek‰ omaan osuuteen kuvasta...

	if ((arg->context = spe_context_create(0, NULL)) == NULL) {
	    perror("Kontekstin luonti ei onnistunut");
	    exit(1);
	}

	// T‰ss‰ ladataan spu-kontekstia...

	// ja luodaan s‰ie...
    }

    // SPE:t laskee kovasti...

    // Joinataan...

    // Tuhotaan s‰ikeet...

    // Piirret‰‰n lopputulos (tiedostoon ja/tai ikkunaan)

    return 0;
}
