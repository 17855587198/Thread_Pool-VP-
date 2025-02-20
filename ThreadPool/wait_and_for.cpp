#include<iostream>
#include<mutex>
#include<condition_variable>
#include<thread>
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    // 模拟工作耗时
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ready = true;
    cv.notify_one();  // 通知等待的线程
}

void wait_example() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });  // 等待直到ready为true
    std::cout << "Condition is satisfied." << std::endl;
}

void wait_for_example() {
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(3), []{ return ready; })) {//检查3秒，超过不再检查
        std::cout << "Timeout and condition not satisfied." << std::endl;
    } else {
        std::cout << "Condition is satisfied within the timeout." << std::endl;
    }
}
int main()
{
    worker();
    wait_example();
    wait_for_example();
    return 0;
}