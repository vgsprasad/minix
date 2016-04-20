#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "fs.h"
#include "inode.h"
#include "super.h"

int main(int argc, char **argv)
{
    struct super_block sb;
    int fd, fd1;
    unsigned long n;
    int count =0;
    int i, j;
    int zmapsize, blksize;
    int index;
    char *ptr, *ptr1 ,*temp, *ptr2, *ptr3, *ptr4;
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
    fd = open(path, O_RDWR);
    if ( fd < 0)
    {
	printf(" Cannot open the file %s\n", path);
	return 0;
    }

    fd1 = open("~/dup_zone_map", O_RDWR);
    if(fd1 <0 )
    {
	printf(" Not able to open dup zone map file \n");
	return 0;
    }
    lseek(fd,1024,SEEK_SET);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, 1024 * 8 + (sb.s_block_size*sb.s_imap_blocks), SEEK_SET);

    zmapsize = sb.s_zmap_blocks;
    blksize = sb.s_block_size;

    ptr = (char * )malloc(zmapsize*blksize);

    n = read(fd, ptr, (zmapsize*blksize));
    if ( n != zmapsize*blksize) {
	printf(" Not able to read full zone map \n");
	return 0;
    }
    write(fd1, ptr, (zmapsize*blksize));

    printf("Zone Bit map is : \n");
    for(i =0; i< zmapsize*blksize; i++)
    {
	printf("%x " ,ptr[i]);
    }
    printf("\n");
    temp = ptr;
    for(i=0;i<(zmapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*temp & (0x1 << j))
		count++;
	}
	temp++;
    }
    printf("Total number of Zones allocated =%d \n",count);

    printf("--------------------------------\n");
    printf(" Now corrupt the zonemap \n");
    lseek(fd, 1024 * 8 + (sb.s_block_size*sb.s_imap_blocks), SEEK_SET);
    ptr1 = (char *) malloc(zmapsize*blksize);
    memset (ptr1, 0 , (zmapsize*blksize));
    write(fd, ptr1, (zmapsize*blksize));

    printf("--------------------------------\n");
    printf(" Now print the zone bit map \n");
    lseek(fd, 1024 * 8 + (sb.s_block_size*sb.s_imap_blocks), SEEK_SET);
    ptr3 = (char *) malloc(zmapsize*blksize);
    n = read(fd, ptr3, (zmapsize*blksize));
    count =0;
    for(i =0; i< zmapsize*blksize; i++)
    {
	printf("%x " ,ptr3[i]);
    }
    printf("\n");
    temp = ptr3;
    for(i=0;i<(zmapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*temp & (0x1 << j))
		count++;
	}
	temp++;
    }
    printf("Total number of Zones allocated =%d \n",count);
    printf("--------------------------------\n");
    printf(" Correct the zone bit map \n");
    lseek(fd1,0,SEEK_SET);
    lseek(fd, 1024 * 8 + (sb.s_block_size*sb.s_imap_blocks), SEEK_SET);
    ptr2 = (char *) malloc(zmapsize*blksize);
    read(fd1, ptr2, (zmapsize*blksize));
    write(fd, ptr2,(zmapsize*blksize));
    printf("--------------------------------\n");
    printf(" Print the zone map now \n");
    lseek(fd, 1024 * 8 + (sb.s_block_size*sb.s_imap_blocks), SEEK_SET);
    ptr4 = (char *) malloc(zmapsize*blksize);
    read(fd, ptr4, (zmapsize*blksize));
    count =0;
    for(i =0; i< zmapsize*blksize; i++)
    {
	printf("%x " ,ptr4[i]);
    }
    printf("\n");
    temp = ptr4;
    for(i=0;i<(zmapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*temp & (0x1 << j))
		count++;
	}
	temp++;
    }
    printf("Total number of Zones allocated =%d \n",count);
    printf("\n");
    close(fd);
    close(fd1);
    return 0;
}
