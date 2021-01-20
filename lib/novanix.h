// Nova / Modern OS compatibility support

// Pretend this is a 16-bit unix 6 system
#define int8_t  char
#define int16_t short
#define int32_t int
#define int64_t long
#define NULL 0

// Unix system calls
void exit(int status);
int creat(const char * pathname, int mode);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
void * sbrk(int inc);
int brk(void * addr);
#define SBRKFAIL (void *)-1
