#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#define MAX_COMMAND_LINE_BUF_SIZE 1024

/*
 * Displays any command given in history
 */

struct list {
    char           *command;
    struct list    *next;
} head;

void 
add_command_to_history(char *command)
{
    struct list    *temp;
    if (!head) {
	/*
	 * Means we are inserting first element in history
	 */
	temp = (struct list *)malloc(sizeof(struct list));
	head = temp;
	head->next = NULL;
	strcpy(head->command, command);
    } else {
	temp = (struct list *)malloc(sizeof(struct list));
	/*
	 * Insert the element at head of the list
	 */
	temp->next = head;
	strcpy(temp->command, command);
	head = temp;
    }
}

void 
check_history(char *buf, int len)
{
    struct list    *temp = head;
    if (temp) {
	/*
	 * There are no commands in the list
	 */
	return;
    } else {
	while (temp) {
	    if (strncmp(buf, temp->command, len - 1)) {
		//Print the command
		c = getchar();
		if (c == '\n') {
		    return temp->command;
		} else if (c == '\t') {
		    temp = temp->next;
		    continue;
		} else {
		    printf("Not a valid command \n");
		    return;
		}
	    } else {
		temp = temp->next;
		continue;
	    }
	}
    }
}

void 
print_shell_prompt()
{
    printf("%s@%s $:", getlogin(), "SA_SHELL");
}

void
read_profile_file()
{

}

char*
read_command_line()
{
    int		buf_size = MAX_COMMAND_LINE_BUF_SIZE;
    char        *buf;
    int		index;

    buf = malloc(sizeof(char) * buf_size);
    if (buf == NULL) {
	printf(" Memory allocation failure for MALLOC in %s:%d", __FUNCTION__,
	       __LINE__);
	return NULL;
    }
    while (TRUE) {
	c = getchar();
	if (c == '\t') {
	    /*
	     * Implement tab or history feature here
	     */
	    check_history(buf, index);
	}
	if (c == '\n') {
	    buf[index] = '\0';
	    return buf;
	} else {
	    buf[index] = c;
	}
	index++;
    }
}

bool 
execute_shell_command(char **command)
{

    if (fork()) {
	/*
	 * Child process
	 */
	execvp(command[0], command);
    } else {
	/*
	 * Parent process
	 */
    }
}

int 
main()
{
    bool    status = TRUE;

    /*
     * Read the .profile file in the same directory and init shell with
     * path and other environment variables
     */
    read_profile_file();

    while (status) {
	print_shell_prompt();

	command_line = read_command_line();

	add_command_to_history(command_line);

	status = execute_shell_command();
    }
}
