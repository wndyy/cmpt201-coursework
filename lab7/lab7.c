#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT 100

typedef struct {
  int line_number;
  int value;
} Input;

typedef struct {
  int line_number;
  int doubled_value;
} IntermediateInput;

typedef struct {
  int doubled_value;
  int line_numbers[MAX_INPUT];
  int count;
} Output;

void map(Input *input, IntermediateInput *intermediate_input);
void groupByKey(IntermediateInput *input, Output *output, int *result_count);
void reduce(Output *output);

int main() {
  Input input_data[MAX_INPUT];
  int input_size = 0;
  int value;

  // Read input values until "end" is encountered
  printf("Enter values (one per line). Type 'end' to finish:\n");
  while (input_size < MAX_INPUT) {
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      break;
    }
    if (sscanf(buffer, "%d", &value) == 1) {
      input_data[input_size].line_number = input_size + 1;
      input_data[input_size].value = value;
      input_size++;
    } else if (sscanf(buffer, "end") == 0) {
      break;
    } else {
      // printf("Invalid input. Please enter an integer or 'end' to finish.\n");
    }
  }

  // Step 1: Map phase

  IntermediateInput mapped_results[MAX_INPUT] = {0};

  for (int i = 0; i < input_size; i++) {
    map(&input_data[i], &mapped_results[i]);
    // printf("mapped_result: %d\n", mapped_results[i].doubled_value);
  }

  // Step 2: Grouping phase

  Output output_results[MAX_INPUT] = {0};
  int result_count = 0;

  for (int i = 0; i < input_size; i++) {
    // printf("insert into groupByKey: %d\n", mapped_results[i].doubled_value);
    groupByKey(&mapped_results[i], output_results, &result_count);
  }

  // Step 3: Reduce phase
  for (int i = 0; i < result_count; i++) {
    if (output_results[i].count > 0) {
      reduce(&output_results[i]);
    }
  }

  return 0;
}

void map(Input *input, IntermediateInput *intermediate_input) {
  // TODO:
  // Double the value of the input
  int calc_double = input->value * 2;
  intermediate_input->line_number = input->line_number;
  intermediate_input->doubled_value = calc_double;
}

void groupByKey(IntermediateInput *input, Output *output, int *result_count) {
  // TODO:
  // Group entries
  // If the doubled value exists in the output entries, add it to the output's line_numbers list
  // If it doesn't exist, add a new entry to the output
  int check = 0;
  for (int x = 0; x < *result_count; x++) {
    // printf("----input's doubled value: %d\n", output[x].doubled_value);
    if (output[x].doubled_value == input->doubled_value) {
      // printf("add to existing\n");
      output[x].line_numbers[output[x].count] = input->line_number;
      output[x].count = output[x].count + 1;
      check = 1;
    }
  }

  if (check == 0) {
    // printf("add new\n");
    Output new_output;
    new_output.doubled_value = input->doubled_value;
    new_output.count = 0;
    new_output.line_numbers[new_output.count] = input->line_number;
    new_output.count = new_output.count + 1;
    output[*result_count] = new_output;
    *result_count = *result_count + 1;
  }
  check = 0;
}

void reduce(Output *output) {
  // TODO:
  // Print the doubled number and line numbers
  printf("(%d, [", output->doubled_value);
  if (output->count > 1) {
    for (int y = 0; y < output->count - 1; y++) {
      printf("%d, ", output->line_numbers[y]);
    }
    printf("%d", output->line_numbers[output->count - 1]);
  } else {
    printf("%d", output->line_numbers[0]);
  }
  printf("])\n");
}
