#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 256

void read_file(char *dst) {
    FILE *input;
    char line[MAX_LINE_LENGTH];

    input = fopen(dst, "r");

    if (input == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        printf("%s\n", line);
    }

    fclose(input);
}

int main(int argc, char **argv) {
  read_file(argv[1]);
  return EXIT_SUCCESS;
}
