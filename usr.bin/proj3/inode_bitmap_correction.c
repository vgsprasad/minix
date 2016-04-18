#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include<dirent.h>
#include "fs.h"
#include "inode.h"
#include "super.h"
#define DIR_NAME_MAX_LEN 1000
void dir_walker(char *dir_name);
void correct_inode_bit_map()
{
       dir_walker("/");
}
void dir_walker(char *dir_name)
{
    DIR * D;
    int fd;
    char val;
    char *ptr;
    struct dirent * dir_entry;
    D = opendir(dir_name);
    if (D == NULL) {
	return;
    }
    dir_entry = readdir(D);

    while((dir_entry = readdir(D)) != NULL)
    {
	if (dir_entry->d_type == DT_DIR)
	{
	    int length;
	    char sub_dir[DIR_NAME_MAX_LEN];
	    length = snprintf(sub_dir, DIR_NAME_MAX_LEN -1 ,
			      "%s/%s", dir_name, dir_entry->d_name);
	    sub_dir[length] = '\0';
	    if (!(strcmp(dir_entry->d_name ,".")))
	    {
		continue;
	    }
	    if (!(strcmp(dir_entry->d_name, "..")))
	    {
		continue;
	    }
	    fd = open("/dev/c0d0p1",O_RDWR);
	    lseek(fd, 1024 + 1024 +dir_entry->d_ino ,SEEK_SET);
	    ptr = (char *)malloc(sizeof(char));
	    read(fd,ptr,sizeof(char));
	    if(!(*ptr & (1 <<((dir_entry->d_ino)%8)))) {
		val = *ptr | (1 <<(dir_entry->d_ino%8));
		lseek(fd, 1024+ 1024 +dir_entry->d_ino/8,SEEK_SET);
		write(fd, &val, sizeof(char));
		close(fd);
	    }
	    dir_walker(sub_dir);
	}
	else
	{
	    fd = open("/dev/c0d0p1",O_RDWR);
	    lseek(fd, 1024 + 1024 +dir_entry->d_ino,SEEK_SET);
	    ptr = (char *)malloc(sizeof(char));
	    read(fd,ptr,sizeof(char));
	    if(!(*ptr & (1 <<((dir_entry->d_ino)%8)))) {
		val = *ptr | (1 <<(dir_entry->d_ino%8));
		lseek(fd, 1024 + 1024 +dir_entry->d_ino/8,SEEK_SET);
		write(fd, &val, sizeof(char));
	    }
	    close(fd);
	}
    }
}
int main()
{
    char val;
    struct super_block sb;
    int fd;
    unsigned long n;
    int count =0;
    int i , j;
    int imapsize, blksize;
    int index;
    char *ptr;
    struct inode *ptr1;
    fd = open("/dev/c0d0p1",O_RDONLY);
    lseek(fd,1024,SEEK_CUR);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, 1024 + 1024 ,SEEK_SET);

    imapsize = sb.s_imap_blocks;
    blksize = sb.s_block_size;

    ptr = (char * )malloc(imapsize*blksize);
    n = read(fd,ptr,(imapsize*blksize));
    for(i=0;i<(imapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*ptr & (0x1 << j)) {
		printf("%d \t ", i);
		count++;
	    }
	}
	ptr++;
    }
    close(fd);
    printf("Total number of inodes allocated =%d \n",count ? count-1:count);

    printf(" Corrupting the first available inode in inode bit map table \n");
    fd = open("/dev/c0d0p1",O_RDWR);
    lseek(fd,1024,SEEK_CUR);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, 1024+ 1024 ,SEEK_SET);

    imapsize = sb.s_imap_blocks;
    blksize = sb.s_block_size;

    ptr = (char * )malloc(imapsize*blksize);
    read(fd,ptr,(imapsize*blksize));
    for(i=0;i<(imapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*ptr & (0x1 << j)) {
		val = *ptr & ~(0x1 << j);
		printf("%x \n", *ptr);
		printf("%x \n", val);
		lseek(fd, 1024 + 1024 + i ,SEEK_SET);
	//write(fd, &val,  1);
		goto done;
	    }
	}
	ptr++;
    }
done:
    printf("One bit in inode bit map is cleared .\n");
    close(fd);
    count =0 ;
    fd = open("/dev/c0d0p1",O_RDONLY);
    lseek(fd, 1024 + 1024 ,SEEK_SET);
    ptr = (char * )malloc(imapsize*blksize);
    read(fd,ptr,imapsize*blksize);
    for(i=0;i<(imapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*ptr & (0x1 << j))
		count++;
	}
	ptr++;
    }
    printf("Total number of inodes allocated =%d \n",count ? (count-1):count);
    printf("\n");
    close(fd);
    //   correct_inode_bit_map();
    return 0;
}
