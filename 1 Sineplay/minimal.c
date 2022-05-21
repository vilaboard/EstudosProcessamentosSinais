#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define FREQ		392 // G note

typedef struct {
	double      samplesPerSine;
	uint32_t    samplePos;
	double      FreqSample;
    int         Play;	
} Sound;

SDL_AudioDeviceID device;
Sound sound;

void windowcreate();

static void SDLAudioCallback(void *data, Uint8 *buffer, int length) {
    Sound *sound = (Sound*) data;

    for(int i = 0; i < length; ++i)
    {
        buffer[i] = 32 * sin(sound->samplePos / sound->samplesPerSine * M_PI * 2);
        sound->samplePos++;
    }	
}

void sound_init(Sound S) {
	
    // https://wiki.libsdl.org/SDL_AudioSpec
	SDL_AudioSpec iscapture, desired;
	SDL_zero(iscapture);
    iscapture.freq = S.FreqSample;
    iscapture.format = AUDIO_S16; // AUDIO_S16LSB
    iscapture.channels = 1;
    iscapture.samples = 2048;
    iscapture.callback = SDLAudioCallback;
    iscapture.userdata = &sound;
	

    device = SDL_OpenAudioDevice(NULL, 0, &iscapture, &desired, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (device == 0) printf ("OpenAudioDevice Failed");
    SDL_PauseAudioDevice(device, sound.Play);
	
}


int main () {

    int quit = 0;
    SDL_Event event;

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    windowcreate();

    atexit(SDL_Quit);

	sound.FreqSample 		= 44100;
	sound.samplesPerSine 	= sound.FreqSample / FREQ;
    sound.Play = 0;
	 
	sound_init(sound);
	
    while( quit == 0 ){

            while( SDL_PollEvent( &event ) ){
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
                                SDL_PauseAudioDevice(device, sound.Play);
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

	SDL_CloseAudioDevice(device);
    SDL_Quit();
    exit(0);
}


void windowcreate() {
    SDL_Window* window = SDL_CreateWindow("",
										  SDL_WINDOWPOS_UNDEFINED,
										  SDL_WINDOWPOS_UNDEFINED,
										  256,
										  256,
 										  SDL_WINDOW_RESIZABLE);
    SDL_Surface *s;
    s = SDL_GetWindowSurface( window );
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));
    SDL_UpdateWindowSurface( window );
}