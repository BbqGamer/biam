#include "qap.h"

void heuristic(struct QAP *qap, struct QAP_results *res);
void localsearchgreedy(struct QAP *qap, struct QAP_results *res);
void localsearchsteepest(struct QAP *qap, struct QAP_results *res);
void randomsearch(struct QAP *qap, struct QAP_results *res);
void randomwalk(struct QAP *qap, struct QAP_results *res);

void simulatedannealing(struct QAP *qap, struct QAP_results *res);
void tabusearch(struct QAP *qap, struct QAP_results *res, int tabutime);
