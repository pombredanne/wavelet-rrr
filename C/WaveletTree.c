#include "WaveletTree.h"

/** Structure used for keeping class -> offsets data */
RRRTable *global_table;

int main (int argc, char* argv[]) {

    FILE *inputFile = NULL;
    char* filename, *input;
    int length = 0, block, superblock, low, high;
    char c, s_char;
    WaveletTree *tree = NULL;

    global_table = (RRRTable *)calloc (1, sizeof(RRRTable));

    if (argc != 5) {
        printf ("Wrong number of parameters: WavletTree.exe filename char low_bound high_bound expected\n");
        return -1;
    }
    /** Arguments input */
    filename = argv[1];
    s_char = argv[2][0];
    low = atoi(argv[3]);
    high = atoi(argv[4]);

    /** Checking boundaries */
    if (low >= high) {
        printf ("Wrong boundaries!\n");
        return -1;
    }

    if (low < 0) {
        printf ("Lower boundary cannot be a negative number: %d\n", low);
        return -1;
    }

    /** Opening input file */
    inputFile = fopen(filename, "r");

    if (inputFile == NULL) {
        printf ("%s doesn't exist\n", filename);
        return -1;
    }

    /** Allocating memory space for input string */
    input = (char *)calloc (MEMORY_SIZE, sizeof(char));

    /** Reading through file and skipping newline characters */
    while ((c = fgetc(inputFile)) != EOF) {
        if (c == '\n') continue;
        input[length] = c;
        length++;
    }
    length--;

    /** Checking if the upper boundary is too high */
    if (high > length) {
        printf ("Upper boundary too high: input length is %d\n", length);
        return -1;
    }

    /** Calculating block and superblock sizes based on
        input length */
    calculateBlockSizes(length, &block, &superblock);

    /** Creating the RRR table */
    buildRRRTable(block, superblock);

    //RRRTableToString();

    /** Building the wavelet tree */
    tree = buildWaveletTree(input, length);

    free(input);

    /** Result of the rank operation */
    printf ("Rank(%c, %d) - Rank (%c, %d) is: %d\n", s_char, high, s_char, low, rankOperation(tree, s_char, high) - rankOperation(tree, s_char, low));

    return 0;

}

/** This function is used for building the wavelet tree.
    It takes input string and it's length as parameters. */
WaveletTree *buildWaveletTree(char* input, int length) {
    int dictLength;

    /** Allocating memory space for wavelet tree structure */
    WaveletTree *tree = (WaveletTree *)calloc (1, sizeof (WaveletTree));

    /** Extracting the alphabet from the input and creating
        a 'dictionary' */
    Dictionary *dict = extractAlphabet (input, length, &dictLength);

    /** Calling the recursive function which creates tree nodes */
    tree->rootNode = buildWaveletNode(input, length, dict, dictLength);

    return tree;
}

