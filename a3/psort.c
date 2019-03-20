#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.c"
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    extern char *optarg;
    FILE *infp, *outfp;
    char ch;
    int processes = 0;
    char *infile = NULL, *outfile = NULL;
    
    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
        exit(1);
    }
    
    /* Read arguments. Make sure no badly defined arguments, and number of processes are non-negative*/
    while ((ch = getopt(argc, argv, "n:f:o:")) != -1) {
        switch(ch) {
            case 'n':
                if(optarg >= 0){
                    processes = strtol(optarg, NULL, 10);
                } else{
                    fprintf(stderr, "Number of Processes cannot be negative\n");
                    exit(1);
                }
                break;
            case 'f':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            default:
                fprintf(stderr, "Usage: mkwords -f <input file name> -o <output file name>\n");
                exit(1);
        }
    }
    
    //if 0 processes, create empty file with name outfile and exit with no errors
    if(processes == 0){
        if ((outfp = fopen(outfile, "w")) == NULL) {
            fprintf(stderr, "Could not open %s\n", outfile);
            exit(1);
        }
        if (fclose(outfp)) {
            perror("fclose");
            exit(1);
        }
        return 0;
    }
    
    //Checkpoint: Have valid input and number of processes cause us to do something
    
    int words_per_child[processes];
    int total_records = set_up_how_many_records_each_child_reads(words_per_child, processes, infile);
    int pipe_arr[processes][2];
    int parent_process_id = getpid();
    int child_responsibility = -1; //int representing what "half" of the file this child is responsible for. Set as -1 as default.
    
    //check_file_div(words_per_child, processes); //Uncomment this line to check how many words we expect per child
    
    //Set up child processes. Open parent for reading and child for writing for each pipe.
    for(int i = 0; i < processes; i++){
        if (pipe(pipe_arr[i])==-1){
            perror("Pipe Failed");
            exit(1);
        }
        int fork_ret = fork();
        if(fork_ret == 0){
            //inside child
            child_responsibility = i;
            //Setup checker to ensure when a write is successful or not
            if (fcntl(pipe_arr[i][1], F_SETFL, O_NONBLOCK) < 0){
                fprintf(stderr, "Child terminated abnormally\n");
                exit(1);
            }
            if(close(pipe_arr[i][0]) != 0){
                fprintf(stderr, "Child terminated abnormally\n");
                exit(1);
            }
            break;
        }
        else{
            //inside parent
            if(close(pipe_arr[i][1]) != 0){
                perror("Failed Closing write end of pipe in parent\n");
                exit(1);
            }
        }
    }
    
    if(getpid() != parent_process_id){
        //Inside Child. Offset input file, read as many records as child is responsible for, sort, then pipe to parent.
        if ((infp = fopen(infile, "r")) == NULL) {
            fprintf(stderr, "Could not open %s\n", infile);
            exit(1);
        }
        
        int offset = getOffset(child_responsibility, words_per_child);
        fseek(infp, offset*sizeof(Rec), SEEK_SET);
        
        //need to read words_per_child[child_responsibility] more words from this point in file
        Rec holder[words_per_child[child_responsibility]];
        int freadRet = 0;
        int maxTries = 15;
        
        //Try to read records from file maximum of 15 times. If the right number of records are read, do not try to read from file again.
        do{
            freadRet = fread(holder, sizeof(Rec), words_per_child[child_responsibility], infp);
            if(freadRet == words_per_child[child_responsibility]){
                break;
            }
            usleep(5);
            maxTries--;
        }while(maxTries > 0);
        
        if(freadRet != words_per_child[child_responsibility]){
            fprintf(stderr, "Reached max amount of fread tries\n");
            exit(1);
        }
        
        qsort(holder, words_per_child[child_responsibility], sizeof(Rec), compare_freq);
        //CHECKPOINT: CHILD HAS SORTED ARRAY OF WORDS IT IS RESPONSIBLE FOR
        
        for(int i = 0; i < words_per_child[child_responsibility];){
            //By fcntl if write > 1, write was successful, if not, no bytes will be written to pipe so try write again.
            if(write(pipe_arr[child_responsibility][1], &holder[i], sizeof(Rec)) > 0){i++;}
        }
        
        if(close(pipe_arr[child_responsibility][1]) != 0){
            perror("Failed Closing write end of pipe in child\n");
            exit(1);
        }
        
        if (fclose(infp)) {
            perror("fclose");
            exit(1);
        }
        //Done with child so let child exit with code 0
        return 0;
        
    }else{
        //Inside Parent. Read pipe from each child, then merge sort and print to file.
        Rec holder[processes];
        int actual_size = 0;
        for(int i = 0; i < processes; i++){
            if(read(pipe_arr[i][0], &holder[i], sizeof(Rec)) == sizeof(Rec)){
                actual_size++;
            }
        }
        
        if ((outfp = fopen(outfile, "w")) == NULL) {
            fprintf(stderr, "Could not open %s\n", outfile);
            exit(1);
        }

        //actual merge sort. Look through holder, find record with smallest frequency, write it, read from the pipe which last spit out smallest, and repeat. If a frequency is negative, ignore it.
        
        mergeSortPipeAndWrite(holder, processes, actual_size, total_records, outfp, pipe_arr);
        
        
        if (fclose(outfp)) {
            perror("fclose");
            exit(1);
        }
        
    }
    
    //Close all pipes. only parent comes here.
    for(int i = 0; i < processes; i++){
        int stat;
        close(pipe_arr[i][0]);
        wait(&stat);
        if(WIFSIGNALED(stat) || WIFSTOPPED(stat)){
            fprintf(stderr, "Child terminated abnormally\n");
        }
    }
    
    return 0;
}




