#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <limits.h>
#include <complex.h> 
#include <fftw3.h>
#include <immintrin.h>
#include <float.h>
#include "sdl/renders.h"

#define PLAY        0
#define STOP        1

#define REAL        0
#define IMG         1
#define M_2PI       ((double) 6.2831853071795864769252867665590057683943387987502116419498891846) 

typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioSpec   Obtained;
	double          step;
	uint32_t        samplePos;
	int             FreqSample;
    int             Play;	
    int             *Wave;
    int             Max;
    int             Min;
    double          Average;
    int             filled;
} Sound;

typedef struct {
    SDL_Renderer *Render;
    SDL_Window   *Window;
} Window;



Sound sound;
int WaterfallInit(Waterfall *W);
void WaterfallAdd(Waterfall *W, int * data, int lenght);
void WaterfallRender(Waterfall *W);

Window * WindowCreate();
void DrawWave(SDL_Renderer * Render, int x, int y, int width, int length, int *data, int buffersize , float xscale, int max);

void SDLAudioCallback(void *data, Uint8 *buffer, int length) {
    Sound *sound = (Sound*) data;

    int len = length >> 1;          // length/2 Porque usei AUDIO_S16SYS como áudio formato
    int16_t *BufferOut = (int16_t *) buffer;
    for(int i = 0; i < len; i++)
    {
        sound->Wave[i] = BufferOut[i];
        sound->samplePos++;
        sound->Average += BufferOut[i] ;
    }	    
    sound->Average = sound->Average / (sound->Obtained.samples + 1);
    sound->filled = 1;
}
 
