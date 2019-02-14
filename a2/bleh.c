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
    
    for(int i = 0; i < family_increment+1; i++){
        fam_ptr->word_ptrs[i] = NULL;
    }
    
    fam_ptr->num_words = 0;
    fam_ptr->max_words = family_increment;
    fam_ptr->next = NULL;
    
    return fam_ptr;
}
