#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "LibMultiSpacc/MultiSpacc.h"
#include "TargetsConfigs.h"
#include "Blocks.h"
#include "Keys.h"
#include "Util.h"

#define swcase break; case

#define AppName "BloccSpacc"

MultiSpacc_Window *Window = NULL;
MultiSpacc_Surface *Screen = NULL;
MultiSpacc_Event Event;
#define GameTick 30

MultiSpacc_Surface *Cursorset = NULL;
#define CursorsNum 2
SDL_Rect Cursors[CursorsNum];

MultiSpacc_Surface *BlocksImg = NULL;

MultiSpacc_Surface *DebugMsg = NULL;
TTF_Font *DebugFont = NULL;
SDL_Color DebugTextColor = { 80, 80, 80 };

int SelectedBlock;
bool InGame, InTitleMenu, InInventory;
bool Quit, Recalc, DebugMode;

// <https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidetimeexamples.html>
Uint32 NextTickTime;
Uint32 CalcTimeLeft() {
	Uint32 Now;
	Now = MultiSpacc_GetTicks();
	if ( NextTickTime <= Now ) {
		return 0;
	} else {
		return NextTickTime - Now;
	};
};

struct UsedKeys {
	bool Up, Down, Left, Right, Above, Below;
	bool Place, Break;
	bool Esc, Inventory;
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
	//if ( MultiSpacc_Init( SDL_INIT_EVERYTHING ) != 0 ) {
	//	printf("[E] Error initializing Core.\n");
	//	return false;
	//};
	MultiSpacc_SurfaceConfig windowConfig = {0};
	Window = MultiSpacc_SetWindow( &windowConfig );
	Screen = MultiSpacc_GetWindowSurface( Window );
	if ( Screen == NULL ) {
		printf("[E] Error initializing Video System.\n");
		return false;
	};
	if( TTF_Init() != 0 ) {
		printf("[E] Error initializing Font System.\n");
		return false;
	};
	MultiSpacc_SetAppTitle( Window, AppName );
	MultiSpacc_SetAppIcon( Window, LoadImage( "Assets/Icon.png", Screen ) );
	return true;
}

bool LoadAssets() {
	bool Error = false;
	Cursorset = LoadImage( "Assets/Cursorset.png", Screen );
	if ( Cursorset == NULL ) {
		Error = true;
	};
	BlocksImg = LoadImage( "Assets/Blocks.png", Screen );
	if ( BlocksImg == NULL ) {
		Error = true;
	};
	DebugFont = TTF_OpenFont( "Assets/LiberationMono-Regular.ttf", 12 );
	if ( DebugFont == NULL ) {
		Error = true;
	};
	if ( Error ) {
		printf("[E] Error loading assets.\n");
		return false;
	};
	return true;
}

void MoveCursor( int Direction ) {
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
	if ( InGame && !InInventory ) {
		if ( UsedKeys.Esc ) {
			Quit = true;
		}
		if ( UsedKeys.Inventory ) {
			InInventory = true;
		}
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
	}
	else if ( InInventory ) {
		if ( UsedKeys.Esc || UsedKeys.Inventory ) {
			InInventory = false;
		}
		if ( UsedKeys.Left && SelectedBlock > 1 ) {
			SelectedBlock--;
		}
		if ( UsedKeys.Right && SelectedBlock < BlocksetNum-1 ) {
			SelectedBlock++;
		}
	}
	UsedKeys.Up = false;
	UsedKeys.Down = false;
	UsedKeys.Left = false;
	UsedKeys.Right = false;
	UsedKeys.Above = false;
	UsedKeys.Below = false;
	UsedKeys.Place = false;
	UsedKeys.Break = false;
	UsedKeys.Esc = false;
	UsedKeys.Inventory = false;
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

void DrawString( char Str[], int x, int y, TTF_Font* Font, SDL_Color Color ) {
	MultiSpacc_Surface *MsgSurf = TTF_RenderText_Blended( Font, Str, Color );
	DrawSurf( x, y, MsgSurf, NULL, Screen );
	SDL_FreeSurface( MsgSurf );
}

void DrawDebug() {
	char Str[128];

	snprintf( Str, sizeof(Str), "CursorPos:  x:%d  y:%d  z:%d", CursorPos.x, CursorPos.y, CursorPos.z );
	DrawString( Str, 8, 8, DebugFont, DebugTextColor );

	snprintf( Str, sizeof(Str), "Camera:  x:%d  y:%d  z:%d", Camera.x, Camera.y, Camera.z );
	DrawString( Str, 8, 20, DebugFont, DebugTextColor );

	struct xyz CursorCoords = OrthoToIso( CursorPos.x, CursorPos.y, CursorPos.z, 1 );
	snprintf( Str, sizeof(Str), "CursorCoords:  x:%d  y:%d  z:%d", CursorCoords.x, CursorCoords.y, CursorCoords.z );
	DrawString( Str, 8, 32, DebugFont, DebugTextColor );
}

void SetSuperflatMap(){
	for ( int z = 0; z < ChunksNum.z; z++ ) {
		for ( int x = 0; x < ChunksNum.x; x++ ) {
			Map[0][z][x] = 4;
		};
	};
	for ( int y = 1; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				Map[y][z][x] = 0;
			};
		};
	};
};