/** Function which is used for creating the tree node */
WaveletNode *buildWaveletNode (char* input, int length, Dictionary *dict, int dictLength) {
    int i, firstChildLength = 0, secondChildLength = 0, rightDictLength, leftDictLength;
    char *firstChildInput, *secondChildInput, *bitmap;

    /**Allocating memory space for WaveletNode and BitMap structures */
    WaveletNode *node = (WaveletNode *)calloc (1, sizeof (WaveletNode));
    node->bitmap = (BitMap *)calloc (1, sizeof (BitMap));

    /** Storing dictionary recieved as a parameter */
    node->dict = dict;
    node->dictLength = dictLength;

    /** Bitmap is represented as the array of characters */
    bitmap = (char *)calloc (length/8 + 1, sizeof (char));

    /** If there are only 2 letters in the dictionary this node
        is a leaf */
    if (dictLength == 2) {

        /** Creating bitmap from input string based on letter value
            stred in dictionary (0 or 1) */
        for (i = 0; i < length; ++i) {
            if (getDictionaryValue(node->dict, node->dictLength, input[i])) {
                bitmap[i/8] |= 0x80 >> (i%8);
            }
        }

        /** Storing bitmap */
        node->bitmap->bm = bitmap;
        node->bitmap->length = length;

        /** Creating RRR structure from bitmap */
        node->rrr = bitmapToRRR(node->bitmap);

        //nodeToString(node);

        /** This node is a leaf */
        node->leftChild = NULL;
        node->rightChild = NULL;

    /** If node has 3 letters in a dictionary it will have
        only one child node (in this case it will always be
        right child node */
    } else if (dictLength == 3) {

        secondChildInput = (char *)calloc (MEMORY_SIZE, sizeof (char));

        /** Input string to bitmap, and also storing the letters with
            value 1 as input string for the right child node */
        for (i = 0; i < length; ++i) {
            if (getDictionaryValue(node->dict, node->dictLength, input[i])) {
                secondChildInput[secondChildLength] = input[i];
                secondChildLength++;

                bitmap[i/8] |= 0x80 >> (i%8);
            }
        }

        /** Storing the bitmap */
        node->bitmap->bm = bitmap;
        node->bitmap->length = length;

        /** Creating RRR structure from bitmap */
        node->rrr = bitmapToRRR(node->bitmap);

        //nodeToString(node);

        /** Creating dictionaries for children nodes (in this case only right child */
        Dictionary *rightDict = (Dictionary *)calloc (node->dictLength, sizeof (Dictionary));
        splitDictionary(node->dict, NULL, rightDict, node->dictLength, &leftDictLength, &rightDictLength);

        /** Recursive call only on right child */
        node->leftChild = NULL;
        node->rightChild = buildWaveletNode(secondChildInput, secondChildLength, rightDict, rightDictLength);

        free(secondChildInput);

    } else {

         firstChildInput = (char *)calloc (MEMORY_SIZE, sizeof (char));
         secondChildInput = (char *)calloc (MEMORY_SIZE, sizeof (char));

        for (i = 0; i < length; ++i) {
            if (getDictionaryValue(node->dict, node->dictLength, input[i])) {
                secondChildInput[secondChildLength] = input[i];
                secondChildLength++;

                bitmap[i/8] |= 0x80 >> (i%8);
            } else {
                firstChildInput[firstChildLength] = input[i];
                firstChildLength++;
            }
        }

        node->bitmap->bm = bitmap;
        node->bitmap->length = length;

        node->rrr = bitmapToRRR(node->bitmap);

        //nodeToString(node);

        Dictionary *rightDict = (Dictionary *)calloc (node->dictLength, sizeof (Dictionary));
        Dictionary *leftDict = (Dictionary *)calloc (node->dictLength, sizeof (Dictionary));

        splitDictionary(node->dict, leftDict, rightDict, node->dictLength, &leftDictLength, &rightDictLength);

        node->leftChild = buildWaveletNode(firstChildInput, firstChildLength, leftDict, leftDictLength);
        node->rightChild = buildWaveletNode(secondChildInput, secondChildLength, rightDict, rightDictLength);

        free(firstChildInput);
        free(secondChildInput);
    }

    return node;


}

Dictionary *extractAlphabet (char *input, int length, int *dictLength) {
    int i, j;
    bool exist;
    Dictionary *dictionary = (Dictionary *)calloc (DICTIONARY_SIZE, sizeof (Dictionary));

    *dictLength = 0;
    for (i = 0; i < length; ++i) {
        exist = false;
        for (j = 0; j < *dictLength; ++j) {
            if (input[i] == dictionary[j].character) {
                exist = true;
                break;
            }
        }

        if (!exist) {
            dictionary[*dictLength].character = input[i];
            (*dictLength)++;
        }
    }

    for (i = 0; i < *dictLength; ++i) {
        if (i < *dictLength/2) {
            dictionary[i].value = 0;
        } else {
            dictionary[i].value = 1;
        }
    }

    return dictionary;
}

void splitDictionary (Dictionary *dict, Dictionary *leftDict, Dictionary *rightDict,
                      int dictLength, int *leftLength, int *rightLength) {

    int i;
    *leftLength = 0;
    *rightLength = 0;

    for (i = 0; i < dictLength; ++i) {
        if (dict[i].value == 0 && leftDict != NULL) {
            //printf ("Left length: %d\n", *leftLength);
            leftDict[*leftLength].character = dict[i].character;
            *leftLength += 1;
        } else if (dict[i].value == 1 && rightDict != NULL) {
            //printf ("Right length: %d\n", *rightLength);
            rightDict[*rightLength].character = dict[i].character;
            *rightLength += 1;
        }
    }

     for (i = 0; i < *leftLength; ++i) {
        if (i < *leftLength/2) {
            leftDict[i].value = 0;
        } else {
            leftDict[i].value = 1;
        }
    }

    for (i = 0; i < *rightLength; ++i) {
        if (i < *rightLength/2) {
            rightDict[i].value = 0;
        } else {
            rightDict[i].value = 1;
        }
    }
}

