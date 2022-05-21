#include "renders.h"

int WaterfallInit(Waterfall *W){

    W->lines = calloc(1,sizeof(Line)*W->h);
    if (W->lines == NULL) return -1;
    W->lalloc = W->h;
    

    for (int i = 0; i < W->h; i++){

        W->lines[i].colors = calloc(1,sizeof(Color)*W->w);
        W->lines[i].size = 0;
        W->lines[i].alloc = W->w;

        if (W->lines[i].colors == NULL) {
            for (int a = 0; a < i; a++) free(W->lines[i].colors);
            free(W->lines);
            return -2;
        }
    }
    
    int Range = abs(W->max - W->min);
    W->scale =  (double) UINT32_MAX / Range;
    W->offset = 0;
    //if (W->min < 0) W->offset = abs(W->min);
    //W->offset += 255;

    W->page = 0;
    return 0;
}

void WaterfallAdd(Waterfall *W, int * data, int length){

    int i,xi;

    if (W->w > W->lines[W->page].alloc) {
        free(W->lines[W->page].colors);
        W->lines[W->page].colors = calloc(1,sizeof(Color)*W->w);
        if (W->lines[W->page].colors == NULL) return -1;
        W->lines[W->page].alloc = W->w;
    }

    if (W->h > W->lalloc) {
        Line *l = W->lines;
        W->lines = realloc(W->lines,sizeof(Line)*W->h);
        if (W->lines == NULL) { 
            W->lines = l;
            return -2;
        }
        W->lalloc = W->h;
    }

    Line *lines   = &W->lines[W->page];    
    Color *colors = W->lines[W->page].colors;
    lines->size = length;

    if (length > W->w) {
        float x_increment = (float) (W->w -1) / length;
        float x = 0;     
        memset(pline,0,W->w * sizeof(Color));   
        for (i = 0; i < length; i++) {
            xi = (int) floor(x);
            pline[xi].U32 = ( pline[xi].U32 + (W->offset + (data[i])) );
            x = x + x_increment;
        }        
    } else {
        if (length == W->w) {
            for (i = 0; i < length; i++) {
                pline[i].U32 = W->offset + data[i] * W->scale;
            }
        } else {
            for (i = 0; i < length; i++) {
                pline[i].U32 = W->offset + data[i] * W->scale;
            }
            for (;i < W->w; i++) {
                pline[i].U32 = 0;
            }
        }
    }

    W->page = (W->page + 1) % W->h;
}

void WaterfallRender(Waterfall *W) {
    int page;
    Color *pline;
    for (int h = 0; h < W->h; h++) {
            page = (h + W->page) % W->h; 
            pline = &W->lines[page*W->w];
        for (int w = 0; w < W->w; w++) {
            SDL_SetRenderDrawColor(W->Render, pline[w].U8.r, pline[w].U8.g, pline[w].U8.b, pline[w].U8.a);
            SDL_RenderDrawPoint(W->Render,W->x + w,W->y + h);
        }
        
    }

}

void DrawWave(SDL_Renderer * Render, int x, int y, int width, int height, int *data, int buffersize , float xscale, int max) {

        int xi = 0,yi = 0,xold = 0,yold = 0;
        if (max == 0) max = 1;
        float scale = (float) (height >> 1) / max;
        int   xscaleint = 1.0f / xscale;
        if (xscaleint < 1) xscaleint = 1;

        SDL_SetRenderDrawBlendMode(Render,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Render, 88, 195, 74, 255);
        xold = x;
        yold =  y + (height >> 1) - data[0] * scale;
        xi = xold + 1;
        yi = y + (height >> 1) - data[1] * scale;
        for (int i = 2 ; i < buffersize; i += xscaleint ) {
            for (int a = y + height; a > yi; a--){
                SDL_SetRenderDrawColor(Render, 88, 195, 74+a/2, 10 + a /8);
                SDL_RenderDrawPoint(Render,xi,a);
            }
            SDL_SetRenderDrawColor(Render, 88, 195, 74, 255);
            SDL_RenderDrawLine(Render, xold,yold,xi,yi);
            xold = xi;
            yold = yi;
            xi += 1;
            yi = y + (height>> 1) - data[i] * scale;
        }
    
    SDL_SetRenderDrawBlendMode(Render,SDL_BLENDMODE_NONE);

}