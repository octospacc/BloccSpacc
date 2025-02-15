#pragma once
#include "LibMultiSpacc/MultiSpacc.h"

#define BlocksetNum  8
#define BlockSize   32
#define ChunkSize   16

struct Block {
	int      Id;
	char     Name[63];
	SDL_Rect Img;
	int      Light;
	bool     Falling;
	bool     Fluid;
};

struct Block Blocks[BlocksetNum] = {
	{ 0, "Air",    {}, 0, false, false },
	{ 1, "White",  {}, 0, false, false },
	{ 2, "Black",  {}, 0, false, false },
	{ 3, "Gray",   {}, 0, false, false },
	{ 4, "Green",  {}, 0, false, false },
	{ 5, "Blue",   {}, 0, false, false },
	{ 6, "Purple", {}, 0, false, false },
	{ 7, "Red",    {}, 0, false, false },
};
