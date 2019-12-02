//CS330 Assignment 1
//part3.c
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

char* appendToPath(const char* path, const char* name) {
	//takes a directory path as a string and appends the name of the next directory or file
	int n1 = strlen(path), n2 = strlen(name);
	if (path[n1-1] == '/') {
		//remove the trailing slash since it will be appended in the routine below
		n1--;
	}
	char* new_path = (char*)malloc((n1+n2+2)*sizeof(char));
	strcpy(new_path, path);
	new_path[n1] = '/';
	strcpy(new_path+n1+1, name);
	//null terminate the new string
	new_path[n1+n2+1] = '\0';
	return new_path;
}

long long childGetSize(const char* root) {
	//recursively get the size of the current directory
	long long size = 0;
	//open the directory pointed to by root
	DIR* curr_dir = opendir(root);
	//pop the first two entries of this directory since they are '.' and '..'
	//readdir(curr_dir);
	//readdir(curr_dir);
	struct dirent* entry = NULL;
	while ((entry = readdir(curr_dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			//we do not have to search inside directories . and .. since they are pointers to
			//current and previous directories
			continue;
		}
		char* entry_path = appendToPath(root, entry->d_name);
		if (entry->d_type == DT_DIR) {
			size += childGetSize(entry_path);
		} else {
			struct stat sbuf;
			stat(entry_path, &sbuf);
			size += sbuf.st_size;
		}
	}
	return size;
}

void getSize(const char* root) {
	long long size = 0;
	//open the directory pointed to by root
	DIR* curr_dir = opendir(root);
	struct dirent* entry = NULL;
	//dir_names stores the names of all sub-directories as well as the root directory
	//dir_sizes stores the size of the corresponding directories
	//dir_cnt is total count of all sub-directories including the root directory
	char* dir_names[100];
	long long dir_sizes[100];
	for(int i = 0; i < 100; i++) {
		dir_names[i] = (char*)malloc(100*sizeof(char));
	}
	strcpy(dir_names[0], root); 
	int dir_cnt = 1;
	//pop the first two entries of this directory since they are '.' and '..'
	//readdir(curr_dir);
	//readdir(curr_dir);
	int pid = 10;
	while ((entry = readdir(curr_dir)) != NULL) {
		//look up all the entries in the current directory
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			//we do not have to search inside directories . and .. since they are pointers to
			//current and previous directories
			continue;
		}
		char* entry_path = appendToPath(root, entry->d_name);
		if (entry->d_type == DT_DIR) {
			//this entry is a directory
			int fd[2];
			if (pipe(fd) == -1) {
				//this never happens
				perror("pipe");
				exit(-1);
			}
			if (pid != 0) {
				//only parent should call fork()
				pid = fork();
			}
			//the child process will start executing from here
			//note that both the child and parent are connected to the same pipe
			//child must close the read end and parent must close the write end
			//then the child can write to the pipe and the parent will be able to read it
			if (pid < 0 ) {
				//this never happens
				perror("fork");
				exit(-1);
			} else if (pid == 0) {
				//this is the child process
				//close the read end, since the child wants to communicate the size to the parent
				//so it will write to the write end
				close(fd[0]);
				//the child must find out the size of entry_path and write it to fd[1]
				long long* s = (long long*)malloc(sizeof(long long));
				*s = childGetSize(entry_path);
				write(fd[1], s, 8);
				//destory the child process using kill or exec something else
				kill(getpid(), SIGINT);
				//execl("/bin/echo", "echo", "killing child process", (char*)NULL);
			} else {
				//this is the parent process
				//wait for the child process to finish
				wait((int*)NULL);
				close(fd[1]);
				//close the write end, since it wants to listen to the child process
				long long* tmp = (long long*)malloc(sizeof(long long));
				read(fd[0], tmp, 8);
				strcpy(dir_names[dir_cnt], entry->d_name);
				dir_sizes[dir_cnt] = *tmp;
				size += *tmp;
				dir_cnt++;
			}
		} else {
			//this is a file whose size needs to be added to the size of the root
			struct stat sbuf;
			stat(entry_path, &sbuf);
			size += sbuf.st_size;
		}
	}
	//printf("pid : %d\n", pid);
	if (pid != 0) {
		//this is the parent process
		//set the size of the root directory
		dir_sizes[0] = size;
		//print the root directory and the sub-directories as well as their sizes
		for(int i = 0; i < dir_cnt; i++) {
			printf("%s %lld\n", dir_names[i], dir_sizes[i]);
		}
		closedir(curr_dir);
	}
}

int main(int argc, char** argv) {
	//io check
	if (argc < 2) {
		fprintf(stderr, "Insufficient arguments!\n Format: ./directory_size root_directory\n");
		return 0;
	}
	getSize(argv[1]);
	return 0;
}