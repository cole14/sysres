#ifndef TRACKER_H
#define TRACKER_H

#include "print_funcs.h"

struct tracker_arg;

typedef void *(*info_func_t)(struct tracker_arg *);

struct tracker_arg{
    info_func_t info_func;
    print_func_t print_func;
    unsigned long poll;
    double print_threshold;
};

void *tracker(void *arg);
void *mem_info_func(struct tracker_arg *arg);
void *cpu_info_func(struct tracker_arg *arg);

#endif /* TRACKER_H */

