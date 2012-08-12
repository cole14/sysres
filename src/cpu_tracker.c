#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "tracker.h"

#define GET_PROC_ENT_STR_LENGTH(val) (sizeof(#val) + 1)
static size_t proc_ent_str_len = GET_PROC_ENT_STR_LENGTH(ULONG_MAX);

static char *proc_ents[5];

static void parse_cpu_info(FILE *proc_fp, unsigned long *usage, unsigned long *total){
    char *cpu_line = NULL;
    size_t cpu_line_len = 0;
    unsigned long user, user_low, system, idle;

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

    //Calculate the CPU usage
    *usage = user + user_low + system;
    *total = *usage + idle;

    return;
}

void *cpu_tracker(void *arg){
    FILE *proc_fp = NULL;
    unsigned long cur_usage = ULONG_MAX;
    unsigned long cur_total = ULONG_MAX;
    unsigned long prev_usage = ULONG_MAX;
    unsigned long prev_total = ULONG_MAX;
    double percent = 0.0;
    double prev_percent = 0.0;
    double delta = 0.0;

    // Options
    struct tracker_arg track = *(struct tracker_arg *)arg;

    //Allocate the char buffers for use in sscanf
    int i;
    for(i = 0; i < 5; i++){
        proc_ents[i] = (char *)malloc(proc_ent_str_len * sizeof(char));
    }

    //Get the initial CPU usage.
    proc_fp = fopen("/proc/stat", "r");
    if(!proc_fp){ error(-1, errno, "Cannot open /proc/stat."); }
    parse_cpu_info(proc_fp, &cur_usage, &cur_total);
    fclose(proc_fp);

    fprintf(stdout, "Total uptime usage: %lf\n", (double)cur_usage / cur_total);

    //Track the cpu usage
    do{
        //Open /proc/stat
        proc_fp = fopen("/proc/stat", "r");
        if(!proc_fp){ error(-1, errno, "Cannot open /proc/stat."); }

        //Cache the old CPU usage.
        prev_usage = cur_usage;
        prev_total = cur_total;

        //Calculate the CPU usage
        parse_cpu_info(proc_fp, &cur_usage, &cur_total);

        prev_percent = percent;
        percent = 100*((double)(cur_usage - prev_usage)) / ((double)(cur_total - prev_total));

        delta = (prev_percent > percent ? prev_percent - percent : percent - prev_percent) / 100;
        if(delta > track.print_threshold){
            track.print_func(percent);
        }

        //Close /proc/stat
        fclose(proc_fp);

        //Wait the polling amount
        usleep(track.poll);
    }while(track.poll);

    return NULL;
}

