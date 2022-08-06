AppName = BloccSpacc
ExeSuffix = .exe

Sources = $(wildcard Source/*.c)
CFlags = -O2
CC = gcc $(Defines)

ifndef Target
	Target = PC
endif

ifdef Target
	ifeq ($(Target), PC)
		Defines = -DTarget_PC
		MultiSpacc_Target = SDL20
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
	Sources += $(wildcard Source/LibMultiSpacc/*.c Source/LibMultiSpacc/SDL12/*.c)
else ifeq ($(MultiSpacc_Target), SDL20)
	Defines += -DMultiSpacc_Target_SDL20
	LdFlags += -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	Sources += $(wildcard Source/LibMultiSpacc/*.c Source/LibMultiSpacc/SDL20/*.c)
endif

Objects = $(Sources:.c=.o)

all: $(AppName)

$(AppName): $(Objects)
	$(CC) $^ $(CFlags) $(LdFlags) -o $(AppName)$(ExeSuffix)

clean:
	find -L . -name "*.o" -type f -delete
	rm -f $(AppName)$(ExeSuffix) $(AppName).*$(ExeSuffix)
