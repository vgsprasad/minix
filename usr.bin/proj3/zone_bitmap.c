#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "fs.h"
#include "inode.h"
#include "super.h"
#define SUPER_BLOCK_SIZE 1024 
int main(int argc, char **argv)
{
    struct super_block sb;
    int fd;
    unsigned long n;
    int count =0;
    int i, j;
    int zmapsize, blksize;
    int index;
    char *ptr;
    FILE *fp;
    char *str=malloc(1024);
    char *path = malloc(1024);
    if (argc < 2) {
	printf("Usage : <binary> <file_name> \n");
	return 1;
    }
    memset(str,0,1024);
    memset(path,0,1024);
    sprintf(str,"df %s | grep dev ",argv[1]);
    fp = popen(str,"r");
    fscanf(fp,"%s",path);
    fd = open(path, O_RDONLY);
    lseek(fd,1024,SEEK_SET);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, SUPER_BLOCK_SIZE + 1024 + 1024 * 6 +(sb.s_block_size*sb.s_imap_blocks), 
	  SEEK_SET);

    zmapsize = sb.s_zmap_blocks;
    blksize = sb.s_block_size;

    /*allocate memory to store inode bitmap data*/
    ptr = (char * )malloc(zmapsize*blksize);

    n = read(fd,ptr,(zmapsize*blksize));
    printf("Zone Bit map is : \n");
    for(i =0; i< zmapsize*blksize; i++)
    {
	printf("%x " ,ptr[i]);
    }
    printf("\n");
    for(i=0;i<(zmapsize*blksize);i++)
    {
	for(j=0;j<8;j++) 
	{
	    if(*ptr & (0x1 << j))
		count++;
	}
	ptr++;
    }
    printf("Total number of Zones allocated =%d \n",count);
    printf("\n");
    close(fd);
    return 0;
