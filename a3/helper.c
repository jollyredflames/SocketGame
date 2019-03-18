#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"


int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    Rec *r1 = (Rec *) rec1;
    Rec *r2 = (Rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

/*set array containing integers for how many words each child should process.
 This implementation ensures all children run on +- 1 word of input*/
int set_up_how_many_records_each_child_reads(int *how_many_records_arr, int processes, char *infile_name){
    int in_size = get_file_size(infile_name);
    int num_records = in_size/(sizeof(Rec));
    
    for(int i = 0; i < processes; i++){
        how_many_records_arr[i] = num_records/processes;
        if(i < num_records % processes){
            how_many_records_arr[i] += 1;
        }
    }
    return num_records;
}


