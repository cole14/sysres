#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

#include "tracker.h"


void *mem_info_func(print_func_t print_func, double print_threshold) {
    long phys_pages = 0;
    long avail_phys_pages = 0;
    static long p_avail_phys_pages = LONG_MAX;

    // Tracks the percent of memory which is free
    double percent_used = 0.0;

    // Get the current number of free pages
    phys_pages = sysconf(_SC_PHYS_PAGES);
    avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

    // Calculate the change since the last print
    long diff = (avail_phys_pages > p_avail_phys_pages) ? avail_phys_pages - p_avail_phys_pages : p_avail_phys_pages  - avail_phys_pages;

    // Print if the percent change in memory usage is greater than the print threshold
    if((double)diff / (double)phys_pages > print_threshold){
        // Convert the number of free pages to memory usage
        percent_used = 100.0 - (100.0*((double)avail_phys_pages/phys_pages));

        // Print the memory usage 
        print_func(percent_used);

        // Update the previous usage 
        p_avail_phys_pages = avail_phys_pages;
    }

    return NULL;
}

