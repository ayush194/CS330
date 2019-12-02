#ifndef __FILE_H_
#define __FILE_H_
#include<context.h>

/*************************
 * Modes of the files
 *************************/
#define   O_READ 0x1
#define   O_WRITE 0x2
#define   O_RDWR (O_READ|O_WRITE)
#define   O_EXEC  0x4
#define   O_CREAT 0x8
/************************/

//Types of File objects
enum{
    STDIN,
    STDOUT,
    STDERR,
    REGULAR,
    PIPE,
    MAX_FILE_TYPE,
};

// Whence used for lseek system call
enum{
      SEEK_SET,
      SEEK_CUR,
      SEEK_END,
      MAX_SEEK,
};

/**
 * File structure
 * Always Create the file structure using  alloc_file() function in file.c
 */
struct file{
    u32 type;               // Type can be REGULAR, PIPE -> (You can use the above enum to set the type)
    u32 mode;               // Mode -> File/Pipe mode, READ, WRITE, WRITE_ONLY, READ_ONLY 
    u32 offp;               // offset -> Last read/write offset of the file
    u32 ref_count;          // Reference count of the file object

    
    struct inode * inode;   // Incase of Pipe, It will be null, 

    
    struct fileops * fops;  // Map the function call based on the type {PIPE, REGULAR}

   
    struct pipe_info * pipe;// Incase of Regular file, It will be null
};



/** Pipe information structure
 * Create pipe_info using the function alloc_pipe_info() in pipe.c 
 * 
 */
struct pipe_info{
    int read_pos;       //Last Read position
    int write_pos;      // Last Write position
    char* pipe_buff;    // pipe buffer At Most It should be of Size 4096 Bytes
    int buffer_offset;  // current buffer length
    int is_ropen;       // Flag to check pipe read end is open or not
    int is_wopen;       // Flag to check pipe write end is open or not
};


/** Standart File operation (function) Structure*/
struct fileops{
    int (*read)(struct file *filep, char * buff, u32 count);
    int (*write)(struct file *filep, char * buff, u32 count); 
    long (*lseek)(struct file *filep, long offset, int whence);
    long (*close)(struct file *filep);
};


//STDIO handlers and functions
//extern void free_file_object(struct file *filep);
extern struct file *alloc_file();
extern struct file* create_standard_IO(int);
extern int open_standard_IO(struct exec_context *ctx, int type);
extern void do_file_fork(struct exec_context *child);
extern void do_file_exit(struct exec_context *ctx);

//Reg file read and writ
extern int do_regular_file_open(struct exec_context *ctx, char *filename, u64 flags, u64 mode);

// Dup, Dup2
extern int fd_dup(struct exec_context *current, int oldfd);
extern int fd_dup2(struct exec_context *current, int oldfd, int newfd);
extern long generic_close(struct file *filep);
#endif
