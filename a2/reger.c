#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int main () {

    int reti;
    char msgbuf[100];
    
    regex_t regex;
    
    reti = regcomp(&regex, "^...$", REG_ICASE);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    
    for(int i = 0; i < 10; i++){
        char comp[10];
        
        scanf("%s", comp);
        comp[9] = '\0';
        printf("\n%s\n", comp);
        
        reti = regexec(&regex, comp, 0, NULL, 0);
        
        if (reti) {
            printf("No Match\n");
        }
        else if (reti == 0) {
            printf("Match\n");
        }
        else {
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            printf("Regex match failed: %s\n", msgbuf);
            exit(1);
        }
    }
}
