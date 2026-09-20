#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  char *token;

  printf("Please enter some text: ");
  while ((nread = getline(&line, &len, stdin)) != -1) {
    char *delim = " ";
    char *saveptr1;

    printf("Tokens:\n");
    while ((token = strtok_r(line, delim, &saveptr1))) {
      printf("\t%s\n", token);
      line = NULL;
    }
    printf("Please enter some text: ");
  }
  if (nread == -1) {
    perror("getline failed");
    exit(EXIT_FAILURE);
  }

  free(line);
}
