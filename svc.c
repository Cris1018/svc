#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <ftw.h>
#include "svc.h"

void *svc_init(){
    // a list of branch
    struct svc_branch_list *all_branches = malloc(sizeof(struct svc_branch_list));
    all_branches->branch_list = malloc(sizeof(struct svc_branch));
    all_branches->branch_list[0].name = "master";
    all_branches->branch_list[0].is_active = 1;
    all_branches->branch_list[0].position_in_branch_list = 0;

    all_branches->num_branch = 1;
    all_branches->current_branch = 0;
    all_branches->num_commit = 0;
    
    return (void *)all_branches;
}

void cleanup(void *helper){


    
}

struct svc_branch *get_current_branch(void *helper){
    struct svc_branch_list *branches = (struct svc_branch_list *)helper;
    int position = branches->current_branch;
    struct svc_branch *current = &branches->branch_list[position];
    return current;
}

void string_byte(char *str, BYTE *res){
    int i = 0;
    while (str[i] != '\0'){
        res[i] = str[i];
        i++;
    }
}

int hash_file(void *helper, char *file_path){
    if (file_path == NULL){
        return -1;
    }

    // read the file
    FILE * ptr = fopen(file_path, "r");
    fseek(ptr, 0, SEEK_END);
    long filelen = ftell(ptr);
    rewind(ptr);

    char buffer[filelen];

    fread(buffer, sizeof(char), filelen, ptr);
    fclose(ptr);

    BYTE name_bytes[strlen(file_path)];
    string_byte(file_path, name_bytes);

    BYTE content_bytes[strlen(buffer)];
    string_byte(buffer, content_bytes);

    int name_length = sizeof(name_bytes)/sizeof(BYTE);
    int content_length = sizeof(content_bytes)/sizeof(BYTE);

    int hash = 0, i = 0, j = 0;
    while (i<name_length){
        // printf("%d, ", name_bytes[i]);
        hash = (hash + name_bytes[i])%1000;
        i++;
    }

    while (j<content_length){
        // printf("%d, ", content_bytes[j]);
        hash = (hash + content_bytes[j])%2000000000;
        j++;
    }
    return hash;
}

int check_file_exist(void *helper, char *file_path){
    struct svc_branch *branch = get_current_branch(helper);

    // added or commited file list
    struct svc_file *commited_files = branch->files;
    struct svc_file *will_add = branch->temp_area_add;

    for (int i=0; i<branch->num_files; i++){
        char *name = commited_files[i].path;
        if (strcmp(name, file_path) == 0){
            // file exist in commited files
            return 1;
        }
    }

    for (int j=0; j<branch->num_will_add; j++){
        char *name = will_add[j].path;
        if (strcmp(name, file_path) == 0){
            return 1;
        }
    }
    return 0;
}

struct commit *create_commit_change(void *helper){
    // go to current branch and find all changed files
    struct svc_branch *branch = get_current_branch(helper);
    struct commit *create_commit = malloc(sizeof(struct commit));

    create_commit->changes = branch->all_changing_files;
    create_commit->change_num = branch->number_add_remove_modify;
    return create_commit;
}

void get_count_changing_files(void *helper){
    struct svc_branch *branch = get_current_branch(helper);
    int num_change = 0;
    
    num_change += branch->num_will_add;
    num_change += branch->num_will_remove;
    num_change += branch->num_will_modify;
    branch->number_add_remove_modify = num_change;

    struct svc_file *changing_files = malloc(sizeof(struct svc_file)*num_change);
    int i=0;
    
    for (i=0; i<branch->num_will_add; i++){
        changing_files[i] = branch->temp_area_add[i];
    }

    for (i=0; i<branch->num_will_remove; i++){
        changing_files[i+branch->num_will_add] = branch->temp_area_remove[i];
    }

    for (i=0; i<branch->num_will_modify; i++){
        changing_files[i+branch->num_will_add+branch->num_will_remove] = branch->temp_area_modify[i];
    }
    branch->all_changing_files = changing_files;
}

// names -> malloc(sizeof(char *) * number_eles)
void sort_changing_files(void *helper){

    struct svc_branch *branch = get_current_branch(helper);
    int num_files = branch->number_add_remove_modify;
    struct svc_file *changing_files = branch->all_changing_files;
    
    for (int i=1; i<num_files; i++){
        for (int j=0; j<i; i++){
            if (strcmp(changing_files[i].path, changing_files[j].path)>0){
                // names[j], names[i]  char pointers
                struct svc_file *temp = &changing_files[i];
                changing_files[j] = changing_files[i];
                changing_files[i] = *temp;
            }
        }
    }
}

