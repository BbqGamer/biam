#include "qap.h"
#include "random.h"
#include <stdio.h>

void read_instance(char *filename) {
  FILE *fp = fopen(filename, "r");
  int i, j;

  fscanf(fp, "%d", &n);

  if (n > MAX_QAP_SIZE) {
    fprintf(stderr, "Exceeded maximum size of input! (%d > %d)\n", n,
            MAX_QAP_SIZE);
    exit(1);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      fscanf(fp, "%d", &A[i * n + j]);
    }
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      fscanf(fp, "%d", &B[i * n + j]);
    }
  }
  fclose(fp);
}
int evaluate_solution(int *sol) {

  int result = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      result += A[i * n + j] * B[sol[i] * n + sol[j]];
    }
  }
  return result;
}

int get_delta(int *sol, int i, int j) {
  /*
  delta: = ( a_{jj} - a_{ii}) ( b_{pipi} - b_{pjpj})
  + ( a_{ji} - a_{ij}) ( b_{pipj} - b_{pjpi})
  + SUM k \in {N\i,j}(
    ( a_{jk} - a_{ik}) ( b_{pipk} - b_{pjpk})
    + ( a_{kj} - a_{ki}) ( b_{pkpi} - b_{pkpj})
)

*/
  int delta;
  delta = (A[j * n + j] - A[i * n + i]) *
              (B[sol[i] * n + sol[i]] - B[sol[j] * n + sol[j]]) +
          (A[j * n + i] - A[i * n + j]) *
              (B[sol[i] * n + sol[j]] - B[sol[j] * n + sol[i]]);
  // SUM
  int sum = 0;
  for (int k = 0; k < n; k++) {
    if ((k == i) || (k == j)) {
      continue;
    }
    sum += (A[j * n + k] - A[i * n + k]) *
               (B[sol[i] * n + sol[k]] - B[sol[j] * n + sol[k]]) +
           (A[k * n + j] - A[k * n + i]) *
               (B[sol[k] * n + sol[i]] - B[sol[k] * n + sol[j]]);
  }
  delta += sum;
  return delta;
}


int argmin(int *array, int n) {
  int idx = 0;
  int result = 1000000000;
  for (int i = 0; i < n; i++) {
    if (array[i] < result) {
      result = array[i];
      idx = i;
    }
  }
  return idx;
}

int argmax(int *array, int n) {
  int idx = 0;
  int result = -1000000000;
  for (int i = 0; i < n; i++) {
    if (array[i] > result) {
      result = array[i];
      idx = i;
    }
  }
  return idx;
}

void heuristic(int *solution) {
  int subsumsA[n];
  int subsumsB[n];
  for (int i = 0; i < n; i++) {
    subsumsA[i] = 0;
    subsumsB[i] = 0;
  }
  for (int i = 0; i < n; i++) {

    for (int j = 0; j < n; j++) {
      subsumsA[i] += A[i * n + j];
      subsumsB[j] += B[i * n + j];
    }
  }
  // printf("SubsumsA\n");
  // for (int i = 0; i < n; i++) {
  //   printf("%d ", subsumsA[i]);
  // }
  // printf("\nSubsumsB\n");
  // for (int i = 0; i < n; i++) {
  //   printf("%d ", subsumsB[i]);
  // }
  // printf("\n");
  int i = 0;
  int j = 0;
  for (int _ = 0; _ < n; _++) {
    i = argmin(subsumsA, n);
    // printf("i: %d\n", i);
    j = argmax(subsumsB, n);
    // printf("j: %d\n", j);
    subsumsA[i] = 100000000;
    subsumsB[j] = -100000000;
    solution[j] = i;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage %s [input file].dat", argv[0]);
    exit(1);
  }
  read_instance(argv[1]);
  printf("Loaded file! \n");
  int solution[n];
  int sum_random = 0;
  int sum_heuristic = 0;
  int max_random = -1;
  int max_heuristic = -1;
  int min_random = 1000000000;
  int min_heuristic = 1000000000;

  for (int _ = 0; _ < 10; _++) {

    srand(_ + 2);
    random_permutation(solution, n);
    int result = evaluate_solution(solution);
    sum_random += result;
    if (result > max_random) {
      max_random = result;
    } else if (result < min_random) {
      min_random = result;
    }
  }
  printf("Random : %d (%d - %d)\n\n", sum_random, min_random, max_random);
  int solution2[n];
  for (int i = 0; i < n; i++) {
    solution2[i] = -1;
  }

  heuristic(solution2);
  int result2 = evaluate_solution(solution2);
  printf("Heuristic: %d", result2);
  int a, b;
  random_pair(&a, &b, n);
  int delta = get_delta(solution2, a, b);
  printf("\nA: %d, B: %d\n", a, b);
  int temp = solution2[a];
  solution2[a] = solution2[b];
  solution2[b] = temp;
  int result3 = evaluate_solution(solution2);
  printf("Before: %d\tAfter: %d", result2 + delta, result3);
}
