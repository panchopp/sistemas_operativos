#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char const *argv[]) {

  printf("BLABLA%s\n", "");

  if (argv[1]){
    printf("Print adentro de prueba: %s\n", argv[1]);
  }
  if (argv[2]){
    printf("Print adentro de prueba segundo termino: %s\n", argv[2]);
  }
  exit(0);
}
