#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define PLAY        0
#define STOP        1
#define FREQ		440 

typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioSpec   Obtained;
	double          step;
	uint32_t        samplePos;
	int             FreqSample;
    int             Play;	
    int             *Wave;
    int             Max;
} Sound;


Sound sound;

SDL_Renderer * WindowCreate();
void DrawWave(SDL_Renderer * Render, int x, int y, int width, int length, int *data, int buffersize , float xscale, int max);

void SDLAudioCallback(void *data, Uint8 *buffer, int length) {
    Sound *sound = (Sound*) data;

    int len = length >> 1;          // length/2 Porque usei AUDIO_S16SYS como áudio formato
    int16_t *BufferOut = (int16_t *) buffer;

    for(int i = 0; i < len; i++)
    {
        BufferOut[i] = (int16_t) (( 2500.0f * sin(sound->samplePos * sound->step) ) + ((double) rand() / RAND_MAX) * 100.0f);
        if (BufferOut[i] > sound->Max) sound->Max = BufferOut[i];
        sound->Wave[i] = BufferOut[i];
        sound->samplePos++;
    }	

}
 
void sound_init(Sound *sound) {

    sound->FreqSample 		= 44100;
	sound->step 	        = (double) ((M_PI * 2) / sound->FreqSample ) * FREQ;
    sound->samplePos        = 0;
	printf ("Step: %f \n", sound->step);

    // https://wiki.libsdl.org/SDL_AudioSpec
	SDL_AudioSpec desired;
	SDL_zero(desired);
    desired.freq = sound->FreqSample;
    desired.format = AUDIO_S16SYS; 
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = SDLAudioCallback;
    desired.userdata = sound;

    sound->device = SDL_OpenAudioDevice(NULL, 0, &desired, &sound->Obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
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


int main () {

    int quit = 0;
    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_Renderer * Render = WindowCreate();
    atexit(SDL_Quit);   
	 
	sound_init(&sound);
    sound.Play              = PLAY;
    SDL_PauseAudioDevice(sound.device, PLAY);
	
    while( quit == 0 ){

        SDL_SetRenderDrawColor(Render, 0, 0, 0, 255);
        SDL_RenderClear(Render);
        

        DrawWave(Render,1,1,1024,256,sound.Wave,2048, 0.25,sound.Max);

        SDL_RenderPresent(Render);


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
            SDL_Delay(100);
    }	

	SDL_CloseAudioDevice(sound.device);
    SDL_Quit();
    exit(0);
}


SDL_Renderer * WindowCreate() {
    SDL_Window* window = SDL_CreateWindow("",
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  512,
										  256,
 										  SDL_WINDOW_RESIZABLE);
    return SDL_CreateRenderer(window, -1, 0);
}

void DrawWave(SDL_Renderer * Render, int x, int y, int width, int length, int *data, int buffersize , float xscale, int max) {

        int xi = 0,yi = 0,xold = 0,yold = 0; 
        if (max == 0) max = 1;
        float scale = (float) (length >> 1) / max;
        int   xscaleint = 1.0f / xscale;
        
        for (int i = 0 ; i < buffersize; i += xscaleint ) {
            int c = abs(data[i] * (255.0f / max)) / 25;
            SDL_SetRenderDrawColor(Render, c, c + 10, c, 100);
            xi = xi + 1;
            SDL_RenderDrawLine(Render,xi + x,(length >> 1) + y,xi + x,y + (length >> 1) - data[i] * scale);
        }
        
        SDL_SetRenderDrawColor(Render, 88, 195, 74, 255);
        xold = x;
        yold =  y + (length >> 1) - data[0] * scale;
        xi = xold + 1;
        yi = y + (length >> 1) - data[1] * scale;
        for (int i = 2 ; i < buffersize; i += xscaleint ) {
            SDL_RenderDrawLine(Render, xold,yold,xi,yi);
            xold = xi;
            yold = yi;
            xi += 1;
            yi = y + (length >> 1) - data[i] * scale;
        }

}