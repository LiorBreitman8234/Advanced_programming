#include "data_chunk.h"

DataChunk::DataChunk(char* buffer,int index)
{
    this->buffer = (char*)malloc(strlen(buffer));
    strncpy(this->buffer,buffer,strlen(buffer));
    this->buffer[strlen(buffer)] = '\0';
    status = 0;
    this->index = index;

}

DataChunk::~DataChunk() {
    free(this->buffer);

}
