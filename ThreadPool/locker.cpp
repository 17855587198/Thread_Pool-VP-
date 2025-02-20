#include<iostream>
#include<condition_variable>
#include<thread>
#include<mutex>
/*�Ƚ�lock_guard��unique_lock:ǰ���Ǽ򵥵�������������:�����Զ���ȡ���ͷ�;�����ֶ��ͷ�;���ɸ���
�����ṩ�����Ļ���������,��ѡ��������(�����ӳ�),֧����������,���ƶ������ɸ���,��ϸ���ȵĿ���
*/
std::mutex x_Mutex_;
auto printEvent=[](int x){
    if(x%2==0)
      {
           std::lock_guard<std::mutex>locker(x_Mutex_);//�����ǹ��캯��
           std::cout<<x<<"is even"<<std::endl;

      }
};
std::mutex mtx;
std::condition_variable conV_;
bool ready=false;
auto printNum=[](int x){//wiat_for�ڳ���ʱ����Ի��ȡ��(��������״̬)
    std::unique_lock<std::mutex>locker(mtx,std::defer_lock);//�����ֶ������ͷ���
    // std::unique_lock<std::mutex>locker(mtx);//�Զ���
    // locker.lock();
    // conV_.wait(locker,[]{return ready;});//�ȴ�ready��Ϊtrue
    if(!conV_.wait_for(locker,std::chrono::seconds(5),[]{return ready;}))
    {
       std::cout<<"ready is false"<<std::endl;
    }
    // conV_.wait(locker,[]{return ready;});//wait��һֱ����
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