AppName = BloccSpacc
ExeName = $(AppName).exe
Sources = $(wildcard Source/*.c)
Objects = $(Sources:.c=.o)

CFlags = -O2
LdFlags = -lSDL -lSDL_gfx -lSDL_image -lSDL_mixer -lSDL_ttf

all: $(AppName)

$(AppName): $(Objects)
	$(CC) $^ $(CFlags) $(LdFlags) -o $(ExeName)

clean:
	rm Source/*.o $(ExeName)
