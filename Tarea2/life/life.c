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

};
typedef struct Cell Cell;

Cell* cell_init(int estado, int pos_x, int pos_y)
{
  Cell* ret = (Cell*)malloc(sizeof(Cell));
  ret -> estado = estado;
  ret -> pos_x = pos_x;
  ret -> pos_y = pos_y;
  return ret;
}

void worker(Cell *matriz, const int fils, const int cols, const int num_threads, const int n_cpu, int *celdas_proceso);

typedef struct _thread_data_t {
  int fils;
  int cols;
  Cell *matriz;
  int *celdas;
} thread_data_t;

void *manipulatematrix(void* arg){
  thread_data_t *data = (thread_data_t *)arg;
  printf("%d\n", data -> celdas[1]);
  //printf("hello from thr_func, thread id: %d\n", data->fils);
  //printf("%s\n", "hi");
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

    for(int i = 0; i < n_cpu; i++){
        printf("%s\n", "a");
        int j = fork();
        if(!j){
            int *celdas_proceso = (int *)malloc(sizeof(int) * round(fils*cols/n_cpu));
            // for (int c = i; c <= fils*cols - 1; c += n_cpu){
            //   celdas_proceso[c/n_cpu] = c;
            // }
            // printf("%d %i\n", celdas_proceso[2], i);
            printf("%d\n", n_cpu);
            worker(matriz, fils, cols, num_threads, n_cpu, celdas_proceso);
    }

    //printf("Parent waiting...\n");

    for(int i = 0; i < n_cpu; i++){
        int exit_code;
        waitpid(-1, &exit_code, 0);
    }


    // Imprimimos el resultado de la matriz
    for (int x = 0; x < fils; x++){
      for (int y = 0; y < cols; y++){
        // Aca lee datos de la celda, ejemplo como para imprimir.

        // ACA SE ESTA IMPRIMIENDO ALGO RARO, NOSE PORQUE PASA, ES COMO QUE SE IMPRIMIERA EN LA POSICION 0,0 EL PUNTERO A ALGO,
        // CUANDO DEBERIA SER SU COORDENADA X, O Y.
        printf("%i ", matriz[x * fils + y].estado);
      }
      printf("\n");
    }

    return 0;

  }
}

void worker(Cell *matriz, const int fils, const int cols, const int num_threads, const int n_cpu, int* celdas_proceso){

    thread_data_t arg;
    arg.fils = fils;
    arg.cols = cols;
    arg.matriz = matriz;

    pthread_t threads[num_threads];
    int status;
    for (int i = 0; i < num_threads; i++) {
      arg.celdas = (int *)malloc(round(sizeof(celdas_proceso)/num_threads));
      for (int c = i; c <= fils*cols - 1; c += num_threads){
        arg.celdas[c/num_threads] = celdas_proceso[c];
      }
      //printf("[main] Creating thread %i\n", i);
      status = pthread_create(&threads[i], NULL, manipulatematrix, &arg);
      if (status != 0) {
        printf("[main] Oops. pthread_create returned error code %d\n", status);
        exit(-1);
          }
      }

    // printf("Hello, I'm the fork\n");
    // for(int i = 0; i < fils; i++){
    //   for(int j = 0; j < cols; j++){
    //     if(i==2){
    //       matriz[i * fils + j].estado = 6;
    //     }
    //   }
    // }

    //printf("Fork ended\n");
    exit(0);
}

