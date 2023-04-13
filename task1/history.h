#ifndef HISTORY_H
#define HISTORY_H

void write_to_history(int fd,char* command);
char* get_from_history(int fd, int index_from_end);
void get_n_line_from_end(int fd, int n,char* line);

#endif