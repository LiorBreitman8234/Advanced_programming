#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <ctype.h>
#include "history.h"
#include "variable.h"


typedef struct Command{
    char** argv;
    int argc;
    char* input_file;
    char* output_file;
    int output_type; //regular output 1, stderr 2, 0 for no redirect,3 for append
    int is_background; // for &
    struct Command* next;
}Command;


char* COMM_FILE = ".commands";
int COMM_FD = 1000;
int LAST_STAT = 0;
var* VARS;
Command* HEAD;
char greeting[128];

void free_list(Command* head);
void print_list(Command* head);
void execute_list(Command* head);
Command* create_node(char** argv,int argc,char* input_file, char* output_file,int output_type,int is_background);
void add_node(Command** head,Command* new);


void if_proccess(char* line);
void my_echo(char* argv[],int* amount);
int count_commands(char* line);
void single_command(char* argv[],int* amount);
void procces_command_line(char* line);
Command* parse_line(char* line);

void setGreeting(char* greeting, char* savein);

void parse_command(char* command, int* amount, char** parts);

void my_cd(char* argv[],int amount);

void sigint_handler(int signum)
{
    printf("You typed Control-C!\n");
    return;
}


int main() {
int comm_fd = -2;
 if(comm_fd == -2)
{
    comm_fd = open(COMM_FILE,O_CREAT|O_RDWR|O_APPEND,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}   
if(comm_fd == -1)
{
    perror("open() error:");
    exit(1);
}


dup2(comm_fd,COMM_FD);
close(comm_fd);
char command[1024];
int argc1, size_comm;
char *argv1[10];


VARS = NULL;
HEAD = NULL;
int amount_vars = 0;


setGreeting("hello",greeting);
signal(SIGINT,sigint_handler);

while (1)
{   
   
    printf("%s: ",greeting);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';
    size_comm = strlen(command);
    
    /* Saving command to history file*/
    if(strcmp("!!",command) && !(size_comm == 0))
    {
        write_to_history(COMM_FD,command);
    }

    
    char command_copy[1024];
    strcpy(command_copy,command);
    /* parse command line */
    int* p_argc1 = &argc1;
    parse_command(command,p_argc1,argv1);

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
        get_n_line_from_end(COMM_FD,1,prev_comm);
        strcpy(command_copy,prev_comm);
    }

    procces_command_line(command_copy);

}
    if(amount_vars > 0){
        freeVars(VARS);
    }

}

void my_cd(char* argv[],int amount)
{
    if(amount > 1){
        if(!strcmp(argv[1],"..")){
            char cwd[512];
            if(getcwd(cwd,sizeof(cwd)) == NULL){
                    perror("getcwd error");
                    return;
            }
            for(int i = strlen(cwd);i > 0;i--){
                if(cwd[i] == '/')
                {
                    cwd[i] = '\0';
                    break;
                }
            }
            if(chdir(cwd) != 0){
                perror("chdir() error");
                return;
            }
        }
        else {
            if(chdir(argv[1]) != 0){
                perror("chdir() error");
                    return;
                }
            }
        }

}
void my_echo(char* argv[],int* amount)
{
    if((*amount) > 1 && !strcmp(argv[1],"$?"))
    {
        printf("%d\n",LAST_STAT);
    }
    else if(argv[1][0] == '$'){
            char str[strlen(argv[1]-1)];
            strcpy(str,argv[1]+1);
            findPrintVar(VARS,str);
    }
    else{
        for(int i = 1; i < (*amount); i++)
        {
            printf("%s ",argv[i]);
        }
        printf("\n");
    }
}

int count_commands(char* line)
{
    int num = 1;
    for(int i =0;i< strlen(line);i++)
    {
        if(line[i] == '|')
        {
            num++;
        }
    }
    return num;
}

