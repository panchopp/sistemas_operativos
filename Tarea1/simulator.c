#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Process
{
  int pid;
	char *name;
  int state;
	int priority;
  int N;
  int* tareas;
  int pos_tarea_actual;
  int num_veces_elegido;
  int num_veces_bloqueado;

  int start_time_in_cpu;
  int start_time_in_waiting;
  int start_time_in_ready;

  int start_turnaround_time;
  int start_response_time;
  int start_waiting_time;
  int total_turnaround_time;
  int total_response_time;
  int total_waiting_time;

};
typedef struct Process Process;

Process* process_init(int pid, char *name, int state, int priority, int N, int* tareas)
{
	Process* ret = malloc(sizeof(*ret));
	ret -> pid = pid;
	ret -> name = name;
  ret -> state = state;
  ret -> N = N;
  ret -> tareas = tareas;
  ret -> pos_tarea_actual = 0;
  ret -> priority = priority;
  ret -> total_turnaround_time = 0;
  ret -> total_response_time = 0;
  ret -> total_waiting_time = 0;
  ret -> num_veces_elegido = 0;
  ret -> num_veces_bloqueado = 0;
	return ret;
}

struct Node
{
  Process* proceso;
	struct Node* next;

};
typedef struct Node Node;

struct Queue
{
  Node* first;
	Node* last;

};
typedef struct Queue Queue;

Node* node_init(Process* proceso)
{
    Node *ret = (Node*)malloc(sizeof(Node));
    ret->proceso = proceso;
    ret->next = NULL;
    return ret;
}

Queue* queue_init()
{
    Queue *ret = (Queue*)malloc(sizeof(Queue));
    ret->first = NULL;
    ret->last = NULL;
    return ret;
}

void free_queue(Queue* queue){
  //Pendiente
}

void free_node(Node* node){
  //Pendiente
}

void push_queue(Queue* queue, Process* proceso){
  Node* new_node = node_init(proceso);
  if (queue -> last == NULL){
    queue -> first = new_node;
    queue -> last = new_node;
  }
  else{
    (queue -> last) -> next = new_node;
    queue -> last = new_node;
  }
}

Process* pop_queue(Queue* queue){
  if (queue -> first == NULL){
    return NULL;
  }
  else{
    Node* node = queue -> first;
    queue -> first = node -> next;
    Process* ret = node -> proceso;
    free_node(node);
    return ret;
  }
}

void sort(Process* array[4], int size){

    int i;
    Process *array_pointer;
    Process *moving_pointer;
    Process *max_pointer;
    int max;
    int max_i;

    while(size > 0){

        max_pointer = *array;
        array_pointer = *array;
        max = -276447232;

        for(i = 0; i < size; i++){
            moving_pointer = array[i];
            if( moving_pointer -> priority >= max) {
                max = moving_pointer -> priority;
                max_pointer = moving_pointer;
                max_i = i;
            }
        }

        if (max_i != size - 1){
            Process aux = *array[size-1];
            array[size-1] = max_pointer;
            array[max_i] = &aux;
        }

        size--;

    }

}

void FCFS() // Tipo 1
{


}

void round_robin() // Tipo 2
{


}

void priority_based() // Tipo 3
{


}

void scheduler(Queue* Q_ready, Queue* Q_waiting, Queue* Q_terminated, const char* tipo){

  // Ordenar segun tipo
  Process* CPU;

  int t = 0;
  CPU = pop_queue(Q_ready);
  CPU -> start_time_in_cpu = t;
  while (Q_ready -> first != NULL && Q_waiting -> first != NULL){ // Se va a caer cuando el ultimo proceso este en la CPU
    int remaining_time_CPU = (CPU -> tareas[CPU -> pos_tarea_actual] + CPU -> start_time_in_cpu) - t;
    Process* first_waiting = Q_waiting -> first;
    int remaining_time_Q_waiting = (first_waiting -> tareas[first_waiting -> pos_tarea_actual] + first_waiting -> start_time_in_waiting) - t;

    // Si proxima tarea es sacar de la CPU
    if (remaining_time_CPU < remaining_time_Q_waiting) {
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      push_queue(Q_waiting, CPU);
      if (Q_ready -> first != NULL){
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
      }
      else if (remaining_time_CPU > remaining_time_Q_waiting){
        t += remaining_time_Q_waiting;
        Process* poped = pop_queue(Q_waiting);
        poped -> start_time_in_ready = t;
        push_queue(Q_ready, poped);

      }

    }


  }


}




int main(int argc, char const *argv[]) {
  const char* scheduler_type;
  const char* file_name;
  int quantum;
  Queue* Q_ready = queue_init();
  Queue* Q_waiting = queue_init();
  Queue* Q_terminated = queue_init();
  FILE* file;
  if (argv[1]){
    scheduler_type = argv[1];
  }
  if (argv[2]){
    file_name = argv[2];
    file = fopen(file_name, "r");
    int pid = 0;

    while (1){
      char name[256];
      int priority = 0;
      int initial_time = 0;
      int N = 0;
      int state = 1; // 0: Running, 1: Ready, 2: Waiting

      char word[1024];
      // Chequea si se acabaron los procesos por leer.
      if(fscanf(file, " %1023s", word) != 1){
        break;
      }
      strcpy(name, word);
      fscanf(file, " %1023s", word);
      priority = atoi(word);
      fscanf(file, " %1023s", word);
      initial_time = atoi(word);
      fscanf(file, " %1023s", word);
      N = atoi(word);
      printf("%s\n", name);
      printf("%s\n", name);
      printf("%d\n", priority);
      printf("%d\n", initial_time);
      printf("%d\n", N);
      int* tareas = calloc(2*N-1, sizeof(int));
      for(int i = 0; i < 2*N-1; i++){
        fscanf(file, " %1023s", word);
        tareas[i] = atoi(word);
        printf("%s\n", word);
      }
      if (initial_time > 0){
        state = 2;
      }
      Process* proceso = process_init(pid, name, state, priority, N, tareas);
      if (proceso -> state == 1){
        push_queue(Q_ready, proceso);
      }
      if (proceso -> state == 2){
        push_queue(Q_waiting, proceso);
      }
      pid += 1;
    }
  }

  if (argv[3]){
    quantum = atoi(argv[3]);
  }
  else{
    quantum = 0;
  }

  //scheduler(Q_ready, Q_waiting, Q_terminated, scheduler_type);

  return 0;
}
