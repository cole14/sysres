#ifndef MEM_TRACKER_H
#define MEM_TRACKER_H

#include "print_funcs.h"

void *mem_tracker(void *arg);

struct tracker_arg{
    print_func_t print_func;
    unsigned int poll;
};

#endif /* MEM_TRACKER_h */

