#ifndef DATA_CHUNK_H
#define DATA_CHUNK_H

#include <iostream>
#include <string.h>
class DataChunk{
public:
    explicit DataChunk(char* buffer,int index);
    ~DataChunk();
    char* buffer;
    int status;//0 - nothing, 1 - encrypted/decrypted, 2 - printed
    int index;

};


#endif