char *get_commit_id(struct commit *commit){
    int id = 0;
    char *msg = commit->message;

    BYTE message_byte[strlen(msg)];
    string_byte(msg, message_byte);

    int byte_len = sizeof(message_byte)/sizeof(BYTE);
    int i=0;
    while (i<byte_len){
        id = (id + message_byte[i])%1000;
        i++;
    }

    // those files are sorted in lexi order
    struct svc_file *changing_files = commit->changes;

    for (int i=0; i<commit->change_num; i++){
        struct svc_file current = changing_files[i];
        
        if (current.will_add == 1){
            id += 376591;
        } 
        if (current.will_remove == 1){
            id += 85973;
        }
        if (current.is_modified == 1){
            id += 9573681;
        }

        BYTE res[strlen(current.path)];
        string_byte(current.path, res);

        int length = sizeof(res)/sizeof(res[0]);

        for (int j=0; j<length; j++){
            int increment = id*(res[j]%37)%15485863 + 1;
            id += increment;
        }
    }

    sprintf(commit->commit_id, "%06x", id);
    return commit->commit_id;
}

/**
 * first add then delete - do nothing
 * first delete then modify - delete
 * first modify then delete - delete
 * first add then modify - add the modified file
 * first delete then add - do nothing
 * */
char *svc_commit(void *helper , char *message){
    get_count_changing_files(helper);
    sort_changing_files(helper);
    
    struct commit *this_commit = create_commit_change(helper);
    this_commit->message = message;
    this_commit->commit_id = get_commit_id(this_commit);

    int adding_num = 0;

    // add all files to current branch
    for (int i=0; i<this_commit->change_num; i++){
        if (this_commit->changes[i].will_add == 1){
            if (this_commit->changes[i].will_remove == 0){
                adding_num++;
            }
        }

        if (this_commit->changes[i].will_remove == 1){

        }
    }

    // delete all files from the current branch
    // modify all files (check the hash value)

    return this_commit->commit_id;
}

void *get_commit(void *helper , char *commit_id){
    if (commit_id == NULL){
        return NULL;
    }
    
    struct svc_branch *branch = get_current_branch(helper);

    int num_commits = branch->number_commits;

    for (int i=0; i<num_commits; i++){
        if (strcmp(commit_id, branch->all_commits[i].commit_id) == 0){
            return (void *)&branch->all_commits[i];
        }
    }
    return NULL;
}

char **get_prev_commits(void *helper , void *commit , int *n_prev){
    if (n_prev == NULL){
        return NULL;
    }

    return NULL;
}

void print_commit(void *helper, char *commit_id){
    if (commit_id == NULL){
        printf("invalid commit id\n");
        return;
    }


}

int svc_branch(void *helper, char *branch_name){
    if (branch_name == NULL){
        return -1;
    }

    int length = strlen(branch_name);
    for (int i=0; i<length; i++){
        if ((isalnum(branch_name[i])) || branch_name[i] == '_' || branch_name[i] == '/' || branch_name[i] == '-'){
            // do nothing
        } else {
            return -1;
        }
    }

    struct svc_branch_list *svc = (struct svc_branch_list *)helper;
    struct svc_branch *ls_branch = svc->branch_list;

    int cur_num_branches = svc->num_branch;
    
    // iterate all branch's name
    for (int i=0; i<cur_num_branches; i++){
        char *curr_name = ls_branch[i].name;

        if (strcmp(branch_name, curr_name) == 0){
            return -2;
        }
    }

    svc->num_branch++;
    struct svc_branch *new_branch = realloc(ls_branch, sizeof(struct svc_branch)*svc->num_branch);
    
    // add the branch to the svc, not check out/ active
    new_branch->name = branch_name;
    new_branch->is_active = 0;
    new_branch->position_in_branch_list = cur_num_branches;

    return 0;
}

int check_branch_exists(void *helper, char *branch_name){
    struct svc_branch_list *svc = (struct svc_branch_list *)helper;
    
    int i=0;
    int total_branches = svc->num_branch;
    while (i<total_branches){

        char *cur_branch_name = svc->branch_list[i].name;
        if (strcmp(branch_name, cur_branch_name) == 0){
            return 1;
        }
        i++;
    }
    return 0; // the branch does not exist
}

