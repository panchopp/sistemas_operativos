#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

void INThandler(int);

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
  int end_response_time;
  int start_waiting_time;
  int end_turnaround_time;
  int total_response_time;
  int total_waiting_time;

};
typedef struct Process Process;

Process* process_init(int pid, char *name, int state, int priority, int N, int* tareas)
{
	Process* ret = (Process*)malloc(sizeof(Process));
	ret -> pid = pid;
	ret -> name = name;
  ret -> state = state;
  ret -> N = N;
  ret -> tareas = tareas;
  ret -> pos_tarea_actual = 0;
  ret -> priority = priority;
  ret -> end_turnaround_time = 0;
  ret -> end_response_time = 0;
  ret -> total_waiting_time = 0;
  ret -> num_veces_elegido = 0;
  ret -> num_veces_bloqueado = 0;
  ret -> start_time_in_cpu = 0;
  ret -> start_time_in_waiting = 0;
  ret -> start_time_in_ready = 0;
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

Queue* Q_ready;
Queue* Q_waiting;
Queue* Q_terminated;
int t;

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
    if (queue ->first == NULL){
      queue -> last = NULL;
    }
    free_node(node);
    return ret;
  }
}


int find_min_waiting_time_left_in_queue(Queue* queue, int t){
    Node* next = queue -> first;
    if (next == NULL){
      return 1000000;
    }
    int min = next -> proceso -> tareas[next -> proceso -> pos_tarea_actual] - (t - next -> proceso -> start_time_in_waiting);
    Node* node_min = next;
    int min_is_first = 1;
    Node* node_before_min;
    Node* actual_node;
    while (next != NULL){

        if(next -> proceso -> tareas[next -> proceso -> pos_tarea_actual] - (t - next -> proceso -> start_time_in_waiting) < min){
            min = next -> proceso -> tareas[next -> proceso -> pos_tarea_actual] - (t - next -> proceso -> start_time_in_waiting);
            printf("%i\n", min);
            node_min = next;
            node_before_min = actual_node;
            min_is_first = 0;
        }
        actual_node = next;
        next = next -> next;
    }
    return node_min -> proceso -> tareas[node_min -> proceso -> pos_tarea_actual] - (t - node_min -> proceso -> start_time_in_waiting);
  }


Node* find_and_pop_min_in_queue(Queue* queue){
    Node* next = queue -> first;
    int min = next -> proceso -> priority;
    Node* node_min = next;
    int min_is_first = 1;
    Node* node_before_min;
    Node* actual_node;
    while (next != NULL){
        if(next -> proceso -> priority < min){
            min = next -> proceso -> priority;
            node_min = next;
            node_before_min = actual_node;
            min_is_first = 0;
        }

        actual_node = next;
        next = next -> next;

    }

    /* borro minimo de lista */
    if (min_is_first == 1){
        if (queue -> first -> next){
            queue -> first = queue -> first -> next;
        }
        else{
            queue -> first = NULL;
            queue -> last = NULL;
        }

    }

    else{
        node_before_min -> next = node_min -> next;
    }

    return node_min;
}

Queue* sort_by_priority(Queue* queue){
    Queue* queue_ordenada = queue_init();
    Node* node_max_priority;

    while (queue -> last != NULL){

        node_max_priority = find_and_pop_min_in_queue(queue);
        push_queue(queue_ordenada, node_max_priority -> proceso);
    }
    return queue_ordenada;

}

int len_queue(Queue* queue){
  Node* next = queue -> first;
  int largo = 0;
  while (next != NULL){
    largo ++;
    next = next -> next;
  }
  return largo;
}

void print_queue(Queue* queue){
  printf("Queue ");
  Node* actual_node = queue -> first;
  for(int i = 0; i < len_queue(queue); i++){
    printf(" %s", actual_node -> proceso -> name);
    actual_node = actual_node -> next;
  }
  printf("\n");
}

