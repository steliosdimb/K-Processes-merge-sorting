#include <stdio.h>
#include <stdlib.h>
#include "parsing.h"
#include "record.h"
#include "mysort.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc, char *argv[])
{
    char *DataFile;
    char *sort1;
    char *sort2;
    int NumofChildren;
    parsing(argc, argv, &NumofChildren, &DataFile, &sort1, &sort2); // parse
    int fd = open_file(DataFile);                                   // open file for reading
    int numberofrecords = find_number_of_records(fd);               // find number of records in file
    printf("there are %d records in file that need to be sorted \n", numberofrecords);
    int pipes[NumofChildren][2]; // create an array of pipes for each children to avoid sychronization errors

    for (int i = 0; i < NumofChildren; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    int beginning_range = 0;
    size_t recordsize = sizeof(struct record);                                      // size of one record in file
    struct record *Records = (struct record *)malloc(numberofrecords * recordsize); // dynamically allocate memory for an array of records
    int counter = 0;
    signal(SIGUSR1, SIG_DFL); // seting the default signal handler
    int signal_counterUSR2 = 0;
    int timer_counter = 0;
    int temp_counter = 0;
    for (int i = 0; i < NumofChildren; i++)
    {
        for (int j = 0; j < NumofChildren - i; j++)
        {
            timer_counter++; // compute how many sorters will be created ,so i can allocate an array to store each sorters time taken
        }
    }
    double *timer = malloc(timer_counter * sizeof(double));
    for (int i = 0; i < NumofChildren; i++)
    {
        int distribute_records = numberofrecords / NumofChildren;
        if (i == 0 && (numberofrecords % NumofChildren != 0))
        { // if we are at the first children and the number of records is odd number then give the rest of records to this children
            distribute_records = distribute_records + (numberofrecords - (distribute_records * NumofChildren));
        }

        if (fork() == 0)
        {
            int old = beginning_range;
            close(pipes[i][0]);
            beginning_range = beginning_range + distribute_records;        // update the new beggining range for the next child
            write(pipes[i][1], &beginning_range, sizeof(beginning_range)); // write updated values to the write end of pipe
            write(pipes[i][1], &distribute_records, sizeof(distribute_records));
            dup2(pipes[i][1], STDOUT_FILENO); // dup2 write end of pipe to stdout so i can write to the write end of pipe the results from k children to the pipe of this child
            close(pipes[i][1]);
            create_sorters(old, distribute_records, NumofChildren - i, DataFile, numberofrecords, sort1, sort2); // beginning range gives the beggining of the range of which records this children will give to sorters to sort

            kill(getpid(), SIGUSR1); // send to child the signal SIGUSR1
        }
        else // if parent
        {

            close(pipes[i][1]);
            read(pipes[i][0], &beginning_range, sizeof(beginning_range));
            read(pipes[i][0], &distribute_records, sizeof(distribute_records)); // parents reads the updated values from children
            for (int j = 0; j < NumofChildren - i; j++)
            {
                read(pipes[i][0], &timer[temp_counter], sizeof(double)); // read time taken for each sorter and store it to timer array
                temp_counter++;                                          // to keep up with array cells
            }
            int temp;
            read(pipes[i][0], &temp, sizeof(int));                                  // get the number of usr2 signals from sorters
            while (read(pipes[i][0], &Records[counter], sizeof(struct record)) > 0) // i am reading and inserting each record from the output of k children to the array of records of this top level child
            {

                counter++; // to keep up with array cells
            }
            signal_counterUSR2 = signal_counterUSR2 + temp; // update total number of signals usr2
            close(pipes[i][0]);
        }
    }
    int status;
    int signal_counterUSR1 = 0;
    for (int i = 0; i < NumofChildren; i++)
    {
        wait(&status);
        if (WIFSIGNALED(status)) // here i dont ignore the signal (like i did in sorters) and if the status is a signal add up to signal counter
        {
            signal_counterUSR1++;
        }
    }
    Records = bubble_sort(Records, numberofrecords);
    for (int i = 0; i < numberofrecords; i++)
    {
        if (Records[i].id == 0) // i have a bug one record =0 ,couldnt fix it
        {
            continue;
        }
        printf("%-12s %-12s %-6d %s \n", Records[i].lastname, Records[i].firstname, Records[i].id, Records[i].postal);
    }
    free(Records);
    printf("%d USR2 signals ,USR1 signals %d\n", signal_counterUSR2, signal_counterUSR1);
    double total_time_taken = 0;
    for (int j = 0; j < timer_counter; j++)
    {
        printf("%f seconds taken for number %d sorter to execute\n", timer[j], j + 1);
        total_time_taken = total_time_taken + timer[j]; // compute total time taken
    }
    printf("total time taken in seconds:%f\n", total_time_taken);
    free(timer);
}
int open_file(char *file)
{
    int fd = open(file, O_RDONLY); // open file for reading
    if (fd == -1)
    {
        printf("Error Opening File!!\n");
        exit(1);
    }
    else
    {
        printf("File opened successfully!\n");
    }
    return fd;
}
int find_number_of_records(int fd)
{
    size_t recordsize = sizeof(struct record);                // size of one record in file
    struct record *Rec = (struct record *)malloc(recordsize); // dynamically allocate memory for one record
    if (Rec == NULL)
    {
        printf("malloc failed\n");
        exit(-1);
    }
    int numberofrecords = 0;
    while (read(fd, Rec, recordsize) == recordsize) // read each record from file
    {
        numberofrecords++; // find the total number of records in file
    }
    free(Rec);
    return numberofrecords;
}
void create_sorters(int beginning_range, int records_num, int how_many_sorters, char *datafile, int total_recs, char *sort1, char *sort2)
{
    int pipes[how_many_sorters][2];                                             // create an array of pipes for each children to avoid sychronization errors
    size_t recordsize = sizeof(struct record);                                  // size of one record in file
    struct record *Records = (struct record *)malloc(records_num * recordsize); // create an array of records length records num
    int counter = 0;
    for (int i = 0; i < how_many_sorters; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    signal(SIGUSR2, SIG_IGN); // seting the ignore signal handler(i dont use it,just take the info that i got a signal),with signal SIGUSR2,i ignore it because i need to kill before execvp,if i used dflt handler i would execute execvp
    int sorter_beginning_range = beginning_range;
    int sorter_ending_range = 0;
    int signal_counter = 0;
    double *timer = malloc(how_many_sorters * sizeof(double)); // create array to store time for each sorter
    for (int i = 0; i < how_many_sorters; i++)
    {
        clock_t start = clock();
        clock_t end;
        int distribute_records = records_num / how_many_sorters; // with the same way like the previous children i find how to distribute those records to sorters
        if (i == 0 && (records_num % how_many_sorters != 0))
        { // if we are at the first children and the number of records is odd number then give the rest of records to this children
            distribute_records = distribute_records + (records_num - (distribute_records * how_many_sorters));
        }
        sorter_ending_range = sorter_beginning_range + distribute_records - 1;
        if (fork() == 0) // if child
        {
            close(pipes[i][0]); // closing read end of pipe beacuse i wont use it
            char *parameter1 = (char *)malloc(32 * sizeof(char));
            sprintf(parameter1, "%d", distribute_records); // store records num as a char so i can pass it to argv
            char *parameter2 = (char *)malloc(32 * sizeof(char));
            sprintf(parameter2, "%d", sorter_beginning_range);
            char *parameter3 = (char *)malloc(32 * sizeof(char));
            sprintf(parameter3, "%d", sorter_ending_range);
            sorter_beginning_range = sorter_beginning_range + distribute_records;
            write(pipes[i][1], &sorter_beginning_range, sizeof(sorter_beginning_range));
            write(pipes[i][1], &distribute_records, sizeof(distribute_records)); // update sorter beginning range before execv because code will be replaced from execvp and nothing will change
            dup2(pipes[i][1], STDOUT_FILENO);
            close(pipes[i][1]); // closing write end of pipe because i will redirect it to stdout

            kill(getpid(), SIGUSR2); // send to chilmd the signal SIGUSR2
            char *argv[] = {
                datafile, parameter1, parameter2, parameter3, NULL};

            if (i % 2 == 0)
            {
                if (execv(sort1, argv) == -1)
                {
                    perror("execv");
                    exit(-1);
                }
            }
            else
            {
                if (execv(sort2, argv) == -1)
                {
                    perror("execv");
                    exit(-1);
                }
            }

            exit(0);
        }
        else // if parent
        {
            close(pipes[i][1]);
            read(pipes[i][0], &sorter_beginning_range, sizeof(sorter_beginning_range));
            read(pipes[i][0], &distribute_records, sizeof(distribute_records)); // parents reads the updated values from children

            while (read(pipes[i][0], &Records[counter], sizeof(struct record)) > 0) // i am reading and inserting each record from the output of the bubblesort to the array
            {
                counter++;
                if (counter == records_num)
                {
                    break;
                }
            }
            end = clock();
            double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
            timer[i] = time_taken;
            close(pipes[i][0]);
        }
    }
    int status;
    for (int i = 0; i < how_many_sorters; i++)
    {
        wait(&status);         // wait each children
        if (WIFEXITED(status)) // if status is that a children has exited add one to signal counter,i ignored the signal because if i didnt execvp would be executed
        {
            signal_counter++;
        }
    }
    for (int j = 0; j < how_many_sorters; j++)
    {
        write(STDOUT_FILENO, &timer[j], sizeof(double)); // print time taken for each sorter
    }
    write(STDOUT_FILENO, &signal_counter, sizeof(int)); // print the number of usr2 signals from sorters
    for (int i = 0; i < records_num; i++)
    {

        write(STDOUT_FILENO, &Records[i], sizeof(Records[i])); // print to stdout so i can dup2 to stdout the write end of parent pipe,and get the result
    }
    free(timer);
    free(Records);
}

struct record *bubble_sort(struct record *Records, int total_recs)
{
    for (int j = 0; j < total_recs; j++)
    {
        for (int i = j + 1; i < total_recs; i++)
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
    return Records;
}

