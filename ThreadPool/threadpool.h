#ifndef THREADPOOL_H//此处的THREADPOOL_H是一个宏,约定此类方式命名.将宏名与文件名关联,避免对头文件的重复宏冲突
#define THREADPOOL_H//"#ifndef XX" "#define XX" "endif XX"
#include<vector>
#include<thread>
#include<functional>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<unordered_map>//对线程排序
using namespace std;
/*
example:
ThreadPool pool;
pool.start(4);
pool.submitTask();
class MyTask()
{
public:
  void run()
  {
    //线程代码
  }
  pool.submitTask(make_shared<MyTask>());
}
*/
class Semaphore{//实现信号量(atomic用于轻量级的锁即可)
   public:
      Semaphore(int limit=0):reslimit_(limit){
      }
      ~Semaphore(){

      }
      void wait()//获取信号量资源
      {
        unique_lock<mutex>lock(mtx_);
        //等待信号量有资源,没有资源即阻塞当前线程
        cond_.wait(lock,[&]()->bool{return reslimit_>0;});
        reslimit_--;
      }
      void post()//增加信号量资源
      {
        unique_lock<mutex>lock(mtx_);
        reslimit_++;
        cond_.notify_all();
      }
   private:
      int reslimit_;
      mutex mtx_;
      condition_variable cond_;


};
class Any1{//接受任何类型的返回值
  public:
   Any1()=default;
    template<typename T>
      Any1(T data):base_(new Derive<T>(data))//基类指针指向派生类的对象
      {}
      template<typename T>//Any1类型接收任意其他类型的数据,将Any1对象里面的数据提取出来
      T cast_()
      {
        //如何从base_指向的Derive<T>对象中取出data成员
        Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//基类指针==>派生类指针 RTTI
        if(pd==nullptr)
        {
          // cout<<"change wrongly!"<<endl;
          throw "type is wrong!";
        }
        return pd->getData();
      }

  private:
     class Base{//基类
      public:
        virtual ~Base()=default;//继承结构中,基类的派生对象如是在堆上创建,派生类的就无法调用基类的析构函数 
     };
     template<typename T>
     class Derive:public Base{//派生类
       public:
          Derive(T data):data_(data)
          {}
          T getData()const{
            return data_;
          }
        private:
          T data_;

     };
    private:
      unique_ptr<Base>base_;//定义基类的指针,可以指向任意对象
};


class Task;//Result需使用Task对象
class Result{//接收提交到线程池的task任务执行完成后的返回值类型
  public:
    Result(shared_ptr<Task>task,bool isValid=true);
    ~Result()=default;
    Any1 getValue();//获取任务执行完的返回值
    //getValue()//用户获取task的返回值
    void setValue(Any1 any);

  private:
     Any1 any_;//存储任务的返回值
     Semaphore sem_;//通信信号量,用户需要等到执行任务完成后再拿到返回值
     shared_ptr<Task>task_;//指向对应获取返回值的任务对象
     atomic_bool isValid_;//返回值是否有效,任务提交失败则返回值无效 
};
//任务抽象基类
class Task{
    public:
    Task();
    ~Task()=default;
     void exec();//调用exec函数执行
     void setResult(Result *res);
     virtual Any1 run()=0;//用户自定义任务类型,重写run方法,自定义任务处理
    private:
      Result *result_;//task里面有一个成员变量获取到result,此处不使用智能指针是防止指针的交叉引用
};
class Thread//自定义线程
{
    public:
      void start();//线程自带的启动函数
       using ThreadFunc=function<void(int)>;//functional是一个可调用对象的封装器,可存储,复制或调用任何对象,void(int)是一个函数签名,表示可以接受任何int类型的参数，返回void
       Thread(ThreadFunc func);
       ~Thread();
       int getThreadId() const;
      private:
      //自定义线程函数
        ThreadFunc func_;
        static int generateID_;
        int threadId_;//借助ID销毁线程
        


};
enum class PoolMode{
  MODE_FIXED,//固定数量
  MODE_CACHED,//动态增长
};
//线程类型
//线程池类型
class ThreadPool{
    public:
       ThreadPool();
       ~ThreadPool();
       void Start(int initThreadSize=4);//开启线程池
       void setMode(PoolMode mode);//设置线程池的工作模式
       void InitThreadSize(int size);//设置线程池的初始数量
       void setTaskQueueThresHold(int threshhold);//定义任务队列的上限
       void setThreadSizeThresHold(int threshHold);//mode模式下线程池的线程数量的阈值
       Result submitTask(shared_ptr<Task> sp);//提交任务对象
       ThreadPool(const ThreadPool&)=delete;//禁止线程池的拷贝构造
       ThreadPool& operator=(const ThreadPool&)=delete;//禁止线程池的复制构造
    private:
      void threadFunc(int threadid);//定义线程执行函数,含有参数的函数被绑定时需使用参数占位符
      bool checkRunningState() const;//判断情况
      PoolMode poolMode;
      // vector<Thread*> threads;//定义线程列表
      // vector<unique_ptr<Thread>>threads;//使用智能的指针指向对象(取代原有的裸指针)
      unordered_map<int,unique_ptr<Thread>>threads;//线程列表,将线程与线程ID绑定在一起(根据ID销毁线程资源)
      size_t initThreadSize;//定义初始的线程大小
      atomic_int curThreadSize;//当前的线程数量(因为当前的线程数量受到各个线程的影响)
      int threadSizeThreshHold_;//线程剩余数量的上限阈值
      atomic_int idleThreadSize_;//记录空闲线程的数量
      queue<shared_ptr<Task>> task_list; //任务队列,此处简单使用裸指针是不行的，需要考虑到任务的生命周期
      atomic_int taskSize;//表示任务的数量
      int taskSizeMaxThreshhold;//设置任务的阈值
      mutex taskQueueMtx;//保证任务队列的线程安全
      condition_variable notFull;//任务队列不满
      condition_variable notEmpty;//任务队列不空
      std::condition_variable exitCond_;//等待线程资源的全部回收
      atomic_bool isStartPool_;//判断当前线程池的启动状态(atomic_bool,多个线程需要判断)
     

};

#endif