void sumar_tiempo_espera_procesos(Queue* q_waiting, Queue* q_ready, int tiempo_agregado){
  Node* actual = q_waiting -> first;
  while (actual != NULL){
    actual -> proceso -> total_waiting_time += tiempo_agregado;
    actual = actual -> next;
  }

  Node* actual2 = q_ready -> first;
  while (actual2 != NULL){
    actual2 -> proceso -> total_waiting_time += tiempo_agregado;
    actual2 = actual2 -> next;
  }

}

void find_parameter_processes_in_queue(Queue* queue, char* parameter){
  Node* next = queue -> first;
  char* p = "priority";
  char* nve = "num_veces_elegido";
  char* nvb = "num_veces_bloqueado";
  char* tat = "turnaround_time";
  char* rt = "response_time";
  char* wt = "waiting_time";
  while (next != NULL){
    if (parameter == p){
      printf("%d\n", next -> proceso -> priority);
    }
    if (parameter == nve){
      printf("Proceso %s fue elegido %d veces\n", next -> proceso -> name, next -> proceso -> num_veces_elegido);
    }
    if (parameter == nvb){
      printf("Proceso %s fue bloqueado %d veces\n", next -> proceso -> name, next -> proceso -> num_veces_bloqueado);
    }
    if (parameter == tat){
      printf("Proceso %s tuvo un turnaround time de %i\n", next -> proceso -> name, next -> proceso -> end_turnaround_time - next -> proceso -> start_turnaround_time);
    }
    if (parameter == rt){
      if (next -> proceso -> end_response_time > 0){
        printf("Proceso %s tuvo un response time de %i\n", next -> proceso -> name, next -> proceso -> end_response_time - next -> proceso -> start_turnaround_time);
      }
    }
    if (parameter == wt){
      printf("Proceso %s tuvo un waiting time de %i\n", next -> proceso -> name, next -> proceso -> total_waiting_time);
    }

    next = next -> next;
  }
}

void ready_up_waiters(Queue* q_waiting, Queue* q_ready, int t, int minimo_espera){
    Node* actual_node = q_waiting -> first;
    Node* previous = NULL;
    while (actual_node != NULL){

        if (actual_node -> proceso -> tareas[actual_node -> proceso -> pos_tarea_actual] - t + actual_node -> proceso -> start_time_in_waiting == minimo_espera){
            /* borrar elemento */

            if (previous == NULL){
                if (q_waiting -> first -> next){
                    q_waiting -> first = q_waiting -> first -> next;
                }
                else{
                    q_waiting -> first = NULL;
                    q_waiting -> last = NULL;
                }
            }
            else{
                previous -> next = actual_node -> next;
            }

            /* agregar elemento a ready */
            if (actual_node -> proceso -> pos_tarea_actual == 0){
              printf("Proceso %s creado\n", actual_node -> proceso -> name);
              actual_node -> proceso -> start_turnaround_time = t + minimo_espera;
            }
            actual_node -> proceso -> pos_tarea_actual ++;
            actual_node -> proceso -> start_time_in_ready = t + minimo_espera;
            push_queue(q_ready, actual_node -> proceso);


        }
        else{
          previous = actual_node;
        }
      actual_node = actual_node -> next;
    }
}

void ready_up_waitersrr(Queue* q_waiting, Queue* q_ready, int t, int minimo_espera){
    Node* actual_node = q_waiting -> first;
    Node* previous = NULL;
    while (actual_node != NULL){

        if (actual_node -> proceso -> tareas[actual_node -> proceso -> pos_tarea_actual] - t + actual_node -> proceso -> start_time_in_waiting == minimo_espera){
            /* borrar elemento */

            if (previous == NULL){
                if (q_waiting -> first -> next){
                    q_waiting -> first = q_waiting -> first -> next;
                }
                else{
                    q_waiting -> first = NULL;
                    q_waiting -> last = NULL;
                }
            }
            else{
                previous -> next = actual_node -> next;
            }

            /* agregar elemento a ready */
            if (actual_node -> proceso -> pos_tarea_actual == 0){
              printf("Proceso %s creado\n", actual_node -> proceso -> name);
              actual_node -> proceso -> start_turnaround_time = t + minimo_espera;
            }
            actual_node -> proceso -> pos_tarea_actual ++;
            actual_node -> proceso -> start_time_in_ready = t + minimo_espera;
            push_queue(q_ready, actual_node -> proceso);


        }
        else{
          previous = actual_node;
        }
      actual_node = actual_node -> next;
    }
}


