#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>

#define MAX_COMMAND_LINE_BUF_SIZE 1024

#define TRUE                      1
#define FALSE                     0
#define STD_INPUT                 0
#define STD_OUTPUT                1
/*
 * Displays any command given in history
 */

/*
 * This list is created to store all the commands which are 
 * entered during the lifespan of shell
 * We can store it in FILE too. Design under consideration 
 */ 
struct list {
    char           *command;
    struct list    *next;
} *head;

/*
 * Once we enter command we store that in the list . 
 * Routine to add command in the list 
 */
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

/*
 * Check for the commands in the list
 * Not complete yet 
 */ 
char *
check_history(char *buf, int len)
{
    char c;
    struct list    *temp = head;
    if (temp) {
	/*
	 * There are no commands in the list
	 */
	return NULL;
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
		    return NULL;
		}
	    } else {
		temp = temp->next;
		continue;
	    }
	}
    }
    return NULL;
}

/*
 * Print the shell prompt 
 */ 
void 
print_shell_prompt()
{
    printf("%s@%s $:", getlogin(), "SA_SHELL");
}

/*
 * It reads the .profile in the current directory and 
 * changes the current dir to HOME directory mentioned in 
 * .profile file 
 */
void
read_profile_file()
{
    char line[1024];
    char homedir[1024];
    char *ptr;

    memset(line,0,1024);
    memset(homedir,0,1024);

    FILE *fp=fopen(".profile","r");

    if(fp!=NULL){
	while(fgets(line,1024,fp) != NULL){
	    printf("%s",line);
	    if(!strncmp(line,"HOME",4)){
		ptr=strchr(line,'=');
		strcpy(homedir,(ptr+1));
		strtok(homedir,"\n");
		printf("homedir=%s\n",homedir);
		if(chdir(homedir) < 0){
		    perror("homedir");
		    printf("chdir to %s failed",homedir);
		}
	    }
	}
	fclose(fp);
    }else{
	perror(".profile");
    }

}

/*
 * Read one charachter at a time and store it in buf 
 */
char*
read_command_line()
{
    int		buf_size = MAX_COMMAND_LINE_BUF_SIZE;
    char        *buf;
    char	c; 
    int		index = 0;
    buf = (char *) malloc(sizeof(char) * buf_size);
    if (!buf) {
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
	//check_history(buf, index);
	} else if ((c == '\n')|| (c == EOF)) {
	    buf[index] = '\0';
	    break;
	} else {
	    buf[index] = c;
	}
	index++;
    }
    return buf;
}

/* 
 * Main function which executes the shell commands by forking child process 
 */
int
execute_shell_command(char **command)
{

    int status, ret;
    pid_t pid ; 
    pid = fork();
    
    if (pid < 0 ) {
	printf("Not able to fork a new process \n");
    } else if (pid == 0) {
	/*
	 * Child process runs the command 
	 */
	ret = execvp(command[0], command);
	if (ret < 0) { 
	    printf("Execv failed Errno = %d\n", errno);
	}
    } else {
	/*
	 * Parent process 
	 * Wait for child to exit and then return 
	 */
	waitpid(pid,&status,0);	
    }
    return TRUE;
}
int
execute_shell_command_with_redir(char **command, char* filename)
{

    int status , fd ,ret ;
    pid_t pid ; 
    pid = fork();
    
    fd = open (filename, O_RDWR);

    if (pid < 0 ) {
	printf("Not able to fork a new process \n");
    } else if (pid == 0) {
	/*
	 * Child process runs the command 
	 */
	close (STD_OUTPUT);
	dup (fd);
	close(fd);
	ret = execvp(command[0], command);
	if (ret < 0) { 
	    printf("Execv failed Errno = %d", errno);
	}
    } else {

	waitpid(pid,&status,0);	
    }
    return TRUE;
}


/* 
 * From single line command line convert those to tokens with strtok 
 * API . 
 */ 
void 
convert_to_tokens(char *line , char ** token )
{
    int token_index = 0, i=0; 
    char *temp_token; 
    
    /* 
     * Now we have full command as one line . Lets break those 
     * into tokens 
     */

    temp_token = strtok(line, " \n\t");
    while (temp_token) {
	token[token_index] = temp_token; 
	token_index++;
	temp_token = strtok(NULL, " \n\t");
    }
    token [token_index] = NULL;
    

}

/*
 * Main function 
 */
int 
main()
{
    int  status = TRUE;
    char * command_line;
    char **token; 
    char *filename;
    int i=0, num_redir_op =0, redir_index =0;

    /*
     * Read the .profile file in the same directory and init shell with
     * path and other environment variables
     */
    read_profile_file();

    while (status) {
	print_shell_prompt();
	i=redir_index=num_redir_op =0;
	command_line = read_command_line();
	// add_command_to_history(command_line);
	token = malloc (sizeof(char) *1024);
	if (token == NULL) {
	    printf ("Not able to allocate memory for tokens \n");
	    return 0; 
	}
        convert_to_tokens(command_line, token);

	while (token[i]) {
	    if (!strcmp( token[i], ">=")) {
		/* there is a redirection operator */
		num_redir_op ++;
		redir_index = i;
	    }
	    i++;
	}
	switch( num_redir_op) {
	case 0:  status = execute_shell_command(token);
		 break;

	case 1:  if (token[redir_index+2] != NULL) {
		     printf("After redirection op there should be only one file name\n");
		     status = TRUE;
		 } else {
		     filename = token[redir_index + 1];
		     token[redir_index] = NULL;
		     token[redir_index+1] = NULL;
		     status = execute_shell_command_with_redir(token, filename);
		 }
		 break;
	default:
		 printf("More than one redirection op's not allowed \n");
		 break;
	}
    }
}
