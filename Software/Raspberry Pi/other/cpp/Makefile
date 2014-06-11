CC = g++
CFLAGS = `pkg-config --libs opencv`

main: main.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o