void FCFS(Queue* Q_ready, Queue* Q_waiting, Queue* Q_terminated) // Tipo 1
{
  extern int t;
  t = 0;
  Process* CPU = NULL;

  while (Q_ready -> first != NULL || Q_waiting -> first != NULL || CPU != NULL){ // Se va a caer cuando el ultimo proceso este en la CPU
    if (CPU == NULL){
      if (Q_ready -> first != NULL){
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
      }
      else{
        int minimo_tiempo_waiting = find_min_waiting_time_left_in_queue(Q_waiting,t);
        sumar_tiempo_espera_procesos(Q_waiting, Q_ready, minimo_tiempo_waiting);
        ready_up_waiters(Q_waiting, Q_ready, t, minimo_tiempo_waiting);
        t += minimo_tiempo_waiting;
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
        printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      }
      printf("Proceso %s elgido para ejecutar en la CPU\n", CPU -> name);
    }

    int remaining_time_CPU = (CPU -> tareas[CPU -> pos_tarea_actual] + CPU -> start_time_in_cpu) - t;
    int remaining_time_first_waiting = find_min_waiting_time_left_in_queue(Q_waiting, t);

    // Si proxima accion es sacar de la CPU
    if (remaining_time_CPU < remaining_time_first_waiting) {
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      CPU -> pos_tarea_actual += 1;
      // Si se acabaron las tareas, se va a terminated, sino a waiting

      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);
      if (2*(CPU -> N) == CPU -> pos_tarea_actual){
        push_queue(Q_terminated, CPU);
        printf("Proceso %s termina su ejecucion\n", CPU -> name);
        CPU -> end_turnaround_time = t;
        CPU = NULL;
      }
      else{
        push_queue(Q_waiting, CPU);
        CPU -> num_veces_bloqueado += 1;
        CPU = NULL;
      }
    } // Si proxima accion es mover desde waiting a ready
    else if (remaining_time_CPU > remaining_time_first_waiting){
      ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_first_waiting);
      t += remaining_time_first_waiting;
    }
    else{ // Si proxima accion es mover desde waiting a ready y cambiar proceso de la CPU
      ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      CPU -> pos_tarea_actual += 1;
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);
      // Si se acabaron las tareas, se va a terminated, sino a waiting
      if (2*(CPU -> N) == CPU -> pos_tarea_actual){
        push_queue(Q_terminated, CPU);
        printf("Proceso %s termina su ejecucion\n", CPU -> name);
        CPU -> end_turnaround_time = t;
        ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU = NULL;
      }
      else{
        push_queue(Q_waiting, CPU);
        ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU -> num_veces_bloqueado += 1;
        CPU = NULL;

      }
    }
  }
  printf("-----Simulacion terminada-----\n");
  printf("%i procesos han terminado su ejecucion\n", len_queue(Q_terminated));
  printf("La simulacion duro %i\n", t);
  printf("Para cada proceso: \n");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_ready, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_ready, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_terminated, "turnaround_time");
  find_parameter_processes_in_queue(Q_terminated, "response_time");
  find_parameter_processes_in_queue(Q_ready, "response_time");
  find_parameter_processes_in_queue(Q_waiting, "response_time");
  find_parameter_processes_in_queue(Q_terminated, "waiting_time");
  find_parameter_processes_in_queue(Q_ready, "waiting_time");
  find_parameter_processes_in_queue(Q_waiting, "waiting_time");

}

double q(int p, int q){
  int pk;
  double Q;
  pk = (p * q) + (pow(-1,round(p/q)) * p);
  Q = pk/64;
  return Q;
}


