#ifndef VFS_H
#define VFS_H

#define OPEN_MAX 19
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#include "multiboot.h"
#include "fs.h"
#include "common.h"

struct fdt_entry_struct {
	fs_node_t* file;
	int flags;
}fdt[OPEN_MAX];


int get_unused_fd();
int reserved(int i);
extern u32int placement_address;
void init_vfs(struct multiboot* mboot_ptr);	//init virtual file system
int open(char* pathname, int flags);
int read(int nFd, char* buf, int byte, int offset);
void fd_install(int fd, fs_node_t* file);

int close(int fd);
int write(int fd, char* buf, int size, int offset);

//void serialize_to_disk(u32int location);


#endif