int svc_checkout(void *helper , char *branch_name){
    if (branch_name == NULL || check_branch_exists(helper, branch_name) == 0){
        return -1;
    }
    
    // change the current_branch
    struct svc_branch_list *svc = (struct svc_branch_list *)helper;

    int i=0;
    int total_branches = svc->num_branch;
    while (i<total_branches){
        struct svc_branch cur = svc->branch_list[i];
        if (strcmp(branch_name, cur.name) == 0){
            cur.is_active = 1;
            int cur_position = cur.position_in_branch_list;
            svc->current_branch = cur_position;
            return 0;
        }
        i++;
    }
    
    // branch not exist
    return -1;
}



char **list_branches(void *helper , int *n_branches){
    // Print all the branches in the order they were created. 
    // In addition, return a dynamically allocated array of the branch names in the same order, 
    // and store the number of branches in the memory area pointed to by n_branches. 
    // If n_branches is NULL, return NULL and do not print anything

    if (n_branches == NULL){
        return NULL;
    }
    
    struct svc_branch_list *svc = (struct svc_branch_list *)helper;
    int number_branch = svc->num_branch;
    
    char **res = malloc(number_branch*sizeof(char *));
    for (int i=0; i<number_branch; i++){
        res[i] = svc->branch_list[i].name;
    }

    n_branches = &number_branch;
    return res;
}


int add_to_branch(void *helper, char *file_name){
    struct svc_file *this_file = malloc(sizeof(struct svc_file));
    this_file->hash_code = hash_file(helper, file_name);
    this_file->path = file_name;
    
    this_file->is_modified = 0; // initialize
    this_file->will_add = 1; // will add
    this_file->will_remove = 0; // initialize

    struct svc_branch *cur_branch = get_current_branch(helper);
    cur_branch->num_will_add++;

    int total = cur_branch->num_will_add;

    struct svc_file *all_files = realloc(cur_branch->temp_area_add, total);
    int file_index = total-1;
    cur_branch->temp_area_add[file_index] = *this_file;

    return this_file->hash_code;
}

/**
 * a file may be removed by the user
 * */
void check_manually_delete(void *helper){


}

void check_files_modified_in_current_branch(void *helper){
    struct svc_branch *current = get_current_branch(helper);
    struct svc_file *all_files = current->files;
    int num_files = current->num_files;
    int i=0;
    while (i<num_files){
        struct svc_file cur_file = all_files[i];
        int cur_hash = cur_file.hash_code;
        int new_hash = hash_file(helper, cur_file.path);

        if (cur_hash!=new_hash){
            // file is modified
            cur_file.is_modified = 1;
            current->num_will_modify++;
        }
        i++;
    }
}

int svc_add(void *helper , char *file_name){
    if (file_name == NULL){
        return -1;
    }

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL){
        return -3;
    }

    // it is commited or already added
    if (check_file_exist(helper, file_name) == 1){
        return -2;
    }
    // return the hash code
    return add_to_branch(helper, file_name);
}

int svc_rm(void *helper , char *file_name){
    if (file_name == NULL){
        return -1;
    } 
    // If the file with the given name is not being tracked, return -2. 
    if (check_branch_exists(helper, file_name) == 0){
        return -2;
    }
    // Otherwise, remove the file from SVC - not commit
    // and return its last known hash value (from adding or committing).
    struct svc_branch *current = get_current_branch(helper);
    struct svc_file *all_files = current->files;
    struct svc_file *all_will_add = current->temp_area_add;
    current->num_will_remove++;

    // realloc the temp area remove!!!!!!

    int res;
    for (int i=0; i<current->num_files; i++){
        if (strcmp(all_files[i].path, file_name) == 0){
            all_files[i].will_remove = 1; // mark as will be removed
            res = hash_file(helper, all_files[i].path);
        }
    }

    for (int j=0; j<current->num_will_add; j++){
        if (strcmp(all_will_add[j].path, file_name) == 0){
            all_will_add[j].will_remove = 1;
            res = hash_file(helper, all_will_add[j].path);
        }
    }
    return res;
}

int svc_reset(void *helper , char *commit_id){
    if (commit_id == NULL){
        return -1;
    }
    return 0;
}

int check_commit_exist(void *helper, char *commit_id){
    return 1;
}

char *svc_merge(void *helper , char *branch_name , resolution *resolutions , int n_resolutions){
    return NULL;
}

int main(){
    return 0;
}