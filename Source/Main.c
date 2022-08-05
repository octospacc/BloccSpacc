#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "TargetsConfigs.h"
#include "Blocks.h"
#include "Keys.h"
#include "Util.h"

#define AppName "BloccSpacc"

SDL_Surface * Screen = NULL;
SDL_Event Event;
#define GameTick 30

SDL_Surface * Cursorset = NULL;
#define CursorsNum 2
SDL_Rect Cursors[CursorsNum];

SDL_Surface * BlocksImg = NULL;

SDL_Surface * DebugMsg = NULL;
TTF_Font * DebugFont = NULL;
SDL_Color DebugTextColor = { 80, 80, 80 };

int SelectedBlock;
bool InGame, InTitleMenu, InInventory;
bool Quit, Recalc, DebugMode;

// <https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidetimeexamples.html>
static Uint32 NextTickTime;
static Uint32 InputTickTime;
Uint32 CalcTimeLeft() {
	Uint32 Now;
	Now = SDL_GetTicks();
	if ( NextTickTime <= Now ) {
		return 0;
	} else {
		return NextTickTime - Now;
	}
}

struct UsedKeys {
	bool Up, Down, Left, Right, Above, Below;
	bool Place, Break;
} UsedKeys;

struct xyz CursorPos, Camera, ChunksNum;

// TODO: Proper map memory management lol
int BlocksNum = ChunkSize*ChunkSize*ChunkSize;
int Map[ChunkSize][ChunkSize][ChunkSize] = {};

struct xyz GetBlocksOnScreenNum() {
	struct xyz Num;
	Num.x = ScreenWidth / BlockSize;
	Num.z = ScreenHeight / BlockSize/2;
	return Num;
}

