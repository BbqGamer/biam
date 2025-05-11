#include "qap.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>

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

  int val;
  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      if(fscanf(fp, "%d", &val) != 1) {
          fprintf(stderr, "Could not read value from matrix A\n");
          exit(1);
      }
      qap->A[i * qap->n + j] = val;
      qap->_AT[j * qap->n + i] = val;
    }
  }

  for (i = 0; i < qap->n; i++) {
    for (j = 0; j < qap->n; j++) {
      if(fscanf(fp, "%d", &val) != 1) {
          fprintf(stderr, "Could not read value from matrix B\n");
          exit(1);
      }
      qap->B[i * qap->n + j] = val;
      qap->_BT[j * qap->n + i] = val;
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

#if defined(__AVX2__)
static inline int hadd8_epi32(__m256i v)
{
    __m128i low  = _mm256_castsi256_si128(v);
    __m128i high = _mm256_extracti128_si256(v, 1);
    __m128i s    = _mm_add_epi32(low, high);          // 4-lane vector

    s = _mm_hadd_epi32(s, s);                         // 2-lane
    s = _mm_hadd_epi32(s, s);                         // 1-lane
    return _mm_cvtsi128_si32(s);
}

int get_delta(
    const int *restrict sol,
    int i, int j,
    const struct QAP *restrict qap)
{
  int n = qap->n;
  int pi = sol[i];
  int pj = sol[j];

  const int *restrict Ai  = qap->A   + i * n;
  const int *restrict Aj  = qap->A   + j * n;
  const int *restrict Aki = qap->_AT + i * n;
  const int *restrict Akj = qap->_AT + j * n;

  const int *restrict Bpi = qap->B    + pi * n;
  const int *restrict Bpj = qap->B    + pj * n;
  const int *restrict BTpi = qap->_BT + pi * n;
  const int *restrict BTpj = qap->_BT + pj * n;

  int delta = (Aj[j] - Ai[i]) * (Bpi[pi] - Bpj[pj]) \
            + (Aj[i] - Ai[j]) * (Bpi[pj] - Bpj[pi]);

  int k;
  __m256i acc = _mm256_setzero_si256();
  for (k = 0; k + 8 < n; k += 8) {
      // A part
      __m256i vaj = _mm256_loadu_si256((const __m256i*)(Aj + k));
      __m256i vai = _mm256_loadu_si256((const __m256i*)(Ai + k));
      __m256i a_diff1 = _mm256_sub_epi32(vaj, vai);   // (A[jk] - A[ik])

      __m256i vakj = _mm256_loadu_si256((const __m256i*)(Akj + k));
      __m256i vaki = _mm256_loadu_si256((const __m256i*)(Aki + k));
      __m256i a_diff2 = _mm256_sub_epi32(vakj, vaki); // (A[kj] - A[ki])

      // B part
      __m256i vs = _mm256_loadu_si256((const __m256i*)(sol + k));
      __m256i vbpi = _mm256_i32gather_epi32(Bpi, vs, 4);
      __m256i vbpj = _mm256_i32gather_epi32(Bpj, vs, 4);
      __m256i b_diff1 = _mm256_sub_epi32(vbpi, vbpj); // (B[p_i p_k] - B[p_j p_k])

      __m256i vbtpi = _mm256_i32gather_epi32(BTpi, vs, 4);
      __m256i vbtpj = _mm256_i32gather_epi32(BTpj, vs, 4);
      __m256i b_diff2 = _mm256_sub_epi32(vbtpi, vbtpj); // (B[p_k p_i] - B[p_k p_j])

      // Accumulate
      __m256i prod1 = _mm256_mullo_epi32(a_diff1, b_diff1);
      __m256i prod2 = _mm256_mullo_epi32(a_diff2, b_diff2);
      __m256i sum = _mm256_add_epi32(prod1, prod2);
      acc = _mm256_add_epi32(acc, sum);
  }
  delta += hadd8_epi32(acc);

  for (; k < n; k++) {
    int s_k = sol[k];

    int a_diff1 = (Aj[k] - Ai[k]);     // (A[jk] - A[ik])
    int a_diff2 = (Akj[k] - Aki[k]);   // (A[kj] - A[ki])

    int b_diff1 = Bpi[s_k] - Bpj[s_k];     // (B[p_i p_k] - B[p_j p_k])
    int b_diff2 = BTpi[s_k] - BTpj[s_k];   // (B[p_k p_i] - B[p_k p_j])

    delta += a_diff1 * b_diff1 + a_diff2 * b_diff2;
  }
    
  delta -= (Aj[i] - Ai[i]) * (Bpi[pi] - Bpj[pi])
         + (Akj[i] - Aki[i]) * (BTpi[pi] - BTpj[pi]);

  delta -= (Aj[j] - Ai[j]) * (Bpi[pj] - Bpj[pj])
         + (Akj[j] - Aki[j]) * (BTpi[pj] - BTpj[pj]);

  return delta;
}
#else
int get_delta(
    const int *restrict sol,
    int i, int j,
    const struct QAP *restrict qap)
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
  int pi = sol[i];
  int pj = sol[j];

  const int *restrict Ai  = qap->A   + i * n;
  const int *restrict Aj  = qap->A   + j * n;
  const int *restrict Aki = qap->_AT + i * n;
  const int *restrict Akj = qap->_AT + j * n;

  const int *restrict Bpi = qap->B    + pi * n;
  const int *restrict Bpj = qap->B    + pj * n;
  const int *restrict BTpi = qap->_BT + pi * n;
  const int *restrict BTpj = qap->_BT + pj * n;

  int delta = (Aj[j] - Ai[i]) * (Bpi[pi] - Bpj[pj]) \
            + (Aj[i] - Ai[j]) * (Bpi[pj] - Bpj[pi]);

  for (int k = 0; k < n; k++) {
    if (k != i && k != j) {
      int s_k = sol[k];

      int a_diff1 = (Aj[k] - Ai[k]);     // (A[jk] - A[ik])
      int a_diff2 = (Akj[k] - Aki[k]);   // (A[kj] - A[ki])

      int b_diff1 = Bpi[s_k] - Bpj[s_k];     // (B[p_i p_k] - B[p_j p_k])
      int b_diff2 = BTpi[s_k] - BTpj[s_k];   // (B[p_k p_i] - B[p_k p_j])


      delta += a_diff1 * b_diff1 + a_diff2 * b_diff2;
    }
  }
  return delta;
}
#endif
