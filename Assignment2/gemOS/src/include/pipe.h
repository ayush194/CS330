#ifndef __PIPE_H_
#define __PIPE_H_

#include "types.h"
#include "context.h"
#include "file.h"


#define PIPE_MAX_SIZE 4096

extern int create_pipe(struct exec_context *current, int *fd);
extern void free_pipe_info(struct pipe_info *p_info);

#endif
