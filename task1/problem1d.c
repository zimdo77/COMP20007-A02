/*
    Written by Grady Fitzpatrick for 
    COMP20007 Assignment 2 2024 Semester 1
    
    Make using
        make problem1d
    
    Run using
        ./problem1d seqA seqB window_size
    
    where seqA is the name of the file with the 
        first sequence of values in the expected 
        format (e.g. test_cases/1d-1-seqA.txt), 
        seqB is the name of the file with the 
        second sequence of values in the expected 
        format (e.g. test_cases/1d-1-seqB.txt),
        and window_size is the size of the window
        for use in the modified DTW, for example:
    
        ./problem1d test_cases/1d-1-seqA.txt test_cases/1d-1-seqB.txt 3
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <error.h>
#include "problem.h"

#define SEQ_A_ARG 1
#define SEQ_B_ARG 2
#define WINDOW_SIZE_ARG 3

#define NUMBER_BASE (10)

int main(int argc, char **argv){
    struct problem *problem;
    struct solution *solution;
    /* Load file with first sequence from argv[1]. */
    FILE *seqAFile = NULL;
    /* Load file with second sequence from argv[2]. */
    FILE *seqBFile = NULL;

    int window_size = 0;

    if(argc < 4){
        fprintf(stderr, "You only gave %d arguments to the program, \n"
            "you should run the program with in the form \n"
            "\t./problem1d seqA seqB window_size\n", argc);
        return EXIT_FAILURE;
    } 
    
    /* Attempt to open dictionary and board files. */
    seqAFile = fopen(argv[SEQ_A_ARG], "r");
    if(! seqAFile){
        fprintf(stderr, "File given as dictionary file was \"%s\", "
            "which was unable to be opened\n", argv[SEQ_A_ARG]);
        perror("Reason for file open failure");
        return EXIT_FAILURE;
    }

    seqBFile = fopen(argv[SEQ_B_ARG], "r");
    if(! seqBFile){
        fprintf(stderr, "File given as board file was \"%s\", "
            "which was unable to be opened\n", argv[SEQ_B_ARG]);
        perror("Reason for file open failure");
        return EXIT_FAILURE;
    }

    window_size = strtol(argv[WINDOW_SIZE_ARG], NULL, NUMBER_BASE);
    
    problem = readProblemD(seqAFile, seqBFile, window_size);

    /* Files are no longer needed since they're read in. */
    if(seqAFile){
        fclose(seqAFile);
    }

    if(seqBFile){
        fclose(seqBFile);
    }

    solution = solveProblemD(problem);

    outputProblem(problem, solution, stdout);

    freeSolution(solution, problem);

    freeProblem(problem);

    return EXIT_SUCCESS;
}
