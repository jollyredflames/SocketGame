#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone_number[11];
    int digit;
    printf("Input 10 digit phone number\n");
    printf("NOTE: Entering more than 10 digits will cause only first 10 characters to be read.\n");
    scanf("%s", phone_number);
    int return_val = 0;
    while (scanf(" %d", &digit) != EOF) {
        if(digit < -1 || digit > 9){
            printf("ERROR\n");
            return_val = 1;
        }
        else if(digit == -1){
            printf("%s\n", phone_number);
        }
        else{
            printf("%c\n", phone_number[digit]);
        }
    }
    return return_val;
    
}
