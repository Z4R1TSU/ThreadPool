#include "threadpool.h"

#include <functional>
#include <thread>
#include <iostream>

const int TASK_MAX_THREASHHOLD = 1024;

// --------- 实现ThreadPool类

ThreadPool::ThreadPool()
    : initThreadSize_(0), 
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
Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    // acquire lock: 在unique_lock构造的时候就已经获取了锁，当析构时也会隐式释放锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    
    // 线程通信 若现在的任务数量大于等于阈值，则进行等待 当等待时间超过1s就会强制停止(不能阻塞用户线程)
    // wait: 即一直等待，直到predict条件成立
    // wait_for: 相较于wait多了时间长度参数，如果条件一直不成立到设定时间长度便停止wait
    // wait_until: 相较于wait_for多了时间点参数，如果条件一直不成立到设定时间点便停止wait
    auto stat = notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool { return taskQue_.size() < static_cast<size_t>(taskQueMaxThreshHold_); });
    if (!stat) {
        std::cerr << "TimeOut: Task Queue is Full, sumbit task failed" << std::endl;
        return Result(sp, false);
    }

    // 将任务放入任务队列当中，并更新
    taskQue_.emplace(sp);
    taskSize_ ++;
    // 线程通信 既然放入了任务，那么任务队列肯定就不为空 通知线程执行任务队列当中的任务
    // 有wait就有notify_all，就像有constructor就有destructor一样
    notEmpty_.notify_all();
}

// 开启线程池
void ThreadPool::start(size_t initThreadSize) {
    // 赋值初始化线程数量，默认为4
    initThreadSize_ = initThreadSize;

    // 创建线程对象
    for (size_t i = 0; i < initThreadSize_; i ++) {
        // 这里是将ThreadPool类的threadFunc函数绑定到Thread类当中，供后者调用
        // 因为threadFunc本就是应该Thread调用的，只是因为需要维护的变量都在ThreadPool当中
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(ptr));
    }

    for (size_t i = 0; i < initThreadSize_; i ++) {
        threads_[i]->start();
    }
}

// 因为我们要维护的描述变量都在ThreadPool类当中，所以我们需要一个Helper Function来供Thread来绑定使用
void ThreadPool::threadFunc() {
    for (;;) {
        // acquire lock 创建一个unique_lock对象来管理互斥量taskQueMtx_
        std::unique_lock<std::mutex> lock(taskQueMtx_);

        // wait notEmpty 当任务队列为空，则等待任务出现再执行
        notEmpty_.wait(lock, [&]() -> bool { return taskSize_ > 0; });

        // 从任务队列取出一个任务
        std::cout << "pop a task and run it" << std::endl;
        auto task = taskQue_.front();
        taskQue_.pop();
        taskSize_ --;

        // 线程通信 通知线程池当前任务队列不空，消费者可以继续消费任务
        if (taskSize_ > 0) {
            notEmpty_.notify_all();
        }

        // 线程通信 通知线程池当前任务队列不满，生产者可以继续生产任务
        notFull_.notify_all();

        // release lock 否则当一个线程在执行任务的时候，其他任务队列中的任务都不会被取出并执行
        lock.unlock();

        // 调用线程执行任务
        if (task != nullptr) {
            task->exec();
        }
    }
    // std::cout << "begin threadFunc" << " tid: " << std::this_thread::get_id() << std::endl;

    // std::cout << "end threadFunc"   << " tid: " << std::this_thread::get_id() << std::endl;
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


// --------- 实现Task类
Task::Task()
    : result_(nullptr)
{}

void Task::exec() {
    if (result_ != nullptr) {
        result_->setVal(run());
    }
}

void Task::setResult(Result *res) {
    result_ = res;
}


// --------- 实现Result类
Result::Result(std::shared_ptr<Task> task, bool isValid)
    : task_(task),
      isValid_(isValid) {
    task_->setResult(this);
}

Any Result::get() {
    if (!isValid_) {
        return "";
    }

    sem_.wait(); // task任务如果没有被执行完毕，则等待其返回输出再捕获

    return std::move(any_);
}