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

    int hash = 0;
    int i=0;
    while (i<name_length){
        // printf("%d, ", name_bytes[i]);
        hash = (hash + name_bytes[i])%1000;
        i++;
    }

    int j=0;
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

    for (int j=0; j<branch->num_temp_add; j++){
        char *name = will_add[j].path;
        if (strcmp(name, file_path) == 0){
            return 1;
        }
    }
    return 0;
}

char *get_commit_id(struct commit *commit){
    int id = 0;
    char *msg = commit->message;

    BYTE res[strlen(msg)];
    string_byte(msg, res);

    int byte_len = sizeof(res)/sizeof(BYTE);
    int i=0;
    while (i<byte_len){
        id = (id + res[i])%1000;
        i++;
    }

    char *changes = commit->changes;

    return NULL;
}

char *svc_commit(void *helper , char *message){
    struct commit *this_commit = malloc(sizeof(struct commit));
    this_commit->message = message;

    // add all files to current branch
    // delete all files from the current branch
    // modify all files (check the hash value)

    return this_commit->commit_id;
}

void *get_commit(void *helper , char *commit_id){
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
    return NULL;
}


int add_to_branch(void *helper, char *file_name){
    struct svc_file *this_file = malloc(sizeof(struct svc_file));
    this_file->hash_code = hash_file(helper, file_name);
    this_file->path = file_name;
    this_file->is_modified = 0; // initialize

    struct svc_branch *cur_branch = get_current_branch(helper);
    cur_branch->num_temp_add++;

    int total = cur_branch->num_temp_add;

    struct svc_file *all_files = realloc(cur_branch->temp_area_add, total);
    int file_index = total-1;
    cur_branch->temp_area_add[file_index] = *this_file;

    return this_file->hash_code;
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
            current->num_modified++;
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


    
    // Otherwise, remove the file from SVC 
    // and return its last known hash value (from adding or committing).

    return 0;
}

int svc_reset(void *helper , char *commit_id){
    return 0;
}

char *svc_merge(void *helper , char *branch_name , resolution *resolutions , int n_resolutions){
    return NULL;
}

int main(){
    return 0;
}