bool getDictionaryValue (Dictionary *dict, int dictLength, char c) {
    int i;

    for (i = 0; i < dictLength; ++i) {
        if (dict[i].character == c) {
            return dict[i].value;
        }
    }

    return 0;
}

int rankOperation (WaveletTree *tree, char c, int i) {

    int Rank = i;
    WaveletNode *current = tree->rootNode;

    while (current != NULL) {
        //Rank = popcount (current->bitmap->bm, getDictionaryValue(current->dict,current->dictLength,c), Rank);
        Rank = popcountRRR (current->rrr, getDictionaryValue(current->dict,current->dictLength,c), Rank);
        if (getDictionaryValue(current->dict, current->dictLength, c)) {
            current = current->rightChild;
        } else {
            current = current->leftChild;
        }
    }

    return Rank;
}

int popcountRRR (RRRStruct *rrr, bool c, int i) {
    int j, k, Rank, index, class_index, blocks_rem, offset = 0, block, offset_rem;

    index = i/global_table->superblock_size;
    Rank = rrr->superblock_sum[index];

    j = rrr->superblock_offset[index];
    blocks_rem = i - (global_table->superblock_size * index);
    blocks_rem /= global_table->block_size;

    offset_rem = i - (global_table->superblock_size * index);
    offset_rem -= blocks_rem * global_table->block_size;

    //printf ("\nLength: %d, Superblocks: %d, Blocks: %d, Remaining offset: %d\n", i, index, blocks_rem, offset_rem);
    //printf ("Superblock sum: %d\n", Rank);
    while (blocks_rem > 0) {
        class_index = 0;
        for (k = j; k < j+global_table->class_bm; ++k) {
            class_index = class_index << 1;
            class_index |= (rrr->bitmap->bm[k/8] >> (7 - (k % 8)) & 0x01);
        }
        Rank += class_index;
        j += global_table->class_bm + global_table->entries[class_index].offset_bm;
        blocks_rem--;
    }

    class_index = 0;
    //printf ("Before offset sum: %d\n", Rank);

    for (k = j; k < j+global_table->class_bm; ++k) {
        class_index = class_index << 1;
        class_index |= (rrr->bitmap->bm[k/8] >> (7 - (k % 8)) & 0x01);
    }

    for (k = j + global_table->class_bm; k < j+ global_table->class_bm +global_table->entries[class_index].offset_bm; ++k) {
        offset = offset << 1;
        offset |= ((rrr->bitmap->bm[k/8] >> (7 - (k % 8))) & 0x01);
    }

    block = global_table->entries[class_index].offsets[offset];
   // printf ("Block : 0x%04x\n", block);
    block = (block >> (global_table->block_size - offset_rem));
    //printf ("Block : 0x%04x\n", block);
    Rank += popcountInt(block, true, offset_rem);

    if (c) {
        return Rank;
    } else {
        return i - Rank;
    }
}

int popcount(char *bitmap, bool c, int i) {
    int j, Rank = 0;

    for (j = 0; j < i; ++j) {
        if ((bitmap[j/8] & (0x80 >> (j%8))) != 0x00) {
            Rank++;
        }
    }

    if (c) {
        return Rank;
    } else {
        return i - Rank;
    }
}

int popcountInt(int bitmap, bool c, int i) {
    int j, Rank = 0;

    for (j = 0; j < i; ++j) {
        if ((bitmap >> j) & 0x01) {
            Rank++;
        }
    }

    if (c) {
        return Rank;
    } else {
        return i - Rank;
    }
}

void nodeToString (WaveletNode *node) {
    int i;

    printf ("Bitmap: ");
    for (i = 0; i < node->bitmap->length/8 + 1; ++i) {
        printf ("0x%02x ", node->bitmap->bm[i] & 0xff);
    }

    printf ("\n");

    printf ("RRR bitmap: ");
    for (i = 0; i < node->rrr->bitmap->length/8 + 1; ++i) {
        printf ("0x%02x ", node->rrr->bitmap->bm[i] & 0xff);
    }

    printf ("\n");

    for (i = 0; i < node->dictLength; ++i) {
        printf ("Dict %c : %d\n", node->dict[i].character, node->dict[i].value);
    }

    printf ("\n");
}

