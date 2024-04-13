#include "threadpool.h"

#include <functional>
#include <thread>
#include <iostream>

const int TASK_MAX_THREASHHOLD = 1024;

// --------- 实现ThreadPool类

ThreadPool::ThreadPool() : 
    initThreadSize_(0), 
    taskSize_(0), 
    taskQueMaxThreshHold_(TASK_MAX_THREASHHOLD), 
    poolMode_(PoolMode::MODE_FIXED)
{}

ThreadPool::~ThreadPool()
{}

// 设置线程池的模式
void ThreadPool::setMode(PoolMode mode) {
    poolMode_ = mode;
}

// 设置任务队列最大阈值
void ThreadPool::setTaskQueMaxThreshHold(size_t Threshhold) {
    taskQueMaxThreshHold_ = Threshhold;
}

// 给线程池提交任务
void ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    // TODO
}

// 开启线程池
void ThreadPool::start(size_t initThreadSize) {
    // 赋值初始化线程数量，默认为4
    initThreadSize_ = initThreadSize;

    // 创建线程对象
    for (size_t i = 0; i < initThreadSize_; i ++) {
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this)));

    }

    for (size_t i = 0; i < initThreadSize_; i ++) {
        threads_[i]->start();
    }
}

// 因为我们要维护的描述变量都在ThreadPool类当中，所以我们需要一个Helper Function来供Thread来绑定使用
void ThreadPool::threadFunc() {
    std::cout << "begin threadFunc" << " tid: " << std::this_thread::get_id() << std::endl;

    std::cout << "end threadFunc"   << " tid: " << std::this_thread::get_id() << std::endl;
}


// --------- 实现Thread类

Thread::Thread(ThreadFunc func)
    : func_(func)
{}

Thread::~Thread()
{}

// 启动线程
void Thread::start() {
    // 创建线程
    std::thread t(func_);
    // 将线程分离
    t.detach();
}