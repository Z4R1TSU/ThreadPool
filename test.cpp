#include "threadpool.h"

#include <iostream>
#include <chrono>
#include <thread>

class MyTask : public Task {
public:
    MyTask(int a, int b)
        : a_(a), 
          b_(b) 
        {}

    Any run() {
        std::cout << "tid: " << std::this_thread::get_id() << std::endl << "thread start" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
        int sum = 0;
        for (size_t i = a_; i < b_; i ++) {
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
    pool.start(4);

    Result res = pool.submitTask(std::make_shared<MyTask>());

    int sum = res.get().cast_<long>();

    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());
    pool.submitTask(std::make_shared<MyTask>());

    getchar();
}