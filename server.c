/*
Questions to answer at top of server.c:
(You should not need to change client.c)
Understanding the Client:
1. How is the client sending data to the server? What protocol?
The client is sending data by creating a stream socket using the AF_INET format which uses IPv4
Internet protocols. It is configured using the default protocol which is TCP (argument 0).It uses
the connect() function to connect to ADDR "127.0.0.1" and sends data by writing to the file
descriptor variable sfd (created by socket).

2. What data is the client sending to the server?
The client created a buf variable that is 1024-byte in size. Then it copies in, one at a time, a
string from an array of strings ("Hello", "Apple", "Car", "Green", "Dog"). Strncpy pads the rest of
the string with zeroes.

Understanding the Server:
1. Explain the argument that the `run_acceptor` thread is passed as an argument.
In the main() function, an acceptor_args struct is created with these fields and values:
- run: true; a flag that keeps while loops running in run_client and run_acceptor. True means
something different in each thread:
-- run_acceptor thread while loop keeps accepting connections/clients
-- The run_client threads while loops keeps taking in words and adds them to the shared linked list
(on main function stack)
- list_handle: &list_handle; pointer to the shared linked list to store the words
-  list_lock: &list_lock; mutex lock to lock shared linked list while reading and writing to prevent
race conditions
2. How are received messages stored?
In a singly linked list which is stored as a local variable pointer to struct list_handle on the
main() function stack. The list is passed as a pointer to the pthread which has the fields for the
last node and the number of nodes. When there's a new message, run_client creates a new node and
adds it to the list.
3. What does `main()` do with the received messages?
Main waits until enough messages are sent and then it sets the run flag in the accepter thread to
false which ends the acceptor while loop and joins all the client threads. It then checks if the
maximum messages were received and then runs collect all on the linked list which prints each
message and frees each node. It then counts how many were collected and checks if it matches the
maximum and prints if all the messages were collected.
4. How are threads used in this sample code?
- The main() function starts the acceptor thread by passing the acceptor_args struct. It changes the
run flag to false when the maximum number of messages have been received which ends the while loop
inside the acceptor thread and joins all the client threads. The main function then joins the
acceptor thrread.
- The acceptor thread initiates a listening socket and then accepts new connections up to the
maximum. A Pthread is then created for each client with their own file descriptor and a pointer to
the shared linked list and shared mutex lock.
- The client threads recieves messages and then adds them to the shared linked list.

Task 2: Server implementation, Question 5
1. How are sockets made non-blocking?
Using set_non_blocking()
2. What sockets are made non-blocking?
The acceptor's listening socket and every socket from client connections
3. Why are these sockets made non-blocking? What purpose does it serve?
The sockets are made non-blocking so that the accept() and read() functions can return right away
and check the running condition in the while loop. Once the run condition is false, the loop can end
and then join the threads.

*/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUF_SIZE 1024
#define PORT 8001
#define LISTEN_BACKLOG 32
#define MAX_CLIENTS 4
#define NUM_MSG_PER_CLIENT 5

#define handle_error(msg)                                                                          \
  do {                                                                                             \
    perror(msg);                                                                                   \
    exit(EXIT_FAILURE);                                                                            \
  } while (0)

struct list_node {
  struct list_node *next;
  void *data;
};

struct list_handle {
  struct list_node *last;
  volatile uint32_t count;
};

struct client_args {
  atomic_bool run;

  int cfd;
  struct list_handle *list_handle;
  pthread_mutex_t *list_lock;
};

struct acceptor_args {
  atomic_bool run;

  struct list_handle *list_handle;
  pthread_mutex_t *list_lock;
};

int init_server_socket() {
  struct sockaddr_in addr;

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
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

  return sfd;
}

// Set a file descriptor to non-blocking mode
void set_non_blocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL");
    exit(EXIT_FAILURE);
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    exit(EXIT_FAILURE);
  }
}

void add_to_list(struct list_handle *list_handle, struct list_node *new_node) {
  struct list_node *last_node = list_handle->last;
  last_node->next = new_node;
  list_handle->last = last_node->next;
  list_handle->count++;
}

