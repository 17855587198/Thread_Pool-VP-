#include<iostream>
#include<mutex>
#include<condition_variable>
#include<thread>
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    // ģ�⹤����ʱ
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ready = true;
    cv.notify_one();  // ֪ͨ�ȴ����߳�
}

void wait_example() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });  // �ȴ�ֱ��readyΪtrue
    std::cout << "Condition is satisfied." << std::endl;
}

void wait_for_example() {
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(3), []{ return ready; })) {//���3�룬�������ټ��
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