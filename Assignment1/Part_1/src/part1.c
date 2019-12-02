//CS330 Assignment 1
//part1.c
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

int wordSearch(const char* search_string, const char* line) {
	//search if the word search_string is present in the line
	int n1 = strlen(search_string), n2 = strlen(line);
	for(int i = 0; i <= n2-n1; i++) {
		if (strncmp(search_string, line+i, n1) == 0) return 1;
	}
	return 0;
}

void grepfile(const char* search_string, const char* search_path, int single_file) {
	//assert that search_path points to a file
	int fd = open(search_path, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error! Cannot open file %s\n", search_path);
		return;
	} else {
		//search for the search_string in the file pointed by new_search_path
		char buf[100000], c;
		buf[0] = 0;
		int i = 0;
		while(read(fd, &c, 1)) {
			if (c == '\n') {
				buf[i] = 0;
				i = 0;
				if (wordSearch(search_string, buf)) {
					if (single_file) {
						//if single file is specified, do not prepend the filepath
						printf("%s\n", buf);
					} else {
						printf("%s:%s\n", search_path, buf);
					}
				}
			} else {
				buf[i] = c;
				i++;
			}
		}
		buf[i] = 0;
		if (buf[0] != 0) {
			if (wordSearch(search_string, buf)) {
				if (single_file) {
					//if single file is specified, do not prepend the filepath
					printf("%s\n", buf);
				} else {
					printf("%s:%s\n", search_path, buf);
				}
			}
		}		
	}
	close(fd);
}

void grep(const char* search_string, const char* search_path) {
	//check if the given search_path is actually a file
	struct stat statbuf;
   	stat(search_path, &statbuf);
	if (!S_ISDIR(statbuf.st_mode)) {
		//this is just a regular file
		grepfile(search_string, search_path, 1);
		return;
	}
	//open the directory pointed to by the search_path
	DIR* curr_dir = opendir(search_path);
	struct dirent* entry = NULL;
	while ((entry = readdir(curr_dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			//we do not have to search inside directories . and .. since they are pointers to
			//current and previous directories
			continue;
		}
		//look up all the entries in the current directory
		char* new_search_path = appendToPath(search_path, entry->d_name);
		if (entry->d_type == DT_DIR) {
			//this entry is a directory
			//recursively grep in this directory
			grep(search_string, new_search_path);
		} else {
			//this entry is a file
			//open the file and read it line by line to check the occurrence of search_string
			grepfile(search_string, new_search_path, 0);
		}
	}
	closedir(curr_dir);
}

int main(int argc, char** argv) {
	//io check
	if (argc < 3) {
		fprintf(stderr, "Insufficient arguments!\n Format: ./mygrep search_string search_path\n");
		return 0;
	}
	//argv[1] is the string to search
	//argv[2] is the path in which to search
	//call grep(argv[1], argv[2]);
	grep(argv[1], argv[2]);
	return 0;
}