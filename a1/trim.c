#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
        fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
        exit(1);
    }
    
    FILE *file_trace = fopen(argv[1], "r");
    FILE *file_marker = fopen(argv[2], "r");
    
    // Addresses should be stored in unsigned long variables
    // unsigned long start_marker, end_marker;
    unsigned long start_marker = 0;
    unsigned long end_marker = 0;
    fscanf(file_marker, "0x%lx 0x%lx", &start_marker, &end_marker);
    
    int found_start = 0;
    int found_end = 0;
    
    unsigned long current_hex = 0;
    char ch;
    while(fscanf(file_trace, " %c %lx,%*d", &ch, &current_hex) != EOF && found_end == 0){
        /* For printing output, use this exact formatting string where the
         * first conversion is for the type of memory reference, and the second
         * is the address
         */
        if(current_hex == start_marker){
            found_start = 1;
            continue;
        }
        if(current_hex == end_marker){
            found_end = 1;
            continue;
        }
        if(found_start == 1){
            printf("%c,%#lx\n", ch, current_hex);
        }
    }    
    return 0;
}

