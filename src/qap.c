#include "qap.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void deepcopy_QAP_results(struct QAP_results *dest,
                          const struct QAP_results *src) {
  dest->score = src->score;
  dest->evaluated = src->evaluated;
  dest->steps = src->steps;
  memcpy(dest->solution, src->solution, sizeof(int) * MAX_QAP_SIZE);
}

void read_instance(char *filename, struct QAP *qap) {
  FILE *fp = fopen(filename, "r");
  int i, j, *p;

  if(fscanf(fp, "%d", &qap->n) != 1) {
    fprintf(stderr, "Could not read n from file\n");
    exit(1);
  }

  if (qap->n > MAX_QAP_SIZE) {
    fprintf(stderr, "Exceeded maximum size of input! (%d > %d)\n", qap->n,
            MAX_QAP_SIZE);
    exit(1);
  }

  p = qap->A;
  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      if(fscanf(fp, "%d", p) != 1) {
          fprintf(stderr, "Could not read value from matrix A\n");
          exit(1);
      }
      p++;
    }
  }

  p = qap->B;
  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      if(fscanf(fp, "%d", p) != 1) {
          fprintf(stderr, "Could not read value from matrix B\n");
          exit(1);
      }
      p++;
    }
  }
  fclose(fp);
}

int read_solution(char *dat_path, int n, struct QAP_results *res) {
  char sln_path[1024];
  strncpy(sln_path, dat_path, sizeof(sln_path) - 1);
  sln_path[sizeof(sln_path) - 1] = '\0';
  char *dot = strrchr(sln_path, '.');
  if (dot && strcmp(dot, ".dat") == 0) {
    strcpy(dot, ".sln");
  } else {
    fprintf(stderr, "Input file must have a .dat extension.\n");
    exit(1);
  }

  FILE *fp;
  if ((fp = fopen(sln_path, "r")) == NULL) {
    return 0;
  }
  int i, *p, nsol;

  if (fscanf(fp, "%d", &nsol) != 1) {
      fprintf(stderr, "Could not read n from solution\n");
  }

  if (n != nsol) {
    fprintf(stderr, "Input sizes doesn't match (%d != %d)\n", n, nsol);
    exit(1);
  }

  if (fscanf(fp, "%d", &(res->score)) != 1) {
      fprintf(stderr, "Could not read score from solution\n");
      exit(1);
  }

  p = res->solution;
  for (i = 0; i < nsol; i++) {
    if (fscanf(fp, "%d%*[, \t\n]", p) != 1) {
        fprintf(stderr, "Could not read value from solution\n");
        exit(1);
    }
    (*p) -= 1;
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
  int delta = (Ajj - Aii) * (Bpi_pi - Bpj_pj) + (Aji - Aij) * (Bpi_pj - Bpj_pi);

  // We'll sum over k != i, j
  int sum = 0;

  // For convenient pointer arithmetic:
  //  - Ai_ptr, Aj_ptr will traverse row i and j in A for columns k=0..n-1
  //  - Aki_ptr, Akj_ptr will traverse column i and j in A for rows k=0..n-1
  int *Ai_ptr = Ai;     // points to A[i*n + k] as k increments
  int *Aj_ptr = Aj;     // points to A[j*n + k]
  int *Aki_ptr = A + i; // points to A[k*n + i]
  int *Akj_ptr = A + j; // points to A[k*n + j]

  // Loop over all k, skip i and j
  for (int k = 0; k < n; k++) {
    if (k != i && k != j) {
      // This is row p_k in B
      int s_k = sol[k];
      int *Bsk = B + s_k * n;

      // Gather B–related differences in local variables
      int b_diff1 = Bpi[s_k] - Bpj[s_k]; // (B[p_i p_k] - B[p_j p_k])
      int b_diff2 = Bsk[pi] - Bsk[pj];   // (B[p_k p_i] - B[p_k p_j])

      // Gather A–related differences
      int a_diff1 = (Aj_ptr[0] - Ai_ptr[0]);   // (A[j,k] - A[i,k])
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

