#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "print_funcs.h"

void *mem_tracker(void *arg);
void restore_term();

struct termios old_settings;

//Long command-line options
static struct option long_options[] = {
    {"help", 0, 0, 'h'},
    {"graphical", 0, 0, 'g'},
    {"poll", 1, 0, 'p'},
    {0, 0, 0, 0}
};

struct tracker_arg{
    print_func_t print_func;
    unsigned int poll;
};

void usage(){
    fprintf(stderr, "Usage: mem_lookup [OPTIONS]\n"
                    "Display memory usage\n\n"
                    " Options are:\n");
    fprintf(stderr, "  -g%-22s display percent free in a graphical manner\n", ", --graphical");
    fprintf(stderr, "  -p%-22s repeatedly poll the memory usage every 'M' seconds\n", ", --poll=M");
    fprintf(stderr, "  -h%-22s print usage\n\n", ", --help");
    fprintf(stderr, " Mandatory arguments for long options are for short options as well.\n");
    fprintf(stderr, " When in polling mode press 'q' to quit.\n");
}

int main(int argc, char *argv[]){
    char buf;
    char opt;
    long int poll_time;
    struct tracker_arg *track;

    if(!(track = malloc(sizeof(struct tracker_arg)))){
        fprintf(stderr, "Out of memory!\n");
        exit(ENOMEM);
    }

    track->print_func = print_free_default;
    track->poll = 0;

    // Parse command-line args
    while(-1 != (opt = getopt_long(argc, argv, "gp:h", long_options, NULL))){
        switch(opt){
            case 'g':
                track->print_func = print_free_visual;
                break;
            case 'p':
                poll_time = strtol(optarg, NULL, 10);
                if(poll_time <= 0){
                    fprintf(stderr, "Polling time %ld invalid!\n", poll_time);
                    usage();
                    exit(EINVAL);
                }
                if(poll_time >= UINT_MAX){
                    fprintf(stderr, "Polling time of %ld seconds is too long!\n", poll_time);
                    usage();
                    exit(EINVAL);
                }
                track->poll = (unsigned int)poll_time;
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
}

void restore_term(){
    tcsetattr(0, TCSANOW, &old_settings);
}

void *mem_tracker(void *arg){
    long phys_pages = 0;
    long avail_phys_pages, p_avail_phys_pages = 0;

    // Used in printing the time
    time_t t = 0;
    char * t_str = NULL;

    // Tracks the percent of memory which is free
    double percent = 0.0;

    // Options
    struct tracker_arg track = *(struct tracker_arg *)arg;

    phys_pages = sysconf(_SC_PHYS_PAGES);
    avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

    fprintf(stdout, "Page Size:\t%ld\n", sysconf(_SC_PAGESIZE));
    fprintf(stdout, "Total Physical Memory:\t%ld pages\t(%lu bytes)\n", 
            phys_pages, (uint64_t)phys_pages * (uint64_t)sysconf(_SC_PAGESIZE));
    fprintf(stdout, "Total Available Memory:\t%ld pages\t(%lu bytes)\n\n",
            avail_phys_pages, (uint64_t)avail_phys_pages * (uint64_t)sysconf(_SC_PAGESIZE)); 

    do{
        avail_phys_pages = sysconf(_SC_AVPHYS_PAGES);

        long diff = avail_phys_pages - p_avail_phys_pages;
        diff = (diff < 0) ? -diff : diff;
        if((double)diff / (double)phys_pages > 0.01){// Only print if a change greater than 1%
            // Get the current time
            time(&t);
            t_str = ctime(&t);
            t_str[strlen(t_str)-1] = 0;

            percent = 100*((double)avail_phys_pages/phys_pages);
            track.print_func(t_str, percent);

            p_avail_phys_pages = avail_phys_pages;
        }

        sleep(track.poll);
    }while(track.poll);

    return 0;
}

