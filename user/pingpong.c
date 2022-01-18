#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

// int pipe(int*);

int main(int argc, char *argv[]) {
    if (argc > 1) {
        fprintf(2, "error\n");
        exit(1);
    }
    int fd1[2];
    int fd2[2];
    int res1 = pipe(fd1);
    int res2 = pipe(fd2);
    if (res1 == -1 || res2 == -1) {
        fprintf(2, "error");
        exit(1);
    }
    int r_fd1 = fd1[0];
    int w_fd1 = fd1[1];

    int r_fd2 = fd2[0];
    int w_fd2 = fd2[1];

    int pid = fork();
    if (pid == -1) {
        fprintf(2, "error");
        exit(1);
    }
    if (pid == 0) {
        char buf[64];
        read(r_fd2, buf, 4);     // 2
        fprintf(2, "%d: received %s\n", getpid(), buf);
        write(w_fd1, "pong", 4); // 4
    } else {
        write(w_fd2, "ping", 4); // 1
        char buf[64]; 
        read(r_fd1, buf, 4);     // 3
        fprintf(2, "%d: received %s\n", getpid(), buf);
    }
    close(r_fd1);
    close(w_fd1);
    close(r_fd2);
    close(w_fd2);
    exit(0);
}
