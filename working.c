#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

typedef unsigned char BYTE;

void string2ByteArr(char *str, BYTE *res){
    int i = 0;
    while (str[i] != '\0'){
        res[i] = str[i];
        i++;
    }
}

char **sort_files(char **names, int num_files){
    
    for (int i=1; i<num_files; i++){
        for (int j=0; j<i; j++){
            if (strcmp(names[j], names[i])>0){
                // names[j], names[i]  char pointers
                char *temp = names[j];
                names[j] = names[i];
                names[i] = temp;
            }
        }
    }
    return names;
}

int main(){

    char *string = "hello";
    // string = hello %s
    // string[0]/ string[1] = h, e %c
    // *string/ *(string+1) = h, e %c

    // char **string_array = malloc(sizeof(char *)*5);

    // string_array[0] = "good";
    // string_array[1] = "hello00000";
    // string_array[2] = "zzzz..";
    // string_array[3] = "C#";
    // string_array[4] = "world";

    // char **sorted = sort_files(string_array, 5);

    // for (int i=0; i<5; i++){
    //     printf("%s \n", sorted[i]);
    // }

    int number = 48;
    char *res = malloc(sizeof(char)*8);

    sprintf(res, "%06x", number);
    printf("%s \n", res);


    // read a file to bytes
    // FILE *ptr = fopen("Hello.txt", "rb"); // open the file in binary mode
    // fseek(ptr, 0, SEEK_END); // jump to the end of file
    // long filelen = ftell(ptr); // get the current byte offset in tthe file
    // rewind(ptr);

    // BYTE res[filelen*2];
    // char *buffer = malloc(sizeof(char)*filelen*2);
    //fread(buffer,1, filelen, ptr);

    // char *buffer = "hello.py";
    // BYTE res[strlen(buffer)];
    
    // string2ByteArr(buffer, res);

    // int i=0;
    // while (buffer[i] != '\0'){
    //     if (buffer[i] == '\n'){
    //         buffer[i] = ' ';
    //     }
        
    //     printf("%c - %d\n", buffer[i], res[i]);
    //     i++;
    // }
    
    
    // fclose(ptr);



    // printf("%s \n", buffer);

    // char * branch_name = "helloworld_113-/________";
    // int length = strlen(branch_name);

    // for (int i=0; i<length; i++){
    //     if ((isalnum(branch_name[i])) || branch_name[i] == '_' || branch_name[i] == '/' || branch_name[i] == '-'){
    //         // some illegal chars in the name
    //         printf("%c \n", branch_name[i]);
    //     }
    // }
    // // printf("success\n");
    return 0;
}