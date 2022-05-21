#ifndef __SDL_RENDERS__
#define __SDL_RENDERS__
#include <SDL2/SDL.h>


typedef union {
    uint32_t U32;
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    }U8;
} Color;

typedef struct {
    int size;
    int alloc;
    Color *colors;
} Line;

typedef struct {
    int     x,y,h,w;
    int     lsize;
    int     lalloc;
    Line   *lines;
    int     max, min;
    double  scale;
    double  offset;
    int     page;
    SDL_Renderer * Render;
} Waterfall;


#endif