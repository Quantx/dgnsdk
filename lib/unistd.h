#ifndef UNISTD_H
#define UNISTD_H

// Define misc UNIX system calls and constants

#ifdef LINUX_COMPAT // Pretend this is a 16-bit unix 6 system
#define int8_t  char
#define int16_t short
#define int32_t int
#define int64_t long
#define NULL ((void *)0)
#define SBRKFAIL ((void *)-1)
#define CAST_NAME
#else // This is actually going to be compiled on a 16-bit system
#define int8_t char
#define int16_t int
#define int32_t long
// No 64 bit integer
#define NULL ((void * _)0)
#define SBRKFAIL ((void * _)-1)
#define CAST_NAME _
#endif

// Unix system calls
void exit(int status);
int creat(char * pathname, int mode);
int  open(char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
void * sbrk(int inc);
int brk(void * addr);
#endif
