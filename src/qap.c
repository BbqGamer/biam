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
      delta = ( A[jj] - A[ii]) * ( B[p_i p_i] - B[p_j p_j])
             + ( A[ji] - A[ij]) * ( B[p_i p_j] - B[p_j p_i])
             + SUM_{k != i, j} [
                 ( A[jk] - A[ik]) * ( B[p_i p_k] - B[p_j p_k])
               + ( A[kj] - A[ki]) * ( B[p_k p_i] - B[p_k p_j])
             ]
    */

    int n   = qap->n;
    int *A  = qap->A; 
    int *B  = qap->B;

    int pi  = sol[i];
    int pj  = sol[j];

    // Pointers to A’s rows for i and j
    int *Ai = A + i * n;  // Row i in A
    int *Aj = A + j * n;  // Row j in A

    // Pointers to B’s rows for pi and pj
    int *Bpi = B + pi * n; // Row p_i in B
    int *Bpj = B + pj * n; // Row p_j in B

    // Load up reused values (avoid re-checking Ai[i], Bpi[pi], etc.)
    int Aii = Ai[i]; 
    int Ajj = Aj[j];
    int Aij = Ai[j];
    int Aji = Aj[i];

    int Bpi_pi = Bpi[pi];
    int Bpj_pj = Bpj[pj];
    int Bpi_pj = Bpi[pj];
    int Bpj_pi = Bpj[pi];

    // First part of delta
    int delta = (Ajj - Aii) * (Bpi_pi - Bpj_pj)
              + (Aji - Aij) * (Bpi_pj - Bpj_pi);

    // Accumulate sum for k != i, j
    int *Aki = A + i;
    int *Akj = A + j;
    int sum = 0;
    for (int k = 0; k < n; k++) {
        int *Bsk = B + sol[k] * n; 

        if (k != i  && k != j) {
          sum += (*Aj - *Ai) * (Bpi[sol[k]] - Bpj[sol[k]])
              + (*Akj - *Aki) * (Bsk[pi] - Bsk[pj]);
        }

        Aki += n;
        Akj += n;
        Aj += 1;
        Ai += 1;
    }

    delta += sum;
    return delta;
}

int argmin(int *array, int n) {
  // finds the index of the minimum element in array
  // resolves ties randomly
  int idx = 0;
  int result = INT_MAX;
  for (int i = 0; i < n; i++) {
    if (array[i] > result || (array[i] == result && rand() % 2 == 0)) {
      continue;
    }
    result = array[i];
    idx = i;
  }
  return idx;
}

int argmax(int *array, int n) {
  // finds the index of the maximum element in array
  // resolves ties randomly
  int idx = 0;
  int result = INT_MIN;
  for (int i = 0; i < n; i++) {
    if (array[i] < result || (array[i] == result && rand() % 2 == 0)) {
      continue;
    }
    result = array[i];
    idx = i;
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
  int i, j = 0, besti, bestj, tmp, delta;
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
  int i, j = 0, besti, bestj, tmp, delta, best_delta;
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

#define TIMEOUT_CHECK_INTERVAL 50

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
    memcpy(res->solution, tmp_solution, qap->n * sizeof(int));
    res->score = evaluate_solution(res->solution, qap);

    int i = 0;
    while (1) {
        if ((i % TIMEOUT_CHECK_INTERVAL) == 0) {
            now = clock();
            double elapsed_ms = ((double)(now - start)) * 1000.0 / CLOCKS_PER_SEC;
            if (elapsed_ms > qap->timeout_ms)
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
