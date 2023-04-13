#include "variable.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

void freeVars(var* vars, int amount)
{
    for(int i =0; i < amount;i++)
    {
        free(vars[i].title);
        free(vars[i].value);
    }

}

void printVars(var* vars, int amount)
{
    for(int i = 0; i < amount;i++)
    {
        printf("%d: title: %s value:%s \n",i,vars[i].title,vars[i].value);
    }
}

void addVar(var** vars, char* title, char* value, int* amount)
{
    for(int i =0; i < (*amount);i++)
    {
        if(!strcmp(vars[i]->title,title))
        {
            vars[i]->value = value;
            return;
        }
    }
    var new_var;
    new_var.title = (char*)malloc(strlen(title));
    new_var.value = (char*)malloc(strlen(value));
    strcpy(new_var.title,title);
    strcpy(new_var.value,value);
    *vars = realloc(*vars,sizeof(var)*((*amount)+1));
    (*vars)[(*amount)++] = new_var;
    
}