#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() {
char command[1024];
char *token;
int i;
char *outfile;
int fd, amper, redirect ,redirect_err, redirect_exist, piping, retid, status, argc1;
int fildes[2];
char *argv1[10], *argv2[10];
char* commands_file = ".commands";

while (1)
{
    //comm_fd = creat(commands_file,);
    printf("hello: ");
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    piping = 0;

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv1[i] = token;
        token = strtok (NULL, " ");
        i++;
        if (token && ! strcmp(token, "|")) {
            piping = 1;
            break;
        }
    }
    argv1[i] = NULL;
    argc1 = i;

    /* Is command empty */
    if (argv1[0] == NULL)
        continue;

    if(!strcmp(argv1[0],"cd"))
        {
            if(argc1 > 1)
            {
                if(!strcmp(argv1[1],".."))
                {
                    char cwd[512];
                    if(getcwd(cwd,sizeof(cwd)) == NULL){
                        perror("getcwd error");
                        exit(1);
                    }
                    for(int i = strlen(cwd);i--;i == 0)
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
            execvp(argv2[0], argv2);
        } 
        
        else
            execvp(argv1[0], argv1);
    }
    /* parent continues over here... */
    /* waits for child to exit if required */
    if (amper == 0)
        retid = wait(&status);
}
}
