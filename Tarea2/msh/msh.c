#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>



int main(int argc, char const *argv[]) {

  char* prompt = "";
  int last_exit_code = 0;
  char* path = NULL;
  char* word;
  char line[1024];
  char* args[1024];

  while(1){
    printf ("Ingrese un comando: ");

    // Se lee comando ingresado por usuario
    scanf("%[^\n]s", line);
    word = strtok(line, " ");

    // Si ingresa exit, el programa termina
    if (strcmp(word, "exit")==0){
      printf("%s\n", "EXIT");
      exit(0);
    }
    // Si ingresa setPrompt, setea el prompt con el valor debido
    else if (strcmp(word, "setPrompt")==0){
      printf("%s\n", "setPrompt");
      prompt = strtok(NULL," ");
      int length_prompt = strlen(prompt);
      for (int i = 0; i < length_prompt; i++){
        if (prompt[i] == *"*"){
          // Si usuario ingresa * en el prompt, se reemplaza por el ultimo exit code
          char *first;
          char *second;
          char *search = "*";
          char exit_code_string[3] = "";
          sprintf(exit_code_string, "%d", last_exit_code);
          first = strtok(prompt, search);
          second = strtok(NULL, search);
          char* new_prompt = malloc(sizeof(char)*(strlen(prompt) + strlen(exit_code_string)));
          strcat(new_prompt, first);
          strcat(new_prompt, exit_code_string);
          strcat(new_prompt, second);
          prompt = new_prompt;
        }
      }
    }
    else if (strcmp(word, "setPath")==0){
      printf("%s\n", "setPath");
      path = strtok(NULL," ");
    }
    else {
      printf ("You entered: %s\n", word);
      printf ("prompt: %s\n", prompt);
      printf ("path: %s\n", path);

      // Aca se extraen los parametros extras (arbitrarios) y se crea el args que es la forma de pasarle los parametros a la funcion llamada
      int i = 0;
      while (word){
        printf("%s\n", "Entro while");
        args[i] = word;
        word = strtok(NULL, " ");
        i += 1;
      }
      args[i] = word;

      pid_t pid = fork();
      printf("%i\n", pid);
      //Aca se ejecuta el proceso hijo con los parametros leidos del comando ingresado
      if (pid==0){ // Si es hijo
        execv(args[0], args);
      }
      else{ // Si es padre
        // Aca espera al hijo
        waitpid(pid, 0, 0);
      }
    }
    // Esto deberiamos sacarlo, el exit y dejar que siga el loop y pida un nuevo comando, pero se cae nose porque :/
    exit(0);
  }

  return 0;
}
