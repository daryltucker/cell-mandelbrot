/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen, Ville-Matti Pasanen 2007.
 */

#include "image.h"
#include "fractal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <libspe2.h>
#include <SDL/SDL.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384

#define MAX_FILE_NAME_LENGTH 200

#define BYTES_PER_PIXEL 4
#define BITS_PER_PIXEL (BYTES_PER_PIXEL * 8)
#define MAX_IMG_WIDTH 3000
#define MAX_IMG_HEIGHT 3000


extern spe_program_handle_t fractal_handle;


typedef struct {
    spe_context_ptr_t context __attribute__((aligned(16)));
    fractal_parameters parameters __attribute__((aligned(16)));
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

    if ( spe_context_run(args.context, &entry, 0, &(args.parameters), NULL, NULL) < 0 )
	fail("SPE kontekstin k‰ynnistys ei onnistunut");

    puts("Hei, olen s‰ie ja kuolen ihan kohta!");

    pthread_exit(NULL);
}


int draw_fractal(char *image, int width, int height)
{
    int i, spu_threads;
    thread_arguments thread_args[MAX_SPU_THREADS];
    pthread_t threads[MAX_SPU_THREADS];

    if ((spu_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1)) < 1){
	fprintf(stderr, "Ei ole vapaata SPE-ydint‰\n");
	exit(1);
    }

    /* Testaillaan fraktaalia ensin yhdell‰ s‰ikeell‰
     * niin pysyy homma yksinkertaisena.
     */
    spu_threads = 1;
    // if (spu_threads > MAX_SPU_THREADS) spu_threads = MAX_SPU_THREADS;

    for (i=0; i<spu_threads; i++)
    {
	fractal_parameters *f = &thread_args[i].parameters;

	f->image = (uint64) image;
	f->width = (uint) width;
	f->height = (uint) height;
	f->re_offset = 0.0f;
	f->im_offset = 0.0f;
	f->zoom = 1.0f;
	f->max_iteration = 100;

	/* T‰h‰n sitten jonkunlainen fiksu jako s‰ikeille,
	 * kun on useampi s‰ie.
	 *
	 * Ja pit‰‰ ottaa huomioon rajallinen 256kt muisti.
	 */
	f->area_x = 0;
	f->area_y = 0;
	f->area_width = (uint) width;
	f->area_heigth = (uint) height;

	f->bytes_per_pixel = BYTES_PER_PIXEL;

	if ((thread_args[i].context = spe_context_create(0, NULL)) == NULL)
	    fail("Kontekstin luonti ei onnistunut");

	// T‰ss‰ ladataan spu-kontekstia...
	if ( spe_program_load(thread_args[i].context, &fractal_handle) != 0 )
	    fail("SPU-ohjelman lataus ei onnistunut");

	pthread_create( &threads[i],
			NULL,
			&run_spu_thread,
			&thread_args[i] );
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

    return 0;
}


void copy_image(const char *image, int width, int height, SDL_Surface *s)
{
    if (SDL_MUSTLOCK(s)) {
        if (SDL_LockSurface(s) < 0)
            exit(1);
    }

    /* Kopioidaan varsin raa'alla tavalla kuva n‰ytˆn bittikarttaan.
     *
     * Toivottavasti formaatit ovat samat!
     */
    memcpy(s->pixels, image, width*height*BYTES_PER_PIXEL);

    if (SDL_MUSTLOCK(s))
        SDL_UnlockSurface(s);
}


void usage(const char *program)
{
    printf("The super-fast fractal drawing program\n"
	   "Usage, command line:\n"
	   "%s -o FILE [-w WIDTH] [-h HEIGHT]\n"
	   "\n"
	   "Usage, window:\n"
	   "%s -X\n"
	   "\n"
	   "Options:\n"
	   "-o\tOutput file\n"
	   "-w\tImage width\n"
	   "-h\tImage height\n"
	   "-X\tShow window\n",
	   program, program);
}


int main(int argc, char *argv[])
{
    int optchar;
    int img_width = 100, img_height = 100;
    int should_draw_window = 0;
    char filename[MAX_FILE_NAME_LENGTH + 1];
    char *image;

    memset(filename, '\0', MAX_FILE_NAME_LENGTH + 1);

    while ((optchar = getopt(argc, argv, "w:h:o:X")) != -1)
    {
	switch (optchar)
	{
	case 'w':
	    img_width = atoi(optarg);
	    if (img_width <= 0) {
		usage(argv[0]);
		exit(2);
	    }
	    if (img_width > MAX_IMG_WIDTH)
		img_width = MAX_IMG_WIDTH;
	    break;
	case 'h':
	    img_height = atoi(optarg);
	    if (img_height <= 0) {
		usage(argv[0]);
		exit(2);
	    }
	    if (img_height > MAX_IMG_HEIGHT)
		img_height = MAX_IMG_HEIGHT;
	    break;
	case 'o':
	    strncpy(filename, optarg, MAX_FILE_NAME_LENGTH);
	    break;
	case 'X':
	    should_draw_window = 1;
	    break;
	default:
	    break;
	}
    }

    if (!should_draw_window && strlen(filename) == 0) {
	usage(argv[0]);
	exit(0);
    }

    image = (char *) memalign(16, img_width*img_height*BYTES_PER_PIXEL);

    //Testing...
    memset(image, '\0', img_width*img_height*BYTES_PER_PIXEL);

    if (should_draw_window) {
	SDL_Event event;
	SDL_Surface *screen;
	int quit = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	    fprintf(stderr, "SDL_Init() ei onnistunut: %s\n", SDL_GetError());
	    exit(1);
	}

	atexit(SDL_Quit);

	if ((screen = SDL_SetVideoMode(img_width, img_height,
				       BITS_PER_PIXEL, SDL_SWSURFACE)) == NULL)
	{
	    fprintf(stderr, "SDL_SetVideoMode() ei onnistunut: %s",
		    SDL_GetError());
	    exit(1);
	}
	SDL_WM_SetCaption("Fraktaali", NULL);

	draw_fractal(image, img_width, img_height);

	printf("Kuvan koko on %dx%dx%d = %d\n", img_width, img_height, BYTES_PER_PIXEL,
	       img_width*img_height*BYTES_PER_PIXEL);

        copy_image(image, img_width, img_height, screen);

	while (!quit && SDL_WaitEvent(&event))
	{
	    switch (event.type)
	    {
	    case SDL_QUIT:
		quit = 1;
		break;
	    case SDL_VIDEOEXPOSE:
                copy_image(image, img_width, img_height, screen);
		break;
	    case SDL_KEYDOWN:
	    case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_ESCAPE)
		    quit = 1;
		break;
	    default:
		break;
	    }
	}

    } else /* No window */ {

	draw_fractal(image, img_width, img_height);

	if (strlen(filename) > 0)
	    save_image(image, img_width, img_height, filename);
    }

    free(image);
    return 0;
}
