#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#define DIR_NAME_MAX_LEN 1000

void usage()
{
    printf(" Usage: dirwalker <dir_name> \n");
}

void dir_walker(char *dir_name) 
{
    DIR * D;
    struct dirent * dir_entry;
    /*
     * Open the directory entry 
     */
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
	    /*
	     * If this is current directory dont to recursion 
	     */
	    if (dir_entry->d_name == ".") 
	    {
		continue; 
	    }
	    /* 
	     * If its previous directory again dont do recursion 
	     */
	    if (dir_entry->d_name == "..") 
	    {
		continue;
	    }
	    printf("%20s  %30ld\n", dir_entry->d_name, 
		   dir_entry->d_ino);
	    dir_walker(sub_dir);
	}
	else 
	{
	    printf("%20s  %30ld\n", dir_entry->d_name, 
		   dir_entry->d_ino);
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

    printf("--------------------------------------------------------------------------------\n");
    printf("        FILE                          |                        Inode Number     \n");
    printf("--------------------------------------------------------------------------------\n");
    dir_name = argv[1];
    dir_walker(dir_name);
}


