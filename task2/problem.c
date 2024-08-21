/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Implementation for module which contains  
        Problem 2-related data structures and 
        functions.
    
    Sample solution implemented by Grady Fitzpatrick
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "problem.h"
#include "problemStruct.c"
#include "solutionStruct.c"
#include "prefixTree.h"

/* Number of words to allocate space for initially. */
#define INITIALWORDSALLOCATION 64

/* Denotes that the dimension has not yet been set. */
#define DIMENSION_UNSET (-1)

/* Maximum number of neighbours to a grid square on any board */
#define MAX_NEIGHBOURS 8

/* Delimiter to signify the end of an adjacency list */
#define ADJ_LIST_END (-1)

/* To mark visited nodes when searching the board */
#define VISITED 1
#define UNVISITED 0

/* For PART D: to check for duplicate characters in a word */
#define SEEN 1
#define UNSEEN 0

struct problem;
struct solution;

/*************************/
/* FUNCTION DECLARATIONS */
/*************************/

struct solution *newSolution(struct problem *problem);
struct prefixTree *readDictionaryIntoTree(struct problem *p, struct prefixTree *tree);
int **createAdjacencyList(struct problem *p);
void freeAdjacencyList(int **adjacencyList, int listSize);
void exploreBoard(int boardIndex, int **adjacencyList, int *visited, struct problem *p, struct solution *s, struct prefixTree *tree);
void findMarkedWords(struct prefixTree *tree, struct solution *s, char* tempWord, int *wordsAdded, int level);
int uniqueCharacters(char *word);

/************************/
/* FUNCTION DEFINITIONS */
/************************/

/* 
    Reads the given dict file into a list of words 
    and the given board file into a nxn board.
*/
struct problem *readProblemA(FILE *dictFile, FILE *boardFile){
    struct problem *p = (struct problem *) malloc(sizeof(struct problem));
    assert(p);

    /* Part B onwards so set as empty. */
    p->partialString = NULL;

    int wordCount = 0;
    int wordAllocated = 0;
    char *dictText = NULL;
    char **words = NULL;

    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&dictText, &allocated, '\0', dictFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading dictionary file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    char *boardText = NULL;
    /* Reset allocated marker. */
    allocated = 0;
    success = getdelim(&boardText, &allocated, '\0', boardFile);

