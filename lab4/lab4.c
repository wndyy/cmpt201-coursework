#define _DEFAULT_SOURCE
#define _ISOC99_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct header {
  uint64_t size;
  struct header *next;
};

/* header: make a header size and next pointer */
struct header makeheader(uint64_t size, struct header *next) {
  struct header temp;

  temp.size = size;
  temp.next = next;
  return temp;
}

void print_out(char *format, void *data, size_t data_size) {
  char buf[64];

  ssize_t len =
      snprintf(buf, 64, format, data_size == sizeof(uint64_t) ? *(uint64_t *)data : *(void **)data);

  if (len < 0) {
    perror("snprintf");
  }
  write(STDOUT_FILENO, buf, len);
}

int main() {
  intptr_t inc = 256;
  char *init_break = sbrk(0);
  // void **init_break_print = (void **)&init_break;
  // print_out("Initial break: %p\n", init_break_print, sizeof(void **));
  // printf("Initial break: %p\n", init_break);
  char *new_break = sbrk(inc);
  if (new_break == (void *)-1) {
    perror("sbrk");
  }
  // printf("New break: %p\n", new_break);

  struct header *first_block = (struct header *)init_break;
  *first_block = makeheader(128, NULL);
  struct header *second_block = (struct header *)init_break + 128;
  *second_block = makeheader(128, first_block);

  print_out("first block: %p\n", (void **)&first_block, sizeof(void **));
  print_out("second block: %p\n", (void **)&second_block, sizeof(void **));

  print_out("first block size: %d\n", (uint64_t *)&first_block->size, sizeof(uint64_t));
  print_out("first block next: %p\n", (void **)&first_block->next, sizeof(void **));
  print_out("second block size: %d\n", (uint64_t *)&second_block->size, sizeof(uint64_t));
  print_out("second block next: %p\n", (uint64_t *)&second_block->next, sizeof(void **));

  memset(first_block + sizeof(struct header), 0, 128 - sizeof(struct header));
  memset(second_block + sizeof(struct header), 1, 128 - sizeof(struct header));
}
