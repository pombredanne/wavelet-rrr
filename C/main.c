#include "WaveletTree.h"
#include "RRR.h"
#include "Utils.h"
#include <sys/time.h>

int main (int argc, char* argv[]) {

    FILE *inputFile = NULL;
    char* filename, *input, *rank_or_select;
    int length = 0, block, superblock, bound, Rank, Select, i;
    char c, s_char;
    bool header_line = false;
    WaveletTree *tree = NULL;
    struct timeval start_time, end_time;
    long us;

    //global_table = (RRRTable *)calloc (1, sizeof(RRRTable));

    if (argc != 5) {
        printf ("Wrong number of parameters: WavletTree.exe filename rank_or_select char bound expected\n");
        return -1;
    }
    /** Arguments input */
    filename = argv[1];
    rank_or_select = argv[2];
    s_char = argv[3][0];
    bound = atoi(argv[4]);

    /** Opening input file */
    inputFile = fopen(filename, "r");

    if (inputFile == NULL) {
        printf ("%s doesn't exist\n", filename);
        return -1;
    }

    input = (char *)calloc (MEMORY_SIZE, sizeof(char));

    /** Reading through file and skipping newline characters */
    while ((c = fgetc(inputFile)) != EOF) {
        if (c == '>') {
            header_line = true;
        }
        if (c == '\n') {
            header_line = false;
            continue;
        }
        if (!header_line) {
            input[length] = c;
            length++;
        }
    }
    length--;

    input = (char *) realloc (input, length * sizeof(char));

    /** Checking if the upper boundary is too high */
    if (bound > length) {
        printf ("Boundary too high: input length is %d\n", length);
        return -1;
    }

    /** Building the wavelet tree */
    gettimeofday(&start_time,NULL);
    tree = buildWaveletTree(input, length);
    gettimeofday(&end_time,NULL);
    us = (long)((1000000 * end_time.tv_sec + end_time.tv_usec) -
                       (1000000 * start_time.tv_sec + start_time.tv_usec));
    printf ("Building wavelet tree from input length %d took %ld us\n", length, us);

    free(input);

    if (!strcmp (rank_or_select, "rank")) {
        /** Calculating rank operation */
        gettimeofday(&start_time,NULL);
        Rank = rankOperation(tree, s_char, bound);
        gettimeofday(&end_time,NULL);
        us = (long)((1000000 * end_time.tv_sec + end_time.tv_usec) -
                           (1000000 * start_time.tv_sec + start_time.tv_usec));

        /** Result of the rank operation */
        printf ("Rank(%c, %d) is: %d\n", s_char, bound, Rank);
        printf ("Calculating rank operation took %ld us\n", us);
    } else if (!strcmp (rank_or_select, "select")) {
        /** Calculating select operation */
        gettimeofday(&start_time,NULL);
        Select = selectOperation(tree, s_char, bound);
        gettimeofday(&end_time,NULL);
        us = (long)((1000000 * end_time.tv_sec + end_time.tv_usec) -
                           (1000000 * start_time.tv_sec + start_time.tv_usec));

        /** Result of the rank operation */
        printf ("Select(%c, %d) is: %d\n", s_char, bound, Select);
        printf ("Calculating select operation took %ld us\n", us);
    } else {
        printf ("Second argument can be either rank or select\n");
        return -1;
    }

    //i = calculateNodeMemoryUsage (tree->rootNode);
    i = calculateNodeMemoryUsageRRR (tree->rootNode);

    printf ("Memory usage: %d\n", i);

    return 0;
}

/** Računanje memorije (bez RRR) */ 
int calculateNodeMemoryUsage (WaveletNode *node) {
    int i = 0, j, k;

    if (node == NULL) return i;

    i+= 2 * node->dictLength;
    i+= node->bitmap->length / 8 + 1;

    i+= calculateNodeMemoryUsage(node->leftChild);
    i+= calculateNodeMemoryUsage(node->rightChild);

    return i;
}

/** Računanje memorije (s RRR) */
int calculateNodeMemoryUsageRRR (WaveletNode *node) {
    int i = 0, j, k;

    if (node == NULL) return i;

    i+= 2 * node->dictLength;
    i+= 8 * node->rrr->bitmap->length / node->table->superblock_size;
    i+= node->rrr->bitmap->length / 8 + 1;
    for (j = 0; j < node->table->block_size + 1; ++j) {
        i += 1;
        for (k = 0; k < node->table->entries[j].offset_count; ++k) {
            i += 2;
        }
    }

    i+= calculateNodeMemoryUsage(node->leftChild);
    i+= calculateNodeMemoryUsage(node->rightChild);

    return i;
}
