
#include<ulib.h>
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int pid;
    char buf1[20];
    int pipes[2];
    char *filename="test.txt";
    
    int create_fd = open(filename, O_CREAT|O_RDWR, O_READ|O_WRITE);
    char *P_buf = "Hello, I am file!";
    printf("P:%d\n", write(create_fd, P_buf, 17));
    
    if(pipe(pipes) == -1)
    {
        return 0;
    }

    int fd1 = dup(create_fd);
    pid = fork();
    
    if(pid == 0){
        char *buf = "Hello, I am Cpipe!";

        close(pipes[0]);
        printf("C:%d\n", write(pipes[1], buf, 18));


        printf("C:%d\n",lseek(fd1, 0, SEEK_SET));
        printf("C:%d\n", write(fd1, buf, 18));
        printf("C:%d",lseek(fd1, 0, SEEK_CUR));


        close(pipes[1]);
        printf("C:%d\n", get_stats());
        close(fd1);
        exit(0);
    }else{

        sleep(50);
        int fd_dup2 = dup2(pipes[0], 15);

        printf("P:%d\n", get_stats());
        
        close(pipes[1]);
        printf("P:%d\n", read(fd_dup2, buf1, 18));
        printf("P:%s\n",buf1);

        char buf2[36];
        printf("P:%d\n", write(create_fd, buf1, 18));

        lseek(fd1, 0, SEEK_SET);

        printf("P:%d\n", read(create_fd, buf2, 35));
        buf1[35] = 0;
        printf("P:%s\n", buf2);

        close(pipes[0]);
        close(fd1);
        close(fd_dup2);
        close(create_fd);
        printf("P:%d\n",get_stats());
        return 0;
    }
}