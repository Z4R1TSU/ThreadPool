#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

/*
example:
ThreadPool pool;
pool.start(4);

class MyTask : public Task {
public:
    void run() override {
        // do something
    }
};

std::shared_ptr<Task> sp = std::make_shared<MyTask>();
pool.submitTask(sp);
*/

// 因为虚函数和模板不相容，所以我们无法在子类进行重载能够接收任意类型的参数，这里手写C++-17引入的Any类型
// 模板类的函数都需写在头文件当中，这样才能在编译期间进行类型检查
class Any {
public:
    // 默认无参数的constructor and destructor
    Any() = default;
    ~Any() = default;
    // 禁用copy constructor and copy assignment 因为成员base_是unique_ptr
    Any (const Any&) = delete;
    Any& operator=(const Any&) = delete;
    // 启用右值构造和赋值
    Any (Any&&) = default;
    Any& operator=(Any&&) = default;

    // 重载构造函数，使得Any类可以接收任意类型的参数
    template<typename T>
    Any(T data) {
        base_ = std::make_unique<Derive<T>>(data);
    }

    // 重载类型转换运算符，使得Any类可以转换为任意类型的参数
    // 通过base_成员变量访问派生类当中的成员变量data_
    template<typename T>
    T cast_() {
        // 智能指针的get方法返回裸指针，需要用dynamic_cast转换为派生类指针
        Derive<T> *ptr = dynamic_cast<Derive<T>>(base_.get());
        if (ptr) {
            return ptr->data_;
        }
        throw std::bad_cast();
    }

private:
    // 基类实现
    class Base {
    public:
        // 虚函数的实现
        virtual ~Base() = default;
    };

    // 派生类实现
    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : public Base {}
        T data_;
    };

    // 基类指针
    std::unique_ptr<Base> base_;
};

class Semaphore {
public:
    Semaphore(int cnt = 0)
        : cnt_(cnt)
    {}
    ~Semaphore() = default;

    // 等待，信号量资源被使用而减少
    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        // 等待条件变量cv_，直到resLimit_大于0
        cv_.wait(lock, [&]() -> bool { return cnt_ > 0; });
        cnt_ --;
    }

    // 通知，返还一个信号量资源
    void post() {
        std::unique_lock<std::mutex> lock(mtx_);
        cnt_ ++;
        // 通知条件变量cv_，使得等待线程得以继续执行
        cv_.notify_all();
    }

private:
    int cnt_; // 信号量资源数量
    std::mutex mtx_;
    std::condition_variable cv_;
};

// 
class Result {
public:
    Result() = default;
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    // get方法 用于获取任务的结果
    Any get();

    // setVal方法 
private:
    Any any_; // 存储任务的结果
    Semaphore sem_; // 信号量，用于通知线程池任务完成
    std::shared_ptr<Task> task_; // 任务的共享指针
    std::atomic_bool isValid_; // 标记返回值是否有效
};

class Task {
public:
    // 可自定义重载run类型
    virtual Any run() = 0;
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
    Result submitTask(std::shared_ptr<Task> sp);

    // 禁用(copy construct)拷贝构造，如`ThreadPool a = ThreadPool()`
    ThreadPool(const ThreadPool&) = delete;
    // 禁用(copy assignment)拷贝赋值、实例赋值，如`ThreadPool b = a`
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    // Thread类当中的method并不能操作ThreadPool当中维护的变量，这个threadFunc相当于是个桥梁
    // 因为我们要维护的描述变量都在ThreadPool类当中，所以我们需要一个Helper Function来供Thread来绑定使用
    void threadFunc();

private:
    // 使用智能指针，使得当threads_在析构时，自动释放指针的资源
    std::vector<std::unique_ptr<Thread>> threads_; // 线程池本身
    size_t initThreadSize_; // 初始线程数量
    std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
    
    // 原子操作 保证线程安全 轻量的锁 适用于计数器
    std::atomic_uint taskSize_ {}; // 记录任务的数量
    size_t taskQueMaxThreshHold_; // 任务数量的最大阈值

    // 线程安全
    std::mutex taskQueMtx_; // 任务队列的互斥锁
    std::condition_variable notFull_ {}; // 任务队列不满
    std::condition_variable notEmpty_ {}; // 任务队列不空

    // 当前线程池的工作模式
    PoolMode poolMode_;
};

#endif