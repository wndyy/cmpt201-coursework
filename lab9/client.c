/*
Questions to answer at top of client.c:
(You should not need to change the code in client.c)
1. What is the address of the server it is trying to connect to (IP address and port number).

IP is 127.0.0.1; por number is 8000

2. Is it UDP or TCP? How do you know?

TCP. I see the connect() function and TCP servers require a connection before sending data.

3. The client is going to send some data to the server. Where does it get this data from? How can
you tell in the code?

Data is from standard input. It is using the read() function which calls the terminal for user
input. The stream for the read function is STDIN_FILENO. It takes this data and calls write(), using
the file descriptor from the socket connection to send the data.

4. How does the client program end? How can you tell that in the code?
The client program ends when there's an empty input from the user. An empty input still sends a
newline character (1 byte) so the function specifies greater than 1. The socket is then closed with
close(sfd) and exit(EXIT_SUCCESS).

There is also exit(EXIT_FAILURE) in handle_error function that ends the program whenever an error is
caught.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}
