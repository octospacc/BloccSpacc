#include <stdbool.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

SDL_Surface * LoadImage ( char * FilePath ) {
	SDL_Surface * a = NULL;
	SDL_Surface * b = NULL;
	a = IMG_Load ( FilePath );
	if ( a == NULL ) {
		printf("[E] Error reading image %s.\n", FilePath);
	} else {
		b = SDL_DisplayFormat ( a );
		SDL_FreeSurface ( a );
		if ( b == NULL ) {
			printf("[E] Error adapting image %s.\n", FilePath);
		} else {
			Uint32 ColorKey = SDL_MapRGB( b->format, 0xFF, 0x00, 0xFF ); // Magenta
			SDL_SetColorKey( b, SDL_SRCCOLORKEY, ColorKey );
		}
	}
	return b;
}

void DrawSurf ( int x, int y, SDL_Surface * Src, SDL_Rect * Clip, SDL_Surface * Dst ) {
	SDL_Rect Offset;
	Offset.x = x;
	Offset.y = y;
	SDL_BlitSurface( Src, Clip, Dst, &Offset );
}

SDL_Surface * ScreenSet ( int Width, int Height, int Bits, SDL_Surface * Screen ) {
	Screen = SDL_SetVideoMode ( Width, Height, Bits, 
		//SDL_SWSURFACE //|
		SDL_HWSURFACE |
		SDL_DOUBLEBUF //|
		//SDL_RESIZABLE //|
		//SDL_FULLSCREEN //|
	);
	return Screen;
}

bool FlipScreen( SDL_Surface * Screen ) {
	if ( SDL_Flip( Screen ) != 0 ) {
		printf("[E] Error updating screen.\n");
		return false;
	}
	SDL_FillRect( Screen, &Screen->clip_rect, SDL_MapRGB( Screen->format, 0xFF, 0xFF, 0xFF ) );
	return true;
}
