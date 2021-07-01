#include"vfs.h"
#include"common.h"
#include"monitor.h"
#include"initrd.h"
#include"fs.h"

void init_vfs(struct multiboot* mboot_ptr) {
    ASSERT(mboot_ptr->mods_count > 0);
    u32int initrd_location = *((u32int*)mboot_ptr->mods_addr);
    u32int initrd_end = *(u32int*)(mboot_ptr->mods_addr + 4);
    placement_address = initrd_end;
    initialise_paging();
    fs_root = initialise_initrd(initrd_location);
    //open stdin, stdout ,stderr
     for (int i = 0; i < OPEN_MAX; i = i + 1) {
        if (!reserved(i)) {
            fdt[i].flags = 0;
        }
    }
}
int reserved(int i){
    if (i == STDERR_FILENO) {
        return 1;
    }
    if (i == STDIN_FILENO) {
        return 1;
    }
    if (i == STDOUT_FILENO) {
        return 1;
    }
    return 0;
}
int get_unused_fd() {
    for (int i = 0; i < OPEN_MAX; i = i + 1) {
        if (!reserved(i)) {
            if (fdt[i].flags == 0) {
                return i;
            }
        }
    }
    return -1;
}

void fd_install(int fd, fs_node_t* f) {
    fdt[fd].file = f;
    fdt[fd].flags = 1;
}

int open(char* pathname, int flags) {
    int i = 0;
    struct dirent* node = 0;
    while ((node = readdir_fs(fs_root, i)) != 0)
    {
//        monitor_write(node->name);
        if (strcmp(node->name, pathname) == 0) {
//	    monitor_write(node->name);
            fs_node_t* fsnode = finddir_fs(fs_root, node->name);

            if ((fsnode->flags & 0x7) == FS_DIRECTORY)
            {
                return -1;
            }
            int fd = get_unused_fd();
            fd_install(fd, fsnode);
            return fd;
        }
	i++;
    }
    return -1;
}

int read(int nFd, char* buf, int byte, int offset) {
    u32int sz = read_fs(fdt[nFd].file, offset, byte, buf);
    return sz;
}

int close(int nFd){
	if(fdt[nFd].flags == 0){
		return -1;
	}
	fdt[nFd].file = 0;
	fdt[nFd].flags = 0;
	return 0;
}

int write(int fd, char* buf,int bytes, int offset){
	u32int sz = write_fs(fdt[fd].file, offset, bytes, buf);
	return sz;
}

void serialize_to_disk(u32int location){
    initrd_header = (initrd_header_t *)location;	//count of files
    file_headers = (initrd_file_header_t *) (location+sizeof(initrd_header_t));		//files
    nroot_nodes = initrd_header->nfiles;

    // For every file...
    int i;
    initrd_file_header_t header;
    for (i = 0; i < initrd_header->nfiles; i++)
    {
	strcpy(&file_headers[i].name, root_nodes[i].name);
	file_headers[i].length = root_nodes[i].length;
	header = file_headers[root_nodes[i].inode];
        memcpy((u8int*)(file_headers[i].offset), (u8int*)header.offset, root_nodes[i].length);					// 3 скопировать содержимое
        file_headers[i].offset -= location;						//повторить в обратную сторону скорее всего этого не нужно делать или в самом конце 4
    }
}