
#include<ulib.h>
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int pid;
    char buf1[20];
    char *file1="test.txt";
    char *file2="test2.txt";

    char *buf = "Hello, I am file!";
    int fd_1 = open(file1, O_CREAT|O_RDWR, O_READ|O_WRITE);
    printf("%d\n",get_stats());
    
    int fd_2 = open(file1, O_READ); 
    printf("%d\n",get_stats());
    
    int fd_3 = open(file2, O_CREAT|O_RDWR, O_READ|O_WRITE);
    printf("%d\n",get_stats()); 
    
    // This will not create any file descriptor as the file with the name "test2.txt" was created already.
    int fd_4 = open(file2, O_CREAT|O_RDWR, O_READ|O_WRITE);
    printf("%d\n",get_stats()); 



    close(fd_1);
    printf("%d\n",get_stats()); 
    close(fd_2);
    printf("%d\n",get_stats()); 
    close(fd_3);
    //Below printf will be printing 3 because of Standard file descriptor(STDIN, STDOUT, STDERROR)
    printf("%d\n",get_stats());
    return 0;
}