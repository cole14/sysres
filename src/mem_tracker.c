#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#include "tracker.h"


void *mem_info_func(struct tracker_arg *arg){
    long phys_pages = 0;
    long avail_phys_pages = 0;
    static long p_avail_phys_pages = LONG_MAX;

    // Tracks the percent of memory which is free
    double percent_used = 0.0;

    // Options
    struct tracker_arg *track = (struct tracker_arg *)arg;

    phys_pages = sysconf(_SC_PHYS_PAGES);
    avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

    long diff = (avail_phys_pages > p_avail_phys_pages) ? avail_phys_pages - p_avail_phys_pages : p_avail_phys_pages  - avail_phys_pages;
    if((double)diff / (double)phys_pages > track->print_threshold){
        percent_used = 100.0 - (100.0*((double)avail_phys_pages/phys_pages));
        track->print_func(percent_used);
    }

    p_avail_phys_pages = avail_phys_pages;

    return NULL;
}

