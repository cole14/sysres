#ifndef TRACKER_H
#define TRACKER_H

#include "print_funcs.h"

typedef void *(*info_func_t)(print_func_t print_func, double print_threshold);

void *mem_info_func(print_func_t print_func, double print_threshold);
void *cpu_info_func(print_func_t print_func, double print_threshold);

#endif /* TRACKER_H */

