#include "fs.h"

struct superblock SB;
struct inode Inodes[MAX_INODES];
struct disk_block DB[MAX_DISK_BLOCKS];

int current_directory = 0; // root

char *command[256];

#define EXIT -1
#define LS 10
#define CD 11
#define MKDIR 12
#define CP 13
#define FIND 14
#define CAT 15
#define COMANDA_INCORECTA 0

/////////////////////erori
int error(char *msj)
{
    printf("%s\n", msj);
    exit(EXIT_FAILURE);
}
//////////////////////

/////////////////////creare
struct inode create_inode(char *name, int i_parent, uint32_t mode, uint32_t uid, uint32_t gid, uint32_t blocks, int size)
{
    struct inode i;
    i.i_mode = mode;
    i.i_uid = uid;
    i.i_gid = gid;
    i.i_blocks = blocks;
    i.i_size = size;
    i.index_blocks = (int *)malloc(sizeof(int) * 0);

    strcpy(i.name, name);
    i.i_parent = i_parent;

    return i;
}

void create_disk_blocks()
{
    for (int i = 0; i < SB.nr_blocks; i++)
    {
        DB[i].data = calloc(DISK_BLOCKS_SIZE, 1);
    }
}

void create_superblock()
{
    SB.nr_blocks = MAX_DISK_BLOCKS;
    SB.nr_inodes = MAX_INODES;
    SB.nr_free_inodes = SB.nr_inodes;
    SB.nr_free_blocks = SB.nr_blocks;

    SB.i_bitmap = calloc(SB.nr_inodes, 4);
    SB.b_bitmap = calloc(SB.nr_blocks, 4);
}

void create_root()
{
    Inodes[0] = create_inode("root", -1, 16877, 0, 0, 0, 0);
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

unsigned char *read_data_for_db(char *buffer)
{
    char *buff = malloc(DISK_BLOCKS_SIZE + 1);
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
    create_disk_blocks();

    char buffer[101];
    read(fd_fs, buffer, 12);

    char *p = strtok(buffer, " ");
    SB.nr_blocks = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_inodes = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_free_blocks = hex2int(p);

    p = strtok(NULL, " ");
    SB.nr_free_inodes = hex2int(p);

    // initializare bitmap
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
        if (SB.i_bitmap[i] == 1)
        {
            char c = '0';
            int index = 0;
            while (c != '|')
            {   
                read(fd_fs, buffer + index, 1);
                c = buffer[index];
                index++;
            }
            char *p = strtok(buffer, "/|");
            unsigned char *temp = read_data_for_db(p);
            strcpy(Inodes[i].name, temp);

            p = strtok(NULL, "/|");
            int aux;
            aux = hex2int(p);
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

            Inodes[i].index_blocks = (int *)malloc(Inodes[i].i_blocks * sizeof(int));

            for (int j = 0; j < Inodes[i].i_blocks; j++)
            {
                p = strtok(NULL, "/|");
                aux = hex2int(p);
                Inodes[i].index_blocks[j] = aux;
            }
        }
        else
        {
            read(fd_fs, buffer, 14);
            Inodes[i] = create_inode("", 0, 0, 0, 0, 0, 0);
        }
    }
}

void citire_DB(int fd_fs)
{
    // pt a avea offsetul
    int offset = lseek(fd_fs, 0, SEEK_CUR);
    for (int i = 0; i < SB.nr_blocks; i++)
    {
        int off = offset;
        if (SB.b_bitmap[i] == 1)
        {
            // offsetul de unde incepe
            /*off += i * DISK_BLOCKS_SIZE * 2; // scrie 2*disk_block_size
            lseek(fd_fs, 0, off);*/
            char buff[DISK_BLOCKS_SIZE * 2 + 1];
            read(fd_fs, buff, DISK_BLOCKS_SIZE * 2 + 1); // pt a lua si |
            DB[i].data = read_data_for_db(buff);
        }
    }
}
////////////////////

/////////////scriere
void scriere_superblock(int fd_fs)
{
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
        write(fd_fs, "/", 1);

        for (int j = 0; j < Inodes[i].i_blocks; j++)
        {
            sprintf(hexstring, "%X", Inodes[i].index_blocks[j]);
            write(fd_fs, hexstring, strlen(hexstring));
            write(fd_fs, "/", 1);
        }

        if (i != SB.nr_inodes - 1)
            write(fd_fs, "|", 1);
    }

    write(fd_fs, " ", 1);
}

