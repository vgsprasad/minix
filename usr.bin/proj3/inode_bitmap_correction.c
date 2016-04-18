#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include<dirent.h>
#include "fs.h"
#include "inode.h"
#include "super.h"
#define DIR_NAME_MAX_LEN 1000

int inode_list[10000] ;
int g_inode_count = 0;
void dir_walker(char *dir_name);

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
	    inode_list[g_inode_count++] = dir_entry->d_ino;
	    dir_walker(sub_dir);
	}
	else
	{
	    inode_list[g_inode_count++] = dir_entry->d_ino;
	}
    }
}

int main(int argc, char **argv)
{
    char val;
    struct super_block sb;
    int fd;
    unsigned long n;
    int count =0;
    int i , j;
    int inode_num;
    int imapsize, blksize;
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

    dir_walker(".");
    fd = open(path, O_RDONLY);
    lseek(fd, 1024, SEEK_CUR);
    n = read(fd,&sb,sizeof(struct super_block ));
    lseek(fd, 1024 * 8 ,SEEK_SET);

    imapsize = sb.s_imap_blocks;
    blksize = sb.s_block_size;

    ptr = (char * )malloc(imapsize*blksize);
    n = read(fd,ptr,(imapsize*blksize));
    for(i=0;i<(imapsize*blksize);i++)
    {
	for(j=0;j<8;j++)
	{
	    if(*ptr & (0x1 << j)) {
		count++;

	    }
	}
	ptr++;
    }
    close(fd);
    printf("Total number of inodes allocated =%d \n",count ? count-1:count);

    fd = open(path, O_RDWR);
    printf(" Choosing random inode number from the allocated inodes \n");
    index = rand() % g_inode_count;
    inode_num = inode_list [index];
    printf("Chosen inode %d\n", inode_num);
    printf(" Corrupt that inode \n");
    lseek(fd, 1024 * 8 + inode_num/8 ,SEEK_SET);
    ptr = (char *)malloc(sizeof(char));
    read(fd, ptr, 1);

    val = *ptr & ~(1 << (8- (inode_num % 8)));

    lseek(fd, 1024 * 8 + inode_num/8 ,SEEK_SET);
    write(fd, &val,  1);

    close(fd);
    count = 0 ;
    fd = open(path, O_RDONLY);
    lseek(fd, 1024 * 8 ,SEEK_SET);
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
    
    fd = open(path,O_RDONLY);
    lseek(fd, 1024*8 ,SEEK_SET);

    ptr = (char * )malloc(imapsize*blksize);

    n = read(fd,ptr,(imapsize*blksize));
    printf("Inode Bit map is : \n");
    for(i =0; i< imapsize*blksize; i++)
    {
	printf("%x" ,ptr[i]);
    }
    printf("\n");
    printf(" Now correct all the inodes which are broken \n");
    fd = open(path, O_RDWR);
    for(i=0; i <g_inode_count; i++)
    {
	inode_num = inode_list[i];
	lseek(fd, 1024 * 8 + inode_num/8 ,SEEK_SET);
	ptr = (char *)malloc(sizeof(char));

	read(fd, ptr, 1);

	if (*ptr & (1 << (8- (inode_num % 8)))) {
	    continue;
	} else {

	    lseek(fd, 1024 * 8 + inode_num/8 ,SEEK_SET);
	    val = *ptr | (1 << (8- (inode_num % 8)));
	    write(fd, &val,  1);
	}
    }
    close (fd);
    count = 0 ;
    fd = open(path, O_RDONLY);
    lseek(fd, 1024 * 8 ,SEEK_SET);
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
    return 0;
}
