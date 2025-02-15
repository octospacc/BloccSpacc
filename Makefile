AppName = BloccSpacc
ExeSuffix = .run

Sources = $(wildcard Source/*.c Source/LibMultiSpacc/*.c)
CFlags = -O2
CC = gcc $(Defines)

ifndef Target
	Target = PC
endif

ifdef Target
	ifeq ($(Target), PC)
		Defines = -DTarget_PC
		MultiSpacc_Target = SDL20
	else ifeq ($(Target), Web)
		Defines = -DTarget_PC -DTarget_Web
		MultiSpacc_Target = Web
		AppName = Emscripten
		ExeSuffix = .js
		CC = emcc $(Defines)
	else ifeq ($(Target), PocketGo)
		ExeSuffix = .PocketGo.exe
		PathPrefix = /opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi
		MultiSpacc_Target = SDL12
		Defines = -DTarget_PocketGo
		CC = $(PathPrefix)-gcc $(Defines)
	endif
endif

ifeq ($(MultiSpacc_Target), SDL12)
	Defines += -DMultiSpacc_Target_SDL12
	LdFlags += -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
	Sources += $(wildcard Source/LibMultiSpacc/SDLCom/*.c Source/LibMultiSpacc/SDL12/*.c)
else ifeq ($(MultiSpacc_Target), SDL20)
	Defines += -DMultiSpacc_Target_SDL20
	LdFlags += -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	Sources += $(wildcard Source/LibMultiSpacc/SDLCom/*.c Source/LibMultiSpacc/SDL20/*.c)
else ifeq ($(MultiSpacc_Target), Web)
	Defines += -DMultiSpacc_Target_Web -DMultiSpacc_Target_SDL20 -DMultiSpacc_Target_SDLWeb
	LdFlags += -sWASM=1 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='["png"]' -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 --preload-file Assets/Blocks.png --preload-file Assets/Cursorset.png --preload-file Assets/Icon.png --preload-file Assets/LiberationMono-Regular.ttf
endif

Objects = $(Sources:.c=.o)

all All: $(AppName)

$(AppName): $(Objects)
	$(CC) $^ $(CFlags) $(LdFlags) -o $(AppName)$(ExeSuffix)

run Run: All
	./$(AppName)$(ExeSuffix)

clean Clean clear Clear:
	find -L . -name "*.o" -type f -delete
	rm -f $(AppName)$(ExeSuffix) $(AppName).*$(ExeSuffix) *.js *.wasm *.data
