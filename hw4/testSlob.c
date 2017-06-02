#include <stdio.h>
#include <unistd.h>

#define get_free syscall(353)
#define get_claimed syscall(354)

int main() {
        float fragmented;
        printf("Our syscalls for project 4...\n");

        fragmented = (float)get_free / (float)get_claimed;
        printf("Free Memory: %lu bytes\n", get_free);
        printf("Claimed Memory: %lu bytes\n", get_claimed);
        printf("Fragmented: %f\n", fragmented);

        return 0;
}
