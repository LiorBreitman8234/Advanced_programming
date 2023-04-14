#include "variable.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

void freeVars(var* vars)
{
    while(vars != NULL)
    {
        var* temp = vars;
        vars = temp->next;
        free(temp->value);
        free(temp->title);
        free(temp);
    }

}

void printVars(var* vars)
{
        while (vars != NULL) {
        printf("%s = %s\n", vars->title, vars->value);
        vars = vars->next;
    }
}

void addVar(var** vars,char* title, char* value)
{
    var* current = *vars;
    while(current != NULL)
    {
        if(!strcmp(current->title,title))
        {
            current->value = value;
            return;
        }
        current = current->next;
    }

    var* new_node = malloc(sizeof(var));
    if(new_node == NULL)
    {
        printf("error allocating");
        exit(1);
    }

    new_node->title = malloc(strlen(title)+1);
    if(new_node->title == NULL)
    {
        printf("error allocating");
        exit(1);
    }
    strcpy(new_node->title,title);


    new_node->value = malloc(strlen(value)+1);
    if(new_node->value == NULL)
    {
        printf("error allocating");
        exit(1);
    }
    strcpy(new_node->value,value);
    new_node->next = NULL;

    if(*vars == NULL)
    {
        *vars = new_node;
    }
    else{
        current = *vars;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }
    
}

void findPrintVar(var* vars,char* title)
{
    while(vars != NULL)
    {
        if(!strcmp(title,vars->title))
        {
            printf("%s",vars->value);
        }
        vars = vars->next;
    }
    printf("\n");
}