    if(success == -1){
        /* Encountered an error. */
        perror("Encountered error reading board file");
        exit(EXIT_FAILURE);
    } else {
        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int dictTextLength = strlen(dictText);

    /* Count words present. */
    while(progress < dictTextLength){
        char *word = NULL;
        int nextProgress;
        /* Read each value into the dictionary. */
        if(progress == 0){
            /* First line. */
            int wordNumberGuess;
            assert(sscanf(dictText + progress, "%d %n", &wordNumberGuess, &nextProgress) == 1);
            /* Will fail if integer missing from the start of the words. */
            assert(nextProgress > 0);
            if(wordNumberGuess > 0){
                wordAllocated = wordNumberGuess;
                words = (char **) malloc(sizeof(char *) * wordAllocated);
                assert(words);
            }
            progress += nextProgress;
            continue;
        } else {
            assert(sscanf(dictText + progress, "%m[^\n] %n", &word, &nextProgress) == 1);
            assert(nextProgress > 0);
            progress += nextProgress;
        }

        /* Check if more space is needed to store the word. */
        if(wordAllocated <= 0){
            words = (char **) malloc(sizeof(char *) * INITIALWORDSALLOCATION);
            assert(words);
            wordAllocated = INITIALWORDSALLOCATION;
        } else if(wordCount >= wordAllocated){
            words = (char **) realloc(words, sizeof(char *) * 
                wordAllocated * 2);
            assert(words);
            wordAllocated = wordAllocated * 2;
            /* Something has gone wrong if there's not sufficient word 
                space for another word. */
            assert(wordAllocated > wordCount);
        }

        words[wordCount] = word;
        wordCount++;
    }
    
    /* Done with dictText */
    if(dictText){
        free(dictText);
    }
    
    /* Now read in board */
    progress = 0;
    int dimension = 0;
    int boardTextLength = strlen(boardText);
    /* Count dimension with first line */
    while(progress < boardTextLength){
        /* Count how many non-space characters appear in line. */
        if(boardText[progress] == '\n' || boardText[progress] == '\0'){
            /* Reached end of line. */
            break;
        }
        if(isalpha(boardText[progress])){
            dimension++;
        }
        progress++;
    }

    assert(dimension > 0);

    /* Check each line has the correct dimension. */
    for(int i = 1; i < dimension; i++){
        int rowDim = 0;
        if(boardText[progress] == '\n'){
            progress++;
        }
        while(progress < boardTextLength){
            /* Count how many non-space characters appear in line. */
            if(boardText[progress] == '\n' || boardText[progress] == '\0'){
                /* Reached end of line. */
                break;
            }
            if(isalpha(boardText[progress])){
                rowDim++;
            }
            progress++;
        }
        if(rowDim != dimension){
            fprintf(stderr, "Row #%d had %d letters, different to Row #1's %d letters.\n", i + 1, rowDim, dimension);
            assert(rowDim == dimension);
        }
    }

    /* Define board. */
    char *boardFlat = (char *) malloc(sizeof(char) * dimension * dimension);
    assert(boardFlat);
    
    /* Reset progress. */
    progress = 0;
    for(int i = 0; i < dimension; i++){
        for(int j = 0; j < dimension; j++){
            int nextProgress;
            assert(sscanf(boardText + progress, "%c %n", &boardFlat[i * dimension + j], &nextProgress) == 1);
            progress += nextProgress;
        }
    }

    char **board = (char **) malloc(sizeof(char **) * dimension);
    assert(board);
    for(int i = 0; i < dimension; i++){
        board[i] = &boardFlat[i * dimension];
    }

    // fprintf(stderr, "\n");

    /* The number of words in the text. */
    p->wordCount = wordCount;
    /* The list of words in the dictionary. */
    p->words = words;

    /* The dimension of the board (number of rows) */
    p->dimension = dimension;

    /* The board, represented both as a 1-D list and a 2-D list */
    p->boardFlat = boardFlat;
    p->board = board;

    /* For Part B only. */
    p->partialString = NULL;

    p->part = PART_A;

    return p;
}

struct problem *readProblemB(FILE *dictFile, FILE *boardFile, 
    FILE *partialStringFile){
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(dictFile, boardFile);

    char *partialString = NULL;

    /* Part B has a string that is partially given - we assume it follows 
        word conventions for the %s specifier. */
    assert(fscanf(partialStringFile, "%ms", &partialString) == 1);
    
    p->part = PART_B;
    p->partialString = partialString;

    return p;
}

struct problem *readProblemD(FILE *dictFile, FILE *boardFile){
    /* Interpretation of inputs is same as Part A. */
    struct problem *p = readProblemA(dictFile, boardFile);
    
