#include "svc.c"
#include "svc.h"

int main(){

    void * helper = malloc(sizeof(char));

    int hash = hash_file(helper, "Tests/diff.txt");

    printf("%d \n", hash);

    return 0;
}

