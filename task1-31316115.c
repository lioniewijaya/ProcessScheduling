/* task1-31316115.c
*
* Name:               Lionie Annabella Wijaya
* Student ID:         31316115
* Start date:         25 September 2021
* Last modified date: 6 October 2021
*
* Task 1: Non-preemptive scheduling, FCFS (First Come, First Served)
* task1-31316115.c is a program built to simulate process scheduling based on FCFS algorithm, a file containing processes information is read and used to calculate
* the result of running the processes based on the algorithm. Terminal will print state of scheduler accordingly in terminal and write result to a specific output file.
*/

/* Special enumerated data type for process state */
typedef enum {
    READY, RUNNING, EXIT
} process_state_t;

/* C data structure used as process information holder from default file read */
typedef struct {
    char process_name[11];
    int arrivalTime;
    int serviceTime;
    int deadline;
} process_read;

/* C data structure used as process control block. The scheduler
 * should create one instance per running proces in the system.
  */
typedef struct {
    char process_name[11]; // A string that identifies the process

    /* Times are measured in seconds. */
    int entryTime;          // The time process entered system
    int serviceTime;        // The total CPU time required by the process
    int remainingTime;      // Remaining service time until completion
    int everRun;            // Indicates if the process has ever been run
    int runTime;            // The total time process has run
    int finishTime;         // The time when process finished running
    int isTerminated;       // Process is terminated, this indicates process no longer exist
    process_read process;   // Process read information
    process_state_t state;  // Current process state (e.g. READY)
} pcb_t;

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Function: readProcessFromFile
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  read information of processes from a file
 *
 *  fptr: file pointer read from, process_read_array: an array to store read processes' information
 *
 *  returns: number of processes read from a file
 */
int readProcessFromFile(FILE *fptr, process_read process_read_array[]);

/*
 * Function: writeProcessToFile
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  write result of a terminated process' information to a file
 *
 *  fptr: file pointer to write, pcb_array: an array of pcb, index: index to get pcb from pcb array, num_of_process: number of processes read from a file
 *
 *  returns: void
 */
void writeProcessToFile(FILE *fptr, pcb_t pcb_array[], int index, int num_of_process);

/*
 * Function: addToScheduler
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  Add zero or more processes if the process enters at the current time 
 *
 *  process_read_array: an array to store read processes' information, pcb_array: an array of pcb, num_of_process: number of processes read from a file, 
 *  process_in_schedule: number of processes in scheduler, time: current time
 *
 *  returns: number of processes in scheduler
 */
int addToScheduler(process_read process_read_array[], pcb_t pcb_array[], int num_of_process, int process_in_schedule, int time);

/*
 * Function: checkProcessesTermination
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *  Check if all processes have been terminated
 *
 *  process_in_schedule: number of process in scheduler, pcb_array: an array of pcb, num_of_process: number of processes read from a file
 *
 *  returns: a boolean indicating if all processes have been terminated
 */
int checkProcessesTermination(int process_in_schedule, pcb_t pcb_array[], int num_of_process);

