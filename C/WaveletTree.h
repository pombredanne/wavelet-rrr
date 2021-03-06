#ifndef WAVELETTREE_H
#define WAVELETTREE_H

#include <stdio.h>
#include <stdlib.h>
#include "RRR.h"
#include "Utils.h"

/** Structure that represents node in WaveletTree. It contains
    RRR structure (bitmap), a dictionary structure (for encoding
    characters into bits and a pointers to left and right child
    nodes and parent node*/
struct WaveletNode {
    RRRStruct *rrr;
    RRRTable *table;
    BitMap *bitmap;
    Dictionary* dict;
    int dictLength;
    struct WaveletNode *leftChild;
    struct WaveletNode *rightChild;
    struct WaveletNode *parent;
};

typedef struct WaveletNode WaveletNode;

/** Structure which represents Wavelet tree. It contains
    WaveletNode structure as a root node. */
typedef struct  {
    WaveletNode *rootNode;
} WaveletTree;

/** This function is used for building the wavelet tree.
    It takes input string and it's length as parameters. */
WaveletTree *buildWaveletTree (char *input, int length);

/** Function which is used for creating the tree node */
WaveletNode *buildWaveletNode (char *input, int length, Dictionary *dict, int dictLenght);

/** Function which represents rank operation. Rank (c, i) means
    number of occurances of character c in the first i characters
    of input string */
int rankOperation (WaveletTree *tree, char c, int i);

/** Popcount used for counting 1's in bitmap from 0 to i (bitmap is char array) */
int popcount (char *bitmap, bool c, int i);

/** Popcount used for counting 1's in bitmap from 0 to i (bitmap is int)*/
int popcountInt (int bitmap, bool c, int i);

/** Select on bitmap */
int selectOnBitmap(char *bitmap, bool c, int i, int length);

void nodeToString(WaveletNode *node);

#endif // WAVELETTREE_H
