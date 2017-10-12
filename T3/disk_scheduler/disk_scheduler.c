#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

void FCFS(int* queue, int queue_length){


}

void SSTF(int* queue, int queue_length){

  for(int i = 0; i < queue_length; i++){
    int best_candidate_pos = i+1;
    int best_candidate_value = queue[best_candidate_pos];
    int shortest_seek_time = abs(queue[i] - queue[best_candidate_pos]);
    for(int j = i+1; j < queue_length; j++){
      if (shortest_seek_time > abs(queue[i] - queue[j])){
        best_candidate_pos = j;
        best_candidate_value = queue[j];
        shortest_seek_time = abs(queue[i] - queue[j]);
      }
    }
    int aux = queue[i+1];
    queue[i+1] = queue[best_candidate_pos];
    queue[best_candidate_pos] = aux;
  }
}

void SCAN(int* queue, int queue_length){ // Comienza ordenando ascendentemente y luego descendentemente

  int cabezal_inicial = queue[0];
  int biggest = 0;
  for(int i = 0; i < queue_length; i++){
    if (queue[i] > biggest){
      biggest = queue[i];
    }
  }
  // Ordenar descendentemente
  for(int i = 0; i < queue_length; i++){
    for(int j = i+1; j < queue_length; j++){
      if (queue[j] > queue[i]){
        int aux = queue[i];
        queue[i] = queue[j];
        queue[j] = aux;
      }
    }
  }
  // Ordenar ascendentemente la primera parte
  for(int i = 0; i < queue_length; i++){
    for(int j = i+1; j < queue_length; j++){
      if (queue[j] < queue[i] && queue[j]>=cabezal_inicial){
        int aux = queue[i];
        queue[i] = queue[j];
        queue[j] = aux;
      }
    }
  }
}

void CLOOK(int* queue, int queue_length){

  int cabezal_inicial = queue[0];
  int biggest = 0;
  for(int i = 0; i < queue_length; i++){
    if (queue[i] > biggest){
      biggest = queue[i];
    }
  }
  // Ordenar ascendentemente
  for(int i = 0; i < queue_length; i++){
    for(int j = i+1; j < queue_length; j++){
      if (queue[j] > queue[i]){
        int aux = queue[i];
        queue[i] = queue[j];
        queue[j] = aux;
      }
    }
  }
  // Traer cabezal y mas grandes al comienzo
  for(int i = 0; i < queue_length; i++){
    for(int j = i+1; j < queue_length; j++){
      if (queue[i]<cabezal_inicial){
        int aux = queue[i];
        queue[i] = queue[j];
        queue[j] = aux;
      }

    }
  }
  // Ordenar ascendentemente la primera parte
  for(int i = 0; i < queue_length; i++){
    for(int j = i+1; j < queue_length; j++){
      if (queue[j] < queue[i] && queue[j] >= cabezal_inicial){
        int aux = queue[i];
        queue[i] = queue[j];
        queue[j] = aux;
      }
    }
  }
}

void count_movements(int* queue, int queue_length, int* seek_time, int* cambio_brazo){
  int dir_brazo = 1; // 1: Hacia numeros mas grandes, 0: Hacia numeros mas chicos
  for(int i = 1; i < queue_length; i++){
    *seek_time += abs(queue[i] - queue[i-1]);
    if (queue[i-1] < queue[i] && dir_brazo == 0){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
    }
    else if (queue[i-1] > queue[i] && dir_brazo == 1){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
    }
  }
}

void count_movements_FCFS(int* queue, int queue_length, int* seek_time, int* cambio_brazo, int cabezal_inicial){
  int dir_brazo = 1; // 1: Hacia numeros mas grandes, 0: Hacia numeros mas chicos
  *seek_time += abs(cabezal_inicial - queue[0]);
  if (cabezal_inicial > queue[0]){
    dir_brazo = 0;
    *cambio_brazo += 1;
  }
  else{
    dir_brazo = 1;
  }
  for(int i = 1; i < queue_length; i++){
    *seek_time += abs(queue[i] - queue[i-1]);
    if (queue[i-1] < queue[i] && dir_brazo == 0){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
    }
    else if (queue[i-1] > queue[i] && dir_brazo == 1){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
    }
  }
}

void count_movements_SCAN(int* queue, int queue_length, int* seek_time, int* cambio_brazo){
  int dir_brazo = 1; // 1: Hacia numeros mas grandes, 0: Hacia numeros mas chicos
  for(int i = 1; i < queue_length; i++){
    *seek_time += abs(queue[i] - queue[i-1]);
    if (queue[i-1] < queue[i] && dir_brazo == 0){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
      *seek_time += 2*abs(queue[i-1]);
    }
    else if (queue[i-1] > queue[i] && dir_brazo == 1){
      dir_brazo = 1 - dir_brazo;
      *cambio_brazo += 1;
      *seek_time += 2*abs(255-queue[i-1]);
    }
  }
}

void poner_cabezal_pos_inicial(int* queue, int queue_length, int cabezal_inicial){
  // Se pone cabezal en la posicion inicial
  for(int i = 0; i < queue_length; i++){
    if (queue[i] == cabezal_inicial){
      int aux = queue[0];
      queue[0] = cabezal_inicial;
      queue[i] = aux;
    }
  }
}



int main(int argc, char const *argv[]) {

  const char* scheduler_type = argv[1];
  const char* file_name = argv[2];
  FILE* file;
  file = fopen(file_name, "r");

  //Queue* queue = queue_init();

  // Se detecta la cantidad de elementos en la queue
  char line[1024];
  int num_accesos = 0;
  while(fscanf(file, "%s\n", line) != EOF){
    num_accesos += 1;
  }
  fclose(file);
  int queue_length = num_accesos -1;

  // Se vuelve a abrir archivo sabiendo la cantidad de lineas a leer

  int queue[queue_length];
  file = fopen(file_name, "r");


  int cabezal_inicial;
  fscanf(file, "%i\n", &cabezal_inicial);

  for (int i = 0; i < queue_length; i++){
    fscanf(file, "%s\n", line);
    queue[i] = atoi(line);
  }




  int seek_time = 0;
  int cambio_brazo = 0;
  // Scheduler ordena queue
  if (strcmp(scheduler_type, "fcfs")==0){
    FCFS(queue, queue_length);
    count_movements_FCFS(queue, queue_length, &seek_time, &cambio_brazo, cabezal_inicial);
  }
  else if (strcmp(scheduler_type, "sstf")==0){
    poner_cabezal_pos_inicial(queue, queue_length, cabezal_inicial);
    SSTF(queue, queue_length);
    count_movements(queue, queue_length, &seek_time, &cambio_brazo);
  }
  else if (strcmp(scheduler_type, "scan")==0){
    poner_cabezal_pos_inicial(queue, queue_length, cabezal_inicial);
    SCAN(queue, queue_length);
    count_movements_SCAN(queue, queue_length, &seek_time, &cambio_brazo);
  }
  else if (strcmp(scheduler_type, "c-look")==0){
    poner_cabezal_pos_inicial(queue, queue_length, cabezal_inicial);
    CLOOK(queue, queue_length);
    count_movements(queue, queue_length, &seek_time, &cambio_brazo);
  }

  for(int i = 0; i < queue_length; i++){
    printf("%i,", queue[i]);
  }
  printf("\n");
  printf("%i\n", seek_time);
  printf("%iT + %iD msec\n", seek_time, cambio_brazo);

  return 0;
}
