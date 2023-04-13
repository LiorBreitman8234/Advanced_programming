#include "history.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

void write_to_history(int fd,char* command)
{
    int size_comm = strlen(command);
    int written_to_file = 0;
    while(written_to_file != size_comm)
    {
        written_to_file += write(fd,command,size_comm);
    }
    write(fd,"\n",1);
}

void get_n_line_from_end(int fd, int n,char* line)
{
    off_t file_size,curr_pos;
    char current;
    file_size = lseek(fd,0,SEEK_END);
    curr_pos = file_size -1;
    if(file_size == 0)
    {
        printf("no prev commands\n");
        return;
    }
    int counter = 0;
    while(curr_pos > 0)
    {
        lseek(fd,curr_pos,SEEK_SET);
        if(read(fd,&current, 1) == -1)
        {
            perror("read() error:");
            exit(1);
        }

        if(current == '\n')
        {
            if(counter != n)
            {
                counter++;
            }
            else
            {
                if(read(fd,&current, 1) == -1)
                {
                    perror("read() error:");
                    exit(1);
                }
                int i = 0;
                while(current != '\n')
                {
                    line[i] = current;
                    i++;
                    if(read(fd,&current, 1) == -1)
                    {
                        perror("read() error:");
                        exit(1);
                    }
                }
                line[i] = '\0';
                break;
            }
        }
        curr_pos--;
    }
}