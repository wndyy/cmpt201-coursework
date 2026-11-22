#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg)                                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0;
int client_id_counter = 1;

// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;
  int client_id;
};

void *handle_client(void *arg) {
  struct client_info *client = (struct client_info *)arg;

  // TODO: print the message received from client
  // TODO: increase total_message_count per message
  char buf[BUF_SIZE];
  size_t nbytes;
  ssize_t bytes_read;
  nbytes = sizeof(buf);

  while ((bytes_read = read(client->cfd, buf, nbytes)) > 0) {
    // Remove null terminator
    buf[bytes_read] = '\0';

    // Increase global total message count and store global count after incremented by thread
    pthread_mutex_lock(&count_mutex);
    total_message_count = total_message_count + 1;
    int thread_total = total_message_count;
    pthread_mutex_unlock(&count_mutex);

    printf("Msg #%4d; Client ID %d: %s", thread_total, client->cfd, buf);
  }

  if (bytes_read == 0) {
    printf("Ending thread for client %d\n", client->cfd);
  }
  if (bytes_read == -1) {
    handle_error("read");
  }
  if (close(client->cfd) == -1) {
    handle_error("close");
  }
  free(client);
  client = NULL;
  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  for (;;) {
    struct client_info *client = (struct client_info *)malloc(sizeof(struct client_info));
    if (client == NULL) {
      handle_error("malloc");
    }

    // get file descriptor
    client->cfd = accept(sfd, NULL, NULL);
    if (client->cfd == -1) {
      free(client);
      handle_error("accept");
    }

    // increment global client id count by one and assign to new client
    pthread_mutex_lock(&client_id_mutex);
    client_id_counter = client_id_counter + 1;
    client->client_id = client_id_counter;
    pthread_mutex_unlock(&client_id_mutex);

    printf("New client created! ID %d on socket FD %d\n", client->client_id, client->cfd);

    // TODO: create a new thread when a new connection is encountered
    // TODO: call handle_client() when launching a new thread, and provide
    // client_info

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, handle_client, client) != 0) {
      free(client);
      handle_error("pthread_create");
    }

    pthread_detach(client_thread);
  }

  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
