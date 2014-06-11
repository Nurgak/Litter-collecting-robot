#include <stdio.h>
#include <string.h>
#include <fcntl.h> // file reading/writing stuff
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

int fd;
char buff;

int main()
{
	printf("Opening\n");
	fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);

	/*struct termios tty;
	memset(&tty, 0, sizeof tty);

	//tcgetattr (fd, &tty );
	cfsetospeed(&tty, B9600);
	cfsetispeed(&tty, B9600);

	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;

	tty.c_iflag |= IGNPAR | IGNCR;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_lflag |= ICANON;

	tty.c_oflag &= ~OPOST;
	tty.c_lflag |= ICANON;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tty);*/

	//fcntl(fd, F_SETFL, 0);
        printf("Writing\n");
	write(fd, "v", 1);
        //printf("Reading\n");
	//usleep(1000);
	//read(fd, &buff, 1);
	//printf("Response: %s\n", buff);
	close(fd);
	return 0;
}