bool SysInit() {
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
	else if ( Direction == 2 && CursorPos.x < BlockSize * (ChunksNum.x - 1) ) { // Right
		CursorPos.x += BlockSize;
	}
	else if ( Direction == 4 && CursorPos.z < BlockSize * (ChunksNum.z - 1) ) { // Down
		CursorPos.z += BlockSize;
	}
	else if ( Direction == 6 && CursorPos.x > 0 ) { // Left
		CursorPos.x -= BlockSize;
	}
	else if ( Direction == 8 && CursorPos.y < BlockSize * (ChunksNum.y - 1) ) { // Above
		CursorPos.y += BlockSize;
	}
	else if ( Direction == 9 && CursorPos.y > 0 ) { // Below
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
		Map[CursorPos.y/BlockSize][CursorPos.z/BlockSize][CursorPos.x/BlockSize] = SelectedBlock;
	}
	if ( UsedKeys.Break ) {
		Map[CursorPos.y/BlockSize][CursorPos.z/BlockSize][CursorPos.x/BlockSize] = 0;
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

void DrawInventory() {
	DebugMsg = TTF_RenderText_Blended( DebugFont, "Inventory", DebugTextColor );
	DrawSurf( ScreenWidth/2 - 4*9, 8, DebugMsg, NULL, Screen );
	int s;
	int y = 0;
	for ( int i = 0; i < BlocksetNum-1; i++ ) {
		s += 1;
		int x = ScreenWidth/16 + s*BlockSize + s*BlockSize/4;
		if ( x >= ScreenWidth - BlockSize - BlockSize/4 ) {
			y += BlockSize + BlockSize/4;
			x = ScreenWidth/16;
			s = 0;
		}
		DrawSurf( x, y, BlocksImg, &Blocks[i+1].Img, Screen );
		if ( SelectedBlock == i+1 ) {
			int Margin = BlockSize/8;
			DrawOutlineRect( x-Margin, y-Margin, BlockSize+2*Margin, BlockSize+2*Margin, 4, 0x00, 0x00, 0x00, Screen );
		}
	}
}

void DrawMap() { // TODO: Reoptimize this to draw only visible blocks
	for ( int y = 0; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				struct xyz MapCoords = OrthoToIso ( x, y, z, BlockSize );
				DrawSurf(
					MapCoords.x - Camera.x - BlockSize/2,
					MapCoords.z - Camera.z - y*BlockSize/2,
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
	Camera.z = xyz.z - ScreenHeight*0.80;
}

int GetCursorState() {
/*
	if ( CursorPos.y < BlockSize * (ChunksNum.y - 1) ) {
		return -1;
	}
*/
	return 1;
}

void DrawCursor() {
	struct xyz CursorCoords = OrthoToIso ( CursorPos.x, CursorPos.y, CursorPos.z, 1 );
	int CursorState = GetCursorState();
	if ( CursorState != -1 ) {
		DrawSurf(
			CursorCoords.x - Camera.x - BlockSize/2,
			CursorCoords.z - Camera.z - BlockSize/2 - CursorPos.y/2,
			Cursorset, & Cursors [CursorState], Screen
		);
	}
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
				if ( r == 1 ) { // Avoid block 1 (all white, hard to see) for testing
					r = 2;
				}
				Map[y][z][x] = r;
			}
		}
	}
}

void GameInit() {
	for ( int i = 0; i < CursorsNum; i++ ) {
		Cursors [i].x = BlockSize * i;
		Cursors [i].y = 0;
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
	CursorPos.x = BlockSize * (ChunksNum.x - 1);
	CursorPos.y = BlockSize * (ChunksNum.y - 1);
	CursorPos.z = BlockSize * (ChunksNum.z - 1);

	SelectedBlock = 2;
	Recalc = true;
	InGame = true;
}

void KeyListen() {
	Uint8 * Keys = SDL_GetKeyState( NULL );
	if ( Keys [KeyUp] ) {
		UsedKeys.Up = 1;
	}
	if ( Keys [KeyDown] ) {
		UsedKeys.Down = 1;
	}
}

int main( int argc, char* args[] ) {
	printf("[I] Starting!\n");
	srand( time( NULL ) );

	if ( !SysInit() ) {
		printf("[E] Error initializing SDL.\n");
		return 1;
	}
	if ( !LoadAssets() ) {
		printf("[E] Error loading assets.\n");
		return 1;
	}
	GameInit();

	while ( !Quit ) {
		NextTickTime = SDL_GetTicks() + GameTick;
		while ( SDL_PollEvent( & Event ) ) {
			//Recalc = true;
			if ( Event.type == SDL_QUIT ) {
				Quit = true;
			}
			else if ( Event.type == SDL_KEYDOWN ) {
				Recalc = true;
			}
			else if ( Event.type == SDL_KEYUP ) {
				if ( Event.key.keysym.sym == KeyEsc ) {
					if ( InGame && !InInventory ) {
						Quit = true;
					}
					if ( InInventory ) {
						InInventory = false;
					}
				}
				else if ( Event.key.keysym.sym == KeyDebug ) {
					DebugMode = !DebugMode;
				}
				else if ( Event.key.keysym.sym == KeyInventory ) {
					if ( InInventory ) {
						InInventory = false;
					} else {
						InInventory = true;
					}
				}
				else if ( Event.key.keysym.sym == KeyGenFlatMap ) {
					SetSuperflatMap();
				}
				else if ( Event.key.keysym.sym == KeyGenNoiseMap ) {
					SetRandomNoiseMap();
				}
				else if ( Event.key.keysym.sym == KeyUp ) {
					//UsedKeys.Up = true;
				}
				else if ( Event.key.keysym.sym == KeyRight ) {
					UsedKeys.Right = true;
				}
				else if ( Event.key.keysym.sym == KeyDown ) {
					//UsedKeys.Down = true;
				}
				else if ( Event.key.keysym.sym == KeyLeft ) {
					UsedKeys.Left = true;
				}
				else if ( Event.key.keysym.sym == KeyAbove ) {
					UsedKeys.Above = true;
				}
				else if ( Event.key.keysym.sym == KeyBelow ) {
					UsedKeys.Below = true;
				}
				else if ( Event.key.keysym.sym == KeyPlace ) {
					UsedKeys.Place = true;
				}
				else if ( Event.key.keysym.sym == KeyBreak ) {
					UsedKeys.Break = true;
				}
			}
		}
		KeyListen();
		if ( InputTickTime % ( GameTick*4 ) == 0 ) {
			EventHandle();
			//Recalc = false;
		}
		if ( Recalc ) {
			FillSurfRGB ( 0xFF, 0xFF, 0xFF, Screen );
			if ( InGame && !InInventory ) {
				SetCamera();
				DrawMap();
				DrawCursor();
				if ( DebugMode ) {
					DrawDebug();
				}
			}
			if ( InInventory ) {
				DrawInventory();
			}
			if ( !FlipScreen( Screen ) ) {
				return 1;
			}
			Recalc = false;
		}
		printf("%d\n", InputTickTime);
		SDL_Delay( CalcTimeLeft() );
		NextTickTime += GameTick;
		InputTickTime += GameTick;
	}

	printf("[I] Exiting!\n");
	SDL_Quit();
	return 0;
}
