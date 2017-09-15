#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

int running_processes = 0;
int pids_workers[1024];

void INThandler(int);

void  INThandler(int sig)
{
  printf("\n");
  printf("-----Se apreto control c-----\n");
  if (running_processes == 0){
    printf("No running processes, terminating \n");
    exit(0);

  }
  else{
    printf("%i running processes, terminating process %i \n", running_processes, pids_workers[0]);
    kill(pids_workers[0], SIGKILL);
  }
}

int main(int argc, char const *argv[]) {

  signal(SIGINT, INThandler);

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

    strcpy(line, buf);

    word = strtok(line, " ");

    // Si ingresa exit, el programa termina
    if (strcmp(word, "exit")==0){
      printf("%s\n", "EXIT");
      exit(0);
    }
    // Si ingresa setPrompt, setea el prompt con el valor debido
    else if (strcmp(word, "setPrompt")==0){
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
      path = strtok(NULL," ");
    }
    else {

      // Aca se extraen los parametros extras (arbitrarios) y se crea el args que es la forma de pasarle los parametros a la funcion llamada
      int i = 0;
      while (word){
        args[i] = word;
        word = strtok(NULL, " ");
        i += 1;
      }


      args[i] = word;
      char* last_argument = args[i-1];
      int has_ampersand = 0;
      if (strstr(last_argument, "&") != NULL){
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

      for (int j = 0; j < n_veces_paralelo; j++){
        pid_t pid = fork();
        pids_workers[j] = pid;
        // Aca se ejecuta el proceso hijo con los parametros leidos del comando ingresado
        if (pid==0){ // Si es hijo
          running_processes += 1;
//          printf("En hijo, voy a ejecutar %s\n", args[0]);
          execv(args[0], args);
          //exit(0);
        }
        else if (pid > 0){ // Si es padre
          if (has_ampersand == 1){
            if (n_veces_paralelo == 1){
                //
            }
            else{
              waitpid(pids_workers[j], 0, 0);
            }
            //printf("En padre, con ampersand\n");
            //
          }
          else{
            //printf("En padre, sin ampersand\n");
            // Aca espera al hijo
            waitpid(pids_workers[j], 0, 0);
            running_processes -= 1;
          }
        }
        else{
          // printf("FAIL!!!\n");
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
