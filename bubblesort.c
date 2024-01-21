#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "record.h"
#include "bubblesort.h"
#include "create_array_of_recs.h"
int main(int argc, char *argv[])
{
    bubblesort(argv[0], atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
}
void bubblesort(char *datafile, int num_of_records, int beginning_range, int ending_range)
{
    struct record *Records = create_array_of_recs(datafile, num_of_records, beginning_range, ending_range); // create the array of records from file

    if (num_of_records <= 1)
    {
        return;
    }
    for (int j = 0; j < num_of_records; j++)
    {
        for (int i = j + 1; i < num_of_records; i++)
        {
            if (strcmp(Records[j].lastname, Records[i].lastname) > 0) // sort by lastname
            {
                struct record temp = Records[j];
                Records[j] = Records[i];
                Records[i] = temp;
            }
            else if (strcmp(Records[j].lastname, Records[i].lastname) == 0) // sort by firstname if lastname equal
            {
                if (strcmp(Records[j].firstname, Records[i].firstname) > 0)
                {
                    struct record temp = Records[j];
                    Records[j] = Records[i];
                    Records[i] = temp;
                }
                else if (strcmp(Records[j].firstname, Records[i].firstname) == 0) // sort by id if firstname lastname equal
                {
                    if (Records[j].id > Records[i].id)
                    {
                        struct record temp = Records[j];
                        Records[j] = Records[i];
                        Records[i] = temp;
                    }
                }
            }
        }
    }
    for (int i = 0; i < num_of_records; i++)
    {
        write(STDOUT_FILENO, &Records[i], sizeof(Records[i]));
    }
    free(Records);
}
