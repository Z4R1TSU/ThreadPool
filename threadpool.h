#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

class Task {
public:
    // 可自定义重载run类型
    virtual void run() = 0;
};

enum class PoolMode {
    MODE_FIXED, 
    MODE_CACHED, 
};

class Thread {
public:
    using ThreadFunc = std::function<void()>;

    // 线程的构造与析构
    Thread(ThreadFunc func);
    ~Thread();

    // 启动指定线程
    void start();
private:
    ThreadFunc func_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    // 开启线程池
    void start(size_t initThreadSize = 4);
    
    // 设置线程池的模式
    void setMode(PoolMode mode);
    
    // 设置任务队列最大阈值
    void setTaskQueMaxThreshHold(size_t Threshhold);
    
    // 给线程池提交任务
    void submitTask(std::shared_ptr<Task> sp);

    // 禁用拷贝构造，如`ThreadPool a = ThreadPool()`
    ThreadPool(const ThreadPool&) = delete;
    // 禁用实例复制，如`ThreadPool b = a`
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    // Thread类当中的method并不能操作ThreadPool当中维护的变量，这个threadFunc相当于是个桥梁
    // 因为我们要维护的描述变量都在ThreadPool类当中，所以我们需要一个Helper Function来供Thread来绑定使用
    void threadFunc();

private:
    // 线程池本身
    std::vector<Thread*> threads_;
    // 初始线程数量
    size_t initThreadSize_;
    // 任务队列
    std::queue<std::shared_ptr<Task>> taskQue_;
    // 记录任务的数量
    std::atomic_uint taskSize_ {};
    // 任务数量的最大阈值
    size_t taskQueMaxThreshHold_;

    // 线程安全
    // 保证任务队列是互斥的
    std::mutex taskQueMtx_;
    // 任务队列不满
    std::condition_variable notFull_ {};
    // 任务队列不空
    std::condition_variable notEmpty_ {};

    // 当前线程池的工作模式
    PoolMode poolMode_;
};

#endif