void single_command(char* argv[],int* amount)
{
    /* Is command empty */
    if (argv[0] == NULL){
    }
    
    /* Quit */
    if((*amount) == 1 && !strcmp(argv[0],"quit"))
    {
        exit(1);
    }

    /* repeat last command*/
    char prev_comm[512];
    if(!strcmp(argv[0],"!!")){
        get_n_line_from_end(COMM_FD,1,prev_comm);
        parse_command(prev_comm,amount,argv);
    }


     /* echo varities*/
    if(!strcmp("echo",argv[0]))
    {
        if((*amount)  > 1 && !strcmp(argv[1],"$?"))
        {
            printf("%d\n",LAST_STAT);
        }
        else if(argv[1][0] == '$'){
            char str[strlen(argv[1]-1)];
            strcpy(str,argv[1]);
            findPrintVar(VARS,str);
        }
        else{
            for(int i = 1; i < (*amount) ; i++)
            {
                printf("%s ",argv[i]);
            }
            printf("\n");
        }
    }
    

}

void procces_command_line(char* line)
{
    if(!strncmp("if ",line,3))
    {
        /* if statment*/
        if_proccess(line+3);
        return;
    }
    HEAD = parse_line(line);
    //print_list(HEAD);
    execute_list(HEAD);
    free_list(HEAD);
    HEAD = NULL;

}

void setGreeting(char* greeting, char* savein){
    int i = 0;
    while(i < strlen(greeting))
    {
        savein[i] = greeting[i];
        i++;
    }
    savein[i] = '\0';
}

void parse_command(char* command, int* amount, char** parts)
{
    int i = 0;
    char * token = strtok (command," ");
    while (token != NULL)
    {
        parts[i] = token;
        token = strtok (NULL, " ");
        i++;
        // if (token && ! strcmp(token, "|")) {
        //     (*piping) = 1;
        //     break;
        // }
    }
    parts[i] = NULL;
    *(amount) = i;
}

Command* create_node(char** argv, int argc, char* input_file, char* output_file,int output_type,int is_background)
{
    Command* new_command = (Command*)malloc(sizeof(Command));
    new_command->argv = (char**)malloc(sizeof(char*)*10);
    for(int i =0; i < argc;i++)
    {
        new_command->argv[i] = (char*)malloc(strlen(argv[i]));
        strcpy(new_command->argv[i],argv[i]);
    }
    new_command->argc = argc;

    if(input_file != NULL)
    {
        new_command->input_file = (char*)malloc(strlen(input_file));
        strcpy(new_command->input_file,input_file);
    }

    if(output_file != NULL)
    {
        new_command->output_file = (char*)malloc(strlen(output_file));
        strcpy(new_command->output_file,output_file);
        new_command->output_type = output_type;
    }
    new_command->next = NULL;
    new_command->is_background = is_background;
    return new_command;
}

void add_node(Command** head,Command* new)
{
    if(*head == NULL)
    {
        *head = new;
    }
    else{
        Command* current = *head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = new;
    }
}

void free_list(Command* head) {
    Command* current_node = head;
    while (current_node != NULL) {
        Command* next_node = current_node->next;
        int i;
        for (i = 0; i < current_node->argc; i++) {
            free(current_node->argv[i]);
        }
        free(current_node->argv);
        free(current_node->input_file);
        free(current_node->output_file);
        current_node->argv = NULL;
        current_node->input_file = NULL;
        current_node->output_file = NULL;
        free(current_node);
        current_node = next_node;
    }
}

