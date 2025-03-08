#include "qap.h"

void read_instance(char *filename) {
    FILE *fp = fopen(filename, "r");
    int i, j;
    
    fscanf(fp, "%d", &n);
    
    if (n > MAX_QAP_SIZE) {
        fprintf(stderr, "Exceeded maximum size of input! (%d > %d)\n",
            n, MAX_QAP_SIZE);
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage %s [input file].dat", argv[0]);
        exit(1);
    }

    read_instance(argv[1]);

    printf("Exiting");

}
