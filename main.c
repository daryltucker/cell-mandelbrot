/*
 * Mandelbrot-fraktaalin PPE-p��ohjelma.
 *
 * K�ynnist�� SPU-s�ikeet, jotka piirt�v�t Mandelbrotin
 * fraktaalin. Tarvittaessa piirt�� fraktaalin ikkunaan tai
 * tiedostoon.
 *
 * Tero J�ntti, Matti Lehtinen, Ville-Matti Pasanen 2007.
 */

#include "fractal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <libspe2.h>
#include <sys/time.h>
#include <SDL/SDL.h>


#define MAX_SPU_THREADS 6
#define MAX_DATA_PER_THREAD 16384

#define MAX_FILE_NAME_LENGTH 200

#define BYTES_PER_PIXEL 4
#define BITS_PER_PIXEL (BYTES_PER_PIXEL * 8)
#define MAX_IMG_WIDTH 4000
#define MAX_IMG_HEIGHT 4000
#define ZOOM_STEP 2.0f
#define MOVE_STEP 0.5f

#define MEGA 1000000


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
	fail("SPE kontekstin k�ynnistys ei onnistunut");

    pthread_exit(NULL);
}


inline long long time_between(const struct timeval *begin, const struct timeval *end)
{
    return ((long long) end->tv_sec * MEGA + end->tv_usec)
        - ((long long) begin->tv_sec * MEGA + begin->tv_usec);
}


int draw_fractal(char *image, int width, int height, int requested_threads, float zoom, float re_offset, float im_offset)
{
    int i, work_threads;
    thread_arguments thread_args[MAX_SPU_THREADS];
    pthread_t threads[MAX_SPU_THREADS];
    struct timeval time_threads_started, time_drawing_started, final_time;

    assert(requested_threads >= 1);
    work_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);
    if (work_threads < 1){
	fprintf(stderr, "Ei ole vapaata SPE:t�\n");
	exit(1);
    } else if (work_threads < requested_threads) {
        fprintf(stderr, "Ei saatu %d SPE:t� k�ytt��n, %d s�iett� k�yt�ss�\n",
                requested_threads, work_threads);
    } else if (work_threads > requested_threads) {
        work_threads = requested_threads;
    }

    gettimeofday(&time_threads_started, NULL);

    uint32 slice_height = (uint32) (height / work_threads);
    uint32 last_slice_height = (uint32)height - ((uint32)(work_threads - 1)) * slice_height;
    assert(slice_height <= last_slice_height);
    assert(last_slice_height - slice_height <= work_threads);

    /* Tulee Bus error kun width = height = 150 ja saikeita on 6!!!
     * Varmaan yritetaan kirjoittaa samalle muistialueelle.
     */ 
    for (i=0; i<work_threads; i++)
    {
	fractal_parameters *f = &thread_args[i].parameters;

	f->image         = (uint64) image;
	f->width         = (uint) width;
	f->height        = (uint) height;
	f->re_offset     = re_offset;
	f->im_offset     = im_offset;
	f->zoom          = zoom;
	f->max_iteration = 200;
	f->area_x        = 0;
	f->area_y        = 0 + i*slice_height;
	f->area_width    = width;
	f->area_heigth   = ((i == work_threads-1) ? last_slice_height : slice_height);

	f->bytes_per_pixel = BYTES_PER_PIXEL;

	if ((thread_args[i].context = spe_context_create(0, NULL)) == NULL)
	    fail("Kontekstin luonti ei onnistunut");

	if ( spe_program_load(thread_args[i].context, &fractal_handle) != 0 )
	    fail("SPU-ohjelman lataus ei onnistunut");

	pthread_create( &threads[i],
			NULL,
			&run_spu_thread,
			&thread_args[i] );
    }


    // Laitetaan s�ikeille viesti milloin voi alloittaa.
    unsigned int message = 1;
    for (i=0; i<work_threads; i++)
        spe_in_mbox_write(thread_args[i].context, &message, 1, SPE_MBOX_ANY_NONBLOCKING);

    gettimeofday(&time_drawing_started, NULL);

    for (i=0; i<work_threads; i++)
    {
	if (pthread_join(threads[i], NULL))
	    fail("pthread_join() ep�onnistui");

	if (spe_context_destroy(thread_args[i].context))
	    fail("Kontekstin tuhoaminen ep�onnistui");
    }

    gettimeofday(&final_time, NULL);

    long long ppe_exec_time = time_between(&time_drawing_started, &final_time);
    long long total_exec_time = time_between(&time_threads_started, &final_time);

    printf("\nTime used (ms):\n");
    printf("drawing\t%llu\n", ppe_exec_time);
    printf("total\t%llu\n\n", total_exec_time);

    return 0;
}


/* Kopioi kuvan SDL_Surface-tyyppiseen kuvaan.
 *
 * Kuvien t�ytyy olla samassa formaatissa.
 */
