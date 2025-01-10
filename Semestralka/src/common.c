#define _POSIX_C_SOURCE 199309L
#include <time.h>    // Pre struct timespec a nanosleep
#include <stddef.h>  // Pre NULL
#include <unistd.h>

void sleep_microseconds(long microseconds) {
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}


