#include "fs.h"

struct superblock SB;
struct inode Inodes[MAX_INODES];

int current_directory = 0;//root

/////////////////////creare
struct inode create_inode(char* name, int i_parent, uint32_t mode, uint32_t uid, uint32_t gid, uint32_t blocks, int size)
{
    struct inode i;
    i.i_mode = mode;
    i.i_uid = uid;
    i.i_gid = gid;
    i.i_blocks = blocks;
    i.i_size = size;

    strcpy(i.name, name);
    i.i_parent = i_parent;

    return i;
}

void create_superblock()
{
    SB.nr_blocks = MAX_DISK_BLOCKS;
    SB.nr_inodes = MAX_INODES;
    SB.nr_free_inodes = SB.nr_inodes;
    SB.nr_free_blocks = SB.nr_blocks;

    SB.i_bitmap = calloc(SB.nr_inodes, sizeof(int));
    SB.b_bitmap = calloc(SB.nr_blocks, sizeof(int));
}

void create_root()
{
    Inodes[0] = create_inode("root", -1, 40777, 0, 0, 0, 0);
    SB.nr_free_inodes--;
    SB.i_bitmap[0] = 1;

    for (int i = 1; i < MAX_INODES; i++)
        Inodes[i] = create_inode("", 0, 0, 0, 0, 0, 0);
}
//////////////////////////

////////////citire
int hex2int(unsigned char hex[])
{
    int number = (int)strtol(hex, NULL, 16);
    return number;
}

struct superblock read_superblock(int fd)
{
    struct superblock SB2;
    read(fd, &SB2, sizeof(struct superblock));
    return SB2;

}

unsigned char* read_data_for_db(char* buffer)
{
    char* buff = malloc(DISK_BLOCKS_SIZE + 1);
    int j = 0;
    for (int i = 0; i < DISK_BLOCKS_SIZE * 2; i += 2)
    {
        char cpy[2];
        cpy[0] = buffer[i];
        cpy[1] = buffer[i + 1];

        unsigned int to_int = hex2int(cpy);
        buff[j] = (char)to_int;
        j++;
    }

    return buff;
}

void citire_SB(int fd_fs)
{
    create_superblock();
    //create_disk_blocks();

    char buffer[101];
    read(fd_fs, buffer, 12);

    char* p = strtok(buffer, " ");
    SB.nr_blocks = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_inodes = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_free_blocks = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_free_inodes = hex2int(p);

    //initializare bitmap
    char buff[DISK_BLOCKS_SIZE];

    read(fd_fs, buff, SB.nr_blocks + 1);
    for (int i = 0; i < SB.nr_blocks; i++)
        SB.b_bitmap[i] = buff[i] - '0';

    read(fd_fs, buff, SB.nr_inodes + 1);
    for (int i = 0; i < SB.nr_inodes; i++)
        SB.i_bitmap[i] = buff[i] - '0';
}

void citire_Inodes(int fd_fs)
{
    char buffer[101];
    for (int i = 0; i < SB.nr_inodes; i++)
    {
        if(SB.i_bitmap[i]==1)
        {
            char c = '0';
            int index = 0;
            while (c != '|')
            {
                read(fd_fs, buffer + index, 1);
                c = buffer[index];
                index++;
            }
            char* p = strtok(buffer, "/|");
            unsigned char* temp = read_data_for_db(p);
            strcpy(Inodes[i].name, temp);

            p = strtok(NULL, "/|");
            int aux = hex2int(p);
            Inodes[i].i_parent = aux;

            p = strtok(NULL, "/|");
            aux = hex2int(p);
            Inodes[i].i_mode = aux;

            p = strtok(NULL, "/|");
            aux = hex2int(p);
            Inodes[i].i_uid = aux;

            p = strtok(NULL, "/|");
            aux = hex2int(p);
            Inodes[i].i_gid = aux;

            p = strtok(NULL, "/|");
            aux = hex2int(p);
            Inodes[i].i_size = aux;


            p = strtok(NULL, "/|");
            aux = hex2int(p);
            Inodes[i].i_blocks = aux;
        }
        else
        {
            read(fd_fs, buffer, 13);
            Inodes[i] = create_inode("", 0, 0, 0, 0, 0, 0);
        }
    }
}
////////////////////

