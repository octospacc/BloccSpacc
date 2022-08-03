#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "Blocks.h"
#include "Util.h"

#define AppName "BloccSpacc"

#define ScreenBits 16
int ScreenWidth = 320;
int ScreenHeight = 320;

SDL_Surface * Screen = NULL;
SDL_Event Event;

SDL_Surface * Cursorset = NULL;
#define CursorsNum 2
SDL_Rect Cursors[CursorsNum];

SDL_Surface * BlocksImg = NULL;

SDL_Surface * DebugMsg = NULL;
TTF_Font * DebugFont = NULL;
SDL_Color DebugTextColor = { 80, 80, 80 };

bool Quit, DebugMode;

struct UsedKeys {
	bool Up, Down, Left, Right, Above, Below;
	bool Place, Break;
} UsedKeys;

struct xyz CursorPos, Camera, ChunksNum;

// TODO: Proper map memory management lol
int BlocksNum = ChunkSize*ChunkSize*ChunkSize; // ChunkSize * ChunksNum.z * ChunksNum.y * ChunksNum.x;
int Map[ChunkSize][ChunkSize][ChunkSize] = {};

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
	SDL_WM_SetCaption ( AppName, "Assets/Icon.png" );
	return true;
}

bool LoadAssets() {
	bool Error = false;
	Cursorset = LoadImage ( "Assets/Cursorset.png" );
	if ( Cursorset == NULL ) {
		Error = true;
	}
	BlocksImg = LoadImage ( "Assets/Blocks.png" );
	if ( BlocksImg == NULL ) {
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

void MoveCursor (int Direction) {
	if ( Direction == 0 && CursorPos.z > 0 ) { // Up
		CursorPos.z -= BlockSize;
	}
	if ( Direction == 2 && CursorPos.x < BlockSize * (ChunksNum.x - 1) ) { // Right
		CursorPos.x += BlockSize;
	}
	if ( Direction == 4 && CursorPos.z < BlockSize * (ChunksNum.z - 1) ) { // Down
		CursorPos.z += BlockSize;
	}
	if ( Direction == 6 && CursorPos.x > 0 ) { // Left
		CursorPos.x -= BlockSize;
	}
	if ( Direction == 8 && CursorPos.y < BlockSize * (ChunksNum.y - 1) ) { // Above
		CursorPos.y += BlockSize;
	}
	if ( Direction == 9 && CursorPos.y > 0 ) { // Below
		CursorPos.y -= BlockSize;
	}

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
		Map[CursorPos.y/BlockSize][CursorPos.z/BlockSize][CursorPos.x/BlockSize] = 0;
	}
	if ( UsedKeys.Break ) {
		Map[CursorPos.y/BlockSize][CursorPos.z/BlockSize][CursorPos.x/BlockSize] = 3;
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

void DrawMap( struct xyz ChunksNum ) {
	for ( int y = 0; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				struct xyz MapCoords = OrthoToIso ( x, y, z, BlockSize );
				DrawSurf(
					MapCoords.x - Camera.x - BlockSize/2,
					MapCoords.z - Camera.z - y*BlockSize/2, //- Camera.y,
					BlocksImg, & Blocks[Map[y][z][x]].Img, Screen
				);
			}
		}
	}
}

void SetCamera() {
	int x = ( CursorPos.x + BlockSize/2 );
	int y = ( CursorPos.y + BlockSize/2 );
	int z = ( CursorPos.z + BlockSize/2 );
	struct xyz xyz = OrthoToIso ( x, y, z, 1 );
    Camera.x = xyz.x - ScreenWidth/2;
    Camera.z = xyz.z - ScreenHeight/2;
}

void DrawCursor() {
	struct xyz CursorCoords = OrthoToIso ( CursorPos.x, CursorPos.y, CursorPos.z, 1 );
	DrawSurf(
		CursorCoords.x - Camera.x - BlockSize/2,
		CursorCoords.z - Camera.z - BlockSize/2 - CursorPos.y/2,
		Cursorset, & Cursors [1], Screen
	);
}

void DrawDebug() { // There's a memory leak somewhere here
	char Str[127];

	snprintf( Str, sizeof(Str), "CursorPos:  x:%d  y:%d  z:%d", CursorPos.x, CursorPos.y, CursorPos.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 8, DebugMsg, NULL, Screen );

	snprintf( Str, sizeof(Str), "Camera:  x:%d  y:%d  z:%d", Camera.x, Camera.y, Camera.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 20, DebugMsg, NULL, Screen );

	struct xyz CursorCoords = OrthoToIso ( CursorPos.x, CursorPos.y, CursorPos.z, 1 );
	snprintf( Str, sizeof(Str), "CursorCoords:  x:%d  y:%d  z:%d", CursorCoords.x, CursorCoords.y, CursorCoords.z );
	DebugMsg = TTF_RenderText_Blended( DebugFont, Str, DebugTextColor );
	DrawSurf( 8, 32, DebugMsg, NULL, Screen );
}

void SetSuperflatMap() {
	for ( int z = 0; z < ChunksNum.z; z++ ) {
		for ( int x = 0; x < ChunksNum.x; x++ ) {
			Map[0][z][x] = 4;
		}
	}
	for ( int y = 1; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				Map[y][z][x] = 0;
			}
		}
	}
}
void SetRandomNoiseMap() {
	for ( int y = 0; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				int r = rand() % BlocksetNum;
				if ( r == 1 ) {
					r = 2;
				}
				Map[y][z][x] = r;
			}
		}
	}
}

int main( int argc, char* args[] ) {
	printf("[I] Starting!\n");
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
		Blocks[i].Img.x = BlockSize * i;
		Blocks[i].Img.y = 0;
		Blocks[i].Img.w = BlockSize;
		Blocks[i].Img.h = BlockSize;
	}

	ChunksNum.x = ChunkSize;
	ChunksNum.y = ChunkSize;
	ChunksNum.z = ChunkSize;

	SetRandomNoiseMap();
	CursorPos.y = BlockSize * (ChunksNum.y - 1);

	while ( !Quit ) {
		while ( SDL_PollEvent( & Event ) ) {
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
		DrawMap( ChunksNum );
		DrawCursor();
		if ( DebugMode ) {
			DrawDebug();
		}

		if ( !FlipScreen( Screen ) ) {
			return 1;
		}
		SDL_Delay( 16 ); // TODO: proper framerate management
	}

	printf("[I] Exiting!\n");
	SDL_Quit();
	return 0;
}

