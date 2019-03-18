#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.c"
#include <fcntl.h>
#include <sys/wait.h>
//#include "tests.c"

void check_file_div(int *words_per_child, int processes){
    for(int i = 0; i < processes; i++){
        printf("%i ", words_per_child[i]);
    }
    printf("\n");
}

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
    
    if ((infp = fopen(infile, "r")) == NULL) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }
    if ((outfp = fopen(outfile, "w")) == NULL) {
        fprintf(stderr, "Could not open %s\n", outfile);
        exit(1);
    }
    //Checkpoint: Have valid input, number of processes cause us to do something, and files opened successfully
    
    int words_per_child[processes];
    int total_records = set_up_how_many_records_each_child_reads(words_per_child, processes, infile);
    //check_file_div(words_per_child, processes); //Uncomment this line to check how many words we expect per child
    
    int pipe_arr[processes][2];
    int parent_process_id = getpid();
    int child_responsibility = -1; //int representing what "half" of the file this child is responsible for. Set as -1 as default.
    
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
                 exit(0);
                 }
                
                if(close(pipe_arr[i][0]) != 0){
                    fprintf(stderr, "Child terminated abnormally\n");
                    exit(0);
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
        //do what child is supposed to
        int offset = 0;
        for(int i = 0; i < child_responsibility; i++){
            offset += words_per_child[i];
        }
        fseek(infp, offset*sizeof(Rec), SEEK_SET);
        
        //need to read words_per_child[child_responsibility] more words from this point in file
        Rec holder[words_per_child[child_responsibility]];
        
        fread(holder, sizeof(Rec), words_per_child[child_responsibility], infp);
        
        qsort(holder, words_per_child[child_responsibility], sizeof(Rec), compare_freq);
        //CHECKPOINT: CHILD IS DONE SORTING ITS RESPONSIBILITY OF WORDS
        
        int x = 0;
        while(x != words_per_child[child_responsibility]){
            fprintf(stderr, "CHILD %i: %s, %i\n", child_responsibility, holder[x].word, holder[x].freq);
            x++;
        }
        
        for(int i = 0; i < words_per_child[child_responsibility];){
            if(write(pipe_arr[child_responsibility][1], &holder[i], sizeof(Rec)) > 0){
                i++;
            }
        }
        
        if(close(pipe_arr[child_responsibility][1]) != 0){
            perror("Failed Closing write end of pipe in child\n");
            exit(1);
        }
        
        return 0;
        
    }else{
        Rec holder[processes];
        int actual_size = 0;
        for(int i = 0; i < processes; i++){
            if(read(pipe_arr[i][0], &holder[i], sizeof(Rec)) == sizeof(Rec)){
                actual_size++;
            }
        }
        int count = 0;
        while(count < total_records){
            int min = holder[0].freq;
            int pos = 0;
            for(int r = 1; r < actual_size; r++){
                if((holder[r].freq < min && holder[r].freq > -1) || min == -1){
                    min = holder[r].freq;
                    pos = r;
                }
            }
            if(min == -1){fprintf(stderr, "CUZ BREAK\n"); break;}
            if(fwrite(&holder[pos], sizeof(struct rec), 1, outfp) > 0){
                count++;
                fprintf(stdout, "%s %d %d\n", holder[pos].word, holder[pos].freq, count);
            }
            if(read(pipe_arr[pos][0], &holder[pos], sizeof(struct rec)) < 1){
                holder[pos].freq = -1;
            }
        }
    }
    
    if(parent_process_id == getpid()){
        for(int closee = 0; closee < processes; closee++){
            close(pipe_arr[closee][0]);
        }
    }
    
    /* Close both files. */
    if (fclose(infp)) {
        perror("fclose");
        exit(1);
    }
    
    if (fclose(outfp)) {
        perror("fclose");
        exit(1);
    }
    
    return 0;
}
