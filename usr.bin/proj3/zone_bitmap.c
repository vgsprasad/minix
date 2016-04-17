#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "fs.h"
#include "inode.h"
#include "super.h"

int main()
{
    struct super_block sb;
    int fd;
    unsigned long n;
    int count =0;
    int i, j;
    int zmapsize, blksize;
    int index;
    char *ptr;
    fd = open("/dev/c0d0p2",O_RDONLY);
    lseek(fd,1024,SEEK_SET);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, sizeof(struct super_block) + 1024 + (sb.s_block_size*sb.s_imap_blocks), 
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
