#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "record.h"
#include "create_array_of_recs.h"
struct record *create_array_of_recs(char *datafile, int num_of_records, int beginning_range, int ending_range)
{

    int fd = open(datafile, O_RDONLY); // open file for reading
    lseek(fd, 0, SEEK_SET);
    if (fd == -1)
    {
        printf("Error Opening File!!\n");
        exit(1);
    }
    else
    {
        // printf("File opened successfully!\n");
    }
    size_t recordsize = sizeof(struct record);                                     // size of one record in file
    struct record *Records = (struct record *)malloc(num_of_records * recordsize); // dynamically allocate memory for an array of records
    struct record *Rec = (struct record *)malloc(recordsize);                      // dynamically allocate memory for one record
    if (Records == NULL)
    {
        printf("malloc failed\n");
        exit(-1);
    }
    int counter = 0;
    if (beginning_range > 0) // if the beginning is 0 skip this part
    {
        while (read(fd, Rec, recordsize) == recordsize) // read each record from file till we reach the desired beginning range
        {
            if (counter == beginning_range)
            {
                break;
            }
            counter++;
        }
    }
    int i = 0;                                      // that will be used for storing in cells of the array
    while (read(fd, Rec, recordsize) == recordsize) // read each record till we reach the desired ending range and add each record to the array of records
    {

        Records[i] = *Rec;

        if (counter == ending_range)
        {
            break;
        }
        counter++;
        i++;
    }

    close(fd);      // close file
    return Records; // return array
}