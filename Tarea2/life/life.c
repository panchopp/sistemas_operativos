#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <sys/mman.h>

struct Cell
{
  int estado; // 1: Viva, 0: Muerta
  int pos_x;
  int pos_y;
  int cambia;

};
typedef struct Cell Cell;

Cell* cell_init(int estado, int pos_x, int pos_y)
{
  Cell* ret = (Cell*)malloc(sizeof(Cell));
  ret -> estado = estado;
  ret -> pos_x = pos_x;
  ret -> pos_y = pos_y;
  ret -> cambia = 0;
  return ret;
}

void worker(Cell *matriz, const int fils, const int cols, const int num_threads, const int n_cpu, int *celdas_proceso);

typedef struct _thread_data_t {
  int fils;
  int cols;
  Cell *matriz_copia;
  int *celdas;
  int tid;
  int pid;
} thread_data_t;

void *manipulatematrix(void* arg){
  thread_data_t *data = (thread_data_t *)arg;
  printf("Thread %i de proceso %i : %d\n", data->tid, data->pid, (data -> celdas)[0]);
  //(data -> matriz_copia)[(data -> celdas)[0]].estado = 9;
  for (int q = 0; q < sizeof(data -> celdas)/sizeof(int); q++){
    int count = 0;
    if ((data -> celdas)[q] + 1 < (data -> fils)*(data -> cols) && (data -> celdas)[q] + 1 >= 0){
      count += (data -> matriz_copia)[(data -> celdas)[q] + 1].estado;
    }

    if ((data -> celdas)[q] - 1 < (data -> fils)*(data -> cols) && (data -> celdas)[q] - 1 >= 0){
      count += (data -> matriz_copia)[(data -> celdas)[q] - 1].estado;
    }

    if (((data -> celdas)[q] + data -> cols < (data -> fils)*(data -> cols)) && ((data -> celdas)[q] + data -> cols >= 0)){
      count += (data -> matriz_copia)[(data -> celdas)[q] + data -> cols].estado;
    }

    if (((data -> celdas)[q] - data -> cols < (data -> fils)*(data -> cols)) && ((data -> celdas)[q] - data -> cols >= 0)){
      count += (data -> matriz_copia)[(data -> celdas)[q] - data -> cols].estado;
    }

    if (((data -> celdas)[q] + data -> cols + 1 < (data -> fils)*(data -> cols)) && ((data -> celdas)[q] + data -> cols + 1 >= 0)){
      count += (data -> matriz_copia)[(data -> celdas)[q] + data -> cols + 1].estado;
    }

    if (((data -> celdas)[q] + data -> cols - 1 < (data -> fils)*(data -> cols)) && ((data -> celdas)[q] + data -> cols - 1 >= 0)){
      count += (data -> matriz_copia)[(data -> celdas)[q] + data -> cols - 1].estado;
    }

    if (((data -> celdas)[q] - data -> cols + 1 < (data -> fils)*(data -> cols)) && (data -> celdas)[q] - data -> cols + 1 >= 0){
      count += (data -> matriz_copia)[(data -> celdas)[q] - data -> cols + 1].estado;
    }

    if (((data -> celdas)[q] - data -> cols - 1 < (data -> fils)*(data -> cols)) && ((data -> celdas)[q] - data -> cols - 1 >= 0)){
      count += (data -> matriz_copia)[(data -> celdas)[q] - data -> cols - 1].estado;
    }
    
    if ((data -> matriz_copia)[(data -> celdas)[q]].estado == 0){
      //printf("%i\n", count);
      if (count == 3){
        (data -> matriz_copia)[(data -> celdas)[q]].cambia = 1;
      }
    }

    else{
      if (count < 2 || count > 3){
        (data -> matriz_copia)[(data -> celdas)[q]].cambia = 1;
      }
    }
  }
  pthread_exit("Exit");
  return 0;
}


// struct Matrix
// {
//   Cell** matriz;
//   int N; // Filas
//   int M; // Columnas
//
// };
// typedef struct Matrix Matrix;

// Matrix* matrix_init(int N, int M)
// {
//  Matrix* ret = (Matrix*)malloc(sizeof(Matrix));
//   ret -> N = N;
//   ret -> M = M;
//   Cell** matriz = (Cell**)malloc(M*sizeof(int));
//   for (int x = 0; x < M; x++){
//     matriz[x] = (Cell*)malloc(N*sizeof(int));
//     for (int y = 0; y < N; y++){
//       matriz[x][y] = *cell_init(0, x, y);
//
//     }
//   }
//   ret -> matriz = matriz;
//  return ret;
// }


