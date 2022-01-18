#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    char temp[1];
    char *commd[32];
    int i, j = 0;
    for (i = 0; i < argc-1; i++) {
        commd[i] = argv[i+1];
    }
    commd[i] = malloc(512);
    while (read(0, temp, 1) == 1) {
       
        switch (temp[0]) {
        case ' ': {
            i++, j = 0;
            commd[i] = malloc(512);
            break;
        }

        case '\n': {
            int pid = fork();
            if (pid == 0) {
                exec(commd[0], commd);
                exit(0);
            }
            wait(0);
            for (int k = argc - 1; k <= i; k++) {
                memset(commd[k],0, 512);
                free(commd[k]);
            }
            i = argc - 1, j = 0;
            commd[i] = malloc(512);
            break;
        }

        default: {
            commd[i][j++] = temp[0];
            break;
        }
        }
    }
    exit(0);
}