void scriere_db(int fd_fs)
{
    char hexablock[DISK_BLOCKS_SIZE * 2 + 1];
    for (int i = 0; i < SB.nr_blocks; i++)
    {
        for (int j = 0; j < DISK_BLOCKS_SIZE; j++)
            sprintf(hexablock + j * 2, "%02X", DB[i].data[j]);
        write(fd_fs, hexablock, strlen(hexablock));
        if (i != SB.nr_blocks - 1)
            write(fd_fs, "|", 1);
    }
}
////////////////////////////

//////////////////copy_from
void inode_info(int fd, int index, char *filename)
{
    struct stat about_file;
    fstat(fd, &about_file);

    // populare tabela inodes
    // nr blocks pt fs-ul nostru
    int nr_blocks = about_file.st_size / DISK_BLOCKS_SIZE;
    if (about_file.st_size % DISK_BLOCKS_SIZE != 0)
        nr_blocks++;
    Inodes[index] = create_inode(filename, current_directory, about_file.st_mode, about_file.st_uid,
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

void populare_disk_blocks(int fd, int index, char *file)
{

    inode_info(fd, index, file);

    // citire din fisier
    if (S_ISREG(Inodes[index].i_mode))
    {
        int size = Inodes[index].i_size;
        char buff[size];
        read(fd, buff, size);
        int citit=0;

        // nr disk block-uri pentru file sys nostru
        int nr_blocks = Inodes[index].i_blocks;

        // verificare diskblock liber -- pt fiecare diskblock necesar
        int offset_buff = 0;
        for (int i = 0; i < nr_blocks; i++)
        {
            int index_bitmap = -1;
            int j = 0;
            while (index_bitmap == -1)
            {
                if (SB.b_bitmap[j] == 0)
                {
                    index_bitmap = j;
                    SB.b_bitmap[index_bitmap] = 1;
                    Inodes[index].index_blocks = realloc(Inodes[index].index_blocks, sizeof(int) * i + 1);
                    Inodes[index].index_blocks[i] = j;
                }
                j++;
            }
            if(i==nr_blocks-1) 
            {
                strncpy(DB[index_bitmap].data, buff+citit, Inodes[index].i_size-citit);
                citit+=Inodes[index].i_size-citit;
            }
            else
            {
                strncpy(DB[index_bitmap].data, buff+citit, DISK_BLOCKS_SIZE);
                SB.nr_free_blocks--;
                citit+=DISK_BLOCKS_SIZE;
            }
        }
    }
    else
        error("Copierea este permisa doar pentru fisiere!");
}

////////////////////////////////////////////////////
void create_fs()
{
    create_superblock();

    create_root();

    create_disk_blocks();
}

void mount_fs()
{
    int fd_fs = open("fs.data", O_RDONLY);

    // Initializare superblock
    citire_SB(fd_fs);

    // Initializare inodes
    citire_Inodes(fd_fs);

    // Initilizare disk block-uri
    citire_DB(fd_fs);

    close(fd_fs);
}

void sync_fs()
{
    // open - fs.data
    int fd_fs = open("fs.data", O_RDWR, 0777);

    if (fd_fs < 0)
        perror("fd");

    // write superblock;
    scriere_superblock(fd_fs);

    // write inodes
    scriere_inodes(fd_fs);

    // write disk blocks
    scriere_db(fd_fs);

    close(fd_fs);
}

void copyfrom(char *file)
{
    for (int i = 0; i < SB.nr_inodes; i++)
    {
        if(strcmp(Inodes[i].name, file) == 0 && Inodes[i].i_parent==current_directory)
            error("Exista deja un fisier cu acest nume in acest director");
    }
    int fd = open(file, O_RDONLY);
    int fd_fs = open("fs.data", O_WRONLY);

    // verificare inode liber
    int index = index_inode_liber();

    // disk block-uri
    populare_disk_blocks(fd, index, file);

    printf("\n");
    sync_fs();

    close(fd_fs);
    close(fd);
}

void change_directory(char *nume_director)
{
    if (strcmp(nume_director, "..") == 0)
        current_directory = Inodes[current_directory].i_parent;
    for (int i = 1; i < SB.nr_inodes; i++)
    {
        if (Inodes[i].i_parent == current_directory && strcmp(Inodes[i].name, nume_director) == 0)
        {
            char verificare[100];
            sprintf(verificare, "%o", Inodes[i].i_mode);
            if (verificare[0] == '4' && verificare[1] == '0')
            {
                current_directory = i;
            }
        }
    }
}

void ls()
{
    for (int i = 0; i < SB.nr_inodes; i++)
    {
        if (Inodes[i].i_parent == current_directory)
        {

            printf("%s\t", Inodes[i].name);
        }
    }
    printf("\n\n");
}

void create_directory(char *dir_name)
{
    int i = index_inode_liber();

    Inodes[i] = create_inode(dir_name, current_directory, Inodes[current_directory].i_mode,
                             Inodes[current_directory].i_uid, Inodes[current_directory].i_gid,
                             0, 0);
    SB.nr_free_inodes--;
}

void create_file(char *filename)
{
    int i = index_inode_liber();

    Inodes[i] = create_inode(filename, current_directory,
                             10755, Inodes[current_directory].i_uid, Inodes[current_directory].i_gid,
                             0, 0);
    SB.nr_free_inodes--;
}

void free_command()
{
    for (int i = 0; i < 256; i++)
    {
        if (command[i] != NULL)
        {
            free(command[i]);
            command[i] = NULL;
        }
    }
}

int verificare_comanda(const char *comanda)
{
    if (strcmp(comanda, "exit") == 0)
        return EXIT;
    if (strcmp(comanda, "ls") == 0)
        return LS;
    if (strcmp(comanda, "cd") == 0)
        return CD;
    if (strcmp(comanda, "mkdir") == 0)
        return MKDIR;
    if (strcmp(comanda, "cp") == 0)
        return CP;
    if (strcmp(comanda, "find") == 0)
        return FIND;
    if (strcmp(comanda, "cat") == 0)
        return CAT;

    error("Comanda este incorecta");
}

void pars_line(const char *line)
{
    char *my_line = strdup(line);
    const char *delimiters = " \n";
    char *p;
    int index = 0;

    free_command();
    p = strtok(my_line, delimiters);
    while (p != NULL)
    {
        command[index] = strdup(p);

        index++;
        p = strtok(NULL, delimiters);
    }
}

void afisare_cale_fisier(int index_fisier)
{
    char **raspuns;
    int index_vector = 0;

    raspuns = (char **)malloc(sizeof(char *));
    raspuns[index_vector] = strdup(Inodes[index_fisier].name);
    index_vector++;

    while (Inodes[index_fisier].i_parent != -1)
    {
        raspuns = (char **)realloc(raspuns, sizeof(char *) * index_vector + 1);
        raspuns[index_vector] = strdup(Inodes[Inodes[index_fisier].i_parent].name);
        index_vector++;
        index_fisier = Inodes[index_fisier].i_parent;
    }

    for (int i = index_vector - 1; i >= 0; i--)
    {
        printf("/%s", raspuns[i]);
    }
    printf("\n");

    for (int i = 0; i < index_vector; i++)
        free(raspuns[i]);

    free(raspuns);
}

void find(char *nume_fisier)
{

    int index_fisier = -1;
    for (int i = 0; i < SB.nr_inodes; i++)
    {

        if (strcmp(Inodes[i].name, nume_fisier) == 0)
        {
            if (S_ISDIR(Inodes[i].i_mode))
                error("E director!");
            else
            {
                index_fisier = i;
                afisare_cale_fisier(i);
            }
        }
    }
    if (index_fisier == -1)
        error("Fisierul cautat nu exista!");
}
void cat(char *nume_fisier)
{
    int index_fisier = -1;
    for (int i = 0; i < SB.nr_inodes; i++)
    {

        if (strcmp(Inodes[i].name, nume_fisier) == 0 && Inodes[i].i_parent == current_directory)
        {

            index_fisier = i;
        }
    }
    
    if(index_fisier == -1)
        printf("Fisierul nu exista in acest director\n");
    else
    {
        if(Inodes[index_fisier].i_blocks==0)
            printf("\n");
        else
        {
            for(int i=0;i<Inodes[index_fisier].i_blocks;i++)
            {
                printf("%s", DB[Inodes[index_fisier].index_blocks[i]].data);
            }
        }
    }
    printf("\n");
}

void main()
{
    char line[2048];
    mount_fs();

    while (1)
    {
        printf("%s>", Inodes[current_directory].name);
        fgets(line, 2048, stdin);

        pars_line(line);
        int command_type = verificare_comanda(command[0]);

        switch (command_type)
        {
        case LS:
            ls();
            break;
        case CD:
            if (command[1] == NULL)
                error("comanda incompleta");
            else
                change_directory(command[1]);
            break;
        case MKDIR:
            if (command[1] == NULL)
                error("comanda incompleta");
            else
                create_directory(command[1]);
            break;
        case CP:
            if (command[1] == NULL)
                error("comanda incompleta");
            else
                copyfrom(command[1]);
            break;
        case FIND:
            if (command[1] == NULL)
                error("comanda incompleta");
            else
                find(command[1]);
            break;
        case CAT:
            if (command[1] == NULL)
                error("comanda incompleta");
            else
                cat(command[1]);
            break;    
        case EXIT:
            goto FINAL;
            break;
        default:
            break;
        }
    }

FINAL:
    sync_fs();
    free_command();
    
}