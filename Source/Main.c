#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "Util.h"

#define ScreenBits 16
int ScreenWidth = 256;
int ScreenHeight = 256;

SDL_Surface * Screen = NULL;
SDL_Event Event;

SDL_Surface * Cursorset = NULL;
#define CursorsNum 2
SDL_Rect Cursors[CursorsNum];

SDL_Surface * Blockset = NULL;
#define BlocksetNum 4
SDL_Rect Blocks[BlocksetNum];

SDL_Surface * DebugMsg = NULL;
TTF_Font * DebugFont = NULL;
SDL_Color DebugTextColor = { 80, 80, 80 };

#define BlockSize 32
#define ChunkSize 8

bool Quit, DebugMode;

struct UsedKeys {
	bool Up, Down, Left, Right, Above, Below;
	bool Place, Break;
} UsedKeys;

// TODO: Proper map memory management lol
int BlocksNum = 4096; // ChunkSize * ChunksNum.z * ChunksNum.y * ChunksNum.x;
int Map[4096];
int SelectedBlock;

struct xyz GetBlocksOnScreenNum() {
	struct xyz Num;
	Num.x = ScreenWidth / BlockSize;
	Num.z = ScreenHeight / BlockSize/2;
	return Num;
}

bool Init() {
	if ( SDL_Init ( SDL_INIT_EVERYTHING ) != 0 ) {
		printf("[E] Error initializing SDL.\n");
		return false;
	}
	Screen = ScreenSet ( ScreenWidth, ScreenHeight, ScreenBits, Screen );
	if ( Screen == NULL ) {
		printf("[E] Error initializing screen.\n");
		return false;
	}
	if( TTF_Init() != 0 ) {
		printf("[E] Error initializing SDL_TTF.\n");
		return false;
	}
	SDL_WM_SetCaption ( "Test", NULL );
	return true;
}

bool LoadAssets() {
	bool Error = false;
	Cursorset = LoadImage ( "Assets/Cursorset.png" );
	if ( Cursorset == NULL ) {
		Error = true;
	}
	Blockset = LoadImage ( "Assets/Blockset.png" );
	if ( Blockset == NULL ) {
		Error = true;
	}
	DebugFont = TTF_OpenFont ( "Assets/LiberationMono-Regular.ttf", 12 );
	if ( DebugFont == NULL ) {
		Error = true;
	}
	if ( Error ) {
		printf("[E] Error loading assets.\n");
		return false;
	}
	return true;
}

void MoveCursor (int Direction) { // Up, Right, Down, Left
	if ( Direction == 0 ) {
		CursorPos.z -= BlockSize;
	}
	if ( Direction == 2 ) {
		CursorPos.x += BlockSize;
	}
	if ( Direction == 4 ) {
		CursorPos.z += BlockSize;
	}
	if ( Direction == 6 ) {
		CursorPos.x -= BlockSize;
	}
	if ( Direction == 8 ) {
		CursorPos.y -= BlockSize;
	}
	if ( Direction == 9 ) {
		CursorPos.y += BlockSize;
	}
}

void SetCamera() {
	int x = ( CursorPos.x + BlockSize/2 );
	int y = ( CursorPos.y + BlockSize/2 );
	int z = ( CursorPos.z + BlockSize/2 );
	struct xyz xyz = OrthoToIso ( x, z, 1 );
    Camera.x = xyz.x - ScreenWidth/2;
    Camera.y = y - ScreenHeight/2;
    Camera.z = xyz.z - ScreenHeight/2;
}

void EventHandle() {
	if ( UsedKeys.Up ) {
		MoveCursor( 0 );
	}
	if ( UsedKeys.Right ) {
		MoveCursor( 2 );
	}
	if ( UsedKeys.Down ) {
		MoveCursor( 4 );
	}
	if ( UsedKeys.Left ) {
		MoveCursor( 6 );
	}
	if ( UsedKeys.Above) {
		MoveCursor( 8 );
	}
	if ( UsedKeys.Below ) {
		MoveCursor( 9 );
	}
	if ( UsedKeys.Place ) {
		Map[SelectedBlock] = 0;
	}
	if ( UsedKeys.Break ) {
		Map[SelectedBlock] = 3;
	}
	UsedKeys.Up = false;
	UsedKeys.Down = false;
	UsedKeys.Left = false;
	UsedKeys.Right = false;
	UsedKeys.Above = false;
	UsedKeys.Below = false;
	UsedKeys.Place = false;
	UsedKeys.Break = false;
}

void DrawMap( int * Map, struct xyz ChunksNum ) {
	struct xyz BlocksOnScreen = GetBlocksOnScreenNum();
	int i = 0;
	//for ( int Row = CursorPos.y/BlockSize - BlocksOnScreen.y*4; Row < CursorPos.y/BlockSize + BlocksOnScreen.y*4; Row++ ) {
		//for ( int Col = 0; Col < BlocksOnScreen.x*2; Col++ ) {
	for ( int c = 0; c < ChunkSize; c++ ) {
		for ( int y = 0; y < ChunksNum.y; y++ ) {
			for ( int z = 0; z < ChunksNum.z; z++ ) {
				for ( int x = 0; x < ChunksNum.x; x++ ) {
					struct xyz MapCoords = OrthoToIso ( x, z, BlockSize );
					int h = 0;
					DrawSurf(
						MapCoords.x - Camera.x - BlockSize/2,
						MapCoords.z + h - Camera.z - y*BlockSize/2 - Camera.y,
						Blockset,
						& Blocks[Map[i]],
						Screen
					);
					i++;
				}
			}
		}
	}
}

