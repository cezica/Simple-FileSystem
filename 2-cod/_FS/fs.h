#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdint.h>

/*
struct superblock{
    int nr_inodes;
    int nr_blocks;
    int size_blocks;
};
*/
struct superblock{
    uint32_t magic; /* Magic number */

    uint32_t nr_blocks; /* Total number of blocks (incl sb & inodes) */
    uint32_t nr_inodes; /* Total number of inodes */

    uint32_t nr_istore_blocks; /* Number of inode store blocks */
    uint32_t nr_ifree_blocks;  /* Number of inode free bitmap blocks */
    uint32_t nr_bfree_blocks;  /* Number of block free bitmap blocks */

    uint32_t nr_free_inodes; /* Number of free inodes */
    uint32_t nr_free_blocks; /* Number of free blocks */

    unsigned long *ifree_bitmap; /* In-memory free inodes bitmap */
    unsigned long *bfree_bitmap; /* In-memory free blocks bitmap */

    //+padding
};

/*
inode 
    -char name
    -int size
*/
struct inode{
    uint32_t i_mode;   // File mode 
    uint32_t i_uid;    // Owner id 
    uint32_t i_gid;    // Group id
    uint32_t i_size;   // Size in bytes 
    uint32_t i_ctime;  // Inode change time 
    uint32_t i_atime;  // Access time 
    uint32_t i_mtime;  // Modification time 
    uint32_t i_blocks; // Block count
    uint32_t i_nlink;  // Hard links count - nume->hardisk
    uint32_t ei_block;  // Block with list of extents for this file 
    char i_data[32]; // store symlink content -conectarea intre fis
};

/*
struct disk_block{
    int next_block_num;
    char data[512];
};
*/

struct disk_block{
/*
    uint32_t ee_block; //first logical block extent covers 
    uint32_t ee_len;   // number of blocks covered
    uint32_t ee_start; // first physical block extent covers 
*/
    ////sau
    int fd; /*file descriptor*/
    size_t bcount; /*block count*/
};

struct file{
    int inode;
    char name[256];
};

void create_fs(); //initializare fs
void mount_fs(); //incarcarea unui fs
void sync_fc(); //scriere fs