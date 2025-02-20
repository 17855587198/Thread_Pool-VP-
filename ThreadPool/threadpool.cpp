#include<iostream>
#include"threadpool.h"//Win�϶���ͷ�ļ��Ĵ�Сд��������
#include<functional>
#include<thread>
#include<mutex>
#include<memory>
const int TASK_MAX_THRESHHOLD=1024;//���Խ�������е���������
const int THREAD_SIZE_THRESHHOLD_=20;
const int THREAD_MAX_TIME=60;
using namespace  std;
ThreadPool::ThreadPool()
      :initThreadSize(4),
      threadSizeThreshHold_(THREAD_SIZE_THRESHHOLD_),//�̵߳�����
       taskSize(0),
       taskSizeMaxThreshhold(TASK_MAX_THRESHHOLD),
       poolMode(PoolMode::MODE_FIXED),
       curThreadSize(0),
       idleThreadSize_(0)//����޿����߳�
{
    cout<<"the constructor function of ThreadPoool is called!"<<endl;
}
ThreadPool::~ThreadPool()
{
  isStartPool_=false;//����Ϊfalse 
  std::unique_lock<mutex>locker(taskQueueMtx);//���������ʹ��   
  notEmpty.notify_all();
  exitCond_.wait(locker,[&]()->bool{return threads.size()==0;});//�������0���ȴ�,����ȴ�������(��Ϊ0���ͷ�)

  //�̳߳��е��̷߳���������״̬:������������ִ��������

  cout<<"the deconstructor function of ThreadPool is called!"<<endl;

}
bool ThreadPool::checkRunningState() const
{
   return isStartPool_;//�ж��Ƿ����߳�����
}
void ThreadPool::setMode(PoolMode mode){
  if(checkRunningState())//�Ѿ������˲�������ģʽ
     return;
    poolMode=mode;
}
void ThreadPool::InitThreadSize(int size)
{
   this->initThreadSize=size;
}
void ThreadPool::setThreadSizeThresHold(int threshHold)
{
  /*�̳߳����Ѿ���������������*/
   if(checkRunningState())
      return;
    if(poolMode==PoolMode::MODE_CACHED)//��һ�μ��ģʽ
       threadSizeThreshHold_=threshHold;//�����̵߳��������
}
void ThreadPool::setTaskQueueThresHold(int threshhold)
{
   if(checkRunningState())
     return;
    taskSizeMaxThreshhold=threshhold;
}
Result ThreadPool::submitTask(shared_ptr<Task> sp)//�̳߳��ύ����
{
    //��ȡ��
    unique_lock<mutex>locker(taskQueueMtx);//��ȡ������е���
    // while(task_list.size()==taskSizeMaxThreshhold)//�ﵽ����,����ȴ�̬���ͷ���.���Ѻ����������ܱ�Ϊ����̬
    // {
    //     notFull.wait(locker);//��ǰ�߳̽���ȵ�״̬��Ҳ�ͷŵ�ǰ����.�ȴ���һ����������
    // }
    //�����ύ�������������һ��,�����ύʧ�ܣ�����
    if(!notFull.wait_for(locker,chrono::seconds(1),[&]()->bool{return task_list.size()<taskSizeMaxThreshhold;}))//lambda���ʽ,&�ǲ�����ʽ
    {//�˴�ȡ�����뱾���෴
  
       //ֻ�ȴ�1��
       cerr<<"Fail to submit the task!"<<endl;//ʱ��������ύ����ʧ��
       return Result(sp,false);//������ȷ���͵�����,�ύʧ���򷵻���Ч�ķ���ֵ
    }
    //�Ƚ�wait-->һֱ�ȣ�ֱ������,wait_for-->(�ȴ�ʱ��),wait_until-->(�ȵ�ʲôʱ��)
    task_list.emplace(sp);//��������
    taskSize++;
    // notFull.wait(locker,[&]()->bool{return task_list.size()==taskSizeMaxThreshhold;})//lambda���ʽ,&�ǲ�����ʽ
    //�̵߳�ͨ��,�ȴ�������еĿ���,������,���������
    //������������,����в���,��notEmpty֪ͨ
    notEmpty.notify_all();//��������,�����߳�ִ������
    //����Task��Result����
    // return task->getresult;//������
    // return Result(task);
    /*��������������Ϳ����߳�������̬�����߳�(cachedģʽ):С���������*/
    if(poolMode==PoolMode::MODE_CACHED&&taskSize>=idleThreadSize_&&curThreadSize<threadSizeThreshHold_)
    {
      //�����µ��߳�
    auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this,placeholders::_1));//�������߳�
    int threadId_=ptr->getThreadId();
    threads.emplace(threadId_,move(ptr));
    threads[threadId_]->start();//�����߳�
    curThreadSize++;//�޸ĵ�ǰ�̵߳�����
    idleThreadSize_++;//�����߳�����
    //  auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this));
    //  threads.emplace_back(move(ptr));//unique_ptr��������Դ�ĸ��Ƶ�

    }

    return Result(sp);//Ĭ�ϵ���true


}
void ThreadPool::Start(int initThreadSize)//����Ĭ������(�����̳߳صķ���ʵ��)
{
  isStartPool_=true;//�̳߳�������״̬
  initThreadSize=initThreadSize;
  curThreadSize=initThreadSize;
  //�����̶߳���ʱ���̺߳������̶߳���
  for(int i=0;i<initThreadSize;i++)
  {
    // auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this));//�������߳�
    auto ptr=make_unique<Thread>(bind(&ThreadPool::threadFunc,this,placeholders::_1));//�������߳�
    int threadId_=ptr->getThreadId();
    threads.emplace(threadId_,move(ptr));//unique_ptr��������Դ�ĸ��Ƶ�
    //threads.emplace_back(move(ptr));
    //threads.emplace_back(new Thread(bind(&ThreadPool::threadFunc,this)));//�����뺯������
  }
  //�������е��߳�
  for(int i=0;i<initThreadSize;i++)
  {
    threads[i]->start();//�߳��ڲ��ĺ���,�����̣߳�ִ�����̺߳���
    idleThreadSize_++;//����һ���߳������ӿ����̵߳�����
    //  ThreadPool::threadHandler() //�̵߳������������̺߳�����
    //  {
         

    //  }
  }

}
void Thread::start()//����һ���̣߳���ִ��һ���̺߳���
{
  thread t(func_,threadId_);
  t.detach();//���÷����߳�,��Ϊ��ʹ�߳̽����ˣ��̺߳���Ҳ���ܱ��ͷ�==>linux�е�pthread_detach pthread_t����Ϊ�����߳�


}
int Thread::getThreadId()const
{
   return threadId_;
}
Task::Task():result_(nullptr)
{

}
void Task::exec()//��TASK������ʵ��
{
    // run();//�˴���run�ĵ���,���������Ӹ��๦�ܣ���Ϊrun�����Ǵ��麯��
    if(result_!=nullptr){
       result_->setValue(run());//��̬����
    }
   

}
void Task::setResult(Result* res){
    result_=res;//��ʼ��˽�г�Ա����
}
int Thread::generateID_=0;//��̬��Ա����
void ThreadPool::threadFunc(int threadid)//�̺߳����������߳̽���
{
  // cout<<"begin threadFunc"<<endl;
  // cout<<this_thread::get_id()<<endl;
  // cout<<"end threadFunc"<<endl;
  // cout<<this_thread::get_id()<<endl;
  auto lastTime=chrono::high_resolution_clock().now();//��ȡ�̵߳ĵ�ǰʱ��
  while(isStartPool_)//����ѭ��ȡ����
  //�����е����������Ժ�ſ��Ի����߳���Դ���˴���Ϊ����ѭ��,���ڲ��ж�
  {
    shared_ptr<Task>task;
    {
    //��ȡ��(ֻ���ڲ����������ʱʹ�ã�ȡ��֮�󼴿��ͷ�)
    unique_lock<mutex>locker(taskQueueMtx);
    cout<<"tid"<<" "<<this_thread::get_id()<<"---try to get task!---"<<endl;
    //cachedģʽ��,���ܴ����ĺܶ��߳�,����������ʱ��,�򽫶�����߳̽�������,��ǰʹʱ��-��һִ��ʱ��>60
     //�ж��Ƿ�ʱ��������ִ�з���
     while( isStartPool_&&task_list.size()==0)//������������������Ҫ�ȴ�
     {
       if(poolMode==PoolMode::MODE_CACHED)//Cachedģʽ
    {
      if(cv_status::no_timeout==notEmpty.wait_for(locker,chrono::seconds(1)))//��������������ʱ����
      {
         auto nowTime=chrono::high_resolution_clock().now();
         auto durTime=chrono::duration_cast<chrono::seconds>(nowTime-lastTime);
          if(durTime.count()>=60&&curThreadSize>initThreadSize)//���ɳ�������
             {
              //���յ�ǰ���߳�
              //�޸ļ�¼�߳����������ֵ
              //���̶߳�����߳��б�������ɾ��,���˴��޷����threadFunc�ж�Ӧ�ĸ��̶߳���
              //threadid-->thread����-->ɾ��
              threads.erase(threadid);//ɾ�����߳�
              curThreadSize--;
              idleThreadSize_--;
              cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
              return;
             }
      }
     }
      else{//fixedģʽ
           //�ȴ��ǿ�����
    notEmpty.wait(locker);//���û������һֱ�ȴ�����
    }
    if(!isStartPool_)//����������񱻻��ѻ��������̳߳ؽ�����������
    {
         //�̳߳ؽ����������߳���Դ
            //  threads.erase(threadid);//�����̳߳�Ҫ����,���ո��߳�
            //   cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
            //   exitCond_.notify_all();//�����˳��̵߳ı�־
            //   return;
            break;
    }
     }
    //�����������ȡ��һ��
    idleThreadSize_--;
    cout<<"tid"<<" "<<this_thread::get_id<<"get the task successfully!"<<endl;
    task=task_list.front();//ȡ�������
    task_list.pop();//�Ӷ�����ɾ��
    taskSize--;
    if(task_list.size()>0)
    {
      notEmpty.notify_all();//֪ͨ����
    }
  
    }//��Ҫ�ͷ���,����������൱��������
    notFull.notify_all();//ȡ������,��֪ͨ����
    //��ǰ�̸߳���ִ������
    if(task!=nullptr)//task���ǿ�ָ��,��ִ������
    {
      //task->run();//����ָ��ָ���ĸ����������,���ɵ�����Ӧ��������ͬ����������
      task->exec();
    }
    idleThreadSize_++;//�߳��������
    lastTime=chrono::high_resolution_clock().now();//���µ�ǰ�̵߳���ִ���������ʱ��
  }
      threads.erase(threadid);//�̺߳�������,���߳�Ҳ����
      cout<<"threadid:"<<this_thread::get_id()<<"exit!"<<endl;
}

