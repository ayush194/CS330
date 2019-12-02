//CS330 Assignment 2
//Name : Ayush Kumar
//Roll No : 170195

#include<types.h>
#include<context.h>
#include<file.h>
#include<lib.h>
#include<serial.h>
#include<entry.h>
#include<memory.h>
#include<fs.h>
#include<kbd.h>
#include<pipe.h>


/************************************************************************************/
/***************************Do Not Modify below Functions****************************/
/************************************************************************************/
void free_file_object(struct file *filep)
{
		if(filep)
		{
			 os_page_free(OS_DS_REG ,filep);
			 stats->file_objects--;
		}
}

struct file *alloc_file()
{
	
	struct file *file = (struct file *) os_page_alloc(OS_DS_REG); 
	file->fops = (struct fileops *) (file + sizeof(struct file)); 
	bzero((char *)file->fops, sizeof(struct fileops));
	stats->file_objects++;
	return file; 
}

static int do_read_kbd(struct file* filep, char * buff, u32 count)
{
	kbd_read(buff);
	return 1;
}

static int do_write_console(struct file* filep, char * buff, u32 count)
{
	struct exec_context *current = get_current_ctx();
	return do_write(current, (u64)buff, (u64)count);
}

struct file *create_standard_IO(int type)
{
	struct file *filep = alloc_file();
	filep->type = type;
	if(type == STDIN)
		 filep->mode = O_READ;
	else
			filep->mode = O_WRITE;
	if(type == STDIN){
				filep->fops->read = do_read_kbd;
	}else{
				filep->fops->write = do_write_console;
	}
	filep->fops->close = generic_close;
	filep->ref_count = 1;
	return filep;
}

int open_standard_IO(struct exec_context *ctx, int type)
{
	 int fd = type;
	 struct file *filep = ctx->files[type];
	 if(!filep){
				filep = create_standard_IO(type);
	 }else{
				 filep->ref_count++;
				 fd = 3;
				 while(ctx->files[fd])
						 fd++; 
	 }
	 ctx->files[fd] = filep;
	 return fd;
}
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/



void do_file_fork(struct exec_context *child)
{
	 /*TODO the child fds are a copy of the parent. Adjust the refcount*/
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (child->files[i]) {
			child->files[i]->ref_count++;
		}
	}
}

void do_file_exit(struct exec_context *ctx)
{
	 /*TODO the process is exiting. Adjust the ref_count
		 of files*/
	//close all files
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (ctx->files[i]) {
			generic_close(ctx->files[i]);
		}
	}
}

long generic_close(struct file *filep)
{
	/** TODO Implementation of close (pipe, file) based on the type 
	 * Adjust the ref_count, free file object
	 * Incase of Error return valid Error code 
	 */
		if (!filep) {
			return -EINVAL;
		}
		if (filep->type == PIPE) {
			//this file is actually a pipe
		    if (!filep->pipe) {
		        return -EINVAL;
		    }
		    if (filep->mode == O_READ) {
		        //this is the read end of the pipe
		        filep->ref_count--;
		        if (filep->ref_count == 0) {
		            //all references to this file have been closed, hence free the file and close the read end of pipe
		            filep->pipe->is_ropen = 0;
		            if (!filep->pipe->is_wopen) {
		                free_pipe_info(filep->pipe);
		            }
		            free_file_object(filep);
		        }
		    } else if (filep->mode == O_WRITE) {
		        //this is the wite end of the pipe
		        filep->ref_count--;
		        if (filep->ref_count == 0) {
		            //all references to this file have been closed, hence free the file and close the write end of pipe
		            filep->pipe->is_wopen = 0;
		            if (!filep->pipe->is_ropen) {
		                free_pipe_info(filep->pipe);
		            }
		            free_file_object(filep);
		        }
		    }			
		} else if (filep->type == REGULAR) {
			//just a regular file
			filep->ref_count--;
			if (filep->ref_count == 0) {
				//all file descriptors for this file have been closed, hence close this file
				free_file_object(filep);
			}
		} else {
			return -EINVAL;
		}
		
		//return 0 on successful execution
		return 0;
}

static int do_read_regular(struct file *filep, char * buff, u32 count)
{
	 /** TODO Implementation of File Read, 
		*  You should be reading the content from File using file system read function call and fill the buf
		*  Validate the permission, file existence, Max length etc
		*  Incase of Error return valid Error code 
		* */
		//since count is of type u32, it is unsigned and hence is always >= 0
		if (!filep || !buff) {
			return -EINVAL;
		}
		if (!(filep->mode & O_READ)) {
			return -EACCES;
		}
		//note that flat_read automatically handles the case when count is greater than inode->file_size
		//if count is more than inode->file_size, it will only read data upto that
		int bytes_read = flat_read(filep->inode, buff, count, &filep->offp);
		//advance the seek position by the appropriate value
		filep->offp += bytes_read;
		return bytes_read;
}


static int do_write_regular(struct file *filep, char * buff, u32 count)
{
		/** TODO Implementation of File write, 
		*   You should be writing the content from buff to File by using File system write function
		*   Validate the permission, file existence, Max length etc
		*   Incase of Error return valid Error code 
		* */
		//since count is of type u32, it is unsigned and hence is always >= 0
		if (!filep || !buff) {
			return -EINVAL;
		}
		if (!(filep->mode & O_WRITE)) {
			return -EACCES;
		}
		//note that flat_write automatically handles the case when count is greater than max capacity
		//if count is more than max capacity, it will only write data upto max capacity and
		//adjust filep->inode->file_size acordingly
		int bytes_written = flat_write(filep->inode, buff, count, &filep->offp);
		if (bytes_written < 0) {
			return -EINVAL;
		}
		//advance the seek position by the appropriate value
		filep->offp += bytes_written;
		return bytes_written;
}

