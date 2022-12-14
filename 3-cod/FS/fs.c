#include "fs.h"

struct superblock SB;
struct inode Inodes[MAX_INODES];
struct disk_block DB[MAX_DISK_BLOCKS];

/////////////////////creare
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

void create_root()
{
    Inodes[0]=create_inode(777,0,0,0);
    SB.nr_free_inodes--;
    SB.i_bitmap[0]=1;

    for(int i=1;i<MAX_INODES;i++)
       Inodes[i]=create_inode(0,0,0,0);
}
//////////////////////////

////////////citire
unsigned int hex2int(unsigned char hex[])
{
    int number = (int)strtol(hex, NULL, 16);
    return number;
}

struct superblock read_superblock(int fd)
{
    struct superblock SB2;
    read(fd,&SB2,sizeof(struct superblock));
    return SB2;

}

unsigned char* read_data_for_db(char* buffer)
{
    char* buff=malloc(DISK_BLOCKS_SIZE+1);
    int j=0;
    for(int i=0;i<DISK_BLOCKS_SIZE*2;i+=2)
    {
        char cpy[2];
        cpy[0]=buffer[i];
        cpy[1]=buffer[i+1];

        unsigned int to_int=hex2int(cpy);
        buff[j]=(char)to_int;
        j++;
    }

    return buff;
}

void citire_SB(int fd_fs)
{
    create_superblock();
    create_disk_blocks();

    char buffer[101];
    read(fd_fs, buffer, 12);

    char*p=strtok(buffer, " ");
    SB.nr_blocks=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_inodes=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_free_blocks=hex2int(p);

    p=strtok(NULL, " ");
    SB.nr_free_inodes=hex2int(p);

    //initializare bitmap
    char buff[DISK_BLOCKS_SIZE];

    read(fd_fs, buff, SB.nr_blocks+1);
    for(int i=0;i<SB.nr_blocks;i++)
        SB.b_bitmap[i]=buff[i]-'0';

    read(fd_fs, buff, SB.nr_inodes+1);
    for(int i=0;i<SB.nr_inodes;i++)
        SB.i_bitmap[i]=buff[i]-'0';
}

void citire_Inodes(int fd_fs)
{
    char buffer[101];
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
        {//citire cu tot cu | 
            read(fd_fs,buffer,8);
            Inodes[i]=create_inode(0,0,0,0);
        }
    }
}

void citire_DB(int fd_fs)
{
    //pt a avea offsetul
    int offset=lseek(fd_fs,0,SEEK_CUR);
    for(int i=0;i<SB.nr_blocks;i++)
    {
        int off=offset;
        if(SB.b_bitmap[i]=='1')
            {
                //offsetul de unde incepe
                off+=i*DISK_BLOCKS_SIZE*2;//scrie 2*disk_block_size
                lseek(fd_fs,0,off);
                char buff[DISK_BLOCKS_SIZE*2+1];
                read(fd_fs,buff,DISK_BLOCKS_SIZE*2+1);//pt a lua si |
                
                DB[i].data=read_data_for_db(buff);
            }
    }
}
////////////////////

/////////////scriere
void scriere_superblock(int fd_fs)
{
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

    char one='1';
    char zero='0';

    for(int i=0;i<SB.nr_blocks;i++)
    if(SB.b_bitmap[i]==1)
        write(fd_fs,&one,sizeof(one));
    else
        write(fd_fs,&zero,sizeof(zero));
    write(fd_fs, " ", 1);

    for(int i=0;i<SB.nr_inodes;i++)
        if(SB.i_bitmap[i]==1)
            write(fd_fs,&one,sizeof(one));
        else
            write(fd_fs,&zero,sizeof(zero));
    write(fd_fs, " ", 1);

}

void scriere_inodes(int fd_fs)
{
    char hexstring[9];
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
}

void scriere_db(int fd_fs)
{
    char hexablock[DISK_BLOCKS_SIZE*2+1];
    for(int i=0;i<SB.nr_blocks;i++)
    {
        for(int j=0;j<DISK_BLOCKS_SIZE;j++)
            sprintf(hexablock+j*2, "%02X", DB[i].data[j]);
        write(fd_fs,hexablock,strlen(hexablock));
        if(i!=SB.nr_blocks-1)
            write(fd_fs, "|", 1);
    }
}
////////////////////////////

//////////////////copy_from
void inode_info(int fd,int index)
{
    struct stat about_file;
    fstat(fd,&about_file);

    //populare tabela inodes
    //nr blocks pt fs-ul nostru
    int nr_blocks=about_file.st_size/DISK_BLOCKS_SIZE;
    if(about_file.st_size%DISK_BLOCKS_SIZE!=0)
        nr_blocks++;
    Inodes[index]=create_inode(about_file.st_mode,about_file.st_uid,
                        about_file.st_gid,nr_blocks);
    SB.nr_free_inodes--;
}

int index_inode_liber()
{
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
    return index;
}

void populare_disk_blocks(int fd,int index)
{
    inode_info(fd,index);
    //citire din fisier
    int size=Inodes[index].i_size;
    char buff[size];
    read(fd,buff,size);

    //nr disk block-uri pentru file sys nostru
    int nr_blocks=Inodes[index].i_blocks;

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
}
//////////////////////


////////////////////////////////////////////////////
void create_fs()
{
    create_superblock();

    create_root();
    
    create_disk_blocks();
}

void mount_fs()
{
    int fd_fs=open("fs.data",O_RDONLY);
    
    //Initializare superblock
    citire_SB(fd_fs);
    
    //Initializare inodes
    citire_Inodes(fd_fs);

    //Initilizare disk block-uri
    citire_DB(fd_fs);

    close(fd_fs);
}

void sync_fs()
{
    //open - fs.data
    int fd_fs=open("fs.data",O_RDWR,0777);

    if(fd_fs<0)
        perror("fd");
    
    //write superblock;
    scriere_superblock(fd_fs);

    //write inodes
    scriere_inodes(fd_fs);
    
    //write disk blocks
    scriere_db(fd_fs);

    close(fd_fs);
}

void copyfrom(const char* file)
{
    int fd=open(file,O_RDONLY);
    int fd_fs=open("fs.data",O_WRONLY);

    //verificare inode liber
    int index=index_inode_liber();
    
    //disk block-uri
    populare_disk_blocks(fd,index);

    sync_fs();

    close(fd_fs);
    close(fd);
}

void main()
{
    //create_fs();
    //sync_fs();
    //copyfrom("fis.txt");
    //mount_fs();
}
