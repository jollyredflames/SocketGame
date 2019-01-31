#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END  0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000
#define STACK_START 0xfff000000

int main(int argc, char **argv) {
    
    FILE *fp = NULL;

    if(argc == 1) {
        fp = stdin;

    } else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }
    
    char ch;
    unsigned long current_hex = 0;
    int countI = 0;
    int countM = 0;
    int countL = 0;
    int countS = 0;
    int countGlobs = 0;
    int countHeap = 0;
    int countStack = 0;
    while(fscanf(fp, " %c,0x%lx", &ch, &current_hex) != EOF){
        if(ch == 'I'){
            countI++;
            continue;
        }else if(ch == 'M'){
            countM++;
        }else if(ch == 'L'){
            countL++;
        }else if(ch == 'S'){
            countS++;
        }
        if(current_hex >= GLOBALS_START && current_hex <= GLOBALS_END){
            countGlobs++;
        }else if(current_hex >= HEAP_START && current_hex <= HEAP_END){
            countHeap++;
        }else if(current_hex >= STACK_START){
            countStack++;
        }
    }
    
    /* Complete the implementation */
    /* Use these print statements to print the ouput. It is important that 
     * the output match precisely for testing purposes.
     * Fill in the relevant variables in each print statement.
     * The print statements are commented out so that the program compiles.  
     * Uncomment them as you get each piece working.
     */
    
    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", countI);
    printf("    Modifications: %d\n", countM);
    printf("    Loads: %d\n", countL);
    printf("    Stores: %d\n", countS);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", countGlobs);
    printf("    Heap: %d\n", countHeap);
    printf("    Stack: %d\n", countStack);

    return 0;
}
