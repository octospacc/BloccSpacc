#include <stdbool.h>
#include "LibMultiSpacc/MultiSpacc.h"

MultiSpacc_Surface * LoadImage ( char *FilePath, MultiSpacc_Surface *Screen ) {
	MultiSpacc_Surface *a = NULL;
	MultiSpacc_Surface *b = NULL;
	a = IMG_Load ( FilePath );///*
	if ( a == NULL ) {
		printf("[E] Error reading image %s.\n", FilePath);
	} else {
		b = SDL_ConvertSurface( a, Screen->format, 0 ); //SDL_DisplayFormat ( a );
		SDL_FreeSurface ( a );
		if ( b == NULL ) {
			printf("[E] Error adapting image %s.\n", FilePath);
		} else {
			Uint32 ColorKey = SDL_MapRGB( b->format, 0xFF, 0x00, 0xFF ); // Magenta
			MultiSpacc_SetColorKey( b, true, ColorKey );
		}
	}
	return b;//*/return a;
}

void DrawSurf ( int x, int y, MultiSpacc_Surface * Src, SDL_Rect * Clip, MultiSpacc_Surface * Dst ) {
	SDL_Rect Offset;
	Offset.x = x;
	Offset.y = y;
	SDL_BlitSurface( Src, Clip, Dst, &Offset );
}

void FillSurfRGB ( int R, int G, int B, MultiSpacc_Surface * Dst ) {
	SDL_FillRect( Dst, &Dst->clip_rect, SDL_MapRGB( Dst->format, R, G, B ) );
}

void DrawOutlineRect ( int x, int y, int w, int h, int Size, int R, int G, int B, MultiSpacc_Surface * Dst ) {
	SDL_Rect Rect;
	Rect.x = x;
	Rect.y = y;
	Rect.w = w;
	Rect.h = Size;
	SDL_FillRect( Dst, &Rect, SDL_MapRGB( Dst->format, R, G, B ) );
	Rect.w = Size;
	Rect.h = h;
	SDL_FillRect( Dst, &Rect, SDL_MapRGB( Dst->format, R, G, B ) );
	Rect.x = x;
	Rect.y = y+h-Size;
	Rect.w = w;
	Rect.h = Size;
	SDL_FillRect( Dst, &Rect, SDL_MapRGB( Dst->format, R, G, B ) );
	Rect.x = x+w-Size;
	Rect.y = y;
	Rect.w = Size;
	Rect.h = h;
	SDL_FillRect( Dst, &Rect, SDL_MapRGB( Dst->format, R, G, B ) );
}

/*
SDL_Surface * ScreenSet ( int Width, int Height, int Bits, SDL_Surface * Screen ) {
	Screen = SDL_SetVideoMode ( Width, Height, Bits, 
	#ifdef Target_PocketGo
		SDL_HWSURFACE
	#else
		SDL_SWSURFACE //|
		//SDL_HWSURFACE |
		//SDL_DOUBLEBUF |
		//SDL_RESIZABLE //|
		//SDL_FULLSCREEN //|
	#endif
	);
	return Screen;
}
*/

bool FlipScreen( MultiSpacc_Window * Window ) {
	if ( MultiSpacc_UpdateWindowSurface( Window ) != 0 ) {
		printf("[E] Error updating screen.\n");
		return false;
	}
	return true;
}
