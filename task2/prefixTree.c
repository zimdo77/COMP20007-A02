/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
     Implementation for module which contains Prefix Trie 
        data structures and functions.
*/
#include "prefixTree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
//#include <error.h>
#include <string.h>

/*
    Creates a new node inside a prefix trie. Returns the resulting trie.
*/
struct prefixTree *newPrefixTree(char inputCharacter) {
    /* Allocate memory to a new trie node */
    struct prefixTree *tree = (struct prefixTree *) malloc(sizeof(struct prefixTree));
    assert(tree);

    /* Store character */
    tree->character = inputCharacter;

    /* Initialise all of the node's children to NULL */
    for (int i = 0; i < CHILD_COUNT; i++) {
        tree->children[i] = NULL;
    }

    /* Initialise node as unmarked */
    tree->wordFound = UNMARKED;

    /* For part B */
    tree->isFollowLetter = UNMARKED;

    return tree;
}

/*
    Inserts a word into a prefix trie. Returns the resulting trie.
*/
struct prefixTree *addWordToTree(struct prefixTree *tree, char *word) {
    /* Create a new tree if adding the first element */
    if (!tree) {
        tree = newPrefixTree(NULL_TERMINATOR);
        assert(tree);
    }

    /* Create a trie node pointer to traverse through trie */
    struct prefixTree *currentNode = tree;

    /* Insert word into trie */
    while (*word) {
    /* For every character in the word, until the character reaches the 
        null terminator... */
        unsigned char asciiIndex = (unsigned char) *word;
        /* If there is no child node for a character, create a new node for it */
        if (!currentNode->children[asciiIndex]) {
            currentNode->children[asciiIndex] = newPrefixTree(*word);
        }
        /* Continue traversing trie */
        currentNode = currentNode->children[asciiIndex];
        word++;
    }

    /* Add a null terminator child to signify that the word has ended */
    if (!currentNode->children[NULL_TERMINATOR_INDEX]) {
        currentNode->children[NULL_TERMINATOR_INDEX] = newPrefixTree(NULL_TERMINATOR);
    } 
    
    return tree;
}

/*
    Returns 1 if a given word was found in the tree. Returns 0 otherwise.
*/
int isInTree(struct prefixTree *tree, char *word) {
    assert(tree);

    /* Create a pointer to a node to traverse through trie */
    struct prefixTree *currentNode = tree;
    
    /* Find if word is in tree */
    while (*word) {
    /* Until the character reaches the null terminator... */
        unsigned char asciiIndex = (unsigned char) *word;
        /* If there is no child node for a character, the word 
            does not exist in the trie */
        if (!currentNode->children[asciiIndex]) {
            return 0;
        }
        /* Continute traversing trie */
        currentNode = currentNode->children[asciiIndex];
        word++;
    }

    /* If the last character is the null terminator, then the word exists in the trie */
    return (currentNode->children[NULL_TERMINATOR_INDEX] != NULL);
}

/*
    Returns 1 if a given character is a child node of a given trie node. 
    Returns 0 otherwise.
*/
int isChildNode(struct prefixTree *tree, char character) {
    assert(tree);
    unsigned char asciiIndex = (unsigned char) character;

    /* The character is a child node of the given tree node if the pointer corresponding
        to that character is not null */
    return (tree->children[asciiIndex] != NULL);
}

/*
    Given a partial string, return the node in a prefix trie where the last character
    of that partial string should be. If the partial string does not exist in the trie,
    return NULL. 
*/
struct prefixTree *goToNode(struct prefixTree *tree, char *partialString) {
    assert(tree);

    /* Create a pointer to a node to traverse through trie */
    struct prefixTree *currentNode = tree;

    while (*partialString) {
    /* For every character in the partial string, until the character 
        reaches the null terminator... */
        unsigned char asciiIndex = (unsigned char) *partialString;
        /* If there is no child node for a character, the partial 
            string does not exist in the trie */
        if (!currentNode->children[asciiIndex]) {
            return NULL;
        }
        /* Continue traversing trie */
        currentNode = currentNode->children[asciiIndex];
        partialString++;
    }

    return currentNode;
}

/*
    Given a trie node, recursively find if there is a marked word
    down all paths from that node.
*/

int hasMarkedWord(struct prefixTree *tree) {
    if (!tree) {
        return 0;
    }

    /* Marked word is found. Return true. */
    if (tree->character == NULL_TERMINATOR && tree->wordFound == MARKED) {
        return 1;
    }

    /* Recursively traverse */ 
    for (int i = 0; i < CHILD_COUNT; i++) {
        if (tree->children[i] != NULL) {
            return hasMarkedWord(tree->children[i]); 
        }
    }
    
    /* No marked words were found */
    return 0;
}

/*
    Frees memory allocated to a prefix trie.
*/
void freePrefixTree(struct prefixTree *tree) {
    if (!tree) {
        return;
    }
    
    /* Free all children nodes */
    for (int i = 0; i < CHILD_COUNT; i++) {
        if (tree->children[i] != NULL) {
            freePrefixTree(tree->children[i]);
        }
    }

    /* Free current node */
    free(tree);
    tree = NULL;
}



















