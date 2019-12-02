#ifndef __ULIB_H__
#define __ULIB_H__

#define NULL (void *)0
typedef unsigned int u32;
typedef 	 int s32;
typedef unsigned short u16;
typedef int      short s16;
typedef unsigned char u8;
typedef char	      s8;
typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

#if __x84_64__
typedef unsigned long u64;
typedef long s64
#else
typedef unsigned long long u64;
typedef long long s64;

#endif

typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

#define SYSCALL_EXIT        1
#define SYSCALL_GETPID      2
#define SYSCALL_EXPAND      4
#define SYSCALL_SHRINK      5
#define SYSCALL_ALARM       6
#define SYSCALL_SLEEP       7
#define SYSCALL_SIGNAL      8
#define SYSCALL_CLONE       9
#define SYSCALL_FORK        10
#define SYSCALL_STATS       11
#define SYSCALL_CONFIGURE   12
#define SYSCALL_PHYS_INFO   13
#define SYSCALL_DUMP_PTT    14
#define SYSCALL_OPEN        15
#define SYSCALL_READ        16
#define SYSCALL_WRITE       17
#define SYSCALL_PIPE        18
#define SYSCALL_DUPE        19
#define SYSCALL_DUPE2       20
#define SYSCALL_CLOSE       21
#define SYSCALL_LSEEK       22

struct os_stats{
                u64 swapper_invocations;
                u64 context_switches;
                u64 lw_context_switches;
                u64 ticks;
                u64 page_faults;
                u64 syscalls;
                u64 used_memory;
                u64 num_processes;
};

struct os_configs{
                u64 global_mapping;
                u64 apic_tick_interval;
                u64 debug;
                u64 adv_global; 
};

#define   O_READ 0x1
#define   O_RDONLY O_READ
#define   O_WRITE 0x2
#define   O_WRONLY O_WRITE
#define   O_RDWR (O_READ|O_WRITE)
#define   O_EXEC  0x4
#define   O_CREAT 0x8

#define CREATE_READ O_READ
#define CREATE_WRITE O_WRITE
#define CREATE_EXEC O_EXEC

#define EINVAL 1
#define EAGAIN 2
#define EBUSY 3
#define EACCES 4
#define ENOMEM 5

enum{
      SEEK_SET,
      SEEK_CUR,
      SEEK_END,
      MAX_SEEK,
};

extern int printf(char *format,...);

extern void exit(int);
extern int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);
extern void exit(int code);
extern long getpid();
extern long fork();
extern long get_stats();
extern long configure(struct os_configs *new_config);
extern long signal(int num, void *handler);
extern long sleep(int ticks);
extern long expand(unsigned size, int flags);
extern long clone(void (func)(void), long stack_addr);
long physinfo();
long dump_page_table(char *address);
extern int open(char * filename, int mode, ...);
extern int write(int fd, void * buf, int count);
extern int read(int fd, void * buf, int count);
extern int pipe(int fd[2]);
extern int dup(int oldfd);
extern int dup2(int oldfd, int newfd);
extern int close(int fd);
extern long lseek(int fd, long offset, int whence);
#endif