void RR(Queue* Q_ready, Queue* Q_waiting, Queue* Q_terminated, int q_entregado) // Tipo 1
{
  extern int t;
  t = 0;
  Process* CPU = NULL;
  int L;

  while (Q_ready -> first != NULL || Q_waiting -> first != NULL || CPU != NULL){ // Se va a caer cuando el ultimo proceso este en la CPU
    if (CPU == NULL){
      if (Q_ready -> first != NULL){
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> tareas[CPU -> pos_tarea_actual] <= q(CPU -> priority, q_entregado)){
          L = CPU -> tareas[CPU -> pos_tarea_actual];
          CPU -> tareas[CPU -> pos_tarea_actual] = 0;
        }
        else{
          CPU -> tareas[CPU -> pos_tarea_actual] -= q(CPU -> priority, q_entregado);
          L = q(CPU -> priority, q_entregado);
        }

        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
      }
      else{
        int minimo_tiempo_waiting = find_min_waiting_time_left_in_queue(Q_waiting,t);
        sumar_tiempo_espera_procesos(Q_waiting, Q_ready, minimo_tiempo_waiting);
        ready_up_waitersrr(Q_waiting, Q_ready, t, minimo_tiempo_waiting);
        t += minimo_tiempo_waiting;
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> tareas[CPU -> pos_tarea_actual] <= q(CPU -> priority, q_entregado)){
          L = CPU -> tareas[CPU -> pos_tarea_actual];
          CPU -> tareas[CPU -> pos_tarea_actual] = 0;
        }
        else{
          CPU -> tareas[CPU -> pos_tarea_actual] -= q(CPU -> priority, q_entregado);
          L = q(CPU -> priority, q_entregado);
        }

        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
        printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      }
      printf("Proceso %s elgido para ejecutar en la CPU\n", CPU -> name);
    }

    int remaining_time_CPU = (L + CPU -> start_time_in_cpu) - t;
    int remaining_time_first_waiting = find_min_waiting_time_left_in_queue(Q_waiting, t);

    // Si proxima accion es sacar de la CPU
    if (remaining_time_CPU < remaining_time_first_waiting) {
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      if (CPU -> tareas[CPU -> pos_tarea_actual] == 0){
        if (2*(CPU -> N) == CPU -> pos_tarea_actual){
          push_queue(Q_terminated, CPU);
          printf("Proceso %s termina su ejecucion\n", CPU -> name);
          CPU -> end_turnaround_time = t;
          CPU = NULL;
        }
        else{
          push_queue(Q_waiting, CPU);
          CPU -> num_veces_bloqueado += 1;
          CPU -> pos_tarea_actual += 1;
          CPU = NULL;
        }
      }
      else{
        push_queue(Q_waiting, CPU);
        CPU -> start_time_in_ready = t;
        CPU = NULL;
      }
      // Si se acabaron las tareas, se va a terminated, sino a waiting

      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);

    } // Si proxima accion es mover desde waiting a ready
    else if (remaining_time_CPU > remaining_time_first_waiting){
      ready_up_waitersrr(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_first_waiting);
      t += remaining_time_first_waiting;
    }
    else{ // Si proxima accion es mover desde waiting a ready y cambiar proceso de la CPU
      ready_up_waitersrr(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      if (CPU -> tareas[CPU -> pos_tarea_actual] == 0){
        CPU -> pos_tarea_actual += 1;
      }
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);
      // Si se acabaron las tareas, se va a terminated, sino a waiting
      if (2*(CPU -> N) == CPU -> pos_tarea_actual){
        push_queue(Q_terminated, CPU);
        printf("Proceso %s termina su ejecucion\n", CPU -> name);
        CPU -> end_turnaround_time = t;
        ready_up_waitersrr(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU = NULL;
      }
      else{
        push_queue(Q_waiting, CPU);
        ready_up_waitersrr(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU -> num_veces_bloqueado += 1;
        CPU = NULL;

      }
    }
  }
  printf("-----Simulacion terminada-----\n");
  printf("%i procesos han terminado su ejecucion\n", len_queue(Q_terminated));
  printf("La simulacion duro %i\n", t);
  printf("Para cada proceso: \n");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_ready, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_ready, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_ready, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_terminated, "turnaround_time");
  find_parameter_processes_in_queue(Q_terminated, "response_time");
  find_parameter_processes_in_queue(Q_ready, "response_time");
  find_parameter_processes_in_queue(Q_waiting, "response_time");
  find_parameter_processes_in_queue(Q_terminated, "waiting_time");
  find_parameter_processes_in_queue(Q_ready, "waiting_time");
  find_parameter_processes_in_queue(Q_waiting, "waiting_time");

}

