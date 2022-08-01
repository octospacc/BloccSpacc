#pragma once

SDL_Surface * LoadImage ( char * FilePath );
void DrawSurf ( int x, int y, SDL_Surface * Src, SDL_Rect * Clip, SDL_Surface * Dst );
SDL_Surface * ScreenSet ( int Width, int Height, int Bits, SDL_Surface * Screen );

struct xyz {
	int x, y, z;
} CursorPos, Camera;

/*** Thanks to <https://gist.github.com/jordwest/8a12196436ebcf8df98a2745251915b5> for the maths! ***/

struct xyz OrthoToIso ( int x, int z, int Multiply ) {
	struct xyz xyz;
	xyz.x = x * 1 * 0.5 * Multiply + z * -1 * 0.5 * Multiply;
	xyz.z = x * 0.5 * 0.5 * Multiply + z * 0.5 * 0.5 * Multiply;
	return xyz;
}

/*** ******************************************************************************************** ***/