void sound_init(Sound *sound) {

    sound->FreqSample 		= 44100;
	sound->step 	        = (double) ((M_PI * 2) / sound->FreqSample );
    sound->samplePos        = 0;
    sound->Max              = INT16_MAX;
    sound->Min              = INT16_MIN;
    sound->filled           = 0;
	printf ("Step: %f \n", sound->step);

    // https://wiki.libsdl.org/SDL_AudioSpec
	SDL_AudioSpec desired;
	SDL_zero(desired);
    desired.freq = sound->FreqSample;
    desired.format = AUDIO_S16SYS; 
    desired.channels = 1;
    desired.samples = 4096;
    desired.callback = SDLAudioCallback;
    desired.userdata = sound;

    sound->device = SDL_OpenAudioDevice(NULL, 1, &desired, &sound->Obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (sound->device == 0 || desired.format != sound->Obtained.format) { 
        printf ("OpenAudioDevice failed");
        exit(-1);
    }
    SDL_PauseAudioDevice(sound->device, STOP);
    
    
    sound->Wave              = malloc(sizeof(int) * sound->Obtained.samples);
    if (sound->Wave == NULL) {
        printf ("sound->Wave allocation failed.");
        exit(-1);
    }
    printf("Samples: %d , Format: %d , Buffer Wave size: %ld \n",sound->Obtained.samples, sound->Obtained.format,sizeof(Uint16) * sound->Obtained.samples);
    
	
}


float dabs (const float complex* in, float* out, float max, const int length)
{
    for (int i = 0; i < length; i += 8)
    {
        const float re[8] __attribute__((aligned (32))) = {creal(in[i]), creal(in[i+1]), creal(in[i+2]), creal(in[i+3]),creal(in[i+4]),creal(in[i+5]),creal(in[i+6]),creal(in[i+7])};
        const float im[8] __attribute__((aligned (32))) = {cimag(in[i]),cimag(in[i+1]),cimag(in[i+2]),cimag(in[i+3]),cimag(in[i+4]),cimag(in[i+5]),cimag(in[i+6]),cimag(in[i+7])};
        __m256 x4 = _mm256_load_ps (re);
        __m256 y4 = _mm256_load_ps (im);
        __m256 b4 = _mm256_sqrt_ps (_mm256_add_ps (_mm256_mul_ps (x4,x4), _mm256_mul_ps (y4,y4)));
        _mm256_storeu_ps (out + i, b4);
    }
    
    return max;
}

int main () {

    int quit = 0;
    SDL_Event event;     

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    Window *W = WindowCreate();
    atexit(SDL_Quit);   
	 
	sound_init(&sound);
    sound.Play              = PLAY;
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    
	
    float *in;// , *out; 
    fftwf_complex *out;
    fftwf_plan p;
    in = fftwf_malloc(sizeof(float) * sound.Obtained.samples + 1);
    out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * sound.Obtained.samples);
    //out = (double*) fftw_malloc(sizeof(double) * 2048 + 1);
    float *mag = _mm_malloc(sizeof(float) * sound.Obtained.samples,sizeof(float));
    int *magi = _mm_malloc(sizeof(int) * sound.Obtained.samples,sizeof(float));
    float *overlapin = _mm_malloc(sizeof(float) * sound.Obtained.samples,sizeof(float));
    p = fftwf_plan_dft_r2c_1d(sound.Obtained.samples, in, out, FFTW_ESTIMATE);
    float max = FLT_MIN;

    SDL_PauseAudioDevice(sound.device, PLAY);


    Waterfall Water;
    Water.Render = W->Render;
    Water.x = 0;
    Water.y = 0;
    Water.w = 512;
    Water.h = 85;
    Water.max = sound.Max;
    Water.min = sound.Min;  

    WaterfallInit(&Water);


    while( quit == 0 ){

        SDL_SetRenderDrawColor(W->Render, 0, 0, 0, 255);
        SDL_RenderClear(W->Render);        
        int w,h;
        SDL_GetWindowSize(W->Window,&w,&h);
        DrawWave(W->Render,1,86,w,h/3,sound.Wave,sound.Obtained.samples,(float) sound.Obtained.samples / h ,sound.Max);
        if (sound.filled == 1) {
            //printf ("Min: %d Max: %d Average: %f\n",sound.Min,sound.Max,sound.Average);
            for (int i = 0; i < (sound.Obtained.samples >> 1); i++) {
                in[i] = overlapin[i] * (0.53836 - 0.46164*cos((M_2PI * i)/(sound.Obtained.samples-1)));
                overlapin[i] = sound.Wave[i + (sound.Obtained.samples >> 1)];
            }
            for (int i = (sound.Obtained.samples >> 1) + 1; i < sound.Obtained.samples; i++) {
                in[i] = sound.Wave[i - (sound.Obtained.samples >> 1)] * (0.53836 - 0.46164*cos((M_2PI * i)/(sound.Obtained.samples-1)));
            }
            fftwf_execute(p);
            
            max = dabs (out, mag, max , sound.Obtained.samples >> 1);
            for (int i = 0; i < sound.Obtained.samples >> 1; i++ ) {
                magi[i] = (int) floor(mag[i]);   
                if (magi[i] > max) max = magi[i];
            }
            
            sound.filled = 0;
        }
        DrawWave(W->Render,1,h-h/6,w,h/3,magi,sound.Obtained.samples >> 1, 1 , max);

        WaterfallAdd(&Water,magi,sound.Obtained.samples >> 1);
        WaterfallRender(&Water);
        SDL_RenderPresent(W->Render);


            while ( SDL_PollEvent( &event ) ) {
                switch( event.type ){
                    case SDL_KEYDOWN:                        
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:  
                            case SDLK_q:
                                quit = 1; 
                            break;
                            case SDLK_SPACE:
                                sound.Play ^= 1; 
                                SDL_PauseAudioDevice(sound.device, sound.Play);
                            break;
                        }
                    break;
                    case SDL_QUIT:
                        quit = 1;
                    break;
                    default:
                    break;
                }

            }
            SDL_Delay(30);
    }	

	SDL_CloseAudioDevice(sound.device);
    SDL_Quit();
    exit(0);
}


Window * WindowCreate() {
    SDL_Window* window = SDL_CreateWindow(__FILE__,
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  512,
										  256,
 										  SDL_WINDOW_RESIZABLE);
    SDL_Surface *s;
    s = SDL_GetWindowSurface( window );
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));
    SDL_UpdateWindowSurface( window );

    Window *W = malloc(sizeof(Window));
    W->Render = SDL_CreateRenderer(window, -1, 0);
    W->Window = window;

    return W;
}
