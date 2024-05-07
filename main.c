#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

time_t start_time;

struct PrioritizedPCB {
    int pid;
    int burst_time;
    int arrival_time;
    int priority;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    char state[20];
    struct PrioritizedPCB *next;
};

struct PCB {
    int pid;
    int burst_time;
    int arrival_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    char state[20];
    struct PCB *next;
};

struct PCB *ready_queue = NULL;
struct PCB *completed_processes = NULL;
pthread_mutex_t ready_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_process(struct PCB *new_process) {
    pthread_mutex_lock(&ready_queue_mutex);
    if (ready_queue == NULL) {
        ready_queue = new_process;
    } else {
        struct PCB *current = ready_queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_process;
    }
    pthread_mutex_unlock(&ready_queue_mutex);
}

struct PCB *select_next_process() {
    pthread_mutex_lock(&ready_queue_mutex);
    if (ready_queue == NULL) {
        pthread_mutex_unlock(&ready_queue_mutex);
        return NULL;
    }
    struct PCB *next_process = ready_queue;
    ready_queue = ready_queue->next;
    next_process->next = NULL;
    pthread_mutex_unlock(&ready_queue_mutex);
    return next_process;
}

void execute_process(struct PCB *process) {
    
    time_t current_time = time(NULL);
    double elapsed_time = difftime(current_time, start_time);

    printf("\n - Current Time: %.0f seconds since start\n\n", elapsed_time);
    printf("Executing process with PID %d, Burst Time: %d\n", process->pid, process->burst_time);


    sleep(process->burst_time);
    printf("Process with PID %d finished execution.\n", process->pid);

    strcpy(process->state, "Terminated");
    time_t end_time = time(NULL);

    process->completion_time = difftime(end_time, start_time);
    process->turnaround_time = process->completion_time - process->arrival_time;
    process->waiting_time = process->turnaround_time - process->burst_time;

    // Instead of freeing the PCB, we add it to the completed list
    pthread_mutex_lock(&ready_queue_mutex);
    process->next = completed_processes;
    completed_processes = process;
    pthread_mutex_unlock(&ready_queue_mutex);
}

void* scheduler(void *args) {
    struct PCB *currently_running_process = NULL;
    while (1) {
        pthread_mutex_lock(&ready_queue_mutex);
        if (!ready_queue) {
            pthread_mutex_unlock(&ready_queue_mutex);
            if (currently_running_process == NULL) {
                printf("No processes in the queue and no process is running.\n");
                break;
            }
            continue;  // Keep the scheduler running if a process is still running
        }
        
        struct PCB *highest_priority_process = (struct PCB *)ready_queue;
        if (currently_running_process == NULL || ((struct PrioritizedPCB *)highest_priority_process)->priority < ((struct PrioritizedPCB *)currently_running_process)->priority) {
            // Preempt the current process if necessary
            if (currently_running_process != NULL) {
                // Simulate preemption by re-adding the preempted process back to the ready queue
                printf("Preempting Process %d with Process %d\n", currently_running_process->pid, highest_priority_process->pid);
                currently_running_process->next = ready_queue;
                ready_queue = currently_running_process;
            }
            // Select the highest priority process to run next
            ready_queue = ready_queue->next;
            highest_priority_process->next = NULL;
            currently_running_process = highest_priority_process;
        }
        pthread_mutex_unlock(&ready_queue_mutex);

        if (currently_running_process != NULL) {
            execute_process(currently_running_process);  // Execute the selected or continuing process
            currently_running_process = NULL;
        }
    }
    return NULL;
}

void display_statistics() {
    struct PCB *current = completed_processes;
    double total_tat = 0, total_wt = 0;
    int count = 0;

    printf("\nProcess Execution Statistics:\n");
    printf("PID\tCompletion Time\tTurnaround Time\tWaiting Time\n");

    while (current != NULL) {
        printf("%d\t%d\t\t%d\t\t%d\n", current->pid, current->completion_time, current->turnaround_time, current->waiting_time);
        total_tat += current->turnaround_time;
        total_wt += current->waiting_time;
        count++;
        current = current->next;
    }

    if (count > 0) {
        printf("\nAverage Turnaround Time: %.2f\n", total_tat / count);
        printf("Average Waiting Time: %.2f\n", total_wt / count);
    }
}


