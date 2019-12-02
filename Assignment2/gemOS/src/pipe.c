//CS330 Assignment 2
//Name : Ayush Kumar
//Roll No : 170195

#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>
/***********************************************************************
 * Use this function to allocate pipe info && Don't Modify below function
 ***********************************************************************/
struct pipe_info* alloc_pipe_info()
{
    struct pipe_info *pipe = (struct pipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);
    pipe ->pipe_buff = buffer;
    return pipe;
}


void free_pipe_info(struct pipe_info *p_info)
{
    if(p_info)
    {
        os_page_free(OS_DS_REG ,p_info->pipe_buff);
        os_page_free(OS_DS_REG ,p_info);
    }
}
/*************************************************************************/
/*************************************************************************/


int pipe_read(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Read the contect from buff (pipe_info -> pipe_buff) and write to the buff(argument 2);
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */
    if (!filep || !buff) {
        return -EINVAL;
    }
    if (filep->type != PIPE || !(filep->mode & O_READ) || !filep->pipe || !filep->pipe->is_ropen) {
        return -EACCES;
    }
    if (count < 0 || count > filep->pipe->buffer_offset) {
        return -EINVAL;
    }
    u32 i = 0;
    for(; i < count; i++) {
        /*
        if (filep->pipe->read_pos + i == filep->pipe->buffer_offset) {
            //reached the end of the buffer so no more data to read
            break;
        }
        */
        buff[i] = filep->pipe->pipe_buff[(filep->pipe->read_pos + i) % PIPE_MAX_SIZE];
    }
    filep->pipe->read_pos = (filep->pipe->read_pos + i) % PIPE_MAX_SIZE;
    filep->pipe->buffer_offset -= i;
    //return the number of bytes read
    return i;
}


int pipe_write(struct file *filep, char *buff, u32 count)
{
    /**
    *  TODO:: Implementation of Pipe Read
    *  Write the contect from   the buff(argument 2);  and write to buff(pipe_info -> pipe_buff)
    *  Validate size of buff, the mode of pipe (pipe_info->mode),etc
    *  Incase of Error return valid Error code 
    */
    if (!filep || !buff) {
        return -EINVAL;
    }
    if (filep->type != PIPE || !(filep->mode & O_WRITE) || !filep->pipe || !filep->pipe->is_wopen) {
        return -EACCES;
    }
    if (count < 0 || filep->pipe->buffer_offset + count > PIPE_MAX_SIZE) {
        return -EINVAL;
    }
    u32 i = 0;
    for(; i < count; i++) {
        /*
        if (filep->pipe->write_pos + i == PIPE_MAX_SIZE) {
            //buffer is full
            break;
        }
        */
        filep->pipe->pipe_buff[(filep->pipe->write_pos + i) % PIPE_MAX_SIZE] = buff[i];
    }
    filep->pipe->write_pos = (filep->pipe->write_pos + i) % PIPE_MAX_SIZE;
    filep->pipe->buffer_offset += i;
    //return the number of bytes written
    return i;
}

int create_pipe(struct exec_context *current, int *fd)
{
    /**
    *  TODO:: Implementation of Pipe Create
    *  Create file struct by invoking the alloc_file() function, 
    *  Create pipe_info struct by invoking the alloc_pipe_info() function
    *  fill the valid file descriptor in *fd param
    *  Incase of Error return valid Error code 
    */
    if (!current || !fd) {
        return -EINVAL;
    }
    //assign two empty file descriptors for the read and write ends
    int descr_idx = 0;
    for(int i = 3; i < MAX_OPEN_FILES; i++) {
        if (!current->files[i]) {
            //current->files[i] = pipe_file[descr_idx];
            fd[descr_idx] = i;
            descr_idx++;
            if (descr_idx == 2) break;
        }
        if (i == MAX_OPEN_FILES - 1) {
            //not enough available descriptors to assign the pipe
            return -EOTHERS;
        }
    }

    struct pipe_info* new_pipe = alloc_pipe_info();
    if (!new_pipe) {
        return -ENOMEM;
    }
    new_pipe->read_pos = 0;
    new_pipe->write_pos = 0;
    //note that pipe_file->pipe->pipe_buff has already been allocated the space by alloc_file_info()
    new_pipe->buffer_offset = 0;
    new_pipe->is_wopen = 1;
    new_pipe->is_ropen = 1;

    //associate two file descriptions pipe_file[0] and pipe_file[1] one for the read end and one for the write end
    //but both the descriptions will point to the same pipe_info
    struct file* pipe_file[2];
    pipe_file[0] = alloc_file();
    if (!pipe_file[0]) {
        //deaalocate the pipes
        free_pipe_info(new_pipe);
        return -ENOMEM;
    }
    pipe_file[1] = alloc_file();
    if (!pipe_file[1]) {
        //dealocate the pipes
        free_pipe_info(new_pipe);
        return -ENOMEM;
    }
    current->files[fd[0]] = pipe_file[0];
    current->files[fd[1]] = pipe_file[1];
    //set the attributes of the file struct
    for(int i = 0; i < 2; i++) {
        pipe_file[i]->type = PIPE;
        //pipe_file[i]->mode = O_RDWR;
        pipe_file[i]->offp = 0;
        pipe_file[i]->ref_count = 1;

        pipe_file[i]->inode = NULL;

        pipe_file[i]->fops->read = &pipe_read;
        pipe_file[i]->fops->write = &pipe_write;
        pipe_file[i]->fops->close = &generic_close;
        pipe_file[i]->fops->lseek = NULL;
    }
    pipe_file[0]->mode = O_READ;
    pipe_file[1]->mode = O_WRITE;
    pipe_file[0]->pipe = pipe_file[1]->pipe = new_pipe;
    //on successful execution, return 0
    return 0;
}

