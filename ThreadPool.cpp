#include "ThreadPool.h"

ThreadPool::ThreadPool(int numThreads) {
    for (int i = 0; i < numThreads; ++i) {
        threads_.emplace_back( [this]{
            while (1) {
                std::unique_lock<std::mutex> lck(mtx_);
                // 阻塞等待知道队列非空或终止
                condition_.wait(lck, [this] {
                    return !tasks_.empty() || is_stop_;
                } );

                if (is_stop_ && tasks_.empty()) {
                    // 如果任务队列里还有任务，则需要完成所有任务才能 stop
                    return;
                }

                // 取任务，释放锁，执行任务
                std::function<void()> task(std::move(tasks_.front()));
                tasks_.pop();
                lck.unlock();
                task();
            }
        } );
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lck(mtx_);
        is_stop_ = true;
    }

    // 通知所有线程工作，将队列中的任务处理完毕
    condition_.notify_all();
    for (auto &t : threads_) {
        t.join();
    }
}

template <typename F, typename... Args>
void ThreadPool::addTask(F &&f, Args &&...args) {
    std::function<void()> task = 
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    {
        std::unique_lock<std::mutex> lck(mtx_);
        tasks_.emplace(std::move(task));
    }
    condition_.notify_one();
}