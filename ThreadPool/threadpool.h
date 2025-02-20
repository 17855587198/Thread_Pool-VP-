#ifndef THREADPOOL_H//�˴���THREADPOOL_H��һ����,Լ�����෽ʽ����.���������ļ�������,�����ͷ�ļ����ظ����ͻ
#define THREADPOOL_H//"#ifndef XX" "#define XX" "endif XX"
#include<vector>
#include<thread>
#include<functional>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<unordered_map>//���߳�����
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
    //�̴߳���
  }
  pool.submitTask(make_shared<MyTask>());
}
*/
class Semaphore{//ʵ���ź���(atomic������������������)
   public:
      Semaphore(int limit=0):reslimit_(limit){
      }
      ~Semaphore(){

      }
      void wait()//��ȡ�ź�����Դ
      {
        unique_lock<mutex>lock(mtx_);
        //�ȴ��ź�������Դ,û����Դ��������ǰ�߳�
        cond_.wait(lock,[&]()->bool{return reslimit_>0;});
        reslimit_--;
      }
      void post()//�����ź�����Դ
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
class Any1{//�����κ����͵ķ���ֵ
  public:
   Any1()=default;
    template<typename T>
      Any1(T data):base_(new Derive<T>(data))//����ָ��ָ��������Ķ���
      {}
      template<typename T>//Any1���ͽ��������������͵�����,��Any1���������������ȡ����
      T cast_()
      {
        //��δ�base_ָ���Derive<T>������ȡ��data��Ա
        Derive<T>* pd=dynamic_cast<Derive<T>*>(base_.get());//����ָ��==>������ָ�� RTTI
        if(pd==nullptr)
        {
          // cout<<"change wrongly!"<<endl;
          throw "type is wrong!";
        }
        return pd->getData();
      }

  private:
     class Base{//����
      public:
        virtual ~Base()=default;//�̳нṹ��,������������������ڶ��ϴ���,������ľ��޷����û������������ 
     };
     template<typename T>
     class Derive:public Base{//������
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
      unique_ptr<Base>base_;//��������ָ��,����ָ���������
};


class Task;//Result��ʹ��Task����
class Result{//�����ύ���̳߳ص�task����ִ����ɺ�ķ���ֵ����
  public:
    Result(shared_ptr<Task>task,bool isValid=true);
    ~Result()=default;
    Any1 getValue();//��ȡ����ִ����ķ���ֵ
    //getValue()//�û���ȡtask�ķ���ֵ
    void setValue(Any1 any);

  private:
     Any1 any_;//�洢����ķ���ֵ
     Semaphore sem_;//ͨ���ź���,�û���Ҫ�ȵ�ִ��������ɺ����õ�����ֵ
     shared_ptr<Task>task_;//ָ���Ӧ��ȡ����ֵ���������
     atomic_bool isValid_;//����ֵ�Ƿ���Ч,�����ύʧ���򷵻�ֵ��Ч 
};
//����������
class Task{
    public:
    Task();
    ~Task()=default;
     void exec();//����exec����ִ��
     void setResult(Result *res);
     virtual Any1 run()=0;//�û��Զ�����������,��дrun����,�Զ���������
    private:
      Result *result_;//task������һ����Ա������ȡ��result,�˴���ʹ������ָ���Ƿ�ָֹ��Ľ�������
};
class Thread//�Զ����߳�
{
    public:
      void start();//�߳��Դ�����������
       using ThreadFunc=function<void(int)>;//functional��һ���ɵ��ö���ķ�װ��,�ɴ洢,���ƻ�����κζ���,void(int)��һ������ǩ��,��ʾ���Խ����κ�int���͵Ĳ���������void
       Thread(ThreadFunc func);
       ~Thread();
       int getThreadId() const;
      private:
      //�Զ����̺߳���
        ThreadFunc func_;
        static int generateID_;
        int threadId_;//����ID�����߳�
        


};
enum class PoolMode{
  MODE_FIXED,//�̶�����
  MODE_CACHED,//��̬����
};
//�߳�����
//�̳߳�����
class ThreadPool{
    public:
       ThreadPool();
       ~ThreadPool();
       void Start(int initThreadSize=4);//�����̳߳�
       void setMode(PoolMode mode);//�����̳߳صĹ���ģʽ
       void InitThreadSize(int size);//�����̳߳صĳ�ʼ����
       void setTaskQueueThresHold(int threshhold);//����������е�����
       void setThreadSizeThresHold(int threshHold);//modeģʽ���̳߳ص��߳���������ֵ
       Result submitTask(shared_ptr<Task> sp);//�ύ�������
       ThreadPool(const ThreadPool&)=delete;//��ֹ�̳߳صĿ�������
       ThreadPool& operator=(const ThreadPool&)=delete;//��ֹ�̳߳صĸ��ƹ���
    private:
      void threadFunc(int threadid);//�����߳�ִ�к���,���в����ĺ�������ʱ��ʹ�ò���ռλ��
      bool checkRunningState() const;//�ж����
      PoolMode poolMode;
      // vector<Thread*> threads;//�����߳��б�
      // vector<unique_ptr<Thread>>threads;//ʹ�����ܵ�ָ��ָ�����(ȡ��ԭ�е���ָ��)
      unordered_map<int,unique_ptr<Thread>>threads;//�߳��б�,���߳����߳�ID����һ��(����ID�����߳���Դ)
      size_t initThreadSize;//�����ʼ���̴߳�С
      atomic_int curThreadSize;//��ǰ���߳�����(��Ϊ��ǰ���߳������ܵ������̵߳�Ӱ��)
      int threadSizeThreshHold_;//�߳�ʣ��������������ֵ
      atomic_int idleThreadSize_;//��¼�����̵߳�����
      queue<shared_ptr<Task>> task_list; //�������,�˴���ʹ����ָ���ǲ��еģ���Ҫ���ǵ��������������
      atomic_int taskSize;//��ʾ���������
      int taskSizeMaxThreshhold;//�����������ֵ
      mutex taskQueueMtx;//��֤������е��̰߳�ȫ
      condition_variable notFull;//������в���
      condition_variable notEmpty;//������в���
      std::condition_variable exitCond_;//�ȴ��߳���Դ��ȫ������
      atomic_bool isStartPool_;//�жϵ�ǰ�̳߳ص�����״̬(atomic_bool,����߳���Ҫ�ж�)
     

};

#endif