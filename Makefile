
CC = gcc -Wall
CFLAGS = `pkg-config --cflags gtk+-2.0`
LIBS = `pkg-config --libs gtk+-2.0`


gpe-mplayer: main.o window.o list.o control.o output.o
	$(CC) -o gpe-mplayer main.o window.o list.o control.o output.o $(LIBS)

main.o: main.c window.h
	$(CC) $(CFLAGS) -c main.c

window.o: window.c window.h control.h list.h
	$(CC) $(CFLAGS) -c window.c

output.o: output.c output.h window.h
	$(CC) $(CFLAGS) -c output.c

control.o: control.c control.h output.h window.h
	$(CC) $(CFLAGS) -c control.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm -f *.o gpe-mplayer