Thread::Thread(ThreadFunc func):func_(func),threadId_(generateID_++)//Thread�Ĺ��캯��
{
      cout<<"the constructor function of Thread is called!"<<endl;

}
 
Thread::~Thread()//��������
{
      cout<<"the deconstructor function of Thread is called!"<<endl;
}
//����
Result::Result(shared_ptr<Task>task,bool isValid):isValid_(isValid),task_(task)
{ 
  task->setResult(this);
}
void Result::setValue(Any1 any)//
{
  this->any_=move(any);//����Ҫ��ֵ����
  sem_.post();//��ȡ����ķ���ֵ�������ź�����Դ
}
Any1 Result::getValue(){//�û�����
  if(!isValid_)
  {
    return "";
  }
  sem_.wait();//�ȴ�������ȫִ��
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
#include<memory>//unique_ptr�ǰ�����memory����
// #define uLong long long
using namespace std;
using uLong=unsigned long long;
/*
ĳЩ�������ܻ᷵������ķ���ֵ
�������:
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
     Any1 run()//�Զ����̺߳���
     {
      //Q1:������run�ķ���ֵ������������,�ο�java�е�Object˼�룬�����������͵Ļ���==>C++17�е�Any1����:Any1 run()
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
//Any1����:�����������ݵ�����
// class Any1{
//   public:
//     template<typename T>
//       Any1(T data):base_(new Derive<T>(data))//����ָ��ָ��������Ķ���
//       {}
//       template<typename T>//Any1���ͽ��������������͵�����,��Any1���������������ȡ����
//       T cast_()
//       {
//         //��δ�base_ָ���Derive<T>������ȡ��data��Ա
//         Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//����ָ��==>������ָ�� RTTI
//         if(pd==nullptr)
//         {
//           // cout<<"change wrongly!"<<endl;
//           throw "type is wrong!";
//         }
//         return pd->data_;
//       }

//   private:
//      class Base{//����
//       public:
//         virtual ~Base()=default;//�̳нṹ��,������������������ڶ��ϴ���,������ľ��޷����û������������ 
//      };
//      template<typename T>
//      class Derive:public Base{//������
//        public:
//           Derive(T data):data_(data)
//           {}
//         private:
//           T data_;

//      };
//     private:
//       unique_ptr<Base>base_;//��������ָ��,����ָ���������
// };

int main()
{
  //ThreadPool���������Ժ�,��ν��̳߳���ص���Դȫ������
  ThreadPool pool;
  pool.setMode(PoolMode::MODE_CACHED);//����ģʽ
  pool.Start(4);
  // this_thread::sleep_for(chrono::seconds(5));
  // Result res=pool.submitTask(make_shared<MyTask>());//�����ƴ˴���Result����
  /*�˴�task��ִ����󣬶��󼴱�����*/
  // int sum=res.get().cast_<long>();
  // res.get().cast_<>();//���ص����Ͷ���
  // pool.submitTask(make_shared<MyTask>());//�Զ����ύ3������
  // pool.submitTask(make_shared<MyTask>());
  // pool.submitTask(make_shared<MyTask>());
  // getchar();//��ͣ������
  // return 0;
  Result res1=pool.submitTask(make_shared<MyTask>(1,1000000));
  Result res2=pool.submitTask(make_shared<MyTask>(10000001,2000000));
  Result res3=pool.submitTask(make_shared<MyTask>(20000001,3000000));
  uLong sum1=res1.getValue().cast_<uLong>();
  uLong sum2=res2.getValue().cast_<uLong>();
  uLong sum3=res3.getValue().cast_<uLong>();
  cout<<(sum1+sum2+sum3)<<endl;//Master-Slave�߳�ģ��,Master�̷ֽ߳�����,�����Slave,���ȴ�Slave��ɵĽ����Master�ϲ�����������
  getchar();
  return 0;

  

}

