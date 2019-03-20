#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"

/*Return file size given its name*/
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

/*Given a child, return how many bytes from beginning of file this child should start to read Records from*/
int getOffset(int child_responsibility, int* words_per_child){
    int offset = 0;
    for(int i = 0; i < child_responsibility; i++){
        offset += words_per_child[i];
    }
    return offset;
}

/*given # processes pipe, find the smallest record and write to file. Once the smallest Record from processes is written, read from 'this' pipe again (the pipe that gave the last smallest Record.) If the pipe has no more records, set the pipe to -1 so we know to not sort this record. otherwise run again.*/
void mergeSortPipeAndWrite(Rec *holder, int processes, int actual_size, int total_records, FILE *outfp, int pipe_arr[processes][2]){
    int count = 0;
    while(count < total_records){
        int min = holder[0].freq;
        int pos = 0;
        for(int i = 1; i < actual_size; i++){
            if((holder[i].freq < min && holder[i].freq > -1) || min == -1){
                min = holder[i].freq;
                pos = i;
            }
        }
        if(min == -1){
            break;
        }
        if(fwrite(&holder[pos], sizeof(struct rec), 1, outfp) > 0){
            count++;
            //fprintf(stdout, "%s %d %d of %i\n", holder[pos].word, holder[pos].freq, count, total_records);
        }
        if(read(pipe_arr[pos][0], &holder[pos], sizeof(struct rec)) < 1){
            //If no data to be read from this pipe, set frequency to -1 so we can stop sorting it.
            holder[pos].freq = -1;
        }
    }
}
