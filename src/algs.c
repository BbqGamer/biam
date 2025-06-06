#include "algs.h"
#include "utils.h"
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

double get_sample_delta(struct QAP *qap, struct QAP_results *res) {
  int sum_delta = 0;
  for (int n = 0; n < (qap->n * qap->n); n++) {
    int i, j;
    random_pair(&i, &j, qap->n);
    int delta = get_delta(res->solution, i, j, qap);
    sum_delta += abs(delta);
  }
  return (double)sum_delta / (double)(qap->n * qap->n);
}
void simulatedannealing(struct QAP *qap, struct QAP_results *res, float alpha) {
  int iter = 0;

  int i, j = 0, besti, bestj, delta;
  res->score = evaluate_solution(res->solution, qap);
  double temperature = -get_sample_delta(qap, res) / log(0.90);

  bool improved = true;
  int permi[MAX_QAP_SIZE], permj[MAX_QAP_SIZE];
  int cooldown = 0;

  int cur_solution[MAX_QAP_SIZE];
  int cur_score = res->score;
  memcpy(cur_solution, res->solution, qap->n * sizeof(int));

  int tolerance = 0;
  // tolerance, chain_length -- we want to stop the algorithm after no
  // improvement in P*L
  while (improved || (temperature > 0)) {
    // Exponential decreasing schema
    if (cooldown == (qap->n * 10)) {
      iter++;
      temperature *= alpha;
      // No improvement over 5 temp levels

      // Final convergence check
      if (temperature <= 0.02) {
        temperature = 0;
      }
      cooldown = 0;
    }
    if (tolerance++ >= (qap->n * 2)) {
      fprintf(stderr, "TOLERANCE ERR temp:%f, iter: %d\n", temperature, iter);
      temperature = 0;
    }
    improved = false;
    random_permutation(permi, qap->n);
    for (i = 0; i < qap->n; i++) {
      if (improved) {
        break;
      }

      random_permutation(permj, qap->n);
      for (j = 0; j < qap->n; j++) {
        delta = get_delta(cur_solution, permi[i], permj[j], qap);
        if (delta < 0) {
          besti = permi[i];
          bestj = permj[j];
          improved = true;
          break;
        }
        if (temperature == 0) {
          continue;
        }
        double random_temp = 0.0;
        random_temp = exp(-1 * (double)delta / temperature);
        if (random_temp >= (double)rand() / RAND_MAX) {
          cooldown++;
          besti = permi[i];
          bestj = permj[j];
          improved = true;
          break;
        }
      }
    }
    res->evaluated += i + j;

    if (improved) {
      tolerance = 0;

      res->steps++;
      cur_score += delta;

      swap(cur_solution, besti, bestj);
    }
    if (cur_score < res->score) {
      memcpy(res->solution, cur_solution, qap->n * sizeof(int));
      res->score = cur_score;
    }
  }
}
#define MAX_ITERATIONS 10000

void tabusearch(struct QAP *qap, struct QAP_results *res, int tabutime, int max_patience) {
  int patience = max_patience;
  int n = qap->n;
  res->score = evaluate_solution(res->solution, qap);

  int cur_solution[MAX_QAP_SIZE];
  memcpy(cur_solution, res->solution, n * sizeof(int));
  int cur_score = res->score;

  static int tabu_tenure[MAX_QAP_SIZE][MAX_QAP_SIZE];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      tabu_tenure[i][j] = 0;
    }
  }

  int iteration;
  for (iteration = 0; iteration < MAX_ITERATIONS && patience >= 0;
       iteration++) {
    int best_i = -1;
    int best_j = -1;
    int best_delta = INT_MAX;

    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        int delta = get_delta(cur_solution, i, j, qap);
        bool isTabu = (tabu_tenure[i][j] > iteration);

        // "Aspiration criterion": if this move yields a solution better
        // than the global best, then ignore the tabu.
        bool aspiration = false;
        int new_score = cur_score + delta;
        if (new_score < res->score) {
          aspiration = true;
        }

        // We look for the best move that is not tabu, or is tabu but meets
        // aspiration notice that we can also select non-improving moves
        if ((delta < best_delta) && (!isTabu || aspiration)) {
          best_delta = delta;
          best_i = i;
          best_j = j;
        }
      }
    }

    res->evaluated += n * (n - 1) / 2;
    if (best_i == -1 || best_j == -1) {
      // Everything is tabu, and no aspiration
      break;
    } else {
      res->steps++;
      swap(cur_solution, best_i, best_j);
      cur_score += best_delta;

      tabu_tenure[best_i][best_j] = iteration + tabutime;
      tabu_tenure[best_j][best_i] = iteration + tabutime;

      if (cur_score < res->score) {
        patience = max_patience;
        res->score = cur_score;
        memcpy(res->solution, cur_solution, sizeof(cur_solution));
      } else {
        patience--;
      }
    }
  }
}
