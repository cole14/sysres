#include <unistd.h> //usleep

#include "tracker.h"

void *tracker(void *arg){
    struct tracker_arg *track = (struct tracker_arg *)arg;

    do{
        track->info_func(track);
        usleep(track->poll);
    }while(track->poll);

    return NULL;
}