/////////////scriere
void scriere_superblock(int fd_fs)
{
    lseek(fd_fs,0,0);
    char hexstring[9];
    sprintf(hexstring, "%X", SB.nr_blocks);
    write(fd_fs, hexstring, strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_inodes);
    write(fd_fs, hexstring, strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_free_blocks);
    write(fd_fs, hexstring, strlen(hexstring));
    write(fd_fs, " ", 1);

    sprintf(hexstring, "%X", SB.nr_free_inodes);
    write(fd_fs, hexstring, strlen(hexstring));
    write(fd_fs, " ", 1);

    char one = '1';
    char zero = '0';

    for (int i = 0; i < SB.nr_blocks; i++)
        if (SB.b_bitmap[i] == 1)
            write(fd_fs, &one, sizeof(one));
        else
            write(fd_fs, &zero, sizeof(zero));
    write(fd_fs, " ", 1);

    for (int i = 0; i < SB.nr_inodes; i++)
        if (SB.i_bitmap[i] == 1)
            write(fd_fs, &one, sizeof(one));
        else
            write(fd_fs, &zero, sizeof(zero));
    write(fd_fs, " ", 1);

}

void scriere_inodes(int fd_fs)
{
    char hexstring[9];
    // | intre inode-uri
    // / intre datele dintr-un inode
    for (int i = 0; i < SB.nr_inodes; i++)
    {
        for (int j = 0; j < strlen(Inodes[i].name); j++)
        {
            sprintf(hexstring, "%02X", (Inodes[i].name)[j]);
            write(fd_fs, hexstring, strlen(hexstring));
        }

        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_parent);
        write(fd_fs, hexstring, strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_mode);
        write(fd_fs, hexstring, strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_uid);
        write(fd_fs, hexstring, strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_gid);
        write(fd_fs, hexstring, strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_size);
        write(fd_fs, hexstring, strlen(hexstring));
        write(fd_fs, "/", 1);

        sprintf(hexstring, "%X", Inodes[i].i_blocks);
        write(fd_fs, hexstring, strlen(hexstring));
        if (i != SB.nr_inodes - 1)
            write(fd_fs, "|", 1);
    }

    write(fd_fs, " ", 1);
}

//citire db doar la scriere
void scriere_db(int fd_fs)
{
    char hexablock[DISK_BLOCKS_SIZE * 2 + 1];

//pentru ca root nu e fisier si mereu root va fi primul inode
    for(int i=1;i<SB.nr_inodes;i++)
    {
        if(SB.i_bitmap[i]==1&&Inodes[i].i_blocks>0)
        {
            struct disk_block DB[Inodes[i].i_blocks];

            int fd=open(Inodes[i].name,O_RDONLY);
            int size = Inodes[i].i_size;
            char buff[size];
            read(fd, buff, size);

                //nr disk block-uri pentru file sys nostru
            int nr_blocks = Inodes[i].i_blocks;

            int offset_buff = 0;
            for (int i_b = 0; i_b < nr_blocks; i_b++)
            {
                DB[i_b].data=calloc(1,DISK_BLOCKS_SIZE);
                i = -1;
                i_b = 0;
                while (i == -1)
                {
                    if (SB.b_bitmap[i] == 0)
                    {
                        i = i_b;
                        SB.b_bitmap[i_b] = 1;
                        strncpy(DB[i_b].data, buff, DISK_BLOCKS_SIZE);
                        int off=OFFSET+sizeof(struct disk_block)*i_b;
                        lseek(fd_fs,0,off);
                        for (int j = 0; j < DISK_BLOCKS_SIZE; j++)
                            sprintf(hexablock + j * 2, "%02X", DB[i_b].data[j]);
                        write(fd_fs, hexablock, strlen(hexablock));
                        if (i != SB.nr_blocks - 1)
                            write(fd_fs, "|", 1);
                        }

                        SB.nr_free_blocks--;
                    }
                    i++;
                }
                
            close(fd);
        }
        else{
            int off=OFFSET+sizeof(struct disk_block)*i;
            char*buff=malloc(DISK_BLOCKS_SIZE*2);
            for(int i=0;i<DISK_BLOCKS_SIZE*2;i++)
                buff[i]='0';
            lseek(fd_fs,0,off);
            write(fd_fs,buff,strlen(buff));
            write(fd_fs, "|", 1);
        }
    }
}
////////////////////////////

