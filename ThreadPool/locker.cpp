#include<iostream>
#include<condition_variable>
#include<thread>
#include<mutex>
/*比较lock_guard和unique_lock:前者是简单的作用域锁管理:锁的自动获取与释放;无需手动释放;不可复制
后者提供更灵活的互斥锁管理,可选的锁管理(比如延迟),支持条件变量,可移动但不可复制,更细粒度的控制
*/
std::mutex x_Mutex_;
auto printEvent=[](int x){
    if(x%2==0)
      {
           std::lock_guard<std::mutex>locker(x_Mutex_);//仅仅是构造函数
           std::cout<<x<<"is even"<<std::endl;

      }
};
std::mutex mtx;
std::condition_variable conV_;
bool ready=false;
auto printNum=[](int x){//wiat_for在超过时间后，仍会获取锁(结束阻塞状态)
    std::unique_lock<std::mutex>locker(mtx,std::defer_lock);//设置手动加锁释放锁
    // std::unique_lock<std::mutex>locker(mtx);//自动锁
    // locker.lock();
    // conV_.wait(locker,[]{return ready;});//等待ready变为true
    if(!conV_.wait_for(locker,std::chrono::seconds(5),[]{return ready;}))
    {
       std::cout<<"ready is false"<<std::endl;
    }
    // conV_.wait(locker,[]{return ready;});//wait会一直阻塞
    std::cout<<"Number:"<<x<<std::endl;
    // else{
    //     std::cout<<"Number:"<<x<<std::endl;
    // }
    
    // locker.unlock();
};

int main()
{
    std::thread t1(printEvent,2);
    std::thread t2(printEvent,4);
    t1.join();
    t2.join();
    std::thread t3(printNum,5);
    t3.join();
    return 0;
}