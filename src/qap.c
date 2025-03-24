#include "qap.h"
#include "random.h"
#include <stdbool.h>
#include <stdio.h>

void read_instance(char *filename, struct QAP *qap) {
  FILE *fp = fopen(filename, "r");
  int i, j, *p;

  fscanf(fp, "%d", &qap->n);

  if (qap->n > MAX_QAP_SIZE) {
    fprintf(stderr, "Exceeded maximum size of input! (%d > %d)\n", qap->n,
            MAX_QAP_SIZE);
    exit(1);
  }

  p = qap->A;
  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      fscanf(fp, "%d", p);
      p++;
    }
  }

  p = qap->B;
  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      fscanf(fp, "%d", p);
      p++;
    }
  }
  fclose(fp);
}

int evaluate_solution(int *sol, struct QAP *qap) {
  int result = 0;
  for (int i = 0; i < qap->n; i++) {
    for (int j = 0; j < qap->n; j++) {
      result += qap->A[i * qap->n + j] * qap->B[sol[i] * qap->n + sol[j]];
    }
  }
  return result;
}

int get_delta(int *sol, int i, int j, struct QAP *qap) {
  /*
  delta: = ( a_{jj} - a_{ii}) ( b_{pipi} - b_{pjpj})
  + ( a_{ji} - a_{ij}) ( b_{pipj} - b_{pjpi})
  + SUM k \in {N\i,j}(
    ( a_{jk} - a_{ik}) ( b_{pipk} - b_{pjpk})
    + ( a_{kj} - a_{ki}) ( b_{pkpi} - b_{pkpj})
)

*/
  int delta;
  delta =
      (qap->A[j * qap->n + j] - qap->A[i * qap->n + i]) *
          (qap->B[sol[i] * qap->n + sol[i]] -
           qap->B[sol[j] * qap->n + sol[j]]) +
      (qap->A[j * qap->n + i] - qap->A[i * qap->n + j]) *
          (qap->B[sol[i] * qap->n + sol[j]] - qap->B[sol[j] * qap->n + sol[i]]);
  // SUM
  int sum = 0;
  for (int k = 0; k < qap->n; k++) {
    if ((k == i) || (k == j)) {
      continue;
    }
    sum += (qap->A[j * qap->n + k] - qap->A[i * qap->n + k]) *
               (qap->B[sol[i] * qap->n + sol[k]] -
                qap->B[sol[j] * qap->n + sol[k]]) +
           (qap->A[k * qap->n + j] - qap->A[k * qap->n + i]) *
               (qap->B[sol[k] * qap->n + sol[i]] -
                qap->B[sol[k] * qap->n + sol[j]]);
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

void heuristic(int *solution, struct QAP *qap) {
  int subsumsA[MAX_QAP_SIZE];
  int subsumsB[MAX_QAP_SIZE];
  for (int i = 0; i < qap->n; i++) {
    subsumsA[i] = 0;
    subsumsB[i] = 0;
  }
  for (int i = 0; i < qap->n; i++) {
    for (int j = 0; j < qap->n; j++) {
      subsumsA[i] += qap->A[i * qap->n + j];
      subsumsB[j] += qap->B[i * qap->n + j];
    }
  }

  int i = 0;
  int j = 0;
  for (int _ = 0; _ < qap->n; _++) {
    i = argmin(subsumsA, qap->n);
    j = argmax(subsumsB, qap->n);
    subsumsA[i] = 100000000;
    subsumsB[j] = -100000000;
    solution[j] = i;
  }
}

int localsearchgreedy(int *solution, struct QAP *qap, int *evaluated,
                      int *steps) {
  int i, j, besti, bestj, tmp;
  int delta, best_score = evaluate_solution(solution, qap);
  bool improved = true;
  int permi[qap->n], permj[qap->n];

  while (improved) {
    improved = false;
    random_permutation(permi, qap->n);
    for (i = 0; i < qap->n; i++) {
      if (improved) {
        break;
      }

      random_permutation(permj, qap->n);
      for (j = 0; j < qap->n; j++) {
        delta = get_delta(solution, permi[i], permj[j], qap);
        if (delta < 0) {
          besti = permi[i];
          bestj = permj[j];
          improved = true;
          break;
        }
      }
    }
    *evaluated += i + j;

    if (improved) {
      (*steps)++;
      best_score += delta;

      tmp = solution[besti];
      solution[besti] = solution[bestj];
      solution[bestj] = tmp;
    }
  }
  return best_score;
}

int localsearchsteepest(int *solution, struct QAP *qap, int *evaluated,
                        int *steps) {
  int i, j, besti, bestj, tmp;
  int delta, best_delta, best_score = evaluate_solution(solution, qap);
  bool improved = true;
  while (improved) {
    improved = false;
    best_delta = 0;

    for (i = 0; i < qap->n; i++) {
      for (j = 0; j < qap->n; j++) {
        delta = get_delta(solution, i, j, qap);
        if (delta < best_delta) {
          best_delta = delta;
          besti = i;
          bestj = j;
        }
      }
    }
    *evaluated += i + j;
    if (best_delta < 0) {
      (*steps)++;
      improved = true;
      best_score += best_delta;

      tmp = solution[besti];
      solution[besti] = solution[bestj];
      solution[bestj] = tmp;
    }
  }
  return best_score;
}

int randomwalk(int *solution, struct QAP *qap, int *evaluated, int *steps) {
  int a, b, tmp;
  int k = 0;
  for (int _ = 0; _ < 1000; _++) {
    k++;
    (*steps)++;
    random_pair(&a, &b, qap->n);
    tmp = solution[a];
    solution[a] = solution[b];
    solution[b] = tmp;
  }
  *evaluated += k;
  int result = evaluate_solution(solution, qap);
  return result;
}