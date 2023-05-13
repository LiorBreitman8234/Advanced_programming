#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class CryptThreadPool {
public:
    explicit CryptThreadPool(size_t num_threads);
    ~CryptThreadPool();

    void enqueue(const std::function<void()>& task);

    void wait();
    bool stop;

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv;
};

#endif