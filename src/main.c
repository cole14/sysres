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

#include "mem_tracker.h"

void restore_term();

struct termios old_settings;

extern char *program_invocation_short_name;

//Long command-line options
static struct option long_options[] = {
    {"help", 0, 0, 'h'},
    {"graphical", 0, 0, 'g'},
    {"poll", 1, 0, 'p'},
    {"threshold", 1, 0, 't'},
    {0, 0, 0, 0}
};

void usage(){
    fprintf(stderr, "Usage: %s [OPTIONS]\n"
                    "Display memory usage\n\n"
                    " Options are:\n", program_invocation_short_name);
    fprintf(stderr, "  -g%-22s display percent free in a graphical manner\n", ", --graphical");
    fprintf(stderr, "  -p%-22s repeatedly poll the memory usage every 'M' seconds (Decimals permitted)\n", ", --poll=M");
    fprintf(stderr, "  -t%-22s percent change required to print the memory usage\n", ", --threshold=T");
    fprintf(stderr, "  -h%-22s print usage\n\n", ", --help");
    fprintf(stderr, " Mandatory arguments for long options are for short options as well.\n");
    fprintf(stderr, " When in polling mode press 'q' to quit.\n");
}

int main(int argc, char *argv[]){
    char buf;
    char opt;
    double poll_time;
    double threshold;
    struct tracker_arg *track;

    if(!(track = malloc(sizeof(struct tracker_arg)))){
        fprintf(stderr, "Out of memory!\n");
        exit(ENOMEM);
    }

    track->print_func = print_free_default;
    track->poll = 0;
    track->print_threshold = -1.0;

    // Parse command-line args
    while(-1 != (opt = getopt_long(argc, argv, "gp:t:h", long_options, NULL))){
        switch(opt){
            case 'g':
                track->print_func = print_free_visual;
                break;
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
                track->poll = (unsigned long)(poll_time);
                break;
            case 't':
                threshold = strtod(optarg, NULL);
                if(threshold >= 100.0 || threshold <= 0.0){
                    fprintf(stderr, "Printing threshold %.2lf invalid! Threshold must be between 0%% and 100%%\n", threshold);
                    usage();
                    exit(EINVAL);
                }
                track->print_threshold = threshold / 100.0;
                break;
            case 'h':
            case '?':
                usage();
            default:
                exit(EINVAL);
        }
    }

    // Spawn the memtracker thread
    pthread_t thr;
    pthread_create(&thr, NULL, mem_tracker, track);

    if(track->poll && isatty(0)){

        // Store the old terminal settings so we can restore them on exit
        atexit(restore_term);

        int result = tcgetattr(0, &old_settings);
        if(result < 0){
            perror("tcgetattr");
            exit(-1);
        }

        struct termios new_settings = old_settings;
        new_settings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
        new_settings.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
        result = tcsetattr(0, TCSANOW, &new_settings);
        if(result < 0){
            perror("tcsetattr");
            exit(-1);
        }

        while(1){
            if(-1 == read(0, &buf, 1)){
                perror("read");
                exit(errno);
            }
            if(buf == 'q'){
                exit(0);
            }
        }
    }else{
        pthread_join(thr, NULL);
    }

    return 0;
}

void restore_term(){
    tcsetattr(0, TCSANOW, &old_settings);
}

