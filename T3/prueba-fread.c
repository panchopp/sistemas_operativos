#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>


int main(int argc, char const *argv[]) {

  FILE* file;
  int buffer[128];
  file = fopen("data.bin", "r");
  int SIZE = 5;

  size_t ret_code = fread(buffer, sizeof *buffer, SIZE, file); // reads an array of doubles
  if(ret_code == SIZE) {
      puts("Array read successfully, contents: ");
      for(int n = 0; n < SIZE; ++n) printf("%i ", buffer[n]);
      printf("\n");
  } else { // error handling
     if (feof(file))
        printf("Error reading test.bin: unexpected end of file\n");
     else if (ferror(file)) {
         perror("Error reading test.bin");
     }
  }

  fclose(file);

  return 0;
}
