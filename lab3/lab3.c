#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *user_inputs[5];
  int input_count = 0;

  while (true) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    printf("Enter input: ");
    nread = getline(&line, &len, stdin);
    if (input_count > 4) {
      for (int i = 0; i < 4; i++) {
        user_inputs[i] = user_inputs[i + 1];
        // printf("inside the shuffler\n");
        // printf("user_inputs[%d] = %s\n", i, user_inputs[i + 1]);
      }
      input_count = 4;
    }
    line[strlen(line) - 1] = '\0';
    // printf("Your input was: %s\n", line);
    user_inputs[input_count] = line;
    // printf("input %d has been set to %s\n", input_count, line);
    // printf("confirm: user_inputs[%d] = %s\n", input_count, user_inputs[input_count]);
    /*if (user_inputs[0]) {
      printf("current first element: user_inputs[%d] = %s\n", 0, user_inputs[0]);
    }
    if (user_inputs[1]) {
      printf("current second element: user_inputs[%d] = %s\n", 1, user_inputs[1]);
    }*/
    input_count++;
    // printf("Start print\n");
    if (strcmp(line, "print") == 0) {
      // printf("You just requested a print.\n");
      for (int x = 0; x < 5; x++) {
        printf("%s\n", user_inputs[x]);
      }
    }
    // printf("End print\n");
  }
}
