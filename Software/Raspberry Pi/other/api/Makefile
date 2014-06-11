CC = g++
CFLAGS = -L/usr/lib/uv4l/uv4lext/armv6l -I ./src/ -L ./ -lraspicam -luv4lext -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l' `pkg-config --cflags opencv` `pkg-config --libs opencv`
DEPS = raspivid.h

%.o: %c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o

