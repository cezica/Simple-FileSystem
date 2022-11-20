#include "fs.h"

struct superblock SB;
struct inode Inodes[MAX_INODES];
struct disk_block DB[MAX_DISK_BLOCKS];

/////////////////////ajutatoare
struct inode create_inode(uint32_t mode, uint32_t uid, uint32_t gid, uint32_t blocks)
{
    struct inode i;
    i.i_mode=mode;
    i.i_uid=uid;
    i.i_gid=gid;
    i.i_blocks=blocks;

    return i;
}

void create_disk_block(struct disk_block* db)
{
    db->data=calloc(1,DISK_BLOCKS_SIZE);
}

void create_superblock()
{
    SB.nr_blocks=MAX_DISK_BLOCKS;
    SB.nr_inodes=MAX_INODES;
    SB.nr_free_inodes=SB.nr_inodes;
    SB.nr_free_blocks=SB.nr_blocks;
    
    SB.i_bitmap=calloc(1,SB.nr_inodes);
    SB.b_bitmap=calloc(1,SB.nr_blocks);
}

//tabela inode si disk block-urile goale
void create_root()
{
    Inodes[0]=create_inode(777,0,0,0);
    SB.nr_free_inodes--;
    SB.i_bitmap[0]=1;

    for(int i=1;i<MAX_INODES;i++)
       Inodes[i]=create_inode(644,0,0,0);
}


struct superblock read_superblock(int fd)
{
    struct superblock SB2;
    read(fd,&SB2,sizeof(struct superblock));
    return SB2;

}
////////////////////
void create_fs()
{
    //alloc superblock
    create_superblock();

    //alloc inode for root
    create_root();
    
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
    int fd_fs=open("fs.data",O_RDWR,0777);

    if(fd_fs<0)
        perror("fd");
    
    //write superblock;
    write(fd_fs,&SB,sizeof(struct superblock));

    //write inodes
    for(int i=0;i<MAX_INODES;i++)
        write(fd_fs,&Inodes[i],sizeof(struct inode));
   
    //write disk blocks
    
    close(fd_fs);
}

void copyfrom(const char* file)
{
    int fd=open(file,O_RDONLY);
    int fd_fs=open("fs.data",O_WRONLY);
    struct stat about_file;
    fstat(fd,&about_file);

    //verificare inode liber
    int index=-1;
    int i=1;
    while(index==-1)
        if(SB.i_bitmap[i]==0)
        {
            index=i;
            SB.i_bitmap[i]=1;
        }

    //citire din fisier
    int size=about_file.st_size;
    char buff[size];
    read(fd,buff,size);

    //nr disk block-uri pentru file sys nostru
    int nr_blocks=size/DISK_BLOCKS_SIZE;
    if(size%DISK_BLOCKS_SIZE!=0)
        nr_blocks++;

    //populare tabela inodes
    Inodes[i]=create_inode(about_file.st_mode,about_file.st_uid,
                        about_file.st_gid,nr_blocks);


    //verificare diskblock liber -- pt fiecare diskblock necesar
    int offset_buff=0;
    for(int i=0;i<nr_blocks;i++)
    {
        index=-1;
        i=0;
        while(index==-1)
            if(SB.b_bitmap[i]==0)
            {
                index=i;
                SB.b_bitmap[i]=1;
            }
        //append la date
        int offset_fs=OFFSET+index*DISK_BLOCKS_SIZE;
        lseek(fd_fs,offset_fs,SEEK_SET);
        //scriere in diskblock
        write(fd_fs,buff+offset_buff,DISK_BLOCKS_SIZE);
        offset_buff+=DISK_BLOCKS_SIZE;
    }

    close(fd_fs);
    close(fd);
}

void main()
{
    create_fs();
    sync_fs();
    copyfrom("fis.txt");
    printf("succes!\n");
}
