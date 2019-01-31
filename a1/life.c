#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_state(char *arr, int size);
void update_state(char *arr, int size);


int main(int argc, char **argv) {

    if (argc != 3) {
    	fprintf(stderr, "Usage: USAGE: life initial n\n");
    	return 1;
    }

    int size = strlen(argv[1]);
    int iterations = strtol(argv[2], NULL, 10);
    char *arr = malloc(sizeof(char)*size);
    strcpy(arr, argv[1]);
    for(int i = 0; i < iterations; i++){
        print_state(arr, size);
        update_state(arr, size);
    }
    free(arr);
    return 0;
}
