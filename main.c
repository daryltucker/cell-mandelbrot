/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen, Ville-Matti Pasanen 2007.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libspe2.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384


extern spe_program_handle_t mandelbrot_handle;


typedef struct {
    spe_context_ptr_t context __attribute__((aligned(16)));
    // ...
} thread_arguments;


void fail(const char *msg)
{
    perror(msg);
    exit(1);
}


void *run_spu_thread(void *arg)
{
    thread_arguments args;
    unsigned int entry = SPE_DEFAULT_ENTRY;
    args = *((thread_arguments *) arg);

    if ( spe_context_run(args.context, &entry, 0, &foo_bar, NULL, NULL) < 0 )
	fail("SPE kontekstin k‰ynnistys ei onnistunut");

    puts("Hei, olen s‰ie ja kuolen ihan kohta!");

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

    spu_threads = 4;

    // Varataan muistia kuvaa varten...

    for (i=0; i<spu_threads; i++)
    {
	thread_arguments *arg = &thread_args[i];

	// Asetetaan s‰ikeelle osoitin kuvaan
	// sek‰ omaan osuuteen kuvasta...

	if ((arg->context = spe_context_create(0, NULL)) == NULL)
	    fail("Kontekstin luonti ei onnistunut");

	// T‰ss‰ ladataan spu-kontekstia...
	if ( spe_program_load(arg->context, &mandelbrot_handle) != 0 )
	    fail("SPU-ohjelman lataus ei onnistunut");

	pthread_create(&threads[i],
		       NULL,
		       &run_spu_thread,
		       arg);
    }

    // SPE:t laskee kovasti...
    puts("P‰‰ohjelma odottelee s‰ikeit‰...");

    for (i=0; i<spu_threads; i++)
    {
	if (pthread_join(threads[i], NULL))
	    fail("pthread_join() ep‰onnistui");

	if (spe_context_destroy(thread_args[i].context))
	    fail("Kontekstin tuhoaminen ep‰onnistui");
    }

    // Piirret‰‰n lopputulos (tiedostoon ja/tai ikkunaan)

    return 0;
}
