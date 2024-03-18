#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    template<typename F, typename... Args>
    void addTask(F &&f, Args&&... args);

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;

    std::mutex mtx_;
    std::condition_variable condition_;

    bool is_stop_{false};
};

#endif