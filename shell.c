#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LINE_BUF_SIZE 1024

#define TRUE                      1
#define FALSE                     0
#define STD_INPUT                 0
#define STD_OUTPUT                1
char *history[1024];
int history_index =0;
char * check_history(const char *text, int state);
static char ** custom_complete_function( char * command ,
					 int start, int end);
int enable = TRUE;
char * dupstr (char* s) {
    char *r;

    r = (char*) malloc ((strlen (s) + 1));
    strcpy (r, s);
    return (r);
}


rl_completion_func_t * rl_attempted_completion_function = custom_complete_function;
/*
 * Once we enter command we store that in the list . 
 * Routine to add command in the list 
 */
void 
add_command_to_history(char *command)
{
    char *new_str;
    int index = 0;
    /* 
     * If there is already same command entered earlier 
     * dont store just return
     */
    while (history[index]) {
	if (!strcmp(history[index], command)) {
	    return;
	}
    }
    new_str = dupstr(command);
    history[history_index] = new_str; 
    history_index++;
}

/*
 * Check for the commands in the list
 * Not complete yet 
 */ 
char *
check_history(const char *text, int state)
{
    char *c;
    static int index, len ;
    if (!state) {
	index = 0;
	len = strlen (text);
    }

    while((c = history[index])) {
	index++;

	if (!strncmp(c, text, len)) {
	    return dupstr(c);
	} 
    }
    return (char *)NULL;
}
static char ** custom_complete_function( char * command , 
					 int start, int end)
{
    char **found ;
    found =(char **) NULL ;
    if (start == 0) {
	found = rl_completion_matches ((char*)command, &check_history);
    } else {
	rl_bind_key('\t', rl_insert);
    }
    return found;
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
 * Signal handler to handle alarm 
 */
void signal_handler_alarm(int sig){
    if(sig == SIGALRM) {
	printf("Command is taking more than 5 seconds \n");
	printf(" Press ctrl + d to avoid this alarm \n");
	printf(" And let the program to run \n");
    }
}

void signal_handler_ctrl_d(int sig) {
    if (sig == SIGQUIT) {
	printf("Switching alarm feature \n");
	if(enable ) {
	    enable = FALSE;
	    alarm(0);
	} else {
	    enable = TRUE; 
	    alarm(5);
	}
    }
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
	if (signal(SIGALRM, signal_handler_alarm) == SIG_ERR) {
	    perror("SIGALRM");
	}
	if (signal(SIGQUIT, signal_handler_ctrl_d) == SIG_ERR) {
	    perror("SIGQUIT");
	}
	/*parent wait for child to complete*/
	alarm(5);
	waitpid(pid,&status,0);
	signal(SIGALRM,SIG_IGN);
    }
    return TRUE;
}
int
execute_shell_command_with_redir(char **command, char* filename)
{

    int status , fd ,ret ;
    pid_t pid ; 
    pid = fork();
    
    fd = open (filename, O_RDWR | O_CREAT , S_IRWXG|S_IRWXO|S_IRWXU);

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
	command_line = readline ("");
	/* 
	 * Call the custom complete function 
	 */
	rl_bind_key('\t', rl_complete);
	add_command_to_history(command_line);
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
