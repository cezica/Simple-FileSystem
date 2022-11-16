#include "fs.h"

struct superblock SB;
struct inode* Inodes;
struct disk_block* DB;

void create_fs()
{
    /*
    SB.nr_inodes=10;
    SB.nr_blocks=100;
    SB.size_blocks=sizeof(struct disk_block);

    Inodes=malloc(sizeof(struct inode)*SB.nr_inodes);
    for(int i=0;i<SB.nr_inodes;i++)
    {
        Inodes[i].size=-1;
        strcpy(Inodes[i].name," ");
    }

    DB=malloc(sizeof(struct disk_block)*SB.nr_blocks);
    for(int i=0;i<SB.nr_blocks;i++)
    {
        DB[i].next_block_num=-1;
        strcpy(DB[i].data," ");
    }
    */

    //alloc superblock
    

}

void mount_fs()
{
    int fd_fs=open("fs.data",O_RDONLY);



    close(fd_fs);
}

void sync_fs()
{
    //open - fs.data
    int fd_fs=open("fs.data",O_RDWR | O_CREAT | O_TRUNC,0777);

    if(fd_fs<0)
        perror("fd");
    
    //write superbloc;

    /*
    char* buff=malloc(sizeof(int)*3+13);//" | "
    char append[5];

    sprintf(append,"%d",SB.nr_inodes);
    strcat(buff,append);
    strcat(buff,"|");
    sprintf(append,"%d",SB.nr_blocks);
    strcat(buff,append);
    strcat(buff,"|");
    sprintf(append,"%d",SB.size_blocks);
    strcat(buff,append);
    strcat(buff,"\n");

    printf("%s",buff);
    //write(fd_fs,buff,strlen(buff));
    //write(fd_fs)

    printf("-----------\n");
    */
    //write inodes
    /*
    for(int i=0;i<SB.nr_inodes;i++)
    {
        char* buff=malloc(strlen(Inodes[i].name+sizeof(int)+1));
        strcat(buff,Inodes[i].name);
        strcat(buff,",");
        char*append=malloc(sizeof(int));
        sprintf(append,"%d",Inodes[i].size);
        strcat(buff,append);
        strcat(buff,"\n");

        printf("%s",buff);
        //write(fd_s,buff,strlen(buff));
    }

    printf("-----------\n");
*/

    //write disk blocks
    /*
    for(int i=0;i<SB.nr_blocks;i++)
    {
        char* buff=malloc(strlen(DB[i].data)+sizeof(int)+sizeof(int));
        strcat(buff,DB[i].data);
        char* append=malloc(sizeof(int));
        strcat(buff,",");
        sprintf(append,"%d",DB[i].next_block_num);
        strcat(buff,append);
        strcat(buff,"\n");

        printf("%s",buff);
        //write(fd_fs,buff,strlen(buff));
    }
*/
    
    close(fd_fs);
}

void main()
{
    create_fs();
    sync_fs();
    printf("succes!\n");
}