void execute_list(Command* head)
{
    int pipe_fd[2];
    int prev_pipe_fd[2] = {-1, -1};
    int error = 0;
    int backup_stdin = 0;
    int backup_stdout = 0;
    int backup_stderr = 0;
    Command* current = head;
    while( current!= NULL)
    {
        if(error)
        {
            break;
        }
        // create pipe if needed;
        if(current->next != NULL)
        {
            if(pipe(pipe_fd) == -1)
            {
                perror("pipe(): ");
                exit(1);
            }
        }

        //fork chile process
        pid_t pid = fork();

        if(pid == -1)
        {
            perror("fork() ");
            exit(1);
        }
        else if(pid == 0)
        {
            //child process

            //redirect input if necessary
            if(current->input_file != NULL)
            {
                int input_fd = open(current->input_file, O_RDONLY);
                if(input_fd == -1)
                {
                    perror("open() ");
                    exit(1);
                }
                backup_stdin = dup(STDIN_FILENO);
                if(dup2(input_fd,STDIN_FILENO) == -1)
                {
                    perror("dup2() ");
                    exit(1);
                }
                close(input_fd);
            }

            //redirect output if necessary
            if(current->output_file != NULL)
            {
                int output_fd = -1;
                if(current->output_type == 1 || current->output_type == 2){
                    output_fd = open(current->output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                }
                else if(current -> output_type == 3)
                {
                    output_fd = open(current->output_file,  O_CREAT | O_WRONLY | O_APPEND, 0644);
                }
                if(output_fd == -1)
                {
                    perror("open()");
                    exit(1);
                }
                if(current->output_type == 2)
                {
                    backup_stderr = dup(STDERR_FILENO);
                    if(dup2(output_fd,STDERR_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }
                else
                {
                    backup_stdout = STDOUT_FILENO;
                    if(dup2(output_fd,STDOUT_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }
                close(output_fd);
                
            }


            // set up input file descriptor from previous command in pipeline
            if (prev_pipe_fd[0] != -1) {
                dup2(prev_pipe_fd[0], STDIN_FILENO);
                close(prev_pipe_fd[0]);
                close(prev_pipe_fd[1]);
            }

            // set up output file descriptor to next command in pipeline
            if (pipe_fd[1] != -1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }

            //echo varaities
            if(!strcmp("echo",current->argv[0]))
            {
                my_echo(current->argv,&current->argc);
                current = current->next;
            }
            /* Change directory*/
            else if(!strcmp(current->argv[0],"cd")){
                my_cd(current->argv,current->argc);
                current = current->next;
            }
            /* Add variable */
            else if(current->argc == 3 && !strcmp(current->argv[1],"=") && current->argv[0][0] == '$')
            {
                char* title = (char*)malloc(sizeof(char)*strlen(current->argv[0])+1);
                char* value = (char*)malloc(sizeof(char)*strlen(current->argv[2])+1);
                int i = 0;
                while(i+1 < strlen(current->argv[0]))
                {
                    title[i] = current->argv[0][i+1];
                    i++;
                }
                title[i] = '\0';

                i = 0;
                while(i < strlen(current->argv[2]))
                {
                    value[i] = current->argv[2][i];
                    i++;
                }
                value[i] = '\0';

                addVar(&VARS,title,value);
                
                free(title);
                free(value);
                current = current->next;
            }
            /* Read command */
            else if(!strcmp(current->argv[0],"read") && current->argc > 1)
            {
                char value[128];
                fgets(value, 128, stdin);
                value[strlen(value)-1] = '\0';
                addVar(&VARS,current->argv[1],value);
                current = current->next;
            }
            /* Change prompt*/
            else if(current->argc == 3 && !strcmp(current->argv[1],"=") && !strcmp("prompt",current->argv[0]))
            {
                setGreeting(current->argv[2],greeting);
                current = current->next;
            }
            else{
                execvp(current->argv[0],current->argv);
                error = 1;
                perror("execvp");
                LAST_STAT = errno;
            }
        }
        else
        {
            //parent

            // close input file descriptor from previous command in pipeline
            if (prev_pipe_fd[0] != -1) {
                close(prev_pipe_fd[0]);
                close(prev_pipe_fd[1]);
            }
            
             // save output file descriptor for next command in pipeline
            if (pipe_fd[1] != -1) {
                prev_pipe_fd[0] = pipe_fd[0];
                prev_pipe_fd[1] = pipe_fd[1];
            }

            //wait for child
            if(current->is_background == 0)
            {
                waitpid(pid,NULL, 0);
            }
            current = current->next;
        }
    }
    dup2(backup_stderr,STDERR_FILENO);
    dup2(backup_stdin,STDIN_FILENO);
    dup2(backup_stdout,STDOUT_FILENO);
}

void print_list(Command* head) {
    Command* current_node = head;
    while (current_node != NULL) {
        printf("Args:");
        for (int i = 0; i < current_node->argc; i++) {
            printf(" %s", current_node->argv[i]);
        }
        printf("\n");
        printf("Input file: %s\n", current_node->input_file);
        printf("Output file: %s\n", current_node->output_file);
        printf("\n");
        current_node = current_node->next;
    }
}


char* strtrim(char *str) {
   size_t len = strlen(str);
   size_t start = 0;
   while (isspace(str[start])) start++;
   size_t end = len - 1;
   while (end > start && isspace(str[end])) end--;
   str[end + 1] = '\0';
   return str + start;
}


Command* parse_line(char* line)
{
    int amount = 1;
    for(int i = 0; i < strlen(line);i++)
    {
        if(line[i] == '|')
        {
            amount++;
        }
    }

    char** commands = (char**)malloc(sizeof(char*)*amount);
    char* token;
    token = strtok(line,"|");
    int i =0;
    while(token != NULL)
    {   
        token = strtrim(token);
        commands[i] = (char*)malloc(strlen(token)+1);
        strcpy(commands[i],token);
        i++;
        token = strtok(NULL,"|");
        
    }

    Command* head = NULL;
    
    for(i =0; i < amount; i++)
    {
        char* args[11];
        int num_args = 0;
        char* input_file = NULL;
        char* output_file = NULL;
        int output_type = 0;
        int background = 0;

        char* arg = strtok(commands[i], " \t\n");
        while (arg != NULL && num_args < 10) {
            args[num_args] = arg;
            num_args++;
            arg = strtok(NULL," \t\n");
        }
        if (num_args > 11) {
            printf("too many args");
            return NULL;
        }

        if (args[num_args - 2] != NULL) {
            if (!strcmp(args[num_args - 2], "<")) {
                if (args[num_args - 1] == NULL) {
                    printf("missing file");
                    return NULL;
                }
                input_file = strdup(args[num_args - 1]);
                num_args -= 2;
            } else if (!strcmp(args[num_args - 2], ">")) {
                if (args[num_args - 1] == NULL) {
                    printf("missing file");
                    return NULL;
                }
                output_file = strdup(args[num_args - 1]);
                num_args -= 2;
                output_type = 1;
            } else if (!strcmp(args[num_args - 2], ">>")) {
                if (args[num_args - 1] == NULL) {
                    printf("missing file");
                    return NULL;
                }
                output_file = strdup(args[num_args - 1]);
                num_args -= 2;
                output_type = 3;
            } else if (!strcmp(args[num_args - 2], "2>")) {
                if (args[num_args - 1] == NULL) {
                    printf("missing file");
                    return NULL;
                }
                output_file = strdup(args[num_args - 1]);
                num_args -= 2;
                output_type = 2;
            }
        }

        if (!strcmp(args[num_args - 1], "&")) {
            background = 1;
            num_args--;
        }

        args[num_args] = NULL;
        Command* new_node = create_node(args, num_args, input_file, output_file, output_type, background);
        if (new_node == NULL) {
            printf("error creating node");
            return NULL;
        }

        add_node(&head, new_node);
    }
    return head;
}

void if_proccess(char* line)
{
    Command* cond = parse_line(line);

    char then[5];
    fgets(then,5,stdin);
    getchar();
    if(strncmp(then,"then",4)){
        printf("no then: wrong if-else syntax\n");
        free_list(cond);
        return;
    }
    printf("enter command for yes\n");
    char comm[1024];
    fgets(comm,1024,stdin);
    comm[strlen(comm)-1] = '\0';
    Command* yes = parse_line(comm);



    char else_com[5];
    fgets(else_com,5,stdin);
    getchar();
    if(strncmp(else_com,"else",4)){
        printf("no else: wrong if-else syntax\n");
        free_list(cond);
        free_list(yes);
        return;
    }

    printf("enter command for no\n");
    fgets(comm,1024,stdin);
    comm[strlen(comm)-1] = '\0';
    Command* no = parse_line(comm);

    execute_list(cond);
    if(LAST_STAT == 0)
    {
        execute_list(yes);
    }
    else{
        execute_list(no);
    }
    free_list(cond);
    free_list(yes);
    free_list(no);

}