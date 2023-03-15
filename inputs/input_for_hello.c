#include <stdio.h>
#include <stdlib.h>

int main() {
    int* ptr = (int*) malloc(3 * sizeof(int));
    int a = 5;
    if (ptr == NULL) {
        printf("Error: memory allocation failed\n");
        return 0;
    }
    ptr[2] = 4; // correct index
    free(ptr);
    ptr = NULL; // nullify dangling pointer
    return 0;
}