void FCFS() {
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, scheduler, NULL);

    // Simulate processes (creating PCBs and adding them to the ready queue)
    for (int i = 1; i <= 5; ++i) {
        struct PCB *new_process = (struct PCB*)malloc(sizeof(struct PCB));
        new_process->pid = i;
        new_process->burst_time = 1 + rand() % 5; // Random burst time between 1 and 5

        time_t current_time = time(NULL);
        double elapsed_time = difftime(current_time, start_time);

        new_process->arrival_time = elapsed_time; // Random arrival time between 0 and 9
        strcpy(new_process->state, "Ready");
        new_process->next = NULL;
        add_process(new_process);
    }

    pthread_join(scheduler_thread, NULL);
}

void SJF() {
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, scheduler, NULL);

    // Simulate processes (creating PCBs and adding them to the ready queue)
    for (int i = 1; i <= 5; ++i) {
        struct PCB *new_process = (struct PCB*)malloc(sizeof(struct PCB));
        new_process->pid = i;
        new_process->burst_time = 1 + rand() % 5; // Random burst time between 1 and 5

        time_t current_time = time(NULL);
        double elapsed_time = difftime(current_time, start_time);

        new_process->arrival_time = elapsed_time; // Random arrival time between 0 and 9
        strcpy(new_process->state, "Ready");
        new_process->next = NULL;
        
        // Insert process in the ready queue based on burst time (SJF)
        pthread_mutex_lock(&ready_queue_mutex);
        if (ready_queue == NULL || new_process->burst_time < ready_queue->burst_time) {
            new_process->next = ready_queue;
            ready_queue = new_process;
        } else {
            struct PCB *current = ready_queue;
            while (current->next != NULL && current->next->burst_time < new_process->burst_time) {
                current = current->next;
            }
            new_process->next = current->next;
            current->next = new_process;
        }
        pthread_mutex_unlock(&ready_queue_mutex);
    }

    pthread_join(scheduler_thread, NULL);
}

void priorityPreemptive() {
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, scheduler, NULL);

    // Simulate processes (creating PCBs and adding them to the ready queue)
    for (int i = 1; i <= 5; ++i) {
        struct PrioritizedPCB *new_process = (struct PrioritizedPCB*)malloc(sizeof(struct PrioritizedPCB));
        new_process->pid = i;
        new_process->burst_time = 1 + rand() % 5; // Random burst time between 1 and 5

        time_t current_time = time(NULL);
        double elapsed_time = difftime(current_time, start_time);

        new_process->arrival_time = elapsed_time; // Random arrival time between 0 and 9
        new_process->priority = rand() % 10; // Random priority between 0 and 9
        strcpy(new_process->state, "Ready");
        new_process->next = NULL;
        
        // Insert process in the ready queue based on priority (preemptive)
        pthread_mutex_lock(&ready_queue_mutex);
        if (ready_queue == NULL || new_process->priority < ((struct PrioritizedPCB *)ready_queue)->priority) {
            new_process->next = (struct PrioritizedPCB *)ready_queue;
            ready_queue = (struct PCB *)new_process;
        } else {
            struct PCB *current = (struct PCB *)ready_queue;
            while (current->next != NULL && ((struct PrioritizedPCB *)current->next)->priority <= new_process->priority) {
                current = current->next;
            }
            new_process->next = ((struct PrioritizedPCB *)current->next);
            current->next = (struct PCB *)new_process;
        }
        pthread_mutex_unlock(&ready_queue_mutex);
    }

    pthread_join(scheduler_thread, NULL);
}


int main() {
    start_time = time(NULL);

    // FCFS();
    // Uncomment as needed:
    // SJF();
    priorityPreemptive();

    
    // Display statistics after all processes have finished executing
    display_statistics();
    return 0;
}
