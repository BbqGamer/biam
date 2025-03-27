#include "qap.h"
#include "random.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

int read_solution(char *filename, int n, struct QAP_results *res) {
  FILE *fp;
  if((fp = fopen(filename, "r")) == NULL) {
    return 0;
  }
  int i, *p, nsol;

  fscanf(fp, "%d", &nsol);
  if (n != nsol) {
    fprintf(stderr, "Input sizes doesn't match (%d != %d)\n", n, nsol);
    exit(1);
  }

  fscanf(fp, "%d", &(res->score));

  p = res->solution;
  for (i = 0; i < nsol; i++) {
    fscanf(fp, "%d", p);
    p++;
  }

  fclose(fp);
  return 1;
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
  int result = INT_MAX;
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
  int result = INT_MIN;
  for (int i = 0; i < n; i++) {
    if (array[i] > result) {
      result = array[i];
      idx = i;
    }
  }
  return idx;
}

void heuristic(struct QAP *qap, struct QAP_results *res) {
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
    subsumsA[i] = INT_MAX;
    subsumsB[j] = INT_MIN;
    res->solution[j] = i;
  }
  res->score = evaluate_solution(res->solution, qap);
}

void localsearchgreedy(struct QAP *qap, struct QAP_results *res) {
  int i, j, besti, bestj, tmp, delta;
  res->score = evaluate_solution(res->solution, qap);
  bool improved = true;
  int permi[MAX_QAP_SIZE], permj[MAX_QAP_SIZE];

  while (improved) {
    improved = false;
    random_permutation(permi, qap->n);
    for (i = 0; i < qap->n; i++) {
      if (improved) {
        break;
      }

      random_permutation(permj, qap->n);
      for (j = 0; j < qap->n; j++) {
        delta = get_delta(res->solution, permi[i], permj[j], qap);
        if (delta < 0) {
          besti = permi[i];
          bestj = permj[j];
          improved = true;
          break;
        }
      }
    }
    res->evaluated += i + j;

    if (improved) {
      res->steps++;
      res->score += delta;

      tmp = res->solution[besti];
      res->solution[besti] = res->solution[bestj];
      res->solution[bestj] = tmp;
    }
  }
}

void localsearchsteepest(struct QAP *qap, struct QAP_results *res) {
  int i, j, besti, bestj, tmp, delta, best_delta;
  res->score = evaluate_solution(res->solution, qap);
  bool improved = true;
  while (improved) {
    improved = false;
    best_delta = 0;

    for (i = 0; i < qap->n; i++) {
      for (j = 0; j < qap->n; j++) {
        delta = get_delta(res->solution, i, j, qap);
        if (delta < best_delta) {
          best_delta = delta;
          besti = i;
          bestj = j;
        }
      }
    }
    res->evaluated += i + j;
    if (best_delta < 0) {
      res->steps++;
      improved = true;
      res->score += best_delta;

      tmp = res->solution[besti];
      res->solution[besti] = res->solution[bestj];
      res->solution[bestj] = tmp;
    }
  }
}

#define TIMEOUT_CHECK_INTERVAL 100
#define TIMEOUT_MS 10

void randomsearch(struct QAP *qap, struct QAP_results *res) {
  int tmp_solution[MAX_QAP_SIZE];
  int score;

  clock_t now, start = clock();
  random_permutation(res->solution, qap->n);
  res->score = evaluate_solution(res->solution, qap);

  int i = 0;
  while (1) {
      if ((i % TIMEOUT_CHECK_INTERVAL) == 0) {
          now = clock();
          double elapsed_ms = ((double)(now - start)) * 1000.0 / CLOCKS_PER_SEC;
          if (elapsed_ms > qap->timeout_ms)
              break;
      }

      random_permutation(tmp_solution, qap->n);
      score = evaluate_solution(tmp_solution, qap);
      res->evaluated += 1;
      if (score < res->score) {
          memcpy(res->solution, tmp_solution, qap->n * sizeof(int));
          res->score = score;
      }
      i++;
  }
}

void randomwalk(struct QAP *qap, struct QAP_results *res) {
    int tmp_solution[MAX_QAP_SIZE];
    int a, b, score, tmp;

    clock_t now, start = clock();
    random_permutation(tmp_solution, qap->n);
    res->score = evaluate_solution(tmp_solution, qap);

    int i = 0;
    while (1) {
        if ((i % TIMEOUT_CHECK_INTERVAL) == 0) {
            now = clock();
            double elapsed_ms = ((double)(now - start)) * 1000.0 / CLOCKS_PER_SEC;
            if (elapsed_ms > TIMEOUT_MS)
                break;
        }

        score = evaluate_solution(tmp_solution, qap);
        res->evaluated += 1;
        if (score < res->score) {
            memcpy(res->solution, tmp_solution, qap->n * sizeof(int));
            res->score = score;
        }
        random_pair(&a, &b, qap->n);
        tmp = tmp_solution[a];
        tmp_solution[a] = tmp_solution[b];
        tmp_solution[b] = tmp;
        i++;
    }
}
