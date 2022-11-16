#include <sys/mount.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int main()
{
    
    //copy_file("./try/bin/bash","./copy_bin");
    return 0;
}
