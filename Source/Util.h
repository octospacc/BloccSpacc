#pragma once
#include <stdbool.h>

SDL_Surface * LoadImage ( char * FilePath );
void DrawSurf ( int x, int y, SDL_Surface * Src, SDL_Rect * Clip, SDL_Surface * Dst );
void FillSurfRGB ( int R, int G, int B, SDL_Surface * Dst );
void DrawOutlineRect ( SDL_Surface * Dst );
SDL_Surface * ScreenSet ( int Width, int Height, int Bits, SDL_Surface * Screen );
bool FlipScreen( SDL_Surface * Screen );

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

