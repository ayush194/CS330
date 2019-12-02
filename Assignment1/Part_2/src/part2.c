//CS330 Assignment 1
//part2.c
//Name : Ayush Kumar
//Roll No : 170195

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

void at(const char* search_string, const char* search_path) {
	if (fork() == 0) {
		int fd[2];
		pipe(fd);
		//child process
		if (fork() == 0) {
			//child of child process
			//need to close the read end of the pipe since it is not required
			close(fd[0]);
			//close stdout and dup(fd[1]) so that when exec() is called, the output gets written into the pipe
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
			execl("/bin/grep", "grep", "-rF", search_string, search_path, (char*)NULL);
			//child of child process destroyed
		}
		//wait for the child (of child) process to finish
		wait((int*)NULL);
		//close the write end of the pipe
		close(fd[1]);
		//close stdin and dup read end of the file
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		execl("/bin/wc", "wc", "-l", (char*)NULL);
		//child process destroyed
	}
	//parent process
}

void dollar(const char* search_string, const char* search_path, char* outfile, char** argv) {
	//read end of the pipe at fd[0] and write end at fd[1]
	if (fork() == 0) {
		int fd1[2];
		pipe(fd1);
		int fd2[2];
		pipe(fd2);
		//child process
		if (fork() == 0) {
			//child of child process
			//need to close the read end of fd1 since only write will be performed by this child
			close(fd1[0]);
			//close stdout and dup(fd[1]) so that when exec() is called, the output gets written into the pipe
			dup2(fd1[1], STDOUT_FILENO);
			close(fd1[1]);
			execl("/bin/grep", "grep", "-rF", search_string, search_path, (char*)NULL);
			//child of child process destroyed
		}

		//simulating tee command
		//close write end of fd1 since nothing will be written into it now
		//note that it is necessary to close the write end of fd1 because
		//when you read from fd1 and if the write end is not closed, it will
		//keep waiting for more data to be read since the write end is open
		close(fd1[1]);
		int fd_out = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, 0644);
		char c;
		while(read(fd1[0], &c, 1)) {
			write(fd2[1], &c, 1);
			write(fd_out, &c, 1);
		}
		//wait for the child (of child) to write into the pipe so that the child can read
		wait(NULL);
		//close the read end of fd1 since no more data to be read from fd1
		close(fd1[0]);
		//close the write end of fd2 since no more data will be written to it
		close(fd2[1]);
		//close the output file since no more operations will be performed with it
		close(fd_out);

		//close stdout and dup read end of fd2
		dup2(fd2[0], STDIN_FILENO);
		close(fd2[0]);
		char* bin_path = "/bin/";
		char* file = (char*)malloc(strlen(bin_path)+strlen(argv[0])+1);
		strcpy(file, bin_path);
		strcpy(file+strlen(bin_path), argv[0]);
		execvp(file, argv);
		//child process destroyed
	}
	//parent process
}

int main(int argc, char** argv) {
	//io check
	if (argc < 4) {
		fprintf(stderr, "Insufficient arguments!\n Format: ./pipes {@,$} Kanpur IITK [output.txt] [command]\n");
		return 0;
	}
	if (argv[1][0] == '@') {
		//printf("      %d\n", at(argv[2], argv[3]));
		at(argv[2], argv[3]);
	}
	else if (argv[1][0] == '$') {
		char *search_string = argv[2];
		char *search_path = argv[3];
		char *outfile = argv[4];
		int i;
		for(i = 0; i < argc-5; i++) {
			argv[i] = argv[i+5];
		}
		argv[i] = NULL;
		dollar(search_string, search_path, outfile, argv);
	} else {
		fprintf(stderr, "Unidentified Symbol! Only @ and $ operators supported!\n");
	}
	return 0;	
}