#include "parsing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void parsing(int argc, char *argv[], int *NumofChildren, char **DataFile, char **sorting1, char **sorting2)
{
    int i, flagi = 0, flagk = 0, flage1 = 0, flage2 = 0;
    for (i = 1; i < argc; i++) // check each user argument
    {

        if (strcmp(argv[i], "-i") == 0) // if flag is -i
        {
            flagi = 1;
            if (argv[i + 1] != NULL)
            {
                *DataFile = argv[i + 1]; // set Datafile in main
            }
            else
            {
                printf("Sorry Wrong Input!Try Again!\n"); // user need to provide a filename after flag -i
                exit(-1);
            }
        }
        else if (strcmp(argv[i], "-k") == 0) // if flag is -k
        {
            flagk = 1;
            if (argv[i + 1] != NULL)
            {
                *NumofChildren = atoi(argv[i + 1]); // set number of NumofChildren in main
                char *check = argv[i + 1];
                strtol(check, &check, 10);
                if (*check != '\0')
                {
                    printf("Sorry Wrong Input,children must be an integer!\n");
                    exit(-1);
                }
            }
            else
            {
                printf("Sorry Wrong Input!Try Again!\n"); // user needs to provide number of NumOfChildren after flag -k
                exit(-1);
            }
        }
        else if (strcmp(argv[i], "-e1") == 0)
        { // if flag is sorting algorithm 1 -e1
            flage1 = 1;
            if (argv[i + 1] != NULL)
            {
                *sorting1 = argv[i + 1];
            }
            else
            {
                printf("Sorry Wrong Input!Try Again!\n"); // user needs to provide a name of sorting algorithm 1
                exit(-1);
            }
        }
        else if (strcmp(argv[i], "-e2") == 0)
        {
            flage2 = 1;
            if (argv[i + 1] != NULL)
            {
                *sorting2 = argv[i + 1];
            }
            else
            {
                printf("Sorry Wrong Input!Try Again!\n"); // user needs to provide a name of sorting algorithm 2
                exit(-1);
            }
        }
    }
    if (flagi == 0 || flagk == 0 || flage1 == 0 || flage2 == 0)
    {
        printf("Sorry Wrong Input!Try Again!\n"); // user needs to provide both flag -i flag -k flag -e1 flag -e2
        exit(-1);
    }
}
