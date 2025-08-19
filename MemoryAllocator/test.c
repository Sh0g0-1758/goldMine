#include <unistd.h>
#include <stdio.h>

void* malloc(size_t);
void free(void*);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);

int main() {
    size_t len = 4;
    int* arr = (int*)malloc(sizeof(int) * len);
    for(size_t i = 0; i < len; i++) arr[i] = i;
    int* barr = (int*)calloc(len, sizeof(int));
    for(size_t i = 0; i < len; i++) barr[i] += arr[i];
    arr = realloc(arr, sizeof(int) * len * 2);
    for(size_t i = 0; i < len; i++) printf("%d ", barr[i]);
    printf("\n");
    free(arr);
    free(barr);
}