void DrawCursor() {
	struct xyz CursorCoords = OrthoToIso ( CursorPos.x, CursorPos.z, 1 );
	DrawSurf ( CursorCoords.x - BlockSize/2 - Camera.x, CursorCoords.z - BlockSize/2 - Camera.z, Cursorset, & Cursors [1], Screen );
}

void DrawDebug() { // There's a memory leak somewhere here
	char Str[127];

	snprintf( Str, sizeof(Str), "CursorPos:  x:%d  y:%d  z:%d", CursorPos.x, CursorPos.y, CursorPos.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 8, DebugMsg, NULL, Screen );

	snprintf( Str, sizeof(Str), "Camera:  x:%d  y:%d  z:%d", Camera.x, Camera.y, Camera.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 20, DebugMsg, NULL, Screen );

	struct xyz CursorCoords = OrthoToIso ( CursorPos.x, CursorPos.z, 1 );
	snprintf( Str, sizeof(Str), "CursorCoords:  x:%d  y:%d  z:%d", CursorCoords.x, CursorCoords.y, CursorCoords.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 32, DebugMsg, NULL, Screen );
}

int FlipScreen() {
	if ( SDL_Flip ( Screen ) == -1 ) {
		printf("[E] Error updating screen.\n");
		return 1;
	}
	SDL_FillRect ( Screen, &Screen -> clip_rect, SDL_MapRGB ( Screen -> format, 0xFF, 0xFF, 0xFF ) );
	return 0;
}

void SetSuperflatMap() {
	for ( int i=0; i < BlocksNum; i++ ) {
		Map[i] = 2;
	}
}
void SetRandomNoiseMap() {
	for ( int i=0; i < BlocksNum; i++ ) {
		int r = rand() % BlocksetNum;
		if ( r == 1 ) {
			r = 2;
		}
		Map[i] = r;
	}
}

int main ( int argc, char* args[] ) {
	srand( time( NULL ) );

	if ( !Init() ) {
		printf("[E] Error initializing SDL.\n");
		return 1;
	}
	if ( !LoadAssets() ) {
		printf("[E] Error loading assets.\n");
		return 1;
	}

	for ( int i = 0; i < CursorsNum; i++ ) {
		Cursors [i].x = 0;
		Cursors [i].y = BlockSize * i;
		Cursors [i].w = BlockSize;
		Cursors [i].h = BlockSize;
	}
	for ( int i = 0; i < BlocksetNum; i++ ) {
		Blocks [i].x = BlockSize * i;
		Blocks [i].y = 0;
		Blocks [i].w = BlockSize;
		Blocks [i].h = BlockSize;
	}

	struct xyz ChunksNum;
	ChunksNum.x = ChunkSize;
	ChunksNum.y = ChunkSize;
	ChunksNum.z = ChunkSize;

	SetRandomNoiseMap();

	while ( !Quit ) {
		while ( SDL_PollEvent ( & Event ) ) {
			if ( Event.type == SDL_QUIT ) {
				Quit = true;
			}
			else if ( Event.type == SDL_KEYDOWN ) {
				if ( Event.key.keysym.sym == SDLK_ESCAPE ) {
					Quit = true;
				}
			}
			else if ( Event.type == SDL_KEYUP ) {
				if ( Event.key.keysym.sym == SDLK_F3 ) {
					DebugMode = !DebugMode;
				}
				if ( Event.key.keysym.sym == SDLK_F6 ) {
					SetSuperflatMap();
				}
				if ( Event.key.keysym.sym == SDLK_F7 ) {
					SetRandomNoiseMap();
				}
				
				if ( Event.key.keysym.sym == SDLK_UP ) {
					UsedKeys.Up = true;
				}
				if ( Event.key.keysym.sym == SDLK_RIGHT ) {
					UsedKeys.Right = true;
				}
				if ( Event.key.keysym.sym == SDLK_DOWN ) {
					UsedKeys.Down = true;
				}
				if ( Event.key.keysym.sym == SDLK_LEFT ) {
					UsedKeys.Left = true;
				}
				if ( Event.key.keysym.sym == SDLK_LSHIFT ) {
					UsedKeys.Above = true;
				}
				if ( Event.key.keysym.sym == SDLK_LCTRL ) {
					UsedKeys.Below = true;
				}

				if ( Event.key.keysym.sym == SDLK_z ) {
					UsedKeys.Place = true;
				}
				if ( Event.key.keysym.sym == SDLK_x ) {
					UsedKeys.Break = true;
				}
			}
		}
		EventHandle();
		SetCamera();
		DrawMap( Map, ChunksNum );
		DrawCursor();
		SelectedBlock = CursorPos.x + 4032 + CursorPos.z;
		if ( DebugMode ) {
			DrawDebug();
		}

		if ( FlipScreen() != 0 ) {
			return 1;
		}
		SDL_Delay( 16 ); // TODO: proper framerate management
	}

	printf("[I] Exiting!\n");
	SDL_Quit();
	return 0;
}
