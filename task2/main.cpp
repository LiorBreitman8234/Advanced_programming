#include <iostream>
#include <string>
#include "thread_pool.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "data_chunk.h"
#include <cstring>
#include <algorithm>
#include <cstdio>

int print_index = 0;


void print_thread(DataChunk* chunk, int fd){
    while(chunk->index != print_index){
           }

    size_t len = strlen(chunk->buffer);
    size_t written = 0;
    while(written != len)
    {
        written += write(fd,chunk->buffer,len);
        std::cout << std::endl;
    }
    print_index++;
}

void func_thread(DataChunk* chunk, void (*function)(char*,int), int key)
{
    function(chunk->buffer,key);
    chunk->status = 1;
}

int main() {

    std::vector<DataChunk*> toChange;
    std::vector<DataChunk*> toPrint;
    int in_fd = open("/home/bravo8234/CLionProjects/advanced_prog_2/test_func.txt",O_RDONLY);
    if(in_fd == -1)
    {
        perror("open");
        return 1;
    }

    int out_fd = open("/home/bravo8234/CLionProjects/advanced_prog_2/test_func_next.txt",O_RDWR | O_CREAT | O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
    if(out_fd== -1)
    {
        perror("open");
        return 1;
    }

    void* handle = dlopen("/home/bravo8234/CLionProjects/advanced_prog_2/libCodec.so",RTLD_LAZY);
    if(!handle){
        std::cerr << "error loading library " << dlerror() << std::endl;
        return 1;
    }

    // Get a pointer to the 'encrypt' function
    auto (*encrypt)(char*, int) = (void (*)(char*, int)) dlsym(handle, "encrypt");
    if (!encrypt) {
        std::cerr << "Error getting function pointer: " << dlerror() << std::endl;
        return 1;
    } 

    auto (*decrypt)(char*, int)= (void (*)(char*, int)) dlsym(handle, "decrypt");
    if (!decrypt) {
        std::cerr << "Error getting function pointer: " << dlerror() << std::endl;
        return 1;
    } 
    // Create a thread pool with 4 threads
    CryptThreadPool pool(4);

    int key = 2;
    std::string flag = "-d";
    size_t n = -1;
    int index = 0;
    char buffer[1024] = {'\0'};
    n = read(in_fd,buffer,15);
    while(n != 0)
    {   
        buffer[n] = '\0';
        std::cout << "got buffer " << buffer << std::endl;
        toChange.push_back(new DataChunk(buffer,index++));
        std::function<void()> f;
        if(flag == "-e")
        {
            f = std::bind(func_thread,toChange.back(),encrypt,key);
        }
        else if(flag == "-d")
        {
           f = std::bind(func_thread,toChange.back(),decrypt,key);
        }
        else
        {
            std::cout << " wrong flag" << std::endl;
            return 1;
        }
        memset(buffer,'\0',n);
        n = read(in_fd,buffer,100);
        pool.enqueue(f);
    }
    while(!toChange.empty())
    {
        for(DataChunk* chunk: toChange)
        {
            if(chunk->status == 1)
            {
                
                toPrint.push_back(chunk);
                int curr_index = chunk->index;
                std::function<void()> f = std::bind(print_thread,chunk,out_fd);
                pool.enqueue(f);
                //decrypt(chunk->buffer,key);
                toChange.erase(std::remove_if(toChange.begin(),toChange.end(),[&curr_index](DataChunk* inner){return inner->index==curr_index;}));
                //write(out_fd_de,chunk->buffer,strlen(chunk->buffer));
                break;
            }
        }
    }
    pool.stop = true;
    dlclose(handle);
    std::cout << "all finished" << std::endl;
    return 0;
}
