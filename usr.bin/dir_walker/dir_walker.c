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
	     * If this is current directory dont do recursio	
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
	    printf("%s\n", dir_entry->d_name);
	    dir_walker(sub_dir);
	}
	else
	{
	    printf("%s\n", dir_entry->d_name);
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

    printf("Entered directory name = %s", argv[1]);
    dir_name = argv[1];
    dir_walker(dir_name);
}