void SetRandomNoiseMap(){
	for ( int y = 0; y < ChunksNum.y; y++ ) {
		for ( int z = 0; z < ChunksNum.z; z++ ) {
			for ( int x = 0; x < ChunksNum.x; x++ ) {
				int r = rand() % BlocksetNum;
				if ( r == 1 ) { // Avoid block 1 (all white, hard to see) for testing
					r = 2;
				};
				Map[y][z][x] = r;
			};
		};
	};
};

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
};

bool GameLoop ( void *args ) {
	NextTickTime = MultiSpacc_GetTicks() + GameTick;
	while ( MultiSpacc_PollEvent( &Event ) ) {
		if ( Event.Type == SDL_QUIT ) {
			Quit = true;
		} else
		if ( Event.Type == SDL_KEYUP ) {
			Recalc = true;
			switch( Event.Key ){
				swcase KeyUp          : UsedKeys.Up        = true;
				swcase KeyRight       : UsedKeys.Right     = true;
				swcase KeyDown        : UsedKeys.Down      = true;
				swcase KeyLeft        : UsedKeys.Left      = true;
				swcase KeyAbove       : UsedKeys.Above     = true;
				swcase KeyBelow       : UsedKeys.Below     = true;
				swcase KeyPlace       : UsedKeys.Place     = true;
				swcase KeyBreak       : UsedKeys.Break     = true;
				swcase KeyEsc         : UsedKeys.Esc       = true;
				swcase KeyInventory   : UsedKeys.Inventory = true;
				swcase KeyDebug       : DebugMode = !DebugMode;
				swcase KeyGenFlatMap  : SetSuperflatMap();
				swcase KeyGenNoiseMap : SetRandomNoiseMap();
			};
		};
	};
	EventHandle();
	if ( Recalc ) {
		FillSurfRGB ( 0xFF, 0xFF, 0xFF, Screen );
		if ( InGame && !InInventory ) {
			SetCamera();
			DrawMap();
			DrawCursor();
			if ( DebugMode ) {
				DrawDebug();
			};
		};
		if ( InInventory ) {
			DrawInventory();
		};
		if ( !FlipScreen( Window ) ) {
			return 1;
		};
		Recalc = false;
	};
	//MultiSpacc_Sleep( CalcTimeLeft() );
	NextTickTime += GameTick;
	return !Quit;
}

int main( int argc, char *args[] ) {
	printf("[I] Starting!\n");
	srand( time( NULL ) );

	if ( !SysInit() ) {
		printf("[E] Error initializing System.\n");
		return 1;
	};
	if ( !LoadAssets() ) {
		printf("[E] Error loading assets.\n");
		return 1;
	};
	GameInit();
	MultiSpacc_SetMainLoop( GameLoop, NULL, NULL );

	printf("[I] Exiting!\n");
	//SDL_Quit();
	return 0;
};
