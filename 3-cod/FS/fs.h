#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdint.h>

#define DISK_BLOCKS_SIZE 64//momentan mica pentru testari
#define MAX_NAME_FILE 255
#define MAX_DISK_BLOCKS 100

#define MAX_INODES 32

#define MAX_READ 4096

struct superblock{
    //uint32_t magic; /* Magic number */

    int nr_blocks; /* Total number of blocks (incl sb & inodes) */
    int nr_inodes; /* Total number of inodes */

    int nr_free_inodes; /* Number of free inodes */
    int nr_free_blocks; /* Number of free blocks */

    int* i_bitmap; //bitmap inodes
    int* b_bitmap; //bitmap disk blocks

    int current_dir;//index inode

    //+padding
};

struct inode{
    int i_mode;   // File mode 
    //int i_ctime;  //status changed
    //int i_mtime;  //status modification
    //int i_atime;  //status acces
    int i_uid;    // Owner id 
    int i_gid;    // Group id
    int i_size;   // Size in bytes 
    int i_blocks; // Blocks

    //char name[MAX_NAME_FILE];
    //int index_parent;
    //int* disk_blocks
};

struct disk_block{
    unsigned char* data;
};

#define OFFSET (sizeof(struct superblock)+sizeof(struct inode)*MAX_INODES)

struct file{
    int inode;
    char name[256];
};

void create_fs(); //initializare fs
void mount_fs(); //incarcarea unui fs
void sync_fc(); //scriere fs
