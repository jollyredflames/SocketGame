#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int i;
    int iterations;

    if (argc != 2) {
        fprintf(stderr, "Usage: forkloop <iterations>\n");
        exit(1);
    }
    
    iterations = strtol(argv[1], NULL, 10);
    int originalID = getpid();
    for (i = 0; i < iterations; i++) {
        int n = 0;
        if(i == 0 || getpid() != originalID){
            originalID = getpid();
            n = fork();
        }
        if (n < 0) {
            perror("fork");
            exit(1);
        }
        printf("ppid = %d, pid = %d, i = %d\n", getppid(), getpid(), i);
    }

    return 0;
}
