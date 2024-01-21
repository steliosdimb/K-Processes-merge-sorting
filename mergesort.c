#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "record.h"
#include "mergesort.h"
#include "create_array_of_recs.h"
int main(int argc, char *argv[])
{
    struct record *Records = create_array_of_recs(argv[0], atoi(argv[1]), atoi(argv[2]), atoi(argv[3])); // create the array of records
    merge_sort(Records, 0, atoi(argv[1]) - 1);                                                           // arguments:array of records,0,total records-1
    for (int i = 0; i < atoi(argv[1]); i++)
    {
        write(STDOUT_FILENO, &Records[i], sizeof(Records[i]));
    }
}
void merge(struct record *Recs, int left, int middle, int right)
{
    int i, j, k;
    int first_half = middle - left + 1;
    int second_half = right - middle;
    struct record *L = (struct record *)malloc(first_half * sizeof(struct record)); // create temporary arrays
    struct record *R = (struct record *)malloc(second_half * sizeof(struct record));
    for (i = 0; i < first_half; i++) // copy data to the arrays created
        L[i] = Recs[left + i];
    for (j = 0; j < second_half; j++)
        R[j] = Recs[middle + 1 + j];
    i = 0;
    j = 0;
    k = left;
    while (i < first_half && j < second_half)
    {
        // compare arrays based on ,lastname,firstname and then id
        int compareResult = strcmp(L[i].lastname, R[j].lastname);
        if (compareResult < 0 || (compareResult == 0 && strcmp(L[i].firstname, R[j].firstname) < 0) ||
            (compareResult == 0 && strcmp(L[i].firstname, R[j].firstname) == 0 && L[i].id < R[j].id))
        {
            Recs[k] = L[i];
            i++;
        }
        else
        {
            Recs[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < first_half) // copy the rest of L array elements
    {
        Recs[k] = L[i];
        i++;
        k++;
    }
    while (j < second_half) // copy the rest of R array elements
    {
        Recs[k] = R[j];
        j++;
        k++;
    }
    free(L);
    free(R);
}

void merge_sort(struct record *Recs, int left, int right)
{
    if (left < right)
    {
        int middle = left + (right - left) / 2; // split array in two halves

        merge_sort(Recs, left, middle);      // sort array from beginning to middle
        merge_sort(Recs, middle + 1, right); // sort array from middle+1 to end

        // merge sorted arrays
        merge(Recs, left, middle, right);
    }
}