void priority(Queue* Q_ready, Queue* Q_waiting, Queue* Q_terminated) // Tipo 3
{
  extern int t;
  t = 0;
  Process* CPU = NULL;

  Q_waiting = sort_by_priority(Q_waiting);

  while (Q_ready -> first != NULL || Q_waiting -> first != NULL || CPU != NULL){ // Se va a caer cuando el ultimo proceso este en la CPU
    if (CPU == NULL){
      if (Q_ready -> first != NULL){
        Q_ready = sort_by_priority(Q_ready);
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
      }
      else{
        int minimo_tiempo_waiting = find_min_waiting_time_left_in_queue(Q_waiting,t);
        sumar_tiempo_espera_procesos(Q_waiting, Q_ready, minimo_tiempo_waiting);
        ready_up_waiters(Q_waiting, Q_ready, t, minimo_tiempo_waiting);
        t += minimo_tiempo_waiting;
        Q_ready = sort_by_priority(Q_ready);
        CPU = pop_queue(Q_ready);
        CPU -> start_time_in_cpu = t;
        CPU -> num_veces_elegido += 1;
        if (CPU -> num_veces_elegido == 1){
          CPU -> end_response_time = t;
        }
        printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      }
      printf("Proceso %s elgido para ejecutar en la CPU\n", CPU -> name);
    }

    int remaining_time_CPU = (CPU -> tareas[CPU -> pos_tarea_actual] + CPU -> start_time_in_cpu) - t;
    int remaining_time_first_waiting = find_min_waiting_time_left_in_queue(Q_waiting, t);

    // Si proxima accion es sacar de la CPU
    if (remaining_time_CPU < remaining_time_first_waiting) {
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      CPU -> pos_tarea_actual += 1;
      // Si se acabaron las tareas, se va a terminated, sino a waiting

      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);
      if (2*(CPU -> N) == CPU -> pos_tarea_actual){
        push_queue(Q_terminated, CPU);
        printf("Proceso %s termina su ejecucion\n", CPU -> name);
        CPU -> end_turnaround_time = t;
        CPU = NULL;
      }
      else{
        push_queue(Q_waiting, CPU);
        CPU -> num_veces_bloqueado += 1;
        CPU = NULL;
      }
    } // Si proxima accion es mover desde waiting a ready
    else if (remaining_time_CPU > remaining_time_first_waiting){
      ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_first_waiting);
      t += remaining_time_first_waiting;
    }
    else{ // Si proxima accion es mover desde waiting a ready y cambiar proceso de la CPU
      ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
      printf("Proceso activo %s lleva %i y le faltan %i intervalos de tiempo \n", CPU -> name, t - CPU -> start_time_in_cpu, CPU -> tareas[CPU -> pos_tarea_actual] - (t - CPU -> start_time_in_cpu));
      t += remaining_time_CPU;
      CPU -> start_time_in_waiting = t;
      CPU -> pos_tarea_actual += 1;
      sumar_tiempo_espera_procesos(Q_waiting, Q_ready, remaining_time_CPU);
      // Si se acabaron las tareas, se va a terminated, sino a waiting
      if (2*(CPU -> N) == CPU -> pos_tarea_actual){
        push_queue(Q_terminated, CPU);
        printf("Proceso %s termina su ejecucion\n", CPU -> name);
        CPU -> end_turnaround_time = t;
        ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU = NULL;
      }
      else{
        push_queue(Q_waiting, CPU);
        ready_up_waiters(Q_waiting, Q_ready, t, remaining_time_first_waiting);
        CPU -> num_veces_bloqueado += 1;
        CPU = NULL;

      }
    }
  }
  printf("-----Simulacion terminada-----\n");
  printf("%i procesos han terminado su ejecucion\n", len_queue(Q_terminated));
  printf("La simulacion duro %i\n", t);
  printf("Para cada proceso: \n");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_ready, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_ready, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_terminated, "turnaround_time");
  find_parameter_processes_in_queue(Q_terminated, "response_time");
  find_parameter_processes_in_queue(Q_ready, "response_time");
  find_parameter_processes_in_queue(Q_waiting, "response_time");
  find_parameter_processes_in_queue(Q_terminated, "waiting_time");
  find_parameter_processes_in_queue(Q_ready, "waiting_time");
  find_parameter_processes_in_queue(Q_waiting, "waiting_time");

}

