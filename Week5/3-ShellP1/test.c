#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    char str1[] = "hola que hace";
    char str2[] = "soy tu pou";

    // char *substr1 = strtok(str1, " ");
    // printf("substr1: %s\n", substr1);
    // char *substr2 = strtok(str2, " ");
    // printf("substr2: %s\n", substr2);
    // substr1 = strtok(str1 + 5, " ");
    // printf("substr1: %s\n", substr1);
    printf("str1 len: %ld\n", strlen(str1));

    return 0;
}