//////////////////copy_from
void inode_info(int fd, int index, char* filename)
{
    struct stat about_file;
    fstat(fd, &about_file);

    //populare tabela inodes
    //nr blocks pt fs-ul nostru
    int nr_blocks = about_file.st_size / DISK_BLOCKS_SIZE;
    if (about_file.st_size % DISK_BLOCKS_SIZE != 0)
        nr_blocks++;
    Inodes[index] = create_inode(filename, current_directory, 10755, about_file.st_uid,
        about_file.st_gid, nr_blocks, about_file.st_size);
    SB.nr_free_inodes--;
}

int index_inode_liber()
{
    int index = -1;
    int i = 1;
    while (index == -1)
    {
        if (SB.i_bitmap[i] == 0)
        {
            index = i;
            SB.i_bitmap[i] = 1;
        }
        i++;
    }
    return index;
}
//////////////////////


//////////////////////////////fs.data
void create_fs()
{
    create_superblock();

    create_root();
}

void mount_fs()
{
    int fd_fs = open("fs.data", O_RDONLY);

    //Initializare superblock
    citire_SB(fd_fs);

    //Initializare inodes
    citire_Inodes(fd_fs);

    close(fd_fs);
}

void sync_fs()
{
    //open - fs.data
    int fd_fs = open("fs.data", O_RDWR, 0777);

    if (fd_fs < 0)
        perror("fd");

    scriere_inodes(fd_fs);

    scriere_db(fd_fs);

    scriere_superblock(fd_fs);

    close(fd_fs);
}
/////////////////////////////////////


/////////////////////////////////////functii
void copyfrom(char* file)
{
    int fd = open(file, O_RDONLY);
    int fd_fs = open("fs.data", O_WRONLY);

    //verificare inode liber
    int index = index_inode_liber();
    inode_info(fd,index,file);

    close(fd_fs);
    close(fd);
}

void change_directory(char* nume_director)
{
    for (int i = 1; i < SB.nr_inodes; i++)
    {
        if(SB.i_bitmap[i]==1)
            if (Inodes[i].i_parent == current_directory && 
            strcmp(Inodes[i].name, nume_director) == 0)
                current_directory = i;
    }
}

void ls()
{
    for (int i = 0; i < SB.nr_inodes; i++)
    {
        if(SB.i_bitmap[i]==1)
            if (Inodes[i].i_parent == current_directory)
                printf("%s\t", Inodes[i].name);
    }
    printf("\n\n");
}

void create_directory(char* dir_name)
{
    int i=index_inode_liber();

    Inodes[i]=create_inode(dir_name,current_directory,Inodes[current_directory].i_mode,
    Inodes[current_directory].i_uid,Inodes[current_directory].i_gid,
    0,0);
}

void create_file(char* filename)
{
    int i=index_inode_liber();

    Inodes[i]=create_inode(filename,current_directory,
    10755,Inodes[current_directory].i_uid,Inodes[current_directory].i_gid,
    0,0);

}

void main()
{
    create_fs();
    copyfrom("fis.txt");
    create_directory("test_directory");
    //mount_fs();
    ls();

    change_directory("test_directory");
    create_file("file.txt");
    //mount_fs();
    ls();
    sync_fs();
}
