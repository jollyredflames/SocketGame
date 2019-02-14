#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}

/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}

/* Generate the signature of the given string word with letter l
 */
char *generate_word_sig(char *word, char letter, int len_word){
    char *sig = malloc(sizeof(char)*(len_word+1));
    if (sig == NULL){
        perror("Malloc ERROR Line:27 Family.c");
        exit(1);
    }
    for(int i = 0; i < len_word; i++){
        sig[i] = '-';
    }
    for(int i = 0; i < len_word; i++){
        if(word[i] == letter){
            sig[i] = letter;
        }
    }
    sig[len_word] = '\0';
    return sig;
}

/* return pointer to last Family struct (the one with Family->next == NULL)
 */
Family *ret_last_family(Family *head){
    Family *current = head;
    while(current->next != NULL){
        current = current->next;
    }
    return current;
}

/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;
    
    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is 
   a copy of str. Initialize word_ptrs to point to 
   family_increment+1 pointers, numwords to 0, 
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
    Family *fam_ptr = malloc(sizeof(Family));
    if (fam_ptr == NULL){
        perror("Malloc during start of new_fam failed Line:51 Family.c");
        exit(1);
    }
    fam_ptr->signature = malloc(sizeof(char*));
    fam_ptr->next = malloc(sizeof(Family*));
    strcpy(fam_ptr->signature, str);
    fam_ptr->word_ptrs = malloc(sizeof(char*)*(family_increment + 1));
    if (fam_ptr->word_ptrs == NULL){
        perror("Malloc setting word_ptrs of new_fam failed Line:57 Family.c");
        exit(1);
    }
    
    //NOT NEEDED?????????????????????????/
    for(int i = 0; i < family_increment+1; i++){
        fam_ptr->word_ptrs[i] = malloc(sizeof(char*));
        if (fam_ptr->word_ptrs[i] == NULL){
            perror("Malloc setting word_ptrs[i] of new_fam failed Line:64 Family.c");
            exit(1);
        }
        fam_ptr->word_ptrs[i] = NULL;
    }
    
    fam_ptr->num_words = 0;
    fam_ptr->max_words = family_increment;
    fam_ptr->next = NULL;
    
    return fam_ptr;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    
    if(fam->num_words > fam->max_words){
        fprintf(stderr, "fam->numwords should never be greater than max_words+1 ever Line:125 family.c\n");
    }
    
    if(fam->num_words >= fam->max_words){ // ==  + 1
        fam->word_ptrs = realloc(fam->word_ptrs, sizeof(fam->word_ptrs) + sizeof(char*)*family_increment); //why sizeof(fam->word_ptrs)?
        if (fam->word_ptrs == NULL){
            perror("Realloc failed while increasing size of word_ptrs line:86 Family.c");
            exit(1);
        }
        
        fam->max_words = ((int) sizeof(fam->word_ptrs)/sizeof(char*)) - 1;
        //max words will just be max words + family increment if you do it correctly
        
        for(int i = fam->num_words; i < fam->max_words + 1; i++){ //why + 1
            fam->word_ptrs[i] = malloc(sizeof(char*));
            if (fam->word_ptrs[i] == NULL){
                perror("Malloc setting word_ptrs[i] of new_fam failed Line:64 Family.c");
                exit(1);
            }
            fam->word_ptrs[i] = NULL; //why NULL? imo delete this linec
        }
    }
    fam->word_ptrs[fam->num_words] = word;
    fam->num_words++;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
    Family *head = fam_list;
//    if(head == NULL){
//        return NULL;
//    }
//    printf("\n\nYOU FUCKING DILDO %s\n\n", head->signature);
//    while(strcmp(head->signature, sig) == 0 && head->next == NULL){
//        head = head->next;
//    }
//    if(!strcmp(head->signature, sig)){
//        return head;
//    }
//    return NULL;
    
    while(head){
        if(strcmp(head->signature, sig) == 0){
            return head;
        }
        head = head->next;
    }
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    Family *most_words_fam = fam_list;
    int most_words = most_words_fam->num_words;
    while(fam_list->next != NULL){
        if(fam_list->num_words > most_words){
            most_words_fam = fam_list;
            most_words = most_words_fam->num_words;
        }
        fam_list = fam_list->next;
    }
    if(most_words == 0){
        return NULL;
    }
    return most_words_fam;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *head = fam_list;
    if(head == NULL){
        return;
    }
    while(head->next != NULL){
        Family *current = fam_list;
        while(current->next != NULL && (current->next)->next != NULL){
            current = current->next;
        }
        Family *last;
        if(current->next != NULL){
            last = current->next;
        }else{
            last = current;
        }
        for(int i = 0; i < ((int) sizeof(last->word_ptrs)/sizeof(char*)); i++){
            free(last->word_ptrs[i]);
        }
        free(last->word_ptrs);
        free(last);
        current->next = NULL;
    }
    free(head);
}
//ABOVE A LIKELY SOURCE OF ERRORS

/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    int len_word = strlen(word_list[0]);
    
    int len_word_list = 0;
    while(word_list[len_word_list] != NULL){
        len_word_list++;
    }
    
    Family *head = new_family(generate_word_sig(word_list[0], letter, len_word));
    
    char *sig;
    
    for(int i = 1; i < len_word_list; i++){
        sig = generate_word_sig(word_list[i], letter, len_word);
        Family *node = find_family(head, sig);
        if(node == NULL){
            Family *new_Node = new_family(sig);
            ret_last_family(head)->next = new_Node;
            add_word_to_family(new_Node, word_list[i]);
        }else{
            add_word_to_family(node, word_list[i]);
        }
    }
    return head;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    char **new_word_list = malloc(sizeof(char*)*fam->num_words);
    if (new_word_list == NULL){
        perror("Malloc failed line:208 Family.c");
        exit(1);
    }
    for(int i = 0; i < fam->num_words; i++){
        new_word_list[i] = fam->word_ptrs[i];
    }
    return new_word_list;
}


/* Return a pointer to a random word from fam. 
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    srand(time(NULL));
    int random_num = rand() % (fam->num_words);
    return fam->word_ptrs[random_num];
}