int main(int argc, char const *argv[]) {

  int n_cpu = sysconf(_SC_NPROCESSORS_ONLN);
  printf("Hay %i CPU's\n", n_cpu);

  FILE* file;
  file = fopen("input.txt", "r");

    int iteraciones = 0;
    int fils = 0;
    int cols = 0;
    int initial_alive_cells = 0;
    int num_threads = 0;

    // Esta linea lee la primera linea del input

    fscanf(file, "%i %i %i %i %i\n", &iteraciones, &fils, &cols, &initial_alive_cells, &num_threads);

    //Matrix* matriz = matrix_init(fils, cols);

    //Creamos matriz como memoria compartida
    Cell *matriz = mmap(0, fils*cols*sizeof(Cell*), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

    //Aca se hace la matriz inicializada con puras Cell creadas con el cell init (Parten en estado 0, muertas)

    //Cell** matriz = (Cell**)malloc(fils*sizeof(Cell*));        //esto se reemplaza por memoria compartido

    for (int x = 0; x < fils; x++){
      for (int y = 0; y < cols; y++){
        matriz[x * fils + y] = *cell_init(0, x, y);

      }
    }

    // Cargamos las vivas

    for (int i = 0; i < initial_alive_cells; i++){
      int x;
      int y;

      // Esto lee las lineas con las coordenadas de las celdas vivas, yo cacho que por aca hay que meterle a que cuando vaya a "revivir" una celda,
      // que chequee las vecinas y las revivas si se cumplen las reglas del enunciado.
      fscanf(file, "%i %i\n", &x, &y);

      // Asi se asignan valores a la celda
      matriz[x * fils + y].estado = 1;
    }


    while (iteraciones > 0){
      int pids_workers[n_cpu];
      for(int i = 0; i < n_cpu; i++){
          int j = fork();
          if(j != 0){
              int *celdas_proceso = (int *)malloc(sizeof(int) * ceil((float)fils*cols/(float)n_cpu));
              for (int c = i; c <= fils*cols - 1; c += n_cpu){
                celdas_proceso[c/n_cpu] = c;
              }
              //printf("Worker with pid: %i\n", getpid());
              pids_workers[i] = getpid();
              worker(matriz, fils, cols, num_threads, n_cpu, celdas_proceso);
            }
          }
      //printf("Parent waiting...\n");

      for(int i = 0; i < n_cpu; i++){
        //printf("Master waiting for worker with pid: %i\n", pids_workers[i]);
          waitpid(pids_workers[i], 0, 0);
      }

      wait(NULL);


      printf("%s\n", "HOLA");
      for (int x = 0; x < fils; x++){
        for (int y = 0; y < cols; y++){
          if (matriz[x * fils + y].cambia == 1){
            if (matriz[x * fils + y].estado == 0){
              matriz[x * fils + y].estado = 1;
            }
            else{
              matriz[x * fils + y].estado = 0;
            }

            matriz[x * fils + y].cambia = 0;
            
          }
        }
      }

      // Imprimimos el resultado de la matriz
      for (int x = 0; x < fils; x++){
        for (int y = 0; y < cols; y++){
          // Aca lee datos de la celda, ejemplo como para imprimir.
          printf("%i ", matriz[x * fils + y].estado);
        }
        printf("\n");
      }

      iteraciones--;

    }

    return 0;
}

void worker(Cell *matriz, const int fils, const int cols, const int num_threads, const int n_cpu, int* celdas_proceso){

    Cell* matriz_copia = (Cell*)malloc(fils*cols*sizeof(Cell*));
    for (int x = 0; x < fils; x++){
      for (int y = 0; y < cols; y++){
        matriz_copia[x * fils + y] = *cell_init(matriz[x * fils + y].estado, x, y);
      }
    }

    thread_data_t lista_args[num_threads];
    for (int a = 0; a < num_threads; a++){
      thread_data_t arg;
      arg.fils = fils;
      arg.cols = cols;
      arg.matriz_copia = matriz_copia;
      arg.pid = getpid();
      lista_args[a] = arg;
    }

    pthread_t threads[num_threads];
    int status;
    for (int i = 0; i < num_threads; i++) {
      lista_args[i].celdas = (int *)malloc(ceil((float)sizeof(celdas_proceso)/(float)num_threads));
      lista_args[i].tid = i;
      for (int c = i; c <= (sizeof(celdas_proceso)/sizeof(int)) - 1; c += num_threads){
        lista_args[i].celdas[(c-i)/num_threads] = celdas_proceso[c];
      }
      status = pthread_create(&threads[i], NULL, manipulatematrix, &lista_args[i]);
      if (status != 0) {
        printf("[main] Oops. pthread_create returned error code %d\n", status);
        exit(-1);
          }
      }

    for (int i = 0; i < num_threads; i++)
       pthread_join(threads[i], NULL);


    // Cambios a matriz original

    for (int x = 0; x < fils; x++){
      for (int y = 0; y < cols; y++){
        if (matriz_copia[x * fils + y].cambia == 1){
          matriz[x * fils + y].cambia = 1;
        }
      }
    }

    printf("%s\n", "termine la wea!");

    exit(0);
}
