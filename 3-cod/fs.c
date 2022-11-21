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

void create_disk_blocks()
{
    for(int i=0;i<SB.nr_blocks;i++)
    {
        DB[i].data=calloc(DISK_BLOCKS_SIZE, 1);
    }
}

void create_superblock()
{
    SB.nr_blocks=MAX_DISK_BLOCKS;
    SB.nr_inodes=MAX_INODES;
    SB.nr_free_inodes=SB.nr_inodes;
    SB.nr_free_blocks=SB.nr_blocks;
    
    SB.i_bitmap=calloc(SB.nr_inodes,1);
    SB.b_bitmap=calloc(SB.nr_blocks,1);
}

//tabela inode si disk block-urile goale
void create_root()
{
    Inodes[0]=create_inode(777,0,0,0);
    SB.nr_free_inodes--;
    SB.i_bitmap[0]=1;

    for(int i=1;i<MAX_INODES;i++)
       Inodes[i]=create_inode(0,0,0,0);
}

//pt hexa
int getNum(char ch)
{
    int num = 0;
    if (ch >= '0' && ch <= '9') {
        num = ch - 0x30;
    }
    else {
        switch (ch) {
        case 'A':
        case 'a':
            num = 10;
            break;
        case 'B':
        case 'b':
            num = 11;
            break;
        case 'C':
        case 'c':
            num = 12;
            break;
        case 'D':
        case 'd':
            num = 13;
            break;
        case 'E':
        case 'e':
            num = 14;
            break;
        case 'F':
        case 'f':
            num = 15;
            break;
        default:
            num = 0;
        }
    }
    return num;
}

//nr^a
int power(int nr,int a)
{
    if(a==0)return 1;
    for(int i=0;i<a;i++)
    {
        nr=nr*nr;
    }
    return nr;
}

unsigned int hex2int(unsigned char hex[])
{
    int number = (int)strtol(hex, NULL, 16);
    return number;
}

void initializare_block(char* info_block)
{
    char*p=strtok(info_block, " ");
    SB.nr_blocks=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_inodes=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_free_blocks=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_free_inodes=hex2int(p);
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
    create_disk_blocks();
}

void mount_fs()
{
    int fd_fs=open("fs.data",O_RDONLY);
    
    //Initializare superblock
    char buffer[101];
    read(fd_fs, buffer, 12);
    initializare_block(buffer);

    read(fd_fs, buffer, SB.nr_blocks+1);
    SB.b_bitmap=calloc(SB.nr_blocks,1);
    for(int i=0;i<SB.nr_blocks;i++)
        SB.b_bitmap[i]=buffer[i];

    read(fd_fs, buffer, SB.nr_inodes+1);
    SB.i_bitmap=calloc(SB.nr_inodes,1);
    for(int i=0;i<SB.nr_inodes;i++)
        SB.i_bitmap[i]=buffer[i];

    //Initializare si inode
    for(int i=0;i<SB.nr_inodes;i++)
    {
        if(SB.i_bitmap[i]=='1')
        {
            char c='0';
            int index=0;
            while(c!='|')
            {  
                read(fd_fs,buffer+index, 1);
                c=buffer[index++];
            }
            char*p=strtok(buffer, "/|");
            int aux=hex2int(p);
            Inodes[i].i_mode=aux;

            p=strtok(NULL, "/|");
            aux=hex2int(p);
            Inodes[i].i_uid=aux;

            p=strtok(NULL, "/|");
            aux=hex2int(p);
            Inodes[i].i_uid=aux;

            p=strtok(NULL, "/|");
            aux=hex2int(p);
            Inodes[i].i_uid=aux;
        }
        else
            Inodes[i]=create_inode(0,0,0,0);
    }


    //Initializare inodes

    close(fd_fs);
}

void sync_fs()
{
    //open - fs.data
    int fd_fs=open("fs.data",O_RDWR,0777);

    if(fd_fs<0)
        perror("fd");
    
    //write superblock;
    char hexstring[9];
    sprintf(hexstring, "%X", SB.nr_blocks);
    write(fd_fs,hexstring,strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_inodes);
    write(fd_fs,hexstring,strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_free_blocks);
    write(fd_fs,hexstring,strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_free_inodes);
    write(fd_fs,hexstring,strlen(hexstring));
    write(fd_fs, " ", 1);

   for(int i=0;i<SB.nr_blocks;i++)
   {
    sprintf(hexstring, "%X", SB.b_bitmap[i]);
    write(fd_fs,hexstring,strlen(hexstring));
   }
    write(fd_fs, " ", 1);

    for(int i=0;i<SB.nr_inodes;i++)
   {
    sprintf(hexstring, "%X", SB.i_bitmap[i]);
    write(fd_fs,hexstring,strlen(hexstring));
   }
    write(fd_fs, " ", 1);

    //write inodes
    // | intre inode-uri
    // / intre datele dintr-un inode
    for(int i=0;i<SB.nr_inodes;i++)
    {
        sprintf(hexstring, "%X", Inodes[i].i_mode);
        write(fd_fs,hexstring,strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_uid);
        write(fd_fs,hexstring,strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_gid);
        write(fd_fs,hexstring,strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_blocks);
        write(fd_fs,hexstring,strlen(hexstring));
        if(i!=SB.nr_inodes-1)
        write(fd_fs, "|", 1);
    }
   
    write(fd_fs, " ", 1);
    char hexablock[DISK_BLOCKS_SIZE*2+1];
    //write disk blocks
    for(int i=0;i<SB.nr_blocks;i++)
    {
        for(int j=0;j<DISK_BLOCKS_SIZE;j++)
            sprintf(hexablock+j*2, "%02X", DB[i].data[j]);
        write(fd_fs,hexablock,strlen(hexablock));
        if(i!=SB.nr_blocks-1)
            write(fd_fs, "|", 1);
    }

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
    {  
        if(SB.i_bitmap[i]==0)
        {
            index=i;
            SB.i_bitmap[i]=1;
        }
        i++;
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
    Inodes[index]=create_inode(about_file.st_mode,about_file.st_uid,
                        about_file.st_gid,nr_blocks);
    SB.nr_free_inodes--;

    //verificare diskblock liber -- pt fiecare diskblock necesar
    int offset_buff=0;
    for(int i=0;i<nr_blocks;i++)
    {
        index=-1;
        i=0;
        while(index==-1)
        {
            if(SB.b_bitmap[i]==0)
            {
                index=i;
                SB.b_bitmap[i]='1';
            }
            i++;
        }
        
            
        strncpy(DB[index].data,buff,DISK_BLOCKS_SIZE);
        SB.nr_free_blocks--;
        
    }

    sync_fs();


    close(fd_fs);
    close(fd);
}

void main()
{
    //create_fs();
    //sync_fs();
    //copyfrom("fis.txt");
    mount_fs();
    printf("succes!\n");
}
