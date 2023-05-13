#include "thread_pool.h"

CryptThreadPool::CryptThreadPool(size_t num_threads): stop(false)
{
    this->log = std::ofstream(".log.txt",std::ofstream::trunc);
    if(!log.is_open())
    {
        std::cout << "error logging file" << std::endl;
        exit(1);
    }
    for(size_t i =0; i < num_threads;i++)
    {
        threads.emplace_back([this](){
            std::function<void()> task;
            int task_counter;
            while(true){
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->cv.wait(lock,[this](){return this->stop || !this->tasks.empty();});
                if (this->stop && this->tasks.empty()){
                            this->log << "thread " <<std::this_thread::get_id() << " did " << task_counter << " tasks!\n";
                            return;
                        }
                task = std::move(this->tasks.front());
                this->tasks.pop();
                task();
                task_counter++;
            }
            
        });
    }
}

CryptThreadPool::~CryptThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    cv.notify_all();
    for (std::thread &thread : threads) {
        thread.join();
    }
}

void CryptThreadPool::enqueue(const std::function<void()>& task)
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        tasks.push(task);
    }
    cv.notify_one();
}

void CryptThreadPool::wait(){
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    cv.wait(lock, [this]() { return this->tasks.empty(); });
}