    p->part = PART_D;
    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution, 
    FILE *outfileName){
    assert(solution);
    switch(problem->part){
        case PART_A:
        case PART_D:
            assert(solution->foundWordCount == 0 || solution->words);
            for(int i = 0; i < solution->foundWordCount; i++){
                fprintf(outfileName, "%s\n", solution->words[i]);
            }
            break;
        case PART_B:
            assert(solution->foundLetterCount == 0 || solution->followLetters);
            for(int i = 0; i < solution->foundLetterCount; i++){
                if(isalpha(solution->followLetters[i])){
                    fprintf(outfileName, "%c\n", solution->followLetters[i]);
                } else {
                    fprintf(outfileName, " \n");
                }
            }
            break;
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem){
    if(solution){
        if(solution->followLetters){
            free(solution->followLetters);
        }
        if(solution->words){
            free(solution->words);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem){
    if(problem){
        if(problem->words){
            for(int i = 0; i < problem->wordCount; i++){
                if(problem->words[i]){
                    free(problem->words[i]);
                }
            }
            free(problem->words);
        }
        if(problem->board){
            free(problem->board);
        }
        if(problem->boardFlat){
            free(problem->boardFlat);
        }
        if(problem->partialString){
            free(problem->partialString);
        }
        free(problem);
    }
}

/* 
    Sets up a solution for the given problem
*/
struct solution *newSolution(struct problem *problem){
    struct solution *s = (struct solution *) malloc(sizeof(struct solution));
    assert(s);
    s->foundWordCount = 0;
    s->words = NULL;
    s->foundLetterCount = 0;
    s->followLetters = NULL;
    
    return s;
}

/* 
    Creates a prefix trie to store all the dictionary words
    given in the problem.
*/
struct prefixTree *readDictionaryIntoTree(struct problem *p, struct prefixTree *tree) {
    if (p->part == PART_A || p->part == PART_B) {
        for (int i = 0; i < p->wordCount; i++) {
            tree = addWordToTree(tree, p->words[i]);
        }
        return tree;
    } else {
    /* PART D */
        for (int i = 0; i < p->wordCount; i++) {
            /* Check if characters are unique before adding to tree */
            if (uniqueCharacters(p->words[i])) {
                tree = addWordToTree(tree, p->words[i]);
            }
        }
        return tree;
    }
}   

/* 
    Creates an adjacency list for the boggle graph where each node is
    represented by the element's index in the 1D boggle board.
*/
int **createAdjacencyList(struct problem *p) {
    int dimension = p->dimension;
    int boardSize = dimension * dimension;

    /* Allocate memory for the adjacency list */
    int **adjacencyList = (int **) malloc(boardSize * sizeof(int *));
    assert(adjacencyList);

    /* For each matrix entry, find its adjacent entries */
    for (int i = 0; i < boardSize; i++) {
        /* Allocate memory for each node on the boggle graph. Each node has a maximum
            of MAX_NEIGHBOUR possible neighbours, and +1 for delimiter */
        adjacencyList[i] = (int *) malloc((MAX_NEIGHBOURS + 1) * sizeof(int));
        assert(adjacencyList[i]);

        int rowIndex = i / dimension;
        int columnIndex = i % dimension;
        int count = 0;    

        /* Check all possible neighbours. If it exists, add to adjacency list */
        if (columnIndex > 0) {
        /* Left entry exists */
            adjacencyList[i][count++] = i - 1;            
        }
        if (columnIndex < dimension - 1) {
        /* Right entry exists */
            adjacencyList[i][count++] = i + 1;
        }
        if (rowIndex > 0) {
        /* Top entry exists */
            adjacencyList[i][count++] = i - dimension;   
        }
        if (rowIndex < dimension - 1) {
        /* Bottom entry exists */
            adjacencyList[i][count++] = i + dimension;
        }
        if (rowIndex > 0 && columnIndex > 0) {
        /* Top left entry exists */
            adjacencyList[i][count++] = i - dimension - 1;
        }
        if (rowIndex > 0 && columnIndex < dimension - 1) {
        /* Top right entry exists */
            adjacencyList[i][count++] = i - dimension + 1;
        }
        if (rowIndex < dimension - 1 && columnIndex > 0) {
        /* Bottom left entry exists */
            adjacencyList[i][count++] = i + dimension - 1;
        }
        if (rowIndex < dimension - 1 && columnIndex < dimension - 1) {
        /* Bottom right entry exists */
            adjacencyList[i][count++] = i + dimension + 1;
        }
        /* Add delimiter to signify end of list */
        adjacencyList[i][count] = ADJ_LIST_END;
    }

    return adjacencyList;
}

/*
    Frees memory allocated to an adjacency list
*/
void freeAdjacencyList(int **adjacencyList, int listSize) {
    assert(adjacencyList);
    
    for (int i = 0; i < listSize; i++) {
        free(adjacencyList[i]);
        adjacencyList[i] = NULL;
    }

    free(adjacencyList);
    adjacencyList = NULL;
}

/* 
    Recursively finds all possible words which can be reached from
    an element on the board, and mark words that are found. The 
    prefix trie limits which elements can be explored next. 
*/
void exploreBoard(int boardIndex, int **adjacencyList, int *visited, struct problem *p, struct solution *s, struct prefixTree *tree) {
    /* Mark board element as VISITED */
    visited[boardIndex] = VISITED;

    /* If the trie has a null terminator child AND it has not been marked before, a new word is found */
    if (tree->children[NULL_TERMINATOR_INDEX] && tree->children[NULL_TERMINATOR_INDEX]->wordFound == UNMARKED) {
        tree->children[NULL_TERMINATOR_INDEX]->wordFound = MARKED;
        (s->foundWordCount)++;
    }
    
    /* For each adjacent board element... */
    int i = 0;
    while (adjacencyList[boardIndex][i] != ADJ_LIST_END) {
        /* Find its corresponding character */
        char boardCharacter = tolower(p->boardFlat[adjacencyList[boardIndex][i]]);
        /* If the character is in the prefix trie AND the element has not been visited,
            recursively explore that element. */
        if ((isChildNode(tree, boardCharacter)) && (visited[adjacencyList[boardIndex][i]] == UNVISITED)) {
            unsigned char asciiIndex = (unsigned char) boardCharacter;
            exploreBoard(adjacencyList[boardIndex][i], adjacencyList, visited, p, s, tree->children[asciiIndex]);
        }
        i++;
    }

    /* Backtrack */
    visited[boardIndex] = UNVISITED;
}

/*
    Recursively finds all the marked words within a trie, and adds
    to the list of words (in the solution struct).
*/
void findMarkedWords(struct prefixTree *tree, struct solution *s, char* tempWord, int *wordsAdded, int level) {
    if(!tree) {
        return;
    }

    /* If the null terminator is found AND the word is marked, append the word to the solution array */
    if (tree->character == NULL_TERMINATOR && tree->wordFound == MARKED) {
        tempWord[level] = NULL_TERMINATOR;
        s->words[*wordsAdded] = strdup(tempWord);
        (*wordsAdded)++;
    }

    /* Recursively traverse */ 
    for (int i = 0; i < CHILD_COUNT; i++) {
        if (tree->children[i] != NULL) {
            tempWord[level] = i;
            findMarkedWords(tree->children[i], s, tempWord, wordsAdded, level + 1); 
        }
    } 
}

/*
    Returns 1 if all characters in a word are unique. Returns 0 otherwise.
*/
int uniqueCharacters(char *word) {
    /* Array to track seen characters */
    int characterSeen[CHILD_COUNT] = {UNSEEN};

    /* Flag characters as seen while iterating through each of them. If a
        duplicate is found, return false. */
    while (*word) {
        unsigned char asciiIndex = (unsigned char) *word;
        if (characterSeen[asciiIndex] == SEEN) {
            return 0;
        } else {
            characterSeen[asciiIndex] = SEEN;
        }
        word++;
    }

    /* No duplicates are found */
    return 1;
}

/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p){
    struct solution *s = newSolution(p);
    /* Fill in: Part A */
    int i;
    int dimension = p->dimension;
    int boardSize = dimension * dimension;

    /* Create a prefix trie for the words in the dictionary */
    struct prefixTree *tree = NULL;
    tree = readDictionaryIntoTree(p, tree);

    /* Make an adjacency list for the boggle graph */
    int **adjacencyList = createAdjacencyList(p);
    
    /* Array to store visited board elements to make sure an element can only be visited once */
    int *visited = (int *) calloc(boardSize, sizeof(int));
    assert(visited);

    /* For each element on the board... */
    for (i = 0; i < boardSize; i++) {
        /* Get its character */
        char boardCharacter = tolower(p->boardFlat[i]);
        /* If the character is in the prefix trie, then search all possible words 
            that can be made starting from that element. */
        if (isChildNode(tree, boardCharacter)) {
            unsigned char asciiIndex = (unsigned char) boardCharacter;
            exploreBoard(i, adjacencyList, visited, p, s, tree->children[asciiIndex]);
        }
    }

    /* Traverse prefix trie, find marked words, and add to the list of words that can be made */
    /* First, allocate memory to the list words in the solution struct */
    s->words = (char **) malloc(s->foundWordCount * sizeof(char *));
    assert(s->words);
    /* Create buffer string to form words during traversal */
    char *tempWord = (char *) malloc((boardSize + 1) * sizeof(char));
    assert(tempWord);
    /* Keep track of the number of words added */
    int wordsAdded = 0;
    /* Recursively traverse and find the marked words */
    findMarkedWords(tree, s, tempWord, &wordsAdded, 0);


    /* Free allocated memory */
    freePrefixTree(tree);
    freeAdjacencyList(adjacencyList, boardSize);

    free(visited);
    visited = NULL;

    free(tempWord);
    tempWord = NULL;

    return s;
}

struct solution *solveProblemB(struct problem *p){
    struct solution *s = newSolution(p);
    /* Fill in: Part B */

    ////////////////////////////////
    /* FIRST PART: SAME AS PART A */
    ////////////////////////////////

    int i;
    int dimension = p->dimension;
    int boardSize = dimension * dimension;

    /* Create a prefix trie for the words in the dictionary */
    struct prefixTree *tree = NULL;
    tree = readDictionaryIntoTree(p, tree);

    /* Make an adjacency list for the boggle graph */
    int **adjacencyList = createAdjacencyList(p);
    
    /* Array to store visited board elements to make sure an element can only be visited once */
    int *visited = (int *) calloc(boardSize, sizeof(int));
    assert(visited);

    /* For each element on the board... */
    for (i = 0; i < boardSize; i++) {
        /* Get its character */
        char boardCharacter = tolower(p->boardFlat[i]);
        /* If the character is in the prefix trie, then search all possible words 
            that can be made starting from that element. */
        if (isChildNode(tree, boardCharacter)) {
            unsigned char asciiIndex = (unsigned char) boardCharacter;
            exploreBoard(i, adjacencyList, visited, p, s, tree->children[asciiIndex]);
        }
    }

    /* Traverse prefix trie, find marked words, and add to the list of words that can be made */
    /* First, allocate memory to the list words in the solution struct */
    s->words = (char **) malloc(s->foundWordCount * sizeof(char *));
    assert(s->words);
    /* Create buffer string to form words during traversal */
    char *tempWord = (char *) malloc((boardSize + 1) * sizeof(char));
    assert(tempWord);
    /* Keep track of the number of words added */
    int wordsAdded = 0;
    /* Recursively traverse and find the marked words */
    findMarkedWords(tree, s, tempWord, &wordsAdded, 0);

    ///////////////////////////////////////
    /* SECOND PART: BUILD ON FROM PART A */
    ///////////////////////////////////////

    /* Go to the prefix trie node of where the last character of the partial string should be */
    struct prefixTree *startNode = goToNode(tree, p->partialString);

    /* If partial string exists in the trie */
    if (startNode) {
        /* Find the number of follow letters */
        for (i = 0; i < CHILD_COUNT; i++) {
            if (hasMarkedWord(startNode->children[i])) {
                startNode->children[i]->isFollowLetter = MARKED;
                s->foundLetterCount++;
            }
        }
        /* Collect the follow letters and append to the solution list */
        s->followLetters = (char *) malloc (s->foundLetterCount * sizeof(char));
        int lettersAdded = 0; 
        for (i = 0; i < CHILD_COUNT; i++) {
            if (startNode->children[i] && startNode->children[i]->isFollowLetter == MARKED) {
                s->followLetters[lettersAdded] = startNode->children[i]->character;
                lettersAdded++;
            }
        }
    }

    /* Free allocated memory */
    freePrefixTree(tree);
    freeAdjacencyList(adjacencyList, boardSize);

    free(visited);
    visited = NULL;

    free(tempWord);
    tempWord = NULL;


    return s;
}

struct solution *solveProblemD(struct problem *p){
        struct solution *s = newSolution(p);
    /* Fill in: Part D */
    int i;
    int dimension = p->dimension;
    int boardSize = dimension * dimension;

    /* Create a prefix trie for the words in the dictionary */
    struct prefixTree *tree = NULL;
    tree = readDictionaryIntoTree(p, tree);

    /* Make an adjacency list for the boggle graph */
    int **adjacencyList = createAdjacencyList(p);
    
    /* Array to store visited board elements to make sure an element can only be visited once */
    int *visited = (int *) calloc(boardSize, sizeof(int));
    assert(visited);

    /* For each element on the board... */
    for (i = 0; i < boardSize; i++) {
        /* Get its character */
        char boardCharacter = tolower(p->boardFlat[i]);
        /* If the character is in the prefix trie, then search all possible words 
            that can be made starting from that element. */
        if (isChildNode(tree, boardCharacter)) {
            unsigned char asciiIndex = (unsigned char) boardCharacter;
            exploreBoard(i, adjacencyList, visited, p, s, tree->children[asciiIndex]);
        }
    }

    /* Traverse prefix trie, find marked words, and add to the list of words that can be made */
    /* First, allocate memory to the list words in the solution struct */
    s->words = (char **) malloc(s->foundWordCount * sizeof(char *));
    assert(s->words);
    /* Create buffer string to form words during traversal */
    char *tempWord = (char *) malloc((boardSize + 1) * sizeof(char));
    assert(tempWord);
    /* Keep track of the number of words added */
    int wordsAdded = 0;
    /* Recursively traverse and find the marked words */
    findMarkedWords(tree, s, tempWord, &wordsAdded, 0);


    /* Free allocated memory */
    freePrefixTree(tree);
    freeAdjacencyList(adjacencyList, boardSize);

    free(visited);
    visited = NULL;

    free(tempWord);
    tempWord = NULL;


    return s;
}

