#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#include "tracker.h"

void *mem_tracker(void *arg){
    long phys_pages = 0;
    long avail_phys_pages = 0;
    long p_avail_phys_pages = LONG_MAX;

    // Tracks the percent of memory which is free
    double percent = 0.0;

    // Options
    struct tracker_arg track = *(struct tracker_arg *)arg;

    phys_pages = sysconf(_SC_PHYS_PAGES);
    avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

    fprintf(stdout, "Page Size:\t%ld\n", sysconf(_SC_PAGESIZE));
    fprintf(stdout, "Total Physical Memory:\t%ld pages\t(%"PRIu64" bytes)\n", 
            phys_pages, (uint64_t)phys_pages * (uint64_t)sysconf(_SC_PAGESIZE));
    fprintf(stdout, "Total Available Memory:\t%ld pages\t(%"PRIu64" bytes)\n\n",
            avail_phys_pages, (uint64_t)avail_phys_pages * (uint64_t)sysconf(_SC_PAGESIZE)); 

    do{
        avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

        long diff = avail_phys_pages - p_avail_phys_pages;
        diff = (diff < 0) ? -diff : diff;
        if((double)diff / (double)phys_pages > track.print_threshold){
            percent = 100*((double)avail_phys_pages/phys_pages);
            track.print_func(percent);

            p_avail_phys_pages = avail_phys_pages;
        }

        usleep(track.poll);
    }while(track.poll);

    return 0;
}

