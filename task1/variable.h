#ifndef VARIABLE_H
#define VARIABLE_H

typedef struct{
    char* title;
    char* value;
} var;

void freeVars(var* vars, int amount);
void printVars(var* vars, int amount);
void addVar(var** vars, char* title, char* value, int* amount);
#endif