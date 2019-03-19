#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

typedef struct rec {
    int freq;
    char word[SIZE];
} Rec;

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
int set_up_how_many_records_each_child_reads(int *how_many_records_arr, int processes, char *infile_name);
int getOffset(int child_responsibility, int *words_per_child);

#endif /* _HELPER_H */