int collect_all(struct list_node head) {
  struct list_node *node = head.next; // get first node after head
  uint32_t total = 0;

  while (node != NULL) {
    printf("Collected: %s\n", (char *)node->data);
    total++;

    // Free node and advance to next item
    struct list_node *next = node->next;
    free(node->data);
    free(node);
    node = next;
  }

  return total;
}

static void *run_client(void *args) {
  struct client_args *cargs = (struct client_args *)args;
  int cfd = cargs->cfd;
  set_non_blocking(cfd);

  char msg_buf[BUF_SIZE];

  while (cargs->run) {
    ssize_t bytes_read = read(cfd, &msg_buf, BUF_SIZE);
    if (bytes_read == -1) {
      if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
        perror("Problem reading from socket!\n");
        break;
      }
    } else if (bytes_read > 0) {
      // Create node with data
      struct list_node *new_node = malloc(sizeof(struct list_node));
      new_node->next = NULL;
      new_node->data = malloc(BUF_SIZE);
      memcpy(new_node->data, msg_buf, BUF_SIZE);

      struct list_handle *list_handle = cargs->list_handle;
      // TODO: Safely use add_to_list to add new_node to the list
      pthread_mutex_lock(cargs->list_lock);
      add_to_list(cargs->list_handle, new_node);
      pthread_mutex_unlock(cargs->list_lock);
    }
  }

  if (close(cfd) == -1) {
    perror("client thread close");
  }
  return NULL;
}

static void *run_acceptor(void *args) {
  int sfd = init_server_socket();
  set_non_blocking(sfd);

  struct acceptor_args *aargs = (struct acceptor_args *)args;
  pthread_t threads[MAX_CLIENTS];
  struct client_args client_args[MAX_CLIENTS];

  printf("Accepting clients...\n");

  uint16_t num_clients = 0;
  while (aargs->run) {
    if (num_clients < MAX_CLIENTS) {
      int cfd = accept(sfd, NULL, NULL);
      if (cfd == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
          handle_error("accept");
        }
      } else {
        printf("Client connected!\n");

        client_args[num_clients].cfd = cfd;
        client_args[num_clients].run = true;
        client_args[num_clients].list_handle = aargs->list_handle;
        client_args[num_clients].list_lock = aargs->list_lock;
        num_clients++;

        // TODO: Create a new thread to handle the client
        pthread_create(&threads[num_clients], NULL, run_client, &client_args[num_clients]);
        num_clients++;
        printf("Client connected!\n");
      }
    }
  }

  printf("Not accepting any more clients!\n");

  // Shutdown and cleanup
  for (int i = 0; i < num_clients; i++) {
    // TODO: Set flag to stop the client thread
    // TODO: Wait for the client thread and close its socket
    client_args[i].run = false;
    pthread_join(threads[i], NULL);
    close(client_args[i].cfd);
  }

  if (close(sfd) == -1) {
    perror("closing server socket");
  }
  return NULL;
}

int main() {
  pthread_mutex_t list_mutex;
  pthread_mutex_init(&list_mutex, NULL);

  // List to store received messages
  // - Do not free list head (not dynamically allocated)
  struct list_node head = {NULL, NULL};
  struct list_node *last = &head;
  struct list_handle list_handle = {
      .last = &head,
      .count = 0,
  };

  pthread_t acceptor_thread;
  struct acceptor_args aargs = {
      .run = true,
      .list_handle = &list_handle,
      .list_lock = &list_mutex,
  };
  pthread_create(&acceptor_thread, NULL, run_acceptor, &aargs);

  // TODO: Wait until enough messages are received
  int max_received = 0;
  while (max_received == 0) {
    pthread_mutex_lock(&list_mutex);
    if (list_handle.count >= MAX_CLIENTS * NUM_MSG_PER_CLIENT) {
      max_received = 1;
    }
    pthread_mutex_unlock(&list_mutex);
  }

  aargs.run = false;
  pthread_join(acceptor_thread, NULL);

  if (list_handle.count != MAX_CLIENTS * NUM_MSG_PER_CLIENT) {
    printf("Not enough messages were received!\n");
    return 1;
  }

  int collected = collect_all(head);
  printf("Collected: %d\n", collected);
  if (collected != list_handle.count) {
    printf("Not all messages were collected!\n");
    return 1;
  } else {
    printf("All messages were collected!\n");
  }

  pthread_mutex_destroy(&list_mutex);

  return 0;
}
