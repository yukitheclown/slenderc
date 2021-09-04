CC=gcc

# debugging on, source compiled into exe currently

CFLAGS  = -Wall -Wextra -g -pg -O2

LFLAGS = -g -pg -O2 -lc

# CFLAGS   = -m64 -pipe

SDL_LDFLAGS = $(shell sdl2-config --libs)

GLEW_LDFLAGS = $(shell pkg-config glew --static --libs)

OPENAL_LDFLAGS = $(shell pkg-config openal freealut --static --libs)

VORBIS_LDFLAGS = $(shell pkg-config vorbisfile --static --libs)

LIBS= -static-libgcc $(SDL_LDFLAGS) $(GLEW_LDFLAGS) -lfreetype -lpng  $(OPENAL_LDFLAGS) $(VORBIS_LDFLAGS) -Wl,-Bstatic -lz -lm

# LIBS = -Wl,-Bdynamic -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -Wl,-Bstatic -lpng16 -lz -lm -Llib/freetype/lib/win_cb/ -lfreetype

SOURCES=main.c image_loader.c hash_table.c framebuffers.c math.c svd.c text.c shaders.c window.c mesh.c skeletal_animation.c \
bounding_box.c octree.c world.c object.c skybox.c sound.c

INC_PATH = -I./lib/freetype -I./lib/FMOD/

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LIBS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $(INC_PATH) $< -o $@

# remember readelf -d main
# then just sudo find / -name 'whaever.so.0'