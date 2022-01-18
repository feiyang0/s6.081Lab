#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void print(int *fd1,int *fd2){

    int pid = fork();
    if(pid==-1) {
        printf("creat fork error\n");
        exit(1);
    }
        
    if(pid==0){
        int integer[1];
        if(read(fd1[0],integer,1)==0){
            close(fd1[0]);
            close(fd2[0]);close(fd2[1]);
            exit(0);
        }
        int p=integer[0];
        printf("prime %d\n",p);
        while(read(fd1[0],integer,1)!=0){
            if(integer[0]%p!=0){
                write(fd2[1],integer,1);
            }
        }
        close(fd2[1]);
        close(fd1[0]);
        pipe(fd1);
        print(fd2,fd1);

    }else {
        close(fd1[0]);
        close(fd2[0]);close(fd2[1]);
        wait(0);
        exit(0);
    }
}
// read:0,write:1
int main(int argc,char *argv[]){
    if(argc>1){
        fprintf(2,"input error\n");
        exit(1);
    }
    int fd1[2];
    int fd2[2];
    int res1=pipe(fd1),res2=pipe(fd2);
    if(res1==-1 || res2==-1){
        fprintf(2,"pipe create error\n");
        exit(1);
    }
    
    for(int i=2;i<=35;i++){
        write(fd1[1],&i,1);
    }
    close(fd1[1]);
    print(fd1,fd2);
    exit(0);
}