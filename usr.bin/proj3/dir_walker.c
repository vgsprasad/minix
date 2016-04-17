#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include "fs.h"
#include "inode.h"
#include "super.h"
#include "lib.h"
#define DIR_NAME_MAX_LEN 1000

void usage()
{
        printf(" Usage: dirwalker <dir_name> \n");
}

void dir_walker(char *dir_name)
{
    DIR * D;
    int fd;
    struct dirent * dir_entry;
    struct inode *inode_str;
    D = opendir(dir_name);

    if (D == NULL) {
	printf(" Cannot open the directory\n");
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
	    printf(" File Name : %s  Inode Number :  %ld ", dir_entry->d_name,
		   dir_entry->d_ino);
	    fd = open(dir_entry->d_name, O_RDONLY, 0);
	    dump_zone_info(dir_entry->d_ino, fd);
	    printf("\n");
	    printf("----------\n");
	    dir_walker(sub_dir);
	}
	else
	{
	    printf("File Name : %s   Inode Number : %ld ", dir_entry->d_name,
		   dir_entry->d_ino);
	    fd = open(dir_entry->d_name, O_RDONLY, 0);
	    dump_zone_info(dir_entry->d_ino, fd);
	    printf("\n");
	    printf("----------\n");
	    close(fd);
	}
    }
}
int main(int argc , char **argv)
{
    char *dir_name ;
    if (argc != 2) {
	usage();
	return 1;
    }
    dir_name = argv[1];
    dir_walker(dir_name);
}
