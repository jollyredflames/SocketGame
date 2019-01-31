#include <stdio.h>
#include <string.h>

void print_state(char *arr, int size){
    for(int i = 0; i < size; i++){
        printf("%c", arr[i]);
    }
    printf("\n");
}

void update_state(char *arr, int size){
    char arrother[size];
    arrother[0] = arr[0];
    arrother[size-1] = arr[size-1];
    for(int i = 0; i < size; i++){
        if(i == 0 || i == size - 1){
            continue;
        }
        else{
            if ((arr[i-1] == 'X' && arr[i+1] == 'X') || (arr[i-1] == '.' && arr[i+1] == '.')){
                arrother[i] = '.';
            }else{
                arrother[i] = 'X';
            }
        }
    }
    strcpy(arr, arrother);
    return;
}
