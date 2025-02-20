#include<iostream>
#include"threadpool.h"//Win上对于头文件的大小写并不敏感
#include<functional>
#include<thread>
#include<mutex>
#include<memory>
const int TASK_MAX_THRESHHOLD=1024;//可以将任务队列的数量减少
const int THREAD_SIZE_THRESHHOLD_=20;
const int THREAD_MAX_TIME=60;
using namespace  std;
ThreadPool::ThreadPool()
      :initThreadSize(4),
      threadSizeThreshHold_(THREAD_SIZE_THRESHHOLD_),//线程的上限
       taskSize(0),
       taskSizeMaxThreshhold(TASK_MAX_THRESHHOLD),
       poolMode(PoolMode::MODE_FIXED),
       curThreadSize(0),
       idleThreadSize_(0)//起初无空闲线程
{
    cout<<"the constructor function of ThreadPoool is called!"<<endl;
}
ThreadPool::~ThreadPool()
{
  isStartPool_=false;//设置为false 
  std::unique_lock<mutex>locker(taskQueueMtx);//配合锁进行使用   
  notEmpty.notify_all();
  exitCond_.wait(locker,[&]()->bool{return threads.size()==0;});//如果等于0不等待,否则等待并析构(不为0则释放)

  //线程池中的线程返回有两种状态:阻塞或者正在执行任务中

  cout<<"the deconstructor function of ThreadPool is called!"<<endl;

}
bool ThreadPool::checkRunningState() const
{
   return isStartPool_;//判断是否有线程运行
}
void ThreadPool::setMode(PoolMode mode){
  if(checkRunningState())//已经运行了不再设置模式
     return;
    poolMode=mode;
}
void ThreadPool::InitThreadSize(int size)
{
   this->initThreadSize=size;
}
void ThreadPool::setThreadSizeThresHold(int threshHold)
{
  /*线程池若已经启动则不允许设置*/
   if(checkRunningState())
      return;
    if(poolMode==PoolMode::MODE_CACHED)//再一次检查模式
       threadSizeThreshHold_=threshHold;//设置线程的最大数量
}
void ThreadPool::setTaskQueueThresHold(int threshhold)
{
   if(checkRunningState())
     return;
    taskSizeMaxThreshhold=threshhold;
}
Result ThreadPool::submitTask(shared_ptr<Task> sp)//线程池提交任务
{
    //获取锁
    unique_lock<mutex>locker(taskQueueMtx);//获取任务队列的锁
    // while(task_list.size()==taskSizeMaxThreshhold)//达到上限,进入等待态，释放锁.唤醒后再抢锁才能变为就绪态
    // {
    //     notFull.wait(locker);//当前线程进入等到状态，也释放当前的锁.等待是一种阻塞体现
    // }
    //任务提交任务最长不能阻塞一秒,否则提交失败，返回
    if(!notFull.wait_for(locker,chrono::seconds(1),[&]()->bool{return task_list.size()<taskSizeMaxThreshhold;}))//lambda表达式,&是捕获表达式
    {//此处取反是与本意相反
  
       //只等待1秒
       cerr<<"Fail to submit the task!"<<endl;//时间过长就提交任务失败
       return Result(sp,false);//返回正确类型的任务,提交失败则返回无效的返回值
    }
    //比较wait-->一直等，直到满足,wait_for-->(等待时间),wait_until-->(等到什么时刻)
    task_list.emplace(sp);//放入任务
    taskSize++;
    // notFull.wait(locker,[&]()->bool{return task_list.size()==taskSizeMaxThreshhold;})//lambda表达式,&是捕获表达式
    //线程的通信,等待任务队列的空余,若空余,则继续放入
    //放入了新任务,则队列不空,对notEmpty通知
    notEmpty.notify_all();//唤醒消费,分配线程执行任务
    //返回Task的Result对象
    // return task->getresult;//不合适
    // return Result(task);
    /*根据任务的数量和空闲线程数量动态创建线程(cached模式):小而快的任务*/
    if(poolMode==PoolMode::MODE_CACHED&&taskSize>=idleThreadSize_&&curThreadSize<threadSizeThreshHold_)
    {
      //创建新的线程
    auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this,placeholders::_1));//创建新线程
    int threadId_=ptr->getThreadId();
    threads.emplace(threadId_,move(ptr));
    threads[threadId_]->start();//启动线程
    curThreadSize++;//修改当前线程的数量
    idleThreadSize_++;//空闲线程增加
    //  auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this));
    //  threads.emplace_back(move(ptr));//unique_ptr不允许资源的复制的

    }

    return Result(sp);//默认的是true


}
void ThreadPool::Start(int initThreadSize)//可以默认设置(这是线程池的方法实现)
{
  isStartPool_=true;//线程池启动的状态
  initThreadSize=initThreadSize;
  curThreadSize=initThreadSize;
  //创建线程对象时将线程函数给线程对象
  for(int i=0;i<initThreadSize;i++)
  {
    // auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this));//创建新线程
    auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this,placeholders::_1));//创建新线程
    int threadId_=ptr->getThreadId();
    threads.emplace(threadId_,move(ptr));//unique_ptr不允许资源的复制的
    //threads.emplace_back(move(ptr));
    //threads.emplace_back(new Thread(bind(&ThreadPool::threadFunc,this)));//绑定器与函数对象
  }
  //启动所有的线程
  for(int i=0;i<initThreadSize;i++)
  {
    threads[i]->start();//线程内部的函数,启动线程，执行其线程函数
    idleThreadSize_++;//启动一个线程则增加空余线程的数量
    //  ThreadPool::threadHandler() //线程的启动是启动线程函数的
    //  {
         

    //  }
  }

}
void Thread::start()//创建一个线程，即执行一个线程函数
{
  thread t(func_,threadId_);
  t.detach();//设置分离线程,因为即使线程结束了，线程函数也不能被释放==>linux中的pthread_detach pthread_t设置为分离线程


}
int Thread::getThreadId()const
{
   return threadId_;
}
Task::Task():result_(nullptr)
{

}
void Task::exec()//对TASK方法的实现
{
    // run();//此处做run的调用,还可以增加更多功能，因为run本身是纯虚函数
    if(result_!=nullptr){
       result_->setValue(run());//多态调用
    }
   

}
void Task::setResult(Result* res){
    result_=res;//初始化私有成员变量
}
int Thread::generateID_=0;//静态成员变量
void ThreadPool::threadFunc(int threadid)//线程函数结束则线程结束
{
  // cout<<"begin threadFunc"<<endl;
  // cout<<this_thread::get_id()<<endl;
  // cout<<"end threadFunc"<<endl;
  // cout<<this_thread::get_id()<<endl;
  auto lastTime=chrono::high_resolution_clock().now();//获取线程的当前时间
  while(isStartPool_)//不断循环取任务
  //当所有的任务均完成以后才可以回收线程资源，此处改为无限循环,在内部判断
  {
    shared_ptr<Task>task;
    {
    //获取锁(只是在操作任务队列时使用，取出之后即可释放)
    unique_lock<mutex>locker(taskQueueMtx);
    cout<<"tid"<<" "<<this_thread::get_id()<<"---try to get task!---"<<endl;
    //cached模式下,可能创建的很多线程,若超过空闲时间,则将多余的线程结束回收,当前使时间-上一执行时间>60
     //判断是否超时或者任务执行返回
     while( isStartPool_&&task_list.size()==0)//任务队列中无任务才需要等待
     {
       if(poolMode==PoolMode::MODE_CACHED)//Cached模式
    {
      if(cv_status::no_timeout==notEmpty.wait_for(locker,chrono::seconds(1)))//表明条件变量超时返回
      {
         auto nowTime=chrono::high_resolution_clock().now();
         auto durTime=chrono::duration_cast<chrono::seconds>(nowTime-lastTime);
          if(durTime.count()>=60&&curThreadSize>initThreadSize)//不可持续回收
             {
              //回收当前的线程
              //修改记录线程数量的相关值
              //将线程对象从线程列表容器中删除,但此处无法理解threadFunc中对应哪个线程对象
              //threadid-->thread对象-->删除
              threads.erase(threadid);//删除该线程
              curThreadSize--;
              idleThreadSize_--;
              cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
              return;
             }
      }
     }
      else{//fixed模式
           //等待非空条件
    notEmpty.wait(locker);//如果没有任务，一直等待即可
    }
    if(!isStartPool_)//检查是有任务被唤醒或是整个线程池将结束被唤醒
    {
         //线程池结束，回收线程资源
            //  threads.erase(threadid);//整个线程池要结束,回收该线程
            //   cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
            //   exitCond_.notify_all();//唤醒退出线程的标志
            //   return;
            break;
    }
     }
    //从任务队列中取出一个
    idleThreadSize_--;
    cout<<"tid"<<" "<<this_thread::get_id<<"get the task successfully!"<<endl;
    task=task_list.front();//取出任务后
    task_list.pop();//从队列中删除
    taskSize--;
    if(task_list.size()>0)
    {
      notEmpty.notify_all();//通知不空
    }
  
    }//需要释放锁,这个大括号相当于作用域
    notFull.notify_all();//取出任务,即通知生产
    //当前线程负责执行任务
    if(task!=nullptr)//task不是空指针,则执行任务
    {
      //task->run();//基类指针指向哪个派生类对象,即可调用相应的派生类同名函数方法
      task->exec();
    }
    idleThreadSize_++;//线程完成任务
    lastTime=chrono::high_resolution_clock().now();//更新当前线程调度执行完任务的时间
  }
      threads.erase(threadid);//线程函数结束,则线程也结束
      cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
}

