#ifndef UTILS_H
#define UTILS_H

#define MEMORY_SIZE 10000000
#define DICTIONARY_SIZE 256

typedef enum {false, true} bool;

typedef struct {
    char character;
    bool value;
} Dictionary;

typedef struct {
    char *bm;
    int length;
} BitMap;

#endif // UTILS_H