static long do_lseek_regular(struct file *filep, long offset, int whence)
{
		/** TODO Implementation of lseek 
		*   Set, Adjust the ofset based on the whence
		*   Incase of Error return valid Error code 
		* */
		if (!filep || filep->type != REGULAR) {
			return -EINVAL;
		}
		if (!(filep->mode & O_RDWR)) {
			return -EACCES;
		}
		switch (whence) {
			case SEEK_SET : {
				if (0 <= offset && offset < FILE_SIZE) {
					filep->offp = offset;
				} else {
					return -EINVAL;
				}
				break;
			}
			case SEEK_CUR : {
				if (0 <= filep->offp + offset && filep->offp + offset < FILE_SIZE) {
					filep->offp += offset;
				} else {
					return -EINVAL;
				}
				break;
			}
			case SEEK_END : {
				if (0 <= filep->inode->file_size + offset && filep->inode->file_size + offset < FILE_SIZE) {
					filep->offp = filep->inode->file_size + offset;
				} else {
					return -EINVAL;
				}
				break;
			}
			default : {
				return -EINVAL;
			} 
		}
		return filep->offp;
}

extern int do_regular_file_open(struct exec_context *ctx, char* filename, u64 flags, u64 mode)
{ 
	/**  TODO Implementation of file open, 
		*  You should be creating file(use the alloc_file function to creat file), 
		*  To create or Get inode use File system function calls, 
		*  Handle mode and flags 
		*  Validate file existence, Max File count is 32, Max Size is 4KB, etc
		*  Incase of Error return valid Error code 
		* */
		if (!ctx || !filename) {
			return -EINVAL;
		}
		struct inode* open_inode = lookup_inode(filename);
		struct file* open_file;
		int ret_fd;
		//ensure mode only contains read, write and exec bits
		//mode = mode & (O_READ | O_WRITE | O_EXEC);
		//check if the file needs to be created
		if (!open_inode) {
			//file needs to be created
			//check if the create flag is passed
			if (flags & O_CREAT) {
				//create a new file
				open_inode = create_inode(filename, mode);
				if (!open_inode) {
					return -ENOMEM;
				}
			} else {
				return -EINVAL;
			}
		} else {
			//inode already exists
			if (flags & O_CREAT) {
				return -EINVAL;
			}
		}
		//file was found or has been created
		//check if the access mode is compatible with the creation mode of the inode
		if ((flags&(~O_CREAT) | open_inode->mode) == open_inode->mode) {
			//access mode is compatible
		} else {
			//invalid access mode
			return -EACCES;
		}
		//open the file
		open_file = alloc_file();
		if (!open_file) {
			return -ENOMEM;
		}
		for(int i = 3; i < MAX_OPEN_FILES; i++) {
			if (!ctx->files[i]) {
				//this descriptor is empty
				ctx->files[i] = open_file;
				ret_fd = i;
				break;
			}
			if (i == MAX_OPEN_FILES - 1) {
				//file descriptor table is full
				return -EOTHERS;
			}
		}
		//set the different fields of file struct
		open_file->type = REGULAR;
		open_file->mode = flags&(~O_CREAT);
		open_file->offp = 0;
		open_file->ref_count = 1;

		open_file->inode = open_inode;
		
		open_file->fops->read = &do_read_regular;
		open_file->fops->write = &do_write_regular;
		open_file->fops->close = &generic_close;
		open_file->fops->lseek = &do_lseek_regular;
		
		open_file->pipe = NULL;
		return ret_fd;
}

int fd_dup(struct exec_context *current, int oldfd)
{
		 /** TODO Implementation of dup 
			*  Read the man page of dup and implement accordingly 
			*  return the file descriptor,
			*  Incase of Error return valid Error code 
			* */
		int newfd;
		if (!current || oldfd < 0 || oldfd >= MAX_OPEN_FILES || !current->files[oldfd]) {
			return -EINVAL;
		}
		for(int i = 0; i < MAX_OPEN_FILES; i++) {
			if (!current->files[i]) {
				//assign this file descriptor to the old file pointer
				current->files[i] = current->files[oldfd];
				current->files[i]->ref_count++;
				newfd = i;
				break;
			}
			if (i == MAX_OPEN_FILES) {
				return -EOTHERS;
			}
		}
		return newfd;
}


int fd_dup2(struct exec_context *current, int oldfd, int newfd)
{
	/** TODO Implementation of the dup2 
		*  Read the man page of dup2 and implement accordingly 
		*  return the file descriptor,
		*  Incase of Error return valid Error code 
		* */
		if (!current || oldfd < 0 || oldfd >= MAX_OPEN_FILES || newfd < 0 || newfd >= MAX_OPEN_FILES || !current->files[oldfd]) {
			return -EINVAL;
		}
		if (newfd == oldfd) {
			return newfd;
		}
		if (current->files[newfd]) {
			generic_close(current->files[newfd]);
		}
		current->files[newfd] = current->files[oldfd];
		current->files[newfd]->ref_count++;
		return newfd;
}
