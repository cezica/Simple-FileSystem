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

    uint32_t nr_blocks; /* Total number of blocks (incl sb & inodes) */
    uint32_t nr_inodes; /* Total number of inodes */

    uint32_t nr_free_inodes; /* Number of free inodes */
    uint32_t nr_free_blocks; /* Number of free blocks */

    unsigned char *i_bitmap; //bitmap inodes
    unsigned char *b_bitmap; //bitmap disk blocks

    //+padding
};

struct inode{
    uint32_t i_mode;   // File mode 
    uint32_t i_uid;    // Owner id 
    uint32_t i_gid;    // Group id
    //uint32_t i_size;   // Size in bytes 
    uint32_t i_blocks; // Block count
    //uint32_t ei_block;  // Block with list of extents for this file 
};

struct disk_block{
    unsigned char* data;
    //uint32_t ee_block; /* first logical block extent covers */
    //uint32_t ee_len;   /* number of blocks covered*/
    //uint32_t ee_start; /* first physical block extent covers */
};

#define OFFSET (sizeof(struct superblock)+sizeof(struct inode)*MAX_INODES)

struct file{
    int inode;
    char name[256];
};

void create_fs(); //initializare fs
void mount_fs(); //incarcarea unui fs
void sync_fc(); //scriere fs
