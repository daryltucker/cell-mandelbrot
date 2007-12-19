/*
 * Mandelbrot-fraktaalin PPE-p‰‰ohjelma.
 *
 * Tero J‰ntti, Matti Lehtinen, Ville-Matti Pasanen 2007.
 */

#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <libspe2.h>
#include <X11/Xlib.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384

#define MAX_FILE_NAME_LENGTH 200


extern spe_program_handle_t fractal_handle;


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

    if ( spe_context_run(args.context, &entry, 0, NULL, NULL, NULL) < 0 )
	fail("SPE kontekstin k‰ynnistys ei onnistunut");

    puts("Hei, olen s‰ie ja kuolen ihan kohta!");

    pthread_exit(NULL);
}


int draw_fractal(COLOR *image, int width, int height)
{
    int i, spu_threads;
    thread_arguments thread_args[MAX_SPU_THREADS];
    pthread_t threads[MAX_SPU_THREADS];

    if ((spu_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1)) < 1){
	fprintf(stderr, "Ei ole vapaata SPE-ydint‰\n");
	exit(1);
    }

    if (spu_threads > MAX_SPU_THREADS) spu_threads = MAX_SPU_THREADS;

    for (i=0; i<spu_threads; i++)
    {
	thread_arguments *arg = &thread_args[i];

	// Asetetaan s‰ikeelle osoitin kuvaan
	// sek‰ omaan osuuteen kuvasta...

	if ((arg->context = spe_context_create(0, NULL)) == NULL)
	    fail("Kontekstin luonti ei onnistunut");

	// T‰ss‰ ladataan spu-kontekstia...
	if ( spe_program_load(arg->context, &fractal_handle) != 0 )
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

    return 0;
}


void usage(const char *program)
{
    printf("The super-fast fractal drawing program\n"
	   "Usage:\n"
	   "%s -o FILE [-w WIDTH] [-h HEIGHT]\n", program);
}


Window create_window(Display *d, int *screen)
{
    Window w;

    *screen = DefaultScreen(d);
    w = XCreateSimpleWindow( d, RootWindow(d, *screen),
			     10, 10, 300, 300, 1,
			     BlackPixel(d, *screen), WhitePixel(d, *screen) );
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
    return w;
}


int main(int argc, char *argv[])
{
    int optchar;
    int img_width = 300, img_height = 300;
    int should_draw_window = 0;
    char filename[MAX_FILE_NAME_LENGTH + 1];
    COLOR *image;

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
	    break;
	case 'h':
	    img_height = atoi(optarg);
	    if (img_height <= 0) {
		usage(argv[0]);
		exit(2);
	    }
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

/*     if (strlen(filename) == 0) { */
/* 	usage(argv[0]); */
/* 	exit(2); */
/*     } */

    image = (COLOR *) memalign(16, img_width*img_height*sizeof(COLOR));

    if (should_draw_window) {
	Display *display;
	int screen;
	Window window;
	GC gc;
	XEvent e;

	if ((display = XOpenDisplay(NULL)) == NULL)
	    // Jotain fiksumpaa t‰h‰n...
	    fail("XOpenDisplay() ei onnistunut");

	window = create_window(display, &screen);
	gc = DefaultGC(display, screen);

	for (;;)
	{
	    XNextEvent(display, &e);

	    switch (e.type)
	    {
	    case Expose:
		XDrawString(display, window, gc,
			    50, 50,
			    "Hello, World!", strlen("Hello, World!"));
		break;
	    case KeyPress:
		goto finished;
	    default:
		break;
	    }
	}

    finished:
	XCloseDisplay(display);

    } else /* No window */ {

	draw_fractal(image, img_width, img_height);

	if (strlen(filename) > 0)
	    save_image(image, img_width, img_height, filename);
    }

    free(image);
    return 0;
}
