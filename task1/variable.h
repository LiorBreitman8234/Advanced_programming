#ifndef VARIABLE_H
#define VARIABLE_H

typedef struct var{
    char* title;
    char* value;
    struct var* next;
} var;


void freeVars(var* vars);
void printVars(var* vars);
void addVar(var** vars,char* title, char* value);
void findPrintVar(var* vars,char* title);
#endif