void copy_image(const char *image, int width, int height, SDL_Surface *s)
{
    int x, y;
    int width_bytes = MIN(width, s->w) * s->format->BytesPerPixel;
    int h = MIN(height, s->h);
    char *pixels = (char *) s->pixels;
    char *buf_p = pixels;
    const char *img_p = image;

    if (SDL_MUSTLOCK(s)) {
        if (SDL_LockSurface(s) < 0)
            exit(1);
    }

    for (y=0; y < h; y++)
    {
        buf_p = pixels + y * width_bytes;
        for (x=0; x < width_bytes; x++)
        {
            *buf_p++ = *img_p++;
        }
    }

    if (SDL_MUSTLOCK(s))
        SDL_UnlockSurface(s);
}


void usage(const char *program)
{
    printf("The lightning-fast fractal drawing program :P\n"
	   "Usage:\n"
	   "%s [-?] [-o FILE] [-w WIDTH] [-h HEIGHT] [-n THREADS] [-X]\n"
	   "\n"
	   "Options:\n"
           "-?\tThis help\n"
	   "-o\tOutput file (in BMP format)\n"
	   "-w\tImage width\n"
	   "-h\tImage height\n"
           "-n\tNumber of threads, excluding the main thread (1 - %d)\n"
	   "-X\tShow window\n",
	   program, MAX_SPU_THREADS);
}


int main(int argc, char *argv[])
{
    int optchar;
    int img_width = 400, img_height = 400, n_threads = 1;
    float zoom = 1.0f, re_offset = 0.0f, im_offset = 0.0f;
    int should_draw_window = 0, quit = 0;
    char filename[MAX_FILE_NAME_LENGTH + 1];
    char *image;
    SDL_Event event;
    SDL_Surface *screen;

    memset(filename, '\0', MAX_FILE_NAME_LENGTH + 1);

    while ((optchar = getopt(argc, argv, "?w:h:o:n:X")) != -1)
    {
	switch (optchar)
	{
            case '?':
                usage(argv[0]);
                exit(0);
                break;
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
            case 'n':
                n_threads = atoi(optarg);
                if (n_threads < 1 || n_threads > MAX_SPU_THREADS) {
                    usage(argv[0]);
                    exit(2);
                }
                break;
            case 'X':
                should_draw_window = 1;
                break;
            default:
                break;
	}
    }

    image = (char *) memalign(16, img_width*img_height*BYTES_PER_PIXEL);

    if (should_draw_window) {

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
    }

    draw_fractal(image, img_width, img_height, n_threads, zoom, re_offset, im_offset);

    printf("Kuvan koko on %dx%dx%d = %d\n", img_width, img_height, BYTES_PER_PIXEL,
           img_width*img_height*BYTES_PER_PIXEL);

    if (strlen(filename) > 0) {
        //save_image(image, img_width, img_height, filename);
        SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, img_width, img_height,
                                              BYTES_PER_PIXEL*8, 0, 0, 0, 0);
        if (! s) {
            fprintf(stderr, "SDL_CreateRGBSurface() ei onnistunut: %s", SDL_GetError());
            exit(1);
        }
        copy_image(image, img_width, img_height, s);
        if (SDL_SaveBMP(s, filename) < 0) {
            fprintf(stderr, "Kuvan talletus ei onnistunut: %s", SDL_GetError());
        }
        SDL_FreeSurface(s);
    }

    if (should_draw_window) {

        copy_image(image, img_width, img_height, screen);
        SDL_UpdateRect(screen, 0, 0, img_width, img_height);

	while (!quit && SDL_WaitEvent(&event))
	{
	    switch (event.type)
	    {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_VIDEOEXPOSE:
                    copy_image(image, img_width, img_height, screen);
                    SDL_UpdateRect(screen, 0, 0, img_width, img_height);
                    break;

                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                        break;
                    }
                    else if (event.key.keysym.sym == SDLK_PLUS ||
                             event.key.keysym.sym == SDLK_KP_PLUS)
                        zoom *= ZOOM_STEP;
                    else if ((event.key.keysym.sym == SDLK_MINUS ||
                              event.key.keysym.sym == SDLK_KP_MINUS) &&
                             (zoom >= 0.0001f))
                        zoom *= 1.0f/ZOOM_STEP;
                    else if (event.key.keysym.sym == SDLK_UP)
                        im_offset -= MOVE_STEP / zoom;
                    else if (event.key.keysym.sym == SDLK_DOWN)
                        im_offset += MOVE_STEP / zoom;
                    else if (event.key.keysym.sym == SDLK_LEFT)
                        re_offset -= MOVE_STEP / zoom;
                    else if (event.key.keysym.sym == SDLK_RIGHT)
                        re_offset += MOVE_STEP / zoom;
                    else if (event.key.keysym.sym == SDLK_KP_MULTIPLY) {
                        zoom = 1.0f;
                        re_offset = im_offset = 0.0f;
                    } else
                        break;

                    draw_fractal(image, img_width, img_height, n_threads, zoom, re_offset, im_offset);
                    copy_image(image, img_width, img_height, screen);
                    SDL_UpdateRect(screen, 0, 0, img_width, img_height);
                    break;

                default:
                    break;
	    }
	}

    }

    free(image);
    return 0;
}
