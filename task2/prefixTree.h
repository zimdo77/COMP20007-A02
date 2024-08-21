/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
     Header for module which contains Prefix Trie 
        data structures and functions.
*/

#ifndef PREFIXTREESTRUCT
#define PREFIXTREESTRUCT
#define CHILD_COUNT (1 << 8)
#define UNMARKED (0)
#define MARKED (1)
#define NULL_TERMINATOR '\0'
#define NULL_TERMINATOR_INDEX (0)

struct prefixTree;

/* A prefix trie node */
struct prefixTree {
    /* An array of pointers to CHILD_COUNT possible children */
    struct prefixTree *children[CHILD_COUNT];
    /* Inserted character */
    char character;
    /* Initially unmarked. Mark if a word is found in the board */
    int wordFound;
    /* For part B. Initially unmarked. Mark if there is a word
        along the node's path which can be made in the boggle
        board. */
    int isFollowLetter;
};

/* Creates a new node inside a prefix trie. Returns the resulting trie. */
struct prefixTree *newPrefixTree(char inputCharacter);

/* Inserts a word into a prefix trie. Returns the resulting trie */
struct prefixTree *addWordToTree(struct prefixTree *pt, char *word);

/* Returns 1 if a given word was found in the tree. Returns 0 otherwise. */
int isInTree(struct prefixTree *tree, char *word);

/* Returns 1 if a given character is a child node of a given trie node. 
    Returns 0 otherwise. */
int isChildNode(struct prefixTree *tree, char character);

/* Given a partial string, return the node in a prefix trie where the last character
    of that partial string should be. If the partial string does not exist in the trie,
    return NULL. */
struct prefixTree *goToNode(struct prefixTree *tree, char *partialString);

/* Given a trie node, recursively find if there is a marked word
    down all paths from that node. */
int hasMarkedWord(struct prefixTree *tree);

/* Frees memory allocated to a prefix trie. */
void freePrefixTree(struct prefixTree *tree);

#endif
