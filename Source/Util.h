#pragma once
#include <stdbool.h>
#include "LibMultiSpacc/MultiSpacc.h"

SDL_Surface * LoadImage ( char *FilePath, MultiSpacc_Surface *Screen );
void DrawSurf ( int x, int y, SDL_Surface * Src, SDL_Rect * Clip, SDL_Surface * Dst );
void FillSurfRGB ( int R, int G, int B, SDL_Surface * Dst );
void DrawOutlineRect ( int x, int y, int w, int h, int Size, int R, int G, int B, SDL_Surface * Dst );
//SDL_Surface * ScreenSet ( int Width, int Height, int Bits, SDL_Surface * Screen );
bool FlipScreen( MultiSpacc_Window * Window );

struct xyz {
	int x, y, z;
};

// <https://gist.github.com/jordwest/8a12196436ebcf8df98a2745251915b5>
struct xyz OrthoToIso ( int x, int y, int z, int Multiply ) {
	struct xyz xyz;
	xyz.x = x * 1 * 0.5 * Multiply + z * -1 * 0.5 * Multiply;
	xyz.z = x * 0.5 * 0.5 * Multiply + z * 0.5 * 0.5 * Multiply;
	xyz.y = xyz.z;
	return xyz;
}

