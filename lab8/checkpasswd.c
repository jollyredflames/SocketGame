#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
    char user_id[MAXLINE];
    char password[MAXLINE];
    int exitStat;
    int fd[2];
    char *exits[4] = {SUCCESS, "ERROR\n", INVALID, NO_USER};
    
    if(fgets(user_id, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }
    if(fgets(password, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }
    if (pipe(fd)==-1)
    {
        perror("Pipe Failed");
        exit(1);
    }
    
    int process = fork();
    
    if(process > 0){
        close(fd[0]);
        write(fd[1], user_id, MAX_PASSWORD);
        write(fd[1], password, MAX_PASSWORD);
        close(fd[1]);
        wait(&exitStat);
    }else{
        close(fd[1]);
        if(dup2(fd[0], STDIN_FILENO)){
            perror("dup2");
            exit(5);
        }
        int never = execl("./validate", "validate");
        if(never){
            perror("ExecL failed");
            exit(1);
        }
    }
    if (WIFEXITED(exitStat)){
        printf("%s", exits[WEXITSTATUS(exitStat)]);
    }
    else{
        fprintf(stderr, "Child returned with no exit status");
        exit(1);
    }

  return 0;
}
