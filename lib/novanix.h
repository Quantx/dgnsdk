// Nova / Modern OS compatibility support

// Pretend this is a 16-bit unix 6 system
#define int short
#define NULL 0

// Unix system calls
void exit(int status);
int creat(const char * pathname, int mode);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
int lseek(int fd, int offset, int whence);
void * sbrk(int inc);
#define SBRKFAIL (void *)-1
