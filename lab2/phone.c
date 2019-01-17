#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone_number[11];
    int digit;
    printf("Input 10 digit phone number\n");
    printf("NOTE: Entering more than 10 digits will cause only first 10 characters to be read.\n");
    scanf("%s %d", phone_number, &digit);
    if(digit < -1 || digit > 9){
        printf("ERROR\n");
        return 1;
    }
    if(digit == -1){
        printf("%s\n", phone_number);
        return 0;
    }
    printf("%c\n", phone_number[digit]);
    return 0;
}