void calculateBlockSizes (int length, int *block, int *superblock) {
    int log2 = 0, length_pom = length;

    while (length_pom != 0) {
        length_pom = (length_pom >> 1);
        log2++;
    }

    *block = log2 / 2;
    *superblock = *block * log2;
}

void buildRRRTable (int block_size, int superblock_size) {
    int length = 1, i, index, j, class_bm = 0;

    global_table->entries = (RRRTableEntry *) calloc (block_size + 1, sizeof (RRRTableEntry));

    for (i = 0; i < block_size + 1; ++i) {
        global_table->entries[i].offset_count = 0;
        global_table->entries[i].RRR_class = i;
        global_table->entries[i].offsets = (int *)calloc(1, sizeof(int));
    }

    global_table->block_size = block_size;
    global_table->superblock_size = superblock_size;

    j = global_table->block_size;
    do {
        class_bm++;
        j = j >> 1;
    } while (j != 0);

    global_table->class_bm = class_bm;
    length = (length << (block_size));

    for (i = 0; i < length; ++i) {
        index = popcountInt(i, true, block_size);
        //printf ("I: %d, Class: %d, Offset_count: %d\n", i, index, global_table->entries[index].offset_count);
        global_table->entries[index].offsets[global_table->entries[index].offset_count] = i;
        global_table->entries[index].offset_count+=1;
        global_table->entries[index].offsets = realloc (global_table->entries[index].offsets, (global_table->entries[index].offset_count + 1)*sizeof(int));
    }

    for (i = 0; i < block_size + 1; ++i) {
        int j = global_table->entries[i].offset_count;
        int offset_bm = 0;
        do {
            offset_bm++;
            j = (j >> 1);
        } while (j != 0);
        global_table->entries[i].offset_bm = offset_bm;

    }
}

RRRStruct *bitmapToRRR (BitMap *bm) {
    int length = 0, i, j, block, index, offset = 0, superblock_sum = 0;

    RRRStruct *rrr = (RRRStruct *)calloc (1, sizeof(RRRStruct));
    rrr->bitmap = (BitMap *) calloc (1, sizeof (BitMap));

    char *bitmap = (char *)calloc (MEMORY_SIZE, sizeof (char));
    rrr->superblock_sum = (int *) calloc (bm->length / global_table->superblock_size + 1, sizeof(int));
    rrr->superblock_offset = (int *) calloc (bm->length / global_table->superblock_size + 1, sizeof(int));

    for (i = 0; i < bm->length; i+=global_table->block_size) {
        block = 0;

        if (i % global_table->superblock_size == 0) {
            rrr->superblock_sum[i/global_table->superblock_size] = superblock_sum;
            rrr->superblock_offset[i/global_table->superblock_size] = length;
        }

        for (j = i; j < i + global_table->block_size; ++j) {
            block = block << 1;
            block |= ((bm->bm[j/8] >> (7-(j%8))) & 0x01);
        }

        index = popcountInt(block, true, global_table->block_size);
        superblock_sum += index;

        for (j = 0; j < global_table->entries[index].offset_count; ++j) {
            if (global_table->entries[index].offsets[j] == block) {
                offset = j;
                break;
            }
        }

        for (j = 0; j < global_table->class_bm; ++j) {
            bitmap[(length+j)/8] |= ((index >> (global_table->class_bm - 1 - j) & 0x01) << (7-((length+j)%8)));
        }

        length += global_table->class_bm;

        for (j = 0; j < global_table->entries[index].offset_bm; ++j) {
            bitmap[(length+j)/8] |= ((offset >> (global_table->entries[index].offset_bm - 1 - j) & 0x01) << (7-((length+j)%8)));
        }

        length += global_table->entries[index].offset_bm;
    }

    rrr->bitmap->bm = bitmap;
    rrr->bitmap->length = length;

    return rrr;

}

void RRRTableToString (){
    int i, j;

    printf ("Block size: %d\n", global_table->block_size);
    printf ("Superblock size: %d\n", global_table->superblock_size);
    printf ("Class bm: %d\n", global_table->class_bm);

    for (i = 0; i < global_table->block_size + 1; ++i) {
        printf ("\n");
        printf ("Class: %d\n", global_table->entries[i].RRR_class);
        printf ("Offset_bm: %d\n", global_table->entries[i].offset_bm);
        printf ("Offsets: ");
        for (j = 0; j < global_table->entries[i].offset_count; ++j) {
            printf ("0x%04x, ", global_table->entries[i].offsets[j]);
        }
        printf ("\n");
    }

}
