#pragma once

#define		KeyUp			SDLK_UP
#define		KeyDown			SDLK_DOWN
#define		KeyLeft			SDLK_LEFT
#define		KeyRight		SDLK_RIGHT
#define		KeyDebug		SDLK_F3

#ifdef Target_PocketGo

#define		KeyEsc			SDLK_RCTRL		// Reset
#define		KeyInventory	SDLK_ESCAPE		// Select
#define		KeyGenFlatMap	SDLK_LSHIFT		// X
#define		KeyGenNoiseMap	SDLK_SPACE		// Y
#define		KeyAbove		SDLK_LALT		// A
#define		KeyBelow		SDLK_LCTRL		// B
#define		KeyPlace		SDLK_TAB		// L
#define		KeyBreak		SDLK_BACKSPACE	// R

#else

#define		KeyEsc			SDLK_ESCAPE
#define		KeyInventory	SDLK_i
#define		KeyGenFlatMap	SDLK_F6
#define		KeyGenNoiseMap	SDLK_F7
#define		KeyAbove		SDLK_LSHIFT
#define		KeyBelow		SDLK_LCTRL
#define		KeyPlace		SDLK_z
#define		KeyBreak		SDLK_x

#endif
