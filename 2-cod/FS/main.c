#include <sys/mount.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h> 

//mount la o partitie specificata
int mount_partition(const char* partname)
{
    if(mount(partname,"./try","ext4",0,NULL)<0)
        {
            perror("mount");
            return -1;
        }
    else return 0;
}

//copiere fisier
int copy_file(const char* src,const char* dest)
{
    char* buf=malloc(sizeof(char));
    int fd_src=open(src,O_RDONLY);
    int fd_dest=open(dest,O_WRONLY|O_TRUNC|O_CREAT,0644);

    if(fd_src==-1||fd_dest==-1)
        return -1;
/*
    while(read(fd_src,buf,1)>0)
        write(fd_dest,buf,1);
*/
// COPIERE EXACTA!!!
    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    fstat(fd_src, &fileinfo);
    int result = sendfile(fd_dest, fd_src, &bytesCopied, fileinfo.st_size);
    if(result!=-1)
        printf("Copiat cu succes!\n");

    close(fd_src);
    close(fd_dest);

    return 0;
}

void show_dir_content(char * path)
{
  DIR * d = opendir(path);
  if(d==NULL) return; 
  struct dirent * dir; //dirent structure 1.file serial number 2.name of entry
  while ((dir = readdir(d)) != NULL) 
    { 
        if(strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 )
            printf("%s\n", dir->d_name);
    }
    closedir(d); 
}

int main()
{
    //show_dir_content("/home/razvan/PSO/L1");
    //copy_file("./try/bin/bash","./copy_bin");
    char line[2048];
    char* comanda;

    while(1)
    {
        printf(">");
        fgets(line, 2048, stdin);

        comanda=strtok(line, " \n");

        if(strcmp(comanda,"exit")==0)
            return 0;
        else if(strcmp(comanda, "ls")==0)
            show_dir_content(".");
        else
            printf("introuceti o comanda corecta\n");
    }
    return 0;
}