Thread::Thread(ThreadFunc func):func_(func),threadId_(generateID_++)//Thread的构造函数
{
      cout<<"the constructor function of Thread is called!"<<endl;

}
 
Thread::~Thread()//析构函数
{
      cout<<"the deconstructor function of Thread is called!"<<endl;
}
//任务
Result::Result(shared_ptr<Task>task,bool isValid):isValid_(isValid),task_(task)
{ 
  task->setResult(this);
}
void Result::setValue(Any1 any)//
{
  this->any_=move(any);//不需要赋值构造
  sem_.post();//获取任务的返回值，增加信号量资源
}
Any1 Result::getValue(){//用户调用
  if(!isValid_)
  {
    return "";
  }
  sem_.wait();//等待任务被完全执行
  return move(any_);

}




// #include<chrono>
// int main()
// {
//     ThreadPool pool;
//     pool.Start();
//     this_thread::sleep_for(chrono::seconds(5));
//     return 0;
// }


#include"threadpool.h"
#include<iostream>
#include<thread>
#include<chrono>
#include<memory>//unique_ptr是包含在memory库中
// #define uLong long long
using namespace std;
using uLong=unsigned long long;
/*
某些场景可能会返回任务的返回值
比如求和:
thread1:1+...1000
thred2:1001+...2000;
thread:2001+...3000
*/
class Any1;
class MyTask:public Task{
   public:
    MyTask(int begin,int end):begin_(begin),end_(end)
    {

    }
     Any1 run()//自定义线程函数
     {
      //Q1:如何设计run的返回值接受任意类型,参考java中的Object思想，它是所有类型的基类==>C++17中的Any1类型:Any1 run()
      cout<<"tid"<<" "<<this_thread::get_id()<<"begin!"<<endl;
      uLong sum=0;
      for(uLong i=begin_;i<=end_;i++)
         sum+=i;
      this_thread::sleep_for(chrono::seconds(2));
      cout<<"tid"<<" "<<this_thread::get_id<<"finish!"<<endl;
      // cerr<<this_thread::get_id<<endl;
        return sum;
     }
     private:
       int begin_;
       int end_;
};
//Any1类型:接受任意数据的类型
// class Any1{
//   public:
//     template<typename T>
//       Any1(T data):base_(new Derive<T>(data))//基类指针指向派生类的对象
//       {}
//       template<typename T>//Any1类型接收任意其他类型的数据,将Any1对象里面的数据提取出来
//       T cast_()
//       {
//         //如何从base_指向的Derive<T>对象中取出data成员
//         Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//基类指针==>派生类指针 RTTI
//         if(pd==nullptr)
//         {
//           // cout<<"change wrongly!"<<endl;
//           throw "type is wrong!";
//         }
//         return pd->data_;
//       }

