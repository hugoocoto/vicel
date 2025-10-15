#include <pty.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int
main(int argc, char **argv)
{
        int amaster;
        int pid;
        if ((pid = forkpty(&amaster, NULL, NULL, NULL)) > 0) {
                execvp(argv[1], argv + 1);
                perror(argv[1]);
                exit(1);
        }
        return 0;
}
