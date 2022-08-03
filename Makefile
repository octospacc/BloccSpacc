AppName = BloccSpacc
ExeSuffix = .exe

Sources = $(wildcard Source/*.c)
Objects = $(Sources:.c=.o)

Defines = -DTarget_PC
CFlags = -O2
LdFlags = -lSDL -lSDL_gfx -lSDL_image -lSDL_mixer -lSDL_ttf

CC = gcc $(Defines)

ifdef Target
	ifeq ($(Target), PocketGo)
		ExeSuffix = .PocketGo.exe
		PathPrefix = /opt/miyoo/bin/arm-miyoo-linux-uclibcgnueabi
		Defines = -DTarget_PocketGo
		CC = $(PathPrefix)-gcc $(Defines)
	endif
endif

all: $(AppName)

$(AppName): $(Objects)
	$(CC) $^ $(CFlags) $(LdFlags) -o $(AppName)$(ExeSuffix)

clean:
	rm -f Source/*.o $(AppName)$(ExeSuffix) $(AppName).*$(ExeSuffix)