//   private:
//      class Base{//基类
//       public:
//         virtual ~Base()=default;//继承结构中,基类的派生对象如是在堆上创建,派生类的就无法调用基类的析构函数 
//      };
//      template<typename T>
//      class Derive:public Base{//派生类
//        public:
//           Derive(T data):data_(data)
//           {}
//         private:
//           T data_;

//      };
//     private:
//       unique_ptr<Base>base_;//定义基类的指针,可以指向任意对象
// };

int main()
{
  //ThreadPool对象析构以后,如何将线程池相关的资源全部回收
  ThreadPool pool;
  pool.setMode(PoolMode::MODE_CACHED);//设置模式
  pool.Start(4);
  // this_thread::sleep_for(chrono::seconds(5));
  // Result res=pool.submitTask(make_shared<MyTask>());//如何设计此处的Result机制
  /*此处task被执行完后，对象即被销毁*/
  // int sum=res.get().cast_<long>();
  // res.get().cast_<>();//返回的类型对象
  // pool.submitTask(make_shared<MyTask>());//自定义提交3个任务
  // pool.submitTask(make_shared<MyTask>());
  // pool.submitTask(make_shared<MyTask>());
  // getchar();//暂停，输入
  // return 0;
  Result res1=pool.submitTask(make_shared<MyTask>(1,1000000));
  Result res2=pool.submitTask(make_shared<MyTask>(10000001,2000000));
  Result res3=pool.submitTask(make_shared<MyTask>(20000001,3000000));
  uLong sum1=res1.getValue().cast_<uLong>();
  uLong sum2=res2.getValue().cast_<uLong>();
  uLong sum3=res3.getValue().cast_<uLong>();
  cout<<(sum1+sum2+sum3)<<endl;//Master-Slave线程模型,Master线程分解任务,分配给Slave,并等待Slave完成的结果由Master合并各个任务结果
  getchar();
  return 0;

  

}

