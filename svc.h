#ifndef svc_h
#define svc_h

#include <stdlib.h>
#include <stdio.h>
#include <ftw.h>

#define COMMIT_LENGTH (6)
#define BUF_LEN (255)
#define TEMP_PATH (12)
#define CHAR_LENGTH (2)
#define  _POSIX_C_SOURCE 200809L
#define  _XOPEN_SOURCE 500L

typedef unsigned char BYTE;

typedef struct resolution {
    char *file_name;
    char *resolved_file;
} resolution;

struct svc_file{
    char *path;
    int hash_code;
    int is_modified; // = 1: is modified, = 0: initial val, not modified
    int will_remove;
    int will_add;
};

struct commit{
    char *commit_id;
    char *message;
    struct svc_file *changes;
    int change_num;
    struct svc_branch *after_commit; // store all files' situation after this commit in order to reset
};

struct svc_branch{
    char *name;
    struct svc_file *files;
    struct svc_file *temp_area_add;
    struct svc_file *temp_area_remove;
    struct svc_file *temp_area_modify;

    int num_files; // files are committed
    
    int num_will_add;
    int num_will_remove;
    int num_will_modify;
    
    int number_add_remove_modify;
    struct svc_file *all_changing_files;
    
    int number_commits;
    struct commit *all_commits; // all commits made in the branch
    
    int is_active; // changed by svc_checkout
    int position_in_branch_list;
};

struct svc_branch_list{
    struct svc_branch *branch_list;
    // svc_checkout
    int current_branch;
    int num_branch;
    // record all commit ids
    struct commit *commit_list;
    int num_commit;
}; 

#endif