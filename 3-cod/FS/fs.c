##include "fs.h"

struct superblock SB;
struct inode Inodes[MAX_INODES];
struct disk_block DB[MAX_DISK_BLOCKS];

/////////////////////ajutatoare
void create_inode(uint32_t mode, uint32_t uid, uint32_t gid, uint32_t blocks)
{
    struct inode i;
    i.i_mode=mode;
    i._uid=uid;
    i.i_gid=gid;
    i.i_blocks=blocks;
}

void create_disk_block(struct disk_block* db)
{
    db->data=calloc(1,DISK_BLOCKS_SIZE);
}

void create_superblock()
{
    SB.nr_blocks=MAX_DISK_BLOCKS;
    SB.nr_inodes=MAX_INODES;
    SB.nr_free_inodes=SB.nr_inodes-1;
    SB.nr_free_blocks=SB.nr_blocks-1;
    
    SB.i_bitmap=calloc(1,SB.nr_inodes);
    SB.b_bitmap=calloc(1,SB.nr_blocks);
}

void create_root()
{

}

////////////////////
void create_fs()
{
    //alloc superblock
    create_superblock();

    //alloc inode for root
    

    //aloc disk_blocks

}

void mount_fs()
{
    int fd_fs=open("fs.data",O_RDONLY);

    close(fd_fs);
}

void sync_fs()
{
    //open - fs.data
    int fd_fs=open("fis.txt",O_RDWR,0777);

    if(fd_fs<0)
        perror("fd");
    
    //write superblock;
    write(fd_fs,&SB,sizeof(struct superblock));

    //write inodes
   
    //write disk blocks
    
    
    close(fd_fs);
}

void main()
{
    create_fs();
    sync_fs();
    printf("succes!\n");
}
