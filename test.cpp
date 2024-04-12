#include "threadpool.h"

#include <iostream>
#include <chrono>
#include <thread>

int main() {
    ThreadPool pool;
    pool.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
}