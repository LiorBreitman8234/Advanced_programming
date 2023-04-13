#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

#include "history.h"
#include "variable.h"

void setGreeting(char* greeting, char* savein){
    int i = 0;
    while(i < strlen(greeting))
    {
        savein[i] = greeting[i];
        i++;
    }
    savein[i] = '\0';
}

void parse_command(char* command,int comm_len, int* piping, int* amount, char** parts)
{
    int i = 0;
    char * token = strtok (command," ");
    while (token != NULL)
    {
        parts[i] = token;
        token = strtok (NULL, " ");
        i++;
        if (token && ! strcmp(token, "|")) {
            (*piping) = 1;
            break;
        }
    }
    parts[i] = NULL;
    *(amount) = i;
}


void sigint_handler(int signum)
{
    printf("You typed Control-C!\n");
    return;
}


int main() {
char command[1024];
char *token;
int i;
char *outfile;
int fd, amper, redirect ,redirect_err, redirect_exist, status, argc1, piping, size_comm;
int* p_pipe = &piping;
int fildes[2];
char *argv1[10], *argv2[10];
char* commands_file = ".commands";
int comm_fd = -2;

var* variables = malloc(sizeof(var));
int amount_vars = 0;
int* p_amvars = &amount_vars;

int last_comm_stat = 0;

char greeting[128];
setGreeting("hello",greeting);

signal(SIGINT,sigint_handler);

while (1)
{   
    if(comm_fd == -2)
    {
        comm_fd = open(commands_file,O_CREAT|O_RDWR|O_APPEND,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    }   
    if(comm_fd == -1)
    {
        perror("open() error:");
        exit(1);
    }
    printf("%s: ",greeting);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    piping = 0;
    size_comm = strlen(command);
    
    /* Saving command to history file*/
    if(strcmp("!!",command))
    {
        write_to_history(comm_fd,command);
    }

    

    /* parse command line */
    int* p_argc1 = &argc1;
    parse_command(command,size_comm,p_pipe,p_argc1,argv1);

    /* Is command empty */
    if (argv1[0] == NULL){
        continue;
    }

    /* Quit */
    if(argc1 == 1 && !strcmp(argv1[0],"quit"))
    {
        break;
    }
    
    /* repeat last command*/
    char prev_comm[512];
    if(!strcmp(argv1[0],"!!")){
        get_n_line_from_end(comm_fd,1,prev_comm);
        parse_command(prev_comm,strlen(prev_comm),p_pipe,p_argc1,argv1);
    }

    

    /* echo varities*/
    if(!strcmp("echo",argv1[0]))
    {
        if(argc1 > 1 && !strcmp(argv1[1],"$?"))
        {
            printf("%d\n",last_comm_stat);
        }
        else if(argv1[1][0] == '$'){
            char str[strlen(argv1[1]-1)];
            strcpy(str,argv1[1]+1);
            for(int i =0; i < amount_vars;i++)
            {
                if(!strcmp(variables[i].title,str))
                {
                    printf("%s",variables[i].value);
                    break;
                }
            }
            printf("\n");
        }
        else{
            for(int i = 1; i < argc1; i++)
            {
                printf("%s ",argv1[i]);
            }
            printf("\n");
        }
        continue;
    }
    /* Change directory*/
    if(!strcmp(argv1[0],"cd")){
        if(argc1 > 1)
        {
            if(!strcmp(argv1[1],".."))
            {
                char cwd[512];
                if(getcwd(cwd,sizeof(cwd)) == NULL){
                    perror("getcwd error");
                    exit(1);
                }
                for(int i = strlen(cwd);i > 0;i--)
                {
                    if(cwd[i] == '/')
                    {
                        cwd[i] = '\0';
                        break;
                    }
                }
                if(chdir(cwd) != 0)
                {
                    perror("chdir() error");
                    exit(1);
                }
            }
            else {
                if(chdir(argv1[1]) != 0)
                {
                    perror("chdir() error");
                    exit(1);
                }
            }
        }
    }
    /* Change prompt*/
    if(argc1 == 3 && !strcmp(argv1[1],"=") && !strcmp("prompt",argv1[0]))
    {
        setGreeting(argv1[2],greeting);
    }

    /* Add variable */
    if(argc1 == 3 && !strcmp(argv1[1],"=") && argv1[0][0] == '$')
    {
        char* title = (char*)malloc(sizeof(char)*strlen(argv1[0]));
        char* value = (char*)malloc(sizeof(char)*strlen(argv1[2]+1));
        int i = 0;
        while(i+1 < strlen(argv1[0]))
        {
            title[i] = argv1[0][i+1];
            i++;
        }
        title[i] = '\0';

        i = 0;
        while(i < strlen(argv1[2]))
        {
            value[i] = argv1[2][i];
            i++;
        }
        value[i] = '\0';

        addVar(&variables,title,value,p_amvars);
        printf("%d\n", amount_vars);
        printVars(variables,amount_vars);
    }

    /* Read command */
    if(!strcmp(argv1[0],"read") && argc1 > 1)
    {
        char value[128];
        fgets(value, 128, stdin);
        value[strlen(value)-1] = '\0';
        for(int i =0;i < amount_vars;i++)
        {
            if(!strcmp(variables[i].title,argv1[1]))
            {
                variables[i].value = value;
                break;
            }
        }
        addVar(&variables,argv1[1],value,p_amvars);

        
    }
    /* Does command contain pipe */
    if (piping) {
        i = 0;
        while (token!= NULL)
        {
            token = strtok (NULL, " ");
            argv2[i] = token;
            i++;
        }
        argv2[i] = NULL;
    }
    /* Does command line end with & */ 
    if (! strcmp(argv1[argc1 - 1], "&")) {
        amper = 1;
        argv1[argc1 - 1] = NULL;
        }
    else 
        amper = 0; 

    if (argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">")) {
        redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];

        }
    else 
        redirect = 0;

    if(!redirect && argc1 > 1 && ! strcmp(argv1[argc1 - 2], "2>")) {
        redirect_err = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        }
    else 
        redirect_err = 0;

    if(!redirect && !redirect_err && argc1 > 1 && ! strcmp(argv1[argc1 - 2], ">>")){
        redirect_exist = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
        }
    else 
        redirect_exist = 0;

    /* for commands not part of the shell command language */ 
    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO); 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        } 
        if(redirect_err){
            fd = creat(outfile,0660);
            close(STDERR_FILENO);
            dup(fd);
            close(fd);
            /*stderr is noe redirected*/
        }
        if(redirect_exist){
            fd = open(outfile,O_CREAT|O_RDWR|O_APPEND);
            close (STDOUT_FILENO); 
            dup(fd); 
            close(fd); 
        }
        if (piping) {
            pipe (fildes);
            if (fork() == 0) { 
                /* first component of command line */ 
                close(STDOUT_FILENO); 
                dup(fildes[1]); 
                close(fildes[1]); 
                close(fildes[0]); 
                /* stdout now goes to pipe */ 
                /* child process does command */ 
                execvp(argv1[0], argv1);
            } 
            /* 2nd command component of command line */ 
            close(STDIN_FILENO);
            dup(fildes[0]);
            close(fildes[0]); 
            close(fildes[1]); 
            /* standard input now comes from pipe */ 
            if(execvp(argv2[0], argv2) == -1)
            {
                last_comm_stat = errno;
            }
        }   
        else{
            if(execvp(argv1[0], argv1) == -1){
                last_comm_stat = errno;
            }
        }
            
    }
    /* parent continues over here... */
    /* waits for child to exit if required */
    if (amper == 0)
        last_comm_stat = wait(&status);
}
if(amount_vars > 0)
{
    freeVars(variables,amount_vars);
    free(variables);
}

}
