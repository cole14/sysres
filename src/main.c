#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <limits.h>

#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>

#include "tracker.h"

extern char *program_invocation_short_name;

//Long command-line options
static struct option long_options[] = {
    {"help", 0, 0, 'h'},
    {"graphical", 0, 0, 'g'},
    {"poll", 1, 0, 'p'},
    {"threshold", 1, 0, 't'},
    {0, 0, 0, 0}
};

void usage() {
    fprintf(stderr, "Usage: %s [OPTIONS] [memory|cpu]\n"
                    "Display system resource usage\n\n"
                    " Options are:\n", program_invocation_short_name);
    fprintf(stderr, "  -g%-22s display resource utilization in a graphical manner\n", ", --graphical");
    fprintf(stderr, "  -p%-22s repeatedly poll the resource utilization every 'M' seconds (Decimals permitted)\n", ", --poll=M");
    fprintf(stderr, "  -t%-22s percent change required to print the resource utilization\n", ", --threshold=T");
    fprintf(stderr, "  -h%-22s print this usage statement\n\n", ", --help");
    fprintf(stderr, "  %-24s track the system's memory utilization\n", "memory");
    fprintf(stderr, "  %-24s track the system's cpu utilization\n", "cpu");
    fprintf(stderr, " Mandatory arguments for long options are for short options as well.\n\n");
}

int main(int argc, char *argv[]) {
    char opt;
    double poll_time;
    double threshold;

    print_func_t print_func = print_free_default;
    info_func_t info_func = cpu_info_func;
    unsigned long poll = 0;
    double print_threshold = -1.0;

    // Parse command-line args
    while(-1 != (opt = getopt_long(argc, argv, "gp:t:h", long_options, NULL))){
        switch(opt){
            //Display the resource utilization using ascii graphics
            case 'g':
                print_func = print_free_visual;
                break;
            //Set the polling delay in seconds
            case 'p':
                poll_time = strtod(optarg, NULL);
                if(poll_time <= 0.0){
                    fprintf(stderr, "Polling time %.2lf invalid! Polling time must be positive\n", poll_time);
                    usage();
                    exit(EINVAL);
                }
                poll_time *= 1000000.0;
                if(poll_time > (double)ULONG_MAX){
                    fprintf(stderr, "Polling time %.2lf invalid! Polling time is too large\n", poll_time);
                    usage();
                    exit(EINVAL);
                }
                poll = (unsigned long)(poll_time);
                break;
            //Set the print threshold 
            case 't':
                threshold = strtod(optarg, NULL);
                if(threshold >= 100.0 || threshold <= 0.0){
                    fprintf(stderr, "Printing threshold %.2lf invalid! Threshold must be between 0%% and 100%%\n", threshold);
                    usage();
                    exit(EINVAL);
                }
                print_threshold = threshold / 100.0;
                break;
            //Print the usage statement
            case 'h':
            case '?':
                usage();
            default:
                exit(EINVAL);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "You must specify which resource to track: 'memory' or 'cpu'\n");
        usage();
        exit(EINVAL);
    }

    //Determine which resource tracking we should begin with
    if (!strncmp(argv[optind], "cpu", strlen(argv[optind]) + 1)) {
        info_func = cpu_info_func;
    } else if (!strncmp(argv[optind], "memory", strlen(argv[optind]) + 1)) {
        info_func = mem_info_func;
    } else {
        fprintf(stderr, "You must specify which resource to track: 'memory' or 'cpu'\n");
        usage();
        exit(EINVAL);
    }

    do {
        info_func(print_func, print_threshold);
        usleep(poll);
    } while (poll);

    return 0;
}

