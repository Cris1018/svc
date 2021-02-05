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
    int is_modified;
    // = 1: is modified
    // = 0: initial val, not modified
};

struct commit{
    char *commit_id;
    char *message;
    char *changes; 
};

struct svc_branch{
    struct svc_file *files;
    struct svc_file *temp_area_add;
    struct svc_file *temp_area_remove;
    
    int num_files;
    int num_temp_add;
    int num_temp_remove;
    int num_modified;
    // name of the branch
    char *name;
    // all commits made in the branch
    struct commit *all_commits;
    // changed by svc_checkout
    int is_active;
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