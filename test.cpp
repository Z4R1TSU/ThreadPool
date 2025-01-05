#include "threadpool.h"

#include <iostream>
#include <chrono>
#include <thread>

#define ll long long

class MyTask : public Task {
public:
    MyTask(int a, int b) : a_(a), b_(b) {}

    Any run() {
        std::cout << "tid: " << std::this_thread::get_id() << std::endl << "thread start" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
        ll sum = 0;
        for (ll i = a_; i < b_; i ++) {
            sum += i;
        }
        return sum;

        std::cout << "thread end" << std::endl;
    }

private:
    int a_;
    int b_;
};

int main() {
    ThreadPool pool;
    // 先设置各种前置模式，再启动线程池
    pool.setMode(PoolMode::MODE_CACHED);
    pool.start(4);

    ll sum = 0;
    for (ll i = 1; i < 300000000; i ++) {
        sum += i;
    }
    std::cout << sum << std::endl;

    Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
    Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
    Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

    ll sum1 = res1.get().cast_<ll>();
    ll sum2 = res2.get().cast_<ll>();
    ll sum3 = res2.get().cast_<ll>();
    
    // Master-worker model 即主线程负责提交任务，子线程负责执行任务，主线程等待多个子线程执行完毕后再获取结果之和
    std::cout << (sum1 + sum2 + sum3) << std::endl;

    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());
    // pool.submitTask(std::make_shared<MyTask>());

    getchar();
}