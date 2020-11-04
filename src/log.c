#include "log.h"

#include <stdio.h>
#include <unistd.h>

void progress_bar(float progress) { 
    int length = 50;

    for (int i = 0; i < length * progress; i++)
        printf("\u2588"); // █

    for (int i = length * progress; i < length; i++)
        printf("\u2581"); // ▁

    printf(" %.2f%% Complete\n", progress * 100);
} 
