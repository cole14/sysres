#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "tracker.h"

#define GET_PROC_ENT_STR_LENGTH(val) (sizeof(#val) + 1)
static size_t proc_ent_str_len = GET_PROC_ENT_STR_LENGTH(ULLONG_MAX);

static char *proc_ents[5];

static void parse_cpu_info(FILE *proc_fp, unsigned long long *usage, unsigned long long *total){
    char *cpu_line = NULL;
    size_t cpu_line_len = 0;
    unsigned long long user, user_low, system, idle;

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
    user     = strtoull(proc_ents[1], NULL, 10);
    user_low = strtoull(proc_ents[2], NULL, 10);
    system   = strtoull(proc_ents[3], NULL, 10);
    idle     = strtoull(proc_ents[4], NULL, 10);
    if(errno != 0){
        error(-1, errno, "Failed parsing the CPU usage in /proc/stat");
    }

    //Calculate the CPU usage
    *usage = user + user_low + system;
    *total = *usage + idle;

    return;
}

static void one_time_cpu_init(struct tracker_arg *track, unsigned long long *cur_usage, unsigned long long *cur_total){
    //Allocate the char buffers for use in sscanf
    int i;
    for(i = 0; i < 5; i++){
        proc_ents[i] = (char *)malloc(proc_ent_str_len * sizeof(char));
    }

    //Get the total CPU usage.
    FILE *proc_fp = fopen("/proc/stat", "r");
    if(!proc_fp){ error(-1, errno, "Cannot open /proc/stat."); }
    parse_cpu_info(proc_fp, cur_usage, cur_total);
    fclose(proc_fp);

    unsigned long delay = track->poll ? track->poll : 10000;
    usleep(delay);
}

void *cpu_info_func(struct tracker_arg *track){
    FILE *proc_fp = NULL;
    double delta = 0.0;

    //Variables to track cpu usage
    static unsigned long long prev_usage = 0;
    static unsigned long long prev_total = 0;
    static unsigned long long cur_usage = 0;
    static unsigned long long cur_total = 0;
    static double prev_percent = 0.0;
    static double percent = DBL_MAX;
    
    //Make sure any necessary resources are allocated
    if(!prev_usage) one_time_cpu_init(track, &cur_usage, &cur_total);

    prev_usage = cur_usage;
    prev_total = cur_total;
    prev_percent = percent;

    //Open /proc/stat
    proc_fp = fopen("/proc/stat", "r");
    if(!proc_fp){ error(-1, errno, "Cannot open /proc/stat."); }

    //Calculate the CPU usage
    parse_cpu_info(proc_fp, &cur_usage, &cur_total);

    percent = 100*((double)(cur_usage - prev_usage)) / ((double)(cur_total - prev_total));

    delta = (prev_percent > percent ? prev_percent - percent : percent - prev_percent) / 100;
    if(delta > track->print_threshold){
        track->print_func(percent);
    }

    //Close /proc/stat
    fclose(proc_fp);

    return NULL;
}