int main (int argc, char *argv[]) {
    FILE *fptr;
    int num_of_process;
    char *infilename;
    char *outfilename = "results-task1.txt";
    process_read process_read_array[100];

    // Open infile to read
    if (argc > 1) {
        infilename = argv[1];
    }
    else {
        infilename = "processes.txt";
    }
    if((fptr = fopen(infilename,"r")) == NULL) {
        printf("Error, file cannot be opened");   
        exit(1);             
    }

    // Read processes information from file into an array
    num_of_process = readProcessFromFile(fptr,process_read_array);

    // Close infile
    fclose(fptr); 

    // Open outfile to write
    if((fptr = fopen(outfilename,"w")) == NULL) {
        printf("Error, file cannot be opened");   
        exit(1);             
    }

    int process_in_schedule = 0;
    int time = 0;
    int done = 0;
    pcb_t pcb_array[num_of_process]; 

    // Run scheduling with FCFS
    while (done == 0) {
        // If a process arrives, add to scheduler
        process_in_schedule = addToScheduler(process_read_array,pcb_array,num_of_process,process_in_schedule,time);

        // Run a process on scheduler 
        // as arrival time in input is sorted already, we can run the very first process met that is ready/running from the beginning of the array
        for (int j = 0; j<process_in_schedule; j++) {
            // Terminated process is disregarded (information is not stored)
            if (pcb_array[j].isTerminated) {
                continue;
            }
            // Run a process that is ready in queue 
            else if (pcb_array[j].state == READY) {
                if (pcb_array[j].everRun == 0) {
                    pcb_array[j].entryTime = time;
                    pcb_array[j].everRun = 1;
                }

                pcb_array[j].state = RUNNING;
                pcb_array[j].runTime++;
                pcb_array[j].remainingTime = pcb_array[j].serviceTime - pcb_array[j].runTime;

                printf("Time %d: %s is in the running state.\n",time,pcb_array[j].process_name);
            }
            // Continue running previous process
            else if (pcb_array[j].state == RUNNING) {
                pcb_array[j].runTime++;
                pcb_array[j].remainingTime = pcb_array[j].serviceTime - pcb_array[j].runTime;
            }

            // Terminate a process
            if (pcb_array[j].everRun == 1  && pcb_array[j].remainingTime == 0) {
                pcb_array[j].state = EXIT;
                pcb_array[j].finishTime = time+1;
                pcb_array[j].isTerminated = 1;

                printf("Time %d: %s has finished execution.\n",pcb_array[j].finishTime,pcb_array[j].process_name);
                writeProcessToFile(fptr,pcb_array,j,num_of_process);
            }
            break;
        }

        // Check if scheduler is done
        done = checkProcessesTermination(process_in_schedule,pcb_array,num_of_process);
        time++;
    }

    fclose(fptr); 
    return 0;
}

int readProcessFromFile(FILE *fptr, process_read process_read_array[]) {
    int num_of_process = 0;
    char *line = NULL;
    char *temp; 
    size_t len = 0;

    while ((getline(&line, &len, fptr)) != -1) {
        process_read process_read;

        // Store line into process_read data structure
        temp = strtok (line," ");
        strcpy(process_read.process_name, temp);
        temp = strtok (NULL," ");
        process_read.arrivalTime = atoi(temp);
        temp = strtok (NULL," ");
        process_read.serviceTime = atoi(temp);
        temp = strtok (NULL," ");
        process_read.deadline = atoi(temp);

        // Add to array
        process_read_array[num_of_process] = process_read;
        num_of_process++;
    }
    if (line)
        free(line);

    return num_of_process;
}

void writeProcessToFile(FILE *fptr, pcb_t pcb_array[], int index, int num_of_process) {
    // Write according to format
    fprintf(fptr,"%s %d %d %d",pcb_array[index].process_name,pcb_array[index].entryTime - pcb_array[index].process.arrivalTime, pcb_array[index].finishTime-pcb_array[index].process.arrivalTime, pcb_array[index].finishTime-pcb_array[index].process.arrivalTime <= pcb_array[index].process.deadline);
    if (index < num_of_process-1) {
        fprintf(fptr,"\n");
    }
}

int addToScheduler(process_read process_read_array[], pcb_t pcb_array[], int num_of_process, int process_in_schedule, int time) {
    pcb_t pcb;
    while (process_in_schedule<num_of_process) {
        // If any process arrives
        if (process_read_array[process_in_schedule].arrivalTime == time) {
            // Save a process into a pcb data structure
            pcb.process = process_read_array[process_in_schedule];

            strcpy(pcb.process_name,process_read_array[process_in_schedule].process_name);
            pcb.serviceTime = process_read_array[process_in_schedule].serviceTime;
            pcb.remainingTime = pcb.serviceTime;
            pcb.everRun = 0;
            pcb.runTime = 0;
            pcb.state = READY;
            pcb.isTerminated = 0;

            pcb_array[process_in_schedule] = pcb;
            process_in_schedule++;

            printf("Time %d: %s has entered the system.\n",time,pcb.process_name);
            continue;
        }
        break;
    }
    return process_in_schedule;
}

int checkProcessesTermination(int process_in_schedule, pcb_t pcb_array[], int num_of_process) {
    int done = 0;
    // Check is done only after all processes are in scheduler
    if (process_in_schedule == num_of_process) {
        for (int j = 0; j<process_in_schedule; j++) {
            // If not all process are exited, scheduler is not done yet
            if (pcb_array[j].state != EXIT) {
                done = 0;
                break;
            }
            else {
                done = 1;
            }
        }
    }
    return done;
}