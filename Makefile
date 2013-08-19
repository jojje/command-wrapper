EXE     = cwrapper
VPATH   = src
objects = command_wrapper.o file_utils.o string_utils.o common.o

#CFLAGS = -ggdb -m32 -g -fno-inline -fno-omit-frame-pointer -static-libgcc
CFLAGS  = -s -Os -Wall

all: $(EXE)

$(EXE): $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)

clean:
	rm -f *.exe *.o
