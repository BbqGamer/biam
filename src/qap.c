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

int get_delta(int *sol, int i, int j, struct QAP *qap)
{
    /*
      delta = ( A[jj] - A[ii] ) * ( B[p_i p_i] - B[p_j p_j] )
            + ( A[ji] - A[ij] ) * ( B[p_i p_j] - B[p_j p_i] )
            + SUM_{k != i, j} [
                ( A[jk] - A[ik] ) * ( B[p_i p_k] - B[p_j p_k] )
              + ( A[kj] - A[ki] ) * ( B[p_k p_i] - B[p_k p_j] )
            ]
    */

    int n = qap->n;
    int *A = qap->A;
    int *B = qap->B;

    // Current assignments for locations i and j
    int pi = sol[i];
    int pj = sol[j];

    // Row pointers in A for row i, row j
    int *Ai = A + i * n;  
    int *Aj = A + j * n;  

    // Row pointers in B for row p_i, row p_j
    int *Bpi = B + pi * n;
    int *Bpj = B + pj * n;

    // Cache a few repeated accesses
    int Aii = Ai[i], Ajj = Aj[j];
    int Aij = Ai[j], Aji = Aj[i];

    int Bpi_pi = Bpi[pi], Bpj_pj = Bpj[pj];
    int Bpi_pj = Bpi[pj], Bpj_pi = Bpj[pi];

    // First (constant) part of delta
    int delta = (Ajj - Aii) * (Bpi_pi - Bpj_pj)
              + (Aji - Aij) * (Bpi_pj - Bpj_pi);

    // We'll sum over k != i, j
    int sum = 0;

    // For convenient pointer arithmetic:
    //  - Ai_ptr, Aj_ptr will traverse row i and j in A for columns k=0..n-1
    //  - Aki_ptr, Akj_ptr will traverse column i and j in A for rows k=0..n-1
    int *Ai_ptr  = Ai;         // points to A[i*n + k] as k increments
    int *Aj_ptr  = Aj;         // points to A[j*n + k]
    int *Aki_ptr = A + i;      // points to A[k*n + i]
    int *Akj_ptr = A + j;      // points to A[k*n + j]

    // Loop over all k, skip i and j
    for (int k = 0; k < n; k++)
    {
        if (k != i && k != j)
        {
            // This is row p_k in B
            int s_k  = sol[k];
            int *Bsk = B + s_k * n;

            // Gather B–related differences in local variables
            int b_diff1 = Bpi[s_k] - Bpj[s_k];     // (B[p_i p_k] - B[p_j p_k])
            int b_diff2 = Bsk[pi] - Bsk[pj];       // (B[p_k p_i] - B[p_k p_j])

            // Gather A–related differences
            int a_diff1 = (Aj_ptr[0] - Ai_ptr[0]);  // (A[j,k] - A[i,k])
            int a_diff2 = (Akj_ptr[0] - Aki_ptr[0]); // (A[k,j] - A[k,i])

            sum += a_diff1 * b_diff1 + a_diff2 * b_diff2;
        }

        // Advance all pointers to move from column k to column k+1 in Ai,Aj
        // and from row k to row k+1 for Aki, Akj
        Ai_ptr++;
        Aj_ptr++;
        Aki_ptr += n;
        Akj_ptr += n;
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
