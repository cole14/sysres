#ifndef TRACKER_H
#define TRACKER_H

#include "print_funcs.h"

void *mem_tracker(void *arg);
void *cpu_tracker(void *arg);

struct tracker_arg{
    print_func_t print_func;
    unsigned long poll;
    double print_threshold;
};

#endif /* TRACKER_H */

