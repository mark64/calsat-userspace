#include <stdio.h>

int main(int argc, char **argv) {
    for(int i = 0; i < 10; i++) {
        fprintf(stderr, "ERR %d\n", i);
        fprintf(stdout, "OUT %d\n", i);
        if (i < argc) fprintf(stderr, "ERR %s\n", argv[i]);
        if (i < argc) fprintf(stdout, "OUT %s\n", argv[i]);
    }
    return 0;
}
