#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<readline/readline.h>
#include<sys/wait.h>
#include <termios.h>
#define MAX_COMMAND_LINE_BUF_SIZE 1024

#define TRUE                      1
#define FALSE                     0
#define STD_INPUT                 0
#define STD_OUTPUT                1

/* 
 * Global variables 
 */
char *command_history[1024] = {0};
char homedir[1024];
int  history_index = 0;
int  almset = 1;
int enable = TRUE;
/* 
 * Function prototypes 
 */
int find_num_tokens(char **token);
void print_shell_prompt(void);
char * check_history(const char *text, int state);
char ** custom_complete_function(const char * command ,
				 int start, int end);
char ** custom_match_function(const char *str, 
			      rl_compentry_func_t *fun);
char * dupstr (char* s);
void convert_to_tokens(char *line , char ** token );
int  execute_shell_command_with_redir(char **command, 
				      char* filename);
int  execute_shell_command(char **command);
void add_command_to_history(char *command);
void read_profile_file(void);
void signal_handler(int sig); 
char ** custom_match_function(const char *str, rl_compentry_func_t *fun);
char * dupstr (char* s) {
    char *r;
    r = (char*) malloc ((strlen (s) + 1));
    if (r == NULL) {
	printf("Not able to allocate memory \n");
	return NULL;
    }
    strcpy (r, s);
    return (r);
}

/*
 * Once we enter command we store that in the list . 
 * Routine to add command in the list 
 */
void 
add_command_to_history(char *command)
{
    char *new_str = dupstr(command);
    int index =0;
    FILE *fp;
    /*
     * If there is already same command entered earlier
     * dont store just return
     */
    while (command_history[index]) {
	if (!strcmp(command_history[index], command)) {
	    return;
	}
	index++;
    }
    fp = fopen("history","a");
    if (!fp) {
	printf(" Could not open history file \n");
    } else {
	fputs(new_str, fp);
	fputs("\n", fp);
	fclose(fp);
    }
    command_history[history_index] = new_str; 
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
    static int index;
    static int len; 
    if (!state) {
	index = 0;
	len = strlen (text);
    }

    while((c = command_history[index])!= NULL) {
	index++;
	if (!strncmp(c, text, len)) {
	    return dupstr(c);
	} 
    }
    return (char *)NULL;
}
char **
custom_match_function(const char *str, rl_compentry_func_t *fun)
{
    size_t len, max, i, j, min;
    char **list, *match, *a, *b;

    len = 1;
    max = 10;
    if ((list = malloc(max * sizeof(*list))) == NULL)
	return NULL;

    while ((match = (*fun)(str, (int)(len - 1))) != NULL) {
	list[len++] = match;
	if (len == max) {
	    char **nl;
	    max += 10;
	    if ((nl = realloc(list, max * sizeof(*nl))) == NULL)
		goto out;
	    list = nl;
	}
    }
    if (len == 1)
	goto out;
    list[len] = NULL;
    if (len == 2) {
	if ((list[0] = strdup(list[1])) == NULL)
	    goto out;
	return list;
    }
    qsort(&list[1], len - 1, sizeof(*list),
	  (int (*)(const void *, const void *)) strcmp);
    min = 0xffffffffU;
    for (i = 1, a = list[i]; i < len - 1; i++, a = b) {
	b = list[i + 1];
	for (j = 0; a[j] && a[j] == b[j]; j++)
	    continue;
	if (min > j)
	    min = j;
    }
    if (min == 0 && *str) {
	if ((list[0] = strdup(str)) == NULL)
	    goto out;
    } else {
	if ((list[0] = malloc((min + 1) * sizeof(*list[0]))) == NULL)
	    goto out;
	(void)memcpy(list[0], list[1], min);
	list[0][min] = '\0';
    }
    return list;

out:
    free(list);
    return NULL;
}

char ** custom_complete_function( const char * command , 
				  int start, int end)
{
    char **found ;
    found =(char **) NULL ;
    if (start == 0) {
	found = custom_match_function((char*)command, &check_history);
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
 * Main function which executes the shell commands by forking child process 
 */
int
execute_shell_command(char **command)
{
    int status; 
    int ret;
    pid_t pid ; 

    /*
     * Fork a new process 
     */
    pid = fork();
    
    if (pid < 0 ) {
	printf("Not able to fork a new process \n");
    } else if (pid == 0) {
	/*
	 * Child process runs the command 
	 */
	ret = execvp(command[0], command);
	if (ret < 0) { 
	    printf("Command failed = %d\n", errno);
	}
    } else {
	if(signal(SIGALRM,signal_handler) == SIG_ERR)
	    perror("SIGALRM");
	if(almset)
	    alarm(5);
	/*
	 * Parent process 
	 * Wait for child to exit and then return 
	 */
	waitpid(pid,&status,0);
	if(almset)
	    alarm(0);
    }
    return TRUE;
}
int
execute_shell_command_with_redir(char **command, char* filename)
{

    int status;
    int fd;
    int ret;
    pid_t pid ;
    /*
     * Fork a new process for redirection op 
     */
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
	    printf("Command failed errno = %d", errno);
	}
    } else {
	if(signal(SIGALRM,signal_handler) == SIG_ERR)
	    perror("SIGALRM");
	if(almset)
	    alarm(5);
	/*
	 * Parent process 
	 * Wait for child to exit and then return 
	 */
	waitpid(pid,&status,0);
	if(almset)
	    alarm(0);
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
    int  token_index = 0; 
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

void signal_handler(int sig)
{
    if(sig == SIGALRM) {
	printf("5 secs over ,do u want to terminate command \n");
    }
    else if(sig == SIGINT){
	if (almset==0) {
	    almset=1;
	} else {
	    almset=0;
	}
    }
}
int find_num_tokens(char **token)
{
    int index = 0;
    while(token[index]) {
	index++;
    }
    return index;
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
    int index=0;
    int num_redir_op =0; 
    int redir_index =0;
    int num_tokens;
    struct termios oldterm;
    struct termios newterm;
    tcgetattr(0,&oldterm);

    signal(SIGINT,signal_handler);

    newterm=oldterm;
    newterm.c_cc[VEOF] = 3;
    newterm.c_cc[VINTR] = 4;
    tcsetattr(0,TCSANOW,&newterm);

    /*
     * Read the .profile file in the same directory and init shell with
     * path and other environment variables
     */
    read_profile_file();

    while (status) {
	print_shell_prompt();
	index = 0;
	redir_index  = 0;
	num_redir_op = 0;
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
	num_tokens = find_num_tokens(token);

	if (num_tokens == 0) {
	    /*
	     * Nothing entered in shell so continue 
	     */
	    continue;
	}
	if (!strncmp(token[0], "cd", 2)) 
	{
	    switch (num_tokens) {
	    case 1: chdir(homedir);
		    break;
	    case 2: if (chdir(token[1])) {
			printf(" Dir change not successfull errno =%d\n", errno);
		    }
		    break;
	    default:printf("command cd should have <= 1 operand\n");
		    break;
	    }
	    continue;
	}
	while (token[index]) {
	    if (!strcmp( token[index], ">=")) {
		/* there is a redirection operator */
		num_redir_op ++;
		redir_index = index;
	    }
	    index++;
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
