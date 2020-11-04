#include <stdio.h>
#include <unistd.h>

void progress_bar(float progress) { 
    int length = 50;

    printf("\r");

    for (int i = 0; i < length * progress; i++)
        printf("\u2588"); // █

    for (int i = length * progress; i < length; i++)
        printf("\u2581"); // ▁

    printf(" %.2f%% Complete", progress * 100);
} 

int main() {
    progress_bar(0.612312);
    progress_bar(0.12413);
    printf("\n");

    return 0;
}