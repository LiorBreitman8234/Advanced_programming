#include <iostream>
#include <string>
#include "thread_pool.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include "data_chunk.h"
#include <cstring>
#include <algorithm>
#include <chrono>
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
    }
    print_index++;
}

void func_thread(DataChunk* chunk, void (*function)(char*,int), int key)
{
    function(chunk->buffer,key);
    chunk->status = 1;
}

void print_use()
{
    std::cout << "Use example:" << std::endl;
    std::cout << "1. Coder key {-e | -d} < input_file > output_file" << std::endl;
    std::cout << "2. pipe |  Coder key {-e | -d} > output_file" << std::endl;
    std::cout << "3. cat file | Coder key {-e | -d} > output_file" << std::endl;
}

int main(int argc, char** argv) {
    std::vector<DataChunk*> toChange;
    std::vector<DataChunk*> toPrint;
    long key;
    std::string flag;
    std::string in_file;
    std::string out_file;
    int in_fd = 0;
    int out_fd = 1;
    if(argc < 3)
    {
        std::cout << "wrong usage!" << std::endl;
        print_use();
        return 1;
    }
    else
    {
        key = strtol(argv[1],nullptr,10);
        flag = argv[2];
    }


    void* handle = dlopen("./libCodec.so",RTLD_LAZY);
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

    CryptThreadPool pool(8);


    size_t n;
    int index = 0;
    char buffer[1024] = {'\0'};
    auto start = std::chrono::system_clock::now();
    auto local_time = std::chrono::system_clock::to_time_t(start);
    pool.log <<"staring time: " << std::ctime(&local_time) << "\n";
    n = read(in_fd,buffer,1);
    while(n != 0)
    {   
        buffer[n] = '\0';
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
        n = read(in_fd,buffer,1);
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
                toChange.erase(std::remove_if(toChange.begin(),toChange.end(),[&curr_index](DataChunk* inner){return inner->index==curr_index;}));
                break;
            }
        }
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end-start;
    auto end_time = std::chrono::system_clock::to_time_t(end);
    pool.stop = true;
    pool.log << "end time: " << std::ctime(&end_time) << '\n';
    pool.log <<"total time: " << elapsed.count() << " s\n";
    dlclose(handle);
    return 0;
}
