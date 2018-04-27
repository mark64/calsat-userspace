#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int num = atoi(argv[1]);
    int k = 0;
    for (int i = 0; i < num; i++) {
        fprintf(stdout, "%d ", k);
        k++;
    }
    for (int i = 0; i < num; i++) {
        fprintf(stderr, "%d ", k);
        k++;
    }
    fprintf(stderr, "\n");
    return 0;
}
