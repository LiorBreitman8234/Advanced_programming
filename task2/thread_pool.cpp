#include "thread_pool.h"

CryptThreadPool::CryptThreadPool(size_t num_threads): stop(false)
{
    for(size_t i =0; i < num_threads;i++)
    {
        threads.emplace_back([this](){
            std::function<void()> task;
            int task_counter;
            while(true){
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->cv.wait(lock,[this](){return this->stop || !this->tasks.empty();});
                if (this->stop && this->tasks.empty()){
                            std::cout << "thread " <<std::this_thread::get_id() << " did " << task_counter << " tasks!" << std::endl;
                            return;
                        }
                task = std::move(this->tasks.front());
                this->tasks.pop();
                task();
                std::cout << std::this_thread::get_id() << "did task " << ++task_counter << std::endl;
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
        std::cout << "added task "<< std::endl;
    }
    cv.notify_one();
    std::cout << "notified" << std::endl;
}

void CryptThreadPool::wait(){
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    cv.wait(lock, [this]() { return this->tasks.empty(); });
}