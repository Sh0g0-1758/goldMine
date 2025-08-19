#include <pthread.h>
#include <string.h>
#include <unistd.h>

/**
 * HEADER
 *       - Aligned to 16 bytes, so that the data allocated always starts at a 16 byte multiple
 *       - Size of the header is 32 bytes, 8(size_t) + 4(unsigned) + 4(padding) + 8(ptr) + 8(padding)
 *       - the last padding is to conform with the alignment requirement, we should be able to create an
 *         an array of header, to do so, the struct must end at a multiple of 16
 */

typedef char ALIGN[16] __attribute__((aligned(16)));

union header {
    struct {
        size_t size;
        unsigned is_free;
        union header* next;
    };
    ALIGN stub;
};

typedef union header header_t;

header_t* head, *tail;
pthread_mutex_t global_malloc_lock;

void* get_free_block(size_t size) {
    header_t* curr = head;
    while(curr) {
        if(curr->is_free && curr->size >= size) {
            return (void*)curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void* malloc(size_t size) {
    write(STDOUT_FILENO, "MALLOC\n", 7);
    if(!size) return NULL;

    size_t total_size;
    void* block;
    header_t* header;

    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)get_free_block(size);
    if (header) {
        header->is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == ((void*)-1)) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->size = size;
    header->is_free = 0;
    header->next = NULL;

    if (!head) head = header;
    if (tail)  tail->next = header;

    tail = header;

    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

void free(void* block) {
    write(STDOUT_FILENO, "FREE\n", 5);
    if(!block) return;

    header_t* curr = ((header_t*)(block) - 1);
    void* programBreak = sbrk(0);
    if ((char*)block + curr->size == programBreak) {
        pthread_mutex_lock(&global_malloc_lock);
        if(head == tail) {
            head = tail = NULL;
        } else {
            header_t* tmp = head;
            while(tmp->next != tail) tmp = tmp->next;
            tmp->next = NULL;
            tail = tmp;
        }
        pthread_mutex_unlock(&global_malloc_lock);

        sbrk(0 - curr->size - sizeof(header_t));
    } else {
        curr->is_free = 1;
    }
}

void* calloc(size_t nlen, size_t nbytes) {
    write(STDOUT_FILENO, "CALLOC\n", 7);
    if(!nlen || !nbytes) return NULL;
    size_t size = nlen * nbytes;
    if(size / nlen != nbytes) return NULL;
    void* block = malloc(size);
    if(!block) return NULL;
    memset(block, 0, size);
    return block;
}

void* realloc(void* block, size_t size) {
    write(STDOUT_FILENO, "REALLOC\n", 8);
    if(!block || !size) return NULL;
    header_t* header = ((header_t*)block - 1);
    if(header->size >= size) return block;
    void* ret = malloc(size);
    if(ret) {
        memcpy(ret, block, header->size);
        free(block);
    }
    return ret;
}
