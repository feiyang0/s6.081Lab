
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// int sleep(int);
int main(int argc, char *argv[]){
    if(argc<=1){
        fprintf(2,"error\n");
        exit(1);
    }
    else{
        int n=atoi(argv[1]);
        fprintf(2,"(nothing happens for a little while)\n");
        sleep(n);
    }
    exit(0);
}