void  INThandler(int sig)
{
  printf("\n");
  printf("-----Simulacion terminada-----\n");
  printf("%i procesos han terminado su ejecucion\n", len_queue(Q_terminated));
  printf("La simulacion duro %i\n", t);
  printf("Para cada proceso: \n");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_ready, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_elegido");
  find_parameter_processes_in_queue(Q_terminated, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_ready, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_waiting, "num_veces_bloqueado");
  find_parameter_processes_in_queue(Q_terminated, "turnaround_time");
  find_parameter_processes_in_queue(Q_terminated, "response_time");
  find_parameter_processes_in_queue(Q_ready, "response_time");
  find_parameter_processes_in_queue(Q_waiting, "response_time");
  find_parameter_processes_in_queue(Q_terminated, "waiting_time");
  find_parameter_processes_in_queue(Q_ready, "waiting_time");
  find_parameter_processes_in_queue(Q_waiting, "waiting_time");

  exit(0);
}


int main(int argc, char const *argv[]) {
  signal(SIGINT, INThandler);
  const char* scheduler_type;
  const char* file_name;
  int quantum;

  extern Queue* Q_ready;
  Q_ready = queue_init();
  extern Queue* Q_waiting;
  Q_waiting = queue_init();
  extern Queue* Q_terminated;
  Q_terminated = queue_init();

  FILE* file;
  if (argv[1]){
    scheduler_type = argv[1];
  }
  if (argv[2]){
    file_name = argv[2];
    file = fopen(file_name, "r");
    int pid = 0;

    while (1){
      int priority = 0;
      int initial_time = 0;
      int N = 0;
      int state = 1; // 0: Running, 1: Ready, 2: Waiting

      char word[1024];
      // Chequea si se acabaron los procesos por leer.
      if(fscanf(file, " %256s", word) != 1){
        break;
      }
      char* name = malloc(256*sizeof(char));
      strcpy(name, word);
      fscanf(file, " %1023s", word);
      priority = atoi(word);
      fscanf(file, " %1023s", word);
      initial_time = atoi(word);
      fscanf(file, " %1023s", word);
      N = atoi(word);
      int* tareas = calloc(2*N, sizeof(int));
      tareas[0] = initial_time;
      for(int i = 1; i < 2*N; i++){
        fscanf(file, " %1023s", word);
        tareas[i] = atoi(word);
      }
      if (initial_time > 0){
        state = 2;
      }
      push_queue(Q_waiting, process_init(pid, name, state, priority, N, tareas));

      pid += 1;
    }
  }

  if (argv[3]){
    quantum = atoi(argv[3]);
  }
  else{
    quantum = 3;
  }

  Queue* queue_ordenada = sort_by_priority(Q_ready);

  char* parametro = "priority";
  find_parameter_processes_in_queue(queue_ordenada, parametro);
  //FCFS(Q_ready, Q_waiting, Q_terminated);
  if (strncmp(scheduler_type, "fcfs", 4) == 0){
    FCFS(Q_ready, Q_waiting, Q_terminated);
  }
  if (strncmp(scheduler_type, "roundrobin", 4) == 0){
    RR(Q_ready, Q_waiting, Q_terminated, quantum);
  }
  if (strncmp(scheduler_type, "priority", 4) == 0){
    priority(Q_ready, Q_waiting, Q_terminated);
  }

  return 0;
}
