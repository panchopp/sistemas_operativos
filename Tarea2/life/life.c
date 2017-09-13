#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

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

    //Aca se hace la matriz inicializada con puras Cell creadas con el cell init (Parten en estado 0, muertas)

    Cell** matriz = (Cell**)malloc(fils*sizeof(Cell*));
    for (int x = 0; x < fils; x++){
      matriz[x] = (Cell*)malloc(cols*sizeof(Cell));
      for (int y = 0; y < cols; y++){
        matriz[x][y] = *cell_init(0, x, y);

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
      matriz[x][y].estado = 1;
    }

    for (int x = 0; x < fils; x++){
      for (int y = 0; y < cols; y++){
        // Aca lee datos de la celda, ejemplo como para imprimir.

        // ACA SE ESTA IMPRIMIENDO ALGO RARO, NOSE PORQUE PASA, ES COMO QUE SE IMPRIMIERA EN LA POSICION 0,0 EL PUNTERO A ALGO,
        // CUANDO DEBERIA SER SU COORDENADA X, O Y.
        printf("%i ", matriz[x][y].estado);
      }
      printf("\n");
    }



  }



