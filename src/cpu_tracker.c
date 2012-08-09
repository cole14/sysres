#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "tracker.h"

#define GET_PROC_ENT_STR_LENGTH(val) (sizeof(#val) + 1)
static size_t proc_ent_str_len = GET_PROC_ENT_STR_LENGTH(ULONG_MAX);

void *cpu_tracker(void *arg){
    char *cpu_line = NULL;
    size_t cpu_line_len = 0;
    FILE *proc_fp = NULL;
    char *proc_ents[5] = { NULL };
    unsigned long user, user_low, system, idle;
    unsigned long cur_usage = ULONG_MAX;
    unsigned long cur_total = ULONG_MAX;
    unsigned long prev_usage = ULONG_MAX;
    unsigned long prev_total = ULONG_MAX;
    double percent = 0.0;
    // Used in printing the time
    time_t t = 0;
    char * t_str = NULL;

    // Options
    struct tracker_arg track = *(struct tracker_arg *)arg;

    //Allocate the char buffers for use in sscanf
    int i;
    for(i = 0; i < 5; i++){
        proc_ents[i] = (char *)malloc(proc_ent_str_len * sizeof(char));
    }

    //Track the cpu usage
    do{
        //Open /proc/stat
        proc_fp = fopen("/proc/stat", "r");
        if(!proc_fp){ error(-1, errno, "Cannot open /proc/stat."); }

        //Read the cpu info line
        if(-1 == getline(&cpu_line, &cpu_line_len, proc_fp)){
            error(-1, errno, "Error reading line from /proc/stat");
        }

        //Parse the cpu info line
        sscanf(cpu_line, "%20s %20s %20s %20s %20s", proc_ents[0], proc_ents[1], proc_ents[2], proc_ents[3], proc_ents[4]);

        if(NULL == strstr(proc_ents[0], "cpu")){
            error(-1, errno, "Unexpected format of /proc/stat");
        }

        errno = 0;
        user     = strtoul(proc_ents[1], NULL, 10);
        user_low = strtoul(proc_ents[2], NULL, 10);
        system   = strtoul(proc_ents[3], NULL, 10);
        idle     = strtoul(proc_ents[4], NULL, 10);
        if(errno != 0){
            error(-1, errno, "Failed parsing the CPU usage in /proc/stat");
        }

        //Cache the old CPU usage.
        prev_usage = cur_usage;
        prev_total = cur_total;

        //Calculate the CPU usage
        cur_usage = user + user_low + system;
        cur_total = cur_usage + idle;

        percent = ((double)(cur_usage - prev_usage)) / ((double)(cur_total - prev_total));

        if(percent > track.print_threshold){
            // Get the current time
            time(&t);
            t_str = ctime(&t);
            t_str[strlen(t_str)-1] = 0;

            percent *= 100;
            track.print_func(t_str, percent);
        }

        fclose(proc_fp);

        //Wait the polling amount
        usleep(track.poll);
    }while(track.poll);

    return NULL;
}

