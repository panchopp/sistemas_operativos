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
  char* path = "";
  char* word;
  char* args[1024];


  while(1){
    printf ("Ingrese un comando: ");
    char line[1024];

    // Se lee comando ingresado por usuario
    scanf("%[^\n\r]", line);

    char buf[2048];
    snprintf(buf, sizeof buf, "%s%s%s", prompt, path, line);

    printf("Prompt before %s\n", prompt);
    strcpy(line, buf);

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
      char* last_argument = args[i-1];
      printf("Last argument: %s\n", last_argument);
      int has_ampersand = 0;
      if (strstr(last_argument, "&") != NULL){
        printf("Has ampersand!!!!!!\n");
        has_ampersand = 1;
      }

      int n_veces_paralelo = 1;
      char* n_veces_paralelo_string = strtok(last_argument, "&");
      if (has_ampersand == 1){
        if (strlen(last_argument) > 1){
          n_veces_paralelo = atoi(n_veces_paralelo_string);
        }
        else{
          //
        }
        args[i-1] = NULL;
      }

      int pids_workers[n_veces_paralelo];

      printf("dd\n");
      // Borramos argumento &numero

      printf("ee\n");

      for (int j = 0; j < n_veces_paralelo; j++){
        pid_t pid = fork();
        pids_workers[j] = pid;
        printf("%i\n", pid);
        // Aca se ejecuta el proceso hijo con los parametros leidos del comando ingresado
        if (pid==0){ // Si es hijo
          printf("En hijo, voy a ejecutar %s\n", args[0]);
          execv(args[0], args);
          //exit(0);
        }
        else if (pid > 0){ // Si es padre
          if (has_ampersand == 1){
            //printf("En padre, con ampersand\n");
            //waitpid(pids_workers[j], 0, 0);
          }
          else{
            //printf("En padre, sin ampersand\n");
            // Aca espera al hijo
            waitpid(pids_workers[j], 0, 0);
          }
        }
        else{
          printf("FAIL!!!\n");
        }
      }
    }
    // Esto deberiamos sacarlo, el exit y dejar que siga el loop y pida un nuevo comando, pero se cae nose porque :/
    //exit(0);
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { }

  }

  return 0;
}
