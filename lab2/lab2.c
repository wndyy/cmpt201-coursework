#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  pid_t pid, w;
  int wstatus, execInt;

  // Execute forever
  while (true) {
    printf("Enter programs to run.\n> ");
    if ((nread = getline(&line, &len, stdin)) != -1) {
      pid = fork();
      if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
      }
      if (pid == 0) {
        line[strlen(line) - 1] = '\0';
        execInt = execlp(line, line, NULL);
        if (execInt == -1) {
          perror("execl");
          exit(EXIT_FAILURE);
        }
      } else {
        do {
          w = waitpid(pid, &wstatus, 0);
          if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
          }
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
      }
    }
    if (nread == -1) {
      perror("getline failed");
      exit(EXIT_FAILURE);
    }
  }
  free(line);
}
